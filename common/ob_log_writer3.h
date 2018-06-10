/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_writer3.h
 * @brief
 *
 *
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_04_19
 */
#ifndef OCEANBASE_COMMON_OB_LOG_WRITER3_H_
#define OCEANBASE_COMMON_OB_LOG_WRITER3_H_
#include "tblog.h"

#include "ob_define.h"
#include "data_buffer.h"
#include "ob_slave_mgr.h"
#include "ob_log_entry.h"
#include "ob_file.h"
#include "ob_log_cursor.h"
#include "ob_log_data_writer.h"
#include "ob_log_generator2.h"
#include "ob_log_writer.h"

namespace oceanbase
{
  namespace common
  {
    class ObLogWriterV3 : public ObILogWriter
    {
    public:
      //delete for 2mb configure by qx :20161222
      //static const int64_t LOG_BUFFER_SIZE = OB_MAX_LOG_BUFFER_SIZE;
      //delete :e
      static const int64_t LOG_FILE_ALIGN_BIT = 9;
      static const int64_t LOG_FILE_ALIGN_SIZE = 1 << LOG_FILE_ALIGN_BIT;
      static const int64_t LOG_FILE_ALIGN_MASK = LOG_FILE_ALIGN_SIZE - 1;
      static const int64_t DEFAULT_DU_PERCENT = 60;

    public:
      ObLogWriterV3();
      virtual ~ObLogWriterV3();

      /// 初始化
      /// @param [in] log_dir 日志数据目录
      /// @param [in] log_file_max_size 日志文件最大长度限制
      /// @param [in] slave_mgr ObSlaveMgr用于同步日志
      int init(const char* log_dir, const int64_t log_file_max_size, ObSlaveMgr *slave_mgr,
               int64_t log_sync_type, MinAvailFileIdGetter* min_avail_fid_getter = NULL, const ObServer* server=NULL, int64_t group_size = GROUP_ARRAY_SIZE);

      int reset_log();
      //add hushuang[scalable commit]20160630
      int init_group(int64_t size);
      //add e

      virtual int write_log_hook(const bool is_master,
                                 const ObLogCursor start_cursor, const ObLogCursor end_cursor,
                                 const char* log_data, const int64_t data_len)
      {
          UNUSED(is_master);
          UNUSED(start_cursor);
          UNUSED(end_cursor);
          UNUSED(log_data);
          UNUSED(data_len);
          return OB_SUCCESS;
      }
      bool check_log_size(const int64_t size) const { return log_generator_.check_log_size(size); }
      LogGroup* get_log_group(int64_t group_id) {return log_generator_.get_log_group(group_id); }
      int start_log(const ObLogCursor& start_cursor);
      //int start_log_maybe(const ObLogCursor& start_cursor); used by rootserver
      int get_flushed_cursor(ObLogCursor& log_cursor) const;
      int get_filled_cursor(ObLogCursor& log_cursor) const  {UNUSED(log_cursor); return OB_SUCCESS;};
      bool is_all_flushed() const;
      int64_t get_file_size() const {return log_writer_.get_file_size();};
      int64_t to_string(char* buf, const int64_t len) const;

      int set_log_position(FLogPos& cur_pos, int64_t len, int64_t& switch_flag, FLogPos& next_pos);
      bool switch_group(FLogPos& cur_pos)
      {
        return log_generator_.switch_group(cur_pos);
      }
      //add hushuang [scalable commit]20160507
      int submmit_callback_info(uint64_t thread_no, int64_t group_id, IAckCallback* cb);
      //add e
      int write_end_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
      int set_sync_to_slave(FLogPos& cur_pos);
      int set_group_start_timestamp(FLogPos& cur_pos);
      int64_t get_trans_id(FLogPos& cur_pos);
      FLogPos get_next_pos() {return log_generator_.get_next_pos();}
//      void switch_next_pos() { return log_generator_.switch_next_pos();}
      /// @brief 写日志
      /// write_log函数将日志存入自己的缓冲区, 缓冲区大小LOG_BUFFER_SIZE常量
      /// 首先检查日志大小是否还允许存入一整个缓冲区, 若不够则切日志
      /// 然后将日志内容写入缓冲区
      /// @param [in] log_data 日志内容
      /// @param [in] data_len 长度
      /// @retval OB_SUCCESS 成功
      /// @retval OB_BUF_NOT_ENOUGH 内部缓冲区已满
      /// @retval otherwise 失败
      int write_log(const LogCommand cmd, const char* log_data, const int64_t data_len, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);

      template<typename T>
      int write_log(const LogCommand cmd, const T& data, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);

      //add chujiajia [log synchronization][multi_cluster] 20160328:b
      inline int64_t get_flushed_clog_id_without_update()
      {
        return slave_mgr_->get_acked_clog_id_without_update();
      }
      //add:e
      int write_keep_alive_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);

