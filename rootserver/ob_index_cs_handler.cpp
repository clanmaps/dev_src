/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_cs_handler.cpp
 * @brief handle chunkserver while index construct,include cs selection/server off_line and so on
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

#include "ob_index_cs_handler.h"
#include "ob_root_worker.h"
namespace oceanbase
{
  namespace rootserver
  {
    ObCSHandler::ObCSHandler()
            :root_worker_(NULL),index_tid_(OB_INVALID_ID),width_(0),init_(false){}

    ObCSHandler::~ObCSHandler()
    {
      cm_.clear();
      index_tid_ = OB_INVALID_ID;
    }

    int ObCSHandler::init(ObRootWorker *worker)
    {
      int ret = OB_SUCCESS;
      if(!init_)
      {
        if(OB_SUCCESS != (ret = cm_.create(hash::cal_next_prime(MAX_CHUNKSERVER_NUM))))
        {
          TBSYS_LOG(WARN, "init cs handler core failed, ret = %d", ret);
        }
        else
        {
          root_worker_ = worker;
          init_ = true;
        }
      }
      return ret;
    }

    void ObCSHandler::reset()
    {
      cm_.clear();
      width_ = 0;
      beat_.reset();
      index_tid_ = OB_INVALID_ID;
    }

    void ObCSHandler::set_index_tid(uint64_t idx)
    {
      index_tid_ = idx;
    }

    bool ObCSHandler::cs_hit_hashmap(ObServer &cs)
    {
      bool ret = false;
      bool val = false;
      if(HASH_EXIST == cm_.get(cs,val) && val)
      {
        ret =  true;
      }
      return ret;
    }

    int ObCSHandler::fetch_tablet_info(const uint64_t table_id, const ObRowkey &row_key, ObScanner &scanner)
    {
      int ret = OB_SUCCESS;
      ObCellInfo cell;
      // cell info not root table id
      cell.table_id_ = table_id;
      cell.column_id_ = 0;
      cell.row_key_ = row_key;
      ObGetParam get_param;
      get_param.set_is_result_cached(false);
      get_param.set_is_read_consistency(false);
      ret = get_param.add_cell(cell);
      if(NULL == root_worker_)
      {
        TBSYS_LOG(WARN, "null pointer of worker_");
        ret = OB_ERROR;
      }
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "get param add cell failed,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = root_worker_->get_root_server().find_root_table_key(get_param,scanner)))
      {
        TBSYS_LOG(WARN, "find root table info failed[%d]", ret);
      }
      return ret;
    }

