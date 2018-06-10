/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_handle_pool.cpp
 * @brief multi-thread's pool, each thread is responsible for one tablet
 *
 * Created by longfei： multi-thread to construct secondary index
 * future work
 *   1.some function need to be realized,see todo list in this page
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */

#include "ob_index_handle_pool.h"
#include "tbsys.h"
#include "ob_chunk_server_main.h"
#include "common/ob_read_common_data.h"
#include "ob_tablet_image.h"
#include "common/utility.h"
#include "sstable/ob_disk_path.h"
#include "common/ob_trace_log.h"
#include "ob_tablet_manager.h"
#include "common/ob_atomic.h"
#include "common/file_directory_utils.h"
#include "ob_index_handler.h"
#include "common/hash/ob_hashmap.h"
#include "common/ob_scanner.h"
#include "common/ob_mod_define.h"
#include "ob_tablet_manager.h"
#include "ob_local_index_handler.h"
#include "ob_global_index_handler.h"


using namespace oceanbase::common;

namespace oceanbase
{
  namespace chunkserver
  {
    using namespace tbutil;
    using namespace common;
    using namespace sstable;

    /*-----------------------------------------------------------------------------
     *  Thread for index building
     *-----------------------------------------------------------------------------*/

    ObIndexHandlePool::ObIndexHandlePool() :
      inited_(false), thread_num_(0), process_idx_tid_(OB_INVALID_ID),
      schedule_idx_tid_(OB_INVALID_ID), tablet_index_(0), range_index_(0),
      hist_width_(0), tablets_have_got_(0), active_thread_num_(0),
      min_work_thread_num_(0), round_start_(ROUND_TRUE), round_end_(TABLET_RELEASE),
      which_stage_(STAGE_INIT)
    {
      total_work_start_time_ = 0;
      static_index_report_infolist = OB_NEW(ObTabletHistogramReportInfoList,
                                            ObModIds::OB_TABLET_HISTOGRAM_REPORT);
    }

    int ObIndexHandlePool::init(ObTabletManager *manager)
    {
      int ret = OB_SUCCESS;
      ObChunkServer& chunk_server =
          ObChunkServerMain::get_instance()->get_chunk_server();
      if (NULL == manager)
      {
        TBSYS_LOG(ERROR, "initialize index worker failed,null pointer");
        ret = OB_ERROR;
      }
      else if (!inited_)
      {
        inited_ = true;
        tablet_manager_ = manager;
        schema_mgr_ = chunk_server.get_schema_manager();
        pthread_mutex_init(&mutex_, NULL);
        pthread_mutex_init(&stage_mutex_, NULL);
        pthread_cond_init(&cond_, NULL);

        int64_t max_work_thread_num =
            chunk_server.get_config().max_merge_thread_num;
        if (max_work_thread_num <= 0 || max_work_thread_num > MAX_WORK_THREAD)
          max_work_thread_num = MAX_WORK_THREAD;

        TBSYS_LOG(INFO,"config merge thread num[%ld],MAX WORK THREAD[%ld]",max_work_thread_num,MAX_WORK_THREAD);

        // 创建两个hashmap,用来保存rs切分的range
        if (OB_SUCCESS != (ret = set_config_param()))
        {
          TBSYS_LOG(ERROR, "failed to set index work param[%d]", ret);
        }
        // 调用tbsys::CDefaultRunnable的start函数来启动多个线程
        else if (OB_SUCCESS != (ret = create_work_thread(max_work_thread_num)))
        {
          TBSYS_LOG(ERROR, "failed to initialize thread for index[%d]", ret);
        }
        // 为每一个handler(完成索引构建的类)分配空间
        else if (OB_SUCCESS != (ret = create_all_index_handlers()))
        {
          TBSYS_LOG(ERROR, "failed to create all index handler[%d]", ret);
        }
        // 初始化处理索引构建失败信息保存类
        else if (OB_SUCCESS != (ret = black_list_array_.init()))
        {
          TBSYS_LOG(ERROR, "failed to init black list array");
        }
        else
        {
          TBSYS_LOG(INFO,"init index handle pool succ");
        }
      }
      else
      {
        TBSYS_LOG(WARN, "index handle pool has been inited!");
      }
      if (OB_SUCCESS != ret && inited_)
      {
        pthread_mutex_destroy(&mutex_);
        pthread_mutex_destroy(&stage_mutex_);
        pthread_cond_destroy(&cond_);
        inited_ = false;
      }
      return ret;
    }

    int ObIndexHandlePool::create_work_thread(const int64_t max_work_thread)
    {
      int ret = OB_SUCCESS;
      setThreadCount(static_cast <int32_t>(max_work_thread));
      active_thread_num_ = max_work_thread;
      thread_num_ = start();

      if (0 >= thread_num_)
      {
        TBSYS_LOG(ERROR, "start thread failed!");
        ret = OB_ERROR;
      }
      else
      {
        if (thread_num_ != max_work_thread)
        {
          TBSYS_LOG(WARN, "failed to start [%ld] threads to build index, there is [%ld] threads",
                    max_work_thread, thread_num_);
        }
        min_work_thread_num_ = thread_num_ / 3;//?
        if (0 == min_work_thread_num_)
          min_work_thread_num_ = 1;
        TBSYS_LOG(INFO, "index work thread_num=%ld "
                        "active_thread_num_=%ld, min_merge_thread_num_=%ld", thread_num_,
                  active_thread_num_, min_work_thread_num_);
      }

      return ret;
    }

