/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_execute_stmt.cpp
 * @brief the ObProcedureExecuteStmt class definition that warp procedure execute statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#include "ob_procedure_execute_stmt.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {

    void ObProcedureExecuteStmt::print(FILE* fp, int32_t level, int32_t index)
    {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureExecuteStmt %d begin>\n", index);
      //print_indentation(fp, level + 1);
      //fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureExecuteStmt %d End>\n", index);
    }

    int ObProcedureExecuteStmt::set_proc_name(const ObString &proc_name)
    {
      proc_name_=proc_name;
      return OB_SUCCESS;
    }
    int ObProcedureExecuteStmt::set_proc_stmt_id(uint64_t proc_stmt_id)
    {
      proc_stmt_id_=proc_stmt_id;
      return OB_SUCCESS;
    }

    const ObString& ObProcedureExecuteStmt::get_proc_name() const
    {
      return proc_name_;
    }
    uint64_t ObProcedureExecuteStmt::get_proc_stmt_id() const
    {
      return proc_stmt_id_;
    }

    int ObProcedureExecuteStmt::add_param_expr(uint64_t expr_id)
    {
      return param_list_.push_back(expr_id);
    }

    uint64_t ObProcedureExecuteStmt::get_param_expr(int64_t index) const
    {
      return param_list_.at(index);
    }

    int64_t ObProcedureExecuteStmt::get_param_size() const
    {
      return param_list_.count();
    }
    //add by wdh 20160718 :b
    int ObProcedureExecuteStmt::set_no_group(bool no_group)
    {
      no_group_ = no_group;
      return OB_SUCCESS;
    }
    //add :e
    bool ObProcedureExecuteStmt::get_no_group()
    {
        return no_group_;
    }
    //add by qx 20170317 :b
    int ObProcedureExecuteStmt::set_long_trans(bool long_trans)
    {
      long_trans_ = long_trans;
      return OB_SUCCESS;
    }
    bool ObProcedureExecuteStmt::get_long_trans()
    {
     return long_trans_;
    }
    //add :e


  }
}
