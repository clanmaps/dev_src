/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_memtable_lock.h
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */
#ifndef MEMTABLELOCK_H
#define MEMTABLELOCK_H

#include "sql/ob_ups_lock_table.h"
#include "ob_sessionctx_factory.h"

namespace oceanbase
{
  namespace updateserver
  {
    class MemTableLock : public sql::ObUpsLockTable
    {
      public:
        MemTableLock(RWSessionCtx &session, ObIUpsTableMgr &host);
        ~MemTableLock();
      public:
        int open();
        int close();
        //int64_t to_string(char* buf, const int64_t buf_len) const;
      private:
        RWSessionCtx &session_;
        ObIUpsTableMgr &host_;
    };
  }
}

#endif // MEMTABLELOCK_H
