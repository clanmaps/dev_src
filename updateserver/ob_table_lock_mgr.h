/**
* (C) 2007-2010 Taobao Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* Version: $Id$
*
* Authors:
*   wangjiahao <wjh2006-1@163.com>
*     - some work details if you want
*/
#ifndef OB_TABLE_LOCK_MGR_H
#define OB_TABLE_LOCK_MGR_H
#include "common/ob_define.h"
#include "ob_table_lock.h"
#include "ob_session_mgr.h"

namespace oceanbase
{
  namespace updateserver
  {
      //using namespace oceanbase::common;
      static const int64_t TABLE_ID_NUMBER_SHIFT  = 3000;
      enum TablelockStat{
        N_LOCK,
        IX_LOCK,
        X_LOCK
      };
      struct TableLockRecord{
        TablelockStat stat_;
        uint64_t table_id_;
      };

      //typedef hash::ObHashMap<uint64_t, TbLock*> TableLock;
      class TableLockMgr{
        //static const int64_t TABLE_ID_NUMBER_SHIFT  = 3000;
        public:
          TableLockMgr():inited_(false){}
          ~TableLockMgr(){}
          int init();
          int up_lock(uint32_t uid, uint64_t table_id, TablelockStat old_stat, TablelockStat new_stat, const int64_t end_time, const volatile bool &wait_flag);
          int down_lock(uint32_t uid, uint64_t table_id, TablelockStat old_stat, TablelockStat new_stat);

        private:
          bool inited_;
          TbLock* tblock_map_[OB_MAX_TABLE_NUMBER + TABLE_ID_NUMBER_SHIFT];

      };

      class RWSessionCtx;
      class SessionTableLockInfo : public ISessionCallback{
        public:
          SessionTableLockInfo(RWSessionCtx* session_ctx)
                :session_ctx_(session_ctx), locked_table_sum_(0)
          {
            //memset(tblock_map_, 0, sizeof(tblock_map_));
          }
          ~SessionTableLockInfo(){}
          int lock_table(TableLockMgr &globle_tblock, uint32_t uid, uint64_t table_id, TablelockStat new_stat);
          int unlock_table(TableLockMgr &globle_tblock, uint32_t uid);
          int cb_func(const bool rollback, void *data, BaseSessionCtx &session);

          int find_table_idx(uint64_t table_id);


        private:
          RWSessionCtx* session_ctx_;
          //TableLockMgr * global_tblock_;
          TablelockStat* tblock_map_[OB_MAX_TABLE_NUMBER];
          uint64_t locked_table_record_[OB_MAX_TABLE_NUMBER];
          uint32_t locked_table_sum_;

          //need a list to store tblock_map_ not null position

      };
  }
}
#endif // OB_TABLE_LOCK_MGR_H
