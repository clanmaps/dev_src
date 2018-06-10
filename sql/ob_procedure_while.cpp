/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_while.cpp
* @brief this class present a procedure "while" loop
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_while.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureWhile::ObProcedureWhile():child_num_(0)
{
}

ObProcedureWhile::~ObProcedureWhile()
{
}

//add by zz 2014-12-3
int ObProcedureWhile::set_expr(ObSqlExpression& expr)
{
	expr_=expr;
	expr_.set_owner_op(this);
	return OB_SUCCESS;
}

void ObProcedureWhile::reset()
{
	child_num_=0;
	ObMultiChildrenPhyOperator::reset();
}
void ObProcedureWhile::reuse()
{
	child_num_=0;
	ObMultiChildrenPhyOperator::reset();
}
int ObProcedureWhile::close()
{
	return ObMultiChildrenPhyOperator::close();
}

int ObProcedureWhile::set_child(int32_t child_idx, ObPhyOperator &child_operator)
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
int32_t ObProcedureWhile::get_child_num() const
{
      int child_num = child_num_;
      if (child_num_ < ObMultiChildrenPhyOperator::get_child_num())
      {
        child_num = ObMultiChildrenPhyOperator::get_child_num();
      }
      return child_num;
}

int ObProcedureWhile::get_row_desc(const common::ObRowDesc *&row_desc) const
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(get_child(0) == NULL))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(WARN, "children_ops_[0] is NULL");
	}
	else
	{
		ret = get_child(0)->get_row_desc(row_desc);
	}
	return ret;
}
int ObProcedureWhile::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
		ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "ObProcedureElseIf no child, ret=%d", ret);
	}
	else
	{
		ret = get_child(0)->get_next_row(row);
	}
	return ret;
}


int ObProcedureWhile::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureWhile open()");
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
	    ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "ObProcedureElseIf no child operator, ret=%d", ret);
	}
	else
	{

		while(true)
		{
			common::ObRow tmp_row;
			const ObObj *result = NULL;
			bool is_error=false;
			if((ret=expr_.calc(tmp_row, result))!=OB_SUCCESS)
			{
                TBSYS_LOG(ERROR, "ObProcedureWhile expr compute failed");
				break;
			}
			else if(result->is_true())
			{
                TBSYS_LOG(DEBUG, "ObProcedureWhile expr value is true");

                TBSYS_LOG(DEBUG, "ObProcedureWhile child_num=%d",ObMultiChildrenPhyOperator::get_child_num());
                /*open while then*/
				for (int32_t i = 0; ret == OB_SUCCESS && i < ObMultiChildrenPhyOperator::get_child_num(); i++)
				{
				  ObPhyOperator *op = NULL;
				  if ((op = get_child(i)) == NULL)
				  {
					ret = OB_ERR_GEN_PLAN;
					is_error=true;
					TBSYS_LOG(WARN, "Can not get %dth child of ObProcedureWhile ret=%d", i, ret);
					break;
				  }
				  else if ((ret = op->open()) != OB_SUCCESS)
				  {
					is_error=true;
					TBSYS_LOG(WARN, "Open the %dth child of ObProcedureWhile failed, ret=%d", i, ret);
					break;
				  }
				  else
				  {
					  TBSYS_LOG(DEBUG, "Open the %dth child of ObProcedureWhile success", i);
				  }
				}
				if(is_error)
				{
					TBSYS_LOG(WARN, "ObProcedureWhile child_op have error! break");
					break;
				}
			}
			else
			{
                TBSYS_LOG(DEBUG, "ObProcedureWhile expr value is false break");
				break;
			}
		}
	}

	return ret;
}




namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureWhile, PHY_PROCEDURE_WHILE);
  }
}

int64_t ObProcedureWhile::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObProcedureWhile (child_num_=%d)\n", child_num_);
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
