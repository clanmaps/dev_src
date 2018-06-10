/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_schema.cpp
 * @brief schema message for oceanbase
 *
 * modified by longfei：
 * 1.add member variables original_table_id_, index_status_ into TableSchema(class)
 * 2.modified DEFINE_SERIALIZE(ObTableSchema) and DEFINE_DESERIALIZE(ObTableSchema) for new member mentioned above
 * 3.create an hash map to log the (tid <--> indexList)
 * 4.add Judgment Rule for using secondary index in select
 *
 * modified by WengHaixing:
 * 1.add a fuction call to find all not available index
 *
 * modified by maoxiaoxiao:
 * 1.add functions to get some information about index lists of a table
 * 
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @date 2016_01_21
 */

/*===============================================================
*   (C) 2007-2010 Taobao Inc.
*
*
*   Version: 0.1 2010-09-26
*
*   Authors:
*          daoan(daoan@taobao.com)
*          maoqi(maoqi@taobao.com)
*          fangji.hcm(fangji.hcm@taobao.com)
*
*
================================================================*/
#ifndef OCEANBASE_COMMON_OB_SCHEMA_H_
#define OCEANBASE_COMMON_OB_SCHEMA_H_
#include <stdint.h>

#include <tbsys.h>

#include "ob_define.h"
#include "ob_object.h"
#include "ob_string.h"
#include "ob_array.h"
#include "hash/ob_hashutils.h"
#include "hash/ob_hashmap.h"
#include "ob_postfix_expression.h"
#include "ob_hint.h"

#define PERM_TABLE_NAME "__perm_info"
#define USER_TABLE_NAME "__user_info"
#define SKEY_TABLE_NAME "__skey_info"
#define PERM_COL_NAME "perm_desc"
#define USER_COL_NAME "password"
#define SKEY_COL_NAME "secure_key"

#define PERM_TABLE_ID 100
#define USER_TABLE_ID 101
#define SKEY_TABLE_ID 102
#define PERM_COL_ID 4
#define USER_COL_ID 4
#define SKEY_COL_ID 4

namespace oceanbase
{
  namespace common
  {
    const int64_t OB_SCHEMA_VERSION = 1;
    const int64_t OB_SCHEMA_VERSION_TWO = 2;
    const int64_t OB_SCHEMA_VERSION_THREE = 3;
    const int64_t OB_SCHEMA_VERSION_FOUR = 4;
    const int64_t OB_SCHEMA_VERSION_FOUR_FIRST = 401;
    const int64_t OB_SCHEMA_VERSION_FOUR_SECOND = 402;
    //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        static const uint8_t META_PREC_MASK = 0x7F;
        static const uint8_t META_SCALE_MASK = 0x3F;
        //add:e

    //add wenghaixing [secondary index]
    struct IndexList
    {
       uint64_t index_tid[OB_MAX_INDEX_NUMS];
       int64_t offset;
    public:
       IndexList()
       {
         offset=0;
         for(int64_t i=0;i<OB_MAX_INDEX_NUMS;i++)
         {
           index_tid[i]=OB_INVALID_ID;
         }
       }

       //modify wenghaixing [secondary index upd]
       inline int add_index(uint64_t tid)
       {
         int ret = OB_ERROR;
         bool exist = false;
         for(int64_t i = 0;i<offset;i++)
         {
           if(index_tid[i] == tid)
           {
             exist = true;
             ret = OB_SUCCESS;
           }
         }
         if(!exist && offset < OB_MAX_INDEX_NUMS)
         {
           offset++;
           index_tid[offset-1]=tid;
           ret = OB_SUCCESS;
         }
         else if(!exist)
         {
           TBSYS_LOG(WARN,"the index num of list is full!");
         }

         return ret;
       }
       //modify e

       inline int64_t get_count()
       {
         return offset;
       }

       inline void get_idx_id(int64_t i,uint64_t &idx_id)
       {
         idx_id = index_tid[i];
       }

    };
    //add e

