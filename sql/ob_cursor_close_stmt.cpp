/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_close_stmt.cpp
* @brief this class  present a cursor close logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#include "ob_cursor_close_stmt.h"

using namespace oceanbase::common;
using namespace oceanbase::sql;
void ObCursorCloseStmt::print(FILE* fp, int32_t level, int32_t index)
{
  UNUSED(index);
  print_indentation(fp, level);
  fprintf(fp, "<ObCursorCloseStmt %d Begin>\n", index);
  print_indentation(fp, level + 1);
  fprintf(fp, "\n");
  print_indentation(fp, level + 1);
  fprintf(fp, "<ObCursorCloseStmt %d End>\n", index);

}


