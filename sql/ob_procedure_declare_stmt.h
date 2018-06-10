/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_declare_stmt.h
 * @brief the ObProcedureDeclareStmt class definition that warp procedure declare statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_DECLARE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_DECLARE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include "ob_sql_expression.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObVariableDef struct
     * procedure variable define
     */
    struct ObVariableDef
    {
      ObString    variable_name_;  ///<  parameter name
      ObObjType   variable_type_;  ///<  parameter type
      bool   is_default_;  ///<  is there a default value
      bool 	 is_array_;  ///<  does represents array
      ObObj default_value_;  ///<  default value
    };
    /**
     * @brief The ObProcedureDeclareStmt class
     * procedure declare statement class definiton
     */
    class ObProcedureDeclareStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureDeclareStmt() :
                ObBasicStmt(T_PROCEDURE_DECLARE) {
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureDeclareStmt() {
        }

        /**
         * @brief add_proc_var
         * add a procedure variable
         * @param proc_var procedure variable
         * @return error code
         */
        int add_proc_var(const ObVariableDef &proc_var);

        /**
         * @brief get_variable
         * get procedure variable by array index
         * @param index array index
         * @return  procedure variable object
         */
        const ObVariableDef& get_variable(int64_t index) const;
        /**
         * @brief get_variable_size
         * get variable size
         * @return variable number
         */
        int64_t get_variable_size() const;
        /**
         * @brief print
         * print procedure declare  statement info
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);
      private:
        ObArray<ObVariableDef> variables_;  ///<  procedure variable array
    };
  }
}

#endif
