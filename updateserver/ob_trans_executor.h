/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_trans_executor.h
 * @brief TransExecutor
 *     modify by guojinwei: support multiple clusters for HA by
 *     adding or modifying some functions, member variables
 *     modify by hushuang: support transaction commit thread of
 *     scalable commit by adding or modifying some functions,
 *     member variables
       modify by zhouhuan: support set log position and fill 
	   in parallel by adding or modifying some functions and
	   member variables
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @author hushuang <51151500017@stu.ecnu.edu.cn>
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_07_25
 */
////===================================================================
 //
 // ob_trans_executor.h updateserver / Oceanbase
 //
 // Copyright (C) 2010, 2013 Taobao.com, Inc.
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

#ifndef  OCEANBASE_UPDATESERVER_TRANS_EXECUTOR_H_
#define  OCEANBASE_UPDATESERVER_TRANS_EXECUTOR_H_
#include "common/ob_define.h"
#include "common/ob_packet.h"
#include "common/ob_spin_lock.h"
#include "common/data_buffer.h"
#include "common/ob_mod_define.h"
#include "common/ob_queue_thread.h"
#include "common/ob_ack_queue.h"
#include "common/ob_ticket_queue.h"
#include "common/ob_fifo_stream.h"
#include "sql/ob_physical_plan.h"
#include "sql/ob_ups_result.h"
#include "ob_session_mgr.h"
#include "common/ob_fifo_allocator.h"
#include "ob_sessionctx_factory.h"
#include "ob_lock_mgr.h"
#include "ob_util_interface.h"
#include "ob_ups_phy_operator_factory.h"
#include "common/ob_log_generator2.h"
#include "common/ob_spin_rwlock.h"
#include "common/group_queue_thread.h" //add by hushuang [scalable commit] 20160329
#include "ob_switch_group_runnable.h"
//add hushuang[scalable commit]20160507
//#include "common/ob_commit_queue.h"

//add e
namespace oceanbase
{
  namespace updateserver
  {
    class TransHandlePool : public common::S2MQueueThread
    {
      public:
        TransHandlePool() : affinity_start_cpu_(-1), affinity_end_cpu_(-1) {};
        virtual ~TransHandlePool() {};
      public:
        volatile int64_t OB_GLOBAL_SYNC_LOG_ID;
      public:
        //modify hushuang [scalable commit]20150506
        /*void handle(void *ptask, void *pdata)
        {
          static volatile uint64_t cpu = 0;
          rebind_cpu(affinity_start_cpu_, affinity_end_cpu_, cpu);
          handle_trans(ptask, pdata);
        };*/
        void handle_local_task(void *pdata)
        {
          handle_commit_end(pdata);
        };

        void handle(void *ptask, void *pdata)
        {
          static volatile uint64_t cpu = 0;
          rebind_cpu(affinity_start_cpu_, affinity_end_cpu_, cpu);
          handle_trans(ptask, pdata);
        };
        //modify e
        void *on_begin()
        {
          return on_trans_begin();
        };
        void on_end(void *ptr)
        {
          on_trans_end(ptr);
        };
        void set_cpu_affinity(const int64_t start, const int64_t end)
        {
          affinity_start_cpu_ = start;
          affinity_end_cpu_ = end;
        };
      public:
        virtual void handle_trans(void *ptask, void *pdata) = 0;
        virtual void *on_trans_begin() = 0;
        virtual void on_trans_end(void *ptr) = 0;
        virtual void handle_commit_end(void *pdata) = 0;
      private:
        int64_t affinity_start_cpu_;
        int64_t affinity_end_cpu_;
    };

    class CommitEndHandlePool : public common::S2MQueueThread
    {
      public:
        CommitEndHandlePool() {};
        virtual ~CommitEndHandlePool() {};
      public:
        void handle(void *ptask, void *pdata)
        {
          handle_trans(ptask, pdata);
        };
        void *on_begin()
        {
          return on_trans_begin();
        };
        void on_end(void *ptr)
        {
          on_trans_end(ptr);
        };
      public:
        virtual void handle_trans(void *ptask, void *pdata) = 0;
        virtual void *on_trans_begin() = 0;
        virtual void on_trans_end(void *ptr) = 0;
    };

