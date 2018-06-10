/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file  ob_schema_service.h
 * @brief schema service
 *
 * modified by longfei：
 * 1.add two more member variables in TableSchema(struct) and their serialize() series function
 * 2.add IndexBeat for construct static secondary index CS <==heart beat==> RS
 *
 * modified by WengHaixing:
 * 1.add some function to fit secondary index constrution
 *
 * modified by maoxiaoxiao:
 * 1.add functions to check column checksum, clean column checksum and get column checksum
 *
 * modified by wangdonghui: add functions  for procedure of create and drop
 *
 * @version __DaSE_VERSION
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @date 2016_07_29
 */

/**
 * (C) 2010-2011 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_schema_service.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *   - 表单schema相关数据结构。创建，删除，获取schema描述结构接口
 *
 */

#ifndef _OB_SCHEMA_SERVICE_H
#define _OB_SCHEMA_SERVICE_H

#include "ob_define.h"
#include "ob_object.h"
#include "ob_string.h"
#include "ob_array.h"
#include "ob_hint.h"

//add by wangdonghui 20160308 :b
#include "common/nb_accessor/nb_query_res.h"
//add :e

namespace oceanbase
{
  namespace common
  {
    typedef ObObjType ColumnType;

    //add longfei [cons static index] 151120:b
    /**
     * @brief The IndexBeat struct
     * index beat is a type of heart beat, can carry some information between rs and cs
     */
    struct IndexBeat
    {
      uint64_t idx_tid_; ///< index table's id
      IndexStatus status_; ///< index's status
      int64_t hist_width_; ///< histogram's width
      ConIdxStage stage_; ///< stage of constructing
      /**
       * @brief IndexBeat constructor
       */
      IndexBeat()
      {
        idx_tid_ = OB_INVALID_ID;
        status_ = ERROR;
        hist_width_ = 0;
        stage_ = STAGE_INIT;
      }
      /**
       * @brief reset
       */
      void reset()
      {
        idx_tid_ = OB_INVALID_ID;
        status_ = ERROR;
        hist_width_ = 0;
        stage_ = STAGE_INIT;
      }
      NEED_SERIALIZE_AND_DESERIALIZE;
    };
    //add e

    /* 表单join关系描述，对应于__all_join_info内部表 */
    struct JoinInfo
    {
      char left_table_name_[OB_MAX_TABLE_NAME_LENGTH];
      uint64_t left_table_id_;
      char left_column_name_[OB_MAX_COLUMN_NAME_LENGTH];
      uint64_t left_column_id_;
      char right_table_name_[OB_MAX_TABLE_NAME_LENGTH];
      uint64_t right_table_id_;
      char right_column_name_[OB_MAX_COLUMN_NAME_LENGTH];
      uint64_t right_column_id_;

      int64_t to_string(char* buf, const int64_t buf_len) const
      {
        int64_t pos = 0;
        databuff_printf(buf, buf_len, pos, "left_table:tname[%s], tid[%lu], cname[%s], cid[%lu]",
          left_table_name_, left_table_id_, left_column_name_, left_column_id_);
        databuff_printf(buf, buf_len, pos, "right_table:tname[%s], tid[%lu], cname[%s], cid[%lu]",
          right_table_name_, right_table_id_, right_column_name_, right_column_id_);
        return pos;
      }

      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    /* 表单column描述，对应于__all_all_column内部表 */
    struct ColumnSchema
    {
      char column_name_[OB_MAX_COLUMN_NAME_LENGTH];
      uint64_t column_id_;
      uint64_t column_group_id_;
      int64_t rowkey_id_;
      uint64_t join_table_id_;
      uint64_t join_column_id_;
      ColumnType data_type_;
      int64_t data_length_;
      int64_t data_precision_;
      int64_t data_scale_;
      bool nullable_;
      //add lbzhong [auto_increment] 20161123:b
      bool auto_increment_;
      //add:e
      int64_t length_in_rowkey_; //如果是rowkey列，则表示在二进制rowkey串中占用的字节数；
      int32_t order_in_rowkey_;
      ObCreateTime gm_create_;
      ObModifyTime gm_modify_;
      ColumnSchema():column_id_(OB_INVALID_ID), column_group_id_(OB_INVALID_ID), rowkey_id_(-1),
          join_table_id_(OB_INVALID_ID), join_column_id_(OB_INVALID_ID), data_type_(ObMinType),
          data_precision_(0), data_scale_(0), nullable_(true),
          //add lbzhong [auto_increment] 20161123:b
          auto_increment_(false),
          //add:e
          length_in_rowkey_(0), order_in_rowkey_(0)
      {
        column_name_[0] = '\0';
      }
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    /* 表单schema描述，对应于__all_all_table内部表 */
    struct TableSchema
    {
      enum TableType
      {
        NORMAL = 1,
        INDEX,
        META,
        VIEW,
      };

