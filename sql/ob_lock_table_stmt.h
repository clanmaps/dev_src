/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_lock_table_stmt.h
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */
#ifndef OB_LOCK_TABLE_STMT_H
#define OB_LOCK_TABLE_STMT_H
#include "ob_stmt.h"

namespace oceanbase
{
  namespace sql
  {
    class ObLockTableStmt : public ObStmt
    {
      public:
        ObLockTableStmt(common::ObStringBuf* name_pool);
        virtual ~ObLockTableStmt();

        void set_lock_table_id(uint64_t table_id);
        uint64_t get_lock_table_id() const;
      private:

        uint64_t lock_table_id_;
    };

  }
}
#endif // OB_LOCK_TABLE_STMT_H
