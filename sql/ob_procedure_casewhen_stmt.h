/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_casewhen_stmt.h
 * @brief the ObProcedureCaseWhenStmt class definition that warp procedure casewhen statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_CASEWHEN_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_CASEWHEN_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include <map>
using namespace oceanbase::common;

namespace oceanbase {
  namespace sql {
    /**
     * @brief The ObProcedureCaseWhenStmt class
     * procedure casewhen statement warper
     */
    class ObProcedureCaseWhenStmt: public ObBasicStmt {
      public:
        /**
         * @brief constructor
         */
        ObProcedureCaseWhenStmt() :
            ObBasicStmt(T_PROCEDURE_CASEWHEN) {
        expr_id_=common::OB_INVALID_ID;
        case_value_expr_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureCaseWhenStmt() {
        }
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief set_expr_id
         * set case expression id
         * @param expr_id  expression id
         * @return error code
         */
        int set_expr_id(uint64_t& expr_id);
        /**
         * @brief set_case_value_expr
         * set when expression id
         * @param expr_id expression id
         * @return
         */
        int set_case_value_expr(uint64_t& expr_id);
        /**
         * @brief get_expr_id
         * get case expression id
         * @return expression id
         */
        uint64_t get_expr_id();
        /**
         * @brief get_case_value_expr
         * get when expression id
         * @return expression id
         */
        uint64_t get_case_value_expr();
        /**
         * @brief add_then_stmt
         * add a then statement id
         * @param stmt_id statement id
         * @return error code
         */
        int add_then_stmt(uint64_t& stmt_id);
        /**
         * @brief get_then_stmts
         * get then statement id array
         * @return statement id array
         */
        const ObArray<uint64_t>& get_then_stmts() const;
        /**
         * @brief get_then_stmt
         * get a then statement id by array index
         * @param index array index
         * @return then statement id
         */
        uint64_t& get_then_stmt(int64_t index);
        /**
         * @brief get_then_stmt_size
         * get then statement size
         * @return statement number
         */
        int64_t get_then_stmt_size();

      private:
        uint64_t expr_id_;  ///<  case expression id
        uint64_t case_value_expr_;  ///<  when expression id
        ObArray<uint64_t> when_then_stmts_;  ///<  when then statements


     };


  }
}

#endif