    //mod by hushuang [scalable commit] 20160407:b
    //class TransCommitThread : public SeqQueueThread
    class TransCommitThread : public GroupQueueThread
    {//mod:e
      public:
        TransCommitThread() : affinity_cpu_(-1) {};
        virtual ~TransCommitThread() {};
      public:
        void handle(void *ptask, void *pdata)
        {
          rebind_cpu(affinity_cpu_);
          handle_commit(ptask, pdata);
        };
        void *on_begin()
        {
          return on_commit_begin();
        };
        void on_end(void *ptr)
        {
          on_commit_end(ptr);
        };
        void on_idle()
        {
          on_commit_idle();
        };
        void on_push_fail(void* task)
        {
          on_commit_push_fail(task);
        }
        void set_cpu_affinity(const int64_t cpu)
        {
          affinity_cpu_ = cpu;
        };
        //add hushuang [scalable commit] 20160410:b
        void on_process(void *task, void *pdata)
        {
          rebind_cpu(affinity_cpu_);
          handle_group(task, pdata);
        }
        //add e
      public:
        virtual void handle_commit(void *ptask, void *pdata) = 0;
        //add hushuang [scalable commit] 20160410:b
        virtual void handle_group(void *ptask, void *pdata) = 0;
        //add e
        virtual void *on_commit_begin() = 0;
        virtual void on_commit_end(void *ptr) = 0;
        virtual void on_commit_push_fail(void* ptr) = 0;
        virtual void on_commit_idle() = 0;
        virtual int64_t get_seq(void* task) = 0;
      private:
        int64_t affinity_cpu_;
    };

    class TransExecutor : public TransHandlePool, public TransCommitThread, public CommitEndHandlePool, public IObAsyncClientCallback
    {
      struct TransParamData
      {
        TransParamData() : mod(common::ObModIds::OB_UPS_PHYPLAN_ALLOCATOR),
                           allocator(2 * OB_MAX_PACKET_LENGTH, mod)
                           //allocator(common::ModuleArena::DEFAULT_PAGE_SIZE, mod)
        {
        };
        common::ObMutator mutator;
        common::ObGetParam get_param;
        common::ObScanParam scan_param;
        common::ObScanner scanner;
        common::ObCellNewScanner new_scanner;
        ObUpsPhyOperatorFactory phy_operator_factory;
        sql::ObPhysicalPlan phy_plan;
        common::ModulePageAllocator mod;
        common::ModuleArena allocator;
        char cbuffer[OB_MAX_PACKET_LENGTH];
        common::ObDataBuffer buffer;
      };
      struct CommitParamData
      {
        CommitParamData()
        {
          buffer.set_data(cbuffer, OB_MAX_PACKET_LENGTH - 1);
        }
        char cbuffer[OB_MAX_PACKET_LENGTH];
        common::ObDataBuffer buffer;
      };
      //modify hushuang [scalablecommit] 20160415
     /*struct Task
      {
        common::ObPacket pkt;
        ObTransID sid;
        onev_addr_e src_addr;
        void reset()
        {
          sid.reset();
        };
      };*/
      struct Task : ICommitTask
      {
        common::ObPacket pkt;
        ObTransID sid;
        onev_addr_e src_addr;
        void reset()
        {
          sid.reset();
        };
      };

      //add by hushuang [scalablecommit] 20160601
      /**
      * @brief The Gtask struct
      * Gtask is designed for
      * reserving group's related information
      * and updating publish_trans_id
      */
      struct Gtask
      {
        int64_t publish_trans_id;
        int64_t last_proc_time_;
        int64_t first_fill_time_;
        int64_t last_fill_time_;
        int64_t group_id;
        int64_t count;
        int64_t len;
        Gtask():publish_trans_id(0),last_proc_time_(0), first_fill_time_(0),last_fill_time_(0),
                group_id(0), count(0), len(0)
        {

        }
        ~Gtask(){}
        void set_last_proc_time(const int64_t proc_time)
        {
          last_proc_time_ = proc_time;
        }

        int64_t get_last_proc_time() const
        {
          return last_proc_time_;
        }
        void set_first_fill_time(const int64_t cur_time)
        {
          first_fill_time_ =cur_time;
        }

        int64_t get_first_fill_time() const
        {
          return first_fill_time_;
        }
        void set_last_fill_time(const int64_t cur_time)
        {
          last_fill_time_ = cur_time;
        }

