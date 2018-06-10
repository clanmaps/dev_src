/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_case.cpp
* @brief this class  present a procedure "case" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_case.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureCase::ObProcedureCase():child_num_(0),have_else_(false)
{
	else_op_=NULL;
}

ObProcedureCase::~ObProcedureCase()
{
}

int ObProcedureCase::set_expr(ObSqlExpression& expr)
{
	expr_=expr;
	expr_.set_owner_op(this);
	return OB_SUCCESS;
}

int ObProcedureCase::set_have_else(bool flag)
{
	have_else_=flag;
	return OB_SUCCESS;
}

int ObProcedureCase::set_else_op(ObPhyOperator &else_op)
{
	else_op_=&else_op;
	return OB_SUCCESS;
}

bool ObProcedureCase::is_have_else()
{
	return have_else_;
}

void ObProcedureCase::reset()
{
	else_op_=NULL;
	child_num_ = 0;
	have_else_=false;
	ObMultiChildrenPhyOperator::reset();
}
void ObProcedureCase::reuse()
{
	else_op_=NULL;
	child_num_ = 0;
	have_else_=false;
	ObMultiChildrenPhyOperator::reset();
}
int ObProcedureCase::close()
{
	return ObMultiChildrenPhyOperator::close();
}

int ObProcedureCase::get_row_desc(const common::ObRowDesc *&row_desc) const
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(get_child(0) == NULL))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(ERROR, "children_ops_[0] is NULL");
	}
	else
	{
		ret = get_child(0)->get_row_desc(row_desc);
	}
	return ret;
}
int ObProcedureCase::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(ERROR, "ObProcedureCase has no child, ret=%d", ret);
	}
	else
	{
		ret = get_child(0)->get_next_row(row);
	}
	return ret;
}
int ObProcedureCase::set_child(int32_t child_idx, ObPhyOperator &child_operator)
{
    int ret = OB_SUCCESS;
    if ((ret = ObMultiChildrenPhyOperator::set_child(child_idx, child_operator)) == OB_SUCCESS)
    {
        if (ObMultiChildrenPhyOperator::get_child_num() > child_num_)
        {
            child_num_++;
        }
    }
    return ret;
}
int32_t ObProcedureCase::get_child_num() const
{
      int child_num = child_num_;
      if (child_num_ < ObMultiChildrenPhyOperator::get_child_num())
      {
        child_num = ObMultiChildrenPhyOperator::get_child_num();
      }
      return child_num;
}
int ObProcedureCase::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureCase open");
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
	    ret = OB_NOT_INIT;
	    TBSYS_LOG(ERROR, "ObProcedureCase has no child operator, ret=%d", ret);
	}
	else
	{
		bool need_do_else=true;
        TBSYS_LOG(DEBUG, "ObProcedureCase num=%d",child_num_);
		for (int32_t i = 0; i < ObMultiChildrenPhyOperator::get_child_num(); i++)
		{
		  ObPhyOperator *op = NULL;
		  if ((op = get_child(i)) == NULL)
		  {
			ret = OB_ERR_GEN_PLAN;
			TBSYS_LOG(WARN, "Can not get %dth child of ObProcedureCase ret=%d", i, ret);
			break;
		  }
          else if ((ret = op->open()) != OB_INVALID_ERROR)
		  {
			need_do_else=false;
			break;
		  }
		  else
		  {
              TBSYS_LOG(DEBUG, "ObProcedureCase i=%d item is false! go on!",i);
		  }
		}
		if(is_have_else()&&need_do_else&&(ret=else_op_->open())!=OB_SUCCESS)
		{
            TBSYS_LOG(ERROR, "ObProcedureCase need else but else_op open faild");
		}
		if(ret == OB_INVALID_ERROR)
		{
			TBSYS_LOG(USER_ERROR, "case not found for case statement.");
		}
	}

	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureCase, PHY_PROCEDURE_CASE);
  }
}

int64_t ObProcedureCase::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  int64_t pos_temp=0;
  databuff_printf(buf, buf_len, pos, "ObProcedureCase (child_num_=%d)\n", child_num_);
  for (int32_t i = 0; i < ObMultiChildrenPhyOperator::get_child_num(); i++)
  {
	  ObPhyOperator *op = NULL;
	  if ((op = get_child(i)) == NULL)
	  {
		  break;
	  }
	  else
	  {
		  pos_temp=op->to_string(buf+pos, buf_len-pos);
		  pos+=pos_temp;
	  }
  }
  return pos;
}