    struct TableSchema;
    //these classes are so close in logical, so I put them together to make client have a easy life
    typedef ObObjType ColumnType;
    const char* convert_column_type_to_str(ColumnType type);
    struct ObRowkeyColumn
    {
      enum Order
      {
        ASC = 1,
        DESC = -1,
      };
      int64_t length_;
      uint64_t column_id_;
      ObObjType type_;
      Order order_;
      NEED_SERIALIZE_AND_DESERIALIZE;
      int64_t to_string(char* buf, const int64_t buf_len) const;
    };

    class ObRowkeyInfo
    {

    public:
      ObRowkeyInfo();
      ~ObRowkeyInfo();

      inline int64_t get_size() const
      {
        return size_;
      }

      //add longfei [assign] 2016-04-22 10:38:48
      int assign(const ObRowkeyInfo& other)
      {
        int ret = OB_SUCCESS;
        reset();
        if(this == &other)
        {
          TBSYS_LOG(INFO, "same rowkeyinfo");
        }
        else
        {
          TBSYS_LOG(DEBUG, "in %s, other size is %ld", __FUNCTION__, other.get_size());
          for(int64_t i = 0; i < other.get_size(); i++)
          {
            ret = this->set_column(i, *other.get_column(i));
            if (OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "in %s, assign failed", __FUNCTION__);
              break;
            }
          }
        }
        return ret;
      }
      //add e

      //add longfei [reset] 2016-04-23 10:44:22
      void reset()
      {
        size_ = 0;
      }
      //add e

      /**
       * get sum of every column's length.
       */
      int64_t get_binary_rowkey_length() const;

      /**
       * Get rowkey column by index
       * @param[in]  index   column index in RowkeyInfo
       * @param[out] column
       *
       * @return int  return OB_SUCCESS if get the column, otherwist return OB_ERROR
       */
      int get_column(const int64_t index, ObRowkeyColumn& column) const;
      const ObRowkeyColumn *get_column(const int64_t index) const;

      /**
       * Get rowkey column id by index
       * @param[in]  index   column index in RowkeyInfo
       * @param[out] column_id in ObRowkeyInfo
       *
       * @return int  return OB_SUCCESS if get the column, otherwist return OB_ERROR
       */
      int get_column_id(const int64_t index, uint64_t & column_id) const;

      /**
       * Add column to rowkey info
       * @param column column to add
       * @return itn  return OB_SUCCESS if add success, otherwise return OB_ERROR
       */
      int add_column(const ObRowkeyColumn& column);

      int get_index(const uint64_t column_id, int64_t &index, ObRowkeyColumn& column) const;
      bool is_rowkey_column(const uint64_t column_id) const;
      int set_column(int64_t idx, const ObRowkeyColumn& column);

      int64_t to_string(char* buf, const int64_t buf_len) const;
      NEED_SERIALIZE_AND_DESERIALIZE;
    private:
      ObRowkeyColumn columns_[OB_MAX_ROWKEY_COLUMN_NUMBER];
      int64_t size_;
    };


    class ObOperator;
    class ObSchemaManagerV2;

    class ObColumnSchemaV2
    {
      public:
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
            struct ObDecimalHelper {

                uint32_t dec_precision_ :7;
                uint32_t dec_scale_ :6;
//                uint32_t reserved:19;
            };
            //add:e
        struct ObJoinInfo
        {
          ObJoinInfo() : join_table_(OB_INVALID_ID),left_column_count_(0) {}
          uint64_t join_table_;   // join table id
          uint64_t correlated_column_;  // column in joined table
          //this means which part of left_column_ to be used make rowkey
          uint64_t left_column_offset_array_[OB_MAX_ROWKEY_COLUMN_NUMBER];
          uint64_t left_column_count_;

          int64_t to_string(char *buf, const int64_t buf_len) const
          {
            int64_t pos = 0;
            databuff_printf(buf, buf_len, pos, "join_table[%lu], correlated_column[%lu], left_column_count[%lu]",
              join_table_, correlated_column_, left_column_count_);
            return pos;
          }
        };

        ObColumnSchemaV2();
        ~ObColumnSchemaV2() {}
        ObColumnSchemaV2& operator=(const ObColumnSchemaV2& src_schema);

        uint64_t    get_id()   const;
        const char* get_name() const;
        ColumnType  get_type() const;
        int64_t     get_size() const;
        uint64_t    get_table_id()        const;
        bool        is_maintained()       const;
        uint64_t    get_column_group_id() const;
        bool        is_join_column() const;
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        uint32_t get_precision() const;
        uint32_t get_scale() const;
        void set_precision(uint32_t pre);
        void set_scale(uint32_t scale);

