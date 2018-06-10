/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_control_unit.cpp
 * @brief control unit of index construction
 *  icu start when common daily merge begin,and will submit mission while common merge finished.
 *  icu will control main procedure of index construction one by one
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

#include "ob_index_control_unit.h"
#include "ob_root_worker.h"
namespace oceanbase
{
  namespace rootserver
  {
    ObIndexControlUnit::ObIndexControlUnit()
        :finish_index_num_(0),mission_start_time_(-1),start_(false),job_running_(false),root_woker_(NULL),designer_(NULL)
    {
    }

    ObIndexControlUnit::~ObIndexControlUnit()
    {
      if(NULL != designer_)
      {
        OB_DELETE(ObIndexDesigner, ObModIds::OB_STATIC_INDEX, designer_);
      }
      designer_ = NULL;
    }

    void ObIndexControlUnit::init(ObRootWorker *worker)
    {
      root_woker_ = worker;
      service_.init(worker);
      ch_.init(worker);
      ObTabletHistogramManager* manager = NULL;
      if (NULL == (manager = OB_NEW(ObTabletHistogramManager,  ObModIds::OB_STATIC_INDEX)))
      {
        TBSYS_LOG(ERROR, "new ObTabletHistogramManager error");
      }
      else if(NULL == (designer_ = OB_NEW(ObIndexDesigner, ObModIds::OB_STATIC_INDEX, &(root_woker_->get_root_server()), manager)))
      {
        TBSYS_LOG(ERROR, "new ObIndexDesigner error");
      }
    }

    //bool ObIndexControlUnit::is_start(){return start_;}

    void ObIndexControlUnit::start()
    {
      tbsys::CThreadGuard lock(&icu_mutex_);
      if(!start_)
      {
        start_ = true;
        TBSYS_LOG(INFO, "index control unit has been started");
      }
      else
      {
        TBSYS_LOG(INFO, "index control unit has alread been started");
      }
    }

    void ObIndexControlUnit::stop()
    {
      start_ = false;
      job_running_ = false;
      finish_index_num_ = 0;
      job_list_.clear();
      //service.stop();
    }

    void ObIndexControlUnit::set_version(int64_t start_version)
    {
      version_ = start_version;
    }