        int64_t get_last_fill_time() const
        {
          return last_fill_time_;
        }
      };

      //add:e
      //add by hushuang [scalablecommit] 20160415
      /*class CommitTask : ICommitTask
      {
        public:
          CommitTask();
          ~CommitTask();
          virtual int end_session();
          virtual int64_t get_seq();
        public:
          void reset()
          {
            sid.reset();
          }
          void set_mgr(SessionMgr *mgr){mgr_ = mgr;}
          void set_seq(int64_t log_id){sync_log_id_ = log_id;}
        private:
          //common::ObPacket pkt;
          ObTransID   sid_;
          onev_addr_e src_addr_;
          SessionMgr  *mgr_;
          int64_t     sync_log_id_;
      };*/
      //add e
      static const int64_t TASK_QUEUE_LIMIT = 100000;
      static const int64_t FINISH_THREAD_IDLE = 5000;
      static const int64_t ALLOCATOR_TOTAL_LIMIT = 10L * 1024L * 1024L * 1024L;
      static const int64_t ALLOCATOR_HOLD_LIMIT = ALLOCATOR_TOTAL_LIMIT / 2;
      static const int64_t ALLOCATOR_PAGE_SIZE = 4L * 1024L * 1024L;
      static const int64_t MAX_RO_NUM = 100000;
      static const int64_t MAX_RP_NUM = 10000;
      //static const int64_t MAX_RW_NUM = 40000;
      static const int64_t MAX_RW_NUM = 80000;
      static const int64_t MAX_LRW_NUM = 10000; // add a parameter by qx 20170314 for long transcation
      static const int64_t QUERY_TIMEOUT_RESERVE = 20000;
      static const int64_t TRY_FREEZE_INTERVAL = 1000000;
      static const int64_t MAX_BATCH_NUM = 1024;
      static const int64_t FLUSH_QUEUE_SIZE = 1024L * 1024L;
      //add by hushuang[scalable commit]20160507
      static const int64_t MAX_THREAD_NUM = 256;///< max threads numbers
      //static const int64_t GROUP_ARRAY_SIZE = 5;
      //add e
      typedef void (*packet_handler_pt)(common::ObPacket &pkt, common::ObDataBuffer &buffer);
      typedef bool (*trans_handler_pt)(TransExecutor &host, Task &task, TransParamData &pdata);
      typedef bool (*commit_handler_pt)(TransExecutor &host, Task &task, CommitParamData &pdata);
      public:
        TransExecutor(ObUtilInterface &ui);
        ~TransExecutor();
      public:
        int init(const int64_t trans_thread_num,
                const int64_t trans_thread_start_cpu,
                const int64_t trans_thread_end_cpu,
                const int64_t commit_thread_cpu);
                //const int64_t commit_end_thread_num);
                //modify by zhouhuan [scalablecommit] 20160427
        void destroy();
      public:
        void handle_packet(common::ObPacket &pkt);
        void handle_trans(void *ptask, void *pdata);
        void *on_trans_begin();
        void on_trans_end(void *ptr);

        void handle_commit(void *ptask, void *pdata);
        //add hushuang [scalable commit] 20160410:b
        /**
        * @brief transaction commit thread's main procedure.
        * @param[in] ptask group .
        * @param[in] pdata data.
        */
        virtual void handle_group(void *ptask, void *pdata);
        void handle_private_task();
        /**
        * @brief process flush queue task;
        * after sync log to slave update publish_trans_id and
        * wake processing threads to response clients
        */
        int handle_flush_queue_task();
        /**
        * @brief flush group to disk
        */
        int commit_group_log(LogGroup* group);
        /**
        * @brief pop task from private queue to publish
        * @param[in] pdata data
        */
        void handle_commit_end(void *pdata); //add by zhutao
        /**
        * @brief push task to corresponding private queue
        * @param[in] task
        * @return OB_SUCCESS if success.
        */
        int push_private_task(Task &task);
        /**
        * @brief push task that needs to response to client to flush queue
        * @param[in] group group
        * @return OB_SUCCESS if success.
        */
        int push_flush_queue(LogGroup *group);
        //add e
        void *on_commit_begin();
        void on_commit_push_fail(void* ptr);
        void on_commit_end(void *ptr);
        void on_commit_idle();
        int64_t get_seq(void* ptr);
        int64_t get_commit_queue_len();
        int handle_response(ObAckQueue::WaitNode& node);
        int on_ack(ObAckQueue::WaitNode& node);
        int handle_flushed_log_();