        //add:e
        void set_table_id(const uint64_t id);
        void set_column_id(const uint64_t id);
        void set_column_name(const char *name);
        void set_column_name(const ObString& name);
        void set_column_type(const ColumnType type);
        void set_column_size(const int64_t size); //only used when type is varchar
        void set_column_group_id(const uint64_t id);
        void set_maintained(bool maintained);

        void set_join_info(const uint64_t join_table, const uint64_t* left_column_id,
            const uint64_t left_column_count, const uint64_t correlated_column);

        const ObJoinInfo* get_join_info() const;

        bool operator==(const ObColumnSchemaV2& r) const;

        static ColumnType convert_str_to_column_type(const char* str);

        //this is for test
        void print_info() const;
        void print(FILE* fd) const;

        NEED_SERIALIZE_AND_DESERIALIZE;

        int deserialize_v3(const char* buf, const int64_t data_len, int64_t& pos);
        int deserialize_v4(const char* buf, const int64_t data_len, int64_t& pos);

        inline bool is_nullable() const { return is_nullable_; }
        inline void set_nullable(const bool null) { is_nullable_ = null; }
        //add lbzhong [auto_increment] 20161124:b
        inline bool is_auto_increment() const { return auto_increment_; }
        inline void set_auto_increment(const bool auto_increment) { auto_increment_ = auto_increment; }
        //add:e
        inline const ObObj & get_default_value()  const { return default_value_; }
        inline void set_default_value(const ObObj& value) { default_value_ = value; }

      private:
        friend class ObSchemaManagerV2;
      private:
        bool maintained_;
        bool is_nullable_;
        //add lbzhong [auto_increment] 20161124:b
        bool auto_increment_;
        //add:e

        uint64_t table_id_;
        uint64_t column_group_id_;
        uint64_t column_id_;

        int64_t size_;  //only used when type is char or varchar
        ColumnType type_;
        char name_[OB_MAX_COLUMN_NAME_LENGTH];

        ObObj default_value_;
        //join info
        ObJoinInfo join_info_;
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b

        ObDecimalHelper ob_decimal_helper_;
        //add:e
        //in mem
        ObColumnSchemaV2* column_group_next_;
    };

    inline static bool column_schema_compare(const ObColumnSchemaV2& lhs, const ObColumnSchemaV2& rhs)
    {
      bool ret = false;
      if ( (lhs.get_table_id() < rhs.get_table_id()) ||
           (lhs.get_table_id() == rhs.get_table_id() && lhs.get_column_group_id() < rhs.get_column_group_id()) ||
           (lhs.get_table_id() == rhs.get_table_id() &&
            (lhs.get_column_group_id() == rhs.get_column_group_id()) && lhs.get_id() < rhs.get_id()) )
      {
        ret = true;
      }
      return ret;
    }

    struct ObColumnSchemaV2Compare
    {
      bool operator()(const ObColumnSchemaV2& lhs, const ObColumnSchemaV2& rhs)
      {
        return column_schema_compare(lhs, rhs);
      }
    };


    class ObTableSchema
    {
      public:
        ObTableSchema();
        ~ObTableSchema() {}
        ObTableSchema& operator=(const ObTableSchema& src_schema);
        enum TableType
        {
          INVALID = 0,
          SSTABLE_IN_DISK,
          SSTABLE_IN_RAM,
        };
        bool is_merge_dynamic_data() const;
        uint64_t    get_table_id()   const;
        TableType   get_table_type() const;
        const char* get_table_name() const;
        const char* get_compress_func_name() const;
        const char* get_comment_str() const;
        uint64_t    get_max_column_id() const;
        const char* get_expire_condition() const;
        const ObRowkeyInfo&  get_rowkey_info() const;
        ObRowkeyInfo& get_rowkey_info();
        int64_t     get_version() const;

        int32_t get_split_pos() const;
        int32_t get_rowkey_max_length() const;

