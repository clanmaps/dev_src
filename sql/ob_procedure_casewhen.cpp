/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_casewhen.cpp
* @brief this class  present a procedure "casewhen" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_casewhen.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureCaseWhen::ObProcedureCaseWhen():child_num_(0)
{
}

ObProcedureCaseWhen::~ObProcedureCaseWhen()
{
}

int ObProcedureCaseWhen::set_expr(ObSqlExpression& expr)
{
	expr_=expr;
	expr_.set_owner_op(this);
	return OB_SUCCESS;
}
int ObProcedureCaseWhen::set_compare_expr(ObSqlExpression& expr)
{
	compare_expr_=expr;
	compare_expr_.set_owner_op(this);
	return OB_SUCCESS;
}
int ObProcedureCaseWhen::set_case_expr(ObSqlExpression& expr)
{
	case_expr_=expr;
	case_expr_.set_owner_op(this);
	return OB_SUCCESS;
}

void ObProcedureCaseWhen::reset()
{
	child_num_=0;
	ObMultiChildrenPhyOperator::reset();
}
void ObProcedureCaseWhen::reuse()
{
	child_num_=0;
	ObMultiChildrenPhyOperator::reset();
}
int ObProcedureCaseWhen::close()
{
	return ObMultiChildrenPhyOperator::close();
}

int ObProcedureCaseWhen::set_child(int32_t child_idx, ObPhyOperator &child_operator)
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
int32_t ObProcedureCaseWhen::get_child_num() const
{
      int child_num = child_num_;
      if (child_num_ < ObMultiChildrenPhyOperator::get_child_num())
      {
        child_num = ObMultiChildrenPhyOperator::get_child_num();
      }
      return child_num;
}

int ObProcedureCaseWhen::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureCaseWhen::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(ERROR, "zz:ObProcedureCaseWhen has no child, ret=%d", ret);
	}
	else
	{
		ret = get_child(0)->get_next_row(row);
	}
	return ret;
}


int ObProcedureCaseWhen::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureCaseWhen open()");
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
	    ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "ObProcedureCaseWhen has no child operator, ret=%d", ret);
	}
	else
	{
		//TODO
		common::ObRow tmp_row1,tmp_row2,tmp_row3;
		const ObObj *case_value = NULL;
		const ObObj *when_value = NULL;
		const ObObj *compare_value = NULL;
		if((ret=expr_.calc(tmp_row1, case_value))!=OB_SUCCESS)
		{
            TBSYS_LOG(WARN, "ObProcedureCaseWhen case_value expr compute failed");
		}
		else if((ret=case_expr_.calc(tmp_row2,when_value))!=OB_SUCCESS)
		{
            TBSYS_LOG(WARN, "ObProcedureCaseWhen when_value expr compute failed");
		}
		else if((ret=compare_expr_.calc(tmp_row3,compare_value))!=OB_SUCCESS)
		{
            TBSYS_LOG(WARN, "ObProcedureCaseWhen when_value expr compute failed");
		}
        else if(!compare_value->is_true())
		{
            TBSYS_LOG(DEBUG, "ObProcedureCaseWhen case expr != when expr");
			ret=OB_INVALID_ERROR;
		}
		else
		{
            TBSYS_LOG(DEBUG, "ObProcedureCaseWhen case expr == when expr");
			for (int32_t i = 0; ret == OB_SUCCESS && i < ObMultiChildrenPhyOperator::get_child_num(); i++)
			{
			  ObPhyOperator *op = NULL;
			  if ((op = get_child(i)) == NULL)
			  {
				ret = OB_ERR_GEN_PLAN;
				TBSYS_LOG(WARN, "Can not get %dth child of ObProcedureCaseWhen ret=%d", i, ret);
				break;
			  }
			  else if ((ret = op->open()) != OB_SUCCESS)
			  {
				if (!IS_SQL_ERR(ret))
				{
				  TBSYS_LOG(WARN, "Open the %dth child of ObProcedureCaseWhen failed, ret=%d", i, ret);
				}
				break;
			  }
			  else
			  {
                  TBSYS_LOG(DEBUG, "ObProcedureCaseWhen child open success!");
			  }
			}
		}

	}

	return ret;
}




namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureCaseWhen, PHY_PROCEDURE_CASE_WHEN);
  }
}

int64_t ObProcedureCaseWhen::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObProcedureCaseWhen (child_num_=%d)\n", child_num_);
  int64_t pos_temp=0;
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
