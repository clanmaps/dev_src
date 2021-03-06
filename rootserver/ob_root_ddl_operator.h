/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_ddl_operator.h
 * @brief for operations of table in rootserver
 *
 * modified by Wenghaixing:add some fuction for secondary index construction
 * modified by wangdonghui:add some functions for procedure create and drop in rs
 * @version __DaSE_VERSION
 * @author Wenghaixing <wenghaixing@ecnu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date  2016_07_26
 */

/*
 * Copyright (C) 2007-2012 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Description here
 *
 * Version: $Id$
 *
 * Authors:
 *   zhidong <xielun.szd@taobao.com>
 *     - some work details here
 */

#ifndef _OB_ROOT_DDL_OPERATOR_H_
#define _OB_ROOT_DDL_OPERATOR_H_

#include "tbsys.h"
#include "common/ob_define.h"
#include "common/ob_schema_service.h"

namespace oceanbase
{
  namespace common
  {
    class ObString;
    class TableSchema;
  }
  namespace rootserver
  {
    class ObRootServer2;
    class ObRootDDLOperator
    {
    public:
      ObRootDDLOperator();
      virtual ~ObRootDDLOperator();
    public:
      void init(ObRootServer2 * server, common::ObSchemaService * service);
      // create a new table
      int create_table(const common::TableSchema & table_schema);
      // delete a exist table
      int drop_table(const common::ObString & table_name);
      // alter table schema
      int alter_table(common::AlterTableSchema & table_schema);
      int modify_table_id(common::TableSchema &table_schema, const int64_t new_table_id);
      // update max used table id
      int update_max_table_id(const uint64_t table_id);
      //add by wangdonghui 20160125 :b
      /**
       * @brief create_procedure
       * insert procedure to table __all_procedure
       * @param proc_name procedure name
       * @param proc_sourcr_code procedure source code
       * @return error code
       */
      int create_procedure(const common::ObString & proc_name, const common::ObString & proc_sourcr_code);
      //add :e
	  //add by wangdonghui 20160225 [drop procedure] :b
      /**
       * @brief drop_procedure
       * delete procedure form __all_procedure
       * @param proc_name procedure name
       * @return error code
       */
      int drop_procedure(const common::ObString & proc_name);
      //add :e
      //add wenghaixing [secondary index.static_index]20151217
      tbsys::CThreadMutex &get_ddl_lock(){return mutex_lock_;}
      //add e
    private:
      bool check_inner_stat(void) const;
      // read max table id and modify table schema
      int allocate_table_id(common::TableSchema & talbe_schema);
      // allocate table id and column ids
      int modify_table_schema(const uint64_t table_id, common::TableSchema & table_schema);
      // update schema table insert new table schema content
      bool insert_schema_table(const common::TableSchema & table_schema);
      // update schema table delete old table schema content
      bool delete_schema_table(const common::ObString & table_name, uint64_t & table_id);
      // delete tablet from root table
      int delete_root_table(const uint64_t table_id);
      // create empty tablet
      int create_empty_tablet(const common::TableSchema & table_schema);
      // alter table schema
      int alter_table_schema(const common::TableSchema & schema, common::AlterTableSchema & table_schema);
      typedef common::AlterTableSchema::AlterColumnSchema AlterColumn;
      // check whether the column can alter
      int check_alter_column(const common::TableSchema & schema, AlterColumn & column);
      // allocate column id
      int set_column_info(const common::TableSchema & schema, const char * column_name,
          uint64_t & max_column_id, AlterColumn & column);

      //add by wangdonghui 20160125 :b
      /**
       * @brief insert_procedure_table
       * insert new procedure called by create_procedure function
       * @param proc_name procedure name
       * @param proc_source_code procedure source code
       * @return error code
       */
      bool insert_procedure_table(const common::ObString & proc_name, const common::ObString & proc_source_code);
      //add :e

      //add by wangdonghui 20160225[drop procedure] :b
      /**
       * @brief delete_procedure
       * delete procedure called by drop_procedure function
       * @param proc_name procedure name
       * @return error code
       */
      bool delete_procedure(const common::ObString & proc_name);
      //add :e
    private:
      // not support parallel ddl operation
      tbsys::CThreadMutex mutex_lock_;
      // schema inner table operator tool
      common::ObSchemaService * schema_client_;
      // select cs and create tablet tool
      // root table operator tool
      ObRootServer2 * root_server_;
    };
    //
    inline bool ObRootDDLOperator::check_inner_stat(void) const
    {
      return ((schema_client_ != NULL) && (root_server_ != NULL));
    }
  }
}



#endif // _OB_ROOT_DDL_OPERATOR_H_
