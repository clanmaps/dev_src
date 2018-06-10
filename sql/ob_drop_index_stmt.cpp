/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_drop_index_stmt.cpp
 * @brief for logical plan of drop index
 *
 * Created by longfei：for resolving logical plan of drop index
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_26
 */
#include "ob_drop_index_stmt.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

int ObDropIndexStmt::generate_inner_index_table_name(
    ObString& idx_name,
    ObString& ori_tab_name,
    char* inner_idx_name,
    int64_t& len)
{
  int ret = OB_SUCCESS;
  char str[OB_MAX_TABLE_NAME_LENGTH];
  char raw[OB_MAX_TABLE_NAME_LENGTH];
  memset(str,0,OB_MAX_TABLE_NAME_LENGTH);
  memset(raw,0,OB_MAX_TABLE_NAME_LENGTH);
  if(idx_name.length() > OB_MAX_TABLE_NAME_LENGTH || ori_tab_name.length() > OB_MAX_TABLE_NAME_LENGTH)
  {
    TBSYS_LOG(WARN,"buff is not enough to generate index table name");
    ret=OB_ERROR;
  }
  else
  {
    strncpy(str, idx_name.ptr(), idx_name.length());
    strncpy(raw, ori_tab_name.ptr(), ori_tab_name.length());
    int wlen = snprintf(inner_idx_name, OB_MAX_TABLE_NAME_LENGTH, "___%s_%s",  raw, str);
    if((size_t)wlen > (size_t)OB_MAX_TABLE_NAME_LENGTH)
    {
      ret = OB_ERROR;
    }
    len = wlen;
  }
  return ret;
}

const ObString& oceanbase::sql::ObDropIndexStmt::getOriTabName() const
{
  return ori_tab_name_;
}

void oceanbase::sql::ObDropIndexStmt::setOriTabName(const ObString& oriTabName)
{
  ori_tab_name_ = oriTabName;
}

bool oceanbase::sql::ObDropIndexStmt::isDrpAll() const
{
  return drp_all_;
}

void oceanbase::sql::ObDropIndexStmt::setDrpAll(bool drpAll)
{
  drp_all_ = drpAll;
}