      enum LoadType
      {
        INVALID = 0,
        DISK = 1,
        MEMORY
      };

      enum TableDefType
      {
        INTERNAL = 1,
        USER_DEFINED
      };

      enum SSTableVersion
      {
        OLD_SSTABLE_VERSION = 1,
        DEFAULT_SSTABLE_VERSION = 2,
        NEW_SSTABLE_VERSION = 3,
      };

      char table_name_[OB_MAX_TABLE_NAME_LENGTH];
      char compress_func_name_[OB_MAX_TABLE_NAME_LENGTH];
      char expire_condition_[OB_MAX_EXPIRE_CONDITION_LENGTH];
      char comment_str_[OB_MAX_TABLE_COMMENT_LENGTH];
      uint64_t table_id_;
      TableType table_type_;
      LoadType load_type_;
      TableDefType table_def_type_;
      bool is_use_bloomfilter_;
      bool is_pure_update_table_;
      int64_t consistency_level_;
      int64_t rowkey_split_;
      int32_t rowkey_column_num_;
      int32_t replica_num_;
      int64_t max_used_column_id_;
      int64_t create_mem_version_;
      int64_t tablet_block_size_;
      int64_t tablet_max_size_;
      int64_t max_rowkey_length_;

      //longfei [create index]
      uint64_t original_table_id_; ///< original table's id
      IndexStatus index_status_; ///< index table's status

      int64_t merge_write_sstable_version_;
      int64_t schema_version_;
      uint64_t create_time_column_id_;
      uint64_t modify_time_column_id_;
      ObArray<ColumnSchema> columns_;
      ObArray<JoinInfo> join_info_;

    public:
        TableSchema()
          :table_id_(OB_INVALID_ID),
           table_type_(NORMAL),
           load_type_(DISK),
           table_def_type_(USER_DEFINED),
           is_use_bloomfilter_(false),
           is_pure_update_table_(false),
           consistency_level_(NO_CONSISTENCY),
           rowkey_split_(0),
           rowkey_column_num_(0),
           replica_num_(OB_SAFE_COPY_COUNT),
           max_used_column_id_(0),
           create_mem_version_(0),
           tablet_block_size_(OB_DEFAULT_SSTABLE_BLOCK_SIZE),
          tablet_max_size_(OB_DEFAULT_MAX_TABLET_SIZE),
          max_rowkey_length_(0),

          //longfei [create index]
          original_table_id_(OB_INVALID_ID),
          index_status_(ERROR),

          merge_write_sstable_version_(DEFAULT_SSTABLE_VERSION),
          schema_version_(0),
          create_time_column_id_(OB_CREATE_TIME_COLUMN_ID),
          modify_time_column_id_(OB_MODIFY_TIME_COLUMN_ID)
      {
        table_name_[0] = '\0';
        compress_func_name_[0] = '\0';
        expire_condition_[0] = '\0';
        comment_str_[0] = '\0';
      }

