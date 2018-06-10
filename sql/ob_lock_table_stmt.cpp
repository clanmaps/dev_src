/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_lock_table_stmt.cpp
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */

#include "common/ob_define.h"
#include "ob_lock_table_stmt.h"
#include <stdio.h>
#include <stdlib.h>

namespace oceanbase
{
  namespace sql
  {
    ObLockTableStmt::ObLockTableStmt(common::ObStringBuf* name_pool)
    : ObStmt(name_pool, T_LOCK_TABLE)
    {
    }
    ObLockTableStmt::~ObLockTableStmt()
    {
    }
    void ObLockTableStmt::set_lock_table_id(uint64_t table_id)
    {
      lock_table_id_ = table_id;
    }

    uint64_t ObLockTableStmt::get_lock_table_id() const
    {
      return lock_table_id_;
    }

  }
}

