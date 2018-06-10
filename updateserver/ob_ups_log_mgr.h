/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_log_mgr.h
 * @brief ObUpsLogMgr
 *     modify by guojinwei, liubozhong: support multiple clusters
 *     for HA by adding or modifying some functions, member variables
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         liubozhong <51141500077@encu.cn>
 * @date 2015_12_30
 */
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
 *   yanran <yanran.hfs@taobao.com>
 *     - some work details if you want
 */

#ifndef OCEANBASE_UPDATESERVER_OB_UPS_LOG_MGR_H_
#define OCEANBASE_UPDATESERVER_OB_UPS_LOG_MGR_H_

#include "common/ob_define.h"
#include "common/ob_log_writer.h"
#include "common/ob_log_writer3.h"
#include "common/ob_mutex_task.h"
#include "common/ob_server_getter.h"
#include "ob_ups_role_mgr.h"
#include "ob_ups_table_mgr.h"
#include "ob_ups_slave_mgr.h"

#include "ob_ups_log_utils.h"
#include "ob_log_buffer.h"
#include "ob_pos_log_reader.h"
#include "ob_cached_pos_log_reader.h"
#include "ob_replay_log_src.h"
#include "ob_log_sync_delay_stat.h"
#include "ob_clog_stat.h"
#include "ob_log_replay_worker.h"
#include "ob_ups_stat.h"
//add lbzhong [Commit Point] 20150820:b
#include "ob_commit_point_runnable.h"
//add:e

namespace oceanbase
{
  namespace tests
  {
    namespace updateserver
    {
      // forward decleration
      class TestObUpsLogMgr_test_init_Test;
    }
  }
  namespace updateserver
  {
    class ObUpsLogMgr : public common::ObLogWriterV3/*ObLogWriter*///modify by zhouhuan
    {
      public:
      enum WAIT_SYNC_TYPE
      {
        WAIT_NONE = 0,
        WAIT_COMMIT = 1,
        WAIT_FLUSH = 2,
      };
      friend class tests::updateserver::TestObUpsLogMgr_test_init_Test;
        class FileIdBeforeLastMajorFrozen: public MinAvailFileIdGetter
        {
          public:
            FileIdBeforeLastMajorFrozen(){}
            ~FileIdBeforeLastMajorFrozen(){}
            virtual int64_t get();
        };

      struct ReplayLocalLogTask
      {
        ReplayLocalLogTask(): lock_(0), log_mgr_(NULL) {}
        virtual ~ReplayLocalLogTask() {}
        int init(ObUpsLogMgr* log_mgr)
        {
          int err = OB_SUCCESS;
          log_mgr_ = log_mgr;
          return err;
        }
        bool is_started() const
        {
          return lock_ >= 1;
        }
        bool is_done() const
        {
          return lock_ >= 2;
        }
        virtual int do_work()
        {
          OnceGuard guard(lock_);
          int err = OB_SUCCESS;
          if (!guard.try_lock())
          {}
          else if (NULL == log_mgr_)
          {
            err = OB_NOT_INIT;
          }
          else if (OB_SUCCESS != (err = log_mgr_->replay_local_log()))
          {
            if (OB_CANCELED == err)
            {
              TBSYS_LOG(WARN, "cancel replay local log");
            }
            else
            {
              TBSYS_LOG(ERROR, "replay_local_log()=>%d, will kill self", err);
              kill(getpid(), SIGTERM);
            }
          }
          else
          {
            TBSYS_LOG(INFO, "replay local log succ.");
          }
          return err;
        }
        volatile uint64_t lock_;
        ObUpsLogMgr* log_mgr_;
      };
      public:
      ObUpsLogMgr();
      virtual ~ObUpsLogMgr();
      int init(const char* log_dir, const int64_t log_file_max_size,
               ObLogReplayWorker* replay_worker_, ObReplayLogSrc* replay_log_src, ObUpsTableMgr* table_mgr,
               ObUpsSlaveMgr *slave_mgr, ObiRole* obi_role, ObUpsRoleMgr *role_mgr, int64_t log_sync_type
				//delete chujiajia [log synchronization][multi_cluster] 20160625:b
               //add by lbzhong [Commit Point] 20150824:b
               //, ObCommitPointRunnable* commit_point_thread
               //add:e
              
               //add hushuang [scalable commit]20160630
               ,int64_t group_size = GROUP_ARRAY_SIZE
               //add e
               );