        bool is_pure_update_table() const;
        bool is_use_bloomfilter()   const;
        bool is_read_static()   const;
        bool has_baseline_data() const;
        bool is_expire_effect_immediately() const;
        int32_t get_block_size()    const;
        int64_t get_max_sstable_size() const;
        int64_t get_expire_frequency()    const;
        int64_t get_query_cache_expire_time() const;
        int64_t get_max_scan_rows_per_tablet() const;
        int64_t get_internal_ups_scan_size() const;
        int64_t get_merge_write_sstable_version() const;
        int64_t get_replica_count() const;
        int64_t get_schema_version() const;

        uint64_t get_create_time_column_id() const;
        uint64_t get_modify_time_column_id() const;
        //longfei [create index]
        /**
         * @brief get_original_table_id
         * @return original_table_id_
         */
        uint64_t get_original_table_id() const;
        /**
         * @brief get_index_status
         * @return index_status_
         */
        IndexStatus get_index_status() const;

        void set_table_id(const uint64_t id);
        void set_max_column_id(const uint64_t id);
        void set_version(const int64_t version);

        void set_table_type(TableType type);
        void set_split_pos(const int64_t split_pos);

        void set_rowkey_max_length(const int64_t len);
        void set_block_size(const int64_t block_size);
        void set_max_sstable_size(const int64_t max_sstable_size);

        void set_table_name(const char* name);
        void set_table_name(const ObString& name);
        void set_compressor_name(const char* compressor);
        void set_compressor_name(const ObString& compressor);

        void set_pure_update_table(bool is_pure);
        void set_use_bloomfilter(bool use_bloomfilter);
        void set_read_static(bool read_static);
        void set_has_baseline_data(const bool base_data);
        void set_expire_effect_immediately(const int64_t expire_effect_immediately);

        void set_expire_condition(const char* expire_condition);
        void set_expire_condition(const ObString& expire_condition);

        void set_comment_str(const char* comment_str);
        void set_expire_frequency(const int64_t expire_frequency);
        void set_query_cache_expire_time(const int64_t expire_time);
        void set_rowkey_info(ObRowkeyInfo& rowkey_info);
        void set_max_scan_rows_per_tablet(const int64_t max_scan_rows);
        void set_internal_ups_scan_size(const int64_t scan_size);
        void set_merge_write_sstable_version(const int64_t version);
        void set_replica_count(const int64_t count) ;
        void set_schema_version(const int64_t version);

        void set_create_time_column(uint64_t id);
        void set_modify_time_column(uint64_t id);

        //longfei [create index]
        /**
         * @brief set_original_table_id
         * @param id
         */
        void set_original_table_id(uint64_t id);
        /**
         * @brief set_index_status
         * @param status
         */
        void set_index_status(IndexStatus status);

        ObConsistencyLevel get_consistency_level() const;
        void set_consistency_level(int64_t consistency_level);

        bool operator ==(const ObTableSchema& r) const;
        bool operator ==(const ObString& table_name) const;
        bool operator ==(const uint64_t table_id) const;

        NEED_SERIALIZE_AND_DESERIALIZE;
        //this is for test
        void print_info() const;
        void print(FILE* fd) const;
      private:
        int deserialize_v3(const char* buf, const int64_t data_len, int64_t& pos);
        int deserialize_v4(const char* buf, const int64_t data_len, int64_t& pos);
      private:
        static const int64_t TABLE_SCHEMA_RESERVED_NUM = 1;
        uint64_t table_id_;
        uint64_t max_column_id_;
        int64_t rowkey_split_;
        int64_t rowkey_max_length_;

        int32_t block_size_; //KB
        TableType table_type_;

        char name_[OB_MAX_TABLE_NAME_LENGTH];
        char compress_func_name_[OB_MAX_TABLE_NAME_LENGTH];
        char expire_condition_[OB_MAX_EXPIRE_CONDITION_LENGTH];
        char comment_str_[OB_MAX_TABLE_COMMENT_LENGTH];
        bool is_pure_update_table_;
        bool is_use_bloomfilter_;
        bool is_merge_dynamic_data_;
        ObConsistencyLevel consistency_level_;
        bool has_baseline_data_;
        ObRowkeyInfo rowkey_info_;
        int64_t expire_frequency_;  // how many frozen version passed before do expire once
        int64_t max_sstable_size_;
        int64_t query_cache_expire_time_;
        int64_t is_expire_effect_immediately_;
        int64_t max_scan_rows_per_tablet_;
        int64_t internal_ups_scan_size_;
        int64_t merge_write_sstable_version_;
        int64_t replica_count_;

