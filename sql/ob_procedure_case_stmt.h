/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_case_stmt.h
 * @brief the ObProcedureCaseStmt class definition that warp procedure case statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_CASE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_CASE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
using namespace oceanbase::common;

namespace oceanbase {
  namespace sql {
    //TODO
    /**
     * @brief The ObProcedureCaseStmt class
     * procedure case statement warper
     */
    class ObProcedureCaseStmt: public ObBasicStmt {
      public:
        /**
         * @brief constructor
         */
        ObProcedureCaseStmt() :
              ObBasicStmt(T_PROCEDURE_CASE) {
          expr_id_ = common::OB_INVALID_ID;
          else_stmt_ = common::OB_INVALID_ID;
          have_else_ = false;
          }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureCaseStmt() {
        }
        /**
         * @brief print
         * print ObProcedureCaseStmt info
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief set_expr_id
         * set expression id
         * @param expr_id
         * @return
         */
        int set_expr_id(uint64_t& expr_id);
        /**
         * @brief add_case_when_stmt
         * add case when statement object id
         * @param stmt_id
         * @return
         */
        int add_case_when_stmt(uint64_t& stmt_id);
        /**
         * @brief set_else_stmt
         * set else statement object id
         * @param stmt_id
         * @return
         */
        int set_else_stmt(uint64_t& stmt_id);
        /**
         * @brief set_have_else
         * set flag whether have else statement
         * @param flag bool value
         * @return error code
         */
        int set_have_else(bool flag);
        /**
         * @brief have_else
         * get have else flag
         * @return bool value
         */
        bool have_else();

        /*case 表达的expr id*/
        /**
         * @brief get_expr_id
         * get case expression id
         * @return expression id
         */
        uint64_t get_expr_id();
        /**
         * @brief get_case_when_stmts
         * get case when statements
         * @return stmt id array
         */
        const ObArray<uint64_t>& get_case_when_stmts() const;	/*case when语句列表*/
        /**
         * @brief get_case_when_stmt
         * get case when statement by array index
         * @param index case when statement array index
         * @return case when statement id
         */
        uint64_t& get_case_when_stmt(int64_t index);
        /**
         * @brief get_else_stmt
         * get else statement id
         * @return else statement id
         */
        uint64_t get_else_stmt();				/*else语句*/
        /**
         * @brief get_case_when_stmt_size
         * get case when statements size
         * @return statements size
         */
        int64_t	get_case_when_stmt_size();		/*返回case when的个数*/



        private:
          uint64_t expr_id_;	///<  case expression id

          ObArray<uint64_t> casewhen_stmts_;  ///<  case when statement id array

          bool have_else_;	///<  else statement flag

          uint64_t else_stmt_;	///<  else statement id

      };


  }
}

#endif
