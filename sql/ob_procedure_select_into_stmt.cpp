/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_select_into_stmt.cpp
 * @brief the ObProcedureSelectIntoStmt class definition that warp procedure select into statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_select_into_stmt.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {

    void ObProcedureSelectIntoStmt::print(FILE* fp, int32_t level, int32_t index)
    {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureSelectIntoStmt %d begin>\n", index);
      //print_indentation(fp, level + 1);
      //fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureSelectIntoStmt %d End>\n", index);
    }

    int ObProcedureSelectIntoStmt::set_declare_id(uint64_t query_id)
    {
      declare_query_id_=query_id;
      return OB_SUCCESS;
    }

    uint64_t ObProcedureSelectIntoStmt::get_declare_id()
    {
      return declare_query_id_;
    }

    int ObProcedureSelectIntoStmt::add_variable(const SpRawVar& var)
    {
      return raw_vars_.push_back(var);
    }

    const SpRawVar& ObProcedureSelectIntoStmt::get_variable(int64_t index)
    {
      return raw_vars_.at(index);
    }

    int64_t ObProcedureSelectIntoStmt::get_variable_size()
    {
      return raw_vars_.count();
    }
  }
}



