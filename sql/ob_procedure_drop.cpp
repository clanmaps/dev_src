/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_drop.cpp
 * @brief the ObProcedureDrop class definition that warp procedure drop PhyOperator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#include "ob_procedure_drop.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"

//add by wangdonghui 20160225 [drop procedure] :b
#include "mergeserver/ob_rs_rpc_proxy.h"
//add :e

using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureDrop::ObProcedureDrop()
  :if_exists_(false), rpc_(NULL)
{
    //delete by wangdonghui 20160225 [drop procedure] :b
    //delete_op_=NULL;
    //delete :e
}

ObProcedureDrop::~ObProcedureDrop()
{
}
int ObProcedureDrop::set_proc_name(ObString &proc_name)
{
	proc_name_=proc_name;
	return OB_SUCCESS;
}
//delete by wangdonghui 20160225 [drop procedure] :b
//int ObProcedureDrop::set_delete_op(ObPhyOperator &delete_op)
//{
//	delete_op_=&delete_op;
//	return OB_SUCCESS;
//}
//delete :e
void ObProcedureDrop::set_if_exists(bool flag)
{
	if_exists_=flag;
}
bool ObProcedureDrop::if_exists()
{
	return if_exists_;
}


void ObProcedureDrop::reset()
{
    //add by wangdonghui 20160225 [drop procedure] :b
    if_exists_ = false;
    rpc_ = NULL;
    //add :e
}
void ObProcedureDrop::reuse()
{
    if_exists_ = false;
    rpc_ = NULL;
}
int ObProcedureDrop::close()
{
	return OB_SUCCESS;
}

int ObProcedureDrop::get_row_desc(const common::ObRowDesc *&row_desc) const
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
int ObProcedureDrop::get_next_row(const common::ObRow *&row)
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

int ObProcedureDrop::open()
{
	TBSYS_LOG(INFO, "zz:ObProcedureDrop::open()");
	int ret = OB_SUCCESS;
    //add by wangdonghui 20160225 [drop procedure] :b
    if(rpc_ == NULL)
    {
        ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "not init, rpc_=%p", rpc_);
    }
    else if(OB_SUCCESS != (ret = rpc_->drop_procedure(if_exists_, proc_name_)))
    {
        if(if_exists_)
        {
            ret = OB_SUCCESS;
        }
        else
        {
            TBSYS_LOG(WARN, "failed to drop procedure, err=%d", ret);
        }
    }
    else
    {
        TBSYS_LOG(INFO, "drop procedure succ, proc_name=[%s] if_exists=%c",
                      to_cstring(proc_name_), if_exists_?'Y':'N');
    }
    return ret;
    //add :e
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedureDrop, PHY_PROCEDURE_DROP);
  }
}

int64_t ObProcedureDrop::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "procedure delete(procedure name =%.*s)\n", proc_name_.length(),proc_name_.ptr());
  return pos;
}
