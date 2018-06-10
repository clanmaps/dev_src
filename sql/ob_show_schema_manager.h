/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_show_schema_manager.h
 * @brief schema for show statement
 *
 * modified by longfei：
 * 1.add function: add_show_index_schema()
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_22
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
 * ob_show_schema_manager.h
 *
 * Authors:
 *   Guibin Du <tianguan.dgb@taobao.com>
 *
 */
#ifndef OCEANBASE_SQL_OB_ENSURE_SHOW_SCHEMAS_H_
#define OCEANBASE_SQL_OB_ENSURE_SHOW_SCHEMAS_H_
#include "sql/ob_basic_stmt.h"
#include "common/ob_schema.h"

namespace oceanbase
{
  namespace sql
  {
    class ObShowSchemaManager
    {
      public:
        static const common::ObSchemaManagerV2* get_show_schema_manager();

      private:
        // disallow
        ObShowSchemaManager() {}
        ObShowSchemaManager(const ObShowSchemaManager &other);
        ObShowSchemaManager& operator=(const ObShowSchemaManager &other);

        static int add_show_schemas(common::ObSchemaManagerV2& schema_mgr);
        static int add_show_schema(common::ObSchemaManagerV2& schema_mgr, int32_t stmt_type);
        static int add_show_tables_schema(common::ObSchemaManagerV2& schema_mgr);
        /**
         * @brief ObShowSchemaManager::add_show_index_schema: add virtual table __index_show's schema to schema_mgr
         * @param schema_mgr
         * @author longfei <longfei@stu.ecnu.edu.cn>
         * @return error code
         */
        static int add_show_index_schema(common::ObSchemaManagerV2& schema_mgr);
        //add:e
        static int add_show_variables_schema(common::ObSchemaManagerV2& schema_mgr);
        static int add_show_columns_schema(common::ObSchemaManagerV2& schema_mgr);
        static int add_show_create_table_schema(common::ObSchemaManagerV2& schema_mgr);
        static int add_show_parameters_schema(common::ObSchemaManagerV2& schema_mgr);
        static int add_show_table_status_schema(common::ObSchemaManagerV2& schema_mgr);


      private:        
        static common::ObSchemaManagerV2 *show_schema_mgr_;
        static tbsys::CThreadMutex mutex_;
    };
  }
}
#endif /* OCEANBASE_SQL_OB_ENSURE_SHOW_SCHEMAS_H_ */

