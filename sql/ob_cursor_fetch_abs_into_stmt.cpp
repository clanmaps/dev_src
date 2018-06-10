/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_abs_into_stmt.cpp
* @brief this class  present a "fetch cursor abs into" logic plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_cursor_fetch_abs_into_stmt.h"
using namespace oceanbase::common;
namespace oceanbase{
namespace sql{
void ObCursorFetchAbsIntoStmt::print(FILE* fp, int32_t level, int32_t index) {
		UNUSED(index);
		print_indentation(fp, level);
		fprintf(fp, "<ObCursorFetchAbsIntoStmt %d begin>\n", index);
		//print_indentation(fp, level + 1);
		//fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
		print_indentation(fp, level);
		fprintf(fp, "<ObCursorFetchAbsIntoStmt %d End>\n", index);
}

int ObCursorFetchAbsIntoStmt::set_cursor_id(uint64_t query_id)
{
	cursor_stmt_id_=query_id;
	return OB_SUCCESS;
}
uint64_t ObCursorFetchAbsIntoStmt::get_cursor_id()
{
	return cursor_stmt_id_;
}
int ObCursorFetchAbsIntoStmt::add_variable(ObString &name)
{
	return variable_name_.push_back(name);
}
ObString& ObCursorFetchAbsIntoStmt::get_variable(int64_t index)
{
	return variable_name_.at(index);
}
int64_t ObCursorFetchAbsIntoStmt::get_variable_size()
{
	return variable_name_.count();
}


}
}



