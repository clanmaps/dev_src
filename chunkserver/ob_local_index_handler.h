/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_local_index_handler.h
* @brief for partitional index
*
* Created by maoxiaoxiao:write partitional index , calculate column checksum of data table and report index histogram info
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OBLOCALINDEXHANDLER_H
#define OBLOCALINDEXHANDLER_H

//#include "ob_tablet_manager.h"
#include "common/ob_range2.h"
#include "common/ob_column_checksum.h"
#include "sstable/ob_disk_path.h"
#include "sstable/ob_sstable_writer.h"
#include "sql/ob_sort.h"
#include "sql/ob_tablet_scan.h"
#include "sql/ob_sstable_scan.h"
#include "sql/ob_sql_scan_param.h"
#include "ob_index_reporter.h"
//#include "ob_index_handle_pool.h" //del longfei:解决重复引用编译问题

namespace oceanbase
{
  namespace chunkserver
  {
    class ObTabletManager;
    class ObIndexHandlePool;
    using namespace common;
    using namespace sql;
    using namespace sstable;

    /**
     * @brief The ObLocalIndexHandler class
     * ObLocalIndexHandler is designed for
     * build partitional index
     */
    class ObLocalIndexHandler
    {
      static const int64_t DEFAULT_ESTIMATE_ROW_COUNT = 256*1024LL;
      static const int64_t DEFAULT_MEMORY_LIMIT = 256*1024*1024LL;
    public:

      /**
       * @brief constructor
       * @param pool need ObIndexHandlePool*
       * @param schema_mgr
       * @param tablet_mgr
       * @param report_info_list
       */
      ObLocalIndexHandler(
          ObIndexHandlePool *pool,
          common::ObMergerSchemaManager *schema_mgr,
          ObTabletManager* tablet_mgr,
          ObTabletHistogramReportInfoList* report_info_list);

      /**
       * @brief destructor
       */
      ~ObLocalIndexHandler();

      /**
       * @brief set_tablet
       * @param tablet
       * @return OB_SUCCESS or other ERROR
       */
      inline int set_tablet(ObTablet* tablet);

      /**
       * @brief set_sample_rate
       * @param sample_rate
       * @return OB_SUCCESS or other ERROR
       */
      inline int set_sample_rate(int64_t sample_rate);

      /**
       * @brief get_index_reporter
       * @return &index_reporter_
       */
      inline ObIndexReporter* get_index_reporter();

      /**
       * @brief get_allocator
       * @return &allocator_
       */
      inline common::ModuleArena* get_allocator();

      /**
       * @brief save_current_row
       * @return OB_SUCCESS or other ERROR
       */
      int save_current_row();

      /**
       * @brief create_new_sstable
       * @param table_id
       * @param disk_no
       * @return OB_SUCCESS or other ERROR
       */
      int create_new_sstable(const uint64_t table_id, const int32_t disk_no);

      /**
       * @brief start
       * start to construct local index
       * @return OB_SUCCESS or other ERROR
       */
      virtual int start();

      /**
       * @brief cons_index_data_row_desc
       * construct row description for columns which all index tables of the data table involve
       * @param index_data_row_desc
       * @param data_tid
       * @param index_tid
       * @return OB_SUCCESS or other ERROR
       */
      int cons_index_data_row_desc(ObRowDesc &index_data_row_desc, uint64_t data_tid, uint64_t index_tid);
	  
      /**
       * @brief push_cid_in_desc_and_ophy
       * push column id to sort operator and to row description to scan sstable and get a index table row
       * @param data_tid
       * @param index_tid
       * @param index_data_row_desc need row description for columns which all index tables of the data table involve
       * @param basic_columns row description to scan sstable
       * @param desc row description to get a index table row
       * @return OB_SUCCESS or other ERROR
       */
      int push_cid_in_desc_and_ophy(uint64_t data_tid, uint64_t index_tid, const ObRowDesc index_data_row_desc, ObArray<uint64_t> &basic_columns, ObRowDesc &desc);
	  