      /// @brief set new replay point
      /// this method will write replay point to replay_point_file
      int write_replay_point(uint64_t replay_point);
      //add lbzhong [Commit Point] 20150820:b
      /**
       * @brief flush commit point to file
       * @return OB_SUCCESS if success
       */
      int flush_commit_point();

      /**
       * @brief get was_master from file
       * @param was_master [out] whether the ups was master before restart
       * @return true if the ups was master before restart
       */
      bool get_was_master(bool& was_master);
      //add:e
      //add:b
      int64_t get_max_flushed_log_id()
      {
        return replay_worker_->get_next_flush_log_id();
      }

      //add:e
      // add by guojinwei [commit point for log replay][multi_cluster] 20151119:b
      /**
       * @brief flush commit point to file
       * @param[in] commit_point  the commit point
       * @return OB_SUCCESS if success
       */
      int flush_commit_point(const int64_t commit_point); 

      /**
       * @brief get last flushed commit point
       * @param[out] last_commit_point  the last flushed commit point
       * @return OB_SUCCESS if success
       */
      int get_last_commit_point(int64_t& last_commit_point) const;
      // add:e
      //add lbzhong [Max Log Timestamp] 20150820:b
      /**
       * @brief get max log timestamp from log file and log buffer
       * @param[out] max_timestamp  the max log timestamp
       * @return OB_SUCCESS if success
       */
      int get_max_log_timestamp(int64_t& max_timestamp) const;
      //add:e
      public:
        // 继承log_writer留下的回调接口

        // 主备UPS每次写盘之后调用
        virtual int write_log_hook(const bool is_master,
                                   const ObLogCursor start_cursor, const ObLogCursor end_cursor,
                                   const char* log_data, const int64_t data_len);
      public:
      // 统计回放日志的时间和生成mutator的时间延迟
      ObLogSyncDelayStat& get_delay_stat()
      {
        return delay_stat_;
      }
      int add_log_replay_event(const int64_t seq, const int64_t mutation_time);
        // 取得recent_log_cache的引用
      ObLogBuffer& get_log_buffer();
      public: // 主要接口
        // UPS刚启动，重放本地日志任务的函数
      int replay_local_log();
	  //add chujiajia [log synchronization][multi_cluster] 20160603:b
      /**
       * @brief update replay cursor after master switch to slave
       * @return OB_SUCCESS if success
       */
      int update_tmp_log_cursor();
      //add:e
      int start_log(const ObLogCursor& start_cursor);
        // 重放完本地日志之后，主UPS调用start_log_for_master_write()，
        //主要是初始化log_writer写日志的起始点
      int start_log_for_master_write();
        // 主UPS给备UPS推送日志后，备UPS调用slave_receive_log()收日志
        // 收到日志后放到recent_log_cache
        int slave_receive_log(const char* buf, int64_t len, const int64_t wait_sync_time_us,
                              const WAIT_SYNC_TYPE wait_event_type);
        // 备UPS向主UPS请求日志时，主UPS调用get_log_for_slave_fetch()读缓冲区或文件得到日志
      int get_log_for_slave_fetch(ObFetchLogReq& req, ObFetchedLog& result);
        // 备UPS向主UPS注册成功后，主UPS返回自己的最大日志号，备UPS调用set_master_log_id()记录下这个值
      int set_master_log_id(const int64_t master_log_id);
        // RS选主时，需要问UPS已收到的连续的最大日志号
      int get_max_log_seq_replayable(int64_t& max_log_seq) const;
        // 如果备UPS本地没有日志，需要问主UPS自己应该从那个点开始写，
        // 主UPS调用fill_log_cursor()填充日志点
      int fill_log_cursor(ObLogCursor& log_cursor);
        // 备UPS的replay线程，调用replay_log()方法不停追赶主机的日志
        // replay_log()接着本地日志开始，不仅负责回放，也负责写盘
      int replay_log();
      int get_replayed_cursor(ObLogCursor& cursor) const;
      ObLogCursor& get_replayed_cursor_(ObLogCursor& cursor) const;
      bool is_sync_with_master() const;
      bool has_nothing_in_buf_to_replay() const;
      bool has_log_to_replay() const;
      int64_t wait_new_log_to_replay(const int64_t timeout_us);
      //modify chujiajia [log synchronization][multi_cluster] 20160625:b
      //int write_log_as_slave(const char* buf, const int64_t len);
      int write_log_as_slave(int64_t start_id, const char* buf, const int64_t len);
      //modify:e
      //add by zhouhuan [scalablecommit] 20160621:b
      /**
      * @brief slave set its group according to timestamp
      * @param[in] log_id log id
      * @param[in] timestamp timestamp
      */
      void set_group_as_slave(const int64_t log_id, const int64_t timestamp)
      {
        log_generator_.set_group_as_slave(log_id, timestamp);
      }
      void reset_next_pos(){ log_generator_.reset_next_pos();}
      //add e
      int64_t to_string(char* buf, const int64_t len) const;
      //add lbzhong [Commit Point] 20150820:b
      /**
       * @brief whether the ups is master master
       * @return true if the ups is master master
       */
      bool is_master_master() const;

