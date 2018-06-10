/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_memtable_lock.cpp
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */
#include "ob_memtable_lock.h"
#include "ob_ups_table_mgr.h"
namespace oceanbase
{
  namespace updateserver
  {
    MemTableLock::MemTableLock(RWSessionCtx &session, ObIUpsTableMgr &host):
      session_(session),host_(host)
    {

    }
    MemTableLock::~MemTableLock()
    {

    }
    int MemTableLock::open()
    {
      int ret = OB_SUCCESS;
      SessionTableLockInfo* stblk_info = NULL;
      int64_t table_id = 0;

      //TBSYS_LOG(INFO, "zcd::test get_lock_table_name() => %.*s table_id=%ld", get_lock_table_name().length(), get_lock_table_name().ptr(), get_lock_table_id());
      if (0 == (table_id = get_lock_table_id()) || table_id <= 3000 || table_id > 5048)
      {
        TBSYS_LOG(ERROR, "Invalid table_id=%ld", table_id);
      }
      else if(NULL ==(stblk_info = session_.get_tblock_info()))
      {
        TBSYS_LOG(ERROR, "SessionTableLockInfo is NULL. table_id=%ld", table_id);
      }
      else
      {

        //SessionTableLockInfo* stblk_info = session_.get_tblock_info();
        uint32_t uid = session_.get_session_descriptor();
        TableLockMgr& global_tblk_mgr = host_.get_table_lock_mgr();

        if (OB_SUCCESS != (ret = stblk_info->lock_table(global_tblk_mgr, uid, table_id, X_LOCK)))
        {
          TBSYS_LOG(WARN, "Lock table failed table_id=%ld err=%d", table_id, ret);
        }
        //session_.add_callback_info(session_, stblk_info, (void*)&global_tblk_mgr);
      }
      return ret;
    }
    int MemTableLock::close()
    {
      return OB_SUCCESS;
    }



  }
}