      /**
       * @brief trans_row_to_sstrow
       * transform ObRow into ObSSTableRow
       * @param row_desc
       * @param row
       * @return OB_SUCCESS or other ERROR
       */
      int trans_row_to_sstrow(ObRowDesc &row_desc, const ObRow &row);
	  
      /**
       * @brief cons_column_checksum_row_desc_for_data
       * construct row description for calculating column checksum of data table
       * @param column_checksum_row_desc
       * @param tid
       * @return OB_SUCCESS or other ERROR
       */
      int cons_column_checksum_row_desc_for_data(ObRowDesc &column_checksum_row_desc, uint64_t tid);
	  
      /**
       * @brief calc_column_checksum_for_data
       * calculate column checksum of data table
       * @param row
       * @param column_checksum_row_desc
       * @param column_checksum
       * @param tid
       * @return OB_SUCCESS or other ERROR
       */
      int calc_column_checksum_for_data(const ObRow& row, ObRowDesc &column_checksum_row_desc, char *column_checksum, uint64_t tid);

      /**
       * @brief write_partitional_index
       * build partitional index
       * @param range
       * @param index_tid
       * @param disk_no
       * @param tablet_row_count
       * @param sample_rate
       * @return OB_SUCCESS or other ERROR
       */
      int write_partitional_index(
          ObNewRange range,
          uint64_t index_tid,
          int32_t disk_no,
          int64_t tablet_row_count,
          int64_t sample_rate);

      /**
       * @brief cons_local_index
       * construct local index
       * @param tablet
       * @param sample_rate
       * @return OB_SUCCESS or other ERROR
       */
      int cons_local_index(ObTablet *tablet, int64_t sample_rate);

    private:
      uint64_t table_id_;
      ObNewRange new_range_; ///<tablet range
      ObSSTableId sstable_id_; ///<new created sstable id
      char sstable_path_[OB_MAX_FILE_NAME_LENGTH]; ///<new created sstable path
      ObSSTableSchema sstable_schema_; ///<new created sstable schema
      ObSSTableWriter sstable_writer_; ///<sstable writer
      int64_t sstable_size_; ///<sstable size
      int64_t frozen_version_; ///<frozen version
      ObSort sort_; ///<sort operator for sorting rows by index column
      ObSSTableRow sst_row_; ///<sstable row
      ObTabletManager* tablet_manager_; ///<tablet manager
      const common::ObTableSchema* new_table_schema_; ///<table schema
      const common::ObSchemaManagerV2* schema_manager_; ///<schema manager
      common::ObMergerSchemaManager *merger_schema_manager_; ///<merger schema manager
      ObColumnChecksum column_checksum_; ///<column checksum for data table
      ModuleArena allocator_; ///<module arena
      ObTablet* tablet_; ///<tablet of data table
      int64_t sample_rate_; ///<frequency of sampling

      ObIndexReporter index_reporter_; ///<for report index histogram information
      ObIndexHandlePool *handle_pool_; ///<thread pool for constructing index
    };

    inline int ObLocalIndexHandler::set_tablet(ObTablet* tablet)
    {
      int ret = OB_SUCCESS;
      tablet_ = tablet;
      if (NULL == tablet_)
      {
        TBSYS_LOG(ERROR,"failed to set tablet_!");
        ret = OB_ERROR;
      }
      return ret;
    }

    inline int ObLocalIndexHandler::set_sample_rate(int64_t sample_rate)
    {
      int ret = OB_SUCCESS;
      sample_rate_ = sample_rate;
      if (0 == sample_rate_)
      {
        TBSYS_LOG(ERROR,"failed to set sample_rate_!");
        ret = OB_ERROR;
      }
      return ret;
    }

    inline common::ModuleArena* ObLocalIndexHandler::get_allocator()
    {
      return &allocator_;
    }

    inline ObIndexReporter* ObLocalIndexHandler::get_index_reporter()
    {
      return &index_reporter_;
    }
  }
}

#endif // OBLOCALINDEXHANDLER_H
