/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_while_stmt.cpp
 * @brief the ObProcedureWhileStmt class definition that warp procedure while statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_29
 */

#include "ob_procedure_while_stmt.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    void ObProcedureWhileStmt::print(FILE* fp, int32_t level, int32_t index)
    {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureWhileStmt %d begin>\n", index);
      //print_indentation(fp, level + 1);
      //fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureWhileStmt %d End>\n", index);
    }
    int ObProcedureWhileStmt::set_expr_id(uint64_t& expr_id)
    {
      expr_id_=expr_id;
      return OB_SUCCESS;
    }
    int ObProcedureWhileStmt::add_do_stmt(uint64_t& stmt_id)
    {
      while_do_stmts_.push_back(stmt_id);
      return OB_SUCCESS;
    }
    /*while expr id*/
    uint64_t ObProcedureWhileStmt::get_expr_id()
    {
      return expr_id_;
    }
    uint64_t& ObProcedureWhileStmt::get_do_stmt(int64_t index)
    {
      return while_do_stmts_.at(index);
    }
    int64_t ObProcedureWhileStmt::get_do_stmt_size()
    {
      return while_do_stmts_.count();
    }

  }
}



