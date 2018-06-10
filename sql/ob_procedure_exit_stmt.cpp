/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_exit_stmt.cpp
 * @brief the ObProcedureExitStmt class definition that warp procedure exit statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_exit_stmt.h"
using namespace oceanbase::common;
namespace oceanbase
{
  namespace sql
  {
    void ObProcedureExitStmt::print(FILE* fp, int32_t level, int32_t index)
    {
        UNUSED(index);
        print_indentation(fp, level);
        fprintf(fp, "<ObProcedureExitStmt %d begin>\n", index);
        print_indentation(fp, level);
        fprintf(fp, "<ObProcedureExitStmt %d End>\n", index);
    }

    int ObProcedureExitStmt::set_expr_id(uint64_t& expr_id)
    {
        expr_id_=expr_id;
        return OB_SUCCESS;
    }

    /*when expr id*/
    uint64_t ObProcedureExitStmt::get_expr_id()
    {
        return expr_id_;
    }
  }
}



