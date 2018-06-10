/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_select_into.cpp
* @brief this class present a procedure "select into" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_select_into.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureSelectInto::ObProcedureSelectInto()
{
}

ObProcedureSelectInto::~ObProcedureSelectInto()
{
}

int ObProcedureSelectInto::add_variable(ObString &var)
{
	variables_.push_back(var);
	return OB_SUCCESS;
}

void ObProcedureSelectInto::reset()
{

}
void ObProcedureSelectInto::reuse()
{

}
int ObProcedureSelectInto::close()
{
	return OB_SUCCESS;
}

int ObProcedureSelectInto::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureSelectInto::get_next_row(const common::ObRow *&row)
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

int ObProcedureSelectInto::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureSelectInto open()");
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
		const ObRow *temp;

		if((ret=child_op_->open())!=OB_SUCCESS)
		{
            TBSYS_LOG(WARN, "child_op_ open failed");
		}
        else if((ret=child_op_->get_next_row(row))==OB_ITER_END)//get next row
		{
            ret=OB_SUCCESS;
            TBSYS_LOG(WARN, "empty result");//result set is empty
		}
		else if((ret=child_op_->get_next_row(temp))==OB_SUCCESS)
		{
			ret = OB_ITER_END;
			TBSYS_LOG(USER_ERROR, "result set exists not only one");

		}
		else
		{
			ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
			for(int64_t i=0;i<variables_.count();i++)
			{
				const ObObj *cell = NULL;
                if(OB_SUCCESS !=(ret=row->raw_get_cell(i, cell, table_id, column_id)))//get one cell
				{
                    TBSYS_LOG(WARN, "raw_get_cell %ld failed, ret = %d",i,ret);
				}
				else
				{
					ObString var_name=variables_.at(i);
					if((ret=session->replace_variable(var_name,*cell))!=OB_SUCCESS)
					{
						TBSYS_LOG(WARN, "replace_variable  ERROR");
					}
				}
			}
			child_op_->close();
		}
	}
	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureSelectInto, PHY_PROCEDURE_SELECT_INTO);
  }
}

int64_t ObProcedureSelectInto::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObProcedureSelectInto (var list size=%ld)\n", variables_.count());
  int64_t pos_temp=0;
  pos_temp=child_op_->to_string(buf+pos,buf_len-pos);
  pos+=pos_temp;
  return pos;
}