      /**
       * @brief [overwrite]
       * @return OB_SUCCESS if success
       */
	   //modify chujiajia [log synchronization][multi_cluster] 20160625:b
      //int store_log(const char* buf, const int64_t buf_len, const bool sync_to_slave=false);
      int store_log(int64_t start_id, const char* buf, const int64_t buf_len, const bool sync_to_slave=false);
      //modify:e
     

      /**
       * @brief [overwrite]
       * If the ups is master master, it will update was_master to true in this function.
       * And it will update max log timestamp of log buffer in this function.
       * @return OB_SUCCESS if success
       */
      //modify by zhouhuan [scalablecommit] 20160505
     // int async_flush_log(int64_t& end_log_id, TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer());
      int async_flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer());

      /**
       * @brief [overwrite]
       * If the ups is not master master, it will update was_master to false in this function.
       * @return OB_SUCCESS if success
       */
      //modify by zhouhuan [scalablecommit] 20160505
     // int flush_log(TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer(),
                          //const bool sync_to_slave = true, const bool is_master = true);
      int flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer(),
                          const bool sync_to_slave = true, const bool is_master = true);
      //add:e
      protected:
        // 下面几个方法都是replay_log()需要的辅助方法
      int replay_and_write_log(const int64_t start_id, const int64_t end_id, const char* buf, int64_t len);
      int retrieve_log_for_replay(const int64_t start_id, int64_t& end_id, char* buf, const int64_t len,
          int64_t& read_count);
        // 确保得到一个确定的日志点才能开始回放后续日志
        // 可能需要用rpc询问主UPS
      int start_log_for_replay();

      protected:
      //add lbzhong [Max Log Timestamp] 20150820:b
      /**
       * @brief get max log timestamp from log file
       * @param[out] max_timestamp  max log timestamp from log file
       * @return OB_SUCCESS if success
       */
      int get_max_log_timestamp_in_file(int64_t& max_timestamp) const;

      /**
       * @brief get max log timestamp from log buffer
       * @param[out] max_timestamp  max log timestamp from log buffer
       * @return OB_SUCCESS if success
       */
      int get_max_log_timestamp_in_buffer(int64_t& max_timestamp) const;
	   //add:e
      //add chujiajia [log synchronization][multi_cluster] 20160419:b
      /**
       * @brief get max commited log id from log file
       * @param[out] cmt_id  max commited log id from log file
       * @param[out] tmp_end_cursor  cursor relevant to the max commited log
       * @return OB_SUCCESS if success
       */
      int get_max_cmt_id_in_file(int64_t& cmt_id, ObLogCursor &tmp_end_cursor) const;
      //add:e
      //delete by lbzhong [Commit point] 20150820:b
      //move this function to public
      //bool is_master_master() const;
      //delete:e
      bool is_slave_master() const;
      int set_state_as_active();
      int get_max_log_seq_in_file(int64_t& log_seq) const;
      int get_max_log_seq_in_buffer(int64_t& log_seq) const;
      public:
      int do_replay_local_log_task()
      {
        return replay_local_log_task_.do_work();
      }
      bool is_log_replay_started() const
      {
        return replay_local_log_task_.is_started();
      }
      bool is_log_replay_finished() const
      {
        return replay_local_log_task_.is_done();
      }
      uint64_t get_master_log_seq() const
      {
        return master_log_id_;
      }
      void signal_stop()
      {
        if (replay_worker_)
        {
          replay_worker_->stop();
        }
        stop_ = true;
      }