    int ObCSHandler::fill_cm_and_calc(ObScanner &scanner, ObRowkey &row_key, uint64_t table_id, bool refresh_width)
    {
      int ret = OB_SUCCESS;
      UNUSED(table_id);
      ObRowkey start_key;
      start_key = ObRowkey::MIN_ROWKEY;
      ObRowkey end_key;
      ObServer server;
      ObCellInfo * cell = NULL;
      bool row_change = false;
      //ObTabletLocationList list;
      CharArena allocator;
      ObScannerIterator iter = scanner.begin();
      ObNewRange range;
      ++iter;
      while ((iter != scanner.end())
             && (OB_SUCCESS == (ret = iter.get_cell(&cell, &row_change))) && !row_change)
      {
        if (NULL == cell)
        {
          ret = OB_INNER_STAT_ERROR;
          break;
        }
        cell->row_key_.deep_copy(start_key, allocator);
        ++iter;
      }

      if (ret == OB_SUCCESS)
      {
        int64_t ip = 0;
        int64_t port = 0;
        /// next cell
        for(++iter; iter != scanner.end(); ++iter)
        {
          ret = iter.get_cell(&cell,&row_change);
          if(ret != OB_SUCCESS)
          {
            TBSYS_LOG(ERROR, "get cell from scanner iterator failed:ret[%d]", ret);
            break;
          }
          else if (row_change) // && (iter != last_iter))
          {
            //construct_tablet_item(table_id,start_key, end_key, range, list);
            if(refresh_width)
            {
              width_++;
            }
            //list.clear();
            start_key = end_key;
          }
          else
          {
            cell->row_key_.deep_copy(end_key, allocator);
            if ((cell->column_name_.compare("1_port") == 0)
                //|| (cell->column_name_.compare("2_port") == 0)
                //|| (cell->column_name_.compare("3_port") == 0)
                )
            {
              ret = cell->value_.get_int(port);
            }
            else if ((cell->column_name_.compare("1_ipv4") == 0)
                     //|| (cell->column_name_.compare("2_ipv4") == 0)
                     //|| (cell->column_name_.compare("3_ipv4") == 0)
                     )
            {
              ret = cell->value_.get_int(ip);
              if (OB_SUCCESS == ret)
              {
                if (port == 0)
                {
                  TBSYS_LOG(WARN, "check port failed:ip[%ld], port[%ld]", ip, port);
                }
                server.set_ipv4_addr(static_cast<int32_t>(ip), static_cast<int32_t>(port));
                cm_.set(server, true, 1);
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
        /// for the last row
        TBSYS_LOG(DEBUG, "get a new tablet start_key[%s], end_key[%s]",
                  to_cstring(start_key), to_cstring(end_key));
        if ((OB_SUCCESS == ret) && (start_key != end_key))
        {
          if(refresh_width)
          {
            width_++;
          }
        }
      }
      else
      {
        TBSYS_LOG(ERROR, "check get first row cell failed:ret[%d]", ret);
      }
      if(OB_SUCCESS == ret)
      {
        row_key = end_key;
      }
//      TBSYS_LOG(INFO,"test::whx range_count(%ld)",range_hash_.size());
      return ret;
    }

    int ObCSHandler::construct_handler_core(bool refresh_width)
    {
      int ret = OB_SUCCESS;
      if(!init_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "handler core has not been inited,ret = %d", ret);
      }
      else
      {
        cm_.clear();
      }
      if(OB_SUCCESS == ret)
      {
        ObSchemaManagerV2* schema_mgr = OB_NEW(ObSchemaManagerV2, ObModIds::OB_RS_SCHEMA_MANAGER);
        const ObTableSchema* schema = NULL;
        if (NULL == schema_mgr)
        {
          TBSYS_LOG(WARN, "fail to new schema_manager.");
          ret = OB_ALLOCATE_MEMORY_FAILED;
        }
        else if(NULL == root_worker_)
        {
          TBSYS_LOG(WARN, "null pointer of root worker");
          ret = OB_ERR_NULL_POINTER;
        }
        else if(OB_SUCCESS != (ret = root_worker_->get_root_server().get_schema(false,false,*schema_mgr)))
        {
          TBSYS_LOG(WARN, "get schema manager for monitor failed,ret[%d]", ret);
        }
        else if(NULL == ((schema = schema_mgr->get_table_schema(index_tid_))))
        {
          TBSYS_LOG(WARN,  "get index schema failed!");
          ret = OB_ERR_NULL_POINTER;
        }

        if(OB_SUCCESS == ret)
        {
          uint64_t data_tid = schema->get_original_table_id();
          ObScanner scanner;
          ObRowkey start_key;
          start_key.set_min_row();
          do
          {
            ret = fetch_tablet_info(data_tid, start_key, scanner);
            if(OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "fetch tablet info failed, ret %d", ret);
              break;
            }
            else if(OB_SUCCESS != (ret = fill_cm_and_calc(scanner, start_key, data_tid, refresh_width)))
            {
              TBSYS_LOG(WARN, "failed to fill cm ,ret =%d",ret);
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
        }
      }
      dump_core();
      return ret;
    }

    void ObCSHandler::set_index_beat(ConIdxStage ph)
    {
      beat_.idx_tid_ = index_tid_;
      beat_.hist_width_ = width_;
      beat_.stage_ = ph;
      beat_.status_ = INDEX_INIT;
    }

    IndexBeat &ObCSHandler::get_beat()
    {
      return beat_;
    }

    IndexBeat &ObCSHandler::get_default_beat()
    {
      return default_beat_;
    }

    int ObCSHandler::server_off_line(ObServer &cs)
    {
      int ret = OB_SUCCESS;
      if(cs_hit_hashmap(cs))
      {
        if(OB_SUCCESS != (ret = construct_handler_core(false)))
        {
          TBSYS_LOG(WARN, "handle server off line failed,ret = %d", ret);
        }
      }
      return ret;
    }

    void ObCSHandler::dump_core()
    {
      hash::ObHashMap<ObServer,bool,hash::NoPthreadDefendMode>::const_iterator itr = cm_.begin();
      int i = 0;
      for(; itr != cm_.end(); itr++)
      {
        TBSYS_LOG(INFO, "dump core server [%s], i = %d", to_cstring(itr->first), i++);
      }
      TBSYS_LOG(INFO, "dump core server count = %d", i);
    }


  }
}
