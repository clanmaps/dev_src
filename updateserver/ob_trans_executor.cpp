/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_trans_executor.cpp
 * @brief TransExecutor
 *     modify by guojinwei: support multiple clusters for HA by
 *     adding or modifying some functions, member variables
 *
 * @version CEDAR 0.2
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */
////===================================================================
 //
 // ob_trans_executor.cpp updateserver / Oceanbase
 //
 // Copyright (C) 2010, 2012, 2013 Taobao.com, Inc.
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

#include "common/ob_common_param.h"
#include "common/ob_profile_log.h"
#include "common/ob_profile_type.h"
#include "common/ob_trace_id.h"
#include "common/base_main.h"
#include "sql/ob_lock_filter.h"
#include "sql/ob_inc_scan.h"
#include "sql/ob_ups_modify.h"
#include "ob_ups_lock_filter.h"
#include "ob_ups_inc_scan.h"
#include "ob_memtable_modify.h"
#include "ob_update_server_main.h"
#include "ob_trans_executor.h"
#include "ob_session_guard.h"
#include "stress.h"

#define UPS ObUpdateServerMain::get_instance()->get_update_server()

namespace oceanbase
{
  namespace updateserver
  {
    class FakeWriteGuard
    {
      public:
        FakeWriteGuard(SessionMgr& session_mgr):
          session_mgr_(session_mgr), session_descriptor_(INVALID_SESSION_DESCRIPTOR), fake_write_granted_(false)
        {
          int ret = OB_SUCCESS;
          RWSessionCtx *session_ctx = NULL;
          if (OB_SUCCESS != (ret = session_mgr.begin_session(ST_READ_WRITE, 0, -1, -1, session_descriptor_)))
          {
            TBSYS_LOG(WARN, "begin fake write session fail ret=%d", ret);
          }
          else if (NULL == (session_ctx = session_mgr.fetch_ctx<RWSessionCtx>(session_descriptor_)))
          {
            TBSYS_LOG(WARN, "fetch ctx fail session_descriptor=%u", session_descriptor_);
            ret = OB_TRANS_ROLLBACKED;
          }
          else
          {
            session_ctx->set_frozen();
            session_mgr.revert_ctx(session_descriptor_);
            fake_write_granted_ = true;
          }
        }
        ~FakeWriteGuard(){
          if (INVALID_SESSION_DESCRIPTOR == session_descriptor_)
          {} // do nothing
          else
          {
            session_mgr_.end_session(session_descriptor_, /*rollback*/ true);
          }
        }
        bool is_granted() { return fake_write_granted_; }
      private:
        SessionMgr& session_mgr_;
        uint32_t session_descriptor_;
        bool fake_write_granted_;
    };

    TransExecutor::TransExecutor(ObUtilInterface &ui) : TransHandlePool(),
                                                        TransCommitThread(),
                                                        ui_(ui),
                                                        my_thread_buffer_(static_cast<int32_t>(OB_LOG_BUFFER_MAX_SIZE)),  // enable handle log pkt
                                                        allocator_(),
                                                        session_ctx_factory_(),
                                                        session_mgr_(),
                                                        lock_mgr_(),
                                                        uncommited_session_list_(),
                                                        ups_result_buffer_(ups_result_memory_, OB_MAX_PACKET_LENGTH)

    {
      allocator_.set_mod_id(ObModIds::OB_UPS_TRANS_EXECUTOR_TASK);
      memset(ups_result_memory_, 0, OB_MAX_PACKET_LENGTH);

      memset(packet_handler_, 0, sizeof(packet_handler_));
      memset(trans_handler_, 0, sizeof(trans_handler_));
      memset(commit_handler_, 0, sizeof(commit_handler_));
      for (int64_t i = 0; i < OB_PACKET_NUM; i++)
      {
        packet_handler_[i] = phandle_non_impl;
        trans_handler_[i] = thandle_non_impl;
        commit_handler_[i] = chandle_non_impl;
      }

      packet_handler_[OB_FREEZE_MEM_TABLE] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_MINOR_FREEZE_MEMTABLE] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_MINOR_LOAD_BYPASS] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_MAJOR_LOAD_BYPASS] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_ASYNC_MAJOR_FREEZE_MEMTABLE] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_ASYNC_AUTO_FREEZE_MEMTABLE] = phandle_freeze_memtable;
      packet_handler_[OB_UPS_CLEAR_ACTIVE_MEMTABLE] = phandle_clear_active_memtable;
      packet_handler_[OB_UPS_ASYNC_CHECK_CUR_VERSION] = phandle_check_cur_version;
      packet_handler_[OB_UPS_ASYNC_CHECK_SSTABLE_CHECKSUM] = phandle_check_sstable_checksum;

      trans_handler_[OB_TRUNCATE_TABLE] = thandle_write_trans; //add hxlong [truncate table] 20170403
      trans_handler_[OB_COMMIT_END] = thandle_commit_end;
      trans_handler_[OB_NEW_SCAN_REQUEST] = thandle_scan_trans;
      trans_handler_[OB_NEW_GET_REQUEST] = thandle_get_trans;
      trans_handler_[OB_SCAN_REQUEST] = thandle_scan_trans;
      trans_handler_[OB_GET_REQUEST] = thandle_get_trans;
      trans_handler_[OB_MS_MUTATE] = thandle_write_trans;
      trans_handler_[OB_WRITE] = thandle_write_trans;
      trans_handler_[OB_PHY_PLAN_EXECUTE] = thandle_write_trans;
      trans_handler_[OB_START_TRANSACTION] = thandle_start_session;
      trans_handler_[OB_UPS_ASYNC_KILL_ZOMBIE] = thandle_kill_zombie;
      trans_handler_[OB_UPS_SHOW_SESSIONS] = thandle_show_sessions;
      trans_handler_[OB_UPS_KILL_SESSION] = thandle_kill_session;
      trans_handler_[OB_END_TRANSACTION] = thandle_end_session;

      //add hxlong [Truncate Table]:20170127:b
      commit_handler_[OB_TRUNCATE_TABLE] = chandle_write_commit;
      //add:e
      commit_handler_[OB_MS_MUTATE] = chandle_write_commit;
      commit_handler_[OB_WRITE] = chandle_write_commit;
      commit_handler_[OB_PHY_PLAN_EXECUTE] = chandle_write_commit;
      commit_handler_[OB_END_TRANSACTION] = chandle_write_commit;
      commit_handler_[OB_SEND_LOG] = chandle_send_log;
      commit_handler_[OB_FAKE_WRITE_FOR_KEEP_ALIVE] = chandle_fake_write_for_keep_alive;
      commit_handler_[OB_SLAVE_REG] = chandle_slave_reg;
      commit_handler_[OB_SWITCH_SCHEMA] = chandle_switch_schema;
      commit_handler_[OB_UPS_FORCE_FETCH_SCHEMA] = chandle_force_fetch_schema;
      commit_handler_[OB_UPS_SWITCH_COMMIT_LOG] = chandle_switch_commit_log;
      //commit_handler_[OB_NOP_PKT] = chandle_nop;
      //delete by zhouhuan [scalablecommit]
    }

    TransExecutor::~TransExecutor()
    {
      destroy();
      session_mgr_.destroy();
      allocator_.destroy();
    }

    int TransExecutor::init(const int64_t trans_thread_num,
                            const int64_t trans_thread_start_cpu,
                            const int64_t trans_thread_end_cpu,
                            const int64_t commit_thread_cpu)
                           // const int64_t commit_end_thread_num)
    {
      int ret = OB_SUCCESS;
      bool queue_rebalance = true;
      bool dynamic_rebalance = true;
      // add by guojinwei [log synchronization][multi_cluster] 20151028:b
      message_residence_time_us_ = 1000;
      message_residence_protection_us_ = UPS.get_param().message_residence_protection_time;
      message_residence_max_us_ = UPS.get_param().message_residence_max_time;
      last_commit_log_time_us_ = 0;
      commit_task_num_ = 0;
      // add:e
      TransHandlePool::set_cpu_affinity(trans_thread_start_cpu, trans_thread_end_cpu);
      TransCommitThread::set_cpu_affinity(commit_thread_cpu);
      if (OB_SUCCESS != (ret = allocator_.init(ALLOCATOR_TOTAL_LIMIT, ALLOCATOR_HOLD_LIMIT, OB_LOG_BUFFER_MAX_SIZE)))
      {
        TBSYS_LOG(WARN, "init allocator fail ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = session_mgr_.init(MAX_RO_NUM, MAX_RP_NUM, MAX_RW_NUM, MAX_LRW_NUM, &session_ctx_factory_)))  // add a parameter by qx 20170314 for long transcation
      {
        TBSYS_LOG(WARN, "init session mgr fail ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = flush_queue_.init(FLUSH_QUEUE_SIZE)))
      {
        TBSYS_LOG(ERROR, "flush_queue.init(%ld)=>%d", FLUSH_QUEUE_SIZE, ret);
      }
      else if (OB_SUCCESS != (ret = TransCommitThread::init(TASK_QUEUE_LIMIT, FINISH_THREAD_IDLE)))
      {
        TBSYS_LOG(WARN, "init TransCommitThread fail ret=%d", ret);
      }
      //modify by hushuang [scalable commit]20160507
//      else if (OB_SUCCESS != (ret = TransHandlePool::init(trans_thread_num, TASK_QUEUE_LIMIT, queue_rebalance, dynamic_rebalance)))
//      {
//        TBSYS_LOG(WARN, "init TransHandlePool fail ret=%d", ret);
//      }
      else
      {
        for(int i = 0; i < trans_thread_num; i++)
        {
          if(OB_SUCCESS != (commit_queue_[i].init(TASK_QUEUE_LIMIT)))
          {
            TBSYS_LOG(WARN, "init commit_queue_ failed ret = %d", ret);
            break;
          }
          //TBSYS_LOG(ERROR, "test::whx ack %p", ack_[i]);
        }
        if (OB_SUCCESS != (ret = TransHandlePool::init(trans_thread_num, TASK_QUEUE_LIMIT, queue_rebalance, dynamic_rebalance, commit_queue_)))
        {
          TBSYS_LOG(WARN, "init TransHandlePool fail ret=%d", ret);
        }
      }

      //delete by zhouhuan [scalablecommit] 20160427
//      else if (OB_SUCCESS != (ret = CommitEndHandlePool::init(commit_end_thread_num, TASK_QUEUE_LIMIT, queue_rebalance, false)))
//      {
//        TBSYS_LOG(WARN, "init CommitEndHandlePool fail ret=%d", ret);
//      }
      //else
      if(OB_SUCCESS == ret)
      //modify  e
      {
        fifo_stream_.init("run/updateserver.fifo", 100L * 1024L * 1024L);
        //nop_task_.pkt.set_packet_code(OB_NOP_PKT);
        //add by zhouhuan [scalablecommit] 20160509:b
        nop_pos_.group_id_ = -1;
        //add:e
        TBSYS_LOG(INFO, "TransExecutor init succ");
      }
      return ret;
    }

    void TransExecutor::destroy()
    {
      StressRunnable::stop_stress();
      TransHandlePool::destroy();
      TransCommitThread::destroy();
      CommitEndHandlePool::destroy();
      fifo_stream_.destroy();
    }

    void TransExecutor::on_commit_push_fail(void* ptr)
    {
      TBSYS_LOG(ERROR, "commit push fail, will kill self, task=%p", ptr);
      kill(getpid(), SIGTERM);
    }

    void TransExecutor::handle_packet(ObPacket &pkt)
    {
      int ret = OB_SUCCESS;
      int pcode = pkt.get_packet_code();
      TBSYS_LOG(DEBUG, "start handle packet pcode=%d", pcode);
      if (0 > pcode
          || OB_PACKET_NUM <= pcode)
      {
        onev_request_e *req = pkt.get_request();
        TBSYS_LOG(ERROR, "invalid packet code=%d src=%s",
                  pcode, NULL == req ? NULL : get_peer_ip(req));
        ret = OB_UNKNOWN_PACKET;
      }
      else if (!handle_in_situ_(pcode))
      {
        int64_t task_size = sizeof(Task) + pkt.get_buffer()->get_capacity();
        Task *task = (Task*)allocator_.alloc(task_size);
        if (NULL == task)
        {
          ret = OB_MEM_OVERFLOW;
        }
        else
        {
          task->reset();
          task->pkt = pkt;
          task->src_addr = get_onev_addr(pkt.get_request());
          char *data_buffer = (char*)task + sizeof(Task);
          memcpy(data_buffer, pkt.get_buffer()->get_data(), pkt.get_buffer()->get_capacity());
          task->pkt.get_buffer()->set_data(data_buffer, pkt.get_buffer()->get_capacity());
          task->pkt.get_buffer()->get_position() = pkt.get_buffer()->get_position();
          TBSYS_LOG(DEBUG, "task_size=%ld data_size=%ld pos=%ld",
                    task_size, pkt.get_buffer()->get_capacity(), pkt.get_buffer()->get_position());
          (task->pkt).set_receive_ts(tbsys::CTimeUtil::getTime());
          //modify by zhouhuan [scalablecommit] 20160417
          //ret = push_task_(*task);
          if (!handle_in_wthread_(pcode))
          {
            ret = TransHandlePool::push(task);
          }
          else
          {
            handle_special_task(task, special_pdata);
          }
          //modify :e
        }
      }
      else
      {
        int64_t packet_timewait = (0 == pkt.get_source_timeout()) ?
                                  UPS.get_param().packet_max_wait_time :
                                  pkt.get_source_timeout();
        // 1.等待正在运行的事务提交
        // 2.等待commitlog缓冲区中的日志刷到磁盘
        // 避免事务运行过程中由于冻结而操作了不同的active_memtable
        ret = session_mgr_.wait_write_session_end_and_lock(packet_timewait);
        if (OB_SUCCESS == ret)
        {
          //modify by zhouhuan [scalablecommit] 20160428
          //ObSpinLockGuard guard(write_clog_mutex1_);
          write_clog_mutex_.wrlock();
          ThreadSpecificBuffer::Buffer* my_buffer = my_thread_buffer_.get_buffer();
          if (NULL == my_buffer)
          {
            TBSYS_LOG(WARN, "get thread specific buffer fail");
            ret = OB_MEM_OVERFLOW;
          }
          else
          {
            my_buffer->reset();
            ObDataBuffer thread_buff(my_buffer->current(), my_buffer->remain());
            packet_handler_[pcode](pkt, thread_buff);
          }
          session_mgr_.unlock_write_session();
          write_clog_mutex_.unlock();
        }
        else
        {
          TBSYS_LOG(WARN, "wait_write_session_end_and_lock(pkt=%d, timeout=%ld)=>%d", pcode, packet_timewait, ret);
        }
      }
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "handle_pkt fail, ret=%d, pkt=%d", ret, pkt.get_packet_code());
        UPS.response_result(ret, pkt);
      }
    }

    bool TransExecutor::handle_in_situ_(const int pcode)
    {
      bool bret = false;
      if (OB_FREEZE_MEM_TABLE == pcode
          || OB_UPS_MINOR_FREEZE_MEMTABLE == pcode
          || OB_UPS_MINOR_LOAD_BYPASS == pcode
          || OB_UPS_MAJOR_LOAD_BYPASS == pcode
          || OB_UPS_ASYNC_MAJOR_FREEZE_MEMTABLE == pcode
          || OB_UPS_ASYNC_AUTO_FREEZE_MEMTABLE == pcode
          || OB_UPS_CLEAR_ACTIVE_MEMTABLE == pcode
          || OB_UPS_ASYNC_CHECK_CUR_VERSION == pcode
          || OB_UPS_ASYNC_CHECK_SSTABLE_CHECKSUM == pcode)
      {
        bret = true;
      }
      return bret;
    }

    //add by zhouhuan [scalablecommit] 20160417:b
    bool TransExecutor::handle_in_wthread_(const int pcode)
    {
      bool bret = false;
      if (OB_SEND_LOG == pcode
          || OB_SLAVE_REG == pcode
          || OB_SWITCH_SCHEMA == pcode
          || OB_UPS_FORCE_FETCH_SCHEMA == pcode
          || OB_UPS_SWITCH_COMMIT_LOG == pcode
          || OB_FAKE_WRITE_FOR_KEEP_ALIVE == pcode)
      {
        bret = true;
      }
      return bret;
    }

    void TransExecutor::handle_special_task(Task *task, CommitParamData& special_pdata)
    {
      int ret = OB_SUCCESS;
      bool release_task = true;
      if (is_only_master_can_handle(task->pkt.get_packet_code()))
      {
        FakeWriteGuard guard(session_mgr_);
        if (!guard.is_granted())
        {
          ret = OB_NOT_MASTER;
          TBSYS_LOG(WARN, "only master can handle pkt_code=%d", task->pkt.get_packet_code());
        }
        else if (OB_FAKE_WRITE_FOR_KEEP_ALIVE == task->pkt.get_packet_code())
        {        
          release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, special_pdata);  
        }
        else
        {
          write_clog_mutex_.wrlock();
          //ObSpinLockGuard guard(write_clog_mutex1_);
          release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, special_pdata);
          write_clog_mutex_.unlock();
        }
      }
      else
      {
        release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, special_pdata);
      }

      if (OB_SUCCESS != ret || OB_SUCCESS != thread_errno())
      {
        TBSYS_LOG(WARN, "process fail ret=%d pcode=%d src=%s",
                  (OB_SUCCESS != ret) ? ret : thread_errno(), task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
      }
      if (OB_SUCCESS != ret)
      {
        UPS.response_result(ret, task->pkt);
      }
      if (release_task)
      {
        allocator_.free(task);
        task = NULL;
      }
    }

    int TransExecutor::handle_precommit(Task &task)
    {
      int err = OB_SUCCESS;
      int64_t switch_flag = 0;
      ObLogEntry entry;
      int64_t len = 0;
      FLogPos cur_pos, next_pos;
      LogGroup* cur_group = NULL;
      ObUpsLogMgr& log_mgr = UPS.get_log_mgr();
      int64_t trans_id = -1;
      int64_t ref_cnt = 0;
      if (is_write_packet(task.pkt))
      {
        SessionGuard session_guard(session_mgr_, lock_mgr_, err);
        RWSessionCtx* session_ctx = NULL;
        if (OB_SUCCESS != (err = session_guard.fetch_session(task.sid, session_ctx)))
        {
          TBSYS_LOG(ERROR, "fetch_session(sid=%s)=>%d", to_cstring(task.sid), err);
        }
        else
        {
          len = session_ctx->get_ups_mutator().get_serialize_size() + entry.get_serialize_size();
        }

        if (!log_mgr.check_log_size(len))
        {
          err = OB_LOG_TOO_LARGE;
          TBSYS_LOG(ERROR, "mutator.size[%ld] too large",
                    session_ctx->get_ups_mutator().get_serialize_size());
        }
        else if (session_ctx->is_frozen())
        {
          err = OB_SESSION_FROZEN;
          task.sid.reset();
          TBSYS_LOG(ERROR, "session stat is frozen, maybe request duplicate, sd=%u",
                    session_ctx->get_session_descriptor());
        }
        else if (0 != session_ctx->get_ups_mutator().get_mutator().size()
                 || (OB_PHY_PLAN_EXECUTE != task.pkt.get_packet_code()
                     && OB_END_TRANSACTION != task.pkt.get_packet_code()))
        {
          write_clog_mutex_.rdlock();
          {
            //ObSpinLockGuard guard(write_clog_mutex1_);
            if (OB_SUCCESS != ( err = log_mgr.set_log_position(cur_pos, len, switch_flag, next_pos)))
            {
              TBSYS_LOG(ERROR, "set_log_position()=>%d", err);
            }
          }
          write_clog_mutex_.unlock();

          if (OB_SUCCESS == err)
          {
            //add by zhouhuan for __all_server_stat 20160530
            int64_t cur_time = tbsys::CTimeUtil::getTime();
            OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_STIME, cur_time - session_ctx->get_last_proc_time());
            session_ctx->set_last_proc_time(cur_time);
            //add :e
            //TBSYS_LOG(ERROR, "test::zhouhuan common write task to set_log_position finished!!!! pcode=%d,data_len=%ld,switch_flag=%ld,cur_pos=[%s], next_pos=[%s]",
                      //task.pkt.get_packet_code(), len, switch_flag, to_cstring(cur_pos), to_cstring(next_pos));
            if (OB_GROUP_SWITCHED == switch_flag)//group switch
            {
              //TBSYS_LOG(INFO, "test::zhouhuan common write task to switch group!!!! group_id = %ld len = %d", cur_pos.group_id_, cur_pos.rel_offset_);
              if(OB_SUCCESS != (err = log_mgr.write_end_log(cur_pos, ref_cnt, log_mgr.get_flushed_clog_id_without_update())))
              {
                TBSYS_LOG(ERROR, "write_end_log()=>%d", err);
              }
              else if (OB_SUCCESS != (err = push_commit_group(cur_pos, ref_cnt)))
              {
                TBSYS_LOG(ERROR, "commit thread queue is full, err=%d", err);
              }
              else
              {
                //TBSYS_LOG(ERROR, "test::zhouhuan handle_precommit GROUP_SWITCH cur_pos=[%s], next_pos=[%s]",to_cstring(cur_pos), to_cstring(next_pos));
                cur_pos = next_pos;
              }

              if (OB_SUCCESS != err)
              {
                TBSYS_LOG(ERROR, "Fill log error when group switch kill self, err=%d", err);
                kill(getpid(), SIGTERM);
              }
            }

            if (OB_SUCCESS == err)
            {
              if ( cur_pos.rel_id_ == 1)
              {
                //TBSYS_LOG(ERROR, "test::zhouhuan set_group_start_timestamp");
                if (OB_SUCCESS != (err = log_mgr.set_group_start_timestamp(cur_pos)))
                {
                  TBSYS_LOG(WARN, "set_group_start_timestamp()=>%d", err);
                }
              }

              //TBSYS_LOG(ERROR, "test::zhouhuan get_trans_id");
              trans_id = log_mgr.get_trans_id(cur_pos); //TODO: some bugs here
              //add by zhouhuan for __all_server_stat 20160530
              int64_t cur_time = tbsys::CTimeUtil::getTime();
              OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_GTIME, cur_time - session_ctx->get_last_proc_time());
              session_ctx->set_last_proc_time(cur_time);
              //add e
              __sync_synchronize();
              session_ctx->set_trans_id(trans_id);
              session_ctx->get_checksum();
              session_ctx->get_ups_mutator().set_mutate_timestamp(trans_id);
              //TBSYS_LOG(ERROR, "test::zhouhuan session_mgr_.precommit");
              if (task.sid.is_valid() && OB_SUCCESS != (err = session_ctx->precommit()))
              {
                TBSYS_LOG(ERROR, "precommit(%s)=>%d",to_cstring(task.sid), err);
              }
              else
              {
                cur_group = log_mgr.get_log_group(cur_pos.group_id_);
                task.log_id_ = cur_group->start_log_id_ + cur_pos.rel_id_;
                err = UPS.get_table_mgr().fill_commit_log1(session_ctx->get_ups_mutator(), session_ctx->get_tlog_buffer(), cur_pos, ref_cnt);
                //add by zhouhuan for __all_server_stat 20160530
                int64_t cur_time = tbsys::CTimeUtil::getTime();
                OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_FTIME, cur_time - session_ctx->get_last_proc_time());
                session_ctx->set_last_proc_time(cur_time);
                //add e
                //TBSYS_LOG(INFO, "test::zhouhuan common write task to fill log!!!! group[%ld].start_log_id=[%ld] ts_seq=[%ld], rel_id[%d], ref_cnt[%ld]",
                //        cur_pos.group_id_, cur_group->start_log_id_, cur_group->ts_seq_, cur_pos.rel_id_, ref_cnt);
                if (OB_SUCCESS == err)
                {
                  //if(task.pkt.get_packet_code() == OB_WRITE)TBSYS_LOG(ERROR, "test::zhouhuan push private task 301,log_id = %ld", task.log_id_);
//                  if (OB_SUCCESS != (err = push_private_task(task.thread_no, &task))) //submit to private Commitqueue //delete by zhutao
                  if( OB_SUCCESS != (err = push_private_task(task))) //add by zhutao
                  {
                    TBSYS_LOG(ERROR, "Handle_thread commit queue is full, err=%d", err);
                  }
                  //add hushuang [scalable commit] 20160507
//                  else if (OB_SUCCESS != (err = UPS.get_log_mgr().submmit_callback_info(task.thread_no, cur_pos.group_id_, ack_[task.thread_no]))) //delete by zhutao
//                  else if (OB_SUCCESS != (err = UPS.get_log_mgr().submmit_callback_info(TransHandlePool::get_thread_index(), cur_pos.group_id_, ack_[TransHandlePool::get_thread_index()]))) //add by zhutao
//                  {
//                    TBSYS_LOG(ERROR, "submit call back function failed,ret = %d", err);
//                  }
                  //add e
                  //task.pkt.set_packet_code(OB_COMMIT_END);
                  if (OB_SUCCESS != (err = push_commit_group(cur_pos, ref_cnt)))
                  {
                    TBSYS_LOG(ERROR, "commit thread queue is full, err=%d", err);
                  }
//                  else if (OB_SUCCESS != (err = CommitEndHandlePool::push(&task)))
//                  {
//                    TBSYS_LOG(ERROR, "push(task=%p)=>%d, will kill self", &task, err);
//                    kill(getpid(), SIGTERM);
//                  }
                  else
                  {
                    session_ctx->mark_done(BaseSessionCtx::ES_UPDATE_COMMITED_TRANS_ID);
                    //add by zhouhuan for __all_server_stat 20160530
                    int64_t cur_time = tbsys::CTimeUtil::getTime();
                    OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_PUTIME, cur_time - session_ctx->get_last_proc_time());
                    session_ctx->set_last_proc_time(cur_time);
                    //add e
                  }
                }
              }

              if (OB_SUCCESS != err || trans_id == -1)
              {
                TBSYS_LOG(ERROR, "Fill log error ,kill self, err=%d", err);
                kill(getpid(), SIGTERM);
              }
            }

            //check the system is idle