        SessionMgr &get_session_mgr() {return session_mgr_;};
        LockMgr &get_lock_mgr() {return lock_mgr_;};
        void log_trans_info() const;
        int &thread_errno();
        int64_t &batch_start_time();
        //add by zhouhuan [scalable commit] 20160711
        /**
        * @brief get_write_clog_mutex
        * @return write_clog_mutex_
        */
        SpinRWLock &get_write_clog_mutex() {return write_clog_mutex_;};
		ObSpinLock &get_spinlock() {return write_clog_mutex1_;};
        volatile int64_t &get_commit_task_num() {return commit_task_num_;};
        volatile int64_t &get_message_residence_time_us() {return message_residence_time_us_;};
        volatile int64_t &get_message_residence_protection_us() {return message_residence_protection_us_;};
        volatile int64_t &get_message_residence_max_us() {return message_residence_max_us_;};
        volatile int64_t &get_last_commit_log_time_us() {return last_commit_log_time_us_;};
        void set_last_commit_log_time_us(int64_t time) {last_commit_log_time_us_ = time;};
        int64_t get_flush_queue_size(){return flush_queue_.size();};
        /**
        * @brief push task from processing threads to commit thread
        * @param[in] cur_pos group that is next commit
        * @param[in] ref_cnt the numbers of group's current logs
        * @return OB_SUCCESS if success.
        */
        int push_commit_group(FLogPos& cur_pos, int64_t ref_cnt);
		//add chujiajia [log synchronization][multi_cluster] 20160606:b
        /**
         * @brief handle uncommited session list after master switch to slave
         * @return OB_SUCCESS if success
         */
        int handle_uncommited_session_list_after_switch();
        //add:e
      private:
        //add by zhouhuan [scalabecommit] 20160417:b
        /**
        * @brief judge whether task is special task
        * @param[in] pcode pcode
        * @return true if task were special task.
        */
        bool handle_in_wthread_(const int pcode);
        /**
        * @brief push task from processing threads to commit thread
        * @param[in] task task
        * @param[in] special_pdata special_pdata
        */
        void handle_special_task(Task *task, CommitParamData& special_pdata);
        /**
        * @brief processing threads occupe and fill buffer,then push group
        *        to commit thread;this function is main procedure of processing threads
        * @param[in] task task
        * @return OB_SUCCESS if success
        */
        int handle_precommit(Task &task);
//        bool is_system_idle(FLogPos& cur_pos);
        //add:e
        bool handle_in_situ_(const int pcode);
        int push_task_(Task &task);
        bool wait_for_commit_(const int pcode);
        bool is_write_packet(ObPacket& pkt);
        bool is_only_master_can_handle(const int pcode);

        int get_session_type(const ObTransID& sid, SessionType& type, const bool check_session_expired);
        int handle_commit_end_(Task &task, common::ObDataBuffer &buffer);

        bool handle_start_session_(Task &task, common::ObDataBuffer &buffer);
        bool handle_end_session_(Task &task, common::ObDataBuffer &buffer);
        bool handle_write_trans_(Task &task, common::ObMutator &mutator, common::ObNewScanner &scanner);
        bool handle_phyplan_trans_(Task &task,
                                  ObUpsPhyOperatorFactory &phy_operator_factory,
                                  sql::ObPhysicalPlan &phy_plan,
                                  common::ObNewScanner &new_scanner,
                                  common::ModuleArena &allocator,
                                  ObDataBuffer& buffer);
        void handle_get_trans_(common::ObPacket &pkt,
                              common::ObGetParam &get_param,
                              common::ObScanner &scanner,
                              common::ObCellNewScanner &new_scanner,
                              common::ObDataBuffer &buffer);
        void handle_scan_trans_(common::ObPacket &pkt,
                                common::ObScanParam &scan_param,
                                common::ObScanner &scanner,
                                common::ObCellNewScanner &new_scanner,
                                common::ObDataBuffer &buffer);
        void handle_kill_zombie_();
        void handle_show_sessions_(common::ObPacket &pkt,
                                  common::ObNewScanner &scanner,
                                  common::ObDataBuffer &buffer);
        void handle_kill_session_(ObPacket &pkt);
        int fill_return_rows_(sql::ObPhyOperator &phy_op, common::ObNewScanner &new_scanner, sql::ObUpsResult &ups_result);
        void reset_warning_strings_();
        void fill_warning_strings_(sql::ObUpsResult &ups_result);

