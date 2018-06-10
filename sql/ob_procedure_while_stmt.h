/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_while_stmt.h
 * @brief the ObProcedureWhileStmt class definition that warp procedure while statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_29
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_WHILE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_WHILE_STMT_H_
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
     * @brief The ObProcedureWhileStmt class
     * procedure while statement class definition
     */
    class ObProcedureWhileStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureWhileStmt() :
            ObBasicStmt(T_PROCEDURE_WHILE)
        {
          expr_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureWhileStmt()
        {
        }
        /**
         * @brief print
         * print procedure while statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief set_expr_id
         * set elseif expression id
         * @param expr_id elseif expression id
         * @return error code
         */
        int set_expr_id(uint64_t& expr_id);
        /**
         * @brief add_do_stmt
         * add a do statement
         * @param stmt_id  do statement id
         * @return error code
         */
        int add_do_stmt(uint64_t& stmt_id);
        /**
         * @brief get_expr_id
         * get while expression id
         * @return while expression id
         */
        uint64_t get_expr_id();
        /**
         * @brief get_do_stmts
         * get do statement id array
         * @return  do statement id array
         */
        ObArray<uint64_t> get_do_stmts();
        /**
         * @brief get_do_stmt
         * get do statement id by array index
         * @param index array index
         * @return do statement id
         */
        uint64_t& get_do_stmt(int64_t index);
        /**
         * @brief get_do_stmt_size
         * get so statement array size
         * @return array size
         */
        int64_t get_do_stmt_size();

      private:
        uint64_t expr_id_;	///< while expression id
        ObArray<uint64_t> while_do_stmts_;	///< while do statement array
    };
  }
}
#endif
