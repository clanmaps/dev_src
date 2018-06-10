/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_drop_stmt.h
 * @brief the ObProcedureDropStmt class definition that warp procedure drop statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_DROP_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_DROP_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include <map>
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObProcedureDropStmt class
     * drop procedure statement warper
     */
    class ObProcedureDropStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureDropStmt() :
            ObBasicStmt(T_PROCEDURE_DROP)
        {
          proc_delete_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureDropStmt() {
        }
        /**
         * @brief set_proc_name
         * set procedure name
         * @param proc_name procedure name
         * @return error code
         */
        int set_proc_name(ObString &proc_name);
        /**
         * @brief get_proc_name
         * get procedure name
         * @return procedure name
         */
        ObString& get_proc_name();
        /**
         * @brief set_proc_delete_id
         * set delete procedure statement id
         * @param stmt_id statement id
         * @return error code
         */
        int set_proc_delete_id(uint64_t& stmt_id);
        /**
         * @brief get_proc_delete_id
         * get delete procedure statement id
         * @return statement id
         */
        uint64_t& get_proc_delete_id();
        /**
         * @brief set_if_exists
         * set if exists flag
         * @param flag bool value
         */
        void set_if_exists(bool flag);
        /**
         * @brief if_exists
         * get if exists flag
         * @return bool value
         */
        bool if_exists();
        /**
         * @brief print
         * print procedure statement info
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);

      private:
        ObString proc_name_;  ///<  procedure name
        uint64_t proc_delete_id_;  ///<  delete procedure statement id
        bool  if_exists_;  ///<  if exists flag
    };

  }
}

#endif
