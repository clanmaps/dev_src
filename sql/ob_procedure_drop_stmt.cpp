/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_drop_stmt.cpp
 * @brief the ObProcedureDropStmt class definition that warp procedure drop statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_drop_stmt.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    void ObProcedureDropStmt::print(FILE* fp, int32_t level, int32_t index)
    {
        UNUSED(index);
        print_indentation(fp, level);
        fprintf(fp, "<ObProcedureDropStmt %d begin>\n", index);
        print_indentation(fp, level);
        fprintf(fp, "<ObProcedureDropStmt %d End>\n", index);
    }

    int ObProcedureDropStmt::set_proc_name(ObString &proc_name)
    {
      proc_name_=proc_name;
      return OB_SUCCESS;
    }

    ObString& ObProcedureDropStmt::get_proc_name()
    {
      return proc_name_;
    }

    int ObProcedureDropStmt::set_proc_delete_id(uint64_t& stmt_id)
    {
      proc_delete_id_=stmt_id;
      return OB_SUCCESS;
    }
    void ObProcedureDropStmt::set_if_exists(bool flag)
    {
      if_exists_=flag;
    }

    bool ObProcedureDropStmt::if_exists()
    {
      return if_exists_;
    }

    uint64_t& ObProcedureDropStmt::get_proc_delete_id()
    {
      return proc_delete_id_;
    }

  }
}



