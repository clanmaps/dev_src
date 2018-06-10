/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_assgin_stmt.cpp
 * @brief the ObProcedureAssginStmt calss define that wrap procedure assgin statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_assgin_stmt.h"
using namespace oceanbase::common;
namespace oceanbase{
  namespace sql{

    void ObProcedureAssginStmt::print(FILE* fp, int32_t level, int32_t index) {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureAssginStmt %d begin>\n", index);
      print_indentation(fp, level + 1);
      fprintf(fp, "Expires Count = %d\n",(int32_t)var_val_list_.count());
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureAssginStmt %d End>\n", index);
    }

    int ObProcedureAssginStmt::add_var_val(ObRawVarAssignVal &var_val)
    {
      return var_val_list_.push_back(var_val);
    }

//    int ObProcedureAssginStmt::add_arr_val(ObRawArrAssignVal &arr_val)
//    {
//      return arr_val_list_.push_back(arr_val);
//    }

//    const ObArray<ObRawVarAssignVal>& ObProcedureAssginStmt::get_var_val_list() const
//    {
//      return var_val_list_;
//    }

    const ObRawVarAssignVal& ObProcedureAssginStmt::get_var_val(int64_t index) const
    {
      return var_val_list_.at(index);
    }

//    const ObRawArrAssignVal& ObProcedureAssginStmt::get_arr_val(int64_t index) const
//    {
//      return arr_val_list_.at(index);
//    }

    int64_t ObProcedureAssginStmt::get_var_val_size() const
    {
      return var_val_list_.count();
    }

//    int64_t ObProcedureAssginStmt::get_arr_val_size() const
//    {
//      return arr_val_list_.count();
//    }
  }
}
