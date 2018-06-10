/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_create.cpp
 * @brief the ObProcedureCreate class is a create procedure PhyOperator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_26
 */

#include "ob_procedure_create.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
//add by wangdonghui 20160120 :b
#include "mergeserver/ob_rs_rpc_proxy.h"
//add :e
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureCreate::ObProcedureCreate()
{
    //delete by wangdonghui 20160128 :b
    //insert_op_=NULL;
    //delete :e
}

ObProcedureCreate::~ObProcedureCreate()
{
//  insert_op_->~ObPhyOperator();
}

int ObProcedureCreate::set_proc_name(ObString &proc_name)
{
	proc_name_=proc_name;
	return OB_SUCCESS;
}

//add by wangdonghui 20160121 :b
int ObProcedureCreate::set_proc_source_code(ObString &proc_source_code)
{
    proc_source_code_ = proc_source_code;
    return OB_SUCCESS;
}
//add :e

//delete by wangdonghui 20160128 :b
//int ObProcedureCreate::set_insert_op(ObPhyOperator &insert_op)
//{
//	insert_op_=&insert_op;
//	return OB_SUCCESS;
//}
//delete :e

void ObProcedureCreate::reset()
{
  //add by wangdonghui 20160120 :b
  if_not_exists_ = false;
  local_context_.rs_rpc_proxy_ = NULL;
  //add :e
}
void ObProcedureCreate::reuse()
{
  //add by wangdonghui 20160120 :b
  if_not_exists_ = false;
  local_context_.rs_rpc_proxy_ = NULL;
  //add :e
}
int ObProcedureCreate::close()
{
  return OB_SUCCESS;
}
//add by wangdonghui 20160120 :b
void ObProcedureCreate::set_sql_context(const ObSqlContext &context)
{
  local_context_ = context;
}
//add :e
int ObProcedureCreate::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureCreate::get_next_row(const common::ObRow *&row)
{
	int ret = OB_SUCCESS;
	if (NULL == child_op_)
	{
		ret = OB_ERR_UNEXPECTED;
    TBSYS_LOG(WARN, "child_op_ is NULL");
	}
	else
	{
	  ret = child_op_->get_next_row(row);
	}
	return ret;
}

int ObProcedureCreate::open()
{
	int ret = OB_SUCCESS;
	if (NULL == child_op_)
	{
		ret = OB_ERR_GEN_PLAN;
        TBSYS_LOG(WARN, "child_op_ is NULL");
	}
	else
	{
		//ret=child_op_->open();
		/*
		ObPhyOperator* old_main_query = my_phy_plan_->get_main_query();
		my_phy_plan_->set_main_query(child_op_);
		my_phy_plan_->remove_phy_query(old_main_query);
		TBSYS_LOG(INFO, "prepare main query=%p", child_op_);
		ObResultSet *result_set = my_phy_plan_->get_result_set();
		ObSQLSessionInfo *session = result_set->get_session();
		if ((ret = session->store_plan(proc_name_, *result_set)) != OB_SUCCESS)
		{
			TBSYS_LOG(WARN, "Store current result failed.");
		}
		else
		{
			TBSYS_LOG(INFO, "zz:ObProcedureCreate store proc_name_=%s plan  success!",proc_name_.ptr());
		}
		*/
        //delete by wangdonghui 20160128 we do the insert operation in RS :b

//        if((ret=insert_op_->open())!=OB_SUCCESS)//打开插入表的insert操作符
//        {
//          ret=-17;
//          ObResultSet *result_set = my_phy_plan_->get_result_set();
//          result_set->set_message("procedure exist!");
//          TBSYS_LOG(USER_ERROR, "procedure exist!");
//        }
//        //add by wangdonghui 20160119 send procedure source and proc_name to RS. :b
//        //Maybe we can delete insert_op_ and create procedure in RS
//        else

        //delete :e

        {
            TBSYS_LOG(INFO, "before rpc: proc name %.*s, proc source code %.*s",  proc_name_.length(),proc_name_.ptr(), proc_source_code_.length(), proc_source_code_.ptr());
            if(NULL == local_context_.rs_rpc_proxy_)
            {
              ret = OB_NOT_INIT;
              TBSYS_LOG(ERROR, "not init, rpc_=%p", local_context_.rs_rpc_proxy_);
            }
            else if(OB_SUCCESS != (ret = local_context_.rs_rpc_proxy_->create_procedure(if_not_exists_, proc_name_, proc_source_code_)))
            {
                TBSYS_LOG(WARN, "failed to create procedure, err=%d", ret);
            }
            else
            {
                my_phy_plan_->get_result_set()->set_affected_rows(1);
            }
        }
        //:e
//		TBSYS_LOG(INFO, "procedure plan is %s",buff);
	}
	return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureCreate, PHY_PROCEDURE_CREATE);
  }
}

int64_t ObProcedureCreate::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "Create Procedure(%.*s)\n", proc_name_.length(), proc_name_.ptr());

  databuff_printf(buf, buf_len, pos, "Procedure execution plan: \n");
  pos += child_op_->to_string(buf+pos, buf_len-pos);

  //delete by wangdonghui 20160128 :b
  //databuff_printf(buf, buf_len, pos, "Save procedure plan: \n");
  //pos += insert_op_->to_string(buf+pos,buf_len-pos);
  //delete :e

  return pos;
}
