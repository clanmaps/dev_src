/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_open.cpp
* @brief this class  present a "cursor open" physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#include "ob_cursor.h"
#include "ob_cursor_open.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObCursorOpen::ObCursorOpen()
{
}

ObCursorOpen::~ObCursorOpen()
{
}

void ObCursorOpen::reset()
{
  ObSingleChildPhyOperator::reset();
}

void ObCursorOpen::reuse()
{
  ObSingleChildPhyOperator::reuse();
}

int ObCursorOpen::get_row_desc(const common::ObRowDesc *&row_desc) const
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

int ObCursorOpen::open()
{
  int ret = OB_SUCCESS;
  // get stored executing plan
  ObResultSet *result_set = NULL;
  ObResultSet *my_result_set = NULL;
  ObPhysicalPlan *physical_plan = NULL;
  ObPhyOperator *main_query = NULL;
  ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
  if ((result_set = session->get_plan(cursor_name_)) == NULL         //get physical plan from session
    || (physical_plan = result_set->get_physical_plan()) == NULL
    || (main_query = physical_plan->get_main_query()) == NULL)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(ERROR, "Stored session plan can not be fount or not correct, result_set=%p main_query=%p phy_plan=%p",
              result_set, main_query, physical_plan);
  }
  else
  {
    my_result_set = my_phy_plan_->get_result_set();
    ObCursor* cursor_op = static_cast<ObCursor*>(main_query);
    if(1 == cursor_op->get_is_opened() )
    {
  	  ret = OB_READ_NOTHING;
      TBSYS_LOG(WARN , "cursor has been opened ,cursor name=%s",cursor_name_.ptr());
    }
    else
    {
       if ( (ret = result_set->open()) != OB_SUCCESS)                //execute physical plan
       {
         TBSYS_LOG(WARN, "failed to open result set, err=%d", ret);
       }
       my_result_set->set_affected_rows(cursor_op->get_row_num());   //total rows of result
    }
  }
  return ret;
}

int ObCursorOpen::close()
{
   int ret = OB_SUCCESS;
   return ret;
   // return ObSingleChildPhyOperator::close();
}

int ObCursorOpen::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if (NULL == child_op_)
  {
    ret = OB_ERR_UNEXPECTED;
    TBSYS_LOG(ERROR, "child_op_ must not NULL");
  }
  else
  {
    ret = child_op_->get_next_row(row);
    TBSYS_LOG(DEBUG, "open get next row");
  }
   return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObCursorOpen, PHY_CURSOR_OPEN);
    }
 }

int64_t ObCursorOpen::to_string(char* buf, const int64_t buf_len) const
{
    int64_t pos = 0;
    databuff_printf(buf, buf_len, pos,"Open (cursor_name=%.*s)\n", cursor_name_.length(), cursor_name_.ptr());
    return pos;
}