        // longfei [create index]
        uint64_t original_table_id_; ///< index table's original table
        IndexStatus index_status_; ///< index status

        int64_t reserved_[TABLE_SCHEMA_RESERVED_NUM];
        int64_t version_;
        int64_t schema_version_;
        //in mem
        uint64_t create_time_column_id_;
        uint64_t modify_time_column_id_;

    };

    class ObSchemaSortByIdHelper;
    class ObSchemaManagerV2
    {
      public:
        enum Status{
          INVALID,
          CORE_TABLES,
          ALL_TABLES,
        };

        friend class ObSchemaSortByIdHelper;
      public:
        ObSchemaManagerV2();
        explicit ObSchemaManagerV2(const int64_t timestamp);
        ~ObSchemaManagerV2();
      public:
        ObSchemaManagerV2& operator=(const ObSchemaManagerV2& schema); //ugly,for hashMap
        ObSchemaManagerV2(const ObSchemaManagerV2& schema);
      public:
        ObSchemaManagerV2::Status get_status() const;
        const ObColumnSchemaV2* column_begin() const;
        const ObColumnSchemaV2* column_end() const;
        const char* get_app_name() const;
        int set_app_name(const char* app_name);
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        int get_cond_val_info(uint64_t tid,uint64_t cid,ObObjType &type,uint32_t &p,uint32_t &s,int32_t* idx = NULL) const;
        //add e
        int64_t get_column_count() const;
        int64_t get_table_count() const;

        uint64_t get_max_table_id() const;
        /**
         * @brief timestap is the version of schema,version_ is used for serialize syntax
         *
         * @return
         */
        int64_t get_version() const;
        void set_version(const int64_t version);
        void set_max_table_id(const uint64_t version);

        /**
         * @brife return code version not schema version
         *
         * @return int32_t
         */
        int32_t get_code_version() const;

        const ObColumnSchemaV2* get_column_schema(const int32_t index) const;

        /**
         * @brief get a column accroding to table_id/column_group_id/column_id
         *
         * @param table_id
         * @param column_group_id
         * @param column_id
         *
         * @return column or null
         */
        const ObColumnSchemaV2* get_column_schema(const uint64_t table_id,
                                                  const uint64_t column_group_id,
                                                  const uint64_t column_id) const;

        /**
         * @brief get a column accroding to table_id/column_id, if this column belongs to
         * more than one column group,then return the column in first column group
         *
         * @param table_id the id of table
         * @param column_id the column id
         * @param idx[out] the index in table of this column
         *
         * @return column or null
         */
        const ObColumnSchemaV2* get_column_schema(const uint64_t table_id,
                                                  const uint64_t column_id,
                                                  int32_t* idx = NULL) const;


        const ObColumnSchemaV2* get_column_schema(const char* table_name,
                                                  const char* column_name,
                                                  int32_t* idx = NULL) const;

        const ObColumnSchemaV2* get_column_schema(const ObString& table_name,
                                                  const ObString& column_name,
                                                  int32_t* idx = NULL) const;


        const ObColumnSchemaV2* get_table_schema(const uint64_t table_id, int32_t& size) const;

        const ObColumnSchemaV2* get_group_schema(const uint64_t table_id,
                                                 const uint64_t column_group_id,
                                                 int32_t& size) const;

        const ObTableSchema* table_begin() const;
        const ObTableSchema* table_end() const;

        const ObTableSchema* get_table_schema(const char* table_name) const;
        const ObTableSchema* get_table_schema(const ObString& table_name) const;
        const ObTableSchema* get_table_schema(const uint64_t table_id) const;
        ObTableSchema* get_table_schema(const char* table_name);
        ObTableSchema* get_table_schema(const uint64_t table_id);
        int64_t get_table_query_cache_expire_time(const ObString& table_name) const;

        uint64_t get_create_time_column_id(const uint64_t table_id) const;
        uint64_t get_modify_time_column_id(const uint64_t table_id) const;

