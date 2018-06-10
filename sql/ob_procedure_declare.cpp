/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_declare.h
* @brief this class  present a procedure "declare" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_declare.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureDeclare::ObProcedureDeclare()
{
}

ObProcedureDeclare::~ObProcedureDeclare()
{
}

int ObProcedureDeclare::add_proc_var(ObVariableDef &proc_var)
{
	variables_.push_back(proc_var);
	return OB_SUCCESS;
}

void ObProcedureDeclare::reset()
{

}
void ObProcedureDeclare::reuse()
{

}
int ObProcedureDeclare::close()
{
	return OB_SUCCESS;
}

int ObProcedureDeclare::get_row_desc(const common::ObRowDesc *&row_desc) const
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(NULL == child_op_))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(ERROR, "child_op_ is NULL");
	}
	else
	{
		ret = child_op_->get_row_desc(row_desc);
	}
	return ret;
}
int ObProcedureDeclare::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (NULL == child_op_)
	{
		ret = OB_ERR_UNEXPECTED;
		TBSYS_LOG(ERROR, "child_op_ is NULL");
	}
	else
	{
	  ret = child_op_->get_next_row(row);
	}
	return ret;
}

int ObProcedureDeclare::open()
{
    TBSYS_LOG(INFO, "ObProcedureDeclare open()");
	int ret = OB_SUCCESS;
	if (variables_.count()<=0)
	{
		ret = OB_ERR_GEN_PLAN;
		TBSYS_LOG(ERROR, "ObProcedureDeclare var is NULL");
	}
	else
	{
		ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
		for (int64_t i = 0; i < variables_.count(); i++)
		{
			 ObVariableDef var=variables_.at(i);

			 if(var.is_default_)
			 {
				 if((ret=session->replace_variable(*var.variable_name_,*var.default_value_))!=OB_SUCCESS)
				 {
					 TBSYS_LOG(WARN, "replace_variable default_value_  ERROR");
				 }
				 else
				 {
					 TBSYS_LOG(INFO, "declare %.*s and set  default_value %s",var.variable_name_->length(),var.variable_name_->ptr(),to_cstring(*var.default_value_));
				 }
			 }
			 else
			 {
				 ObObj new_value_obj;
				 new_value_obj.set_null();
				 new_value_obj.set_type(var.variable_type_);
				 if((ret=session->replace_variable(*var.variable_name_,new_value_obj))!=OB_SUCCESS)
				 {
					 TBSYS_LOG(WARN, "replace_variable default_value_  ERROR");
				 }
				 else
				 {
					 TBSYS_LOG(INFO, "declare %.*s and set  default_value null",var.variable_name_->length(),var.variable_name_->ptr());
				 }
			 }

		}
	}
	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureDeclare, PHY_PROCEDURE_DECLARE);
  }
}

int64_t ObProcedureDeclare::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObProcedureDeclare (var list size=%ld)\n", variables_.count());
  return pos;
}
