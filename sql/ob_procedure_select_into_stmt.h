/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_select_into_stmt.h
 * @brief the ObProcedureSelectIntoStmt class definition that warp procedure select into statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_SELECT_INTO_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_SELECT_INTO_STMT_H_
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
     * @brief The SpRawVar struct
     * procedure raw variable
     */
    struct SpRawVar
    {
      ObString var_name_;  ///< variable name
      ObObj idx_value_;   ///<  variable value index
      //      uint64_t idx_expr_id_;
      /**
       * @brief SpRawVar constructor
       */
      SpRawVar()  { idx_value_.set_null(); }
    };
    /**
     * @brief The ObProcedureSelectIntoStmt class
     * procedure select into statement class definition
     */
    class ObProcedureSelectIntoStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureSelectIntoStmt() :
                ObBasicStmt(T_PROCEDURE_SELECT_INTO)
        {
          declare_query_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureSelectIntoStmt()
        {
        }
        /**
         * @brief print
         * print procedure select into statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
        /**
         * @brief set_declare_id
         * set declare statement id
         * @param query_id declare statement id
         * @return error code
         */
        int set_declare_id(uint64_t query_id);
        /**
         * @brief get_declare_id
         * get declare statement id
         * @return
         */
        uint64_t get_declare_id();
        /**
         * @brief add_variable
         * add a variable
         * @param raw_var raw variable
         * @return error code
         */
        int add_variable(const SpRawVar &raw_var);
        /**
         * @brief get_variable
         * get variable by array index
         * @param index
         * @return
         */
        const SpRawVar & get_variable(int64_t index);
        /**
         * @brief get_variable_size
         * get variable array size
         * @return variable array size
         */
        int64_t get_variable_size();

      private:
        uint64_t declare_query_id_;  ///<  declare statement query id
        ObArray<SpRawVar> raw_vars_;  ///<  raw variable array

    };
  }
}

#endif
