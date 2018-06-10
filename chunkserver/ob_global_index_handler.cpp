/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_global_index_handler.cpp
 * @brief for global stage of construct secondary index
 *
 * Created by longfei： for global stage of construct secondary index
 * future work
 *   1.some function need to be realized,see todo list in this page
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_05
 */

#include "ob_global_index_handler.h"
#include "ob_chunk_server_main.h"
#include "ob_cs_interactive_scan.h"
#include "sstable/ob_sstable_writer.h"
#include "common/file_directory_utils.h"
#include "ob_chunk_server_main.h"

namespace oceanbase
{
  namespace chunkserver
  {

    ObGlobalIndexHandler::ObGlobalIndexHandler(ObIndexHandlePool *pool,
                                               common::ObMergerSchemaManager *schema_mgr, ObTabletManager* tablet_mgr) :
      ObIndexHandler(pool, schema_mgr, tablet_mgr),handle_range_(NULL),
      ms_wrapper_(*(ObChunkServerMain::get_instance()->get_chunk_server().get_rpc_proxy()),
                  ObChunkServerMain::get_instance()->get_chunk_server().get_config().merge_timeout)
    {
      table_id_ = OB_INVALID_ID;
    }

    ObGlobalIndexHandler::~ObGlobalIndexHandler()
    {
    }

    int ObGlobalIndexHandler::to_string()
    {
      int ret = OB_SUCCESS;
      //@todo(longfei):complete
      return ret;
    }

    int ObGlobalIndexHandler::init()
    {
      int ret = OB_SUCCESS;
      if(NULL == get_handle_pool())
      {
        TBSYS_LOG(ERROR,"null pointer of handler_pool_");
        ret = OB_ERR_NULL_POINTER;
      }
      else if(NULL == (range_server_hash_ = get_handle_pool()->get_range_info()))
      {
        TBSYS_LOG(ERROR,"null pointer of range_hash_pointer");
        ret = OB_ERR_NULL_POINTER;
      }
      return ret;
    }

    int ObGlobalIndexHandler::start()
    {
      int ret = OB_SUCCESS;
      //1.构建处理的range --> 应该交给上层去完成
      if (NULL == handle_range_)
      {
        TBSYS_LOG(ERROR,"handle_range_ is null.");
        ret = OB_ERROR;
      }
      //2.交给start_impl来实现
      else if (OB_SUCCESS != (ret = cons_global_index(handle_range_)))
      {
        TBSYS_LOG(WARN, "global index construction failed,ret[%d]",ret);
      }
      else
      {
        TBSYS_LOG(DEBUG,"construct global index succ.");
      }
      //3.打印错误信息，返回错误码
      return ret;
    }

