/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_schema_service_impl.h
 * @brief implementation of schema service
 *
 * modified by longfei：add some function for new a core table
 * modified by WengHaixing: add some funcfion for secondary index status/columnchecksum
 * modified by maoxiaoxiao:add implementations of functions to check column checksum, clean column checksum and get column checksum
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author Weng Haixing <wenghaixing@ecnu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_26
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
 * ob_schema_service_impl.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *
 */

#ifndef _OB_SCHEMA_SERVICE_IMPL_H
#define _OB_SCHEMA_SERVICE_IMPL_H

#include "common/ob_table_id_name.h"
#include "common/ob_schema_service.h"
#include "common/hash/ob_hashmap.h"
//add maoxx
#include "common/ob_column_checksum.h"
//add e
//add by wangdonghui
#include "common/nb_accessor/nb_query_res.h"
//add e
class TestSchemaService_assemble_table_Test;
class TestSchemaTable_generate_new_table_name_Test;
class TestSchemaService_assemble_column_Test;
class TestSchemaService_assemble_join_info_Test;
class TestSchemaService_create_table_mutator_Test;
class TestSchemaService_get_table_name_Test;
#define OB_STR(str) \
  ObString(0, static_cast<int32_t>(strlen(str)), const_cast<char *>(str))

namespace oceanbase
{
  namespace common
  {
    static ObString first_tablet_entry_name = OB_STR(FIRST_TABLET_TABLE_NAME);
    static ObString column_table_name = OB_STR(OB_ALL_COLUMN_TABLE_NAME);
    static ObString joininfo_table_name = OB_STR(OB_ALL_JOININFO_TABLE_NAME);
    static ObString privilege_table_name = OB_STR(OB_ALL_TABLE_PRIVILEGE_TABLE_NAME);
    static ObString secondary_index_table_name = OB_STR(OB_ALL_SECONDAYR_INDEX_TABLE_NAME);//longfei [create index]
    //add lbzhong [auto_increment] 20161211:b
    static ObString auto_increment_table_name = OB_STR(OB_ALL_AUTO_INCREMENT_TABLE_NAME);
    //add:e
    static ObString table_name_str = OB_STR("table_id");
    //add by wangdonghui 20160125 :b
    static ObString procedure_table_name = OB_STR(OB_ALL_PROCEDURE_TABLE_NAME);  ///< static string procedure table name
    //add :e
    static const char* const TMP_PREFIX = "tmp_";

    class ObSchemaServiceImpl : public ObSchemaService
    {
      public:
        ObSchemaServiceImpl();
        virtual ~ObSchemaServiceImpl();

        int init(ObScanHelper* client_proxy, bool only_core_tables);

        virtual int get_table_schema(const ObString& table_name, TableSchema& table_schema);
        //add by wangdonghui 20160308 :b
        /**
         * @brief get_procedure_info
         * scan OB_ALL_PROCEDURE_TABLE
         * @param res_  return Query Result
         * @return error code
         */
        virtual int get_procedure_info(common::nb_accessor::QueryRes *&res_);
        //add :e
        virtual int create_table(const TableSchema& table_schema);
        virtual int drop_table(const ObString& table_name);
        virtual int alter_table(const AlterTableSchema& table_schema, const int64_t old_schema_version);
        virtual int get_table_id(const ObString& table_name, uint64_t& table_id);
        virtual int get_table_name(uint64_t table_id, ObString& table_name);
        virtual int modify_table_id(TableSchema& table_schema, const int64_t new_table_id);
        virtual int get_max_used_table_id(uint64_t &max_used_tid);
        virtual int set_max_used_table_id(const uint64_t max_used_tid);
        virtual int prepare_privilege_for_table(const nb_accessor::TableRow* table_row,
            ObMutator *mutator, const int64_t table_id);
        //add by wangdonghui 20160125 :b
        /**
         * @brief create_procedure
         * create procedure mutator and execute
         * @param proc_name procedure name
         * @param proc_source_code procedure source name
         * @return error code
         */
        virtual int create_procedure(const common::ObString& proc_name, const common::ObString & proc_source_code);
        //add :e
        //add by wangdonghui 20160225 [drop procedure] :b
        /**
         * @brief drop_procedure
         * delete row from __all_procedure
         * @param proc_name procedure name
         * @return error code
         */
        int drop_procedure(const ObString& proc_name);
        //add :e
        friend class ::TestSchemaService_assemble_table_Test;
        friend class ::TestSchemaService_assemble_column_Test;
        friend class ::TestSchemaService_assemble_join_info_Test;
        friend class ::TestSchemaService_create_table_mutator_Test;
        friend class ::TestSchemaService_get_table_name_Test;

