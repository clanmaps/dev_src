/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_first_into.cpp
* @brief this class  present a "fetch cursor first into" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_cursor_fetch_first_into.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObCursorFetchFirstInto::ObCursorFetchFirstInto()
{
}

ObCursorFetchFirstInto::~ObCursorFetchFirstInto()
{
}

int ObCursorFetchFirstInto::add_variable(ObString &var)
{
	variables_.push_back(var);
	return OB_SUCCESS;
}

void ObCursorFetchFirstInto::reset()
{

}
void ObCursorFetchFirstInto::reuse()
{

}
int ObCursorFetchFirstInto::close()
{
	return OB_SUCCESS;
}

int ObCursorFetchFirstInto::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObCursorFetchFirstInto::get_next_row(const common::ObRow *&row)
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

int ObCursorFetchFirstInto::open()
{
	TBSYS_LOG(INFO, "zz:ObCursorFetchFirstInto::open()");
	int ret = OB_SUCCESS;
	if (child_op_==NULL)
	{
		ret = OB_ERR_GEN_PLAN;
		TBSYS_LOG(ERROR, "child_op_ is NULL");
	}
	else
	{
		uint64_t table_id = OB_INVALID_ID;
		uint64_t column_id = OB_INVALID_ID;
		const ObRow *row;
		if((ret=child_op_->open())!=OB_SUCCESS)
		{
			TBSYS_LOG(WARN, "zz:child_op_ open failed");
		}
		else if((ret=child_op_->get_next_row(row))!=OB_SUCCESS)//取出一行
		{
			TBSYS_LOG(WARN, "zz:get_next_row failed");
		}
		else
		{
			ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
			for(int64_t i=0;i<variables_.count();i++)
			{
				const ObObj *cell = NULL;
				if(OB_SUCCESS !=(ret=row->raw_get_cell(i, cell, table_id, column_id)))//取出一列
				{
					TBSYS_LOG(WARN, "zz:raw_get_cell %ld failed",i);
					TBSYS_LOG(USER_ERROR, "declare cursor not have enough cell.");
				}
				else
				{
					ObString var_name=variables_.at(i);
					ObObj val;
					if(!session->variable_exists(var_name))
					{
						 ret=OB_ERR_VARIABLE_UNKNOWN;
						 TBSYS_LOG(USER_ERROR, "Variable %.*s does not declare", var_name.length(), var_name.ptr());
					}
					else
					{
						if((ret=session->get_variable_value(var_name,val))!=OB_SUCCESS)
						{
							TBSYS_LOG(WARN, "get_variable_value  ERROR");
						}
						else if(val.get_type()!=cell->get_type())
						{
							ret=OB_ERR_ILLEGAL_TYPE;
							TBSYS_LOG(USER_ERROR, "declare variable %.*s data type not suite table column", var_name.length(), var_name.ptr());
						}
						else if((ret=session->replace_variable(var_name,*cell))!=OB_SUCCESS)
						{
							TBSYS_LOG(WARN, "replace_variable  ERROR");
						}
					}

				}
			}
			child_op_->close();//不关闭会挂
		}
	}
	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObCursorFetchFirstInto, PHY_CURSOR_FETCH_FIRST_INTO);
  }
}

int64_t ObCursorFetchFirstInto::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObCursorFetchFirstInto (var list size=%ld)\n", variables_.count());
  return pos;
}