      inline void init_as_inner_table()
      {
        // clear all the columns at first
        clear();
        this->table_id_ = OB_INVALID_ID;
        this->table_type_ = TableSchema::NORMAL;
        this->load_type_ = TableSchema::DISK;
        this->table_def_type_ = TableSchema::INTERNAL;
        this->replica_num_ = OB_SAFE_COPY_COUNT;
        this->create_mem_version_ = 1;
        this->tablet_block_size_ = OB_DEFAULT_SSTABLE_BLOCK_SIZE;
        this->tablet_max_size_ = OB_DEFAULT_MAX_TABLET_SIZE;
        strcpy(this->compress_func_name_, OB_DEFAULT_COMPRESS_FUNC_NAME);
        this->is_use_bloomfilter_ = false;
        this->is_pure_update_table_ = false;
        this->consistency_level_ = NO_CONSISTENCY;
        this->rowkey_split_ = 0;
        this->merge_write_sstable_version_ = DEFAULT_SSTABLE_VERSION;
        this->create_time_column_id_ = OB_CREATE_TIME_COLUMN_ID;
        this->modify_time_column_id_ = OB_MODIFY_TIME_COLUMN_ID;
      }

      inline int add_column(const ColumnSchema& column)
      {
        return columns_.push_back(column);
      }

      inline int add_join_info(const JoinInfo& join_info)
      {
        return join_info_.push_back(join_info);
      }

      inline int64_t get_column_count(void) const
      {
        return columns_.count();
      }
      inline ColumnSchema* get_column_schema(const int64_t index)
      {
        ColumnSchema * ret = NULL;
        if ((index >= 0) && (index < columns_.count()))
        {
          ret = &columns_.at(index);
        }
        return ret;
      }
      inline ColumnSchema* get_column_schema(const uint64_t column_id)
      {
        ColumnSchema *ret = NULL;
        for(int64_t i = 0; i < columns_.count(); i++)
        {
          if (columns_.at(i).column_id_ == column_id)
          {
            ret = &columns_.at(i);
            break;
          }
        }
        return ret;
      }
      inline ColumnSchema* get_column_schema(const char * column_name)
      {
        ColumnSchema *ret = NULL;
        for(int64_t i = 0; i < columns_.count(); i++)
        {
          if (strcmp(column_name, columns_.at(i).column_name_) == 0)
          {
            ret = &columns_.at(i);
            break;
          }
        }
        return ret;
      }
      inline const ColumnSchema* get_column_schema(const char * column_name) const
      {
        const ColumnSchema *ret = NULL;
        for(int64_t i = 0; i < columns_.count(); i++)
        {
          if (strcmp(column_name, columns_.at(i).column_name_) == 0)
          {
            ret = &columns_.at(i);
            break;
          }
        }
        return ret;
      }
      inline int get_column_rowkey_index(const uint64_t column_id, int64_t &rowkey_idx)
      {
        int ret = OB_SUCCESS;
        rowkey_idx = -1;
        for (int64_t i = 0; i < columns_.count(); i++)
        {
          if (columns_.at(i).column_id_ == column_id)
          {
            rowkey_idx = columns_.at(i).rowkey_id_;
          }
        }
        if (-1 == rowkey_idx)
        {
          TBSYS_LOG(WARN, "fail to get column. column_id=%lu", column_id);
          ret = OB_ENTRY_NOT_EXIST;
        }
        return ret;
      }
        void clear();
        bool is_valid() const;
        int to_string(char* buffer, const int64_t length) const;

        static bool is_system_table(const common::ObString &tname);
        NEED_SERIALIZE_AND_DESERIALIZE;
    };

    inline bool TableSchema::is_system_table(const common::ObString &tname)
    {
      bool ret = false;
      if (tname.length() >= 1)
      {
        const char *p = tname.ptr();
        if ('_' == p[0])
        {
          ret = true;
        }
      }
      return ret;
    }

