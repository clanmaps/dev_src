/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_elseif_stmt.h
 * @brief the ObProcedureElseIfStmt class definition that warp procedure elseif statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_ELSEIF_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_ELSEIF_STMT_H_
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
     * @brief The ObProcedureElseIfStmt class
     * procedure elseif statement class definition
     */
    class ObProcedureElseIfStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureElseIfStmt() :
            ObBasicStmt(T_PROCEDURE_ELSEIF)
        {
          expr_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureElseIfStmt()
        {
        }
        /**
         * @brief print
         * print procedure elseif statement information
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
         * @brief add_elseif_then_stmt
         * add an elseif statement
         * @param stmt_id  elseif statement id
         * @return error code
         */
        int add_elseif_then_stmt(uint64_t& stmt_id);
        /**
         * @brief get_expr_id
         * get elseif expression id
         * @return elseif expression id
         */
        uint64_t get_expr_id();
        /**
         * @brief get_then_stmts
         * get then statement id array
         * @return then statement id array
         */
        ObArray<uint64_t> get_then_stmts();
        /**
         * @brief get_then_stmt
         * get then statement id by array index
         * @param index array index
         * @return then statement id
         */
        uint64_t& get_then_stmt(int64_t index);
        /**
         * @brief get_then_stmt_size
         * get then statement array size
         * @return then statement array number
         */
        int64_t get_then_stmt_size();

      private:
        uint64_t expr_id_;	///<  elseif expression id
        ObArray<uint64_t> elseif_then_stmts_;  ///<  else if then

    };
  }
}

#endif