      void wait()
      {
        if (replay_worker_)
        {
          replay_worker_->wait();
        }
      }
      inline int64_t get_last_receive_log_time() {return last_receive_log_time_;}

        inline ObClogStat& get_clog_stat() {return clog_stat_; }
      int add_slave(const common::ObServer& server, uint64_t &new_log_file_id, const bool switch_log);

      private:
      bool is_inited() const;
      private:
      int load_replay_point_();
      //add lbzhong [Commit Point] 20150820:b
      int get_commit_point_from_file(int64_t& commit_seq);
      //add:e
      inline int check_inner_stat() const
      {
        int ret = common::OB_SUCCESS;
        if (!is_initialized_)
        {
          TBSYS_LOG(ERROR, "ObUpsLogMgr has not been initialized");
          ret = common::OB_NOT_INIT;
        }
        return ret;
      }

      private:
      FileIdBeforeLastMajorFrozen last_fid_before_frozen_;
      ObUpsTableMgr *table_mgr_;
      ObiRole *obi_role_;
      ObUpsRoleMgr *role_mgr_;
      ObLogBuffer recent_log_cache_; // 主机写日志或备机收日志时保存日志用的缓冲区
      ObPosLogReader pos_log_reader_; //从磁盘上根据日志ID读日志， 用来初始化cached_pos_log_reader_
      ObCachedPosLogReader cached_pos_log_reader_; // 根据日志ID读日志，先查看缓冲区，在查看日志文件
      ObReplayLogSrc* replay_log_src_; // 备机replay_log()时从replay_log_src_中取日志

      IObServerGetter* master_getter_; // 用来指示远程日志源的地址和类型(是lsync还是ups)
      ReplayLocalLogTask replay_local_log_task_;
      //common::ObLogCursor replayed_cursor_; // 已经回放到的点，不管发生什么情况都有保持连续递增
      common::ObLogCursor start_cursor_; // start_log()用到的参数，start_log()之后就没用了。
      int64_t local_max_log_id_when_start_;
      ObLogSyncDelayStat delay_stat_;
      ObClogStat clog_stat_;
      volatile bool stop_; // 主要用来通知回放本地日志的任务结束
      ThreadSpecificBuffer log_buffer_for_fetch_;
      ThreadSpecificBuffer log_buffer_for_replay_;
      ObLogReplayWorker* replay_worker_;
      volatile int64_t master_log_id_; // 备机知道的主机最大日志号
      tbsys::CThreadCond master_log_id_cond_;
      int64_t last_receive_log_time_;
      ObLogReplayPoint replay_point_;
      uint64_t max_log_id_;
      bool is_initialized_;
      char log_dir_[common::OB_MAX_FILE_NAME_LENGTH];
      bool is_started_;
      //add lbzhong [Commit Point] 20150820:b
      ObCommitPointRunnable* commit_point_thread_;
      int64_t last_commit_point_;
      int is_master_;
      ObFileForLog<int64_t> commit_point_;
      ObFileForLog<bool> was_master_;
      //add:e
      //add lbzhong [Max Log Timestamp] 20150820:b
      int64_t local_max_log_timestamp_when_start_;
      int64_t local_max_log_timestamp_;
      //add:e
	  //add chujiajia [log synchronization][multi_cluster] 20160419:b
      int64_t local_max_cmt_id_when_start_;  ///< max commited log id from log file when server started
      common::ObLogCursor local_max_log_cursor_;  ///< cursor relevant to the max local log
      common::ObLogCursor local_commited_max_cursor_;  ///< cursor relevant to the max commited log
      //add:e
    };
  } // end namespace updateserver
} // end namespace oceanbase

#endif // OCEANBASE_UPDATESERVER_OB_UPS_LOG_MGR_H_
