/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_mgr.cpp
 * @brief
 *     modify by guojinwei, bingo: support REPEATABLE-READ isolation
 *     add RRLockInfo functions implementation and complete Lock assign
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         bingo <bingxiao@stu.ecnu.edu.cn>
 * @date 2016_06_16
 */

////===================================================================
 //
 // ob_lock_mgr.cpp updateserver / Oceanbase
 //
 // Copyright (C) 2010 Taobao.com, Inc.
 //
 // Created on 2012-08-30 by Yubai (yubai.lk@taobao.com) 
 //
 // -------------------------------------------------------------------
 //
 // Description
 //
 //
 // -------------------------------------------------------------------
 // 
 // Change Log
 //
////====================================================================

#include "common/ob_common_stat.h"
#include "ob_lock_mgr.h"
#include "ob_sessionctx_factory.h"

namespace oceanbase
{
  namespace updateserver
  {
    int IRowUnlocker::cb_func(const bool rollback, void *data, BaseSessionCtx &session)
    {
      UNUSED(rollback);
      // ��Ҫ Ҫ�ڽ����ʱ��te_value��cur_uc_info��գ����ⱻ��һ��session�ظ�ʹ�����������˽�е�cur_uc_info
      // ������߳��ύ�󣬽������ύ�̷߳���
      // ������߳̽������ύ�߳��ύ�ͷ���
      TEValue *te_value = (TEValue*)data;
      if (NULL != te_value)
      {
        te_value->cur_uc_info = NULL;
      }
      return unlock(te_value, session);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    RowExclusiveUnlocker::RowExclusiveUnlocker()
    {
    }

    RowExclusiveUnlocker::~RowExclusiveUnlocker()
    {
    }

    int RowExclusiveUnlocker::unlock(TEValue *value, BaseSessionCtx &session)
    {
      int ret = OB_SUCCESS;
      if (NULL == value)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if (OB_SUCCESS != (ret = value->row_lock.exclusive_unlock(session.get_session_descriptor())))
      {
        TBSYS_LOG(ERROR, "exclusive unlock row fail sd=%u %s value=%p", session.get_session_descriptor(), value->log_str(), value);
      }
      else
      {
        TBSYS_LOG(DEBUG, "exclusive unlock row succ sd=%u %s value=%p", session.get_session_descriptor(), value->log_str(), value);
      }
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    RPLockInfo::RPLockInfo(RPSessionCtx &session_ctx) : ILockInfo(READ_COMMITED),
                                                        session_ctx_(session_ctx),
                                                        row_exclusive_unlocker_(),
                                                        callback_mgr_()
    {
    }

    RPLockInfo::~RPLockInfo()
    {
    }

    int RPLockInfo::on_trans_begin()
    {
      return OB_SUCCESS;
    }

    int RPLockInfo::on_read_begin(const TEKey &key, TEValue &value)
    {
      UNUSED(key);
      UNUSED(value);
      return OB_SUCCESS;
    }

    int RPLockInfo::on_write_begin(const TEKey &key, TEValue &value)
    {
      int ret = OB_SUCCESS;
      uint32_t sd = session_ctx_.get_session_descriptor();
      if (!value.row_lock.is_exclusive_locked_by(sd))
      {
        int64_t session_end_time = session_ctx_.get_session_start_time() + session_ctx_.get_session_timeout();
        session_end_time = (0 <= session_end_time) ? session_end_time : INT64_MAX;
        int64_t stmt_end_time = session_ctx_.get_stmt_start_time() + session_ctx_.get_stmt_timeout();
        stmt_end_time = (0 <= stmt_end_time) ? stmt_end_time : INT64_MAX;
        int64_t end_time = std::min(session_end_time, stmt_end_time);
        if (OB_SUCCESS == (ret = value.row_lock.exclusive_lock(sd, end_time, session_ctx_.is_alive())))
        {
          if (OB_SUCCESS != (ret = callback_mgr_.add_callback_info(session_ctx_, &row_exclusive_unlocker_, &value)))
          {
            row_exclusive_unlocker_.unlock(&value, session_ctx_);
          }
          else
          {
            TBSYS_LOG(DEBUG, "exclusive lock row succ sd=%u %s %s value=%p",
                            session_ctx_.get_session_descriptor(), key.log_str(), value.log_str(), &value);
          }
        }
        else
        {
          ret = OB_ERR_EXCLUSIVE_LOCK_CONFLICT;
        }
      }
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(USER_ERROR, "Exclusive lock conflict \'%s\' for key \'PRIMARY\'", to_cstring(key.row_key));
        TBSYS_LOG(INFO, "Exclusive lock conflict \'%s\' for key \'PRIMARY\' table_id=%lu request=%u owner=%u",
                  to_cstring(key.row_key), key.table_id, sd, (uint32_t)(value.row_lock.uid_ & QLock::UID_MASK));
      }
      return ret;
    }

    void RPLockInfo::on_trans_end()
    {
      // do nothing
    }

    void RPLockInfo::on_precommit_end()
    {
      bool rollback = false; // do not care
      callback_mgr_.callback(rollback, session_ctx_);
    }

    int RPLockInfo::cb_func(const bool rollback, void *data, BaseSessionCtx &session)
    {
      UNUSED(data);
      return callback_mgr_.callback(rollback, session);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    RCLockInfo::RCLockInfo(RWSessionCtx &session_ctx) : ILockInfo(READ_COMMITED),
                                                        session_ctx_(session_ctx),
                                                        row_exclusive_unlocker_(),
                                                        callback_mgr_()
    {
    }

    RCLockInfo::~RCLockInfo()
    {
    }

    int RCLockInfo::on_trans_begin()
    {
      return OB_SUCCESS;
    }

    int RCLockInfo::on_read_begin(const TEKey &key, TEValue &value)
    {
      UNUSED(key);
      UNUSED(value);
      return OB_SUCCESS;
    }

    int RCLockInfo::on_write_begin(const TEKey &key, TEValue &value)
    {
      int ret = OB_SUCCESS;
      uint32_t sd = session_ctx_.get_session_descriptor();
      if (!value.row_lock.is_exclusive_locked_by(sd))
      {
        int64_t session_end_time = session_ctx_.get_session_start_time() + session_ctx_.get_session_timeout();
        session_end_time = (0 <= session_end_time) ? session_end_time : INT64_MAX;
        int64_t stmt_end_time = session_ctx_.get_stmt_start_time() + session_ctx_.get_stmt_timeout();
        stmt_end_time = (0 <= stmt_end_time) ? stmt_end_time : INT64_MAX;
        int64_t end_time = std::min(session_end_time, stmt_end_time);
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        end_time = std::min(end_time, cur_time + LOCK_WAIT_TIME);
        if (OB_SUCCESS == (ret = value.row_lock.exclusive_lock(sd, end_time, session_ctx_.is_alive())))
        {
          cur_time = tbsys::CTimeUtil::getTime() - cur_time;
          if (OB_SUCCESS != (ret = callback_mgr_.add_callback_info(session_ctx_, &row_exclusive_unlocker_, &value)))
          {
            row_exclusive_unlocker_.unlock(&value, session_ctx_);
          }
          else
          {
            TBSYS_LOG(DEBUG, "exclusive lock row succ sd=%u %s %s value=%p",
                            session_ctx_.get_session_descriptor(), key.log_str(), value.log_str(), &value);
          }
        }
        else
        {
          cur_time = tbsys::CTimeUtil::getTime() - cur_time;
          session_ctx_.set_conflict_processor_index(session_ctx_.get_host().get_processor_index(value.row_lock.get_uid()));
          ret = OB_ERR_EXCLUSIVE_LOCK_CONFLICT;
        }
        OB_STAT_INC(UPDATESERVER, UPS_STAT_LOCK_WAIT_TIME, cur_time);
      }
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(USER_ERROR, "Exclusive lock conflict \'%s\' for key \'PRIMARY\'",
                  to_cstring(key.row_key));
        TBSYS_LOG(INFO, "Exclusive lock conflict \'%s\' for key \'PRIMARY\' table_id=%lu request=%u owner=%u",
                  to_cstring(key.row_key), key.table_id, sd, (uint32_t)(value.row_lock.uid_ & QLock::UID_MASK));
      }
      return ret;
    }

    void RCLockInfo::on_trans_end()
    {
      // do nothing
    }

    void RCLockInfo::on_precommit_end()
    {
      // do nothing
    }

    int RCLockInfo::cb_func(const bool rollback, void *data, BaseSessionCtx &session)
    {
      UNUSED(data);
      return callback_mgr_.callback(rollback, session);
    }

    // add by guojinwei [repeatable read] 20160307:b
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    RRLockInfo::RRLockInfo(RWSessionCtx &session_ctx) : ILockInfo(REPEATABLE_READ),
                                                        session_ctx_(session_ctx),
                                                        row_exclusive_unlocker_(),
                                                        callback_mgr_()
    {
    }

    RRLockInfo::~RRLockInfo()
    {
    }

    int RRLockInfo::on_trans_begin()
    {
      return OB_SUCCESS;
    }

    int RRLockInfo::on_read_begin(const TEKey &key, TEValue &value)
    {
      UNUSED(key);
      UNUSED(value);
      return OB_SUCCESS;
    }

    int RRLockInfo::on_write_begin(const TEKey &key, TEValue &value)
    {
      int ret = OB_SUCCESS;
      uint32_t sd = session_ctx_.get_session_descriptor();
      if (!value.row_lock.is_exclusive_locked_by(sd))
      {
        int64_t session_end_time = session_ctx_.get_session_start_time() + session_ctx_.get_session_timeout();
        session_end_time = (0 <= session_end_time) ? session_end_time : INT64_MAX;
        int64_t stmt_end_time = session_ctx_.get_stmt_start_time() + session_ctx_.get_stmt_timeout();
        stmt_end_time = (0 <= stmt_end_time) ? stmt_end_time : INT64_MAX;
        int64_t end_time = std::min(session_end_time, stmt_end_time);
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        end_time = std::min(end_time, cur_time + LOCK_WAIT_TIME);
        if (OB_SUCCESS == (ret = value.row_lock.exclusive_lock(sd, end_time, session_ctx_.is_alive())))
        {
          cur_time = tbsys::CTimeUtil::getTime() - cur_time;
          if (OB_SUCCESS != (ret = callback_mgr_.add_callback_info(session_ctx_, &row_exclusive_unlocker_, &value)))
          {
            row_exclusive_unlocker_.unlock(&value, session_ctx_);
          }
          else
          {
            TBSYS_LOG(DEBUG, "exclusive lock row succ sd=%u %s %s value=%p",
                            session_ctx_.get_session_descriptor(), key.log_str(), value.log_str(), &value);
          }
        }
        else
        {
          cur_time = tbsys::CTimeUtil::getTime() - cur_time;
          session_ctx_.set_conflict_processor_index(session_ctx_.get_host().get_processor_index(value.row_lock.get_uid()));
          ret = OB_ERR_EXCLUSIVE_LOCK_CONFLICT;
        }
        OB_STAT_INC(UPDATESERVER, UPS_STAT_LOCK_WAIT_TIME, cur_time);
      }
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(USER_ERROR, "Exclusive lock conflict \'%s\' for key \'PRIMARY\'",
                  to_cstring(key.row_key));
        TBSYS_LOG(INFO, "Exclusive lock conflict \'%s\' for key \'PRIMARY\' table_id=%lu request=%u owner=%u",
                  to_cstring(key.row_key), key.table_id, sd, (uint32_t)(value.row_lock.uid_ & QLock::UID_MASK));
      }
      return ret;
    }