      private:
        bool check_inner_stat();
        // for read
        int fetch_table_schema(const ObString& table_name, TableSchema& table_schema);
        int create_table_mutator(const TableSchema& table_schema, ObMutator* mutator);

        //add by wangdonghui 20160125 :b
        /**
         * @brief create_procedure_mutator
         * create procedure mutator
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @param mutator point generated mutator
         * @return error code
         */
        int create_procedure_mutator(const common::ObString & proc_name, const common::ObString & proc_source_code, ObMutator* mutator);
        //add :e
        int alter_table_mutator(const AlterTableSchema& table_schema, ObMutator* mutator, const int64_t old_schema_version);
        int assemble_table(const nb_accessor::TableRow* table_row, TableSchema& table_schema);
        int assemble_column(const nb_accessor::TableRow* table_row, ColumnSchema& column);
        int assemble_join_info(const nb_accessor::TableRow* table_row, JoinInfo& join_info);
        int init_id_name_map(ObTableIdNameIterator& iterator);
        // for update
        int add_join_info(ObMutator* mutator, const TableSchema& table_schema);
        int add_column(ObMutator* mutator, const TableSchema& table_schema);
        int update_column_mutator(ObMutator* mutator, ObRowkey & rowkey, const ColumnSchema & column);
        int reset_column_id_mutator(ObMutator* mutator, const AlterTableSchema & schema, const uint64_t max_column_id);
        int reset_schema_version_mutator(ObMutator* mutator, const AlterTableSchema & schema, const int64_t old_schema_version);
        int init_id_name_map();
        int generate_new_table_name(char* buf, const uint64_t lenght, const char* table_name, const uint64_t table_name_length);
        //add lbzhong [auto_increment] 20161201:b
        int check_auto_increment(const TableSchema& table_schema, uint64_t& column_id);
        int auto_increment_mutator(const uint64_t table_id, const uint64_t column_id, ObMutator* mutator);
        int fetch_auto_column_id(const uint64_t table_id, uint64_t& auto_column_id);
        int delete_auto_increment(const int64_t table_id);
        //add:e

      // longfei [create index]
      // secondary index service
      /**
       * @brief create_index_mutator: 根据table_schema构造mutator
       * @param [in] table_schema
       * @param [out] mutator
       * @return error code
       */
      int create_index_mutator(const TableSchema& table_schema, ObMutator* mutator);
      /**
       * @brief is_index_table_or_not: 判断是否是索引表
       * @param table_name
       * @return true for yes or false for not
       */
      bool is_index_table_or_not(const ObString& table_name);
      /**
       * @brief assemble_index_table: get schema from table row
       * @param [in] table_row
       * @param [out] table_schema
       * @return
       */
      int assemble_index_table(const nb_accessor::TableRow* table_row, TableSchema& table_schema);
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
      virtual int check_column_checksum(const int64_t orginal_table_id, const int64_t index_table_id, const int64_t cluster_id, const int64_t current_version, bool &column_checksum_flag);

      /**
       * @brief clean_column_checksum
       * clean column checksum
       * @param max_draution_of_version
       * @param current_version
       * @return OB_SUCCESS or other ERROR
       */
      virtual int clean_column_checksum(const int64_t max_draution_of_version, const int64_t current_version);

      /**
       * @brief get_column_checksum
       * get column checksum
       * @param range
       * @param cluster_id
       * @param required_version
       * @param column_checksum
       * @return OB_SUCCESS or other ERROR
       */
      virtual int get_column_checksum(const ObNewRange range, const int64_t cluster_id, const int64_t required_version, ObString& column_checksum);
      //add e
      //add wenghaixing [secondary index.static_index]20151217
      virtual int get_index_stat(const uint64_t table_id, const int64_t cluster_count, IndexStatus &stat);
      //virtual int fetch_index_stat(const uint64_t table_id, const int64_t cluster_id, int64_t &stat);
      virtual int get_cluster_count(int64_t &cc);
      //add e



      private:
        static const int64_t TEMP_VALUE_BUFFER_LEN = 32;
        ObScanHelper* client_proxy_;
        nb_accessor::ObNbAccessor nb_accessor_;
        hash::ObHashMap<uint64_t, ObString> id_name_map_;
        ObStringBuf string_buf_;
        tbsys::CThreadMutex string_buf_write_mutex_;
        tbsys::CThreadMutex mutex_;
        bool is_id_name_map_inited_;
        bool only_core_tables_;
    };
  }
}

#endif /* _OB_SCHEMA_SERVICE_IMPL_H */
