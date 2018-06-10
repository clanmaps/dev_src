/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_tablet_merger.h
* @brief for merging tablets
*
* modified by maoxiaoxiao:calculate column checksum of data table and index table during daily merge
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2015_01_21
*/

/*
 * (C) 2007-2010 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License 
 * version 2 as published by the Free Software Foundation. 
 *
 *         ob_tablet_merger.h is for what ...
 *
 *  Version: $Id: ob_tablet_merger.h 12/25/2012 02:46:43 PM qushan Exp $
 *
 *  Authors:
 *     qushan < qushan@taobao.com >
 *        - some work details if you want
 */


#ifndef OB_CHUNKSERVER_OB_TABLET_MERGER_V1_H_
#define OB_CHUNKSERVER_OB_TABLET_MERGER_V1_H_

#include "common/ob_define.h"
#include "sstable/ob_sstable_writer.h"
#include "sstable/ob_sstable_reader.h"
#include "ob_get_scan_proxy.h"
#include "ob_get_cell_stream_wrapper.h"
#include "ob_query_agent.h"
#include "ob_tablet_merge_filter.h"
//add maoxx
#include "common/ob_column_checksum.h"
//add e

namespace oceanbase
{
  namespace common
  {
    class ObTableSchema;
    class ObInnerCellInfo;
  }
  namespace sstable
  {
    class ObSSTableSchema;
  }
  namespace chunkserver
  {
    class ObTabletManager;
    class ObTablet;
    class ObChunkMerge;

    class ObTabletMerger
    {
      public:
        static const int64_t DEFAULT_ESTIMATE_ROW_COUNT = 256*1024LL;
      public:
        ObTabletMerger(ObChunkMerge& chunk_merge, ObTabletManager& manager);
        virtual ~ObTabletMerger();

        virtual int init();
        virtual int merge(ObTablet *tablet, int64_t frozen_version) = 0;

        int update_meta(ObTablet* old_tablet, 
            const common::ObVector<ObTablet*> & tablet_array, const bool sync_meta);
        int gen_sstable_file_location(const int32_t disk_no, 
            sstable::ObSSTableId& sstable_id, char* path, const int64_t path_size);
        int64_t calc_tablet_checksum(const int64_t checksum);

      protected:
        ObChunkMerge& chunk_merge_;
        ObTabletManager& manager_;

        ObTablet* old_tablet_;            // merge or import tablet object.
        int64_t frozen_version_;          // merge or import tablet new version.
        sstable::ObSSTableId sstable_id_; // current generate new sstable id
        char path_[common::OB_MAX_FILE_NAME_LENGTH]; // current generate new sstable path
        common::ObVector<ObTablet *> tablet_array_;  //generate new tablet objects.
    };

    class ObTabletMergerV1 : public ObTabletMerger
    {
      public:
        ObTabletMergerV1(ObChunkMerge& chunk_merge, ObTabletManager& manager);
        ~ObTabletMergerV1() {}

        int init();
        int merge(ObTablet *tablet,int64_t frozen_version);

      private:
        int reset();

        DISALLOW_COPY_AND_ASSIGN(ObTabletMergerV1);

        enum RowStatus
        {
          ROW_START = 0,
          ROW_GROWING = 1,
          ROW_END = 2
        };

        bool is_table_need_join(const uint64_t table_id);
        int fill_sstable_schema(const common::ObTableSchema& common_schema,
          sstable::ObSSTableSchema& sstable_schema);
        int update_range_start_key();
        int update_range_end_key();
        int create_new_sstable();
        int create_hard_link_sstable(int64_t& sstable_size);
        int finish_sstable(const bool is_tablet_unchanged, ObTablet*& new_tablet);
        int report_tablets(ObTablet* tablet_list[],int32_t tablet_size);
        bool maybe_change_sstable() const;

        int fill_scan_param(const uint64_t column_group_id);
        int wait_aio_buffer() const;
        int reset_local_proxy() const;
        int merge_column_group(
            const int64_t column_group_idx,
            const uint64_t column_group_id,
            int64_t& split_row_pos,
            const int64_t max_sstable_size, 
            const bool is_need_join,
            bool& is_tablet_splited,
            bool& is_tablet_unchanged
            );

        int save_current_row(const bool current_row_expired);
        int check_row_count_in_column_group();
        void reset_for_next_column_group();

        //add maoxx
        /**
         * @brief is_index_or_with_index
         * decide if the table is an index table or a data table with index
         * @param table_id
         * @return OB_SUCCESS or other ERROR
         */
        int is_index_or_with_index(const uint64_t table_id);

        /**
         * @brief cons_column_checksum_row_desc_for_data
         * construct construct row description for calculating column checksum of data table
         * @param row_desc
         * @param tid
         * @return OB_SUCCESS or other ERROR
         */
        int cons_column_checksum_row_desc_for_data(ObRowDesc &row_desc, uint64_t tid);

        /**
         * @brief cons_column_checksum_row_desc_for_index
         * construct construct row description for calculating column checksum of index table
         * @param row_desc
         * @param tid
         * @param max_data_table_cid
         * @return OB_SUCCESS or other ERROR
         */
        int cons_column_checksum_row_desc_for_index(ObRowDesc &row_desc, uint64_t tid, uint64_t &max_data_table_cid);

        /**
         * @brief calc_tablet_col_checksum_for_data
         * calculate column checksum of data table
         * @param row
         * @param row_desc
         * @param column_checksum
         * @return OB_SUCCESS or other ERROR
         */
        int calc_tablet_col_checksum_for_data(const sstable::ObSSTableRow& row, ObRowDesc row_desc, char *column_checksum);

        /**
         * @brief calc_tablet_col_checksum_for_index
         * calculate column checksum of index table
         * @param row
         * @param row_desc
         * @param column_checksum
         * @param max_data_table_cid
         * @return OB_SUCCESS or other ERROR
         */
        int calc_tablet_col_checksum_for_index(const sstable::ObSSTableRow& row, ObRowDesc row_desc, char *column_checksum, const uint64_t max_data_table_cid);
        //add e

      private:
        common::ObRowkey split_rowkey_;
        common::ObMemBuf split_rowkey_buffer_;
        common::ObMemBuf last_rowkey_buffer_;

        common::ObInnerCellInfo*     cell_;
        const common::ObTableSchema* new_table_schema_;

        sstable::ObSSTableRow    row_;
        sstable::ObSSTableSchema sstable_schema_;
        sstable::ObSSTableWriter writer_;
        common::ObNewRange new_range_;

        int64_t current_sstable_size_;
        int64_t row_num_;
        int64_t pre_column_group_row_num_;

        common::ObString path_string_;
        common::ObString compressor_string_;

        ObGetScanProxy           cs_proxy_;
        common::ObScanParam      scan_param_;

        ObGetCellStreamWrapper    ms_wrapper_;
        ObQueryAgent          merge_join_agent_;
        ObTabletMergerFilter tablet_merge_filter_;

        //add maoxx
        ObColumnChecksum column_checksum_; ///<column checksum
        bool is_index_; ///<true if the table is index table
        bool is_have_index_; ///<true if the data table have index
        bool is_have_init_index_; ///<true if the data table have initialized index
        ObRowDesc column_checksum_row_desc_; ///<row description for column checksum
        uint64_t max_data_table_cid_; ///<max column id of data table
        //add e
        bool is_static_truncated_; /*add hxlong [Truncate Table]:20170318*/
    };
  } /* chunkserver */
} /* oceanbase */

#endif //OB_CHUNKSERVER_OB_TABLET_MERGER_V1_H_