    int ObIndexControlUnit::schedule()
    {
      int ret = OB_SUCCESS;
      while(true)
      {
        ret = generate_job_list();
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN,"generate job list for icu! sleep and retry!");
          usleep(3000000);
        }
        else
        {
          break;
        }
      }
      if(0 == job_list_.count())
      {
        TBSYS_LOG(INFO, "there is no index to be build");
        stop();
      }
      else
      {
        run_job();
        if(finish_index_num_ == job_list_.count())
        {
          TBSYS_LOG(INFO,"mission complete!All index's static data has been build");
          stop();
        }
        else
        {
          TBSYS_LOG(WARN,"mission complete. There are [%ld] index has not been build",failed_list_.count());
          for(int64_t i = 0; i < failed_list_.count();i++)
          {
            TBSYS_LOG(WARN,"The index[%ld] has not been build ok", failed_list_.at(i));
          }
          stop();
        }
      }
      return 0;
    }

    int ObIndexControlUnit::start_mission()
    {
      int ret = OB_SUCCESS;
      icu_mutex_.lock();
      if(job_running_)
      {
        //TBSYS_LOG(INFO, "control unit running job already!");
        icu_mutex_.unlock();
      }
      else
      {
        job_running_ = true;
        icu_mutex_.unlock();
        TBSYS_LOG(INFO,"start running index job now");
        ret = service_.submit_job(OB_INDEX_JOB);
      }
      return ret;
    }

    int ObIndexControlUnit::generate_job_list()
    {
      int ret = OB_SUCCESS;
      ObArray<uint64_t> jobs;
      if(NULL == root_woker_)
      {
        TBSYS_LOG(WARN, "root worker cannot be null");
        ret = OB_ERR_NULL_POINTER;
      }
      else if(OB_SUCCESS != (ret = root_woker_->get_root_server().get_init_index(version_, &jobs)))
      {
        TBSYS_LOG(WARN, "get index job list failed,ret[%d]", ret);
      }
      else
      {
        uint64_t index_id = OB_INVALID_ID;
        int64_t status = -1;
        int64_t cluster_id = root_woker_->get_root_server().get_config().cluster_id;
        for(int64_t i  = 0; i < jobs.count(); i++)
        {
          if(OB_SUCCESS != (ret = jobs.at(i, index_id)))
          {
            TBSYS_LOG(WARN, "get from ob array error, ret[%d]", ret);
            break;
          }
          else if(OB_SUCCESS != (ret = service_.fetch_index_stat((int64_t)index_id, cluster_id, status)))
          {
            TBSYS_LOG(WARN, "fetch status idx_tid[%ld],cluster_id[%ld] failed", index_id, cluster_id);
            break;
          }
          else if(-1 == status)
          {
            if(OB_SUCCESS != (ret = job_list_.push_back(index_id)))
            {
              TBSYS_LOG(WARN, "index list push back failed,tid [%ld]", index_id);
              break;
            }
          }
        }
      }
      TBSYS_LOG(INFO, "fetch init job list ok,size[%ld], version[%ld]", job_list_.count(), version_);
      return ret;
    }

    int ObIndexControlUnit::handle_histograms(const ObTabletHistogramReportInfoList &add_tablets, const int server_index)
    {
      int ret = OB_SUCCESS;
      int64_t hist_index = OB_INVALID_INDEX;
      int64_t meta_index = OB_INVALID_INDEX;
      if(NULL == designer_ || NULL == designer_->get_hist_manager())
      {
        ret = OB_ERR_NULL_POINTER;
        TBSYS_LOG(WARN, "hist manager cannot be NULL");
      }
      //TBSYS_LOG(ERROR, "test::whx tablet_info = %s", to_cstring(add_tablets.tablets[0].tablet_info.range_));
      for (int64_t i = 0; i < add_tablets.tablet_list.get_array_index() && OB_SUCCESS == ret; i ++)
      {
        const ObTabletHistogramReportInfo &report_info = add_tablets.tablets[i];
        //TBSYS_LOG(ERROR, "test::whx tablet_info pre = %s, other = %s, idx = %ld", to_cstring(report_info.tablet_info.range_), to_cstring(add_tablets.tablets[i].tablet_info.range_), i);
        if(report_info.tablet_info.range_.table_id_ != designer_->get_table_id())
        {
          ret = OB_INVALID_DATA;
          TBSYS_LOG(WARN, "tid is not equal report info, info tid = %ld, designer tid = %ld", report_info.tablet_info.range_.table_id_, designer_->get_table_id());
        }
        else if(report_info.tablet_info.range_.table_id_ != designer_->get_hist_manager()->get_table_id())
        {
          ret = OB_INVALID_DATA;
          TBSYS_LOG(WARN, "tid is not equal report info, info tid = %ld, hist manager tid = %ld", report_info.tablet_info.range_.table_id_, designer_->get_hist_manager()->get_table_id());
        }
        else
        {
          //TBSYS_LOG(ERROR, "test::whx tablet_info = %s, other = %s, idx = %ld", to_cstring(report_info.tablet_info.range_), to_cstring(add_tablets.tablets[i].tablet_info.range_), i);
          tbsys::CThreadGuard hist_mutex_gard(&designer_mutex_);
          if(OB_SUCCESS != (ret = designer_->add_hist_meta(add_tablets.tablets[i].tablet_info, meta_index, server_index)))
          {
            TBSYS_LOG(WARN, "add hist meta into designer failed, ret = %d", ret);
          }
          else
          {
            TBSYS_LOG(INFO, "add hist meta into designer succ.");
          }
        }

        if(OB_SUCCESS == ret)
        {
          if(OB_SUCCESS != (ret = designer_->get_hist_manager()->add_histogram(report_info.static_index_histogram, hist_index)))
          {
            TBSYS_LOG(WARN, "add histogram in hist manager failed, ret = %d", ret);
          }
          else if(OB_SUCCESS != (ret = designer_->set_meta_index(meta_index, hist_index)))
          {
            TBSYS_LOG(WARN, "set histgram failed, meta_index = %ld, hist_index = %ld", meta_index, hist_index);
          }
        }

      }
      return ret;
    }

    int ObIndexControlUnit::stage_prepare(uint64_t index_tid)
    {
      int ret = OB_SUCCESS;
      uint64_t table_id = OB_INVALID_ID;

      if(NULL == root_woker_ || NULL == designer_)
      {
        ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(WARN,"woker pointer cannot be null, ret = %d", ret);
      }
      else if(OB_SUCCESS != (ret = root_woker_->get_root_server().get_table_from_index(index_tid, table_id)))
      {
        TBSYS_LOG(WARN, "get table_id failed, ret = %d",ret);
      }
      else
      {
        ch_.set_index_tid(index_tid);
        service_.set_index_tid(index_tid);
        common::ObTabletHistogramManager* manager = designer_->get_hist_manager();
        if(NULL == manager)
        {
          TBSYS_LOG(WARN, "hist manager is NULL");
          ret = OB_INNER_STAT_ERROR;
        }
        else
        {
          manager->reuse();
          ret = manager->set_table_id(table_id, index_tid);
          tbsys::CThreadGuard mutex_gard(&designer_mutex_);
          designer_->reset();
          ret = designer_->set_table_id(table_id, index_tid);
        }
      }
      return ret;
    }

    void ObIndexControlUnit::run_job()
    {
      int err = OB_SUCCESS;
     // service_.start_timer();
      if(NULL == designer_)
      {
        TBSYS_LOG(WARN, "designer cannot be null");
      }
      else
      {
        for(int64_t i = 0; i < job_list_.count(); i++)
        {
          uint64_t index_id = job_list_.at(i);
          err = pipe_run_job(index_id);
          if(OB_SUCCESS != err)
          {
            TBSYS_LOG(WARN, "index[%ld] build failed.push to failed list,err = %d", index_id, err);
            failed_list_.push_back(index_id);
          }
          else
          {
            finish_index_num_ ++;
            TBSYS_LOG(INFO, "index[%ld] build success!", index_id);
          }
            ch_.reset();
            {
              tbsys::CThreadGuard mutex_gard(&designer_mutex_);
              {
                designer_->reset();
              }
            }
        }
      }
    }

    int ObIndexControlUnit::fill_all_samples()
    {
      int ret = OB_SUCCESS;
      tbsys::CThreadGuard hist_mutex_gard(&designer_mutex_);
      if(NULL == designer_)
      {
        ret = OB_ERR_NULL_POINTER;
      }
      else
      {
        ret = designer_->sort_all_sample();
      }
      return ret;
    }

    int ObIndexControlUnit::pipe_run_job(uint64_t idx_id)
    {
      int ret = OB_SUCCESS;

      if(NULL == root_woker_ || NULL == designer_)
      {
        TBSYS_LOG(WARN, "some pointer cannot be NULL");
        ret = OB_INNER_STAT_ERROR;
      }
      else if(OB_SUCCESS != (ret = stage_prepare(idx_id)))
      {
        TBSYS_LOG(WARN, "prepare stage [%ld] failed, ret = %d", idx_id, ret);
      }
      else if(OB_SUCCESS != (ret = ch_.construct_handler_core()))
      {
        TBSYS_LOG(WARN, "index[%ld] construct_handler_core failed,ret = %d", idx_id, ret);
        failed_list_.push_back(idx_id);
      }
      if(OB_SUCCESS == ret)
      {
        int try_time = 0;
        TBSYS_LOG(INFO, "Now start index[%ld] constrcut!", idx_id);
        mission_start_time_ = tbsys::CTimeUtil::getTime();
        while(true)
        {
          try_time ++;
          const int64_t sleep_interval = 3000000;
          int64_t now = tbsys::CTimeUtil::getTime();
          bool finished1 = false, finished2 = false;
          bool need_delete_rt = false;
          ///step1: 判断创建局部索引的时间是否超时
          ch_.set_index_beat(LOCAL_INDEX_STAGE);
          while(!finished1)
          {
            now = tbsys::CTimeUtil::getTime();
            if(now > mission_start_time_ + root_woker_->get_root_server().get_config().monitor_create_index_timeout)
            {
              ret = OB_ERROR;
              TBSYS_LOG(WARN, "build local static index timeout, mission_start_time_=%ld, now=%ld.", mission_start_time_, now);
              break;
            }
            else if(OB_SUCCESS != (ret = designer_->check_local_index_build_done(idx_id, finished1, designer_mutex_)))
            {
              TBSYS_LOG(WARN, "check create local index[%lu] failed", idx_id);
              break;
            }
            else if (!finished1)
            {
              TBSYS_LOG(INFO, "building local static index [%lu], sleep %ldus and check again.", idx_id, sleep_interval);
              usleep(sleep_interval);
            }
          }


          if(OB_SUCCESS == ret)
          {
            ///step2, if local index build success, write golbal index range  into rt
            if(OB_SUCCESS != (ret = fill_all_samples()))
            {
              TBSYS_LOG(WARN, "failed to fill samples, ret = %d", ret);
            }
            else if(OB_SUCCESS != (ret = designer_->design_global_index(ch_.get_width(), designer_mutex_)))
            {
              TBSYS_LOG(WARN, "failed to fill samples, ret = %d", ret);
              need_delete_rt = true;
            }
            else
            {
              need_delete_rt = true;
              ///step3 check if global index done
              ch_.set_index_beat(GLOBAL_INDEX_STAGE);
              while(!finished2)
              {
                now = tbsys::CTimeUtil::getTime();
                if (now > mission_start_time_ + root_woker_->get_root_server().get_config().monitor_create_index_timeout)
                {
                  ret = OB_INNER_STAT_ERROR;
                  TBSYS_LOG(WARN, "build global static index timeout, mission_start_time_=%ld, now=%ld.", mission_start_time_, now);
                  break;
                }
                else if(OB_SUCCESS != (ret = designer_->check_global_index_build_done(idx_id, finished2)))
                {
                  TBSYS_LOG(WARN, "check create global index[%lu] failed", idx_id);
                  break;
                }
                else if (!finished2)
                {
                  TBSYS_LOG(INFO, "building global static index [%lu], sleep %ldus and check again.", idx_id, sleep_interval);
                  usleep(sleep_interval);
                }
              }//end while 2
            }
          }

          ///step4 check column checksum
          if(OB_SUCCESS == ret)
          {
            /// check column checksum here
            if(OB_SUCCESS != (ret = root_woker_->get_root_server().check_column_checksum(idx_id)))
            {
              TBSYS_LOG(WARN, "check index %ld column checksum failed", idx_id);
            }
            else if(OB_SUCCESS != (ret = designer_->balance_index(idx_id)))
            {
              TBSYS_LOG(WARN, "failed to balance index, ret = %d", ret);
            }
            else if(OB_SUCCESS != (ret = service_.modify_index_process_info(idx_id, NOT_AVALIBALE)))
            {
              TBSYS_LOG(WARN, "fail modify index table's stat to [NOT_AVALIBALE], index_tid=%ld", idx_id);
            }
            else
            {
              usleep(sleep_interval);
            }
          }
          ///clear up the mess
          if (OB_SUCCESS != ret)
          {
            clean_mess(idx_id, need_delete_rt);
          }
          if(try_time > 0 || OB_SUCCESS == ret)break;
        }
      }
      return ret;
    }

    int ObIndexControlUnit::clean_mess(const uint64_t idx_id, const bool need_delete_rt)
    {
      int err = OB_SUCCESS;
      common::ObArray<uint64_t> delete_table;
      ///step1. modify index stat to ERROR, if modify failed, does not matter, because index is not avaliable now.
      if (OB_SUCCESS != (err = service_.modify_index_process_info(idx_id, ERROR)))
      {
        TBSYS_LOG(WARN, "fail modify index table's stat to [ERROR], index_tid=%ld.", idx_id);
      }
      else
      {
        TBSYS_LOG(INFO, "index table[%ld] stat changed to [ERROR].", idx_id);
      }
      if (need_delete_rt)
      {
       ///step2. delete index table from rt.
       if (OB_SUCCESS != (err = delete_table.push_back(idx_id)))
       {
         TBSYS_LOG(WARN, "add idx_id to delete table failed. index_tid=%ld", idx_id);
       }
       else if (OB_SUCCESS != (err = root_woker_->get_root_server().delete_tables(false, delete_table)))
       {
         TBSYS_LOG(ERROR, "fail to delete index table from rt, need clean root table after merge done. idex_tid=%ld, err=%d", idx_id, err);
       }
       else
       {
         TBSYS_LOG(INFO, "delete index table from rt success. index_tid=%ld", idx_id);
       }
     }
      return err;
    }



  }     //end of rootserver
}//end of oceanbase
