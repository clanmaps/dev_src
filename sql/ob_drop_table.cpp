/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_drop_table.cpp
 * @brief physical operator for drop table
 *
 * modified by longfei：if this table has index, drop table will drop index first
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_22
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_drop_table.cpp
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#include "sql/ob_drop_table.h"
#include "common/utility.h"
#include "mergeserver/ob_rs_rpc_proxy.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;
using namespace oceanbase::mergeserver; // longfei for drop index
ObDropTable::ObDropTable()
  :if_exists_(false), rpc_(NULL),has_indexs_(false)
{
}

ObDropTable::~ObDropTable()
{
}

void ObDropTable::reset()
{
  if_exists_ = false;
  rpc_ = NULL;
  // longfei [drop index]
  has_indexs_ = false;
}

void ObDropTable::reuse()
{
  if_exists_ = false;
  rpc_ = NULL;
  // longfei [drop index]
  has_indexs_ = false;
}

void ObDropTable::set_if_exists(bool if_exists)
{
  if_exists_ = if_exists;
}

const bool ObDropTable::get_if_exists() const
{
  return if_exists_;
}

ObMergerRootRpcProxy* ObDropTable::get_rpc_stub() const
{
  return rpc_;
}

int ObDropTable::add_table_name(const ObString &tname)
{
  return tables_.add_string(tname);
}

int ObDropTable::open()
{
  int ret = OB_SUCCESS;
  if (NULL == rpc_ || 0 >= tables_.count())
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(ERROR, "not init, rpc_=%p", rpc_);
  }
  // add longfei [drop index] 20151028
  else if(has_indexs_ && OB_SUCCESS != (ret = rpc_->drop_index(if_exists_, all_indexs_)))
  {
    TBSYS_LOG(WARN, "failed to drop index on table, err=%d", ret);
  }
  // add e
  else if (OB_SUCCESS != (ret = rpc_->drop_table(if_exists_, tables_)))
  {
    TBSYS_LOG(WARN, "failed to drop table, err=%d", ret);
  }
  else
  {
    TBSYS_LOG(INFO, "drop table succ, tables=[%s] if_exists=%c",
              to_cstring(tables_), if_exists_?'Y':'N');
  }
  return ret;
}

int ObDropTable::close()
{
  int ret = OB_SUCCESS;
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObDropTable, PHY_DROP_TABLE);
  }
}

int64_t ObDropTable::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "DropTables(if_exists=%c tables=[", if_exists_?'Y':'N');
  pos += tables_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, "])\n");
  return pos;
}

int ObDropTable::add_all_indexs(const ObString &idxname)
{
  return all_indexs_.add_string(idxname);
}

bool ObDropTable::is_all_indexs_empty() const
{
  int64_t count;
  count = all_indexs_.count();
  if(count <= 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
