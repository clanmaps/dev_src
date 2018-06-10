/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_extra_tables_schema.h
 * @brief define schema of core table and system table
 *
 * modified by longfei：add an core table: "__all_secondary_index" for storing secondary index table
 * modified by maoxiaoxiao:add system table "__all_column_checksum_info" and "__index_service_info"
 * modified by zhujun：add procedure related system table '__all_procedure' schema
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author zhujun <51141500091@ecnu.edu.cn>
 * @date 2016_01_21
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_extra_tables_schema.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *   Zhidong SUN  <xielun.szd@alipay.com>
 *
 */
#ifndef _OB_EXTRA_TABLES_SCHEMA_H
#define _OB_EXTRA_TABLES_SCHEMA_H 1

#include "ob_schema_service_impl.h"

namespace oceanbase
{
  namespace common
  {
    /////////////////////////////////////////////////////////
    // WARNING: the sys table schema can not modified      //
    /////////////////////////////////////////////////////////
    class ObExtraTablesSchema
    {
    public:
      static const int64_t TEMP_ROWKEY_LENGTH = 64;
      static const int64_t SERVER_TYPE_LENGTH = 16;
      static const int64_t SERVER_IP_LENGTH = 32;
      // core tables
      static int first_tablet_entry_schema(TableSchema& table_schema);
      static int all_all_column_schema(TableSchema& table_schema);
      static int all_join_info_schema(TableSchema& table_schema);
      /**
       * @brief all_secondary_index_schema: for __all_secondary_index's schema
       * @param table_schema
       * @return error code
       */
      static int all_secondary_index_schema(TableSchema& table_schema); //longfei [create index]
    public:
      // other sys tables
      static int all_sys_stat_schema(TableSchema &table_schema);
      static int all_sys_param_schema(TableSchema &table_schema);
      static int all_sys_config_schema(TableSchema &table_schema);
      static int all_sys_config_stat_schema(TableSchema &table_schema);
      static int all_user_schema(TableSchema &table_schema);
      static int all_table_privilege_schema(TableSchema &table_schema);
      static int all_trigger_event_schema(TableSchema& table_schema);
      static int all_cluster_schema(TableSchema& table_schema);
      static int all_server_schema(TableSchema& table_schema);
      static int all_client_schema(TableSchema& table_schema);
      static int all_truncate_op_info(TableSchema &table_schema); //add hxlong [Truncate Table]:20170318
      //add lbzhong [auto_increment] 20161126:b
      static int all_auto_increment_schema(TableSchema &table_schema);
      //add:e
      // virtual sys tables
      static int all_server_stat_schema(TableSchema &table_schema);
      static int all_server_session_schema(TableSchema &table_schema);
      static int all_statement_schema(TableSchema &table_schema);
      //add maoxx
      /**
       * @brief all_index_service_schema
       * construct schema of system table for index service information
       * @param table_schema
       * @return OB_SUCCESS or other ERROR
       */
      static int all_index_service_schema(TableSchema &table_schema);

      static int all_column_checksum_info(TableSchema &table_schema);

      /**
       * @brief all_column_checksum_stat
       * construct schema of system table for column checksum information
       * @param table_schema
       * @return OB_SUCCESS or other ERROR
       */
      static int all_column_checksum_stat(TableSchema &table_schema);
      //add e
	  
	  //add zhujun[2015-3-11]:b
      static int all_procedure_schema(TableSchema &table_schema);
      //add:e
    private:
      ObExtraTablesSchema();
    };
  } // end namespace common
} // end namespace oceanbase

#endif /* _OB_EXTRA_TABLES_SCHEMA_H */
