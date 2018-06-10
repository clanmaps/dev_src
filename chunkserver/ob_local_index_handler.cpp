/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_local_index_handler.cpp
* @brief for partitional index
*
* Created by maoxiaoxiao:write partitional index , calculate column checksum of data table and report index histogram info
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#include "ob_local_index_handler.h"
#include "ob_index_handle_pool.h"
#include "ob_disk_manager.h"
#include "ob_tablet_manager.h"
#include "common/file_directory_utils.h"

namespace oceanbase
{
  namespace chunkserver
  {
    ObLocalIndexHandler::ObLocalIndexHandler(
        ObIndexHandlePool *pool,
        common::ObMergerSchemaManager *schema_mgr,
        ObTabletManager* tablet_mgr,
        ObTabletHistogramReportInfoList* report_info_list):new_table_schema_(NULL)
    {
      handle_pool_ = pool;
      merger_schema_manager_ = schema_mgr;
      tablet_manager_ = tablet_mgr;
      //index_reporter_.init(tablet_mgr);
      index_reporter_.get_tablet_histogram()->set_allocator(&allocator_);
      index_reporter_.get_tablet_histogram_report_info_list() = report_info_list;


    }

    ObLocalIndexHandler::~ObLocalIndexHandler()
    {

    }

    int ObLocalIndexHandler::create_new_sstable(const uint64_t table_id, const int32_t disk_no)
    {
      int ret = OB_SUCCESS;
      sstable_id_.sstable_file_id_ = tablet_manager_->allocate_sstable_file_seq();
      sstable_id_.sstable_file_offset_ = 0;
      int64_t sstable_block_size = OB_DEFAULT_SSTABLE_BLOCK_SIZE;
      bool sstable_exist_flag = false;
      sstable_schema_.reset();
      if(disk_no < 0)
      {
        TBSYS_LOG(ERROR, "does't have enough disk space for static index");
        sstable_id_.sstable_file_id_ = 0;
        ret = OB_CS_OUTOF_DISK_SPACE;
      }
      if(NULL == schema_manager_)
      {
        TBSYS_LOG(ERROR,"faild to get schema manager!");
        ret = OB_SCHEMA_ERROR;
      }
      else if(NULL == (new_table_schema_ = schema_manager_->get_table_schema(table_id)))
      {
        TBSYS_LOG(ERROR, "failed to get table[%ld] schema",table_id);
        ret = OB_SCHEMA_ERROR;
      }
      else if(OB_SUCCESS != (ret = build_sstable_schema(table_id, *schema_manager_, sstable_schema_, false)))
      {
        TBSYS_LOG(ERROR,"fill sstable schema error");
      }
      else
      {
        if (new_table_schema_->get_block_size() > 0 && 64 != new_table_schema_->get_block_size())
        {
          sstable_block_size = new_table_schema_->get_block_size();
        }
      }
      if (OB_SUCCESS == ret)
      {
        do
        {
          sstable_id_.sstable_file_id_ = (sstable_id_.sstable_file_id_ << 8) | (disk_no & 0xff);
          if ((OB_SUCCESS == ret) && (OB_SUCCESS != (ret = get_sstable_path(sstable_id_, sstable_path_, sizeof(sstable_path_)))))
          {
            TBSYS_LOG(ERROR,"Create Index : can't get the path of new sstable");
            ret = OB_ERROR;
          }
          if (OB_SUCCESS == ret)
          {
            sstable_exist_flag = FileDirectoryUtils::exists(sstable_path_);
            if (sstable_exist_flag)
            {
              sstable_id_.sstable_file_id_ = tablet_manager_->allocate_sstable_file_seq();
            }
          }
        }while (OB_SUCCESS == ret && sstable_exist_flag);
        if (OB_SUCCESS == ret)
        {
          ObString path_string_;
          ObString compress_string_;
          int64_t element_count = DEFAULT_ESTIMATE_ROW_COUNT;
          path_string_.assign_ptr(sstable_path_, static_cast<int32_t>(strlen(sstable_path_) + 1));
          compress_string_.assign_ptr(const_cast<char*>(new_table_schema_->get_compress_func_name()), static_cast<int32_t>(sizeof(new_table_schema_->get_compress_func_name()) + 1));
          if (OB_SUCCESS != (ret = sstable_writer_.create_sstable(sstable_schema_, path_string_, compress_string_, frozen_version_, OB_SSTABLE_STORE_DENSE, sstable_block_size, element_count)))
          {
            if (OB_IO_ERROR == ret)
              tablet_manager_->get_disk_manager().set_disk_status(disk_no, DISK_ERROR);
            TBSYS_LOG(ERROR, "Merge : create sstable failed : [%d]",ret);
          }
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::cons_index_data_row_desc(ObRowDesc &index_data_row_desc, uint64_t data_tid, uint64_t index_tid)
    {
      int ret = OB_SUCCESS;

      uint64_t index_cid = OB_INVALID_ID;
      const ObTableSchema *table_schema = schema_manager_->get_table_schema(data_tid);
      const ObTableSchema *index_table_schema = schema_manager_->get_table_schema(index_tid);
      const ObRowkeyInfo idx_ori = index_table_schema->get_rowkey_info();
      index_data_row_desc.set_rowkey_cell_count(idx_ori.get_size());
      for(int64_t i = 0; i < idx_ori.get_size(); i++)
      {
        if(OB_SUCCESS != (ret = idx_ori.get_column_id(i, index_cid)))
        {
          TBSYS_LOG(WARN,"get_column_id for rowkey failed!");
          ret = OB_ERROR;
          break;
        }
        else
        {
          if(OB_SUCCESS != (ret = index_data_row_desc.add_column_desc(index_tid, index_cid)))
          {
            TBSYS_LOG(WARN,"index_data_row_desc.add_column_desc occur an error,ret[%d]", ret);
          }
        }
      }
      bool column_hit_index_flag = false;
      for(int j = OB_APP_MIN_COLUMN_ID; j <= (int64_t)(table_schema->get_max_column_id()); j++)
      {
	    /*mod maoxx
        const ObColumnSchemaV2* idx_ocs = schema_manager_->get_column_schema(index_tid, j);
        if(idx_ori.is_rowkey_column(j) || NULL == idx_ocs)
        {

        }
        else
        {
          index_cid = idx_ocs->get_id();
          if(OB_SUCCESS != (ret = index_data_row_desc.add_column_desc(index_tid, index_cid)))
          {
            TBSYS_LOG(ERROR,"error in add_column_desc");
            break;
          }
		}
      mod e*/
		if(!idx_ori.is_rowkey_column(j))
        {
            if(OB_SUCCESS != (ret = schema_manager_->column_hit_index(data_tid, j, column_hit_index_flag)))
            {
              TBSYS_LOG(ERROR,"failed to check if hint index,ret=%d",ret);
              break;
            }
            else if(column_hit_index_flag)
            {
              if(OB_SUCCESS != (ret = index_data_row_desc.add_column_desc(index_tid, j)))
              {
                TBSYS_LOG(ERROR,"error in add_column_desc");
                break;
              }
            }
		}
      }
      if(OB_SUCCESS == ret && schema_manager_->is_index_has_storing(index_tid))
      {
        ret = index_data_row_desc.add_column_desc(index_tid, OB_INDEX_VIRTUAL_COLUMN_ID);
      }
      return ret;
    }

    int ObLocalIndexHandler::push_cid_in_desc_and_ophy(uint64_t data_tid, uint64_t index_tid, const ObRowDesc index_data_row_desc, ObArray<uint64_t> &basic_columns, ObRowDesc &desc)
    {
      int ret = OB_SUCCESS;
      uint64_t tid = OB_INVALID_ID;
      uint64_t cid = OB_INVALID_ID;
      desc.set_rowkey_cell_count(index_data_row_desc.get_rowkey_cell_count());
      for(int64_t i = 0; i < index_data_row_desc.get_rowkey_cell_count(); i++)
      {
        if(OB_SUCCESS != (ret = index_data_row_desc.get_tid_cid(i, tid, cid)))
        {
          TBSYS_LOG(WARN,"get column schema failed,idx[%ld] ret[%d]", i, ret);
          break;
        }
        else if(OB_SUCCESS != (ret = basic_columns.push_back(cid)))
        {
          TBSYS_LOG(WARN, "push back basic columns failed, ret = %d", ret);
          break;
        }
        else if(OB_SUCCESS != (ret = sort_.add_sort_column(data_tid, cid, true)))
        {
          TBSYS_LOG(WARN, "add sort column failed ,data_tid[%ld],cid[%ld], ret = %d", data_tid, cid, ret);
          break;
        }
        else if(OB_SUCCESS != (ret = desc.add_column_desc(data_tid, cid)))
        {
          TBSYS_LOG(WARN, "add desc column failed ,data_tid[%ld],cid[%ld], ret = %d", data_tid, cid, ret);
          break;
        }
      }
      for(int64_t j = index_data_row_desc.get_rowkey_cell_count(); j < index_data_row_desc.get_column_num(); j++)
      {
        if(OB_SUCCESS != (ret = index_data_row_desc.get_tid_cid(j, tid, cid)))
        {
          TBSYS_LOG(WARN,"get column schema failed,cid[%ld]", cid);
          break;
        }
        else if(OB_SUCCESS != (ret = basic_columns.push_back(cid)))
        {
          TBSYS_LOG(WARN, "push back basic columns failed, ret = %d", ret);
          break;
        }
        else if(OB_SUCCESS != (ret = sort_.add_sort_column(data_tid, cid, true)))
        {
          TBSYS_LOG(WARN, "add sort column failed ,data_tid[%ld],cid[%ld], ret = %d", data_tid, cid, ret);
          break;
        }
        else if(NULL != (schema_manager_->get_column_schema(index_tid, cid)))
        {
          if(OB_SUCCESS != (ret = desc.add_column_desc(data_tid, cid)))
          {
            TBSYS_LOG(WARN, "add desc column failed ,data_tid[%ld],cid[%ld], ret = %d", data_tid, cid, ret);
            break;
          }
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::trans_row_to_sstrow(ObRowDesc &row_desc, const ObRow &row)
    {
      int ret = OB_SUCCESS;
      uint64_t tid = OB_INVALID_ID;
      uint64_t cid = OB_INVALID_ID;
      const ObObj *obj = NULL;
      if(NULL == new_table_schema_)
      {
        TBSYS_LOG(WARN, "null pointer of table schema");
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        sst_row_.clear();
        sst_row_.set_rowkey_obj_count(row_desc.get_rowkey_cell_count());
      }
      for(int64_t i = 0; OB_SUCCESS == ret && i < row_desc.get_column_num();i++)
      {
        if(OB_SUCCESS != (ret = row_desc.get_tid_cid(i, tid, cid)))
        {
          TBSYS_LOG(WARN,"failed in get tid_cid from row desc,ret[%d]", ret);
          break;
        }
        else if(OB_INDEX_VIRTUAL_COLUMN_ID == cid)
        {
          ObObj vi_obj;
          vi_obj.set_null();
          ret = sst_row_.add_obj(vi_obj);
        }
        /*
        else if(NULL == schema_manager_ || OB_INVALID_ID == (data_tid =  schema_manager_->get_table_schema(tid)->get_original_table_id()))
        {
          TBSYS_LOG(WARN,"get original table id failed.data_tid[%ld].",data_tid);
        }
        */
        else if(OB_SUCCESS != (ret = row.get_cell(tid, cid, obj)))
        {
          TBSYS_LOG(WARN, "get cell from obrow failed!tid[%ld],cid[%ld],ret[%d]", tid, cid, ret);
          break;
        }
        if(OB_SUCCESS == ret && OB_INDEX_VIRTUAL_COLUMN_ID != cid)
        {
          if(NULL == obj)
          {
            TBSYS_LOG(WARN, "obj pointer cannot be NULL!");
            ret = OB_INVALID_ARGUMENT;
            break;
          }
          else if(OB_SUCCESS != (ret = sst_row_.add_obj(*obj)))
          {
            TBSYS_LOG(WARN, "set cell for index row failed,ret = %d",ret);
            break;
          }
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::cons_column_checksum_row_desc_for_data(ObRowDesc &column_checksum_row_desc, uint64_t tid)
    {
      int ret = OB_SUCCESS;
      bool column_hit_index_flag = false;
      const ObTableSchema *table_schema = schema_manager_->get_table_schema(tid);
      for(uint64_t cid = OB_APP_MIN_COLUMN_ID; cid <= table_schema->get_max_column_id(); cid++)
      {
        if(OB_SUCCESS != (ret = schema_manager_->column_hit_index(tid, cid, column_hit_index_flag)))
        {
          TBSYS_LOG(ERROR,"failed to check if hint index,ret=%d",ret);
          break;
        }
        else if(column_hit_index_flag)
        {
          ret = column_checksum_row_desc.add_column_desc(tid, cid);
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::calc_column_checksum_for_data(const ObRow& row, ObRowDesc &column_checksum_row_desc, char *column_checksum, uint64_t tid)
    {
      int ret = OB_SUCCESS;
      int pos = 0, len = 0;
      const ObObj* obj = NULL;
      int64_t row_desc_count = 0;
      const ObRowDesc::Desc *row_desc_idx = column_checksum_row_desc.get_cells_desc_array(row_desc_count);
      ObBatchChecksum bc;

      for(int64_t i = 0; i < row_desc_count; i++)
      {
        if(OB_SUCCESS != (ret = row.get_cell(tid, row_desc_idx[i].column_id_, obj)))
        {
          ret = OB_ERROR;
          TBSYS_LOG(ERROR, "get sstable row obj error,ret=%d", ret);
          break;
        }
        else if(obj == NULL)
        {
          ret = OB_ERROR;
          TBSYS_LOG(ERROR, "get sstable row obj error,ret=%d", ret);
          break;
        }
        else
        {
          bc.reset();
          obj->checksum(bc);
        }
        len = snprintf(column_checksum + pos, OB_MAX_COL_CHECKSUM_STR_LEN - 1 - pos, "%ld", row_desc_idx[i].column_id_);
        if(len < 0)
        {
          TBSYS_LOG(ERROR,"write column checksum error");
          ret = OB_ERROR;
          break;
        }
        else
        {
          pos += len;
          column_checksum[pos++] = ':';
          if(pos < OB_MAX_COL_CHECKSUM_STR_LEN - 1)
          {
            len = snprintf(column_checksum + pos,OB_MAX_COL_CHECKSUM_STR_LEN - 1 - pos, "%lu", bc.calc());
            pos += len;
          }
          if(i != (row_desc_count - 1))
          {
            column_checksum[pos++] = ',';
          }
          else
          {
            column_checksum[pos++] = '\0';
          }
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::save_current_row()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != (ret = sstable_writer_.append_row(sst_row_, sstable_size_)))
      {
        TBSYS_LOG(ERROR, "write_index : append row failed [%d], this row_, obj count:%ld, "
                         "table:%lu, group:%lu",
                  ret, sst_row_.get_obj_count(), sst_row_.get_table_id(), sst_row_.get_column_group_id());
        for(int32_t i = 0; i < sst_row_.get_obj_count(); i++)
        {
          sst_row_.get_obj(i)->dump(TBSYS_LOG_LEVEL_ERROR);
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::write_partitional_index(
        ObNewRange range,
        uint64_t index_tid,
        int32_t disk_no,
        int64_t tablet_row_count,
        int64_t sample_rate)
    {
      int ret = OB_SUCCESS;
      uint64_t data_tid = range.table_id_;
      int64_t row_in_sample_rate = 1;
      int64_t temp_tablet_row_count = 1;
      ObObj objs[OB_MAX_ROWKEY_COLUMN_NUMBER];
      ObRowDesc index_data_row_desc;
      ObRowDesc desc;
      ObNewRange sample_range;
      ObTabletSample index_sample;
      ObArray<uint64_t> basic_columns;
      ObTabletScan tablet_scan;
      ObSSTableScan sstable_scan;
      ObSqlScanParam sql_scan_param;
      ObSSTableScanParam sstable_scan_param;
      ScanContext sc;
      tablet_manager_->build_scan_context(sc);
      sort_.set_child(0, sstable_scan);
      ObColumnChecksum row_column_checksum;
      ObRowDesc column_checksum_row_desc;
      int64_t trailer_offset = 0;

      ///计算采样频率
      //debugb longfei 2016-03-18 14:39:35
      //TBSYS_LOG(WARN, "debug::longfei>>>table_row_count[%ld],sample_rate[%ld]", tablet_row_count, sample_rate);
      //debuge
      const int64_t MAX_SAMPLE_BUCKET = 256; //add longfei 2016-03-18 14:53:14 :e

      sample_rate = (sample_rate > MAX_SAMPLE_BUCKET) ? MAX_SAMPLE_BUCKET : sample_rate;//add longfei 2016-03-18 18:32:52 :e
      if(0 == tablet_row_count % sample_rate)
      {
        sample_rate = tablet_row_count/sample_rate;
      }
      else
      {
        sample_rate = tablet_row_count/sample_rate + 1;
      }

      if(OB_SUCCESS != (ret = create_new_sstable(index_tid, disk_no)))
      {
        TBSYS_LOG(WARN,
                  "failed to create new sstable for index[%ld], disk_no[%d], ret = %d",
                  index_tid, disk_no, ret);
      }
      else
      {
        TBSYS_LOG(INFO,
                  "create new sstable for index[%ld], disk_no[%d], ret = %d",
                  index_tid, disk_no, ret);
        basic_columns.clear();
        sst_row_.set_column_group_id(0);
        sst_row_.set_table_id(index_tid);

        const ObTableSchema* index_table_schema = schema_manager_->get_table_schema(index_tid);
        const ObRowkeyInfo* index_rowkey_info = &(index_table_schema->get_rowkey_info());
        if(NULL == index_table_schema || NULL == new_table_schema_)
        {
          TBSYS_LOG(WARN, "index_table_schema or new_table_schema_ error, null pointer!");
          ret = OB_SCHEMA_ERROR;
        }
        else if(OB_SUCCESS != (ret = sql_scan_param.set_range(range)))
        {
          TBSYS_LOG(WARN, "set ob_sql_param_range failed, ret= %d, range = %s", ret, to_cstring(range));
        }
        else
        {
          sql_scan_param.set_is_result_cached(false);
          sample_range.table_id_ = index_tid;
          sample_range.border_flag_.set_inclusive_start();
          sample_range.border_flag_.set_inclusive_end();

          if(OB_SUCCESS != (ret = cons_index_data_row_desc(index_data_row_desc, data_tid, index_tid)))
          {
            TBSYS_LOG(WARN, "construct row desc failed,ret = [%d]", ret);
          }
          else if(OB_SUCCESS != (ret = push_cid_in_desc_and_ophy(data_tid, index_tid, index_data_row_desc, basic_columns, desc)))
          {
            TBSYS_LOG(WARN, "push cid indesc failed,data_tid[%ld], index_data_row_desc = %s, desc = %s, ret = %d", data_tid, to_cstring(index_data_row_desc), to_cstring(desc), ret);
          }
          else if(OB_SUCCESS != (ret = tablet_scan.build_sstable_scan_param_pub(basic_columns, sql_scan_param, sstable_scan_param)))
          {
            TBSYS_LOG(WARN,"failed to build sstable scan param");
          }
          else if(OB_SUCCESS != (ret = sstable_scan.open_scan_context(sstable_scan_param, sc)))
          {
            TBSYS_LOG(WARN,"failed to open scan context");
          }
          else if(OB_SUCCESS != (ret = sort_.open()))
          {
            TBSYS_LOG(WARN,"failed to open ObSsTableScan!");
          }
          else if(OB_SUCCESS != (ret = cons_column_checksum_row_desc_for_data(column_checksum_row_desc, data_tid)))
          {
            TBSYS_LOG(WARN, "construct row desc failed, ret = [%d]", ret);
          }
          else
          {
            if(0 == tablet_row_count)
            {
              index_sample.range.table_id_ = index_tid;
              index_sample.row_count = 0;
              index_sample.range.start_key_.set_min_row();
              index_sample.range.end_key_.set_max_row();
              if(OB_SUCCESS != (ret = index_reporter_.get_tablet_histogram()->add_sample_with_deep_copy(index_sample)))
              {
                TBSYS_LOG(WARN,"fail to add sample to histogram =%d",ret);
              }
            }
            else
            {
              const ObRow *row = NULL;
              while(OB_SUCCESS == ret && OB_SUCCESS == (ret = sort_.get_next_row(row)))
              {
                if(NULL != row)
                {
                  if(OB_SUCCESS != (ret = trans_row_to_sstrow(desc, *row)))
                  {
                    TBSYS_LOG(WARN,"failed to trans row to sstable row, ret = %d", ret);
                    break;
                  }
                  else
                  {
                    if(1 == row_in_sample_rate || temp_tablet_row_count == tablet_row_count)
                    {
                      if(1 == row_in_sample_rate)
                      {
                        for(int64_t i = 0; i < index_rowkey_info->get_size(); i++)
                        {
                          if(OB_SUCCESS != (ret = ob_write_obj(allocator_, sst_row_.get_row_obj(i), objs[i] )))
                          {
                            TBSYS_LOG(ERROR,"write obj faield,ret [%d]",ret);
                            break;
                          }
                        }
                        sample_range.start_key_.assign(objs, index_rowkey_info->get_size());
                      }
                      if(temp_tablet_row_count == tablet_row_count)
                      {
                        sample_range.end_key_.assign(sst_row_.get_obj_without_const(0), index_rowkey_info->get_size());
                        index_sample.range= sample_range;
                        index_sample.row_count= row_in_sample_rate;
                        if(OB_SUCCESS != (ret = index_reporter_.get_tablet_histogram()->add_sample_with_deep_copy(index_sample)))
                        {
                          TBSYS_LOG(WARN,"fail to add sample to histogram =%d",ret);
                          break;
                        }
                      }
                      row_in_sample_rate++;
                      temp_tablet_row_count++;
                    }
                    else if(row_in_sample_rate == sample_rate && temp_tablet_row_count != tablet_row_count)
                    {
                      sample_range.end_key_.assign(sst_row_.get_obj_without_const(0), index_rowkey_info->get_size());
                      index_sample.range = sample_range;
                      index_sample.row_count = row_in_sample_rate;
                      if(OB_SUCCESS != (ret = index_reporter_.get_tablet_histogram()->add_sample_with_deep_copy(index_sample)))
                      {
                        TBSYS_LOG(WARN,"fail to add sample to histogram =%d", ret);
                        break;
                      }
                      temp_tablet_row_count++;
                      row_in_sample_rate = 1;
                    }
                    else
                    {
                      row_in_sample_rate++;
                      temp_tablet_row_count++;
                    }
                  }
                  if(OB_SUCCESS == ret)
                  {
                    if(OB_SUCCESS != (ret = calc_column_checksum_for_data(*row, column_checksum_row_desc, row_column_checksum.get_str(), data_tid)))
                    {
                      TBSYS_LOG(ERROR,"fail to calculate data table column checksum index =%d", ret);
                      break;
                    }
                    else if(OB_SUCCESS != (ret = column_checksum_.add(row_column_checksum)))
                    {
                      TBSYS_LOG(ERROR,"checksum sum error =%d", ret);
                      break;
                    }
                    row_column_checksum.reset();
                  }
                  if(OB_SUCCESS == ret)
                  {
                    if(OB_SUCCESS != (ret = save_current_row()))
                    {
                      TBSYS_LOG(ERROR,"write row error,ret=%d", ret);
                      break;
                    }
                  }
                }
              }//end while
            }
            if(OB_ITER_END == ret)
              ret = OB_SUCCESS;
            if(OB_SUCCESS == ret)
            {
              (index_reporter_.get_tablet_histogram_report_info())->static_index_histogram = *(index_reporter_.get_tablet_histogram());

              ObMultiVersionTabletImage& tablet_image = tablet_manager_->get_serving_tablet_image();
              frozen_version_ = tablet_image.get_serving_version();
              if(OB_SUCCESS != (ret = tablet_manager_->send_tablet_column_checksum(column_checksum_, new_range_, frozen_version_)))
              {
                TBSYS_LOG(ERROR,"send tablet column checksum failed =%d", ret);
              }
              column_checksum_.reset();
            }
          }
        }
      }
      if(OB_SUCCESS != sstable_scan.close())
      {
        TBSYS_LOG(WARN, "close sstable scan failed!");
      }
      sstable_writer_.close_sstable(trailer_offset,sstable_size_);
      sort_.reset();
      return ret;
    }

    int ObLocalIndexHandler::cons_local_index(ObTablet *tablet, int64_t sample_rate)
    {
      uint64_t tablet_row_count = 0;
      int ret = OB_SUCCESS;
      int32_t disk_no = -1;
      if(NULL == tablet)
      {
        TBSYS_LOG(ERROR, "tablet pointer is null");
        ret = OB_INVALID_ARGUMENT;
      }
      else if(NULL == merger_schema_manager_)
      {
        TBSYS_LOG(ERROR, "MergeSchemaManager pointer is NULL");
        ret = OB_INVALID_ARGUMENT;
      }
      else if(NULL == (schema_manager_ = merger_schema_manager_->get_schema(0)))//@param:version
      {
        TBSYS_LOG(ERROR, "get null schema manager pointer!");
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        if(OB_SUCCESS != (ret = (tablet_manager_->fill_tablet_histogram_info(
                                   *tablet,
                                   *(index_reporter_.get_tablet_histogram_report_info())))))
        {
          TBSYS_LOG(WARN,"fill tablet histogram info failed.");
        }
        tablet_row_count = tablet->get_row_count();
        new_range_ = tablet->get_range();
        disk_no = tablet->get_disk_no();
        TBSYS_LOG(INFO,"get tablet range[%s] to build index", to_cstring(new_range_));
        uint64_t table_id = handle_pool_->get_process_tid();
        if(OB_SUCCESS != (ret = write_partitional_index(new_range_,
                                                        table_id,
                                                        disk_no,
                                                        tablet_row_count,
                                                        sample_rate)))
        {
          TBSYS_LOG(ERROR, "gen static index test error,ret[%d]", ret);
        }
        //将创建的sstable加入到原表的tablet当中
        else if(OB_SUCCESS != (ret = tablet->add_local_index_sstable_by_id(sstable_id_)))
        {
          TBSYS_LOG(ERROR, "add sstable_id[%ld] in tablet error", sstable_id_.sstable_file_id_);
        }
        if (OB_LIKELY(OB_SUCCESS == ret))
        {
          TBSYS_LOG(INFO, "build local index sstable[%ld] successed ! Add in tablet;", sstable_id_.sstable_file_id_);
        }
      }
      if(NULL != schema_manager_)
      {
        if(OB_LIKELY(OB_SUCCESS == merger_schema_manager_->release_schema(schema_manager_)))
        {
          schema_manager_ = NULL;
        }
      }
      return ret;
    }

    int ObLocalIndexHandler::start()
    {
      int ret = OB_SUCCESS;

      if (OB_SUCCESS != (ret = cons_local_index(tablet_, sample_rate_)))
      {
        TBSYS_LOG(ERROR, "local index construction failed");
      }

      return ret;
    }

  }
}