        int get_column_index(const char *table_name,const char* column_name,int32_t index_array[],int32_t& size) const;
        int get_column_index(const uint64_t table_id, const uint64_t column_id, int32_t index_array[],int32_t& size) const;

        int get_column_schema(const uint64_t table_id, const uint64_t column_id,
                              ObColumnSchemaV2* columns[],int32_t& size) const;

        int get_column_schema(const char *table_name, const char* column_name,
                              ObColumnSchemaV2* columns[],int32_t& size) const;

        int get_column_schema(const ObString& table_name,
                              const ObString& column_name,
                              ObColumnSchemaV2* columns[],int32_t& size) const;


        int get_column_groups(uint64_t table_id,uint64_t column_groups[],int32_t& size) const;

        bool is_compatible(const ObSchemaManagerV2& schema_manager) const;

        int add_column(ObColumnSchemaV2& column);
        int add_column_without_sort(ObColumnSchemaV2& column);
        int add_table(ObTableSchema& table);
        void del_column(const ObColumnSchemaV2& column);

        /**
         * @brief if you don't want to use column group,set drop_group to true and call this
         *        method before deserialize
         *
         * @param drop_group true - don't use column group,otherwise not
         *
         */
        void set_drop_column_group(bool drop_group = true);

        // convert new table schema into old one and insert it into the schema_manager
        // zhuweng.yzf@taobao.com
        int add_new_table_schema(const TableSchema& tschema);
        //convert new table schema into old one and insert it into the schema_manager
        //rongxuan.lc@taobao.com
        int add_new_table_schema(const ObArray<TableSchema>& schema_array);

      //longfei [create index]
      public:
        /**
         * @brief get_id_index_hash
         * @return id_index_hash_map_
         */
        const hash::ObHashMap<uint64_t,IndexList,hash::NoPthreadDefendMode>*  get_id_index_hash() const;
        /**
         * @brief get_index_column_num
         * @param table_id
         * @param num
         * @return error code
         */
        int get_index_column_num(uint64_t& table_id,int64_t &num) const;
        /**
         * @brief add_index_in_map: put info into hash map
         * @param tschema
         * @return error code
         */
        int add_index_in_map(ObTableSchema *tschema);
        /**
         * @brief init_index_hash
         * @return error code
         */
        int init_index_hash();
        /**
         * @brief get_index_list
         * @param table_id
         * @param [out] out_list
         * @return error code
         */
        const int get_index_list(uint64_t table_id,IndexList& out_list) const;
        /**
         * @brief is_modify_expire_condition
         * @param table_id
         * @param cid
         * @return
         */
        bool is_modify_expire_condition(uint64_t table_id,uint64_t cid)const;
        /**
         * @brief get_init_index: 找出所有状态为init的索引表
         * @param table_id
         * @param size
         * @return error code
         */
        int get_init_index(uint64_t* table_id, int64_t& size) const;
        /**
         * @brief get_init_index: 找出所有状态为init的索引表
         * @param [out] index_list
         * @return
         */
        int get_init_index(ObArray<uint64_t>& index_list) const;
        /**
         * @brief is_index_has_storing
         * @param table_id
         * @return true: has storing or false: do not have
         */
        bool is_index_has_storing(uint64_t table_id) const;

        volatile bool isIsIdIndexHashMapInit() const {
          return is_id_index_hash_map_init_;
        }

      ///longfei [secondary index select]
      public:
        /**
         * @brief is_this_table_avalibale 判断tid为参数的表是否是可用的索引表
         * @param tid
         * @return error code
         */
        bool is_this_table_avalibale(uint64_t tid) const;
        /**
         * @brief is_cid_in_index: 找到以tid为参数的表的所有可用的索引表，第一主键为cid的索引表，存到数组index_tid_array
         * @param cid
         * @param tid
         * @param index_tid_array
         * @return
         */
        bool is_cid_in_index(
            uint64_t cid,
            uint64_t tid,
            uint64_t *index_tid_array) const;
        /**
         * @brief get_all_index_tid
         * @param [out] index_id_list
         * @return error code
         */
        int get_all_index_tid(ObArray<uint64_t> &index_id_list) const;
        /**
         * @brief get_all_notav_index_tid
         * @param [out] index_id_list
         * @return error code
         */
        int get_all_notav_index_tid(ObArray<uint64_t> &index_id_list) const;
        /**
         * @brief get_all_avaiable_index_list
         * @param [out] index_id_list
         * @return error code
         */
        int get_all_avaiable_index_list(ObArray<uint64_t> &index_id_list) const;

