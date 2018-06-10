/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_loop_stmt.h
 * @brief the ObProcedureLoopStmt class definition that warp procedure loop statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */


#ifndef OBPROCEDURELOOPSTMT_H
#define OBPROCEDURELOOPSTMT_H

#include "common/ob_string.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"

using namespace oceanbase::common;
namespace oceanbase
{
  namespace sql
  {
    //create by zt 20151128
    /**
     * @brief The ObProcedureLoopStmt class
     * procedure loop statement class definition
     */
    class ObProcedureLoopStmt : public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureLoopStmt() :
                ObBasicStmt(T_PROCEDURE_LOOP),
                lowest_expr_id_(OB_INVALID_ID),
                highest_expr_id_(OB_INVALID_ID),
                step_size_(1),
                reverse_(false)
        {
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureLoopStmt();
        /**
         * @brief print
         * print procedure loop statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief add_loop_stmt
         * add a loop statement id
         * @param stmt_id loop statement id
         * @return error code
         */
        int add_loop_stmt(uint64_t stmt_id) { return loop_body_stmts_.push_back(stmt_id); }
        /**
         * @brief set_lowest_expr
         * set lowest expression id
         * @param lowest_expr_id lowest expression id
         */
        void set_lowest_expr(uint64_t lowest_expr_id) { lowest_expr_id_ = lowest_expr_id; }
        /**
         * @brief set_highest_expr
         * set highest expression id
         * @param highest_expr_id highest expression id
         */
        void set_highest_expr(uint64_t highest_expr_id) { highest_expr_id_ = highest_expr_id; }
        /**
         * @brief set_reverse
         * set reverse flag
         * @param rev bool value
         */
        void set_reverse(bool rev) { reverse_ = rev; }
        /**
         * @brief set_loop_count_name
         * set loop counter name
         * @param count_name loop counter name
         */
        void set_loop_count_name(const ObString &count_name) { loop_count_ = count_name; }
        /**
         * @brief get_lowest_expr_id
         * get lowest expression id
         * @return lowest expression id
         */
        uint64_t get_lowest_expr_id() const { return lowest_expr_id_; }
        /**
         * @brief get_highest_expr_id
         * get highest expression id
         * @return highest expression id
         */
        uint64_t get_highest_expr_id() const { return highest_expr_id_ ; }
        /**
         * @brief get_loop_stmt
         * get loop inner statement id by array id
         * @param idx array id
         * @return loop inner statement id
         */
        uint64_t get_loop_stmt(int64_t idx) const;
        /**
         * @brief get_loop_body_size
         * get loop body statement array size
         * @return loop body statement array size
         */
        int64_t get_loop_body_size() const { return loop_body_stmts_.count(); }
        /**
         * @brief get_loop_counter_name
         * get loop counter name
         * @return loop counter name
         */
        const ObString & get_loop_counter_name() const { return loop_count_; }
        /**
         * @brief is_reverse
         * get reverse flag
         * @return bool value
         */
        bool is_reverse() const { return reverse_; }
        /**
         * @brief get_step_size
         * get step size
         * @return step length
         */
        int64_t get_step_size() const { return step_size_; }

      private:

        ObString loop_count_; 	///<	loop variable name

        uint64_t lowest_expr_id_;  ///<  the start loop value
        uint64_t highest_expr_id_;  ///<  the end loop value

        int64_t step_size_;     ///<  now the step_size is default set to 1

        bool reverse_; 			///<  is loop in the reverse order, for high to low

        ObArray<uint64_t> loop_body_stmts_;	 ///< loop body statement id array
    };
  }
}

#endif // OBPROCEDURELOOPSTMT_H
