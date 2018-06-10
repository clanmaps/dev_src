/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file  ob_index_service.cpp
 * @brief ObIndexService provide interface of inner system table, with scan / update
 *        generally use it to modify index info
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

#include "ob_index_service.h"
#include "ob_root_worker.h"
#include "common/ob_schema_service_impl.cpp"
namespace oceanbase
{
  namespace rootserver
  {
    ObIndexService::ObIndexService()
      :index_tid_(OB_INVALID_ID),root_worker_(NULL){}
    ObIndexService::~ObIndexService(){}

    void ObIndexService::init(ObRootWorker *worker)
    {
      root_worker_ = worker;
    }

    int ObIndexService::submit_job(const common::PacketCode pc)
    {
      int ret = OB_SUCCESS;
      ObPacket packet;
      packet.set_packet_code(pc);
      if(NULL == root_worker_)
      {
        TBSYS_LOG(WARN, "the pointer of root_worker in service is NULL");
        ret = OB_ERROR;
      }
      else if(OB_SUCCESS != (ret = root_worker_->submit_job(packet)))
      {
        TBSYS_LOG(WARN, "submit job [%d] to rootserver failed,ret[%d]", pc, ret);
      }
      else
      {
        TBSYS_LOG(INFO, "submit a index job success!");
      }
      return ret;
    }

