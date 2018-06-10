/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_else.h
* @brief this class  present a procedure "else" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_else.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureElse::ObProcedureElse()
{
	child_num_ = 0;
}

ObProcedureElse::~ObProcedureElse()
{
}
void ObProcedureElse::reset()
{
	child_num_ = 0;
	ObMultiChildrenPhyOperator::reset();
}
void ObProcedureElse::reuse()
{
	child_num_ = 0;
	ObMultiChildrenPhyOperator::reset();
}
int ObProcedureElse::close()
{
	return ObMultiChildrenPhyOperator::close();
}

int ObProcedureElse::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureElse::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(WARN, "ObProcedureElse has no child, ret=%d", ret);
	}
	else
	{
		ret = get_child(0)->get_next_row(row);
	}
	return ret;
}
int ObProcedureElse::set_child(int32_t child_idx, ObPhyOperator &child_operator)
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
int32_t ObProcedureElse::get_child_num() const
{
      int child_num = child_num_;
      if (child_num_ < ObMultiChildrenPhyOperator::get_child_num())
      {
        child_num = ObMultiChildrenPhyOperator::get_child_num();
      }
      return child_num;
}
int ObProcedureElse::open()
{
	TBSYS_LOG(DEBUG, "zz:ObProcedureElse::open()");
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
	    ret = OB_NOT_INIT;
	    TBSYS_LOG(ERROR, "ObProcedureElse has no child operator, ret=%d", ret);
	}
	else
	{
		TBSYS_LOG(DEBUG, "zz:ObProcedureElse child number=%d",ObMultiChildrenPhyOperator::get_child_num());
		for (int32_t i = 0; ret == OB_SUCCESS && i < ObMultiChildrenPhyOperator::get_child_num(); i++)
		{
		  ObPhyOperator *op = NULL;
		  if ((op = get_child(i)) == NULL)
		  {
			ret = OB_ERR_GEN_PLAN;
			TBSYS_LOG(WARN, "Can not get %dth child of ObProcedureElse ret=%d", i, ret);
			break;
		  }
		  else if ((ret = op->open()) != OB_SUCCESS)
		  {
			if (!IS_SQL_ERR(ret))
			{
			  TBSYS_LOG(WARN, "Open the %dth child of ObProcedureElse failed, ret=%d", i, ret);
			}
			break;
		  }
		  else
		  {
			  TBSYS_LOG(DEBUG, "Open the %dth child of ObProcedureElse success", i);
		  }
		}
	}

	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureElse, PHY_PROCEDURE_ELSE);
  }
}

int64_t ObProcedureElse::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "procedure else(child_num_=%d)\n", child_num_);
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
