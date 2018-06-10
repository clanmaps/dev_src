/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_sessionctx_factory.cpp
 * @brief modify by zhouhuan: support scalable commit by adding
 *        or modifying some functions, member variables
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_03_14
 */
////===================================================================
 //
 // ob_sessionctx_factory.cpp updateserver / Oceanbase
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

#include "common/ob_mod_define.h"
#include "ob_sessionctx_factory.h"
#include "ob_update_server_main.h"

#define UPS ObUpdateServerMain::get_instance()->get_update_server()
namespace oceanbase
{
  namespace updateserver
  {
    RWSessionCtx::v4si RWSessionCtx::v4si_zero = {0,0,0,0};

    RWSessionCtx::RWSessionCtx(const SessionType type,
                               SessionMgr &host,
                               FIFOAllocator &fifo_allocator,
                               const bool need_gen_mutator) : BaseSessionCtx(type, host),
                                                               CallbackMgr(),
                                                               mod_(fifo_allocator),
                                                               page_arena_(ALLOCATOR_PAGE_SIZE, mod_),
                                                               stmt_page_arena_(ALLOCATOR_PAGE_SIZE, mod_),
                                                               stmt_page_arena_wrapper_(stmt_page_arena_),
                                                               stat_(ST_ALIVE),
                                                               alive_flag_(true),
                                                               commit_done_(false),
                                                               need_gen_mutator_(need_gen_mutator),
                                                               ups_mutator_(page_arena_),
                                                               ups_result_(stmt_page_arena_wrapper_),
                                                               uc_info_(),
                                                               lock_info_(NULL),
                                                               publish_callback_list_(),
                                                               free_callback_list_(),
                                                               group_id_(-1) //add by zhouhuan [scalablecommit] 20160426
    {
    }

    RWSessionCtx::~RWSessionCtx()
    {
    }

