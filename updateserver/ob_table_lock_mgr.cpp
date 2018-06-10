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
#include "ob_table_lock_mgr.h"
#include "ob_sessionctx_factory.h"
using namespace oceanbase::common;
namespace oceanbase
{
  namespace updateserver
  {
    int TableLockMgr::init()
    {
      int ret = OB_SUCCESS;
      if (true == inited_)
      {
        ret = OB_INIT_TWICE;
      }
      else{
        for (int64_t i = TABLE_ID_NUMBER_SHIFT + 1; i <= TABLE_ID_NUMBER_SHIFT + OB_MAX_TABLE_NUMBER; i++)
        {
          TbLock* tblk = (TbLock*)ob_malloc(sizeof(TbLock), ObModIds::OB_UPS_COMMON);

          if (NULL == tblk)
          {
            TBSYS_LOG(ERROR, "No memory to alloc Global Table Lock.");
            ret = OB_ERROR;
          }
          else
          {
            tblock_map_[i] = new(tblk)TbLock();
          }
        }
      }
      return ret;
    }
    int TableLockMgr::up_lock(const uint32_t uid, uint64_t table_id, TablelockStat old_stat, TablelockStat new_stat, const int64_t end_time, const bool volatile &wait_flag)
    {
      int ret = OB_SUCCESS;
      TbLock* tblk = NULL;
      if (OB_UNLIKELY(0 == uid || table_id <= (uint64_t)TABLE_ID_NUMBER_SHIFT || table_id > (uint64_t)(OB_MAX_TABLE_NUMBER + TABLE_ID_NUMBER_SHIFT)))
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        tblk = tblock_map_[table_id];
      }
      if (OB_UNLIKELY(OB_SUCCESS != ret || new_stat < old_stat))
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if (N_LOCK == old_stat && IX_LOCK == new_stat && OB_SUCCESS != (ret = tblk->intention_lock(uid, end_time, wait_flag)))
      {
        if (OB_EAGAIN == ret)
        {
          ret = OB_ERR_TABLE_INTENTION_LOCK_CONFLICT;
          TBSYS_LOG(WARN, "intention lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
        else
        {
          TBSYS_LOG(ERROR, "intention lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
      }
      else if (N_LOCK == old_stat && X_LOCK == new_stat && OB_SUCCESS != (ret = tblk->exclusive_lock(uid, end_time, wait_flag)))
      {
        if (OB_EAGAIN == ret)
        {
          ret = OB_ERR_TABLE_EXCLUSIVE_LOCK_CONFLICT;
          TBSYS_LOG(WARN, "exclusive lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
        else
        {
          TBSYS_LOG(ERROR, "exclusive lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
      }
      else if (IX_LOCK == old_stat && X_LOCK == new_stat && OB_SUCCESS != (ret = tblk->intention2exclusive_lock(uid, end_time, wait_flag)))
      {
        if (OB_EAGAIN == ret)
        {
          ret = OB_ERR_TABLE_EXCLUSIVE_LOCK_CONFLICT;
          TBSYS_LOG(WARN, "intention2exclusive lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
        else
        {
          TBSYS_LOG(ERROR, "intention2exclusive lock table failed. uid=%u tid=%lu err=%d", uid, table_id, ret);
        }
      }
      else
      {
        //succ
      }
      return ret;
    }

    int TableLockMgr::down_lock(uint32_t uid, uint64_t table_id, TablelockStat old_stat, TablelockStat new_stat)
    {
      int ret = OB_SUCCESS;
      TbLock* tblk = NULL;
      if (OB_UNLIKELY(0 == uid || table_id <= (uint64_t)TABLE_ID_NUMBER_SHIFT || table_id > (uint64_t)(OB_MAX_TABLE_NUMBER + TABLE_ID_NUMBER_SHIFT)))
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        tblk = tblock_map_[table_id];
      }
      if (OB_UNLIKELY(OB_SUCCESS != ret))
      {}
      else if (old_stat < new_stat) //already have high level lock
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if (IX_LOCK == old_stat && N_LOCK == new_stat && OB_SUCCESS != (ret = tblk->intention_unlock(uid)))
      {
        TBSYS_LOG(WARN, "intention unlock table failed. uid=%u tid=%lu ", uid, table_id);
      }
      else if (X_LOCK == old_stat && N_LOCK == new_stat && OB_SUCCESS != (ret = tblk->exclusive_unlock(uid)))
      {
        TBSYS_LOG(WARN, "exclusive unlock table failed. uid=%u tid=%lu ", uid, table_id);
      }
      else if (X_LOCK == old_stat && IX_LOCK == new_stat && OB_SUCCESS != (ret = tblk->exclusive2intention_lock(uid)))
      {
        TBSYS_LOG(WARN, "exclusive2intention lock table failed. uid=%u tid=%lu ", uid, table_id);
      }
      else
      {
        //succ
      }
      return ret;
    }
    int SessionTableLockInfo::lock_table(TableLockMgr& global_tblock_mgr, uint32_t uid, uint64_t table_id, TablelockStat new_stat)
    {
      int ret = OB_SUCCESS;
      //bool new_table_id = false;
      TablelockStat* cur_stat = NULL;
      int idx;
      if (OB_UNLIKELY(0 == uid || table_id <= (uint64_t)TABLE_ID_NUMBER_SHIFT || table_id > (uint64_t)(OB_MAX_TABLE_NUMBER + TABLE_ID_NUMBER_SHIFT)))
      {
        ret = OB_INVALID_ARGUMENT;
      }
      if (OB_INVALID_INDEX == (idx = find_table_idx(table_id)))
      {
        locked_table_record_[locked_table_sum_] = table_id;
        if (NULL == (cur_stat = (TablelockStat*)session_ctx_->alloc(sizeof(TablelockStat)))) {
            TBSYS_LOG(ERROR, "alloc TablelockStat info fail, ctx=%s", to_cstring(*session_ctx_));
            ret = OB_MEM_OVERFLOW;
        } else {
            tblock_map_[locked_table_sum_] = cur_stat;
            *cur_stat = N_LOCK;
            locked_table_sum_++;
        }
      }
      else if (NULL == (cur_stat = tblock_map_[idx]))
      {
        TBSYS_LOG(ERROR, "tblock_map_[idx] is NULL. table_id=%lu idx=%d", table_id, idx);
        ret = OB_ERROR;
      }
      //TBSYS_LOG(INFO, "##TEST_PRINT## before session lock_table uid=%u table_id=%lu cur_stat=%d new_stat=%d", uid, table_id, *cur_stat, new_stat);
      if (OB_UNLIKELY(OB_SUCCESS != ret))
      {}
      else if (*cur_stat >= new_stat) //already have high level lock
      {}
      else
      {
        int64_t session_end_time = session_ctx_->get_session_start_time() + session_ctx_->get_session_timeout();
        session_end_time = (0 <= session_end_time) ? session_end_time : INT64_MAX;
        int64_t stmt_end_time = session_ctx_->get_stmt_start_time() + session_ctx_->get_stmt_timeout();
        stmt_end_time = (0 <= stmt_end_time) ? stmt_end_time : INT64_MAX;
        int64_t end_time = std::min(session_end_time, stmt_end_time);
        if (OB_SUCCESS != (ret = global_tblock_mgr.up_lock(uid, table_id, *cur_stat, new_stat, end_time, session_ctx_->is_alive())))
        {
          TBSYS_LOG(WARN, "up_lock failed. table_id=%lu cur_stat=%d new_stat=%d err=%d", table_id, *cur_stat, new_stat, ret);
          //if (new_table_id)
            //locked_table_sum_--;
        }
        else
        {
          *cur_stat = new_stat;
        }
      }
      return ret;
    }

    int SessionTableLockInfo::unlock_table(TableLockMgr& globle_tblock, uint32_t uid)
    {
      int ret = OB_SUCCESS;
      for (uint32_t i = 0; i < locked_table_sum_ && OB_SUCCESS == ret; i++)
      {
        uint64_t table_id = locked_table_record_[i];
        TablelockStat* cur_stat = tblock_map_[i];
        if (NULL == cur_stat)
        {
          TBSYS_LOG(ERROR, "cur_stat is NULL");
          ret = OB_ERROR;
        }
        else if (*cur_stat == N_LOCK)
        {
          //if lock failed because of the lock contention case, this table_record stat will be N_LOCK.
        }
        else if (OB_SUCCESS != (ret = globle_tblock.down_lock(uid, table_id, *cur_stat, N_LOCK)))
        {
           TBSYS_LOG(ERROR, "down_lock failed. table_id=%lu cur_stat=%d err=%d", table_id, *cur_stat, ret);
        }
      }
      return ret;
    }

    int SessionTableLockInfo::cb_func(const bool rollback, void *data, BaseSessionCtx &session)
    {
      UNUSED(rollback);
      int ret = OB_SUCCESS;
      TableLockMgr* global_tblk_mgr = (TableLockMgr*)data;
      if (NULL == global_tblk_mgr)
      {
        TBSYS_LOG(ERROR, "global_tblk_mgr is NULL");
        ret = OB_ERROR;
      }
      else if (OB_SUCCESS != (ret = unlock_table(*global_tblk_mgr, session.get_session_descriptor())))
      {
        TBSYS_LOG(ERROR, "unlock_table fail. session=%u err=%d", session.get_session_descriptor(), ret);
      }
      return ret;
    }

    int SessionTableLockInfo::find_table_idx(uint64_t table_id)
    {
      int ret = OB_INVALID_INDEX;
      for (uint32_t i = 0; i < locked_table_sum_; i++)
      {
        if (locked_table_record_[i] == table_id)
        {
          ret = (int)i;
          break;
        }
      }
      return ret;
    }


  }
}
