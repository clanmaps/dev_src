/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_exit_stmt.h
 * @brief the ObProcedureExitStmt class definition that warp procedure exit statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

//add by wangdonghui 20160623
#ifndef OCEANBASE_SQL_OB_PROCEDURE_EXIT_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_EXIT_STMT_H_
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
     * @brief The ObProcedureExitStmt class
     * procedure exit statement class definition
     */
    class ObProcedureExitStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureExitStmt() :
                  ObBasicStmt(T_PROCEDURE_EXIT)
        {
          expr_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureExitStmt()
        {
        }
        /**
         * @brief print
         * print procedure exit statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief set_expr_id
         * set exit loop expression id
         * @param expr_id exit loop expression id
         * @return error code
         */
        int set_expr_id(uint64_t& expr_id);
        /**
         * @brief get_expr_id
         * get exit loop expression id
         * @return exit loop expression id
         */
        uint64_t get_expr_id();

      private:
        uint64_t expr_id_;		///<  exit loop expression id  -1 represent no when

    };
  }
}

#endif
