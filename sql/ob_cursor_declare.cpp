/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_declare.cpp
* @brief this class  present a declare cursor logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#include "ob_cursor_declare.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "parse_malloc.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObCursorDeclare::ObCursorDeclare()
{
}

ObCursorDeclare::~ObCursorDeclare()
{
}

int ObCursorDeclare::get_row_desc(const common::ObRowDesc *&row_desc) const
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

int ObCursorDeclare::open()
{
  int ret = OB_SUCCESS;
  if (cursor_name_.length() <= 0 || child_op_ == NULL)
  {
    ret = OB_ERR_GEN_PLAN;
    TBSYS_LOG(WARN, "Prepare statement is not initiated");
  }
  else
  {
    ObPhyOperator* old_main_query = my_phy_plan_->get_main_query();
    my_phy_plan_->set_main_query(child_op_);
    my_phy_plan_->remove_phy_query(old_main_query);
    ret = store_phy_plan_to_session();
  }
  return ret;
}

int ObCursorDeclare::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  ret = child_op_->get_next_row(row);
  return ret;
}

int ObCursorDeclare::close()
{
  int ret = OB_SUCCESS;
  return ret;
}

int ObCursorDeclare::store_phy_plan_to_session()
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(DEBUG, "prepare main query=%p", child_op_);
  ObResultSet *result_set = my_phy_plan_->get_result_set();
  ObSQLSessionInfo *session = result_set->get_session();
  if ((ret = session->store_plan(cursor_name_, *result_set)) != OB_SUCCESS)
  {
    TBSYS_LOG(WARN, "Store current result failed.");
  }
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObCursorDeclare, PHY_CURSOR_DECLARE);
  }
}

int64_t ObCursorDeclare::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "Declare(cursor_name=%.*s)\n", cursor_name_.length(), cursor_name_.ptr());
  return pos;
}