    int ObIndexService::set_env(ObScanHelper *client_proxy)
    {
      int ret = OB_SUCCESS;
      if (NULL == client_proxy)
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(WARN, "client proxy is null");
      }
      else
      {
        this->client_proxy_ = client_proxy;
        ret = nb_accessor_.init(client_proxy_);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "init nb accessor fail:ret[%d]", ret);
        }
        else
        {
          nb_accessor_.set_is_read_consistency(true);
        }
      }
      return ret;
    }

    int ObIndexService::fetch_index_stat(const uint64_t table_id, const int64_t cluster_id, int64_t &stat)
    {
      int ret = OB_SUCCESS;
      stat = -1;
      if(NULL == client_proxy_)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "check inner stat fail");
      }
      else
      {
        ObObj rowkey_objs[2];
        rowkey_objs[0].set_int(table_id);
        rowkey_objs[1].set_int(cluster_id);
        ObRowkey rowkey;
        rowkey.assign(rowkey_objs, 2);
        QueryRes* res = NULL;
        if (OB_SUCCESS != (ret = nb_accessor_.get(res, OB_INDEX_SERVICE_INFO_TABLE_NAME, rowkey, SC("status"))))
        {
          TBSYS_LOG(WARN, "failed to access row, err=%d", ret);
        }
        else
        {
          TableRow* table_row = res->get_only_one_row();
          //int32_t st = -1;
          if (NULL == table_row)
          {
            TBSYS_LOG(INFO, "failed to get row from query results");
          }
          else
          {
            ASSIGN_INT("status", stat, int64_t);
          }
          nb_accessor_.release_query_res(res);
          res = NULL;
        }
      }
      return ret;
    }

    int ObIndexService::modify_index_process_info(const uint64_t index_tid, const IndexStatus stat)
    {
      int ret = OB_SUCCESS;
      ObMutator* mutator = NULL;
      ObServer master_master_ups;
      ObServer obi_rs;
      ObGeneralRpcStub rpc_stub;
      int64_t timeout = 3000000;
      ObRowkey rowkey;
      ObObj obj[2];
      ObObj status;
      if(NULL == root_worker_)
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "root worker cannot be NULL");
      }
      else
      {
        rpc_stub = root_worker_->get_general_rpc_stub();
      }
      //1. first, we get a master ups in master cluster.
      if(OB_SUCCESS == ret)
      {
        if(OB_SUCCESS != (ret = rpc_stub.get_master_obi_rs(timeout, root_worker_->get_root_server().get_self(), obi_rs)))
        {
          TBSYS_LOG(WARN, "get obi rs failed, ret = %d", ret);
        }
      }
      //2. next, we construct mutator to apply
      if(OB_SUCCESS == ret)
      {
        mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
        if(NULL == mutator)
        {
          ret = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(WARN, "get thread specific ObMutator failed");
        }
        else if(OB_SUCCESS != (ret = mutator->reset()))
        {
          TBSYS_LOG(WARN, "reset mutator failed, ret[%d]", ret);
        }
        else
        {
          uint64_t stat_cid = 18;
          obj[0].set_int(index_tid);
          obj[1].set_int(root_worker_->get_root_server().get_config().cluster_id);
          status.set_int(stat);
          rowkey.assign(obj, 2);
          if(OB_SUCCESS != (ret = mutator->insert(OB_INDEX_SERVICE_INFO_TID, rowkey, stat_cid, status)))
          {
            TBSYS_LOG(WARN, "construct mutator for modify index process info failed, ret[%d]", ret);
          }
        }
      }

      //3. finally, we send mutator to apply
      ObScanner* scanner=NULL;
      if(OB_SUCCESS == ret)
      {
        int64_t retry_times = root_worker_->get_root_server().get_config().retry_times;
        for(int64_t i = 0; i < retry_times; i++)
        {
          if(OB_SUCCESS != (ret = rpc_stub.get_master_ups_info(timeout, obi_rs, master_master_ups)))
          {
            TBSYS_LOG(WARN, "failed to get master master ups, ret[%d]", ret);
          }
          else if(OB_SUCCESS != (ret = rpc_stub.mutate(timeout, master_master_ups, *mutator, false, *scanner)))
          {
            TBSYS_LOG(WARN, "replace mutator failed [%ld] times, ret[%d]", i, ret);
            //usleep(5000000);
          }
          else
          {
            break;
          }
        }
      }
      return ret;
    }

    int ObIndexService::modify_index_stat_ddl(ObString index_table_name, uint64_t index_table_id, int stat)
    {
      int ret = OB_SUCCESS;
      UNUSED(index_table_id);
      tbsys::CThreadGuard guard(&mutex_);
      ObMutator* mutator = NULL;
      mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
      if(NULL == mutator)
      {
        ret = OB_ALLOCATE_MEMORY_FAILED;
        TBSYS_LOG(WARN, "get thread specific Mutator fail");
      }
      else if(NULL == root_worker_)
      {
        ret = OB_INNER_STAT_ERROR;
      }
      tbsys::CThreadMutex& ddl_lock = root_worker_->get_root_server().get_ddl_lock();
      tbsys::CThreadGuard ddl_guard(&ddl_lock);
      if(OB_SUCCESS == ret)
      {
        ret = mutator->reset();
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "reset ob mutator fail:ret[%d]", ret);
        }
      }
      if(OB_SUCCESS == ret)
      {
        ret = create_index_status_mutator(index_table_name,stat, mutator);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "create table mutator fail:ret[%d]", ret);
        }
      }

      if(OB_SUCCESS == ret)
      {
        ret = client_proxy_->mutate(*mutator);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "apply mutator fail:ret[%d]", ret);
        }
      }
      return ret;
    }

    int ObIndexService::create_index_status_mutator(ObString index_table_name, int status, ObMutator *mutator)
    {
      int ret = OB_SUCCESS;
      ObObj table_name_value;
      table_name_value.set_varchar(index_table_name);
      ObRowkey rowkey;
      rowkey.assign(&table_name_value,1);
      UPDATE_INT(common::secondary_index_table_name,rowkey,"index_status",status);
      return ret;
    }

    int ObIndexService::get_index_stat(const uint64_t table_id, const int64_t cluster_count, IndexStatus &stat)
    {
        int ret = OB_SUCCESS;
        stat = INDEX_INIT;
        if(NULL == client_proxy_)
        {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "check inner stat fail");
        }
        else
        {
          QueryRes* res = NULL;
          ObNewRange range;
          int32_t rowkey_column = 2;
          ObObj start_rowkey[rowkey_column];
          ObObj end_rowkey[rowkey_column];
          start_rowkey[0].set_int(table_id);
          start_rowkey[1].set_min_value();
          end_rowkey[0].set_int(table_id);
          end_rowkey[1].set_max_value();
          if (OB_SUCCESS == ret)
          {
            range.start_key_.assign(start_rowkey, rowkey_column);
            range.end_key_.assign(end_rowkey, rowkey_column);
          }
          if(OB_SUCCESS == ret)
          {
            ret = nb_accessor_.scan(res, OB_INDEX_SERVICE_INFO_TABLE_NAME, range, SC("status"));
            if(OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "scan __index_process_info fail:ret[%d]", ret);
            }
          }

          if(OB_SUCCESS == ret)
          {
            int64_t count = 0;
            int32_t st = -1;
            IndexStatus tmp_stat = INDEX_INIT;
            TableRow* table_row = NULL;
            int err = OB_SUCCESS;
            while(OB_SUCCESS == (err = res->next_row()) && OB_SUCCESS == ret)
            {
              res->get_row(&table_row);
              if (NULL == table_row)
              {
                TBSYS_LOG(WARN, "failed to get row from query results");
                ret = OB_ERROR;
              }
              else
              {
                count++;
                ASSIGN_INT("status", st, int32_t);
                switch (st) {
                  case 0:
                    tmp_stat = NOT_AVALIBALE;
                    break;
                  case 1:
                    tmp_stat = AVALIBALE;
                    break;
                  case 2:
                    tmp_stat = ERROR;
                    break;
                  case 3:
                    tmp_stat = WRITE_ONLY;
                    break;
                  case 4:
                    tmp_stat = INDEX_INIT;
                    break;
                  default:
                    break;
                }
                if (ERROR == tmp_stat)
                {
                  break;
                }
              }
            }//end while

            if (OB_SUCCESS == ret)
            {
              if (ERROR == tmp_stat ||
                  (NOT_AVALIBALE == tmp_stat && count == cluster_count))
              {
                stat = tmp_stat;
              }
              /*else
              {
                ret = OB_EAGAIN;//记录数不等于cluster数
              }*/
            }
          }

          nb_accessor_.release_query_res(res);
          res = NULL;
        }
        return ret;
    }



  }//end of rootserver
}//end of oceanbase
