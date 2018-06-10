/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_else_stmt.h
 * @brief the ObProcedureElseStmt class definition that warp procedure else statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_ELSE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_ELSE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    /**
    * @brief The ObProcedureElseStmt class
    * procedure else statement class definition
    */
    class ObProcedureElseStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureElseStmt() :
            ObBasicStmt(T_PROCEDURE_ELSE)
        {
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureElseStmt()
        {
        }
        /**
         * @brief print
         * print procedure else statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief add_else_stmt
         * add a else statement
         * @param stmt_id else statement id
         * @return error code
         */
        int add_else_stmt(uint64_t& stmt_id);
        /**
         * @brief get_else_stmts
         * get else statement array
         * @return else statement array
         */
        const ObArray<uint64_t> &get_else_stmts() const;
        /**
         * @brief get_else_stmt
         * get else statement id  by array index
         * @param index array index
         * @return else statement id
         */
        uint64_t& get_else_stmt(int64_t index);
        /**
         * @brief get_else_stmt_size
         * get else statement size
         * @return else statement number
         */
        int64_t get_else_stmt_size();

      private:
        ObArray<uint64_t> else_stmts_;  ///<	else stmt
    };
  }
}

#endif
