/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_handler.h
 * @brief base class of ObGlobalIndexHandler
 *
 * Created by longfei： base class of ObGlobalIndexHandler
 * future work:
 *   1.maybe ob_local_index_handler can derived from this class too
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_05
 */

#ifndef CHUNKSERVER_OB_INDEX_HANDLER_H_
#define CHUNKSERVER_OB_INDEX_HANDLER_H_

#include "ob_index_handle_pool.h"
#include "ob_tablet.h"
#include "sstable/ob_sstable_writer.h"
#include "sstable/ob_sstable_reader.h"


namespace oceanbase
{
  namespace chunkserver
  {
    using namespace sstable;
    /**
     * @brief The ObIndexHandler class
     * working thread for constructing index
     */
    class ObIndexHandler
    {
    public:
      static const int64_t DEFAULT_ESTIMATE_ROW_COUNT = 256*1024LL;
      static const int64_t DEFAULT_MEMORY_LIMIT = 256*1024*1024LL;
    public:
      /**
       * @brief ObIndexHandler constructor
       * @param pool
       * @param schema_mgr
       * @param tablet_mgr
       */
      ObIndexHandler(ObIndexHandlePool *pool,common::ObMergerSchemaManager *schema_mgr,ObTabletManager* tablet_mgr);
      /**
       * @brief ~ObIndexHandler destructor
       */
      virtual ~ObIndexHandler();
    public:
      /**
       * @brief init: Pure virtual function
       * @return
       */
      virtual int init() = 0;  //每个子类对象使用之前必须初始化
      /**
       * @brief start: Pure virtual function
       * @return
       */
      virtual int start() = 0; //开始处理索引构建过程
      /**
       * @brief create_new_sstable
       * @param table_id
       * @param disk
       * @return err code
       */
      int create_new_sstable(uint64_t table_id, int32_t disk);
      /**
       * @brief fill_sstable_schema
       * @param table_id
       * @param common_schema
       * @param [out] sstable_schema
       * @return err code
       */
      int fill_sstable_schema(
          const uint64_t table_id,
          const common::ObSchemaManagerV2& common_schema,
          sstable::ObSSTableSchema& sstable_schema);
      /**
       * @brief save_current_row
       * @return succ or fail
       */
      int save_current_row();
      /**
       * @brief trans_row_to_sstrow
       * @param desc
       * @param row
       * @param [out] sst_row
       * @return succ or fail
       */
      int trans_row_to_sstrow(ObRowDesc &desc, const ObRow &row, ObSSTableRow &sst_row);
    public:
      //get() series
      inline int32_t get_disk_no() const
      {
        return disk_no_;
      }
      inline common::ObNewRange get_new_range()
      {
        return new_range_;
      }
      inline ObSSTableSchema& get_sstable_schema()
      {
        return sstable_schema_;
      }
      inline ObSSTableRow& get_sstable_row()
      {
        return row_;
      }
      inline int64_t get_frozen_version() const
      {
        return frozen_version_;
      }
      inline ObTabletExtendInfo& get_tablet_extend_info()
      {
        return index_extend_info_;
      }
      inline int64_t get_cur_sstable_size()
      {
        return current_sstable_size_;
      }
      inline const common::ObSchemaManagerV2* get_schema_mgr() const
      {
        return current_schema_manager_;
      }
      inline common::ObMergerSchemaManager* get_merge_schema_mgr()
      {
        return manager_;
      }
      inline ObSSTableId& get_sstable_id()
      {
        return sstable_id_;
      }
      inline ObTabletManager* get_tablet_mgr()
      {
        return tablet_manager_;
      }
      inline const common::ObTableSchema* get_table_schema() const
      {
        return new_table_schema_;
      }
      inline ObSSTableWriter& get_sstable_writer()
      {
        return sstable_writer_;
      }
      inline ObIndexHandlePool* get_handle_pool()
      {
        return handle_pool_;
      }

    public:
      //set() series
      inline void set_frozen_version(int64_t frozen_version)
      {
        frozen_version_ = frozen_version;
      }
      inline void set_sstable_size(int64_t tmp_size)
      {
        current_sstable_size_ = tmp_size;
      }
      inline void set_schema_mgr(const common::ObSchemaManagerV2* cur_schema_mgr)
      {
        current_schema_manager_ = cur_schema_mgr;
      }
      inline void set_disk_no(int32_t disk_no)
      {
        disk_no_ = disk_no;
      }
      inline void set_handle_pool(ObIndexHandlePool *handle_pool)
      {
        handle_pool_ = handle_pool;
      }
      inline void set_new_range(const ObNewRange& new_range)
      {
        new_range_ = new_range;
      }

    private:
      ObIndexHandlePool *handle_pool_; ///< allocating space for the handler
      common::ObMergerSchemaManager* manager_; ///< schema manager
      const common::ObSchemaManagerV2* current_schema_manager_; ///< current schema_manager_
      common::ObNewRange new_range_; ///< data in this range is what we need to construct
      ObTabletManager* tablet_manager_; ///< for tablet manager
      ObSSTableId sstable_id_; ///< new sstable we create
      ObSSTableSchema sstable_schema_;///< sstable schema
      const common::ObTableSchema* new_table_schema_; ///< the table schema of the new_range_
      char path_[common::OB_MAX_FILE_NAME_LENGTH]; ///< sstable's path
      ObString path_string_; ///< sstable's path
      ObString compressor_string_;///< compressor method
      int64_t frozen_version_; ///< frozen version
      ObSSTableWriter sstable_writer_; ///< for saving sstable in disk
      int32_t disk_no_; ///< disk number of sstable we create
      ObSSTableRow row_; ///< sstable row
      int64_t current_sstable_size_; ///< the size of sstable
      ObTabletExtendInfo index_extend_info_; ///< set checksum
    };

  } /* namespace chunkserver */
} /* namespace oceanbase */

#endif /* CHUNKSERVER_OB_INDEX_HANDLER_H_ */
