/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_variable_set_array_value_stmt.cpp
 * @brief the ObVariableSetArrayValueStmt class definition that warp procedure variable set array value statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

#include "ob_variable_set_array_value_stmt.h"

namespace oceanbase
{
  namespace sql
  {
    ObVariableSetArrayValueStmt::~ObVariableSetArrayValueStmt()
    {
    }

    void ObVariableSetArrayValueStmt::print(FILE *fp, int32_t level, int32_t index)
    {
      print_indentation(fp, level);

      fprintf(fp, "<ObArrayValueList %d begin>\n", index);

      print_indentation(fp, level + 1);
      fprintf(fp, "value list: {");
      for(int64_t i = 0; i < values_.count(); ++i)
      {
        fprintf(fp, "%s ", to_cstring(values_.at(i)));
      }
      fprintf(fp, "}\n");
      print_indentation(fp, level);
      fprintf(fp, "<ObArrayValueList %d End>\n", index);
    }
  }
}