    // for alter table add or drop columns rpc construct
    struct AlterTableSchema
    {
      public:
        enum AlterType
        {
          ADD_COLUMN = 0,
          DEL_COLUMN = 1,
          MOD_COLUMN = 2,
        };
        struct AlterColumnSchema
        {
          AlterType type_;
          ColumnSchema column_;
        };
        // get table name
        const char * get_table_name(void) const
        {
          return table_name_;
        }
        // add new column
        int add_column(const AlterType type, const ColumnSchema& column)
        {
          AlterColumnSchema schema;
          schema.type_ = type;
          schema.column_ = column;
          return columns_.push_back(schema);
        }
        // clear all
        void clear(void)
        {
          table_id_ = OB_INVALID_ID;
          table_name_[0] = '\0';
          columns_.clear();
        }
        int64_t get_column_count(void) const
        {
          return columns_.count();
        }
        NEED_SERIALIZE_AND_DESERIALIZE;
        uint64_t table_id_;
        char table_name_[OB_MAX_TABLE_NAME_LENGTH];
        ObArray<AlterColumnSchema> columns_;
    };

    // ob table schema service interface layer
    class ObScanHelper;
    class ObSchemaService
    {
      public:
        virtual ~ObSchemaService() {}
        virtual int init(ObScanHelper* client_proxy, bool only_core_tables) = 0;
        virtual int get_table_schema(const ObString& table_name, TableSchema& table_schema) = 0;
        virtual int create_table(const TableSchema& table_schema) = 0;
        //add by wangdonghui 20160125 :b
        /**
         * @brief create_procedure
         * pure virtual function,create procedure
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @return error code
         */
        virtual int create_procedure(const common::ObString& proc_name, const common::ObString & proc_source_code) = 0;
        /**
         * @brief get_procedure_info
         * get procedure information
         * @param res_ query result set
         * @return  error code
         */
        virtual int get_procedure_info(common::nb_accessor::QueryRes *&res_) = 0;
        //add :e
        virtual int drop_table(const ObString& table_name) = 0;

        //add by wangdonghui 20160225 [drop procedure] :b
        /**
         * @brief drop_procedure
         * drop procedure
         * @param proc_name procecure name
         * @return error code
         */
        virtual int drop_procedure(const ObString& proc_name) = 0;
        //add :e
        virtual int alter_table(const AlterTableSchema& table_schema, const int64_t old_schema_version) = 0;
        virtual int get_table_id(const ObString& table_name, uint64_t& table_id) = 0;
        virtual int get_table_name(uint64_t table_id, ObString& table_name) = 0;
        virtual int get_max_used_table_id(uint64_t &max_used_tid) = 0;
        virtual int modify_table_id(TableSchema& table_schema, const int64_t new_table_id) = 0;
        virtual int set_max_used_table_id(const uint64_t max_used_tid) = 0;
      //add maoxx
      /**
       * @brief check_column_checksum
       * check column checksum
       * @param orginal_table_id
       * @param index_table_id
       * @param cluster_id
       * @param current_version
       * @param column_checksum_flag
       * @return OB_SUCCESS or other ERROR
       */
      virtual int check_column_checksum(const int64_t orginal_table_id, const int64_t index_table_id, const int64_t cluster_id, const int64_t current_version, bool &column_checksum_flag) = 0;

      /**
       * @brief clean_column_checksum
       * clean column checksum
       * @param max_draution_of_version
       * @param current_version
       * @return OB_SUCCESS or other ERROR
       */
      virtual int clean_column_checksum(const int64_t max_draution_of_version, const int64_t current_version) = 0;

      /**
       * @brief get_column_checksum
       * get column checksum
       * @param range
       * @param cluster_id
       * @param required_version
       * @param column_checksum
       * @return OB_SUCCESS or other ERROR
       */
      virtual int get_column_checksum(const ObNewRange range, const int64_t cluster_id, const int64_t required_version, ObString& column_checksum) = 0;
      //add e    
      //add wenghaixing [secondary index.static_index]20151217
      virtual int get_cluster_count(int64_t &cc) = 0;
      virtual int get_index_stat(const uint64_t table_id, const int64_t cluster_count, IndexStatus &stat) = 0;
      //add e
    };

  }
}


#endif /* _OB_SCHEMA_SERVICE_H */