    int RWSessionCtx::precommit()
    {
      int ret = OB_SUCCESS;
      int64_t begin_time = tbsys::CTimeUtil::getTime();
      end(false);
      mark_done(BaseSessionCtx::ES_CALLBACK);
      set_frozen();
      OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_LTIME, tbsys::CTimeUtil::getTime() - begin_time);
      return ret;
    }
    void RWSessionCtx::end(const bool need_rollback)
    {
      if (!commit_done_)
      {
        commit_prepare_list();
        commit_prepare_checksum();
        callback(need_rollback, *this);
        if (NULL != lock_info_)
        {
          lock_info_->on_trans_end();
        }
        int64_t *d = (int64_t*)&dml_count_;
        OB_STAT_INC(UPDATESERVER, UPS_STAT_DML_REPLACE_COUNT, d[OB_DML_REPLACE - 1]);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_DML_INSERT_COUNT,  d[OB_DML_INSERT  - 1]);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_DML_UPDATE_COUNT,  d[OB_DML_UPDATE  - 1]);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_DML_DELETE_COUNT,  d[OB_DML_DELETE  - 1]);
        commit_done_ = true;
      }
    }

    void RWSessionCtx::publish()
    {
      bool rollback = false;
      publish_callback_list_.callback(rollback, *this);
    }

    int RWSessionCtx::on_free()
    {
      bool rollback = false;
      // modify by qx 20170225 :b
      // free_callback_list_.callback(rollback, *this);
      int ret = free_callback_list_.callback(rollback, *this);
      return ret;
      //modify :e
    }

    int RWSessionCtx::add_publish_callback(ISessionCallback *callback, void *data)
    {
      return publish_callback_list_.add_callback_info(*this, callback, data);
    }

    int RWSessionCtx::add_free_callback(ISessionCallback *callback, void *data)
    {
      return free_callback_list_.add_callback_info(*this, callback, data);
    }

    void *RWSessionCtx::alloc(const int64_t size)
    {
      TBSYS_LOG(DEBUG, "session alloc %p size=%ld", this, size);
      return page_arena_.alloc(size);
    }

    void RWSessionCtx::reset()
    {
      ups_result_.clear();
      ups_mutator_.clear();
      stat_ = ST_ALIVE;
      alive_flag_ = true;
      commit_done_ = false;
      stmt_page_arena_.free();
      page_arena_.free();
      CallbackMgr::reset();
      BaseSessionCtx::reset();
      uc_info_.reset();
      lock_info_ = NULL;
      publish_callback_list_.reset();
      free_callback_list_.reset();
      checksum_callback_.reset();
      checksum_callback_list_.reset();
      dml_count_ = v4si_zero;
      group_id_ = -1; //add by zhouhuan
    }

    ObUpsMutator &RWSessionCtx::get_ups_mutator()
    {
      return ups_mutator_;
    }

    TransUCInfo &RWSessionCtx::get_uc_info()
    {
      return uc_info_;
    }

    TEValueUCInfo *RWSessionCtx::alloc_tevalue_uci()
    {
      TEValueUCInfo *ret = (TEValueUCInfo*)alloc(sizeof(TEValueUCInfo));
      if (NULL != ret)
      {
        ret->reset();
      }
      return ret;
    }

    int RWSessionCtx::init_lock_info(LockMgr& lock_mgr, const IsolationLevel isolation)
    {
      int ret = OB_SUCCESS;
      if (NULL == (lock_info_ = lock_mgr.assign(isolation, *this)))
      {
        TBSYS_LOG(WARN, "assign lock_info fail");
        ret = OB_MEM_OVERFLOW;
      }
      else if (OB_SUCCESS != (ret = lock_info_->on_trans_begin()))
      {
        TBSYS_LOG(WARN, "invoke on_trans_begin fail ret=%d", ret);
      }
      return ret;
    }

    //add wangjiahao [tablelock] 20160616 :b
    int RWSessionCtx::init_table_lock_info()
    {
      int ret = OB_SUCCESS;
      void *buffer = NULL;
      if (NULL == (buffer = alloc(sizeof(SessionTableLockInfo))))
      {
        TBSYS_LOG(ERROR, "alloc rclock info fail, ctx=%s", to_cstring(*this));
      }
      else if (NULL == (table_lock_info_ = new(buffer) SessionTableLockInfo(this)))
      {
        TBSYS_LOG(ERROR, "new alloc rclock info fail, ctx=%s", to_cstring(*this));
      }
      return ret;
    }
    //add :e

    ILockInfo *RWSessionCtx::get_lock_info()
    {
      return lock_info_;
    }

    int64_t RWSessionCtx::get_min_flying_trans_id()
    {
      return get_host().get_min_flying_trans_id();
    }

    void RWSessionCtx::flush_min_flying_trans_id()
    {
      get_host().flush_min_flying_trans_id();
    }

    sql::ObUpsResult &RWSessionCtx::get_ups_result()
    {
      return ups_result_;
    }

    const bool volatile &RWSessionCtx::is_alive() const
    {
      return alive_flag_;
    }

    bool RWSessionCtx::is_killed() const
    {
      return (ST_KILLING == stat_);
    }

    void RWSessionCtx::kill()
    {
      //mod chujiajia [log synchronization][multi_cluster] 20160923:b
      //if (ST_ALIVE != ATOMIC_CAS(&stat_, ST_ALIVE, ST_KILLING))
      if (ObiRole::MASTER == UPS.get_obi_role().get_role() && ST_ALIVE != ATOMIC_CAS(&stat_, ST_ALIVE, ST_KILLING))
      //mod:e
      {
        TBSYS_LOG(WARN, "session will not be killed sd=%u stat=%d session_start_time=%ld stmt_start_time=%ld session_timeout=%ld stmt_timeout=%ld",
                  get_session_descriptor(), stat_, get_session_start_time(), get_stmt_start_time(), get_session_timeout(), get_stmt_timeout());
      }
      else
      {
        TBSYS_LOG(INFO, "session is being killed sd=%u stat=%d session_start_time=%ld stmt_start_time=%ld session_timeout=%ld stmt_timeout=%ld",
                  get_session_descriptor(), stat_, get_session_start_time(), get_stmt_start_time(), get_session_timeout(), get_stmt_timeout());
        alive_flag_ = false;
      }
    }

    void RWSessionCtx::set_frozen()
    {
      Stat old_stat = stat_;
      stat_ = ST_FROZEN;
      if (ST_KILLING == old_stat)
      {
        TBSYS_LOG(INFO, "session has been set frozen, will not be killed, sd=%u", get_session_descriptor());
      }
    }

    bool RWSessionCtx::is_frozen() const
    {
      return (ST_FROZEN == stat_);
    }

    void RWSessionCtx::reset_stmt()
    {
      ups_result_.clear();
      stmt_page_arena_.reuse();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    SessionCtxFactory::SessionCtxFactory() : mod_(ObModIds::OB_UPS_SESSION_CTX),
                                             allocator_(ALLOCATOR_PAGE_SIZE, mod_),
                                             ctx_allocator_(),
                                             long_trans_ctx_allocator_() //add by qx 20170314
    {
      // modify by qx 20170310 :b

      if (OB_SUCCESS != ctx_allocator_.init(ALLOCATOR_TOTAL_LIMIT, ALLOCATOR_HOLD_LIMIT, ALLOCATOR_PAGE_SIZE))
      //if (OB_SUCCESS != ctx_allocator_.init(OB_ALLOCATOR_TOTAL_LIMIT, OB_ALLOCATOR_HOLD_LIMIT, ALLOCATOR_PAGE_SIZE))
      // modify :e
      {
        TBSYS_LOG(ERROR, "init allocator fail");
      }
      else
      {
        ctx_allocator_.set_mod_id(ObModIds::OB_UPS_SESSION_CTX);
      }
      //add by qx 20170314 :b
      if (OB_SUCCESS != long_trans_ctx_allocator_.init(ALLOCATOR_TOTAL_LIMIT, ALLOCATOR_HOLD_LIMIT, ALLOCATOR_PAGE_SIZE))
      {
        TBSYS_LOG(ERROR, "init long_trans_ctx_allocator_ allocator fail");
      }
      else
      {
        ctx_allocator_.set_mod_id(ObModIds::OB_UPS_SESSION_CTX);
      }
      //add :e
    }

    SessionCtxFactory::~SessionCtxFactory()
    {
    }

    BaseSessionCtx *SessionCtxFactory::alloc(const SessionType type, SessionMgr &host)
    {
      char *buffer = NULL;
      BaseSessionCtx *ret = NULL;
      switch (type)
      {
      case ST_READ_ONLY:
        buffer = allocator_.alloc(sizeof(ROSessionCtx));
        if (NULL != buffer)
        {
         ret = new(buffer) ROSessionCtx(type, host);
        }
        break;
      case ST_REPLAY:
        buffer = allocator_.alloc(sizeof(RPSessionCtx));
        if (NULL != buffer)
        {
          ret = new(buffer) RPSessionCtx(type, host, ctx_allocator_);
        }
        break;
      case ST_READ_WRITE:
        buffer = allocator_.alloc(sizeof(RWSessionCtx));
        if (NULL != buffer)
        {
          ret = new(buffer) RWSessionCtx(type, host, ctx_allocator_);
        }
        break;
      case ST_LONG_READ_WRITE:
        buffer = allocator_.alloc(sizeof(RWSessionCtx));
        if (NULL != buffer)
        {
          ret = new(buffer) RWSessionCtx(type, host, long_trans_ctx_allocator_);
        }
        break;
      default:
        TBSYS_LOG(WARN, "invalid session type=%d", type);
        break;
      }
      return ret;
    }

    void SessionCtxFactory::free(BaseSessionCtx *ptr)
    {
      UNUSED(ptr);
    }
  }
}

