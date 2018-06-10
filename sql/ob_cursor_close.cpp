/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_close.cpp
* @brief this class  present a cursor close physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#include "ob_cursor_close.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObCursorClose::ObCursorClose()
  :stmt_id_(OB_INVALID_ID)
{
}

ObCursorClose::~ObCursorClose()
{
}

void ObCursorClose::reset()
{
  stmt_id_ = OB_INVALID_ID;
  ObSingleChildPhyOperator::reset();
}

void ObCursorClose::reuse()
{
  stmt_id_ = OB_INVALID_ID;
  ObSingleChildPhyOperator::reuse();
}

int ObCursorClose::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(NULL == child_op_))
  {
    ObResultSet *result_set = NULL;
    ObPhysicalPlan *physical_plan = NULL;
    ObPhyOperator *main_query = NULL;
    if ((result_set = my_phy_plan_->get_result_set()->get_session()->get_plan(cursor_name_)) == NULL
      || (physical_plan = result_set->get_physical_plan()) == NULL
      || (main_query = physical_plan->get_main_query()) == NULL)
    {
      ret = OB_NOT_INIT;
      TBSYS_LOG(ERROR, "Stored session plan can not be fount or not correct");
    }
    else
    {
      ret = main_query->get_row_desc(row_desc);
    }
  }
  else
  {
    ret = child_op_->get_row_desc(row_desc);
  }
  return ret;
}

int ObCursorClose::open()
{
  TBSYS_LOG(INFO, "ObCursorClose::open()");
  int ret = OB_SUCCESS;
   // get stored executing plan
   ObResultSet *result_set = NULL;
   ObPhysicalPlan *physical_plan = NULL;
   ObPhyOperator *main_query = NULL;
   ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
   if ((result_set = session->get_plan(cursor_name_)) == NULL
     || (physical_plan = result_set->get_physical_plan()) == NULL
     || (main_query = physical_plan->get_main_query()) == NULL)
   {
     ret = OB_NOT_INIT;
     TBSYS_LOG(ERROR, "Stored session plan can not be fount or not correct, result_set=%p main_query=%p phy_plan=%p",
               result_set, main_query, physical_plan);
   }
   else
   {
	 if (session->plan_exists(cursor_name_, &stmt_id_) == false)
	 {
	   ret = OB_ERR_PREPARE_STMT_UNKNOWN;

	 }
	 else
	 {
	   ret = main_query->close();                //delete result
	  // ret =  session->remove_plan(stmt_id_);    //delete physical plan
     }
   }
  return ret;
}

int ObCursorClose::close()
{
  return ObSingleChildPhyOperator::close();
}

int ObCursorClose::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  ret = child_op_->get_next_row(row);
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObCursorClose, PHY_CURSOR_CLOSE);
  }
}

int64_t ObCursorClose::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "Close(stmt_id_=<%lu>, \n", stmt_id_);
  return pos;
}