        int handle_write_commit_(Task &task);
        int fill_log_(Task &task, RWSessionCtx &session_ctx);
        int commit_log_();
        int commit_log_(LogGroup* cur_group);//add by zhouhuan for [scalablecommit] 20160822
        void try_submit_auto_freeze_();
        void log_scan_qps_();
        void log_get_qps_();
      private:
        static void phandle_non_impl(common::ObPacket &pkt, ObDataBuffer &buffer);
        static void phandle_freeze_memtable(common::ObPacket &pkt, ObDataBuffer &buffer);
        static void phandle_clear_active_memtable(common::ObPacket &pkt, ObDataBuffer &buffer);
        static void phandle_check_cur_version(common::ObPacket &pkt, ObDataBuffer &buffer);
        static void phandle_check_sstable_checksum(ObPacket &pkt, ObDataBuffer &buffer);
      private:
        static bool thandle_non_impl(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_commit_end(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_scan_trans(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_get_trans(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_write_trans(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_start_session(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_kill_zombie(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_show_sessions(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_kill_session(TransExecutor &host, Task &task, TransParamData &pdata);
        static bool thandle_end_session(TransExecutor &host, Task &task, TransParamData &pdata);
      private:
        static bool chandle_non_impl(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_write_commit(TransExecutor &host, Task &task, CommitParamData &data);
        static bool chandle_fake_write_for_keep_alive(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_send_log(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_slave_reg(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_switch_schema(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_force_fetch_schema(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_switch_commit_log(TransExecutor &host, Task &task, CommitParamData &pdata);
        static bool chandle_nop(TransExecutor &host, Task &task, CommitParamData &pdata);

        //add by zhutao : 20160517:b
        //static int64_t task_get_seq(void *ptask);
        /**
        * @brief end session
        * @param[in] ptask ptask
        * @param[in] pdata pdata
        * @return OB_SUCCESS if success
        */
        int task_end_session(void *ptask, void *pdata);
        //add by zhutao : 20160517:e
    private:
      private:
        ObUtilInterface &ui_;
        common::ThreadSpecificBuffer my_thread_buffer_;
        packet_handler_pt packet_handler_[common::OB_PACKET_NUM];
        trans_handler_pt trans_handler_[common::OB_PACKET_NUM];
        commit_handler_pt commit_handler_[common::OB_PACKET_NUM];
        ObTicketQueue flush_queue_;

        common::FIFOAllocator allocator_;
        SessionCtxFactory session_ctx_factory_;
        SessionMgr session_mgr_;
        LockMgr lock_mgr_;
        ObSpinLock write_clog_mutex1_;
        //add by zhouhuan[scalable commit] 20160711:b
        SpinRWLock write_clog_mutex_;///< write_clog_mutex_
        volatile int64_t commit_task_num_;
        //add:e

        common::ObList<Task*> uncommited_session_list_;
        char ups_result_memory_[OB_MAX_PACKET_LENGTH];
        common::ObDataBuffer ups_result_buffer_;
        //Task nop_task_;
        LogGroup nop_pos_;
        SpinRWLock task_lock_;


        common::ObFIFOStream fifo_stream_;
        // add by guojinwei [log synchronize][multi_cluster] 20151028:b
        volatile int64_t message_residence_time_us_;         ///< used for log synchronization
        volatile int64_t message_residence_protection_us_;   ///< used for log synchronization
        volatile int64_t message_residence_max_us_;          ///< used for log synchronization
        volatile int64_t last_commit_log_time_us_;           ///< used for log synchronization
        // add:e
        //add by hushuang[scalable commit]20160507
        typedef ObFixedQueue<void> ObCommitQueue;///< private queue
        ObCommitQueue commit_queue_[MAX_THREAD_NUM];///< array that contains private queue of each thread
//        common::CommitCallBack *ack_[MAX_THREAD_NUM];
        CommitParamData special_pdata;
        //add e

//        // add by guojinei test
//        int64_t last_test_;
//        // add:e
    };
  }
}

#endif //OCEANBASE_UPDATESERVER_TRANS_EXECUTOR_H_
