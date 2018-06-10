/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_create_stmt.cpp
 * @brief the ObProcedureCreateStmt class definition that warp procedure create statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_create_stmt.h"
using namespace oceanbase::common;
namespace oceanbase{
namespace sql{
void ObProcedureCreateStmt::print(FILE* fp, int32_t level, int32_t index) {
		UNUSED(index);
		print_indentation(fp, level);
		fprintf(fp, "<ObProcedureCreateStmt %d begin>\n", index);
		//print_indentation(fp, level + 1);
		//fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
		print_indentation(fp, level);
		fprintf(fp, "<ObProcedureCreateStmt %d End>\n", index);
}
//add by wangdonghui 20160121 :b
int ObProcedureCreateStmt::set_proc_source_code(ObString &proc_source_code)
{
    proc_source_code_ = proc_source_code;
    return OB_SUCCESS;
}

ObString& ObProcedureCreateStmt::get_proc_source_code()
{
    return proc_source_code_;
}
//add :e
int ObProcedureCreateStmt::set_proc_name(ObString &proc_name)
{
	proc_name_=proc_name;
	return OB_SUCCESS;
}

ObString& ObProcedureCreateStmt::get_proc_name()
{
	return proc_name_;
}
int ObProcedureCreateStmt::set_proc_id(uint64_t& stmt_id)
{
	proc_id_=stmt_id;
	return OB_SUCCESS;
}

uint64_t& ObProcedureCreateStmt::get_proc_id()
{
	return proc_id_;
}


}
}



