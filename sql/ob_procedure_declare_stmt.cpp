/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_declare_stmt.cpp
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

#include "ob_procedure_declare_stmt.h"
using namespace oceanbase::common;
namespace oceanbase
{
  namespace sql
  {
    void ObProcedureDeclareStmt::print(FILE* fp, int32_t level, int32_t index) {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureDeclareStmt %d begin>\n", index);
      //print_indentation(fp, level + 1);
      //fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureDeclareStmt %d End>\n", index);
    }

    int ObProcedureDeclareStmt::add_proc_var(const ObVariableDef &proc_var)
    {
      return variables_.push_back(proc_var);
    }

    const ObVariableDef& ObProcedureDeclareStmt::get_variable(int64_t index) const
    {
      return variables_.at(index);
    }

    int64_t ObProcedureDeclareStmt::get_variable_size() const
    {
      return variables_.count();
    }
  }
}