      public:
        bool parse_from_file(const char* file_name, tbsys::CConfig& config);
        bool parse_one_table(const char* section_name, tbsys::CConfig& config, ObTableSchema& schema);
        bool parse_column_info(const char* section_name, tbsys::CConfig& config, ObTableSchema& schema);
        bool parse_join_info(const char* section_name, tbsys::CConfig& config, ObTableSchema& schema);
        bool parse_rowkey_info(const char* section_name, tbsys::CConfig& config, ObTableSchema& schema);
        int change_table_id(const uint64_t table_id, const uint64_t new_table_id);
        int write_to_file(const char* file_name);
        int write_table_to_file(FILE *fd, const int64_t table_index);
        int write_column_group_info_to_file(FILE *fd, const int64_t table_index);
        int write_column_info_to_file(FILE *fd, const ObColumnSchemaV2 *column_schema);
        int write_rowkey_info_to_file(FILE *fd, const uint64_t table_id, const ObRowkeyInfo &rowkey);

      private:
        /**
         * parse rowkey column description into ObRowkeyColumn
         * rowkey column description may contains compatible old binary rowkey structure.
         * @param column_str column description string.
         * @param [out] column rowkey column id, length, data type.
         * @param [in, out] schema check column if exist, and set rowkey info.
         */
        bool parse_rowkey_column(const char* column_str, ObRowkeyColumn& column,  ObTableSchema& schema);

        /**
         * check join column if match with joined table's rowkey column.
         */
        bool check_join_column(const int32_t column_index, const char* column_name, const char* join_column_name,
            ObTableSchema& schema, const ObTableSchema& join_table_schema, uint64_t& column_offset);

      public:
        void print_info() const;
        void print(FILE* fd) const;
      public:
        struct ObColumnNameKey
        {
          int64_t hash() const;
          bool operator==(const ObColumnNameKey& key) const;
          ObString table_name_;
          ObString column_name_;
        };

        struct ObColumnIdKey
        {
          int64_t hash() const;
          bool operator==(const ObColumnIdKey& key) const;
          uint64_t table_id_;
          uint64_t column_id_;
        };

        struct ObColumnInfo
        {
          ObColumnSchemaV2* head_;
          int32_t table_begin_index_;
          ObColumnInfo() : head_(NULL), table_begin_index_(-1) {}
        };

        struct ObColumnGroupHelper
        {
          uint64_t table_id_;
          uint64_t column_group_id_;
        };

        struct ObColumnGroupHelperCompare
        {
          bool operator() (const ObColumnGroupHelper& l,const ObColumnGroupHelper& r) const;
        };

      public:
        NEED_SERIALIZE_AND_DESERIALIZE;
        int sort_column();
        bool check_table_expire_condition() const;
        bool check_compress_name() const;
        static const int64_t MAX_COLUMNS_LIMIT = OB_MAX_TABLE_NUMBER * OB_MAX_COLUMN_NUMBER;
        static const int64_t DEFAULT_MAX_COLUMNS = 16 * OB_MAX_COLUMN_NUMBER;
        //add maoxx
        /**
         * @brief get_all_modifiable_index
         * get a list of modifiable index of a table
         * @param table_id
         * @param modifiable_index_list
         * @return OB_SUCCESS or other ERROR
         */
        int get_all_modifiable_index(uint64_t table_id, IndexList &modifiable_index_list) const;

        /**
         * @brief is_have_modifiable_index
         * decide if the table has modifiable index
         * @param table_id
         * @return true if have or false if not
         */
        bool is_have_modifiable_index(uint64_t table_id) const;

        /**
         * @brief is_have_init_index
         * decide if the table has initialized index
         * @param table_id
         * @return true if have or false if not
         */
        bool is_have_init_index(uint64_t table_id) const;