      int async_flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer());
      int64_t get_flushed_clog_id();
      /// @brief 将缓冲区中的日志写入磁盘
      /// flush_log首相将缓冲区中的内容同步到Slave机器
      /// 然后写入磁盘
      /// @retval OB_SUCCESS 成功
      /// @retval otherwise 失败
      int flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer = oceanbase::common::TraceLog::get_logbuffer(),
                    const bool sync_to_slave = true, const bool is_master = true);
      int switch_log_group(FLogPos& cur_pos, FLogPos& next_pos, int64_t& ref_cnt, const int64_t max_cmt_id);

      /// @brief 写日志并且切换group
      /// 序列化日志并且序列化结束日志
      int write_and_end_log(const LogCommand cmd, const char* log_data, const int64_t data_len, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
      template<typename T>
      int write_and_end_log(const LogCommand cmd, const T& data, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);

      /// @brief 将缓冲区内容写入日志文件
      /// 然后将缓冲区内容刷入磁盘
      /// @retval OB_SUCCESS 成功
      /// @retval otherwise 失败
      int store_log(const char* buf, const int64_t buf_len, const bool sync_to_slave=false);

      void set_disk_warn_threshold_us(const int64_t warn_us);
      void set_net_warn_threshold_us(const int64_t warn_us);
      /// @brief Master切换日志文件
      /// 产生一条切换日志文件的commit log
      /// 同步到Slave机器并等待返回
      /// 关闭当前正在操作的日志文件, 日志序号+1, 打开新的日志文件
      /// @retval OB_SUCCESS 成功
      /// @retval otherwise 失败
      virtual int switch_log_file(uint64_t &new_log_file_id, FLogPos& cur_pos, const int64_t max_cmt_id);

      virtual int switch_log_file(uint64_t &new_log_file_id)
      {
        int ret = OB_SUCCESS;

        UNUSED(new_log_file_id);
        return ret;
      }

      virtual int write_replay_point(uint64_t replay_point)
      {
        UNUSED(replay_point);
        TBSYS_LOG(WARN, "not implement");
        return common::OB_NOT_IMPLEMENT;
      };

      /// @brief 写一条checkpoint日志并返回当前日志号
      /// 写完checkpoint日志后切日志
      //used by rootserver
      //int write_checkpoint_log(uint64_t &log_file_id);

      inline ObSlaveMgr* get_slave_mgr() {return slave_mgr_;}

      inline int64_t get_last_net_elapse() {return last_net_elapse_;}
      inline int64_t get_last_disk_elapse() {return last_disk_elapse_;}
      inline int64_t get_last_flush_log_time() {return last_flush_log_time_;}

      /*void printBuf()
      {
        log_generator_.printBuf();
      }*/
    protected:
      inline int check_inner_stat() const
      {
        int ret = OB_SUCCESS;
        if (!is_initialized_)
        {
          TBSYS_LOG(ERROR, "ObLogWriter has not been initialized");
          ret = OB_NOT_INIT;
        }
        return ret;
      }

      protected:
        bool is_initialized_;
        ObSlaveMgr *slave_mgr_;
        ObLogGeneratorV2 log_generator_;
        ObLogDataWriter log_writer_;
        int64_t net_warn_threshold_us_;
        int64_t disk_warn_threshold_us_;
        int64_t last_net_elapse_;  ///<上一次写日志网络同步耗时
        int64_t last_disk_elapse_;  ///<上一次写日志磁盘耗时
        int64_t last_flush_log_time_; ///<上次刷磁盘的时间
    };

    template<typename T>
    int ObLogWriterV3::write_log(const LogCommand cmd, const T& data, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int ret = OB_SUCCESS;

      if (OB_SUCCESS != (ret = check_inner_stat()))
      {
        TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
      }
      else if (OB_SUCCESS != (ret = log_generator_.write_log(cmd, data, cur_pos, ref_cnt, max_cmt_id)))
      {
        TBSYS_LOG(WARN, "log_generator.write_log(cmd=%d, data=%p)=>%d", cmd, &data, ret);
      }
      return ret;
    }

    template<typename T>
    int ObLogWriterV3::write_and_end_log(const LogCommand cmd, const T& data, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int ret = check_inner_stat();
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      ObLogEntry entry;

      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
      }
      else
      {
        while(true)
        {
          if(cur_group->ts_seq_ != cur_pos.group_id_)
          {
            usleep(1);
          }
          else
          {
            ++cur_pos.rel_id_;
            cur_pos.rel_offset_ = cur_pos.rel_offset_ + (int32_t)data.get_serialize_size() + (int32_t)entry.get_serialize_size();
            cur_group->sync_to_slave_ = true;
            if (OB_SUCCESS != (ret = set_group_start_timestamp(cur_pos)))
            {
              TBSYS_LOG(ERROR, "set_group_start_timestamp()=>%d", ret);
            }
            else if (OB_SUCCESS != (ret = write_log(cmd, data, cur_pos, ref_cnt, max_cmt_id)))
            {
              TBSYS_LOG(ERROR, "write_log(cmd=%d, log_data=%p ret=%d)", cmd, &data, ret);
            }
            else if(OB_SUCCESS != (ret = write_end_log(cur_pos, ref_cnt, max_cmt_id)))
            {
              TBSYS_LOG(WARN, "write end log fail ret=%d", ret);
            }
            break;
          }
        }
      }
      return ret;
    }

  } // end namespace common
} // end namespace oceanbase

#endif //OCEANBASE_COMMON_OB_LOG_WRITER3_H_

