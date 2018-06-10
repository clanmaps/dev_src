/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_execute.cpp
 * @brief the ObProcedureExecute class definition that wrap procedure execute physical operator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#include "ob_procedure_execute.h"
#include "ob_procedure_stmt.h"
#include "ob_procedure.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
#include "ob_cursor_close.h"
#include "ob_phy_operator.h"
#include "ob_single_child_phy_operator.h"
#include "ob_deallocate.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObProcedureExecute::ObProcedureExecute()
{
	stmt_id_=common::OB_INVALID_ID;
}

ObProcedureExecute::~ObProcedureExecute()
{
}

int ObProcedureExecute::set_proc_name(const ObString &proc_name)
{
	proc_name_=proc_name;
	return OB_SUCCESS;
}
int ObProcedureExecute::set_stmt_id(uint64_t stmt_id)
{
	stmt_id_=stmt_id;
	return OB_SUCCESS;
}

int ObProcedureExecute::add_param_expr(ObSqlExpression& expr)
{
  expr.set_owner_op(this);
  return param_list_.push_back(expr);
}

int64_t ObProcedureExecute::get_param_size() const
{
	return param_list_.count();
}
//add by wdh 20160718 :b
int ObProcedureExecute::set_no_group(bool no_group)
{
  no_group_ = no_group;
  return OB_SUCCESS;
}
bool ObProcedureExecute::get_no_group()
{
  return no_group_;
}
//add :e
//add by qx 20170317 :b
bool ObProcedureExecute::get_long_trans()
{
  return long_trans_;
}
int ObProcedureExecute::set_long_trans(bool long_trans)
{
  long_trans_ = long_trans;
  return OB_SUCCESS;
}
//add :e
void ObProcedureExecute::reset()
{
  stmt_id_=common::OB_INVALID_ID;
  ObSingleChildPhyOperator::reset();
}

void ObProcedureExecute::reuse()
{
  stmt_id_=common::OB_INVALID_ID;
  ObSingleChildPhyOperator::reuse();
}

int ObProcedureExecute::close()
{
  return ObSingleChildPhyOperator::close();
}

int ObProcedureExecute::get_row_desc(const common::ObRowDesc *&row_desc) const
{
	int ret = OB_SUCCESS;
	if (OB_UNLIKELY(NULL == child_op_))
	{
    ret = OB_NOT_INIT;
    TBSYS_LOG(WARN, "child_op_ is NULL");
	}
	else
	{
		ret = child_op_->get_row_desc(row_desc);
	}
	return ret;
}

int ObProcedureExecute::get_next_row(const common::ObRow *&row)
{
  int ret = OB_ITER_END;
  UNUSED(row);
	return ret;
}

int ObProcedureExecute::open()
{
  int ret = OB_SUCCESS;
  ObProcedure *proc = NULL;
  ObResultSet *result_set = NULL;
  ObPhysicalPlan *physical_plan = NULL;
  ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
  if( (result_set = session->get_plan(stmt_id_)) == NULL
      ||  (physical_plan = result_set->get_physical_plan()) == NULL
      ||  (proc = dynamic_cast<ObProcedure*>(physical_plan->get_main_query())) == NULL)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(ERROR, "Find stored procedure plan failed, stmt_id_: %ld", stmt_id_);
  }
  else if (OB_SUCCESS != (ret = set_child(0, *proc)) )
  {
    TBSYS_LOG(ERROR, "Failed to set main query");
  }
  else{
    result_set->set_running_procedure(proc);

    session->set_current_result_set(result_set); 
  }
  if ( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = proc->fill_parameters(param_list_)) )
  {
    TBSYS_LOG(WARN, "fill paramters fail");
  }
  else if( OB_SUCCESS != (ret=result_set->open()) )
  {
    TBSYS_LOG(WARN, "procedure execute error!");
  }
  else
  {
    TBSYS_LOG(TRACE, "procedure execute success!");
  }
  return ret;
}


namespace oceanbase
{
  namespace sql
  {
    REGISTER_PHY_OPERATOR(ObProcedureExecute, PHY_PROCEDURE_EXEC);
  }
}

int64_t ObProcedureExecute::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "procedure execute(stmt_id=%ld)\n", stmt_id_);
  return pos;
}