    int ObIndexHandlePool::create_all_index_handlers()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != (ret = create_index_handlers(global_handler_, local_handler_, MAX_WORK_THREAD)))
      {
        TBSYS_LOG(ERROR, "failed to create index handlers");
      }
      return ret;
    }

    int ObIndexHandlePool::create_index_handlers(
        ObGlobalIndexHandler **global_handler,
        ObLocalIndexHandler **local_handler,
        const int64_t size)
    {
      int ret = OB_SUCCESS;
      char* ptr = NULL;
      if (NULL == global_handler || NULL == local_handler || 0 > size)
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "the pointer of index handler is null");
      }
      else if (NULL == (ptr = reinterpret_cast <char*>(ob_malloc((sizeof(ObGlobalIndexHandler) + sizeof(ObLocalIndexHandler)) * size, ObModIds::OB_INDEX_HANDLER))))
      {
        TBSYS_LOG(WARN, "allocate memory for index handler object error");
        ret = OB_ALLOCATE_MEMORY_FAILED;
      }
      else if (NULL == schema_mgr_)
      {
        TBSYS_LOG(ERROR, "schema_manager pointer is NULL");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (NULL == tablet_manager_)
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "tablet_manager pointer is NULL");
      }
      else
      {
        for (int64_t i = 0; i < size; i++)
        {
          ObLocalIndexHandler* local_handlers =
              new (ptr + i * sizeof(ObLocalIndexHandler)) ObLocalIndexHandler(this,
                                                                              schema_mgr_,
                                                                              tablet_manager_,
                                                                              static_index_report_infolist);
          if (NULL == local_handlers
              || OB_SUCCESS != (ret /* call init function*/))
          {
            TBSYS_LOG(WARN, "init index handler error, ret[%d]", ret);
            ret = OB_ERROR;
            break;
          }
          else
          {
            local_handler[i] = local_handlers;
          }
        }
        ptr = ptr + size * sizeof(ObLocalIndexHandler);
        for (int64_t i = 0; i < size; i++)
        {
          ObGlobalIndexHandler* global_handlers =
              new (ptr + i * sizeof(ObGlobalIndexHandler)) ObGlobalIndexHandler(this,
                                                                                schema_mgr_,
                                                                                tablet_manager_);
          if (NULL == global_handlers
              || OB_SUCCESS != (ret = global_handlers->init()))
          {
            TBSYS_LOG(WARN, "init index handler error, ret[%d]", ret);
            break;
          }
          else
          {
            global_handler[i] = global_handlers;

          }
        }
        //mod longfei
        if (OB_SUCCESS == ret)
        {
          TBSYS_LOG(INFO,"allocate memory for index handler success");
        }
//        TBSYS_LOG(INFO,"allocate memory for index handler finish");
        //mod e
      }
      return ret;
    }

    int ObIndexHandlePool::schedule()
    {
      int ret = OB_SUCCESS;
      if (!can_launch_next_round())
      {
        TBSYS_LOG(INFO, "cannot launch next round.");
        ret = OB_CS_EAGAIN;
      }
      else if (0 == tablet_manager_->get_serving_data_version())
      {
        TBSYS_LOG(INFO, "empty chunkserver, wait for data");
        ret = OB_CS_EAGAIN;
      }
      else
      {
        ret = start_round();
      }

      if (inited_ && OB_SUCCESS == ret && thread_num_ > 0)
      {
        local_work_start_time_ = tbsys::CTimeUtil::getTime();
        round_start_ = ROUND_TRUE;
        round_end_ = ROUND_FALSE;
        //准备好了tablet或者range之后就唤醒在CS创建的多线程
        TBSYS_LOG(INFO,"stage[%d] ready, send boradcast.",which_stage_);
        pthread_cond_broadcast(&cond_);
      }
      return ret;
    }

    int ObIndexHandlePool::start_round()
    {
      int ret = OB_SUCCESS;
      if (true)
      {
        //@todo 将process_idx_tid的数据表的tablet所加的附加局部索引sstable删除
        //注意，如果你process_idx_tid == schedule_idx_tid,则不能删除这个局部索引，因为这个时候的状态是处理上次没有完成的索引构建！！
        //既然已经唤醒了一个新的索引构建任务，那么上一次的任务必然是完成的，可能这个CS掉线导致没有同步上任务，那么这些
        //遗留下的局部索引sstable是应该删除的
      }
      if (process_idx_tid_ != schedule_idx_tid_)
      {
        process_idx_tid_ = schedule_idx_tid_;
      }
      if (LOCAL_INDEX_STAGE == which_stage_)
      {
        if (OB_SUCCESS != (ret = fetch_tablet_info(LOCAL_INDEX_STAGE)))
        {
          TBSYS_LOG(ERROR, "start build index,round local error");
        }
      }
      else if (GLOBAL_INDEX_STAGE == which_stage_)
      {
        if (OB_SUCCESS != (ret = fetch_tablet_info(GLOBAL_INDEX_STAGE)))
        {
          TBSYS_LOG(ERROR, "start build index,round global error");
        }
      }
      else
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN,
                  "can't understand the stage of cons static index,which_stage = %d",
                  (int) which_stage_);
      }
      return ret;
    }

    // add longfei [cons static index] 151124:b
    int ObIndexHandlePool::fetch_tablet_info(common::ConIdxStage which_stage)
    {
      int ret = OB_SUCCESS;
      if (STAGE_INIT == which_stage)
      {
        TBSYS_LOG(ERROR, "can't understand which stage,which_stage = %d",
                  which_stage);
        ret = OB_ERROR;
      }

      if (ret == OB_SUCCESS && LOCAL_INDEX_STAGE == which_stage)
      {
        // get original table's tablet on this cs,and sort them
        const ObSchemaManagerV2* schema_mgr = schema_mgr_->get_schema(process_idx_tid_);
        const ObTableSchema* index_schema = NULL;
        // process_idx_tid is the index table's id && ori_tid is the original table's id
        // by original,i mean the table which index table comes from
        uint64_t ori_tid = OB_INVALID_ID;
        ObVector <ObTablet*> tablet_list;
        if (NULL == schema_mgr)
        {
          TBSYS_LOG(ERROR, "failed in take schema!");
          ret = OB_SCHEMA_ERROR;
        }
        else if (NULL == (index_schema = schema_mgr->get_table_schema(process_idx_tid_)))
        {
          TBSYS_LOG(ERROR, "failed in find schema[%ld]", process_idx_tid_);
          ret = OB_SCHEMA_ERROR;
        }
        ori_tid = index_schema->get_original_table_id();
        //找到本机上存储ori_tid对应的table的tablet的数据放到vector中
        if (OB_SUCCESS != (ret = tablet_manager_->get_serving_tablet_image().get_serving_image().acquire_tablets_by_table_id(ori_tid, tablet_list)))
        {
          TBSYS_LOG(WARN, "get tablets error!");
        }
        if (OB_SUCCESS == ret)
        {
          bool need_index = true;//需要构建索引
          for (ObVector <ObTablet*>::iterator it = tablet_list.begin();
               it != tablet_list.end(); ++it)
          {
            bool is_handled = false;//tablet是否被处理过
            if (OB_SUCCESS == (ret = is_tablet_handle(*it, is_handled)))
            {
              if (!is_handled)
              {
                if (OB_SUCCESS != (ret =  is_tablet_need_build_static_index(*it, need_index)))
                {
                  TBSYS_LOG(ERROR, "error in is_need_static_index_tablet,ret[%d]", ret);
                }
                else if (need_index)
                {
                  TabletRecord record;
                  record.tablet_ = *it;
                  //tablet_array_中存放本机上那些还没有挂第二块sstable的tablet
                  //也就是说还没有进行排序
                  tablet_array_.push_back(record);
                }
                else if (OB_SUCCESS != (ret = tablet_manager_->get_serving_tablet_image().release_tablet(*it)))
                {
                  TBSYS_LOG(WARN, "release tablet array failed, ret = [%d]",ret);
                }
              }
              else if (OB_SUCCESS != (ret = tablet_manager_->get_serving_tablet_image().release_tablet(*it)))
              {
                TBSYS_LOG(WARN, "release tablet array failed, ret = [%d]", ret);
              }
            }
          }
        }
        if (NULL != schema_mgr)
        {
          if (OB_SUCCESS != (ret = schema_mgr_->release_schema(schema_mgr)))
          {
            TBSYS_LOG(ERROR, "failed to release schema ret[%d]", ret);
          }
          //add wenghaixing [realse schema while function end!] 20160128
          else
          {
            schema_mgr = NULL;
          }
          //add e
        }
        if(OB_SUCCESS == ret)
        {
          TBSYS_LOG(INFO,"tablet array ready");
        }
      }

      if (ret == OB_SUCCESS && GLOBAL_INDEX_STAGE == which_stage)
      {
        // receive range info from rs, find all the tablet info including in other cs in this range,put them into range_array_
        // in fact,in the get_ready_for_cons_idx stage,we simply put all the tablet info of this index table into
        // multcs_range_hash,and put the the info of the tablet in this cs into range_hash
        // and then we construct range_arry_
        // 做两遍fetch_tablet_info,分别传原表的id,和索引表的id
        TBSYS_LOG(INFO,">>>begin global index construction stage.");
        const int64_t timeout = 2000000;
        ObGeneralRpcStub rpc_stub = THE_CHUNK_SERVER.get_rpc_stub();
        ObScanner scanner;
        ObRowkey start_key;
        range_hash_.clear();
        bool need_other_cs = false;
        bool need_release_all_tablet = false;
        start_key.set_min_row();
        do
        {
          ret = rpc_stub.fetch_tablet_location(timeout,
                                               THE_CHUNK_SERVER.get_root_server(),
                                               0,
                                               process_idx_tid_,
                                               start_key,
                                               scanner);
          if(ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN,"fetch location failed,no tablet in root_table,ret=%d",ret);
            need_release_all_tablet = true;
            break;
          }
          else
          {
            ret = parse_location_from_scanner(scanner, start_key, process_idx_tid_, need_other_cs);
          }
          if (ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "parse tablet info from ObScanner failed, ret=[%d]", ret);
            need_release_all_tablet = true;
            break;
          }
          else if (ObRowkey::MAX_ROWKEY == start_key)
          {
            TBSYS_LOG(INFO, "get all tablets info from rootserver success");
            break;
          }
          else
          {
            TBSYS_LOG(DEBUG, "need more request to get next tablet info");
            scanner.reset();
          }
        }while(true);

        start_key.set_min_row();
        need_other_cs = true;
        scanner.reset();
        if (OB_SUCCESS != (ret = data_multcs_range_hash_.clear()))
        {
          TBSYS_LOG(WARN,"data multcs range hash clear error!ret[%d]",ret);
        }
        const ObSchemaManagerV2* schema = NULL;
        const ObTableSchema* index_schema = NULL;
        uint64_t ori_tid = OB_INVALID_ID;
        if (NULL == (schema = schema_mgr_->get_schema(process_idx_tid_)))
        {
          TBSYS_LOG(WARN,"get schema manager failed.");
        }
        else if(NULL == (index_schema = schema->get_table_schema(process_idx_tid_)))
        {
          TBSYS_LOG(WARN,"get schema manager failed.");
        }
        else if (OB_INVALID_ID == (ori_tid = index_schema->get_original_table_id()))
        {
          TBSYS_LOG(WARN,"get original table failed.");
        }
        do
        {
          ret = rpc_stub.fetch_tablet_location(timeout,
                                               THE_CHUNK_SERVER.get_root_server(),
                                               0,
                                               ori_tid,
                                               start_key,
                                               scanner);
          if(ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN,"fetch location failed,no tablet in root_table,ret=%d",ret);
            need_release_all_tablet = true;
            break;
          }
          else
          {
            ret = parse_location_from_scanner(scanner, start_key, ori_tid, need_other_cs);
          }
          if (ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "parse tablet info from ObScanner failed, ret=[%d]", ret);
            need_release_all_tablet = true;
            break;
          }
          else if (ObRowkey::MAX_ROWKEY == start_key)
          {
            TBSYS_LOG(INFO, "get all tablets info from rootserver success");
            break;
          }
          else
          {
            TBSYS_LOG(DEBUG, "need more request to get next tablet info");
            scanner.reset();
          }
        }while(true);

        //add wenghaixing [realse schema while function end!] 20160128
        if(NULL != schema)
        {
          if (OB_SUCCESS != (ret = schema_mgr_->release_schema(schema)))
          {
            TBSYS_LOG(ERROR, "failed to release schema ret[%d]", ret);
          }
          else
          {
            schema = NULL;
          }
        }
        //add e

        hash::ObHashMap<ObNewRange, ObTabletLocationList,hash::NoPthreadDefendMode>::const_iterator iter = range_hash_.begin();
        RangeRecord record;
        for (;iter != range_hash_.end(); ++iter)
        {
          if(0 == range_array_.count())
          {
            record.range_ = iter->first;
            range_array_.push_back(record);
          }
          else
          {
            bool in_array = false;
            for(int64_t i = 0;i < range_array_.count();i++)
            {
              if(iter->first == range_array_.at(i).range_)
              {
                in_array = true;
                break;
              }
            }
            if(!in_array)
            {
              record.range_ = iter->first;
              range_array_.push_back(record);
            }
          }
        }
      }
      return ret;
    }
    // add e

    int ObIndexHandlePool::set_config_param()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS
          != (ret = data_multcs_range_hash_.create(hash::cal_next_prime(512))))
      {
        TBSYS_LOG(ERROR, "init data range hash error,ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = range_hash_.create(hash::cal_next_prime(512))))
      {
        TBSYS_LOG(ERROR, "init index range hash error,ret=%d", ret);
      }
      return ret;
    }

    int ObIndexHandlePool::parse_location_from_scanner(
        ObScanner &scanner,
        ObRowkey &row_key,
        uint64_t table_id,
        bool need_other_cs)
    {
      int ret = OB_SUCCESS;
      ObRowkey start_key;
      start_key = ObRowkey::MIN_ROWKEY;
      ObRowkey end_key;
      ObServer server;
      ObCellInfo * cell = NULL;
      bool row_change = false;
      ObTabletLocationList list;
      //CharArena allocator;
      ObScannerIterator iter = scanner.begin();
      ObNewRange range;
      ++iter;

      scanner.dump_all(TBSYS_LOG_LEVEL_ERROR); //test

      while ((iter != scanner.end())
             && (OB_SUCCESS == (ret = iter.get_cell(&cell, &row_change)))
             && !row_change)
      {
        if (NULL == cell)
        {
          ret = OB_INNER_STAT_ERROR;
          break;
        }
        cell->row_key_.deep_copy(start_key, allocator_);
        ++iter;
      }

      if (ret == OB_SUCCESS)
      {
        int64_t ip = 0;
        int64_t port = 0;
        // next cell
        for (++iter; iter != scanner.end(); ++iter)
        {
          ret = iter.get_cell(&cell, &row_change);
          if (ret != OB_SUCCESS)
          {
            TBSYS_LOG(ERROR, "get cell from scanner iterator failed:ret[%d]",ret);
            break;
          }
          else if (row_change) // && (iter != last_iter))
          {
            construct_tablet_item(table_id, start_key, end_key, range, list);
            list.print_info();
            if (need_other_cs)
            {
              //把所有table_id对应的所有的tablet的，包括所有副本的信息都放到data_multcs_range_hash里面去
              if (-1 == data_multcs_range_hash_.set(list.get_tablet_range(), list,1))
              {
                TBSYS_LOG(ERROR, "insert data_multcs_range_hash_ error!");
              }
            }
            //need other cs 设置为true，则不做下面的range_hash_的填充
            else if (list[0].server_.chunkserver_.get_ipv4() == THE_CHUNK_SERVER.get_self().get_ipv4())
            {
              //如果第一副本(list[0])是自己的话，那么把这个tablet(包括其他副本)放入range_hash里面去
              if(-1 == range_hash_.set(list.get_tablet_range(), list, 1))
              {
                TBSYS_LOG(ERROR,"insert range_hash_ error!");
              }
            }
            list.clear();
            start_key = end_key;
          }
          else
          {
            cell->row_key_.deep_copy(end_key, allocator_);
            if ((cell->column_name_.compare("1_port") == 0)
                || (cell->column_name_.compare("2_port") == 0)
                || (cell->column_name_.compare("3_port") == 0))
            {
              ret = cell->value_.get_int(port);
            }
            else if ((cell->column_name_.compare("1_ipv4") == 0)
                     || (cell->column_name_.compare("2_ipv4") == 0)
                     || (cell->column_name_.compare("3_ipv4") == 0))
            {
              ret = cell->value_.get_int(ip);
              if (OB_SUCCESS == ret)
              {
                if (port == 0)
                {
                  TBSYS_LOG(WARN, "check port failed:ip[%ld], port[%ld]", ip,
                            port);
                }
                server.set_ipv4_addr(static_cast <int32_t>(ip),
                                     static_cast <int32_t>(port));
                ObTabletLocation addr(0, server);
                if (OB_SUCCESS != (ret = list.add(addr)))
                {
                  TBSYS_LOG(ERROR,"add addr failed:ip[%ld], port[%ld], ret[%d]", ip, port, ret);
                  break;
                }
                else
                {
                  TBSYS_LOG(DEBUG, "add addr succ:ip[%ld], port[%ld], server:%s", ip, port, to_cstring(server));
                }
                ip = port = 0;
              }
            }
            if (ret != OB_SUCCESS)
            {
              TBSYS_LOG(ERROR, "check get value failed:ret[%d]", ret);
              break;
            }
          }
        }
        // for the last row
        TBSYS_LOG(DEBUG, "get a new tablet start_key[%s], end_key[%s]", to_cstring(start_key), to_cstring(end_key));
        if ((OB_SUCCESS == ret) && (start_key != end_key))
        {
          construct_tablet_item(table_id, start_key, end_key, range, list);
          list.print_info();
          if (need_other_cs)
          {
            if (-1 == data_multcs_range_hash_.set(list.get_tablet_range(), list,1))
            {
              TBSYS_LOG(ERROR, "insert data_multcs_range hash error!");
              ret = OB_ERROR;
            }
          }
          else if (list[0].server_.chunkserver_.get_ipv4() == THE_CHUNK_SERVER.get_self().get_ipv4())
          {
            if(-1 == range_hash_.set(list.get_tablet_range(),list,1))
            {
              TBSYS_LOG(ERROR,"insert range hash error!");
              ret = OB_ERROR;
            }
          }
        }
      }
      else
      {
        TBSYS_LOG(ERROR, "check get first row cell failed:ret[%d]", ret);
      }
      if (OB_SUCCESS == ret)
      {
        row_key = end_key;
      }

      return ret;
    }

    int ObIndexHandlePool::is_tablet_need_build_static_index(
        ObTablet *tablet,
        bool &is_need_index)
    {
      int ret = OB_SUCCESS;
      is_need_index = true;
      if (NULL == tablet)
      {
        TBSYS_LOG(ERROR, "null pointer for tablet");
        ret = OB_ERROR;
      }
      else if (is_need_index
          && tablet->get_sstable_id_list().count() == ObTablet::MAX_SSTABLE_PER_TABLET)
      {
        is_need_index = false;
      }
      //TBSYS_LOG(INFO,"TEST::LONGFEI>>>is_need_index[%s]",is_need_index ? "true" : "false");
      return ret;
    }

    int ObIndexHandlePool::finish_phase1(bool &reported)
    {
      int ret = OB_SUCCESS;
      if (0 == static_index_report_infolist->tablet_list.get_array_index())
      {

      }
      else if (OB_SUCCESS != (ret = ObIndexReporter::send_local_index_info(tablet_manager_, static_index_report_infolist)))
      {
        TBSYS_LOG(ERROR, "report index info failed, ret[%d]", ret);
        static_index_report_infolist->reset();
        for (int i = 0; i < 10; i++)
        {
          //完成发送local index info失败时候内存释放函数
          local_handler_[i]->get_allocator()->reuse();   //发送失败,释放内存
          local_handler_[i]->get_index_reporter()->reset_report_info();
        }
        TBSYS_LOG(WARN,"partional tablet histogram info report failed, memory reuse");
      }
      else
      {
        reported = true;
        static_index_report_infolist->reset();
        for (int i = 0; i < 10; i++)
        {
          local_handler_[i]->get_allocator()->reuse();   //发送成功,释放内存
          local_handler_[i]->get_index_reporter()->reset_report_info();
        }
        TBSYS_LOG(INFO,"partional tablet histogram info report success, memory reuse");
      }

      return ret;
    }

    int ObIndexHandlePool::finish_phase2(bool & total_reported)
    {
      int ret = OB_SUCCESS;
      pthread_mutex_lock(&stage_mutex_);
      if (ROUND_FALSE >= round_end_)
      {
        if (OB_RESPONSE_TIME_OUT ==
            (ret =ObIndexReporter::send_index_info(tablet_manager_, process_idx_tid_)))
        {
          TBSYS_LOG(WARN, "send index tablets info failed = %d", ret);
        }
        else
        {
          total_reported = true;
          tablet_manager_->report_capacity_info();
        }
      }
      if (OB_SUCCESS == ret && ROUND_FALSE >= round_end_)
      {
        ret =
            tablet_manager_->get_serving_tablet_image().get_serving_image().delete_local_index_sstable();
        //TBSYS_LOG(ERROR,"test::whx delete tablet!");
      }
      if (OB_SUCCESS == ret && ROUND_FALSE >= round_end_)
      {
        round_end_ = ROUND_TRUE;
        round_start_ = ROUND_FALSE;

      }
      pthread_mutex_unlock(&stage_mutex_);
      return ret;
    }

    int ObIndexHandlePool::add_tablet_info(ObTabletReportInfo *tablet)
    {
      int ret = OB_SUCCESS;
      ObNewRange copy_range;
      ObTabletReportInfo copy;
      copy = *tablet;

      if (NULL == tablet)
      {
        ret = OB_ERROR;
        TBSYS_LOG(ERROR, "tablet is null");
      }
      else if (OB_SUCCESS
               != (ret = deep_copy_range(report_allocator_,
                                         tablet->tablet_info_.range_, copy_range)))
      {
        TBSYS_LOG(ERROR, "copy range failed.");
      }
      else
      {
        copy.tablet_info_.range_ = copy_range;
      }
      if (OB_SUCCESS != ret || NULL == tablet)
      {
        TBSYS_LOG(ERROR, "failed to add tablet report info for sstable local index,null pointer");
        ret = OB_ERROR;
      }
      else
      {
        ret = report_info_.add_tablet(copy);
      }
      return ret;
    }

    int ObIndexHandlePool::get_tablets_ranges(
        TabletRecord *&tablet,
        RangeRecord *&range,
        int &err)
    {
      int ret = OB_SUCCESS;
      bool reported = false;
      bool total_reported = false;
      err = OB_GET_NOTHING;
      UNUSED(range);
      //int err = OB_ERROR;
      tablet = NULL;
      range = NULL;
      if (tablet_array_.count() > 0 && tablet_index_ < tablet_array_.count())
      {
        TBSYS_LOG(INFO,"get tablet from local list,tablet_index_:%ld,tablets_num_:%ld",tablet_index_, tablet_array_.count());
        tablet = &(tablet_array_.at(tablet_index_++));
        err = OB_GET_TABLETS;
      }
      // 多线程已经处理完tablet_array_.count()个tablet
      else if (tablets_have_got_ == tablet_array_.count() && tablets_have_got_ != 0)
      {
        if (check_if_tablet_range_failed(true, tablet, range)) //检查是否有失败的任务，有的话，赋值，继续完成
        {
          err = OB_GET_TABLETS;
        }
        else if (is_local_stage_need_end() && OB_SUCCESS == (ret = finish_phase1(reported)))
        {
          if (reported)
          {
            TBSYS_LOG(INFO, "report local index tablet success!");
          }
        }
      }
      if (OB_GET_NOTHING == err)
      {
        if (0 < range_array_.count() && range_index_ < range_array_.count())
        {
          range = &range_array_.at(range_index_++);
          err = OB_GET_RANGES;
        }
        else if (range_have_got_ == range_array_.count() && range_have_got_ != 0)
        {
          if (check_if_tablet_range_failed(false, tablet, range)) //检查是否有失败的任务，有的话，赋值，继续完成
          {
            err = OB_GET_RANGES;
          }
          else if (is_global_stage_need_end() && OB_SUCCESS ==
                   (ret = finish_phase2(total_reported)))
          {
            if (total_reported)
            {
              TBSYS_LOG(INFO, "report total index tablet success!");
            }
          }
        }
      }
      TBSYS_LOG(DEBUG,">>>ret[%d],err[%d]",ret,err);
      return ret;
    }

    int ObIndexHandlePool::release_tablet_array()
    {
      int ret = OB_SUCCESS;
      ObTablet *tablet = NULL;
      for (int64_t i = 0; i < tablet_array_.count(); i++)
      {
        tablet = tablet_array_.at(i).tablet_;
        if (OB_SUCCESS != (ret = tablet_manager_->get_serving_tablet_image().release_tablet(tablet)))
        {
          TBSYS_LOG(WARN, "release tablet array failed, ret = [%d]", ret);
          break;
        }
      }
      if (OB_SUCCESS == ret)
      {
        round_end_ = TABLET_RELEASE;
      }
      return ret;
    }

    void ObIndexHandlePool::inc_get_tablet_count()
    {
      if (tablet_array_.count() > tablets_have_got_)
      {
        tablets_have_got_++;
      }
      else
      {
        TBSYS_LOG(INFO,"all tablet has been consume, check if has failed record");
      }
    }

    void ObIndexHandlePool::inc_get_range_count()
    {
      if (range_array_.count() > range_have_got_)
      {
        range_have_got_++;
      }
      else
      {
        TBSYS_LOG(INFO,"all range has been consume , check if has failed record");
      }
    }

    //add longfei [cons static index] 151220:b
    int ObIndexHandlePool::get_global_index_handler(
        const int64_t thread_no,
        ObGlobalIndexHandler *&global_handler)
    {
      int ret = OB_SUCCESS;
      global_handler = NULL;
      ObGlobalIndexHandler** global_handlers = NULL;
      global_handlers = global_handler_;

      if (thread_no >= MAX_WORK_THREAD)
      {
        TBSYS_LOG(ERROR, "thread_no=%ld >= max_work_thread_num=%ld", thread_no,
                  MAX_WORK_THREAD);
        ret = OB_SIZE_OVERFLOW;
      }
      else if (NULL == global_handlers)
      {
        TBSYS_LOG(ERROR, "thread_no=%ld handlers is NULL", thread_no);
        ret = OB_SIZE_OVERFLOW;
      }
      else if (NULL == (global_handler = global_handlers[thread_no]))
      {
        TBSYS_LOG(ERROR, "thread_no=%ld handlers is NULL", thread_no);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        TBSYS_LOG(DEBUG,"GLOBAL_HANDLER[%p]",global_handler);
      }
      return ret;
    }

    int ObIndexHandlePool::get_local_index_handler(
        const int64_t thread_no,
        ObLocalIndexHandler *&local_handler)
    {
      int ret = OB_SUCCESS;
      local_handler = NULL;
      ObLocalIndexHandler** local_handlers = NULL;
      local_handlers = local_handler_;

      if (thread_no >= MAX_WORK_THREAD)
      {
        TBSYS_LOG(ERROR, "thread_no=%ld >= max_work_thread_num=%ld", thread_no,
                  MAX_WORK_THREAD);
        ret = OB_SIZE_OVERFLOW;
      }
      else if (NULL == local_handlers)
      {
        TBSYS_LOG(ERROR, "thread_no=%ld handlers is NULL", thread_no);
        ret = OB_SIZE_OVERFLOW;
      }
      else if (NULL == (local_handler = local_handlers[thread_no]))
      {
        TBSYS_LOG(ERROR, "thread_no=%ld handlers is NULL", thread_no);
        ret = OB_INVALID_ARGUMENT;
      }

      return ret;
    }
    //add e

    void ObIndexHandlePool::construct_index(const int64_t thread_no)
    {
      int ret = OB_SUCCESS;
      int err = OB_SUCCESS;
      const int64_t sleep_interval = 5000000;
      TabletRecord *tablet = NULL;
      RangeRecord *range = NULL;
      ObGlobalIndexHandler * global_handler = NULL; // add longfei [cons static index] 151205:e
      ObLocalIndexHandler * local_handler = NULL; // add longfei [cons static index] 151220:e

      while (true)
      {
        if (!inited_)
        {
          break;
        }
        pthread_mutex_lock(&mutex_);
        TBSYS_LOG(INFO,">>>try to get a tablet/range to build index");
        ret = get_tablets_ranges(tablet, range, err);
        while (true)
        {
          if (OB_GET_NOTHING == err && OB_SUCCESS != ret)
          {
            pthread_mutex_unlock(&mutex_);
            usleep(sleep_interval);
            // retry to get tablet until got one or got nothing.
            pthread_mutex_lock(&mutex_);
          }
          else if (OB_GET_NOTHING == err)
          {
            --active_thread_num_;
            TBSYS_LOG(INFO,"there is no tablet need build static index,sleep wait for new index process.");
            pthread_cond_wait(&cond_, &mutex_);//在这儿阻塞handler，等待start_round(...)准备好信息之后发送广播。
            TBSYS_LOG(INFO, "awake by signal,active_thread_num_=:%ld",active_thread_num_);
            ++active_thread_num_;
          }
          else
          {
            break;
          }
          ret = get_tablets_ranges(tablet, range, err);
        }
        pthread_mutex_unlock(&mutex_);

        if(OB_GET_TABLETS == err)//如果是拿到的tablet，则就是出于局部索引的构建阶段
        {
          if(OB_SUCCESS != (ret = get_local_index_handler(thread_no, local_handler)))
          {
            TBSYS_LOG(ERROR, "get index builder error, ret[%d]",ret);
          }
          else
          {
            int err_local = OB_SUCCESS;
            TBSYS_LOG(INFO,"now local index handler begin to work!");
            if (OB_SUCCESS != (err_local = local_handler->set_tablet(tablet->tablet_)))
            {
              TBSYS_LOG(WARN,"failed to do set_tablet,err[%d]",err_local);
            }
            else if (OB_SUCCESS != (err_local = local_handler->set_sample_rate(hist_width_)))
            {
              TBSYS_LOG(WARN,"failed to do set_sample_rate,err[%d]",err_local);
            }
            if(OB_SUCCESS != (err_local = local_handler->start()))
            {
              TBSYS_LOG(WARN, "build partitional index failed,tablet[%s],fail count[%d],if_process[%d],err[%d]",to_cstring(tablet->tablet_->get_range()), (int)tablet->fail_count_,(int)tablet->if_process_, err_local);
              tablet->if_process_ = 0;
              tablet->fail_count_++;
              if(tablet->fail_count_ > MAX_FAILE_COUNT && !tablet->work_send_)
              {
                //todo 如果失败超过一定次数的处理方法
                //bool is_local_index = true;
                ObNewRange no_use_range;
                if(OB_SUCCESS != (err_local = retry_failed_work(LOCAL_INDEX_SST_BUILD_FAILED, tablet->tablet_, no_use_range)))
                {
                  ret = OB_INDEX_BUILD_FAILED;
                  tablet->if_process_ = 1;
                  TBSYS_LOG(WARN, "whipping wok failed! err = [%d]", err_local);
                }
                else
                {
                  tablet->work_send_ = 1;
                  tablet->fail_count_ = 0;
                  tablet->if_process_ = -1;
                }
                TBSYS_LOG(WARN,"tablet failed too much");
              }
            }
            else
            {
              if(OB_SUCCESS != (ret = local_handler->get_index_reporter()->get_tablet_histogram_report_info_list()->add_tablet(*(local_handler->get_index_reporter()->get_tablet_histogram_report_info()))))//一个tablet结束将tabletreport信息加入汇报总的数组
              {
                TBSYS_LOG(WARN,"add histogram report info failed = %d",ret);
              }
              tablet->fail_count_ = 0;
              tablet->if_process_ = -1;
            }
          }
#if 0
          if (tablet_manager_->get_serving_tablet_image().release_tablet(tablet) != OB_SUCCESS)
          {
            TBSYS_LOG(WARN,"release tablet failed");
          }
#endif
          pthread_mutex_lock(&tablet_range_mutex_);
          inc_get_tablet_count();
          pthread_mutex_unlock(&tablet_range_mutex_);
          if ( tablet_manager_->is_stoped() )
          {
            TBSYS_LOG(WARN,"stop in index");
            ret = OB_ERROR;
          }
        }
        else if (OB_GET_RANGES == err)
        {
          // 如果是得到一个range，则处于全局索引构建的阶段
          if (OB_SUCCESS != (ret = get_global_index_handler(thread_no, global_handler)))
          {
            TBSYS_LOG(ERROR, "get index handler error, ret[%d]", ret);
          }
          else
          {
            int err_global = OB_SUCCESS;
            //add longfei [cons static index] 151221:b
            if (OB_SUCCESS != (err_global = global_handler->set_handle_range(&range->range_)))
            {
              TBSYS_LOG(WARN,"failed to do set_handle_range,err[%d]",err_global);
            }
            //add e
            else if (OB_SUCCESS != (err_global = global_handler->start()))
            {
              TBSYS_LOG(WARN, "build global index failed,err[%d]", err_global);
              if (OB_TABLET_HAS_NO_LOCAL_SSTABLE == err_global)
              {
                ObNewRange fake_range;
                if (OB_SUCCESS != global_handler->get_failed_fake_range(fake_range))
                {
                  ret = OB_INDEX_BUILD_FAILED;
                }
                else
                {
                  if (OB_SUCCESS
                      != (err_global = retry_failed_work(LOCAL_INDEX_SST_NOT_FOUND,
                                                         NULL, fake_range)))
                  {
                    ret = OB_INDEX_BUILD_FAILED;
                  }
                  else
                  {
                    //range->if_process_ = 0;
                    range->fail_count_++;
                    range->if_process_ = 0;
                  }
                }
                //we re-construct this block, so that we can control it until sstable build success
              }
              else
              {
                range->if_process_ = 0;
                range->fail_count_++;
                if (range->fail_count_ > MAX_FAILE_COUNT && !range->work_send_)
                {
                  //todo 如果失败超过一定次数的处理方法
                  TBSYS_LOG(WARN, "range failed too much");
                  if (OB_SUCCESS
                      != (err_global = retry_failed_work(GLOBAL_INDEX_BUILD_FAILED,
                                                         NULL, range->range_)))
                  {
                    ret = OB_INDEX_BUILD_FAILED;
                    range->if_process_ = 1;
                  }
                  else
                  {
                    range->fail_count_ = 0;
                    range->if_process_ = -1;
                    range->work_send_ = 1;
                  }
                }
              }
            }
            else
            {
              range->fail_count_ = 0;
              range->if_process_ = -1;
            }
          }
          pthread_mutex_lock(&tablet_range_mutex_);
          inc_get_range_count();
          pthread_mutex_unlock(&tablet_range_mutex_);
          if (tablet_manager_->is_stoped())
          {
            TBSYS_LOG(WARN, "stop in index");
            ret = OB_ERROR;
          }
        }
      }

    }

    void ObIndexHandlePool::run(tbsys::CThread *thread, void *arg)
    {
      UNUSED(thread);
      int64_t thread_no = reinterpret_cast <int64_t>(arg);
      construct_index(thread_no);         //索引构建阶段
    }

    bool ObIndexHandlePool::can_launch_next_round()
    {
      bool ret = false;
      // int64_t now = tbsys::CTimeUtil::getTime();
      // TBSYS_LOG(DEBUG,"inited_[%d],active_thread_num_[%ld]",(int)inited_,active_thread_num_);
      // 删除判断is_work_stoped()
      if (inited_ /*&& is_work_stoped()*/
          // && now - total_work_last_end_time_ > THE_CHUNK_SERVER.get_config().min_merge_interval
          //&& THE_CHUNK_SERVER.get_tablet_manager().get_bypass_sstable_loader().is_loader_stoped()
          )
      {
        ret = true;
      }
      return ret;
    }

    void ObIndexHandlePool::construct_tablet_item(
        const uint64_t table_id,
        const ObRowkey & start_key,
        const ObRowkey & end_key,
        ObNewRange & range,
        ObTabletLocationList & list)
    {
      range.table_id_ = table_id;
      range.border_flag_.unset_inclusive_start();
      range.border_flag_.set_inclusive_end();
      range.start_key_ = start_key;
      range.end_key_ = end_key;
      if (range.end_key_.is_max_row())
      {
        range.border_flag_.unset_inclusive_end();
      }
      list.set_timestamp(tbsys::CTimeUtil::getTime());
      list.set_tablet_range(range);
      // list.sort(addr);
      // double check add all range->locationlist to cache
      if (range.start_key_ >= range.end_key_)
      {
        TBSYS_LOG(WARN, "check range invalid:start[%s], end[%s]", to_cstring(range.start_key_), to_cstring(range.end_key_));
      }
      else
      {
        TBSYS_LOG(DEBUG, "got a tablet:%s, with location list:%ld",to_cstring(range), list.size());
      }
    }

    bool ObIndexHandlePool::check_if_tablet_range_failed(
        bool is_local_index,
        TabletRecord *&tablet,
        RangeRecord *&range)
    {
      bool ret = false;
      tablet = NULL;
      range = NULL;
      if (is_local_index)
      {
        for (int64_t i = 0; i < tablet_array_.count(); i++)
        {
          if (tablet_array_.at(i).fail_count_ > 0
              && 0 == tablet_array_.at(i).if_process_)
          {
            tablet_array_.at(i).if_process_ = 1;
            tablet = &(tablet_array_.at(i));
            ret = true;
            break;
          }
        }
      }
      else
      {
        for (int64_t i = 0; i < range_array_.count(); i++)
        {
          if (range_array_.at(i).fail_count_ > 0
              && 0 == range_array_.at(i).if_process_)
          {
            range = &range_array_.at(i);
            range_array_.at(i).if_process_ = 1;
            ret = true;
            break;
          }
        }
      }
      return ret;
    }

    int ObIndexHandlePool::retry_failed_work(
        ErrNo level,
        const ObTablet *tablet,
        ObNewRange range)
    {
      int ret = OB_SUCCESS;
      ObTabletLocationList list;
      bool in_list = false;
      int64_t index = 0;
      BlackList black_list;
      ObServer next_server;
      ObNewRange wok_range;
      hash::ObHashMap <ObNewRange, ObTabletLocationList, hash::NoPthreadDefendMode> * range_info = NULL;
      switch (level)
      {
        case LOCAL_INDEX_SST_BUILD_FAILED:
          if (NULL == tablet)
          {
            ret = OB_INVALID_ARGUMENT;
          }
          else
          {
            wok_range = tablet->get_range();
            range_info = &range_hash_;
          }
          break;
        case GLOBAL_INDEX_BUILD_FAILED:
          wok_range = range;
          range_info = &range_hash_;
          break;
        case LOCAL_INDEX_SST_NOT_FOUND:
          wok_range = range;
          range_info = &data_multcs_range_hash_;
          break;
        default:
          ret = OB_INVALID_ARGUMENT;
      }
      if (OB_SUCCESS == ret)
      {
        if (hash::HASH_EXIST != range_info->get(wok_range, list))
        {
          ret = OB_INVALID_ARGUMENT;
          TBSYS_LOG(ERROR,"range hash cannot find range,wok_range[%s],hash_size[%ld]",
                    to_cstring(wok_range), data_multcs_range_hash_.size());
        }
        else
        {
          if (OB_SUCCESS != (ret = black_list_array_.check_range_in_list(
                               wok_range,
                               in_list,
                               index)))
          {
            //never happen
          }
          else if (in_list)
          {
            black_list = black_list_array_.get_black_list(index, ret);
            if (OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "get black list failed, ret = [%d]", ret);
            }
            else if (0 == black_list.get_work_send())
            {
              black_list.set_server_unserved(THE_CHUNK_SERVER.get_self());
              if(OB_SUCCESS != (ret = black_list.next_replic_server(next_server)))
              {
                TBSYS_LOG(WARN, "all replicate failed!!!");
              }
              else if(OB_SUCCESS != (ret = tablet_manager_->retry_failed_work(black_list, next_server)))
              {
                black_list.set_server_unserved(next_server);
                black_list.set_work_send(0);
                TBSYS_LOG(WARN, "whipping wok failed,ret [%d]", ret);
              }
              else
              {
                black_list.set_work_send();
              }
            }
          }
          else if(!in_list)
          {
            if (OB_SUCCESS != (ret = black_list.write_list(wok_range, list)))
            {
              TBSYS_LOG(WARN, "write wok range in black list failed ret[%d]",ret);
            }
            else if (0 == black_list.get_work_send())
            {
              black_list.set_server_unserved(THE_CHUNK_SERVER.get_self());
              if(OB_SUCCESS != (ret = black_list_array_.push(black_list)))
              {
                TBSYS_LOG(WARN,"pus black list in array failed ret [%d] ",ret);
              }
              else if(OB_SUCCESS != (ret = black_list.next_replic_server(next_server)))
              {
                TBSYS_LOG(WARN, "all replicate failed!!!");
              }
              else if(OB_SUCCESS != (ret = tablet_manager_->retry_failed_work(black_list, next_server)))
              {
                black_list.set_work_send(0);
                black_list.set_server_unserved(next_server);
                TBSYS_LOG(WARN, "whipping wok failed,ret [%d]", ret);
              }
              else
              {
                black_list.set_work_send();
              }
            }
          }
        }

      }

      return ret;
    }

    bool ObIndexHandlePool::is_local_stage_need_end()
    {
      bool ret = true;
      for (int64_t i = 0; i < tablet_array_.count(); i++)
      {
        if (-1 != tablet_array_.at(i).if_process_)
        {
          ret = false;
          break;
        }
      }
      return ret;
    }

    bool ObIndexHandlePool::is_global_stage_need_end()
    {
      bool ret = true;
      for (int64_t i = 0; i < range_array_.count(); i++)
      {
        if (-1 != range_array_.at(i).if_process_)
        {
          ret = false;
          break;
        }
      }
      return ret;
    }

    int ObIndexHandlePool::push_work(BlackList &list)
    {
      return black_list_array_.push(list);
    }

    void ObIndexHandlePool::reset()
    {
      tablet_array_.clear();
      range_array_.clear();
      black_list_array_.reset();
      range_have_got_ = 0;
      tablets_have_got_ = 0;
      tablet_index_ = 0;
      range_index_ = 0;
      hist_width_ = 0;
      process_idx_tid_ = OB_INVALID_ID;
      total_work_last_end_time_ = 0;
      total_work_start_time_ = 0;
      allocator_.reuse();
    }

    int ObIndexHandlePool::is_tablet_handle(ObTablet *tablet, bool &is_handle)
    {
      int ret = OB_SUCCESS;
      is_handle = false;
      if (NULL == tablet)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        for (int64_t i = 0; i < tablet_array_.count(); i++)
        {
          if (NULL == tablet_array_.at(i).tablet_)
          {
            ret = OB_INVALID_ARGUMENT;
            break;
          }
          else
          {
            if (tablet_array_.at(i).tablet_->get_range() == tablet->get_range())
            {
              is_handle = true;
              break;
            }
          }
        }
      }
      return ret;
    }

    int ObIndexHandlePool::try_stop_mission(uint64_t index_tid)
    {
      int ret = OB_SUCCESS;
      if (0 == total_work_last_end_time_ && index_tid != process_idx_tid_)
      {
        pthread_mutex_lock(&mutex_);
        if (active_thread_num_ != 0
            || (active_thread_num_ == 0 && round_end_ != TABLET_RELEASE))
        {
          TBSYS_LOG(WARN,"try stop index build work current idx_tid:[%ld] new idx_tid:[%ld]", process_idx_tid_, index_tid);
          if (tablet_index_ != tablet_array_.count())
          {
            tablet_index_ = tablet_array_.count();
          }
          if (tablets_have_got_ != tablet_array_.count())
          {
            tablets_have_got_ = tablet_array_.count();
          }
          if (range_index_ != range_array_.count())
          {
            range_index_ = range_array_.count();
          }
          if (range_have_got_ != range_array_.count())
          {
            range_have_got_ = range_array_.count();
          }
          {
            int64_t time = tbsys::CTimeUtil::getTime();
            total_work_last_end_time_ = time;
          }

          //等待所有线程都停下来
          while(true)
          {
            TBSYS_LOG(INFO,">>>wait here...active_thread_num[%ld]",active_thread_num_);
            if(active_thread_num_ == 0)
              break;
            //睡1s
            usleep(1000000);
          }
          //线程都做完
          if (OB_SUCCESS != (ret = release_tablet_array()))
          {
            TBSYS_LOG(WARN,">>>release failed.round_end[%d]",round_end_);
          }
          reset();
          //add longfei 151231
          //[bugfix:修复一个没有释放sstable导致局部索引失败的bug]
          if(OB_SUCCESS != (ret = tablet_manager_->get_serving_tablet_image().get_serving_image().delete_local_index_sstable()))
          {
            TBSYS_LOG(WARN,"delete local index sstable failed.ret[%d]",ret);
          }
          //add e
        }
        pthread_mutex_unlock(&mutex_);
      }
      return ret;
    }

    bool ObIndexHandlePool::check_if_in_processing(uint64_t index_tid)
    {
      bool res = false;
      if (OB_INVALID_ID != index_tid && index_tid == process_idx_tid_)
      {
        res = true;
      }
      return res;
    }

  }//end chunkserver

} //end oceanbase
