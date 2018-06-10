/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_if.cpp
* @brief this class  present a procedure "if" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#include "ob_procedure_if.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureIf::ObProcedureIf():child_num_(0),have_elseif_(false),have_else_(false)
{
	else_op_=NULL;
}

ObProcedureIf::~ObProcedureIf()
{
}

//add by zz 2014-12-3
int ObProcedureIf::set_expr(ObSqlExpression& expr)
{
	expr_=expr;
	expr_.set_owner_op(this);
	return OB_SUCCESS;
}

int ObProcedureIf::set_have_elseif(bool flag)
{
	have_elseif_=flag;
	return OB_SUCCESS;
}

int ObProcedureIf::add_elseif_op(ObPhyOperator &elseif_op)
{
	return elseif_ops_.push_back(&elseif_op);
}

int ObProcedureIf::set_have_else(bool flag)
{
	have_else_=flag;
	return OB_SUCCESS;
}

int ObProcedureIf::set_else_op(ObPhyOperator &else_op)
{
	else_op_=&else_op;
	return OB_SUCCESS;
}

bool ObProcedureIf::is_have_elseif()
{
	return have_elseif_;
}

bool ObProcedureIf::is_have_else()
{
	return have_else_;
}

void ObProcedureIf::reset()
{
	else_op_=NULL;
	child_num_ = 0;
	have_elseif_=false;
	have_else_=false;
	ObMultiChildrenPhyOperator::reset();
}
void ObProcedureIf::reuse()
{
	else_op_=NULL;
	child_num_ = 0;
	have_elseif_=false;
	have_else_=false;
	ObMultiChildrenPhyOperator::reset();
}
int ObProcedureIf::close()
{
	return ObMultiChildrenPhyOperator::close();
}

int ObProcedureIf::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureIf::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
		ret = OB_NOT_INIT;
		TBSYS_LOG(WARN, "ObProcedureIf has no child, ret=%d", ret);
	}
	else
	{
		ret = get_child(0)->get_next_row(row);
	}
	return ret;
}
int ObProcedureIf::set_child(int32_t child_idx, ObPhyOperator &child_operator)
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
int32_t ObProcedureIf::get_child_num() const
{
      int child_num = child_num_;
      if (child_num_ < ObMultiChildrenPhyOperator::get_child_num())
      {
        child_num = ObMultiChildrenPhyOperator::get_child_num();
      }
      return child_num;
}
int ObProcedureIf::open()
{
    TBSYS_LOG(DEBUG, "ObProcedureIf open");
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(ObMultiChildrenPhyOperator::get_child_num() <= 0))
	{
	    ret = OB_NOT_INIT;
	    TBSYS_LOG(ERROR, "ObProcedureIf has no child operator, ret=%d", ret);
	}
	else
	{
		bool need_do_else=true;
		common::ObRow tmp_row;
		const ObObj *result = NULL;
		if((ret=expr_.calc(tmp_row, result))!=OB_SUCCESS)
		{
            TBSYS_LOG(ERROR, "ObProcedureIf expr compute failed");
		}
        else if(result->is_true())
		{
			need_do_else=false;
            TBSYS_LOG(DEBUG, "ObProcedureIf expr is true open then");
            /*open all "if" then operator*/
			for (int32_t i = 0; ret == OB_SUCCESS && i < ObMultiChildrenPhyOperator::get_child_num(); i++)
			{
			  ObPhyOperator *op = NULL;
			  if ((op = get_child(i)) == NULL)
			  {
				ret = OB_ERR_GEN_PLAN;
				TBSYS_LOG(WARN, "Can not get %dth child of ObProcedureIf ret=%d", i, ret);
				break;
			  }
			  else if ((ret = op->open()) != OB_SUCCESS)
			  {
				if (!IS_SQL_ERR(ret))
				{
				  TBSYS_LOG(WARN, "Open the %dth child of ObProcedureIf failed, ret=%d", i, ret);
				}
				break;
			  }
			  else
			  {
                  TBSYS_LOG(DEBUG, "ObProcedureIf child open success!");
			  }
			}
		}
        else if(is_have_elseif())
		{
            TBSYS_LOG(DEBUG, "ObProcedureIf is_have_elseif elseif_op num=%ld",elseif_ops_.count());
            //foreach open all of "elseif" operator,and if "elseif" branch excute return sucess break this loop
			for (int32_t i = 0; i < elseif_ops_.count(); ++i)
			{
				 ObPhyOperator *else_op = elseif_ops_.at(i);
				 if ((ret = else_op->open()) != OB_INVALID_ERROR)
				 {
                     TBSYS_LOG(DEBUG, "else_op i=%d item is true! break",i);
                     //present have one "elseif" execute sucess,break
					 need_do_else=false;
					 break;
				 }
				 else
				 {
                     TBSYS_LOG(DEBUG, "else_op i=%d item is false! go on!",i);
				 }
			}
            ret=OB_SUCCESS;
            TBSYS_LOG(DEBUG, "else_op have over! need_do_else=%d",need_do_else);
		}
        if(is_have_else()&&need_do_else&&(ret=else_op_->open())!=OB_SUCCESS)
		{
            TBSYS_LOG(ERROR, "ObProcedureIf need else but else_op open faild");
		}

	}

	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureIf, PHY_PROCEDURE_IF);
  }
}

int64_t ObProcedureIf::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObProcedureIf (child_num_=%d)\n", child_num_);
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
