/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_if_stmt.h
 * @brief the ObProcedureIfStmt class definition that warp procedure if statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_IF_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_IF_STMT_H_
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
     * @brief The ObProcedureIfStmt class
     * procedure if statement class definition
     */
    class ObProcedureIfStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureIfStmt() :
            ObBasicStmt(T_PROCEDURE_IF)
        {
          expr_id_ = common::OB_INVALID_ID;
          else_stmt_ = common::OB_INVALID_ID;
          have_else_if_ = false;
          have_else_ = false;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureIfStmt()
        {
        }
        /**
         * @brief print
         * print procedure if statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);

        /**
         * @brief set_expr_id
         * set if condition expression id
         * @param expr_id if condition expression id
         * @return error code
         */
        int set_expr_id(uint64_t& expr_id);
        /**
         * @brief add_then_stmt
         * add a then statement
         * @param stmt_id then statement id
         * @return error code
         */
        int add_then_stmt(uint64_t& stmt_id);
        /**
         * @brief add_else_if_stmt
         * add a else if statement
         * @param stmt_id else if statement id
         * @return error code
         */
        int add_else_if_stmt(uint64_t& stmt_id);
        /**
         * @brief set_else_stmt
         * set else statement id
         * @param stmt_id else statement id
         * @return error code
         */
        int set_else_stmt(uint64_t& stmt_id);
        /**
         * @brief set_have_elseif
         * set have elseif flag
         * @param flag have elseif flag
         * @return error code
         */
        int set_have_elseif(bool flag);
        /**
         * @brief set_have_else
         * set have else flag
         * @param flag have else flag
         * @return error code
         */
        int set_have_else(bool flag);
        /**
         * @brief have_elseif
         * get have elseif flag
         * @return elseif flag
         */
        bool have_elseif() const;
        /**
         * @brief have_else
         * get have else flag
         * @return  have else flag
         */
        bool have_else() const;

        /*if表达的id*/
        /**
         * @brief get_expr_id
         * get if condition expression id
         * @return if condition expression id
         */
        uint64_t get_expr_id() const;
        /**
         * @brief get_then_stmts
         * get then statement id array
         * @return then statement id array
         */
        const ObArray<uint64_t> &get_then_stmts() const;		/*then语句列表*/
        /**
         * @brief get_then_stmt
         * get then statement id by array index
         * @param index array index
         * @return  then statement id
         */
        uint64_t get_then_stmt(int64_t index) const;
        /**
         * @brief get_elseif_stmts
         * get elseif statement id array
         * @return elseif statement id array
         */
        const ObArray<uint64_t> &get_elseif_stmts() const;	/*else if语句列表*/
        /**
         * @brief get_elseif_stmt
         * get elseif statement id by array index
         * @param index array index
         * @return elseif statement id
         */
        uint64_t get_elseif_stmt(int64_t index) const;
        /**
         * @brief get_else_stmt
         * get else statement id
         * @return  else statement id
         */
        uint64_t 	get_else_stmt() const;			/*else语句*/
        /**
         * @brief get_then_stmt_size
         * get then statement size
         * @return then statement size
         */
        int64_t	get_then_stmt_size() const;		/*返回 if then 下面的语句长度*/
        /**
         * @brief get_elseif_stmt_size
         *get elseif statement size
         * @return elseif statement size
         */
        int64_t	get_elseif_stmt_size() const;		/*返回elseif的个数*/



      private:
        uint64_t expr_id_;	  ///<  if condition expression id

        ObArray<uint64_t> then_stmts_;  ///<  if then statement id array

        bool have_else_if_;  ///<  have elseif statement flag

        ObArray<uint64_t> elseif_stmts_;  ///<  elseif statement id array

        bool have_else_;  ///<  have else statement flag

        uint64_t else_stmt_;	 ///<  else statement id

    };

  }
}

#endif