//            if (OB_SUCCESS == err && true == is_system_idle(cur_pos))
//            {
//              OB_STAT_INC(UPDATESERVER, UPS_STAT_IDLE_COUNT,1);
//              if (OB_SUCCESS != (err = UPS.get_log_mgr().write_end_log(cur_pos, ref_cnt)))
//              {
//                TBSYS_LOG(ERROR, "switch_log_group, err=%d", err);
//              }
//              else if (OB_SUCCESS != (err = push_commit_group(cur_pos, ref_cnt)))
//              {
//                TBSYS_LOG(ERROR, "commit thread queue is full, err=%d", err);
//              }

//              if(OB_SUCCESS != err)
//              {
//                TBSYS_LOG(ERROR, "handle frozen group error ,kill self, err=%d", err);
//                kill(getpid(), SIGTERM);
//              }
//            }
          }

        }
        else
        {//对于那种只读的长事务或者select for update事务来说。原OB是压入flush_queue中的
          FLogPos pos = log_mgr.get_next_pos();
          cur_group= log_mgr.get_log_group(pos.group_id_);
          task.log_id_ = cur_group->start_log_id_ + cur_pos.rel_id_;
          //task.pkt.set_packet_code(OB_COMMIT_END);
          if (task.sid.is_valid() && OB_SUCCESS != (err = session_ctx->precommit()))
          {
            TBSYS_LOG(ERROR, "precommit(%s)=>%d",to_cstring(task.sid), err);
          }
//          else if (OB_SUCCESS != (err = TransHandlePool::push_private_task(task.thread_no, &task))) //delete by zhutao
          else if( OB_SUCCESS != (err = push_private_task(task)) ) //add by zhutao
          {
            TBSYS_LOG(ERROR, "Handle_thread commit queue is full, err=%d", err);
          }
//          else if (OB_SUCCESS != (err = CommitEndHandlePool::push(&task)))
//          {
//            TBSYS_LOG(ERROR, "push(task=%p)=>%d, will kill self", &task, err);
//            kill(getpid(), SIGTERM);
//          }
          //压入私有队列中或者直接响应客户端，两种选择是由 这种事务是否加入了回调函数中来决定。
        }
      }

      if (OB_SUCCESS != err)
      {
        task.sid.reset();
      }
      //UPS.get_log_mgr().printBuf();
      return err;
    }

    int TransExecutor::push_commit_group(FLogPos& cur_pos, int64_t ref_cnt)
    {
      int ret = OB_SUCCESS;
      LogGroup* cur_group = UPS.get_log_mgr().get_log_group(cur_pos.group_id_);
//      TBSYS_LOG(ERROR,"test::zhouhuan pre push commit group cur_pos.group_id=%ld, cur_group->group_id=%ld, ref_cnt=%ld  count=%ld, startlogid %ld"
//                , cur_pos.group_id_, cur_group->group_id_, ref_cnt, cur_group->count_, cur_group->start_log_id_);
      if (ref_cnt == cur_group->count_ && cur_pos.group_id_ == cur_group->group_id_)
      {
        int64_t trans_id = cur_group->start_timestamp_ + cur_group->count_ - 1;
        cur_group->need_ack_ = true;
        //add by zhouhuan for __all_server_stat 20160530
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        //OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_FTIME, cur_time - cur_group->get_last_proc_time());
        //cur_group->set_last_proc_time(cur_time);
        cur_group->set_last_fill_time(cur_time);
        //add:e
//       TBSYS_LOG(ERROR,"test::zhouhuan push commit group group_id=%ld, ref_cnt=%ld  count=%ld, rel_id=%d log_id[%ld,%ld]"
//                 , cur_pos.group_id_, ref_cnt, cur_group->count_, cur_pos.rel_id_, cur_group->start_cursor_.log_id_, cur_group->end_cursor_.log_id_);
        if (OB_SUCCESS != (ret = TransCommitThread::push(cur_pos.group_id_, cur_group)))//push to commit thread
        {
          TBSYS_LOG(ERROR, "commit thread queue is full, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = session_mgr_.update_commited_trans_id(trans_id)))
        {
          TBSYS_LOG(ERROR, "update_commited_trans_id(trans_id=%ld), err=%d", trans_id, ret);
        }
      }
      return ret;
    }

//    bool TransExecutor::is_system_idle(FLogPos& cur_pos)
//    {
//      int ret = false;
//      int64_t cur_time_us = tbsys::CTimeUtil::getTime();
////      TBSYS_LOG(INFO,"test::zhouhuan commit_task=[%ld], commit_queue=[%ld], slave_ptime=[%ld], peroid_switch=[%ld]",
////                get_commit_task_num(), TransHandlePool::get_queued_num(),message_residence_time_us_, cur_time_us - last_commit_log_time_us_);
//      //if (0 == TransCommitThread::get_queued_num() || 0 != TransHandlePool::get_queued_num())
//      if (0 >=get_commit_task_num() && 0 >= TransHandlePool::get_queued_num()
//          && ((message_residence_time_us_<message_residence_max_us_?
//               (message_residence_time_us_+message_residence_protection_us_):message_residence_max_us_)
//             <= (cur_time_us - last_commit_log_time_us_)))
//      {
//        //ObSpinLockGuard guard(write_clog_mutex1_);
//        TBSYS_LOG(INFO,"test::zhouhuan commit_task=[%ld], commit_queue=[%ld], slave_ptime=[%ld], peroid_switch=[%ld], group[%ld].rel_id=%d",
//                  get_commit_task_num(), TransHandlePool::get_queued_num(),
//                  message_residence_time_us_, cur_time_us - last_commit_log_time_us_, cur_pos.group_id_, cur_pos.rel_id_);
//        last_commit_log_time_us_ = tbsys::CTimeUtil::getTime();
//        FLogPos next_pos = UPS.get_log_mgr().get_next_pos();
//        if ( cur_pos.equal(next_pos) && (next_pos.rel_id_ != 0 || next_pos.rel_offset_ != 0)
//            && UPS.get_log_mgr().switch_group(cur_pos))
//        {
//          ret = true;
//        }
//      }
//      return ret;
//    }
    //add by :e

    //delete by zhouhuan [scalable commit] 20160510:b
   /* int TransExecutor::push_task_(Task &task)
    {
      int ret = OB_SUCCESS;
      switch (task.pkt.get_packet_code())
      {
        case OB_NEW_SCAN_REQUEST:
        case OB_NEW_GET_REQUEST:
        case OB_SCAN_REQUEST:
        case OB_GET_REQUEST:
        case OB_MS_MUTATE:
        case OB_WRITE:
        case OB_PHY_PLAN_EXECUTE:
        case OB_START_TRANSACTION:
        case OB_UPS_ASYNC_KILL_ZOMBIE:
        case OB_UPS_SHOW_SESSIONS:
        case OB_UPS_KILL_SESSION:
        case OB_END_TRANSACTION:
          ret = TransHandlePool::push(&task);
          break;
        case OB_SEND_LOG:
        case OB_FAKE_WRITE_FOR_KEEP_ALIVE:
        case OB_SLAVE_REG:
        case OB_SWITCH_SCHEMA:
        case OB_UPS_FORCE_FETCH_SCHEMA:
        case OB_UPS_SWITCH_COMMIT_LOG:
          ret = TransCommitThread::push(&task);
          break;
        default:
          TBSYS_LOG(ERROR, "unknown packet code=%d src=%s",
                    task.pkt.get_packet_code(), inet_ntoa_r(task.src_addr));
          ret = OB_UNKNOWN_PACKET;
          break;
      }
      return ret;
    }*/
    //delete:e

    bool TransExecutor::wait_for_commit_(const int pcode)
    {
      bool bret = true;
      if (OB_MS_MUTATE == pcode
          || OB_WRITE == pcode
          || OB_PHY_PLAN_EXECUTE == pcode
          || OB_END_TRANSACTION == pcode
          || OB_NOP_PKT == pcode)
      {
        bret = false;
      }
      return bret;
    }

    bool TransExecutor::is_only_master_can_handle(const int pcode)
    {
      return OB_FAKE_WRITE_FOR_KEEP_ALIVE == pcode
        || OB_SWITCH_SCHEMA == pcode
        || OB_UPS_FORCE_FETCH_SCHEMA == pcode
        || OB_UPS_SWITCH_COMMIT_LOG == pcode;
    }

    int TransExecutor::fill_return_rows_(sql::ObPhyOperator &phy_op, ObNewScanner &scanner, sql::ObUpsResult &ups_result)
    {
      int ret = OB_SUCCESS;
      scanner.reuse();
      const ObRow *row = NULL;
      while (OB_SUCCESS == (ret = phy_op.get_next_row(row)))
      {
        if (NULL == row)
        {
          TBSYS_LOG(WARN, "row null pointer, phy_op=%p type=%d", &phy_op, phy_op.get_type());
          ret = OB_ERR_UNEXPECTED;
          break;
        }
        if (OB_SUCCESS != (ret = scanner.add_row(*row)))
        {
          TBSYS_LOG(WARN, "add row to scanner fail, ret=%d %s", ret, to_cstring(*row));
          break;
        }
      }
      if (OB_ITER_END == ret)
      {
        if (OB_SUCCESS != (ret = ups_result.set_scanner(scanner)))
        {
          TBSYS_LOG(WARN, "set scanner to ups_result fail ret=%d", ret);
        }
      }
      return ret;
    }

    void TransExecutor::reset_warning_strings_()
    {
      tbsys::WarningBuffer *warning_buffer = tbsys::get_tsi_warning_buffer();
      if (NULL != warning_buffer)
      {
        warning_buffer->reset();
      }
    }

    void TransExecutor::fill_warning_strings_(sql::ObUpsResult &ups_result)
    {
      tbsys::WarningBuffer *warning_buffer = tbsys::get_tsi_warning_buffer();
      if (NULL != warning_buffer)
      {
        for (uint32_t i = 0; i < warning_buffer->get_total_warning_count(); i++)
        {
          ups_result.add_warning_string(warning_buffer->get_warning(i));
          TBSYS_LOG(DEBUG, "fill warning string idx=%d [%s]", i, warning_buffer->get_warning(i));
        }
        warning_buffer->reset();
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void TransExecutor::handle_trans(void *ptask, void *pdata)
    {

      ob_reset_err_msg();
      int ret = OB_SUCCESS;
      thread_errno() = OB_SUCCESS;
      bool release_task = true;
      Task *task = (Task*)ptask;
      TransParamData *param = (TransParamData*)pdata;
      int64_t packet_timewait = (NULL == task || 0 == task->pkt.get_source_timeout()) ?
                                UPS.get_param().packet_max_wait_time :
                                task->pkt.get_source_timeout();
      int64_t process_timeout = packet_timewait - QUERY_TIMEOUT_RESERVE;
      if (NULL == task)
      {
        TBSYS_LOG(WARN, "null pointer task=%p", task);
      }
      else if (NULL == param)
      {
        TBSYS_LOG(WARN, "null pointer param data pdata=%p src=%s",
                  pdata, inet_ntoa_r(task->src_addr));
        ret = OB_ERR_UNEXPECTED;
      }
      else if (0 > task->pkt.get_packet_code()
              || OB_PACKET_NUM <= task->pkt.get_packet_code())
      {
        TBSYS_LOG(ERROR, "unknown packet code=%d src=%s",
                  task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
        ret = OB_UNKNOWN_PACKET;
      }
      else if (OB_COMMIT_END != task->pkt.get_packet_code()
              && (process_timeout < 0 || process_timeout < (tbsys::CTimeUtil::getTime() - task->pkt.get_receive_ts())))
      {
        OB_STAT_INC(UPDATESERVER, UPS_STAT_PACKET_LONG_WAIT_COUNT, 1);
        TBSYS_LOG(WARN, "process timeout=%ld not enough cur_time=%ld receive_time=%ld packet_code=%d src=%s",
                  process_timeout, tbsys::CTimeUtil::getTime(), task->pkt.get_receive_ts(), task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
        ret = OB_PROCESS_TIMEOUT;
      }
      else
      {
        //忽略log自带的前两个字段，trace id和chid，以后续的trace id和chid为准
        PROFILE_LOG(DEBUG, TRACE_ID SOURCE_CHANNEL_ID PCODE WAIT_TIME_US_IN_RW_QUEUE,
                    (task->pkt).get_trace_id(),
                    (task->pkt).get_channel_id(),
                    (task->pkt).get_packet_code(),
                    tbsys::CTimeUtil::getTime() - (task->pkt).get_receive_ts());
        release_task = trans_handler_[task->pkt.get_packet_code()](*this, *task, *param);
      }
      if (NULL != task)
      {
        if ((OB_SUCCESS != ret && !IS_SQL_ERR(ret) && OB_BEGIN_TRANS_LOCKED != ret)
            || (OB_SUCCESS != thread_errno() && !IS_SQL_ERR(thread_errno()) && OB_BEGIN_TRANS_LOCKED != thread_errno()))
        {
          TBSYS_LOG(WARN, "process fail ret=%d pcode=%d src=%s",
                    (OB_SUCCESS != ret) ? ret : thread_errno(), task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
        }
        if (OB_SUCCESS != ret)
        {
          UPS.response_result(ret, task->pkt);
        }
        if (release_task)
        {
          allocator_.free(task);
          task = NULL;
        }
      }
    }

#define LOG_SESSION(header, session_ctx, task)                     \
  FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), header " packet wait=%ld start_time=%ld timeout=%ld src=%s fd=%d %s ctx=%p", \
                 tbsys::CTimeUtil::getTime() - task.pkt.get_receive_ts(), \
                 task.pkt.get_receive_ts(),                             \
                 task.pkt.get_source_timeout(),                         \
                 inet_ntoa_r(task.src_addr),                            \
                 get_fd(task.pkt.get_request()),                        \
                 to_cstring(task.sid),                                  \
                 session_ctx);

    int TransExecutor::get_session_type(const ObTransID& sid, SessionType& type, const bool check_session_expired)
    {
      int ret = OB_SUCCESS;
      SessionGuard session_guard(session_mgr_, lock_mgr_, ret);
      BaseSessionCtx *session_ctx = NULL;
      if (OB_SUCCESS != (ret = session_guard.fetch_session(sid, session_ctx, check_session_expired)))
      {
        TBSYS_LOG(WARN, "fetch_session(%s)=>%d", to_cstring(sid), ret);
      }
      else
      {
        type = session_ctx->get_type();
      }
      return ret;
    }

    bool TransExecutor::handle_write_trans_(Task &task, ObMutator &mutator, ObNewScanner &scanner)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      ObTransReq req;
      SessionGuard session_guard(session_mgr_, lock_mgr_, ret);
      RWSessionCtx *session_ctx = NULL;
      ILockInfo *lock_info = NULL;
      int64_t pos = task.pkt.get_buffer()->get_position();
      int64_t packet_timewait = (0 == task.pkt.get_source_timeout()) ?
                                UPS.get_param().packet_max_wait_time :
                                task.pkt.get_source_timeout();
      int64_t process_timeout = packet_timewait - QUERY_TIMEOUT_RESERVE;
      req.start_time_ = task.pkt.get_receive_ts();
      req.timeout_ = process_timeout;
      req.idle_time_ = process_timeout;
      task.sid.reset();
      bool give_up_lock = true;
      if (!UPS.is_master_lease_valid())
      {
        ret = OB_NOT_MASTER;
        TBSYS_LOG(WARN, "ups master_lease is invalid, pcode=%d", task.pkt.get_packet_code());
      }
      else if (OB_SUCCESS != (ret = ui_.ui_deserialize_mutator(*task.pkt.get_buffer(), mutator)))
      {
        TBSYS_LOG(WARN, "deserialize mutator fail ret=%d", ret);
      }
      else
      {
        task.pkt.get_buffer()->get_position() = pos;
      }
      if (OB_SUCCESS != ret)
      {}
      else if (OB_SUCCESS != (ret = session_guard.start_session(req, task.sid, session_ctx)))
      {
        if (OB_BEGIN_TRANS_LOCKED == ret)
        {
          int tmp_ret = OB_SUCCESS;
          if (OB_SUCCESS != (tmp_ret = TransHandlePool::push(&task)))
          {
            ret = tmp_ret;
          }
        }
        else
        {
          TBSYS_LOG(ERROR, "start_session()=>%d", ret);
        }
      }
      else
      {
        //add by zhouhuan for __all_server_stat:20160531
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_WTIME, cur_time - task.pkt.get_receive_ts());
        session_ctx->set_last_proc_time(cur_time);
        //add:e
        LOG_SESSION("mutator start", session_ctx, task);
        session_ctx->set_start_handle_time(tbsys::CTimeUtil::getTime());
        session_ctx->set_stmt_start_time(task.pkt.get_receive_ts());
        session_ctx->set_stmt_timeout(process_timeout);
        session_ctx->set_self_processor_index(TransHandlePool::get_thread_index() + TransHandlePool::get_thread_num());
        session_ctx->set_priority((PriorityPacketQueueThread::QueuePriority)task.pkt.get_packet_priority());
        if (ST_READ_WRITE != session_ctx->get_type())
        {
          ret = OB_TRANS_NOT_MATCH;
          TBSYS_LOG(ERROR, "session.type[%d] is not RW", session_ctx->get_type());
        }
        else if (NULL == (lock_info = session_ctx->get_lock_info()))
        {
          ret = OB_NOT_INIT;
          TBSYS_LOG(ERROR, "lock_info == NULL");
        }
        else if (OB_SUCCESS != (ret = UPS.get_table_mgr().apply(OB_WRITE != task.pkt.get_packet_code(), *session_ctx, *lock_info, mutator)))
        {
          if ((OB_ERR_EXCLUSIVE_LOCK_CONFLICT == ret
                || OB_ERR_SHARED_LOCK_CONFLICT == ret)
              && !session_ctx->is_session_expired()
              && !session_ctx->is_stmt_expired())
          {
            int tmp_ret = OB_SUCCESS;
            if (OB_SUCCESS != (tmp_ret = TransHandlePool::push(&task)))
            {
              TBSYS_LOG(WARN, "push back task fail, ret=%d conflict_processor_index=%ld task=%p",
                        tmp_ret, session_ctx->get_conflict_processor_index(), &task);
            }
            else
            {
              give_up_lock = false;
            }
          }
          if (give_up_lock)
          {
            OB_STAT_INC(UPDATESERVER, UPS_STAT_APPLY_FAIL_COUNT, 1);
            TBSYS_LOG(WARN, "table mgr apply fail ret=%d", ret);
          }
        }
        else
        {
          scanner.reuse();
          if (OB_SUCCESS != (ret = session_ctx->get_ups_result().set_scanner(scanner)))
          {
            TBSYS_LOG(ERROR, "session_ctx.set_scanner()=>%d", ret);
          }
          else if (OB_SUCCESS != (ret = session_ctx->get_ups_mutator().get_mutator().pre_serialize()))
          {
            TBSYS_LOG(ERROR, "session_ctx.mutator.pre_serialize()=>%d", ret);
          }
          else
          {
            TBSYS_LOG(DEBUG, "precommit end timeu=%ld", tbsys::CTimeUtil::getTime() - task.pkt.get_receive_ts());
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, COUNT), 1);
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, QTIME), session_ctx->get_start_handle_time() - session_ctx->get_stmt_start_time());
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, TIMEU), tbsys::CTimeUtil::getTime() - session_ctx->get_start_handle_time());
            //add by zhouhuan for __all_server_stat 20150531
            cur_time = tbsys::CTimeUtil::getTime();
            OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_HTIME, cur_time - session_ctx->get_last_proc_time());
            session_ctx->set_last_proc_time(cur_time);
            //add e

            session_guard.revert();
            //modify by zhouhuan [scalablecommit] 20160418:b
            //ret = TransCommitThread::push(&task);
            ret = handle_precommit(task);
            //modify:e
            if (OB_SUCCESS != ret && task.sid.is_valid())
            {
              session_mgr_.end_session(task.sid.descriptor_, true);
            }
          }
        }
      }
      if (OB_SUCCESS != ret && OB_BEGIN_TRANS_LOCKED != ret && give_up_lock)
      {
        UPS.response_result(ret, task.pkt);
      }
      return (OB_SUCCESS != ret && OB_BEGIN_TRANS_LOCKED != ret && give_up_lock);
    }

    bool TransExecutor::handle_start_session_(Task &task, ObDataBuffer &buffer)
    {
      int &ret = thread_errno();
      bool need_free_task = true;
      ret = OB_SUCCESS;
      ObTransReq req;
      int64_t pos = task.pkt.get_buffer()->get_position();
      SessionGuard session_guard(session_mgr_, lock_mgr_, ret);
      BaseSessionCtx* session_ctx = NULL;
      task.sid.reset();
      if (!UPS.is_master_lease_valid())
      {
        ret = OB_NOT_MASTER;
        TBSYS_LOG(WARN, "ups master_lease is invalid, pcode=%d", task.pkt.get_packet_code());
      }
      else if (OB_SUCCESS != (ret = req.deserialize(task.pkt.get_buffer()->get_data(),
                                                    task.pkt.get_buffer()->get_capacity(),
                                                    pos)))
      {
        TBSYS_LOG(WARN, "deserialize session_req fail ret=%d", ret);
      }
      else
      {
        req.start_time_ = task.pkt.get_receive_ts();
      }
      if (OB_SUCCESS != ret)
      {}
      else if (OB_SUCCESS != (ret = session_guard.start_session(req, task.sid, session_ctx)))
      {
        if (OB_BEGIN_TRANS_LOCKED == ret)
        {
          int tmp_ret = OB_SUCCESS;
          if (OB_SUCCESS != (tmp_ret = TransHandlePool::push(&task)))
          {
            ret = tmp_ret;
          }
          else
          {
            need_free_task = false;
          }
        }
        else
        {
          TBSYS_LOG(WARN, "begin session fail ret=%d", ret);
        }
      }
      else
      {
        LOG_SESSION("session start", session_ctx, task);
        PRINT_TRACE_BUF(session_ctx->get_tlog_buffer());
      }
      if (need_free_task)
      {
        UPS.response_trans_id(ret, task.pkt, task.sid, buffer);
      }
      return need_free_task;
    }

    bool TransExecutor::handle_end_session_(Task &task, ObDataBuffer &buffer)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      bool need_free_task = true;
      ObEndTransReq req;
      SessionType type = ST_READ_ONLY;
      UNUSED(buffer);
      task.sid.reset();
      const bool check_session_expired = true;
      int64_t pos = task.pkt.get_buffer()->get_position();
      if (!UPS.is_master_lease_valid())
      {
        ret = OB_NOT_MASTER;
        TBSYS_LOG(WARN, "ups master_lease is invalid, pcode=%d", task.pkt.get_packet_code());
      }
      else if (OB_SUCCESS != (ret = req.deserialize(task.pkt.get_buffer()->get_data(),
                                                    task.pkt.get_buffer()->get_capacity(),
                                                    pos)))
      {
        TBSYS_LOG(WARN, "deserialize session_req fail ret=%d", ret);
      }
      else if (!req.trans_id_.is_valid())
      {
        ret = OB_TRANS_NOT_MATCH;
        TBSYS_LOG(ERROR, "sid[%s] is invalid", to_cstring(task.sid));
      }
      if (OB_SUCCESS != ret)
      {
        UPS.response_result(ret, task.pkt);
      }
      else if (OB_SUCCESS != (ret = get_session_type(req.trans_id_, type, check_session_expired)))
      {
        UPS.response_result(req.rollback_? OB_SUCCESS: ret, task.pkt);
        TBSYS_LOG(WARN, "get_session_type(%s)=>%d", to_cstring(req.trans_id_), ret);
      }
      else
      {
        // sid 正确
        if (ST_READ_WRITE == type && !req.rollback_)
        {
          task.sid = req.trans_id_;
          //modify by zhouhuan [scalablecommit] 20160426:b
          //if (OB_SUCCESS != (ret = TransCommitThread::push(&task)))
          if (OB_SUCCESS != (ret = handle_precommit(task)))
          {
            //TBSYS_LOG(ERROR, "push task=%p to TransCommitThread fail, ret=%d %s", &task, ret, to_cstring(req.trans_id_));
            TBSYS_LOG(ERROR, "handle_precommit fail task=%p, ret=%d %s", &task, ret, to_cstring(req.trans_id_));
            //modify:e
            session_mgr_.end_session(req.trans_id_.descriptor_, true);
            UPS.response_result(OB_TRANS_ROLLBACKED, task.pkt);
          }
          else
          {
            need_free_task = false;
          }
        }
        else
        {
          ret = session_mgr_.end_session(req.trans_id_.descriptor_, req.rollback_);
          UPS.response_result(req.rollback_? OB_SUCCESS: ret, task.pkt);
        }
      }
      return need_free_task;
    }

    bool TransExecutor::handle_phyplan_trans_(Task &task,
                                             ObUpsPhyOperatorFactory &phy_operator_factory,
                                             sql::ObPhysicalPlan &phy_plan,
                                             ObNewScanner &new_scanner,
                                             ModuleArena &allocator,
                                             ObDataBuffer& buffer)
    {
      reset_warning_strings_();
      bool need_free_task = true;
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      int end_session_ret = OB_SUCCESS;
      SessionGuard session_guard(session_mgr_, lock_mgr_, end_session_ret);
      RWSessionCtx* session_ctx = NULL;
      int64_t packet_timewait = (0 == task.pkt.get_source_timeout()) ?
                                UPS.get_param().packet_max_wait_time :
                                task.pkt.get_source_timeout();
      int64_t process_timeout = packet_timewait - QUERY_TIMEOUT_RESERVE;
      int64_t pos = task.pkt.get_buffer()->get_position();
      bool with_sid = false;
      task.sid.reset();
      bool give_up_lock = true;
      if (!UPS.is_master_lease_valid())
      {
        ret = OB_NOT_MASTER;
        TBSYS_LOG(WARN, "ups master_lease is invalid, pcode=%d", task.pkt.get_packet_code());
      }
      else if (OB_SUCCESS != (ret = phy_plan.deserialize_header(task.pkt.get_buffer()->get_data(),
                                                                task.pkt.get_buffer()->get_capacity(),
                                                                pos)))
      {
        TBSYS_LOG(WARN, "phy_plan.deseiralize_header ret=%d", ret);
      }
      else if ((with_sid = phy_plan.get_trans_id().is_valid()))
      {
        const bool check_session_expired = true;
        if (OB_SUCCESS != (ret = session_guard.fetch_session(phy_plan.get_trans_id(), session_ctx, check_session_expired)))
        {
          TBSYS_LOG(WARN, "Session has been killed, error %d, \'%s\'", ret, to_cstring(phy_plan.get_trans_id()));
        }
        else if (session_ctx->get_stmt_start_time() >= task.pkt.get_receive_ts())
        {
          TBSYS_LOG(ERROR, "maybe an expired request, will skip it, last_stmt_start_time=%ld receive_ts=%ld",
                    session_ctx->get_stmt_start_time(), task.pkt.get_receive_ts());
          ret = OB_STMT_EXPIRED;
        }
        else
        {
          CLEAR_TRACE_BUF(session_ctx->get_tlog_buffer());
          session_ctx->reset_stmt();
          task.sid = phy_plan.get_trans_id();
          LOG_SESSION("session stmt", session_ctx, task);
        }
      }
      else
      {
        phy_plan.get_trans_req().start_time_ = task.pkt.get_receive_ts();
        //add qx test
        //TBSYS_LOG(ERROR,"req type =%d",phy_plan.get_trans_req().type_);
        //ob_print_mod_memory_usage();
        if (OB_SUCCESS != (ret = session_guard.start_session(phy_plan.get_trans_req(), task.sid, session_ctx)))
        {
          if (OB_BEGIN_TRANS_LOCKED == ret)
          {
            int tmp_ret = OB_SUCCESS;
            if (OB_SUCCESS != (tmp_ret = TransHandlePool::push(&task)))
            {
              ret = tmp_ret;
            }
            else
            {
              need_free_task = false;
            }
          }
          else
          {
            TBSYS_LOG(ERROR, "start_session()=>%d", ret);
          }
        }
        else
        {
          if (phy_plan.get_start_trans())
          {
            session_ctx->get_ups_result().set_trans_id(task.sid);
          }
          LOG_SESSION("start_trans", session_ctx, task);
        }
      }
      if (OB_SUCCESS != ret)
      {}
      else
      {
        //test add by qx 20170313 :b
        //TBSYS_LOG(ERROR,"no group = %d long_trans = %d", phy_plan.is_group_exec(), phy_plan.is_long_trans_exec());
        //add :e
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_WTIME, cur_time - task.pkt.get_receive_ts());
        session_ctx->set_last_proc_time(cur_time);

        session_ctx->set_start_handle_time(tbsys::CTimeUtil::getTime());
        session_ctx->set_stmt_start_time(task.pkt.get_receive_ts());
        session_ctx->set_stmt_timeout(process_timeout);
        session_ctx->set_self_processor_index(TransHandlePool::get_thread_index() + TransHandlePool::get_thread_num());
        session_ctx->set_priority((PriorityPacketQueueThread::QueuePriority)task.pkt.get_packet_priority());
        session_ctx->mark_stmt();
        sql::ObPhyOperator *main_op = NULL;
        phy_operator_factory.set_session_ctx(session_ctx);
        phy_operator_factory.set_table_mgr(&UPS.get_table_mgr());
        phy_plan.clear();
        allocator.reuse();
        phy_plan.set_allocator(&allocator);
        phy_plan.set_operator_factory(&phy_operator_factory);

        int64_t pos = task.pkt.get_buffer()->get_position();
        if (OB_SUCCESS != (ret = phy_plan.deserialize(task.pkt.get_buffer()->get_data(),
                                                      task.pkt.get_buffer()->get_capacity(),
                                                      pos)))
        {
          TBSYS_LOG(WARN, "deserialize phy_plan fail ret=%d", ret);
        }
        else
        {
          //test add by qx 20170313 :b
          //TBSYS_LOG(ERROR,"no group = %d long_trans = %d", phy_plan.is_group_exec(), phy_plan.is_long_trans_exec());
          //TBSYS_LOG(ERROR,"phyplan allocator used=%ld total=%ld",allocator.used(), allocator.total());
          //add :e

          FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "phyplan allocator used=%ld total=%ld",
                        allocator.used(), allocator.total());
        }
        if (OB_SUCCESS != ret)
        {}
        else if (NULL == (main_op = phy_plan.get_main_query()))
        {
          TBSYS_LOG(WARN, "main query null pointer");
          ret = OB_ERR_UNEXPECTED;
        }
        //add lbzhong [auto_increment] 20161218:b
        else if (phy_plan.is_auto_increment() && PHY_UPS_MODIFY_WITH_DML_TYPE == main_op->get_type()
                 && OB_SUCCESS != (ret = static_cast<MemTableModifyWithDmlType*>(main_op)->set_is_update_auto_value()))
        {
          //do nothing
        }
        //add:e
        else if (OB_SUCCESS != (ret = main_op->open())
                || OB_SUCCESS != (ret = fill_return_rows_(*main_op, new_scanner, session_ctx->get_ups_result())))
        {
          session_ctx->rollback_stmt();
          //add wangjiahao [table lock] 20160616 :b

          if ((OB_ERR_TABLE_EXCLUSIVE_LOCK_CONFLICT == ret
                || OB_ERR_TABLE_INTENTION_LOCK_CONFLICT == ret)
                && !session_ctx->is_session_expired()
                && !session_ctx->is_stmt_expired())
          {
            TBSYS_LOG(WARN, "##TEST_PRINT##rollback because of table lock ret=%d", ret);
            if (!with_sid)
            {
              end_session_ret = ret;
            }
            give_up_lock = false;
            need_free_task = false;
          }
          //add :e
          if ((OB_ERR_EXCLUSIVE_LOCK_CONFLICT == ret
                || OB_ERR_SHARED_LOCK_CONFLICT == ret)
              && !session_ctx->is_session_expired()
              && !session_ctx->is_stmt_expired())
          {
            int tmp_ret = OB_SUCCESS;
            uint32_t session_descriptor = session_ctx->get_session_descriptor();
            int64_t conflict_processor_index = session_ctx->get_conflict_processor_index();
            session_ctx->set_stmt_start_time(0);
            if (!with_sid)
            {
              end_session_ret = ret;
            }
            session_ctx = NULL;
            session_guard.revert();
            if (OB_SUCCESS != (tmp_ret = TransHandlePool::push(&task)))
            {
              TBSYS_LOG(WARN, "push back task fail, ret=%d conflict_processor_index=%ld task=%p",
                        tmp_ret, conflict_processor_index, &task);
              session_mgr_.end_session(session_descriptor, true);
            }
            else
            {
              give_up_lock = false;
              need_free_task = false;
            }
          }
          if (OB_ERR_PRIMARY_KEY_DUPLICATE != ret
              && give_up_lock)
          {
            OB_STAT_INC(UPDATESERVER, UPS_STAT_APPLY_FAIL_COUNT, 1);
            //TBSYS_LOG(WARN, "main_op open fail ret=%d", ret);
          }
          main_op->close();
        }
        //else if (OB_SUCCESS != (ret = fill_return_rows_(*main_op, new_scanner, session_ctx->get_ups_result())))
        //{
        //  main_op->close();
        //  TBSYS_LOG(WARN, "fill result rows with main_op fail ret=%d", ret);
        //}
        else
        {
          //add lbzhong [auto_increment] 20161218:b
          if (phy_plan.is_auto_increment() && PHY_UPS_MODIFY_WITH_DML_TYPE == main_op->get_type())
          {
              session_ctx->get_ups_result().set_auto_value(static_cast<MemTableModifyWithDmlType*>(main_op)->get_updated_auto_value());
              session_ctx->get_ups_result().set_affected_rows(0);//ignore affect
          }
          //add:e
          session_ctx->commit_stmt();
          main_op->close();
          fill_warning_strings_(session_ctx->get_ups_result());
          TBSYS_LOG(DEBUG, "precommit end timeu=%ld", tbsys::CTimeUtil::getTime() - task.pkt.get_receive_ts());
        }

        if (OB_SUCCESS != ret)
        {
          if ((phy_plan.get_start_trans()
              && NULL != session_ctx
              && session_ctx->get_ups_result().get_trans_id().is_valid()
              && give_up_lock)
              //add lbzhong [auto_increment] 20161130:b
              //|| OB_ERR_AUTO_VALUE_NOT_SERVE == ret
              //add:e
              )
          {
            session_ctx->get_ups_result().set_error_code(ret);
            ret = OB_SUCCESS;
            session_ctx->get_ups_result().serialize(buffer.get_data(), buffer.get_capacity(), buffer.get_position());
            const char *error_string = ob_get_err_msg().ptr();
            UPS.response_buffer(ret, task.pkt, buffer, error_string);
          }
        }
        else if (with_sid || phy_plan.get_start_trans())
        {
          session_ctx->get_ups_result().serialize(buffer.get_data(), buffer.get_capacity(), buffer.get_position());
          UPS.response_buffer(ret, task.pkt, buffer);
          OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, COUNT), 1);
          OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, QTIME), session_ctx->get_start_handle_time() - session_ctx->get_stmt_start_time());
          OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, TIMEU), tbsys::CTimeUtil::getTime() - session_ctx->get_start_handle_time());
        }
        else
        {
          if (OB_SUCCESS != (ret = session_ctx->get_ups_mutator().get_mutator().pre_serialize()))
          {
            TBSYS_LOG(ERROR, "session_ctx.mutator.pre_serialize()=>%d", ret);
          }
          else
          {
            FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "ret=%d affected_rows=%ld", ret, session_ctx->get_ups_result().get_affected_rows());
            PRINT_TRACE_BUF(session_ctx->get_tlog_buffer());
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, COUNT), 1);
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, QTIME), session_ctx->get_start_handle_time() - session_ctx->get_stmt_start_time());
            OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), APPLY, TIMEU), tbsys::CTimeUtil::getTime() - session_ctx->get_start_handle_time());

            cur_time = tbsys::CTimeUtil::getTime();
            OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_HTIME, cur_time - session_ctx->get_last_proc_time());
            session_ctx->set_last_proc_time(cur_time);

            session_ctx = NULL;
            session_guard.revert();
            //modify by zhouhuan [scalablecommit] 20160426:b
            //if (OB_SUCCESS != (ret = TransCommitThread::push(&task)))
            //{
            //  TBSYS_LOG(WARN, "commit thread queue is full, ret=%d", ret);
            //}
            if (OB_SUCCESS != (ret = handle_precommit(task)))
            {
              TBSYS_LOG(WARN, "handle_precommit, ret=%d", ret);
            }
            //modify:e
            else
            {
              need_free_task = false;
            }
            if (OB_SUCCESS != ret && task.sid.is_valid())
            {
              session_mgr_.end_session(task.sid.descriptor_, true);
            }
          }
        }
        if (NULL != session_ctx)
        {
          FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "ret=%d affected_rows=%ld", ret, session_ctx->get_ups_result().get_affected_rows());
          PRINT_TRACE_BUF(session_ctx->get_tlog_buffer());
        }
        if (OB_SUCCESS != ret
            && ((!with_sid && !phy_plan.get_start_trans()) || !IS_SQL_ERR(ret))
            && give_up_lock)
        {
          end_session_ret = ret;
          TBSYS_LOG(DEBUG, "need rollback session %s ret=%d", to_cstring(task.sid), ret);
        }
      }
      phy_plan.clear();
      if (OB_SUCCESS != ret && OB_BEGIN_TRANS_LOCKED != ret && give_up_lock)
      {
        //ret = (OB_ERR_SHARED_LOCK_CONFLICT == ret) ? OB_EAGAIN : ret;
        const char *error_string = ob_get_err_msg().ptr();
        UPS.response_result(ret, error_string, task.pkt);
      }
      return need_free_task;
    }

    void TransExecutor::handle_get_trans_(ObPacket &pkt,
                                          ObGetParam &get_param,
                                          ObScanner &scanner,
                                          ObCellNewScanner &new_scanner,
                                          ObDataBuffer &buffer)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      uint32_t session_descriptor = UINT32_MAX;
      ROSessionCtx *session_ctx = NULL;
      int64_t packet_timewait = (0 == pkt.get_source_timeout()) ?
                                UPS.get_param().packet_max_wait_time :
                                pkt.get_source_timeout();
      int64_t process_timeout = packet_timewait - QUERY_TIMEOUT_RESERVE;
      int64_t pos = pkt.get_buffer()->get_position();
      if (OB_SUCCESS != (ret = get_param.deserialize(pkt.get_buffer()->get_data(),
                                                    pkt.get_buffer()->get_capacity(),
                                                    pos)))
      {
        TBSYS_LOG(WARN, "deserialize get_param fail ret=%d", ret);
      }
      else if (!UPS.can_serve_read_req(get_param.get_is_read_consistency(), get_param.get_version_range().get_query_version()))
      {
        TBSYS_LOG(WARN, "the scan request require consistency, ObiRole:%s RoleMgr:%s, query_version=%ld",
                  UPS.get_obi_role().get_role_str(), UPS.get_role_mgr().get_role_str(), get_param.get_version_range().get_query_version());
        ret = OB_NOT_MASTER;
      }
      else if (OB_SUCCESS != (ret = session_mgr_.begin_session(ST_READ_ONLY, pkt.get_receive_ts(), process_timeout, process_timeout, session_descriptor)))
      {
        TBSYS_LOG(WARN, "begin session fail ret=%d", ret);
      }
      else if (NULL == (session_ctx = session_mgr_.fetch_ctx<ROSessionCtx>(session_descriptor)))
      {
        TBSYS_LOG(WARN, "fetch ctx fail session_descriptor=%u", session_descriptor);
        ret = OB_ERR_UNEXPECTED;
      }
      else if (session_ctx->is_session_expired())
      {
        session_mgr_.revert_ctx(session_descriptor);
        session_ctx = NULL;
        ret = OB_TRANS_ROLLBACKED;
      }
      else
      {
        FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "start handle get, packet wait=%ld start_time=%ld timeout=%ld src=%s",
                      tbsys::CTimeUtil::getTime() - pkt.get_receive_ts(),
                      pkt.get_receive_ts(),
                      pkt.get_source_timeout(),
                      NULL == pkt.get_request() ? NULL : get_peer_ip(pkt.get_request()));
        thread_read_prepare();
        session_ctx->set_start_handle_time(tbsys::CTimeUtil::getTime());
        session_ctx->set_stmt_start_time(pkt.get_receive_ts());
        session_ctx->set_stmt_timeout(process_timeout);
        session_ctx->set_priority((PriorityPacketQueueThread::QueuePriority)pkt.get_packet_priority());
        // add by guojinwei [repeatable read] 20160418:b
        TBSYS_LOG(DEBUG, "ISOLATION_LEVEL = %d, guojinwei", get_param.get_trans_id().isolation_level_);
        if (common::REPEATABLE_READ == get_param.get_trans_id().isolation_level_
            && get_param.get_trans_id().is_valid()
            && 0 != get_param.get_trans_id().start_time_us_)
        {
          session_ctx->set_trans_start_time(get_param.get_trans_id().trans_start_time_us_);
        }
        if (get_param.get_trans_id().is_valid())
        {
          session_ctx->set_trans_descriptor(get_param.get_trans_id().descriptor_);
        }
        // add:e
        if (OB_NEW_GET_REQUEST == pkt.get_packet_code())
        {
          new_scanner.reuse();
          common::ObRowDesc row_desc;
          if(OB_SUCCESS != (ret = ObNewScannerHelper::get_row_desc(get_param, true, row_desc)))
          {
            TBSYS_LOG(WARN, "get row desc fail:ret[%d]", ret);
          }
          else
          {
            new_scanner.set_row_desc(row_desc);
            ret = UPS.get_table_mgr().new_get(*session_ctx, get_param, new_scanner, pkt.get_receive_ts(), process_timeout);
          }
          if (OB_SUCCESS == ret)
          {
            UPS.response_scanner(ret, pkt, new_scanner, buffer);
          }
        }
        else
        {
          scanner.reset();
          ret = UPS.get_table_mgr().get(*session_ctx, get_param, scanner, pkt.get_receive_ts(), process_timeout);
          if (OB_SUCCESS == ret)
          {
            UPS.response_scanner(ret, pkt, scanner, buffer);
          }
        }
        FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "get from table mgr ret=%d", ret);
        //OB_STAT_INC(UPDATESERVER, UPS_STAT_NL_GET_COUNT, 1);
        OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), GET, COUNT), 1);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_GET_QTIME, session_ctx->get_start_handle_time() - session_ctx->get_session_start_time());
        //OB_STAT_INC(UPDATESERVER, UPS_STAT_NL_GET_TIMEU, session_ctx->get_session_timeu());
        OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), GET, TIMEU), session_ctx->get_session_timeu());
        thread_read_complete();
        session_ctx->set_last_active_time(tbsys::CTimeUtil::getTime());
        session_mgr_.revert_ctx(session_descriptor);
        session_mgr_.end_session(session_descriptor);
        log_get_qps_();
      }
      if (OB_SUCCESS != ret)
      {
        UPS.response_result(ret, pkt);
      }
    }

    void TransExecutor::handle_scan_trans_(ObPacket &pkt,
                                          ObScanParam &scan_param,
                                          ObScanner &scanner,
                                          ObCellNewScanner &new_scanner,
                                          ObDataBuffer &buffer)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      uint32_t session_descriptor = UINT32_MAX;
      ROSessionCtx *session_ctx = NULL;
      int64_t packet_timewait = (0 == pkt.get_source_timeout()) ?
                                UPS.get_param().packet_max_wait_time :
                                pkt.get_source_timeout();
      int64_t process_timeout = packet_timewait - QUERY_TIMEOUT_RESERVE;
      int64_t pos = pkt.get_buffer()->get_position();
      if (OB_SUCCESS != (ret = scan_param.deserialize(pkt.get_buffer()->get_data(),
                                                      pkt.get_buffer()->get_capacity(),
                                                      pos)))
      {
        TBSYS_LOG(WARN, "deserialize get_param fail ret=%d", ret);
      }
      else if (!UPS.can_serve_read_req(scan_param.get_is_read_consistency(), scan_param.get_version_range().get_query_version()))
      {
        TBSYS_LOG(WARN, "the scan request require consistency, ObiRole:%s RoleMgr:%s, query_version=%ld",
                  UPS.get_obi_role().get_role_str(), UPS.get_role_mgr().get_role_str(), scan_param.get_version_range().get_query_version());
        ret = OB_NOT_MASTER;
      }
      else if (OB_SUCCESS != (ret = session_mgr_.begin_session(ST_READ_ONLY, pkt.get_receive_ts(), process_timeout, process_timeout, session_descriptor)))
      {
        TBSYS_LOG(WARN, "begin session fail ret=%d", ret);
      }
      else if (NULL == (session_ctx = session_mgr_.fetch_ctx<ROSessionCtx>(session_descriptor)))
      {
        TBSYS_LOG(WARN, "fetch ctx fail session_descriptor=%u", session_descriptor);
        ret = OB_ERR_UNEXPECTED;
      }
      else if (session_ctx->is_session_expired())
      {
        session_mgr_.revert_ctx(session_descriptor);
        session_ctx = NULL;
        ret = OB_TRANS_ROLLBACKED;
      }
      else
      {
        FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "start handle scan, packet wait=%ld start_time=%ld timeout=%ld src=%s",
                      tbsys::CTimeUtil::getTime() - pkt.get_receive_ts(),
                      pkt.get_receive_ts(),
                      pkt.get_source_timeout(),
                      NULL == pkt.get_request() ? NULL : get_peer_ip(pkt.get_request()));
        thread_read_prepare();
        session_ctx->set_start_handle_time(tbsys::CTimeUtil::getTime());
        session_ctx->set_stmt_start_time(pkt.get_receive_ts());
        session_ctx->set_stmt_timeout(process_timeout);
        session_ctx->set_priority((PriorityPacketQueueThread::QueuePriority)pkt.get_packet_priority());
        // add by guojinwei [repeatable read] 20160418:b
        TBSYS_LOG(DEBUG, "ISOLATION_LEVEL = %d, guojinwei", scan_param.get_trans_id().isolation_level_);
        if (common::REPEATABLE_READ == scan_param.get_trans_id().isolation_level_
            && scan_param.get_trans_id().is_valid()
            && 0 != scan_param.get_trans_id().start_time_us_)
        {
          session_ctx->set_trans_start_time(scan_param.get_trans_id().trans_start_time_us_);
        }
        if (scan_param.get_trans_id().is_valid())
        {
          session_ctx->set_trans_descriptor(scan_param.get_trans_id().descriptor_);
        }
        // add:e
        if (OB_NEW_SCAN_REQUEST == pkt.get_packet_code())
        {
          new_scanner.reuse();
          common::ObRowDesc row_desc;
          if(OB_SUCCESS != (ret = ObNewScannerHelper::get_row_desc(scan_param, row_desc)))
          {
            TBSYS_LOG(WARN, "get row desc fail:ret[%d]", ret);
          }
          else
          {
            new_scanner.set_row_desc(row_desc);
            ret = UPS.get_table_mgr().new_scan(*session_ctx, scan_param, new_scanner, pkt.get_receive_ts(), process_timeout);
          }
#if 0
          if (OB_SUCCESS == ret)
          {
            ObUpsRow tmp_ups_row;
            tmp_ups_row.set_row_desc(row_desc);
            if (OB_SUCCESS != (ret = ObNewScannerHelper::print_new_scanner(new_scanner, tmp_ups_row, true)))
            {
              TBSYS_LOG(WARN, "print new scanner fail:ret[%d]", ret);
            }
          }
#endif
          if (OB_SUCCESS == ret)
          {
            UPS.response_scanner(ret, pkt, new_scanner, buffer);
          }
        }
        else
        {
          scanner.reset();
          ret = UPS.get_table_mgr().scan(*session_ctx, scan_param, scanner, pkt.get_receive_ts(), process_timeout);
          if (OB_SUCCESS == ret)
          {
            UPS.response_scanner(ret, pkt, scanner, buffer);
          }
        }
        FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "get from table mgr ret=%d", ret);
        //OB_STAT_INC(UPDATESERVER, UPS_STAT_NL_SCAN_COUNT, 1);
        OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), SCAN, COUNT), 1);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_SCAN_QTIME, session_ctx->get_start_handle_time() - session_ctx->get_session_start_time());
        //OB_STAT_INC(UPDATESERVER, UPS_STAT_NL_SCAN_TIMEU, session_ctx->get_session_timeu());
        OB_STAT_INC(UPDATESERVER, get_stat_num(session_ctx->get_priority(), SCAN, TIMEU), session_ctx->get_session_timeu());
        thread_read_complete();
        session_ctx->set_last_active_time(tbsys::CTimeUtil::getTime());
        session_mgr_.revert_ctx(session_descriptor);
        session_mgr_.end_session(session_descriptor);
        log_scan_qps_();
      }
      if (OB_SUCCESS != ret)
      {
        UPS.response_result(ret, pkt);
      }
    }

    void TransExecutor::handle_kill_zombie_()
    {
      const bool force = false;
      session_mgr_.kill_zombie_session(force);
    }

    void TransExecutor::handle_show_sessions_(ObPacket &pkt,
                                              ObNewScanner &scanner,
                                              ObDataBuffer &buffer)
    {
      scanner.reuse();
      session_mgr_.show_sessions(scanner);
      UPS.response_scanner(OB_SUCCESS, pkt, scanner, buffer);
    }

    void TransExecutor::handle_kill_session_(ObPacket &pkt)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      uint32_t session_descriptor = INVALID_SESSION_DESCRIPTOR;
      int64_t pos = pkt.get_buffer()->get_position();
      if (OB_SUCCESS != (ret = serialization::decode_vi32(pkt.get_buffer()->get_data(),
                                                          pkt.get_buffer()->get_capacity(),
                                                          pos,
                                                          (int32_t*)&session_descriptor)))
      {
        TBSYS_LOG(WARN, "deserialize session descriptor fail ret=%d", ret);
      }
      else
      {
        ret = session_mgr_.kill_session(session_descriptor);
        TBSYS_LOG(INFO, "kill session ret=%d sd=%u", ret, session_descriptor);
      }
      UPS.response_result(ret, pkt);
    }

    void *TransExecutor::on_trans_begin()
    {
      TransParamData *ret = NULL;
      void *buffer = ob_malloc(sizeof(TransParamData), ObModIds::OB_UPS_PARAM_DATA);
      if (NULL != buffer)
      {
        ret = new(buffer) TransParamData();
        ret->buffer.set_data(ret->cbuffer, sizeof(ret->cbuffer));
      }
      return ret;
    }

    void TransExecutor::on_trans_end(void *ptr)
    {
      if (NULL != ptr)
      {
        ob_free(ptr);
        ptr = NULL;
      }
    }

    void TransExecutor::log_scan_qps_()
    {
      static volatile uint64_t counter = 0;
      static const int64_t mod = 100000;
      static int64_t last_report_ts = 0;
      if (1 == (ATOMIC_ADD(&counter, 1) % mod))
      {
        int64_t cur_ts = tbsys::CTimeUtil::getTime();
        TBSYS_LOG(INFO, "SCAN total=%lu, SCAN_QPS=%ld", counter, 1000000 * mod/(cur_ts - last_report_ts));
        last_report_ts = cur_ts;
      }
    }

    void TransExecutor::log_get_qps_()
    {
      static volatile uint64_t counter = 0;
      static const int64_t mod = 100000;
      static int64_t last_report_ts = 0;
      if (1 == (ATOMIC_ADD(&counter, 1) % mod))
      {
        int64_t cur_ts = tbsys::CTimeUtil::getTime();
        TBSYS_LOG(INFO, "GET total=%lu, GET_QPS=%ld", counter, 1000000 * mod/(cur_ts - last_report_ts));
        last_report_ts = cur_ts;
      }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool TransExecutor::is_write_packet(ObPacket& pkt)
    {
      bool ret = false;
      switch(pkt.get_packet_code())
      {
        case OB_MS_MUTATE:
        case OB_WRITE:
        case OB_PHY_PLAN_EXECUTE:
        case OB_TRUNCATE_TABLE:  //add hxlong [truncate table] 20170403
        case OB_END_TRANSACTION:
          ret = true;
          break;
        default:
          ret = false;
      }
      return ret;
    }

    int64_t TransExecutor::get_commit_queue_len()
    {
      //modify by zhouhuan [scalablecommit] 20160511:b
      //return session_mgr_.get_trans_seq().get_seq() - TransCommitThread::task_queue_.get_seq();
      return TransCommitThread::seq_queue_.get_seq();
      //modify:e
    }

    int64_t TransExecutor::get_seq(void* ptr)
    {
      int64_t seq = 0;
      Task& task = *((Task*)ptr);
      int64_t trans_id = tbsys::CTimeUtil::getTime();
      int ret = OB_SUCCESS;
      seq = session_mgr_.get_trans_seq().next(trans_id);
      if (NULL == ptr)
      {
        TBSYS_LOG(ERROR, "commit queue, NULL ptr, will kill self");
        kill(getpid(), SIGTERM);
      }
      else if (is_write_packet(task.pkt))
      {
        {
          SessionGuard session_guard(session_mgr_, lock_mgr_, ret);
          RWSessionCtx* session_ctx = NULL;
          if (OB_SUCCESS != (ret = session_guard.fetch_session(task.sid, session_ctx)))
          {
            TBSYS_LOG(ERROR, "fetch_session(sid=%s)=>%d", to_cstring(task.sid), ret);
          }
          else if (!UPS.get_log_mgr().check_log_size(session_ctx->get_ups_mutator().get_serialize_size()))
          {
            ret = OB_LOG_TOO_LARGE;
            TBSYS_LOG(ERROR, "mutator.size[%ld] too large",
                      session_ctx->get_ups_mutator().get_serialize_size());
          }
          else if (session_ctx->is_frozen())
          {
            task.sid.reset();
            TBSYS_LOG(ERROR, "session stat is frozen, maybe request duplicate, sd=%u",
                      session_ctx->get_session_descriptor());
          }
          else
          {
            session_ctx->set_trans_id(trans_id);
            session_ctx->get_checksum();
          }
        }

        //add by zhouhuan [scalablecommit] 20160413:b
        if (OB_SUCCESS == ret && task.sid.is_valid()
            && OB_SUCCESS != (ret = session_mgr_.precommit(task.sid.descriptor_)))
        {
          TBSYS_LOG(ERROR, "precommit(%s)=>%d",to_cstring(task.sid), ret);
        }
        //add 20160413:e
        if (OB_SUCCESS != ret)
        {
          task.sid.reset();
        }
      }
      return seq;
    }

    void TransExecutor::handle_commit(void *ptask, void *pdata)
    {
      int ret = OB_SUCCESS;
      thread_errno() = OB_SUCCESS;
      bool release_task = true;
      Task *task = (Task*)ptask;
      CommitParamData *param = (CommitParamData*)pdata;
      if (NULL == task)
      {
        TBSYS_LOG(WARN, "null pointer task=%p", task);
      }
      else if (NULL == param)
      {
        TBSYS_LOG(WARN, "null pointer param data pdata=%p src=%s",
                  pdata, inet_ntoa_r(task->src_addr));
        ret = OB_ERR_UNEXPECTED;
      }
      else if (0 > task->pkt.get_packet_code()
              || OB_PACKET_NUM <= task->pkt.get_packet_code())
      {
        TBSYS_LOG(ERROR, "unknown packet code=%d src=%s",
                  task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
        ret = OB_UNKNOWN_PACKET;
      }
      else if (OB_SUCCESS != (ret = handle_flushed_log_()))
      {
        TBSYS_LOG(ERROR, "handle_flushed_clog()=>%d", ret);
      }
      else
      {
        //忽略log自带的前两个字段，trace id和chid，以后续的trace id和chid为准
        PROFILE_LOG(DEBUG, TRACE_ID SOURCE_CHANNEL_ID PCODE WAIT_TIME_US_IN_COMMIT_QUEUE,
                    (task->pkt).get_trace_id(),
                    (task->pkt).get_channel_id(),
                    (task->pkt).get_packet_code(),
                    tbsys::CTimeUtil::getTime() - (task->pkt).get_receive_ts());
        if (wait_for_commit_(task->pkt.get_packet_code()))
        {
          {
            ObSpinLockGuard guard(write_clog_mutex1_);
            commit_log_(); // 把日志缓存区刷盘，并且提交end_session和响应客户端的任务
          }
          if (is_only_master_can_handle(task->pkt.get_packet_code()))
          {
            FakeWriteGuard guard(session_mgr_);
            if (!guard.is_granted())
            {
              ret = OB_NOT_MASTER;
              TBSYS_LOG(WARN, "only master can handle pkt_code=%d", task->pkt.get_packet_code());
            }
            else
            {
              ObSpinLockGuard guard(write_clog_mutex1_);
              // 这个分支不能加FakeWriteGuard, 否则备机收日志就没不能处理了
              release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, *param);
            }
          }
          else
          {
            ObSpinLockGuard guard(write_clog_mutex1_);
            release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, *param);
          }
        }
        else
        {
          ObSpinLockGuard guard(write_clog_mutex1_);
          release_task = commit_handler_[task->pkt.get_packet_code()](*this, *task, *param);
        }
      }
      if (NULL != task)
      {
        if (OB_SUCCESS != ret
            || OB_SUCCESS != thread_errno())
        {
          TBSYS_LOG(WARN, "process fail ret=%d pcode=%d src=%s",
                    (OB_SUCCESS != ret) ? ret : thread_errno(), task->pkt.get_packet_code(), inet_ntoa_r(task->src_addr));
        }
        if (OB_SUCCESS != ret)
        {
          UPS.response_result(ret, task->pkt);
        }
        if (release_task)
        {
          allocator_.free(task);
          task = NULL;
        }
      }
    }

    //add hushuang [scalable commit] 20160410:b
    void TransExecutor::handle_private_task()
    {
      thread_errno() = OB_SUCCESS;

      // add:e
    }

    int TransExecutor::handle_flush_queue_task()
    {
      int ret = OB_SUCCESS;
      bool update_global_id = false;
      //FLogPos *pos = NULL;
      //LogGroup *task = NULL;
      Gtask *gtask = NULL;
      int64_t flush_seq = 0;
      int64_t flushed_clog_id = UPS.get_log_mgr().get_flushed_clog_id();
      while(true)
      {
        //TBSYS_LOG(ERROR, "test::whx in handle flush loop => %ld", flush_queue_.size());
        if(OB_SUCCESS != (ret = flush_queue_.tail(flush_seq, (void*&)gtask))
          && OB_ENTRY_NOT_EXIST != ret)
        {
          TBSYS_LOG(ERROR, "flush queue.tail()=>%d, will kill self", ret);
          kill(getpid(), SIGTERM);
        }
        else if(OB_ENTRY_NOT_EXIST == ret)
        {
          ret = OB_SUCCESS;
          break;
        }
        else
        {
          //TBSYS_LOG(ERROR, "test::zhouhuan handle flush queue flush_seq=%ld flushed_clog_id=%ld GLOBAL_LOG_ID=%ld",
            //        flush_seq, flushed_clog_id, OB_GLOBAL_SYNC_LOG_ID);
          if(NULL == gtask)
          {
            ret = OB_ERROR;
            break;
          }
          else if(flush_seq > flushed_clog_id)
          {
            break;
          }

          //add by zhouhuan for __all_server_stat 20160530
          int64_t cur_time = tbsys::CTimeUtil::getTime();
          OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_WPTIME, cur_time - gtask->get_last_proc_time());
         // OB_STAT_INC(UPDATESERVER, UPS_STAT_FTRANS_TIME, cur_time - gtask->get_first_fill_time());
          //OB_STAT_INC(UPDATESERVER, UPS_STAT_LTRANS_TIME, cur_time - gtask->get_last_fill_time());
//          TBSYS_LOG(INFO,"test::zhouhuan group_id = %ld, group_size = %ld ,len = %ld, rate = %lf, f_time = %ld, l_time = %ld", gtask->group_id, gtask->count,
//                     gtask->len, static_cast<double>(gtask->len)/static_cast<double>(1024*1024),
//                    cur_time - gtask->get_first_fill_time(), cur_time - gtask->get_last_fill_time());
          gtask->set_last_proc_time(cur_time);
          //add:e
          OB_GLOBAL_SYNC_LOG_ID = flush_seq;
          //TBSYS_LOG(ERROR,"test::zhouhuan update global log_id = %ld",OB_GLOBAL_SYNC_LOG_ID);
          __sync_synchronize();
          update_global_id = true;
          session_mgr_.update_published_trans_id(gtask->publish_trans_id);
          //TBSYS_LOG(ERROR, "test::zhouhuan:Group reuse!! group[%ld]", task->group_id_ );

          //add by zhouhuan for __all_server_stat 20160530
          OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_COUNT, 1);
          //add:e
          //add:e
          allocator_.free(gtask);
          gtask = NULL;
          //TBSYS_LOG(ERROR, "test::zhouhuan:Group reuse!! group[%ld]->ts_seq=[%ld]", task->group_id_, task->ts_seq_ );
          if(OB_SUCCESS != (ret = flush_queue_.pop()))
          {
            TBSYS_LOG(ERROR, "flush_queue.consume_tail()=>%d", ret);
          }
          //TBSYS_LOG(ERROR, "test::whx in flush_queue.pop log_id => %ld size=%ld group_id=%ld",
                   // task->start_log_id_ + task->count_, flush_queue_.size(), task->group_id_);
        }
      }
      if( update_global_id )
      {
        int64_t cb_time = tbsys::CTimeUtil::getTime();
        for(int64_t i = 0; i < TransHandlePool::get_thread_num(); ++i)
        {
          if( commit_queue_[i].get_total() != 0 )
          {
            TransHandlePool::wake_up(i);
          }
        }
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_CBTIME, cur_time - cb_time);

      }
      //TBSYS_LOG(ERROR, "test::whx in handle flush queue task flush_queue.size => %ld", flush_queue_.size());
      return ret;
    }

    //add by hushuang [scalablecommit] 20160601
    int TransExecutor::push_flush_queue(LogGroup *group)
    {

       int ret = OB_SUCCESS;
       int64_t push_time = tbsys::CTimeUtil::getTime();
       int64_t gtask_size = sizeof(Gtask);
       Gtask *gtask = (Gtask *)allocator_.alloc(gtask_size);
       if(group == NULL)
       {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "null pointer");
       }
       else
       {
          if(group->need_ack_)
          {
            //TBSYS_LOG(ERROR, "test::zhouhuan in push_flush_queue group_id=[%ld]",group->group_id_);
            int64_t cur_time = tbsys::CTimeUtil::getTime();
            OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_PUTIME, cur_time - push_time);
            gtask->set_last_proc_time(cur_time);
            gtask->set_first_fill_time(group->get_first_fill_time());
            gtask->set_last_fill_time(group->get_last_fill_time());
            gtask->publish_trans_id = group->start_timestamp_ + group->count_ - 1;
            gtask->group_id = group->group_id_;
            gtask->count = group->count_;
            gtask->len = group->len_;
            __sync_synchronize();
            if(OB_SUCCESS != (ret = flush_queue_.push(group->start_log_id_ + group->count_ - 1, gtask)))
            {
              TBSYS_LOG(WARN, "push grtask in flush queue failed => %d", ret);
            }

//            TBSYS_LOG(ERROR, "test::zhouhuan push need ack, group.size=[%ld] push_log_id=[%ld] group_id=%ld",
//                      group->count_, group->start_log_id_+group->count_, group->group_id_);
          }
       }
       return ret;
    }

    int TransExecutor::commit_group_log(LogGroup *group)
    {
      int ret = OB_SUCCESS;
      CLEAR_TRACE_BUF(TraceLog::get_logbuffer());
      int64_t flush_time = tbsys::CTimeUtil::getTime();
      //TBSYS_LOG(INFO, "test::zhouhuan commit group log group_id=%ld  sync_to_slave=%d", group->group_id_, group->sync_to_slave_);
      if(NULL == group)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "null pointer");
      }
      else
      {
        while(true)
        {
          if (group->log_cursor_seq_ != group->group_id_ + group->READY)
          {
            usleep(1);
          }
          else
          {
            if(group->sync_to_slave_)
            {
              ret = UPS.get_log_mgr().flush_log(group);
              if(OB_SUCCESS == ret)
              {
                --commit_task_num_;
              }
              //TBSYS_LOG(ERROR,"test::zhouhuan:flush_log start=>%ld",group->ts_seq_);
            }
            else
            {
              //TBSYS_LOG(ERROR,"test::zhouhuan:async_flush_log start!");
              //ret = UPS.get_log_mgr().async_flush_log(group);
              ret = commit_log_(group);
            }
          }
          break;
        }
      }

      int64_t cur_time = tbsys::CTimeUtil::getTime();
      OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_FLTIME, cur_time - flush_time);

      return ret;
    }

    void TransExecutor::handle_group(void *ptask, void *pdata)
    {
      int ret = OB_SUCCESS;
      UNUSED(pdata);
      thread_errno() = OB_SUCCESS;

      LogGroup *group = (LogGroup*)ptask;
      //LogGroup *group = UPS.get_log_mgr().get_log_group(pos->group_id_);
      //TBSYS_LOG(ERROR, "test::zhouhuan handle group: handle_group:  group_id=%ld, boolean = %d, log_id = %ld"
        //       , group->group_id_, group->need_ack_, group->start_log_id_ + group->count_);
      //TBSYS_LOG(INFO, "test::zhouhuan handle group: handle_group:  group_id=%ld", group->group_id_);
      if((int64_t)OB_INVALID_INDEX == group->group_id_)
      {
        int64_t ng_time = tbsys::CTimeUtil::getTime();
        if(OB_SUCCESS != (ret = handle_flush_queue_task()))
        {
          TBSYS_LOG(ERROR, "handle_flush_queue_task=>%d", ret);
        }
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_NGTIME, cur_time - ng_time);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_NGCOUNT, 1);
      }
      else if (NULL != group)
      {
        //add by zhouhuan for __all_server_stat 20160530
//        TBSYS_LOG(INFO, "test::zhouhuan check group usage :group_id = %ld, len = %ld rate = %lf", group->group_id_, group->len_,
//                  static_cast<double>(group->len_)/static_cast<double>(1024*1024));
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_CTIME, cur_time - group->get_last_proc_time());
        group->set_last_proc_time(cur_time);
        //add:e
        int64_t cg_time = tbsys::CTimeUtil::getTime();
        ++commit_task_num_;
        if(OB_SUCCESS != (ret = handle_flush_queue_task()))
        {
          TBSYS_LOG(ERROR, "handle_flush_queue_task=>%d", ret);
        }
        else if(OB_SUCCESS != (ret = push_flush_queue(group)))
        {
          TBSYS_LOG(WARN, "push group in flush queue failed => %d", ret);
        }
        else if(OB_SUCCESS != (ret = commit_group_log(group)))
        {
          TBSYS_LOG(WARN, "commit group log failed, ret=>%d, sys_slave =>%d", ret, group->sync_to_slave_);
        }
        cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_CGTIME, cur_time - cg_time);
        OB_STAT_INC(UPDATESERVER, UPS_STAT_COMMIT_CGCOUNT, 1);

        //TBSYS_LOG(ERROR, "test::whx after group_id = %ld, boolean = %d, log_id = %ld", group->group_id_, group->need_ack_,
          //        group->start_log_id_ + group->count_);
      }
    }


    //add e
    int TransExecutor::handle_write_commit_(Task &task)
    {
      int &ret = thread_errno();
      ret = OB_SUCCESS;
      if (!task.sid.is_valid())
      {
        ret = OB_TRANS_ROLLBACKED;
        TBSYS_LOG(WARN, "session is rollbacked, maybe precommit faile");
      }
      else
      {
        SessionGuard session_guard(session_mgr_, lock_mgr_, ret);
        RWSessionCtx* session_ctx = NULL;
        if (OB_SUCCESS != (ret = session_guard.fetch_session(task.sid, session_ctx)))
        {
          TBSYS_LOG(ERROR, "fetch_session(sid=%s)=>%d", to_cstring(task.sid), ret);
        }
        else if (OB_SUCCESS != (ret = session_mgr_.update_commited_trans_id(session_ctx)))
        {
          TBSYS_LOG(ERROR, "session_mgr.update_commited_trans_id(%s)=>%d", to_cstring(*session_ctx), ret);
        }
        else
        {
		//delete by zhouhuan [scalablecommit]
//          if (0 != session_ctx->get_last_proc_time())
//          {
//            int64_t cur_time = tbsys::CTimeUtil::getTime();
//            OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_CTIME, cur_time - session_ctx->get_last_proc_time());
//            session_ctx->set_last_proc_time(cur_time);
//          }

          batch_start_time() = (0 == batch_start_time()) ? tbsys::CTimeUtil::getTime() : batch_start_time();
          int64_t cur_timestamp = session_ctx->get_trans_id();
          session_ctx->get_uc_info().uc_checksum = ob_crc64(session_ctx->get_uc_info().uc_checksum, &cur_timestamp, sizeof(cur_timestamp));
          if (cur_timestamp <= 0)
          {
            TBSYS_LOG(ERROR, "session_ctx.trans_id=%ld <= 0, will kill self", cur_timestamp);
            kill(getpid(), SIGTERM);
          }
          FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "trans_checksum=%lu trans_id=%ld",
                         session_ctx->get_uc_info().uc_checksum, cur_timestamp);
          int ret = fill_log_(task, *session_ctx);
          if (OB_SUCCESS != ret)
          {
            if (OB_EAGAIN != ret)
            {
              TBSYS_LOG(WARN, "fill log fail ret=%d %s", ret, to_cstring(task.sid));
            }
            else if (OB_SUCCESS != (ret = commit_log_()))
            {
              TBSYS_LOG(WARN, "commit log fail ret=%d %s", ret, to_cstring(task.sid));
            }
            else if (OB_SUCCESS != (ret = fill_log_(task, *session_ctx)))
            {
              TBSYS_LOG(ERROR, "second fill log fail ret=%d %s serialize_size=%ld uncommited_number=%ld",
                        ret, to_cstring(task.sid),
                        session_ctx->get_ups_mutator().get_serialize_size(),
                        uncommited_session_list_.size());
            }
            else
            {
              TBSYS_LOG(INFO, "second fill log succ %s", to_cstring(task.sid));
            }
          }
        }
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(ERROR, "unexpect error, ret=%d %s, will kill self", ret, to_cstring(task.sid));
          kill(getpid(), SIGTERM);
        }
      }
      // add by guojinwei [log synchronization][multi_cluster] 20151028:b
      int64_t cur_time_us = tbsys::CTimeUtil::getTime();
      // add:e
      if (OB_SUCCESS == ret
      // modify by guojinwei [log synchronization][multi_cluster] 20151028:b
      //    && (0 == TransCommitThread::get_queued_num()
      //        || MAX_BATCH_NUM <= uncommited_session_list_.size()))
          && (0 == TransCommitThread::get_queued_num())
          && ((message_residence_time_us_<message_residence_max_us_?
                (message_residence_time_us_+message_residence_protection_us_):message_residence_max_us_) 
              <= (cur_time_us - last_commit_log_time_us_)))
      // modify:e
      {
        // add by guojinwei [log synchronization][multi_cluster] 20151028:b
        last_commit_log_time_us_ = tbsys::CTimeUtil::getTime();
        // add:e
        ret = commit_log_();
      }
      if (OB_SUCCESS != ret)
      {
        UPS.response_result(ret, task.pkt);
      }
      return ret;
    }

    //mod by zhouhuan for [scalablecommit] 20160822:b
    void TransExecutor::on_commit_idle()
    {
      //commit_log_();
      //handle_flushed_log_();
      //try_submit_auto_freeze_();
      if (0 < flush_queue_.size())
      {
        handle_flush_queue_task();
      }
    }

    int TransExecutor::handle_response(ObAckQueue::WaitNode& node)
    {
      int ret = OB_SUCCESS;
      TBSYS_LOG(TRACE, "send_log response: %s", to_cstring(node));
      if (OB_SUCCESS != node.err_)
      {
        TBSYS_LOG(WARN, "send_log %s faile", to_cstring(node));
        UPS.get_slave_mgr().delete_server(node.server_);
      }
      else
      {
        UPS.get_log_mgr().get_clog_stat().add_net_us(node.start_seq_, node.end_seq_, node.get_delay());
        // add by guojinwei [log synchronization][multi_cluster] 20151028:b
        message_residence_time_us_ = (message_residence_time_us_ >> 1) + (node.message_residence_time_us_ >> 1);
//        TBSYS_LOG(INFO,"test::zhouhuan handle_response message_residence_time=[%ld], node.message_redidence_time_us=[%ld]",
//                  message_residence_time_us_, node.message_residence_time_us_);
        // add:e
      }
      return ret;
    }

    //modify by zhouhuan [scalablecommit] 20160520:b
    int TransExecutor::on_ack(ObAckQueue::WaitNode& node)
    {
      int ret = OB_SUCCESS;
      TBSYS_LOG(TRACE, "on_ack: %s", to_cstring(node));
      //modify by zhouhuan [scalablecommit] 20160503:b
      //if (OB_SUCCESS != (ret = TransCommitThread::push(&nop_task_)))
      if (OB_SUCCESS != (ret = TransCommitThread::push(nop_pos_.group_id_, &nop_pos_)))
      {
        //TBSYS_LOG(ERROR, "push nop_task to wakeup commit thread fail, %s, ret=%d", to_cstring(node), ret);
        TBSYS_LOG(ERROR, "push nop_task to wakeup handle thread fail, %s, ret=%d", to_cstring(node), ret);
      }
      //modify:e
      return ret;
    }

    int TransExecutor::handle_flushed_log_()
    {
      int err = OB_SUCCESS;
      int64_t flushed_clog_id = UPS.get_log_mgr().get_flushed_clog_id();
      int64_t flush_seq = 0;
      Task *task = NULL;
       //delete chujiajia [log synchronization][multi_cluster] 20160625:b
      // add by guojinwei [commit point for log replay][multi_cluster] 20151127:b
      //int64_t last_commit_point = 0;
      //if (OB_SUCCESS != (err = UPS.get_log_mgr().get_last_commit_point(last_commit_point)))
      //{
     // TBSYS_LOG(WARN, "fail to get last commit point, err=%d", err);
     // }
      //else if ((common::OB_COMMIT_POINT_ASYNC != UPS.get_param().commit_point_sync_type) 
      //         && (flushed_clog_id > last_commit_point))
     // {
     //   TBSYS_LOG(DEBUG, "sync flush commit point, flushed_clog_id=%ld", flushed_clog_id);  // test
     //   UPS.get_log_mgr().flush_commit_point(flushed_clog_id);
     // }
      // add:e
	  //delete:e
      while(true)
      {
        if (OB_SUCCESS != (err = flush_queue_.tail(flush_seq, (void*&)task))
            && OB_ENTRY_NOT_EXIST != err)
        {
          TBSYS_LOG(ERROR, "flush_queue_.tail()=>%d, will kill self", err);
          kill(getpid(), SIGTERM);
        }
        else if (OB_ENTRY_NOT_EXIST == err)
        {
          err = OB_SUCCESS;
          break;
        }
        else if (flush_seq > flushed_clog_id)
        {
          break;
        }
        else
        {
          int ret_ok = OB_SUCCESS;
          SessionGuard session_guard(session_mgr_, lock_mgr_, ret_ok);
          RWSessionCtx *session_ctx = NULL;
          if (OB_SUCCESS != (err = session_guard.fetch_session(task->sid, session_ctx)))
          {
            TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", err, to_cstring(task->sid));
            kill(getpid(), SIGTERM);
          }
          else
          {
            task->pkt.set_packet_code(OB_COMMIT_END);
            session_guard.revert();
            session_mgr_.end_session(session_ctx->get_session_descriptor(), false, true, BaseSessionCtx::ES_PUBLISH);
            if (OB_SUCCESS != (err = CommitEndHandlePool::push(task)))
            {
              TBSYS_LOG(ERROR, "push(task=%p)=>%d, will kill self", task, err);
              kill(getpid(), SIGTERM);
            }
            else if (OB_SUCCESS != (err = flush_queue_.pop()))
            {
              TBSYS_LOG(ERROR, "flush_queue.consume_tail()=>%d", err);
            }
          }
        }
      }
      return err;
    }

    int TransExecutor::fill_log_(Task &task, RWSessionCtx &session_ctx)
    {
      int ret = OB_SUCCESS;
      MemTable *mt = NULL;
      if (0 != session_ctx.get_ups_mutator().get_mutator().size()
          || (OB_PHY_PLAN_EXECUTE != task.pkt.get_packet_code()
              && OB_END_TRANSACTION != task.pkt.get_packet_code()))
      {
        if (NULL == (mt = session_ctx.get_uc_info().host))
        {
          ret = OB_ERR_UNEXPECTED;
        }
        else
        {
          int64_t uc_checksum = 0;
          uc_checksum = mt->calc_uncommited_checksum(session_ctx.get_uc_info().uc_checksum);
          int64_t old = uc_checksum;
          uc_checksum ^= session_ctx.get_checksum();
          session_ctx.get_ups_mutator().set_mutate_timestamp(session_ctx.get_trans_id());
          session_ctx.get_ups_mutator().set_memtable_checksum_before_mutate(mt->get_uncommited_checksum());
          session_ctx.get_ups_mutator().set_memtable_checksum_after_mutate(uc_checksum);
          uc_checksum = old;

          ret = UPS.get_table_mgr().fill_commit_log(session_ctx.get_ups_mutator(), session_ctx.get_tlog_buffer());
          if (OB_SUCCESS == ret)
          {
            session_ctx.get_uc_info().uc_checksum = uc_checksum;
            mt->update_uncommited_checksum(session_ctx.get_uc_info().uc_checksum);
          }
        }
      }
      else
      {
        session_ctx.get_uc_info().host = NULL;
      }
      if (OB_SUCCESS == ret)
      {
        ObLogCursor filled_cursor;
        if (OB_SUCCESS != (ret = UPS.get_log_mgr().get_filled_cursor(filled_cursor)))
        {
          TBSYS_LOG(ERROR, "get_fill_cursor()=>%d", ret);
        }
        else if (0 != flush_queue_.push(filled_cursor.log_id_, &task))
        {
          ret = (OB_SUCCESS == ret) ? OB_MEM_OVERFLOW : ret;
          TBSYS_LOG(ERROR, "unexpected push task to uncommited_session_list fail list_size=%ld, will kill self", uncommited_session_list_.size());
          kill(getpid(), SIGTERM);
        }
        else
        {
          // 保证在flush commit log成功后不会被kill掉
          session_ctx.set_frozen();
        }
      }
      FILL_TRACE_BUF(session_ctx.get_tlog_buffer(), "checksum=%lu affected_rows=%ld ret=%d",
                    (NULL == mt) ? 0 : mt->get_uncommited_checksum(),
                    session_ctx.get_ups_result().get_affected_rows(), ret);
      return ret;
    }

	//modify by zhouhuan [scalablecommit]
    int TransExecutor::handle_commit_end_(Task &task, ObDataBuffer &buffer)
    {
      int ret = OB_SUCCESS;
      while (true)
      {
        if (OB_GLOBAL_SYNC_LOG_ID >= task.get_seq())
        {
          {
            int ret_ok = OB_SUCCESS;
            SessionGuard session_guard(session_mgr_, lock_mgr_, ret_ok);
            RWSessionCtx *session_ctx = NULL;
            if (OB_SUCCESS != (ret = session_guard.fetch_session(task.sid, session_ctx)))
            {
              TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", ret, to_cstring(task.sid));
              kill(getpid(), SIGTERM);
            }
            else
            {
              //add by zhouhuan for __all_server_stat 20160530
              if (0 != session_ctx->get_last_proc_time())
              {
                int64_t cur_time = tbsys::CTimeUtil::getTime();
                OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_WPTIME, cur_time - session_ctx->get_last_proc_time());
                //TBSYS_LOG(INFO,"test::zhouhuan log_id = %ld, wp_time = %ld",task.log_id_, cur_time - session_ctx->get_last_proc_time());
                session_ctx->set_last_proc_time(cur_time);
              }
              //add:e
              session_ctx->get_ups_result().serialize(buffer.get_data(),
                                                      buffer.get_capacity(),
                                                      buffer.get_position());
            }
          }
          bool rollback = false;
          if (OB_SUCCESS != ret)
          {}
          else if (OB_SUCCESS != (ret = session_mgr_.end_session(task.sid.descriptor_, rollback, true, BaseSessionCtx::ES_PUBLISH)))
          {
            TBSYS_LOG(ERROR, "unexpected end_session fail ret=%d %s, will kill self", ret, to_cstring(task.sid));
            kill(getpid(), SIGTERM);
          }
          else if (OB_SUCCESS != (ret = session_mgr_.end_session(task.sid.descriptor_, rollback, true, BaseSessionCtx::ES_FREE)))
          {
            TBSYS_LOG(ERROR, "unexpected end_session fail ret=%d %s, will kill self", ret, to_cstring(task.sid));
            kill(getpid(), SIGTERM);
          }
          if (OB_SUCCESS == ret)
          {
            UPS.response_buffer(ret, task.pkt, buffer); // phyplan的话不能多线程执行
          }
          else
          {
            UPS.response_result(ret, task.pkt);
          }
          break;
        }
        else
        {
          usleep(1);
        }
      }
      return ret;
    }

    int TransExecutor::commit_log_()
    {
      int ret = OB_SUCCESS;
      //int64_t end_log_id = 0;
      if (0 < flush_queue_.size())
      {
        CLEAR_TRACE_BUF(TraceLog::get_logbuffer());
        //ret = UPS.get_log_mgr().async_flush_log(end_log_id, TraceLog::get_logbuffer());
        /*
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(ERROR, "flush commit log fail ret=%d uncommited_number=%ld, will kill self", ret, uncommited_session_list_.size());
          kill(getpid(), SIGTERM);
        }
        TBSYS_TRACE_LOG("[GROUP_COMMIT] %s", TraceLog::get_logbuffer().buffer);
        bool rollback = (OB_SUCCESS != ret);
        int64_t i = 0;
        ObList<Task*>::iterator iter;
        for (iter = uncommited_session_list_.begin(); iter != uncommited_session_list_.end(); iter++, i++)
        {
          Task *task = *iter;
          task->pkt.set_packet_code(OB_COMMIT_END);
          if (OB_SUCCESS != (ret = CommitEndHandlePool::push(task)))
          {
            TBSYS_LOG(ERROR, "push(task=%p)=>%d, will kill self", task, ret);
            kill(getpid(), SIGTERM);
          }
          continue;
          if (NULL == task)
          {
            TBSYS_LOG(ERROR, "unexpected task null pointer batch=%ld, will kill self", uncommited_session_list_.size());
            kill(getpid(), SIGTERM);
          }
          else
          {
            {
              int ret_ok = OB_SUCCESS;
              SessionGuard session_guard(session_mgr_, lock_mgr_, ret_ok);
              RWSessionCtx *session_ctx = NULL;
              if (OB_SUCCESS != (ret = session_guard.fetch_session(task->sid, session_ctx)))
              {
                TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", ret, to_cstring(task->sid));
                kill(getpid(), SIGTERM);
              }
              else
              {
                FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "%sbatch=%ld:%ld", TraceLog::get_logbuffer().buffer, i, uncommited_session_list_.size());
                ups_result_buffer_.set_data(ups_result_memory_, OB_MAX_PACKET_LENGTH);
                session_ctx->get_ups_result().serialize(ups_result_buffer_.get_data(),
                                                        ups_result_buffer_.get_capacity(),
                                                        ups_result_buffer_.get_position());
              }
            }
            if (OB_SUCCESS != (ret = session_mgr_.end_session(task->sid.descriptor_, rollback)))
            {
              TBSYS_LOG(ERROR, "unexpected end_session fail ret=%d %s, will kill self", ret, to_cstring(task->sid));
              kill(getpid(), SIGTERM);
            }
            ret = rollback ? OB_TRANS_ROLLBACKED : ret;
            if (OB_PHY_PLAN_EXECUTE == task->pkt.get_packet_code()
                && OB_SUCCESS == ret)
            {
              UPS.response_buffer(ret, task->pkt, ups_result_buffer_);
            }
            else
            {
              UPS.response_result(ret, task->pkt);
            }
            allocator_.free(task);
            task = NULL;
          }
        }
        uncommited_session_list_.clear(); */
        // debug by guojinwei [inner table error][multi_cluster] 20150919:b
        if (0 != batch_start_time())
        {
        // debug:e
          OB_STAT_INC(UPDATESERVER, UPS_STAT_BATCH_COUNT, 1);
          OB_STAT_INC(UPDATESERVER, UPS_STAT_BATCH_TIMEU, tbsys::CTimeUtil::getTime() - batch_start_time());
          batch_start_time() = 0;
        // debug by guojinwei [inner table error][multi_custer] 20150919:b
        }
        // debug:e
      }
      try_submit_auto_freeze_();
      return ret;
    }

    //add by zhouhuan for [scalablecommit] 20160822:b
    int TransExecutor::commit_log_(LogGroup* cur_group)
    {
      int ret = OB_SUCCESS;
      if (0 < flush_queue_.size())
      {
        CLEAR_TRACE_BUF(TraceLog::get_logbuffer());
        ret = UPS.get_log_mgr().async_flush_log(cur_group, TraceLog::get_logbuffer());
        if(OB_SUCCESS == ret)
        {
          --commit_task_num_;
        }
        // debug by guojinwei [inner table error][multi_cluster] 20150919:b
        if (0 != batch_start_time())
        {
        // debug:e
          OB_STAT_INC(UPDATESERVER, UPS_STAT_BATCH_COUNT, 1);
          OB_STAT_INC(UPDATESERVER, UPS_STAT_BATCH_TIMEU, tbsys::CTimeUtil::getTime() - batch_start_time());
          batch_start_time() = 0;
        // debug by guojinwei [inner table error][multi_custer] 20150919:b
        }
        // debug:e
      }
      try_submit_auto_freeze_();
      return ret;
    }

    void TransExecutor::try_submit_auto_freeze_()
    {
      int err = OB_SUCCESS;
      static int64_t last_try_freeze_time = 0;
      if (TRY_FREEZE_INTERVAL < (tbsys::CTimeUtil::getTime() - last_try_freeze_time)
          && UPS.get_table_mgr().need_auto_freeze())
      {
        int64_t cur_ts = tbsys::CTimeUtil::getTime();
        if (OB_SUCCESS != (err = UPS.submit_auto_freeze()))
        {
          TBSYS_LOG(WARN, "submit_auto_freeze()=>%d", err);
        }
        else
        {
          TBSYS_LOG(INFO, "submit async auto freeze task, last_ts=%ld, cur_ts=%ld", last_try_freeze_time, cur_ts);
          last_try_freeze_time = cur_ts;
        }
      }
    }

    void *TransExecutor::on_commit_begin()
    {
      CommitParamData *ret = NULL;
      void *buffer = ob_malloc(sizeof(CommitParamData), ObModIds::OB_UPS_PARAM_DATA);
      if (NULL != buffer)
      {
        ret = new(buffer) CommitParamData();
        ret->buffer.set_data(ret->cbuffer, sizeof(ret->cbuffer));
      }
      return ret;
    }

    void TransExecutor::on_commit_end(void *ptr)
    {
      if (NULL != ptr)
      {
        ob_free(ptr);
        ptr = NULL;
      }
    }

    void TransExecutor::log_trans_info() const
    {
      TBSYS_LOG(INFO, "==========log trans executor start==========");
      TBSYS_LOG(INFO, "allocator info hold=%ld allocated=%ld", allocator_.hold(), allocator_.allocated());
      TBSYS_LOG(INFO, "session_mgr info flying session num ro=%ld rp=%ld rw=%ld",
                session_mgr_.get_flying_rosession_num(),
                session_mgr_.get_flying_rpsession_num(),
                session_mgr_.get_flying_rwsession_num());
      TBSYS_LOG(INFO, "queued_num trans_thread=%ld commit_thread=%ld",
                TransHandlePool::get_queued_num(),
                TransCommitThread::get_queued_num());
      TBSYS_LOG(INFO, "==========log trans executor end==========");
    }

    int &TransExecutor::thread_errno()
    {
      static __thread int thread_errno = OB_SUCCESS;
      return thread_errno;
    }

    int64_t &TransExecutor::batch_start_time()
    {
      static __thread int64_t batch_start_time = 0;
      return batch_start_time;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    void TransExecutor::phandle_non_impl(ObPacket &pkt, ObDataBuffer &buffer)
    {
      UNUSED(buffer);
      TBSYS_LOG(ERROR, "packet code=%d no implement phandler", pkt.get_packet_code());
      UPS.response_result(OB_NOT_IMPLEMENT, pkt);
    }

    void TransExecutor::phandle_freeze_memtable(ObPacket &pkt, ObDataBuffer &buffer)
    {
      UPS.ups_freeze_memtable(pkt.get_api_version(),
                              &pkt,
                              buffer,
                              pkt.get_packet_code());
    }

    void TransExecutor::phandle_clear_active_memtable(ObPacket &pkt, ObDataBuffer &buffer)
    {
      UNUSED(buffer);
      UPS.ups_clear_active_memtable(pkt.get_api_version(),
                                    pkt.get_request(),
                                    pkt.get_channel_id());
    }

    void TransExecutor::phandle_check_cur_version(ObPacket &pkt, ObDataBuffer &buffer)
    {
      UNUSED(pkt);
      UNUSED(buffer);
      UPS.ups_check_cur_version();
    }

    void TransExecutor::phandle_check_sstable_checksum(ObPacket &pkt, ObDataBuffer &buffer)
    {
      UNUSED(pkt);
      UNUSED(buffer);
      UPS.ups_commit_check_sstable_checksum(*pkt.get_buffer());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool TransExecutor::thandle_non_impl(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      UNUSED(host);
      UNUSED(pdata);
      TBSYS_LOG(ERROR, "packet code=%d no implement thandler", task.pkt.get_packet_code());
      UPS.response_result(OB_NOT_IMPLEMENT, task.pkt);
      return true;
    }

    bool TransExecutor::thandle_commit_end(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      pdata.buffer.get_position() = 0;
      host.handle_commit_end_(task, pdata.buffer);
      return true;
    }

    bool TransExecutor::thandle_scan_trans(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      if (common::PACKET_RECORDER_FLAG)
      {
        host.fifo_stream_.push(&task.pkt);
      }
      pdata.buffer.get_position() = 0;
      host.handle_scan_trans_(task.pkt, pdata.scan_param, pdata.scanner, pdata.new_scanner, pdata.buffer);
      return true;
    }

    bool TransExecutor::thandle_get_trans(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      if (common::PACKET_RECORDER_FLAG)
      {
        host.fifo_stream_.push(&task.pkt);
      }
      pdata.buffer.get_position() = 0;
      host.handle_get_trans_(task.pkt, pdata.get_param, pdata.scanner, pdata.new_scanner, pdata.buffer);
      return true;
    }

    bool TransExecutor::thandle_write_trans(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      bool ret = true;
      pdata.buffer.get_position() = 0;
      if (OB_PHY_PLAN_EXECUTE == task.pkt.get_packet_code())
      {
        ret = host.handle_phyplan_trans_(task, pdata.phy_operator_factory, pdata.phy_plan, pdata.new_scanner, pdata.allocator, pdata.buffer);
      }
      else
      {
        ret = host.handle_write_trans_(task, pdata.mutator, pdata.new_scanner);
      }
      return ret;
    }

    bool TransExecutor::thandle_start_session(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      bool ret = true;
      pdata.buffer.get_position() = 0;
      ret = host.handle_start_session_(task, pdata.buffer);
      return ret;
    }

    bool TransExecutor::thandle_kill_zombie(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      UNUSED(task);
      UNUSED(pdata);
      host.handle_kill_zombie_();
      return true;
    }

    bool TransExecutor::thandle_show_sessions(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      pdata.buffer.get_position() = 0;
      host.handle_show_sessions_(task.pkt, pdata.new_scanner, pdata.buffer);
      return true;
    }

    bool TransExecutor::thandle_kill_session(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      UNUSED(pdata);
      host.handle_kill_session_(task.pkt);
      return true;
    }

    bool TransExecutor::thandle_end_session(TransExecutor &host, Task &task, TransParamData &pdata)
    {
      pdata.buffer.get_position() = 0;
      return host.handle_end_session_(task, pdata.buffer);
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    bool TransExecutor::chandle_non_impl(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      UNUSED(pdata);
      TBSYS_LOG(ERROR, "packet code=%d no implement chandler", task.pkt.get_packet_code());
      UPS.response_result(OB_NOT_IMPLEMENT, task.pkt);
      return true;
    }

    bool TransExecutor::chandle_write_commit(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(pdata);
      int ret = host.handle_write_commit_(task);
      return (OB_SUCCESS != ret);
    }

    bool TransExecutor::chandle_send_log(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      pdata.buffer.get_position() = 0;
      UPS.ups_slave_write_log(task.pkt.get_api_version(),
                              *(task.pkt.get_buffer()),
                              task.pkt.get_request(),
                              task.pkt.get_channel_id(),
                              pdata.buffer);
      return true;
    }

    bool TransExecutor::chandle_fake_write_for_keep_alive(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      UNUSED(task);
      UNUSED(pdata);
     // TBSYS_LOG(INFO, "test::zhouhuan:handle_fake_write_for_keep_alive start!");
      UPS.ups_handle_fake_write_for_keep_alive();
      return true;
    }

    bool TransExecutor::chandle_slave_reg(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      pdata.buffer.get_position() = 0;
      UPS.ups_slave_register(task.pkt.get_api_version(),
                            *(task.pkt.get_buffer()),
                            task.pkt.get_request(),
                            task.pkt.get_channel_id(),
                            pdata.buffer);
      return true;
    }

    bool TransExecutor::chandle_switch_schema(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      UNUSED(pdata);
      //TBSYS_LOG(INFO,"test::zhouhuan switch schema!!");
      UPS.ups_switch_schema(task.pkt.get_api_version(),
                            &(task.pkt),
                            *(task.pkt.get_buffer()));
      return true;
    }

    bool TransExecutor::chandle_force_fetch_schema(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      UNUSED(pdata);
      //TBSYS_LOG(INFO,"test::zhouhuan force fetch schema!!");
      UPS.ups_force_fetch_schema(task.pkt.get_api_version(),
                                 task.pkt.get_request(),
                                 task.pkt.get_channel_id());
      return true;
    }

    bool TransExecutor::chandle_switch_commit_log(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      pdata.buffer.get_position() = 0;
      //TBSYS_LOG(INFO,"test::zhouhuan switch commit log!!");
      UPS.ups_switch_commit_log(task.pkt.get_api_version(),
                                task.pkt.get_request(),
                                task.pkt.get_channel_id(),
                                pdata.buffer);
      return true;
    }

    bool TransExecutor::chandle_nop(TransExecutor &host, Task &task, CommitParamData &pdata)
    {
      UNUSED(host);
      UNUSED(task);
      UNUSED(pdata);
      //TBSYS_LOG(INFO, "handle nop");
      return false;
    }
    //add by hushuang [scalablecommit] 20160415
   /* TransExecutor::CommitTask::CommitTask():mgr_(NULL)
    {

    }

    TransExecutor::CommitTask::~CommitTask()
    {
      mgr_ = NULL;
    }

    int TransExecutor::CommitTask::end_session()
    {
      int err = OB_SUCCESS;
      int ret_ok = OB_SUCCESS;
      SessionGuard session_guard(mgr_, lock_mgr_, ret_ok);
      RWSessionCtx *session_ctx = NULL;
      if(OB_SUCCESS != (err = session_guard.fetch_session(sid_, session_ctx)))
      {
        TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", err, to_cstring(task->sid));
        kill(getpid(), SIGTERM);
      }
      else
      {
        session_guard.revert();
        err = session_mgr_.end_session(session_ctx->get_session_descriptor(), false, true, BaseSessionCtx::ES_PUBLISH);
      }
      if(OB_SUCCESS == err)
      {
        err = session_mgr_.end_session(session_ctx->get_session_descriptor(), false, true, )
      }
      return err;
    }

    int64_t TransExecutor::CommitTask::get_seq()
    {
      return sync_log_id_;
    }
    */
//    int TransExecutor::Task::end_session()
    int TransExecutor::task_end_session(void *ptask, void *pdata)
    {
      int err = OB_SUCCESS;
      int ret_ok = OB_SUCCESS;
      Task *task = (Task*) ptask;
      TransParamData *trans_data = (TransParamData *)(pdata);
      ObDataBuffer &buffer = trans_data->buffer;
      buffer.get_position() = 0;
      RWSessionCtx *session_ctx = NULL;

      {
        SessionGuard session_guard(session_mgr_, lock_mgr_, ret_ok);
        if(OB_SUCCESS != (err = session_guard.fetch_session(task->sid, session_ctx)))
        {
          TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", err, to_cstring(task->sid));
          kill(getpid(), SIGTERM);
        }
        else
        {
          //add by zhouhuan for __all_server_stat 20160530
          if (0 != session_ctx->get_last_proc_time())
          {
            int64_t cur_time = tbsys::CTimeUtil::getTime();
            OB_STAT_INC(UPDATESERVER, UPS_STAT_TRANS_WPTIME, cur_time - session_ctx->get_last_proc_time());
            //TBSYS_LOG(INFO,"test::zhouhuan log_id = %ld, wp_time = %ld",task->log_id_, cur_time - session_ctx->get_last_proc_time());
            session_ctx->set_last_proc_time(cur_time);
          }
          //add:e
          session_ctx->get_ups_result().serialize(buffer.get_data(), buffer.get_capacity(), buffer.get_position());
        }
      }

      if(OB_SUCCESS != err)
      {}
      else if (OB_SUCCESS != (err = session_mgr_.end_session(session_ctx->get_session_descriptor(), false, true, BaseSessionCtx::ES_PUBLISH)))
      {
        TBSYS_LOG(ERROR,"unexpected end_sseion(ES_PUBLISH) fail ret=%d %s, will kill self", err, to_cstring(task->sid));
        kill(getpid(), SIGTERM);
      }
      else if (OB_SUCCESS != (session_mgr_.end_session(session_ctx->get_session_descriptor(), false, true, BaseSessionCtx::ES_FREE)))
      {
        TBSYS_LOG(ERROR,"unexpected end_sseion(ES_FREE) fail ret=%d %s, will kill self", err, to_cstring(task->sid));
        kill(getpid(), SIGTERM);
      }

      if(OB_SUCCESS == err)
      {

        UPS.response_buffer(err, task->pkt, buffer);
      }
      else
      {
        //TBSYS_LOG(ERROR,"test::zhouhuan common task failed. response the err!! pcode=%d", task->pkt.get_packet_code());
        UPS.response_result(err, task->pkt);
      }
      allocator_.free(task);
      return err;
    }

    void TransExecutor::handle_commit_end(void *pdata)
    {
      int ret = OB_SUCCESS;
      ObCommitQueue &task_queue = commit_queue_[TransHandlePool::get_thread_index()];
      void *ptask = NULL;
      ICommitTask *task = NULL;
      while( task_queue.get_total() > 0 )
      {
        if(OB_SUCCESS != (ret = task_queue.next(ptask)))
        {
          TBSYS_LOG(ERROR, "failed to next data, %d",ret);
          break;
        }
        else
        {
          task = (ICommitTask*)ptask;
          //TBSYS_LOG(ERROR,"test::zhouhuan task_queue_.next()");
        }
        __sync_synchronize();
        //int64_t num = ATOMIC_INC(&response_id);
        //TBSYS_LOG(ERROR, "test::zhouhuan begin to end_task,global_log_id=[%ld]  task.log_id=[%ld] task_queue[%ld].size=[%ld]",
          //         OB_GLOBAL_SYNC_LOG_ID, task->get_seq(), TransHandlePool::get_thread_index(), task_queue.get_total());

        if( OB_GLOBAL_SYNC_LOG_ID >= task->get_seq() )
        {
          //int64_t num = ATOMIC_INC(&response_id);
          //TBSYS_LOG(ERROR, "test::zhouhuan begin to end_task,global_log_id=[%ld] > task.log_id=[%ld] size=%ld thread_no=%ld id=%ld",
          //           OB_GLOBAL_SYNC_LOG_ID, task->get_seq(),task_queue.get_total(), TransHandlePool::get_thread_index(), num);
          if( OB_SUCCESS != (ret = task_end_session(ptask, pdata)))
          {
            TBSYS_LOG(WARN, "end session failed, ret = %d", ret);
            break;
          }
          else if( OB_SUCCESS != (ret = task_queue.pop(ptask)))
          {
            TBSYS_LOG(WARN, "failed to pop task from the commit queue");
            break;
          }
        }
        else
        {
          break;
        }
      }
    }

    int TransExecutor::push_private_task(Task &task)
    {
      //int64_t num = ATOMIC_INC(&push_id);
      //TBSYS_LOG(ERROR,"test::zhouhuan push_private_task log_id = %ld CommitQueue[%ld].size=%ld",
        //        task.log_id_, TransHandlePool::get_thread_index(), commit_queue_[TransHandlePool::get_thread_index()].get_total());
      return commit_queue_[TransHandlePool::get_thread_index()].push(&task);
    }

//    int64_t TransExecutor::task_get_seq(void *ptask)
//    {
//      Task *task = (Task*) ptask;
//      return task->log_id_;
//    }

//    int64_t TransExecutor::Task::get_seq()
//    {
//      return log_id_;
//    }
    //add e

    //add chujiajia [log synchronization][multi_cluster] 20160606:b
    int TransExecutor::handle_uncommited_session_list_after_switch()
    {
      int ret =OB_SUCCESS;
      bool rollback = true;
      int64_t i = 0;
      ObList<Task*>::iterator iter;
      for (iter = uncommited_session_list_.begin(); iter != uncommited_session_list_.end(); iter++, i++)
      {
        Task *task = *iter;
        TBSYS_LOG(INFO, "task->sid[%d]", task->sid.descriptor_);
        task->pkt.set_packet_code(OB_COMMIT_END);
        if (OB_SUCCESS != (ret = CommitEndHandlePool::push(task)))
        {
          TBSYS_LOG(ERROR, "push(task=%p)=>%d, will kill self", task, ret);
          kill(getpid(), SIGTERM);
        }
        continue;
        if (NULL == task)
        {
          TBSYS_LOG(ERROR, "unexpected task null pointer batch=%ld, will kill self", uncommited_session_list_.size());
          kill(getpid(), SIGTERM);
        }
        else
        {
          int ret_ok = OB_SUCCESS;
          SessionGuard session_guard(session_mgr_, lock_mgr_, ret_ok);
          RWSessionCtx *session_ctx = NULL;
          if (OB_SUCCESS != (ret = session_guard.fetch_session(task->sid, session_ctx)))
          {
            TBSYS_LOG(ERROR, "unexpected fetch_session fail ret=%d %s, will kill self", ret, to_cstring(task->sid));
            kill(getpid(), SIGTERM);
          }
          else
          {
            FILL_TRACE_BUF(session_ctx->get_tlog_buffer(), "%sbatch=%ld:%ld", TraceLog::get_logbuffer().buffer, i, uncommited_session_list_.size());
            ups_result_buffer_.set_data(ups_result_memory_, OB_MAX_PACKET_LENGTH);
            session_ctx->get_ups_result().serialize(ups_result_buffer_.get_data(),
                                                    ups_result_buffer_.get_capacity(),
                                                    ups_result_buffer_.get_position());
          }
        }
        if (OB_SUCCESS != (ret = session_mgr_.end_session(task->sid.descriptor_, rollback)))
        {
           TBSYS_LOG(ERROR, "unexpected end_session fail ret=%d %s, will kill self", ret, to_cstring(task->sid));
           kill(getpid(), SIGTERM);
        }
        ret = rollback ? OB_TRANS_ROLLBACKED : ret;
        if (OB_PHY_PLAN_EXECUTE == task->pkt.get_packet_code()
            && OB_SUCCESS == ret)
        {
          UPS.response_buffer(ret, task->pkt, ups_result_buffer_);
        }
        else
        {
          UPS.response_result(ret, task->pkt);
        }
        allocator_.free(task);
        task = NULL;
      }
      uncommited_session_list_.clear();
      return ret;
    }
    //add:e
  }
}
