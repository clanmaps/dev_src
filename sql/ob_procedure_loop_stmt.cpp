/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_loop_stmt.h
 * @brief the ObProcedureLoopStmt class definition that warp procedure loop statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_loop_stmt.h"

namespace oceanbase
{
  namespace sql
  {
    ObProcedureLoopStmt::~ObProcedureLoopStmt()
    {
    }

    void ObProcedureLoopStmt::print(FILE* fp, int32_t level, int32_t index)
    {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureLoopStmt %d begin>\n", index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureLoopStmt %d End>\n", index);
    }

    uint64_t ObProcedureLoopStmt::get_loop_stmt(int64_t idx) const
    {
      uint64_t stmt_id = OB_INVALID;
      if( OB_SUCCESS != loop_body_stmts_.at(idx, stmt_id) )
      {
        TBSYS_LOG(WARN, "stmt idx out of range in loop body block");
      }
      return stmt_id;
    }
  }
}
