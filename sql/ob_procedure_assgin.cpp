/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_assgin.cpp
* @brief this class  present a procedure "assgin" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_assgin.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureAssgin::ObProcedureAssgin()
{
}

ObProcedureAssgin::~ObProcedureAssgin()
{
}

int ObProcedureAssgin::add_var_val(ObVariableSetVal &var_val)
{
	var_val_list_.push_back(var_val);
	return OB_SUCCESS;
}

void ObProcedureAssgin::reset()
{

}
void ObProcedureAssgin::reuse()
{

}
int ObProcedureAssgin::close()
{
	return OB_SUCCESS;
}

int ObProcedureAssgin::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureAssgin::get_next_row(const common::ObRow *&row)
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

int ObProcedureAssgin::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureAssgin open()");
	int ret = OB_SUCCESS;
	if (var_val_list_.count()<=0)
	{
		ret = OB_ERR_GEN_PLAN;
		TBSYS_LOG(ERROR, "ObProcedureAssgin var is NULL");
	}
	else
	{
		ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
		for (int64_t i = 0; i < var_val_list_.count(); i++)
		{
			 ObVariableSetVal var_val=var_val_list_.at(i);
			 common::ObRow input_row;
			 const ObObj *val = NULL;
			 if((ret=var_val.var_value_.calc(input_row, val))!=OB_SUCCESS)
			 {
                 TBSYS_LOG(WARN, "ObProcedureAssgin expr compute failed");
			 }
			 else if(!session->variable_exists(var_val.variable_name_))
			 {
				 ret=OB_ERR_VARIABLE_UNKNOWN;
				 TBSYS_LOG(USER_ERROR, "Variable %.*s does not declare", var_val.variable_name_.length(), var_val.variable_name_.ptr());
			 }
			 else if((ret=session->replace_variable(var_val.variable_name_,*val))!=OB_SUCCESS)
			 {
				 TBSYS_LOG(WARN, "replace_variable value_  ERROR");
			 }
			 else
			 {
				 TBSYS_LOG(DEBUG, "assgin %.*s and assgin value = %s",var_val.variable_name_.length(),var_val.variable_name_.ptr(),to_cstring(*val));
			 }
		}
	}
	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureAssgin, PHY_PROCEDURE_ASSGIN);
  }
}

int64_t ObProcedureAssgin::to_string(char* buf, const int64_t buf_len) const
{
    int64_t pos = 0;
    databuff_printf(buf, buf_len, pos, "ObProcedureAssgin (var_val_list_ size=%ld)\n", var_val_list_.count());
    return pos;
}