    int ObGlobalIndexHandler::fill_scan_param(ObScanParam &param)
    {
      int ret = OB_SUCCESS;
      const ObSSTableSchemaColumnDef* def = NULL;
      param.set_fake(true);
      param.set_copy_args(true);
      //add column
      if (OB_SUCCESS == (ret = param.set_range(get_new_range())))
      {
        for (int64_t i = 0; i < get_sstable_schema().get_column_count(); i++)
        {
          if (NULL == (def = get_sstable_schema().get_column_def((int) i)))
          {
            TBSYS_LOG(WARN, "get column define failed");
            ret = OB_ERROR;
            break;
          }
          else
          {
            param.add_column(def->column_name_id_);
          }
        }
      }
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "fill scan param failed[%d]", ret);
      }
      return ret;
    }

    int ObGlobalIndexHandler::write_global_index()
    {
      int ret = OB_SUCCESS;
      const ObRow *row = NULL;
      ObRowDesc desc;
      ObCsInteractiveScan cs2cs_scan;
      if(OB_SUCCESS != (ret = sort_.set_child(0, cs2cs_scan)))
      {
        TBSYS_LOG(WARN,"set sort's children failed.ret[%d]",ret);
      }
      ObRowDesc index_desc;
      cc.reset();
      if (OB_SUCCESS != ret || NULL == get_handle_pool()
          || NULL == get_handle_pool()->get_tablet_manager())
      {
        ret = OB_ERR_UNEXPECTED;
        TBSYS_LOG(WARN, "handler construct error");
      }
      else if (OB_SUCCESS != (ret = get_sstable_row().set_table_id(table_id_)))
      {
        TBSYS_LOG(WARN, "failed to set sstable row tid[%d]", ret);
      }
      else if (OB_SUCCESS != (ret = get_sstable_row().set_column_group_id(0)))
      {
        TBSYS_LOG(WARN, "failed to set sstable row column_group_id[%d]", ret);
      }
      else if (OB_SUCCESS != (ret = cons_row_desc(desc)))
      {
        TBSYS_LOG(WARN, "failed to construct row desc[%d]", ret);
      }
      else if (OB_SUCCESS != (ret = cons_row_desc_without_virtual(index_desc)))
      {
        TBSYS_LOG(WARN, "failed to construct index row desc[%d]", ret);
      }

      /*construct operator local_scan*/
      if (OB_SUCCESS == ret)
      {
        ScanContext sc;
        local_agent_.set_row_desc(desc);
        local_agent_.set_server(THE_CHUNK_SERVER.get_self());
        get_handle_pool()->get_tablet_manager()->build_scan_context(sc);
        local_agent_.set_scan_context(sc);
        if(range_server_hash_ != NULL)
        {
          local_agent_.set_range_server_hash(range_server_hash_);
        }
        else
        {
          ret = OB_ERR_NULL_POINTER;
          TBSYS_LOG(ERROR,"range_server_hash hasn't been set.ret[%d]",ret);
        }
      }
      /*construct operator ia_scan*/
      if (OB_SUCCESS == ret)
      {
        interactive_agent_.set_row_desc(desc);
      }
      if (OB_SUCCESS == ret)
      {
        for (int64_t i = 0; i < desc.get_rowkey_cell_count(); i++)
        {
          uint64_t tid = OB_INVALID_ID;
          uint64_t cid = OB_INVALID_ID;
          if (OB_SUCCESS != (ret = desc.get_tid_cid(i, tid, cid)))
          {
            TBSYS_LOG(WARN, "get tid cid from row desc failed,i[%ld],ret[%d]",
                      i, ret);
            break;
          }
          else if (OB_SUCCESS != (ret = sort_.add_sort_column(tid, cid, true)))
          {
            TBSYS_LOG(WARN, "set sort column failed,tid[%ld], cid[%ld],ret[%d]",
                      tid, cid, ret);
            break;
          }
        }
      }
      /*construct operator cs2cs_scan*/
      if (OB_SUCCESS != (ret = cs2cs_scan.set_child(0, local_agent_))) //设置左孩子
      {
        TBSYS_LOG(ERROR, "set local scan failed!");
        ret = OB_ERROR;
      }
      if (OB_SUCCESS != (ret = cs2cs_scan.set_child(1, interactive_agent_))) //设置右孩子
      {
        TBSYS_LOG(ERROR, "set local scan failed!");
        ret = OB_ERROR;
      }
      if (OB_SUCCESS == ret)
      {
        ret = sort_.open();
      }
      if (OB_SUCCESS == ret)
      {
        while (OB_SUCCESS == (ret = sort_.get_next_row(row)))
        {
          if (OB_SUCCESS != (ret = calc_tablet_col_checksum_index(
                               *row,
                               index_desc,
                               cc.get_str(),
                               table_id_)))
          {
            TBSYS_LOG(ERROR, "fail to calculate tablet column checksum index =%d", ret);
            break;
          }
          else if (OB_SUCCESS != (ret = column_checksum_.add(cc)))
          {
            TBSYS_LOG(ERROR, "checksum sum error =%d", ret);
            break;
          }
          cc.reset();
          if (OB_SUCCESS != (ret = trans_row_to_sstrow(desc, *row, get_sstable_row())))
          {
            TBSYS_LOG(WARN, "failed to trans row to sstable_row,ret = %d", ret);
            break;
          }
          else if (OB_SUCCESS != (ret = save_current_row()))
          {
            TBSYS_LOG(WARN, "failed to save current row,ret = %d", ret);
            break;
          }
        }
        if (OB_ITER_END == ret)
        {
          {
            ObMultiVersionTabletImage& tablet_image =
                get_tablet_mgr()->get_serving_tablet_image();
            int64_t frozen_version = tablet_image.get_serving_version();
            set_frozen_version(frozen_version);
            //delete [secondary index bug.fix]
            //ObServer master_master_ups;
            //if(OB_SUCCESS != (ret = ObChunkServerMain::get_instance()->get_chunk_server().get_rpc_proxy()->get_master_master_update_server(true,master_master_ups)))
            //{
            //  TBSYS_LOG(ERROR,"failed to get ups for report index cchecksum");
            //}
            //delete e
            if (OB_SUCCESS != (ret = get_tablet_mgr()->send_tablet_column_checksum(
                                 column_checksum_, get_new_range(), get_frozen_version())))
            {
              TBSYS_LOG(ERROR, "send tablet column checksum failed =%d", ret);
            }
            column_checksum_.reset();
            get_tablet_extend_info().row_checksum_ = get_sstable_writer().get_row_checksum();
          }
        }
      }
      if (OB_SUCCESS == ret)
      {
        ObTablet* index_image_tablet;
        ObMultiVersionTabletImage& tablet_image = get_tablet_mgr()->get_serving_tablet_image();
        bool sync_meta = THE_CHUNK_SERVER.get_config().each_tablet_sync_meta;
        if ((ret = tablet_image.alloc_tablet_object(
               get_new_range(),
               get_frozen_version(),
               index_image_tablet)) != OB_SUCCESS)
        {
          TBSYS_LOG(ERROR, "alloc_index_image_tablet_object failed.");
        }
        else
        {
          if ((ret = construct_index_tablet_info(index_image_tablet)) == OB_SUCCESS)
          {
            if (OB_SUCCESS != (ret = update_meta_index(index_image_tablet, sync_meta)))
            {
              TBSYS_LOG(WARN, "upgrade new index tablets error [%d]", ret);
            }
          }
        }
      }
      sort_.reset();
      local_agent_.reset();
      interactive_agent_.reset();
      set_sstable_size(0);
      /*when we stop to read sstable,we should reset tmi*/
      ObThreadAIOBufferMgrArray* aio_buf_mgr_array = GET_TSI_MULT(ObThreadAIOBufferMgrArray, TSI_SSTABLE_THREAD_AIO_BUFFER_MGR_ARRAY_1);
      if (NULL != aio_buf_mgr_array)
      {
        aio_buf_mgr_array->reset();
      }
      return ret;
    }

    int ObGlobalIndexHandler::cons_row_desc(ObRowDesc &desc)
    {
      int ret = OB_SUCCESS;
      int64_t key_count = 0; //rowkey column count
      int64_t column_count = 0; //total column count
      const ObSSTableSchemaColumnDef* col_def = NULL;
      if (OB_SUCCESS != (ret = get_sstable_schema().get_rowkey_column_count(table_id_,key_count)))
      {
        TBSYS_LOG(WARN, "get rowkey count failed,tid[%ld],ret[%d]", table_id_, ret);
      }
      else
      {
        column_count = get_sstable_schema().get_column_count();
      }
      desc.set_rowkey_cell_count(key_count);
      for (int64_t i = 0; i < column_count; i++)
      {
        if (NULL == (col_def = get_sstable_schema().get_column_def((int) i)))
        {
          TBSYS_LOG(WARN, "failed to get column def of table[%ld], i[%ld],ret[%d]",
                    table_id_, i, ret);
          break;
        }
        else if (OB_SUCCESS != (desc.add_column_desc(table_id_, col_def->column_name_id_)))
        {
          TBSYS_LOG(WARN, "failed to set column desc table_id[%ld], ret[%d]",
                    table_id_, ret);
          break;
        }
      }
      return ret;
    }

    int ObGlobalIndexHandler::cons_row_desc_without_virtual(ObRowDesc &desc)
    {
      int ret = OB_SUCCESS;
      const ObTableSchema* index_schema;
      const ObTableSchema* table_schema;
      const ObSSTableSchemaColumnDef* col_def = NULL;
      uint64_t max_data_table_cid = OB_INVALID_ID;
      index_schema = get_schema_mgr()->get_table_schema(table_id_);
      uint64_t data_tid = index_schema->get_original_table_id();
      table_schema = get_schema_mgr()->get_table_schema(data_tid);
      max_data_table_cid = table_schema->get_max_column_id();
      desc.set_rowkey_cell_count(index_schema->get_rowkey_info().get_size());
      for (int64_t i = 0; i < get_sstable_schema().get_column_count(); i++)
      {
        if (NULL == (col_def = get_sstable_schema().get_column_def((int) i)))
        {
          TBSYS_LOG(WARN,
                    "failed to get column def of table[%ld], i[%ld],ret[%d]",
                    table_id_, i, ret);
          break;
        }
        else if (col_def->column_name_id_ <= max_data_table_cid)
        {
          if (OB_SUCCESS
              != (desc.add_column_desc(table_id_, col_def->column_name_id_)))
          {
            TBSYS_LOG(WARN, "failed to set column desc table_id[%ld], ret[%d]",
                      table_id_, ret);
            break;
          }
        }
      }
      return ret;
    }

    /*
     * longfei:不管你上面给我什么样的range，这个函数负责的是把这个range所对应的table的所有的tablet上在这个range上的数据
     * 拿过来做一个排序！所以会分为两个部分：一部分是我这个cs上有这个range的数据，那么用封装的sstablescan去取数据；
     * 第二部分是在其他cs上的数据，通过rpc调用去其他cs上拿过来！
     */
    int ObGlobalIndexHandler::cons_global_index(ObNewRange* range)
    {
      int ret = OB_SUCCESS;
      const RangeServerHash* range_server = NULL;
      ObScanParam param;
      ObMergerSchemaManager *merge_schema_mgr = get_merge_schema_mgr();
      const ObSchemaManagerV2 *current_schema_manager = NULL;
      int64_t trailer_offset = 0;
      int64_t cur_sstab_sz = 0;
      if (NULL == get_handle_pool() || NULL == range || NULL == merge_schema_mgr)
      {
        TBSYS_LOG(ERROR, "some pointer is null,invalid argument");
        ret = OB_ERROR;
      }
      else if (NULL == (current_schema_manager = merge_schema_mgr->get_schema(0)))
      {
        TBSYS_LOG(ERROR, "SchemaManagerV2 is NULL pointer!");
        ret = OB_ERROR;
      }
      else
      {
        set_schema_mgr(current_schema_manager);
        set_new_range(*range);
        table_id_ = range->table_id_;
      }

      if (OB_SUCCESS == ret)
      {
        //首先创建空的sstable
        int32_t disk_no = get_tablet_mgr()->get_disk_manager().get_dest_disk();
        if (OB_SUCCESS != (ret = create_new_sstable(get_new_range().table_id_, disk_no)))
        {
          TBSYS_LOG(WARN, "create new sstable for table[%ld] failed", get_new_range().table_id_);
        }
        else if (NULL == (range_server = get_handle_pool()->get_range_info()))
        {
          TBSYS_LOG(WARN, "failed to get range server info");
          ret = OB_ERROR;
        }
        else
        {
          TBSYS_LOG(INFO, "create global index sstable success,table[%ld]",get_new_range().table_id_);
          set_disk_no(disk_no);
        }
      }

      if (OB_SUCCESS == ret)
      {
        (*ms_wrapper_.get_cs_interactive_cell_stream()).set_self(THE_CHUNK_SERVER.get_self());
        if (OB_SUCCESS != (ret = fill_scan_param(param)))
        {
          TBSYS_LOG(WARN, "fill scan param for index failed,ret[%d]", ret);
        }
        //@TODO: start_agent() --> agent's open()
        else if (OB_SUCCESS != (ret = interactive_agent_.start_agent(param, *ms_wrapper_.get_cs_interactive_cell_stream(), range_server)))
        {
          if(OB_ITER_END == ret)
          {
            //query_agent_.set_not_used();
          }
          else
          {
            TBSYS_LOG(WARN, "start query agent failed,ret[%d]", ret);
          }
        }
      }

      if (OB_SUCCESS == ret || OB_ITER_END == ret)
      {
        if (OB_SUCCESS != (ret = local_agent_.set_scan_param(&param)))
        {
          TBSYS_LOG(WARN,"local agent set scan param failed!ret[%d]",ret);
        }
        else if (OB_SUCCESS != (ret = write_global_index()))
        {
          TBSYS_LOG(WARN, "write total index failed,ret[%d]", ret);
        }
        else
        {
          interactive_agent_.stop_agent();
        }
      }

      if (NULL != get_schema_mgr())
      {
        if (OB_SUCCESS != (get_merge_schema_mgr()->release_schema(get_schema_mgr())))
        {
          TBSYS_LOG(WARN, "release schema failed,ret = [%d]", ret);

        }
        else
        {
          set_schema_mgr(NULL);
        }
      }
      //must close sstable!
      cur_sstab_sz = get_cur_sstable_size();
      get_sstable_writer().close_sstable(trailer_offset, cur_sstab_sz);
      return ret;
    }

    int ObGlobalIndexHandler::calc_tablet_col_checksum_index(const ObRow& row,
                                                             ObRowDesc desc, char *column_checksum, int64_t tid)
    {

      int ret = OB_SUCCESS;
      int64_t row_desc_count = 0;
      const ObRowDesc::Desc *row_desc_idx = desc.get_cells_desc_array(row_desc_count);
      int pos = 0, len = 0;
      const ObObj* obj = NULL;
      ObBatchChecksum bc;
      for (int64_t i = 0; i < row_desc_count; i++)
      {
        row.get_cell(tid, row_desc_idx[i].column_id_, obj);
        if (obj == NULL)
        {
          ret = OB_ERROR;
          TBSYS_LOG(ERROR, "get sstable row obj error,ret=%d", ret);
          break;
        }
        else
        {
          bc.reset();
          obj->checksum(bc);
          len = snprintf(column_checksum + pos,
                         OB_MAX_COL_CHECKSUM_STR_LEN - 1 - pos, "%ld", row_desc_idx[i].column_id_);
          if (len < 0)
          {
            TBSYS_LOG(ERROR, "write column checksum error");
            ret = OB_ERROR;
            break;
          }
          else
          {
            pos += len;
            column_checksum[pos++] = ':';
            if (pos < OB_MAX_COL_CHECKSUM_STR_LEN - 1)
            {
              len = snprintf(column_checksum + pos,
                             OB_MAX_COL_CHECKSUM_STR_LEN - 1 - pos, "%lu", bc.calc());
              pos += len;
            }
            if (i != row_desc_count - 1)
            {
              column_checksum[pos++] = ',';
            }
            else
            {
              column_checksum[pos++] = '\0';
            }
          }
        }
      }
      return ret;
    }

    int ObGlobalIndexHandler::construct_index_tablet_info(ObTablet* tablet)
    {
      int ret = OB_SUCCESS;
      if (NULL != tablet && get_tablet_mgr() != NULL)
      {
        const ObSSTableTrailer* trailer = NULL;
        trailer = &get_sstable_writer().get_trailer();
        tablet->set_disk_no(get_disk_no());
        tablet->set_data_version(get_frozen_version());
        set_new_range(tablet->get_range());
        if (NULL == trailer)
        {
          TBSYS_LOG(ERROR, "null pointor of construct index tablet trailer");
          ret = OB_ERROR;
        }
        else if (OB_SUCCESS != (ret = tablet->add_sstable_by_id(get_sstable_id())))
        {
          TBSYS_LOG(ERROR, "STATIC INDEX : add sstable to tablet failed.");
        }
        else
        {
          get_tablet_extend_info().occupy_size_ = get_cur_sstable_size();
          get_tablet_extend_info().row_count_ = trailer->get_row_count();
          //get_tablet_extend_info().row_checksum_=get_sstable_writer().get_row_checksum();
          get_tablet_extend_info().check_sum_ =
              get_sstable_writer().get_trailer().get_sstable_checksum();
          get_tablet_extend_info().last_do_expire_version_ =
              get_tablet_mgr()->get_serving_data_version(); //todo @haixing same as data table
          get_tablet_extend_info().sequence_num_ = 0;            //todo checksum
        }
        if (OB_SUCCESS == ret)
        {
          tablet->set_extend_info(get_tablet_extend_info());
        }
        if (OB_SUCCESS == ret)
        {
          get_tablet_mgr()->get_disk_manager().add_used_space(
                (get_sstable_id().sstable_file_id_ & DISK_NO_MASK),
                get_cur_sstable_size(), false);
        }
      }
      else
      {
        TBSYS_LOG(ERROR, "index image tablet is null");
        ret = OB_ERROR;
      }
      return ret;
    }

    //add longfei [cons static index] 151207:b
    int ObGlobalIndexHandler::check_tablet(const ObTablet* tablet)
    {
      int ret = OB_SUCCESS;
      if (NULL == tablet)
      {
        TBSYS_LOG(WARN, "tablet is NULL");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (!get_tablet_mgr()->get_disk_manager().is_disk_avail(
                 tablet->get_disk_no()))
      {
        TBSYS_LOG(WARN, "tablet:%s locate on bad disk:%d, sstable_id:%ld",
                  to_cstring(tablet->get_range()), tablet->get_disk_no(),
                  (tablet->get_sstable_id_list()).count() > 0 ?
                    (tablet->get_sstable_id_list()).at(0).sstable_file_id_ : 0);
        ret = OB_ERROR;
      }
      //check sstable path where exist
      else if ((tablet->get_sstable_id_list()).count() > 0)
      {
        int64_t sstable_id = 0;
        char path[OB_MAX_FILE_NAME_LENGTH];
        path[0] = '\0';
        sstable_id = (tablet->get_sstable_id_list()).at(0).sstable_file_id_;

        if (OB_SUCCESS
            != (ret = get_sstable_path(sstable_id, path, sizeof(path))))
        {
          TBSYS_LOG(WARN,
                    "can't get the path of sstable, tablet:%s sstable_id:%ld",
                    to_cstring(tablet->get_range()), sstable_id);
        }
        else if (false == FileDirectoryUtils::exists(path))
        {
          TBSYS_LOG(WARN,
                    "tablet:%s sstable file is not exist, path:%s, sstable_id:%ld",
                    to_cstring(tablet->get_range()), path, sstable_id);
          ret = OB_FILE_NOT_EXIST;
        }
      }
      return ret;
    }
    //add e

    int ObGlobalIndexHandler::get_failed_fake_range(ObNewRange &range)
    {
      return interactive_agent_.get_failed_fake_range(range);
    }

    int ObGlobalIndexHandler::update_meta_index(ObTablet* tablet, const bool sync_meta)
    {
      int ret = OB_SUCCESS;
      if (tablet == NULL)
      {
        TBSYS_LOG(WARN, "update meta index error, null pointor of tablet");
        ret = OB_ERROR;
      }
      if (get_tablet_mgr() == NULL)
      {
        TBSYS_LOG(WARN, "update meta index error, null pointor of tabletmanager");
        ret = OB_ERROR;
      }
      if (OB_SUCCESS == ret)
      {
        ObMultiVersionTabletImage& tablet_image = get_tablet_mgr()->get_serving_tablet_image();
        if (OB_SUCCESS == ret)
        {
          ret = check_tablet(tablet);
        }

        if (OB_SUCCESS == ret)
        {
          if (OB_SUCCESS != (ret = tablet_image.upgrade_index_tablet(tablet, false)))
          {
            TBSYS_LOG(WARN, "upgrade new index tablets error [%d]", ret);
          }
          else
          {
            if (sync_meta)
            {
              // sync new index tablet meta files;

              if (OB_SUCCESS != (ret = tablet_image.write(tablet->get_data_version(),
                                                          tablet->get_disk_no())))
              {
                TBSYS_LOG(WARN,
                          "write new index meta failed , version=%ld, disk_no=%d",
                          tablet->get_data_version(), tablet->get_disk_no());
              }
            }
          }
        }
      }
      return ret;
    }

  } /* namespace chunkserver */
} /* namespace oceanbase */
