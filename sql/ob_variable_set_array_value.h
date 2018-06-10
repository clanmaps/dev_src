/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_variable_set_array_value.h
 * @brief the ObVariableSetArrayValue class definition
 *
 * create by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

#ifndef OBVARIABLESETARRAYVALUE_H
#define OBVARIABLESETARRAYVALUE_H

#include "ob_no_children_phy_operator.h"
#include "common/ob_string.h"
#include "common/ob_object.h"
#include "common/ob_array.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObVariableSetArrayValue class
     * array variable physical plan definition for procedure
     */
    class ObVariableSetArrayValue : public ObNoChildrenPhyOperator
    {
      public:
        /**
         * @brief ObVariableSetArrayValue constructor
         */
        ObVariableSetArrayValue();
        /**
         * @brief destructor
         */
        virtual ~ObVariableSetArrayValue();
        /**
         * @brief reset
         * clear object
         */
        virtual void reset() {}
        /**
         * @brief reuse
         * clear object values_
         */
        virtual void reuse() { values_.clear(); }
        /**
         * @brief open
         * open physical plan operator
         * @return error code
         */
        virtual int open();
        /**
         * @brief close
         * close physical plan operator
         * @return error code
         */
        virtual int close() { return OB_SUCCESS; }
        /**
         * @brief get_type
         * get physical plan operator type
         * @return operator type
         */
        virtual ObPhyOperatorType get_type() const { return PHY_VARIABLE_SET_ARRAY; }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return  byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief get_next_row
         * get next row
         * @param row returned ObRow object point
         * @return next row state
         */
        virtual int get_next_row(const ObRow *&row) { row = NULL; return OB_ITER_END; }
        /**
         * @brief get_row_desc
         * get row descriptor
         * @param row_desc row descriptor
         * @return error code
         */
        virtual int get_row_desc(const ObRowDesc *&row_desc) const { row_desc = NULL; return OB_NOT_SUPPORTED; }
        /**
         * @brief set_var_name
         * set variable name
         * @param var_name  variable name
         */
        void set_var_name(const ObString &var_name) { var_name_ = var_name; }
        /**
         * @brief add_array_value
         * push a array element value to array
         * @param obj array element
         * @return error code
         */
        int add_array_value(const ObObj &obj) { return values_.push_back(obj); }

      private:
        /**
         * @brief ObVariableSetArrayValue copy constructor disable
         * @param other ObVariableSetArrayValue object
         */
        ObVariableSetArrayValue(const ObVariableSetArrayValue &other);
        /**
         * @brief operator =
         * = opeartor overload disable
         * @param other ObVariableSetArrayValue object
         * @return ObVariableSetArrayValue object
         */
        ObVariableSetArrayValue& operator = (const ObVariableSetArrayValue &other);

        ObString var_name_;  ///<  array variable name
        ObArray<ObObj> values_;  ///<  vlaue stack
    };
  }
}

#endif // OBVARIABLESETARRAYVALUE_H