    void RRLockInfo::on_trans_end()
    {
      // do nothing
    }

    void RRLockInfo::on_precommit_end()
    {
      // do nothing
    }

    int RRLockInfo::cb_func(const bool rollback, void *data, BaseSessionCtx &session)
    {
      UNUSED(data);
      return callback_mgr_.callback(rollback, session);
    }

    // add:e

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    LockMgr::LockMgr()
    {
    }

    LockMgr::~LockMgr()
    {
    }

    ILockInfo *LockMgr::assign(const IsolationLevel level, BaseSessionCtx &session_ctx)
    {
      int tmp_ret = OB_SUCCESS;
      void *buffer = NULL;
      ILockInfo *ret = NULL;
      RPSessionCtx* rpsession_ctx = NULL;
      RWSessionCtx* rwsession_ctx = NULL;
      switch (level)
      {
      case NO_LOCK:
        if (NULL == (rpsession_ctx = dynamic_cast<RPSessionCtx*>(&session_ctx)))
        {
          TBSYS_LOG(ERROR, "can not cast session_ctx to RPSessionCtx, ctx=%s", to_cstring(*rpsession_ctx));
        }
        else if (NULL == (buffer = rpsession_ctx->alloc(sizeof(RPLockInfo))))
        {
          TBSYS_LOG(ERROR, "alloc rplock info fail, ctx=%s", to_cstring(*rpsession_ctx));
        }
        else if (OB_SUCCESS != (tmp_ret = rpsession_ctx->add_callback_info(*rpsession_ctx, ret = new(buffer) RPLockInfo(*rpsession_ctx), NULL)))
        {
          TBSYS_LOG(ERROR, "add_callback_info()=>%d, ctx=%s", tmp_ret, to_cstring(*rpsession_ctx));
          ret = NULL;
        }
        break;
      case READ_COMMITED:
        if (NULL == (rwsession_ctx = dynamic_cast<RWSessionCtx*>(&session_ctx)))
        {
          TBSYS_LOG(ERROR, "can not cast session_ctx to RWSessionCtx, ctx=%s", to_cstring(*rwsession_ctx));
        }
        else if (NULL == (buffer = rwsession_ctx->alloc(sizeof(RCLockInfo))))
        {
          TBSYS_LOG(ERROR, "alloc rclock info fail, ctx=%s", to_cstring(*rwsession_ctx));
        }
        else if (OB_SUCCESS != (tmp_ret = rwsession_ctx->add_callback_info(*rwsession_ctx, ret = new(buffer) RCLockInfo(*rwsession_ctx), NULL)))
        {
          TBSYS_LOG(ERROR, "add_callback_info()=>%d, ctx=%s", tmp_ret, to_cstring(*rwsession_ctx));
          ret = NULL;
        }
        break;
      // add by guojinwei [repeatable read] 20160307:b
      case REPEATABLE_READ:
        if (NULL == (rwsession_ctx = dynamic_cast<RWSessionCtx*>(&session_ctx)))
        {
          TBSYS_LOG(ERROR, "can not cast session_ctx to RWSessionCtx, ctx=%s", to_cstring(*rwsession_ctx));
        }
        else if (NULL == (buffer = rwsession_ctx->alloc(sizeof(RRLockInfo))))
        {
          TBSYS_LOG(ERROR, "alloc rrlock info fail, ctx=%s", to_cstring(*rwsession_ctx));
        }
        else if (OB_SUCCESS != (tmp_ret = rwsession_ctx->add_callback_info(*rwsession_ctx, ret = new(buffer) RRLockInfo(*rwsession_ctx), NULL)))
        {
          TBSYS_LOG(ERROR, "add_callback_info()=>%d, ctx=%s", tmp_ret, to_cstring(*rwsession_ctx));
          ret = NULL;
        }
        break;
      // add:e
      default:
        TBSYS_LOG(WARN, "isolation level=%d not support", level);
        break;
      }
      return ret;
    }
  }
}

