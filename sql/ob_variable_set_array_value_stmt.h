/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_variable_set_array_value_stmt.h
 * @brief the ObVariableSetArrayValueStmt class definition that warp procedure variable set array value statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

#ifndef OB_VARIABLE_SET_ARRAY_VALUE_STMT_H
#define OB_VARIABLE_SET_ARRAY_VALUE_STMT_H

#include "common/ob_array.h"
#include "sql/ob_basic_stmt.h"
#include "common/ob_object.h"

using namespace oceanbase::common;
namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObVariableSetArrayValueStmt class
     * procedure variable set array value statement class definition
     */
    class ObVariableSetArrayValueStmt : public ObBasicStmt
    {
      public:
        /**
         * @brief ObVariableSetArrayValueStmt constructor
         */
        ObVariableSetArrayValueStmt() :
          ObBasicStmt(T_VARIABLE_SET_ARRAY_VALUE) {

        }
        /**
         * @brief destructor
         */
        virtual ~ObVariableSetArrayValueStmt();
        /**
         * @brief set_var_name
         * set variable name
         * @param var_name variable name
         */
        void set_var_name(const ObString &var_name) { var_name_ = var_name; }
        /**
         * @brief add_value
         * add a array variable element value
         * @param val array variable element value
         * @return error code
         */
        int add_value(const ObObj &val) { return values_.push_back(val); }
        /**
         * @brief get_var_name
         * get array variable name
         * @return array variable name
         */
        const ObString &get_var_name() const { return var_name_; }
        /**
         * @brief get_value
         * get array element value by array index
         * @param idx array index
         * @return array element value
         */
        const ObObj & get_value(int64_t idx) const { return values_.at(idx); }
        /**
         * @brief count
         * get array element count
         * @return array element count
         */
        int64_t count() const { return values_.count();}
        /**
         * @brief print
         * print array variable information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE *fp, int32_t level, int32_t index);
      private:
        ObArray<ObObj> values_;  ///<  array variable value stack
        ObString var_name_;  ///<  array variable name
    };
  }
}

#endif // OBARRAYVALUELISTSTMT_H