        /**
         * @brief column_hit_index
         * get a list of index of a given table which consist of a given column
         * @param table_id
         * @param cid
         * @param hit_index_list
         * @return OB_SUCCESS or other ERROR
         */
        int column_hit_index(uint64_t table_id, uint64_t cid, IndexList &hit_index_list) const;

        /**
         * @brief column_hit_index
         * decide if the given column hit the index of the given table
         * @param table_id
         * @param cid
         * @param column_hit_index_flag
         * @return OB_SUCCESS or other ERROR
         */
        int column_hit_index(uint64_t table_id, uint64_t cid, bool &column_hit_index_flag) const;

        /**
         * @brief column_hit_index_and_rowkey
         * decide if the given column hit the index of the given table and is the rowkey of this index
         * @param table_id
         * @param cid
         * @param hit_flag
         * @return OB_SUCCESS or other ERROR
         */
        int column_hit_index_and_rowkey(uint64_t table_id, uint64_t cid, bool &hit_flag) const;
        //add e

      private:
        int replace_system_variable(char* expire_condition, const int64_t buf_size) const;
        int check_expire_dependent_columns(const ObString& expr,
          const ObTableSchema& table_schema, ObExpressionParser& parser) const;
        int ensure_column_storage();
        int prepare_column_storage(const int64_t column_num, bool need_reserve_space = false);

      private:
        int32_t   schema_magic_;
        int32_t   version_;
        int64_t   timestamp_;
        uint64_t  max_table_id_;
        int64_t   column_nums_;
        int64_t   table_nums_;
        //longfei [create index]
        volatile bool is_id_index_hash_map_init_; ///< is id_index_hash_map init?

        char app_name_[OB_MAX_APP_NAME_LENGTH];

        ObTableSchema    table_infos_[OB_MAX_TABLE_NUMBER];
        ObColumnSchemaV2 *columns_;
        int64_t   column_capacity_; // current %columns_ occupy size.

        //just in mem
        bool drop_column_group_; //
        volatile bool hash_sorted_;       //after deserialize,will rebuild the hash maps
        hash::ObHashMap<ObColumnNameKey,ObColumnInfo,hash::NoPthreadDefendMode> column_hash_map_;
        hash::ObHashMap<ObColumnIdKey,ObColumnInfo,hash::NoPthreadDefendMode> id_hash_map_;

        //longfei [create index]
        hash::ObHashMap<uint64_t, IndexList,hash::NoPthreadDefendMode> id_index_hash_map_; ///< <table_id,index_list> hash map

        int64_t column_group_nums_;
        ObColumnGroupHelper column_groups_[OB_MAX_COLUMN_GROUP_NUMBER * OB_MAX_TABLE_NUMBER];
    };

    class ObSchemaSortByIdHelper
    {
      public:
        explicit ObSchemaSortByIdHelper(const ObSchemaManagerV2* schema_manager)
        {
          init(schema_manager);
        }
        ~ObSchemaSortByIdHelper()
        {
        }
      public:
        struct Item
        {
          int64_t table_id_;
          int64_t index_;
          bool operator<(const Item& rhs) const
          {
            return table_id_ < rhs.table_id_;
          }
        };
      public:
        const ObTableSchema* get_table_schema(const Item* item) const
        {
          return &schema_manager_->table_infos_[item->index_];
        }
        const Item* begin() const { return table_infos_index_; }
        const Item* end() const { return table_infos_index_ + table_nums_; }

      private:
        inline int init(const ObSchemaManagerV2* schema_manager)
        {
          int ret = OB_SUCCESS;
          if (NULL == schema_manager)
          {
            ret = OB_INVALID_ARGUMENT;
          }
          else
          {
            schema_manager_ = schema_manager;
            memset(table_infos_index_, 0, sizeof(table_infos_index_));
            int64_t i = 0;
            for (; i < schema_manager_->table_nums_; ++i)
            {
              table_infos_index_[i].table_id_ = schema_manager_->table_infos_[i].get_table_id();
              table_infos_index_[i].index_ = i;
            }
            table_nums_ = i;
            std::sort(table_infos_index_, table_infos_index_ + table_nums_);
          }
          return ret;
        }

        Item table_infos_index_[OB_MAX_TABLE_NUMBER];
        int64_t table_nums_;
        const ObSchemaManagerV2* schema_manager_;

    };
  }
}
#endif
