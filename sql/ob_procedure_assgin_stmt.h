/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_assgin_stmt.h
 * @brief the ObProcedureAssginStmt calss define that wrap procedure assgin statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_ASSGIN_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_ASSGIN_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include "ob_sql_expression.h"
using namespace oceanbase::common;

namespace oceanbase {
  namespace sql {
  /**
     * @brief The ObRawVarAssignVal struct
     * expression raw variable value
     */
    struct ObRawVarAssignVal
    {
      ObRawVarAssignVal() : val_expr_id_(OB_INVALID_ID)
      {
        idx_value_.set_null();
      }

      ObString var_name_;  ///< variable name
      ObObj idx_value_;  ///<  null for normal variable, int or varchar for array variable
      uint64_t val_expr_id_;  ///<  expression value id
//      ObRawVarAssignVal() : val_expr_id_(0)
//      {}
    };

//    struct ObRawArrAssignVal
//    {
//      ObString var_name_;
//      uint64_t val_expr_id_;
////      uint64_t idx_expr_id_;
//      ObObj idx_value_;

//      ObRawArrAssignVal() : val_expr_id_(0), idx_expr_id_(0)
//      {}
//    };
    /**
     * @brief The ObProcedureAssginStmt class
     * procedure assgin statement
     */
    class ObProcedureAssginStmt: public ObBasicStmt {
    public:
      /**
       * @brief constructor
       */
      ObProcedureAssginStmt() :
              ObBasicStmt(T_PROCEDURE_ASSGIN) {
      }
      /**
       * @brief destructor
       */
      virtual ~ObProcedureAssginStmt() {
      }

      /**
       * @brief add_var_val
       * add ObRawVarAssignVal object
       * @param var_val ObRawVarAssignVal object
       * @return error code
       */
      int add_var_val(ObRawVarAssignVal &var_val);/*添加一个var_val*/
//      int add_arr_val(ObRawArrAssignVal &arr_val); //add the array[idx] assign

//      const ObArray<ObRawVarAssignVal>& get_var_val_list() const;/*返回所有赋值*/
      /**
       * @brief get_var_val
       * get ObRawVarAssignVal object by id
       * @param index ObRawVarAssignVal object id
       * @return ObRawVarAssignVal object
       */
      const ObRawVarAssignVal& get_var_val(int64_t index) const;/*返回一个赋值*/
//      const ObRawArrAssignVal& get_arr_val(int64_t index) const; //get the array[idx] assign
      /**
       * @brief get_var_val_size
       * get variable size
       * @return variable number
       */
      int64_t get_var_val_size() const;/*返回变量列表大小*/
//      int64_t get_arr_val_size() const;
      /**
       * @brief print
       * print ObProcedureAssginStmt info
       * @param fp output file
       * @param level
       * @param index
       */
      virtual void print(FILE* fp, int32_t level, int32_t index);
    private:
//      ObArray<ObVarAssignVal> var_val_list_;/*赋值变量列表*/
      ObArray<ObRawVarAssignVal> var_val_list_;  ///< assignment variable list
//      ObArray<ObRawArrAssignVal> arr_val_list_;
    };
  }
}

#endif
