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

#include "ob_log_writer3.h"
#include "ob_trace_log.h"
#include "ob_malloc.h"

using namespace oceanbase::common;

ObLogWriterV3::ObLogWriterV3(): is_initialized_(false),
                            net_warn_threshold_us_(5000),
                            disk_warn_threshold_us_(5000),
                            last_net_elapse_(0), last_disk_elapse_(0), last_flush_log_time_(0)
{
}

ObLogWriterV3::~ObLogWriterV3()
{
}

int ObLogWriterV3::init(const char* log_dir, const int64_t log_file_max_size, ObSlaveMgr *slave_mgr,
                      int64_t log_sync_type, MinAvailFileIdGetter* fufid_getter, const ObServer* server, int64_t group_size)
{
  int ret = OB_SUCCESS;
  int64_t du_percent = DEFAULT_DU_PERCENT;

  if (is_initialized_)
  {
    TBSYS_LOG(ERROR, "ObLogWriterV3 has been initialized");
    ret = OB_INIT_TWICE;
  }
  else if (NULL == log_dir || NULL == slave_mgr)
  {
    TBSYS_LOG(ERROR, "Parameter are invalid[log_dir=%p slave_mgr=%p]", log_dir, slave_mgr);
    ret = OB_INVALID_ARGUMENT;
  }
  else if (OB_SUCCESS != (ret = log_generator_.init(OB_MAX_LOG_BUFFER_SIZE, log_file_max_size, server, group_size)))  //modify by qx 20161229 replace LOG_BUFFER_SIZE
  {
    TBSYS_LOG(ERROR, "log_generator.init(buf_size=%ld, file_limit=%ld)=>%d", OB_MAX_LOG_BUFFER_SIZE, log_file_max_size, ret);
  }
  else if (OB_SUCCESS != (ret = log_writer_.init(log_dir, log_file_max_size, du_percent, log_sync_type, fufid_getter)))
  {
    TBSYS_LOG(ERROR, "log_writer.init(log_dir=%s, file_limit=%ld, du_percent=%ld, sync_type=%ld, fufid_getter=%p)=>%d",
              log_dir, log_file_max_size, du_percent, log_sync_type, fufid_getter, ret);
  }

  if (OB_SUCCESS == ret)
  {
    slave_mgr_ = slave_mgr;
    is_initialized_ = true;
    TBSYS_LOG(INFO, "ObLogWriterV3 initialize successfully[log_dir_=%s log_file_max_size_=%ld"
              " slave_mgr_=%p log_sync_type_=%ld]",
        log_dir, log_file_max_size, slave_mgr_, log_sync_type);
  }

  return ret;
}

int ObLogWriterV3::reset_log()
{
  int err = OB_SUCCESS;
  if (OB_SUCCESS != (err = log_writer_.reset()))
  {
    TBSYS_LOG(ERROR, "log_writer.reset()=>%d", err);
  }
  else if (OB_SUCCESS != (err = log_generator_.reset()))
  {
    TBSYS_LOG(ERROR, "log_generator.reset()=>%d", err);
  }
  return err;
}
//add hushuang[scalable commit]20160630
int ObLogWriterV3::init_group(int64_t size)
{
  return log_generator_.init_group(size);
}
//add e
int ObLogWriterV3::start_log(const ObLogCursor& log_cursor)
{
  int err = OB_SUCCESS;
  if (OB_SUCCESS != (err = log_writer_.start_log(log_cursor)))
  {
    TBSYS_LOG(ERROR, "log_writer.start_log(%s)=>%d", to_cstring(log_cursor), err);
  }
  else if (OB_SUCCESS != (err = log_generator_.start_log(log_cursor)))
  {
    TBSYS_LOG(ERROR, "log_generator.start_log(%s)=>%d", to_cstring(log_cursor), err);
  }
  return err;
}

//used by rootserver
/*int ObLogWriterV3::start_log_maybe(const ObLogCursor& start_cursor)
{
  int err = OB_SUCCESS;
  ObLogCursor log_cursor;
  if (log_generator_.is_log_start())
  {
    if (OB_SUCCESS != (err = log_generator_.get_end_cursor(log_cursor)))
    {
      TBSYS_LOG(ERROR, "log_generator.get_end_cursor()=>%d", err);
    }
    else if (!log_cursor.equal(start_cursor))
    {
      err = OB_DISCONTINUOUS_LOG;
      TBSYS_LOG(ERROR, "log_cursor.equal(start_cursor=%s)=>%d", to_cstring(start_cursor), err);
    }
  }
  else if (OB_SUCCESS != (err = start_log(start_cursor)))
  {
    TBSYS_LOG(ERROR, "start_log(%s)=>%d", to_cstring(start_cursor), err);
  }
  return err;
}*/

int ObLogWriterV3::get_flushed_cursor(ObLogCursor& log_cursor) const
{
  return log_writer_.get_cursor(log_cursor);
}


int ObLogWriterV3::set_log_position(FLogPos &cur_pos, int64_t len, int64_t& switch_flag, FLogPos& next_pos)
{
  int ret = OB_SUCCESS;

  if (OB_SUCCESS != (ret = check_inner_stat()))
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.set_log_position(cur_pos, len, switch_flag, next_pos)))
  {
    TBSYS_LOG(WARN, "log_generator.set_log_position()=>%d", ret);
  }
  return ret;
}

//add hushuang [scalable commit]20160507
int ObLogWriterV3::submmit_callback_info(uint64_t thread_no, int64_t group_id, IAckCallback *cb)
{
  int ret = OB_SUCCESS;
  LogGroup* group = NULL;
  if(0 > group_id || NULL == cb)
  {
    ret = OB_ERROR;
    TBSYS_LOG(ERROR, "call back info thread_no %ld, group_id %ld, pointer %p", thread_no, group_id, cb);
  }
  else if(NULL == (group = log_generator_.get_log_group(group_id)))
  {
    ret = OB_ERROR;
  }
  else if(OB_SUCCESS != (ret = group->cb_list.add_cb_func(thread_no, cb)))
  {
    TBSYS_LOG(WARN, "add cb function failed, ret = %d", ret);
  }
  return ret;
}
//add e

int ObLogWriterV3::write_end_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
{
  int ret = check_inner_stat();

  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.write_end_log(cur_pos, ref_cnt, max_cmt_id)))
  {
    TBSYS_LOG(WARN, "log_generator.write_end_log()=>%d", ret);
  }
  //TBSYS_LOG(ERROR, "test::zhouhuan write_end_log cur_pos=[%s]", to_cstring(cur_pos));
  return ret;
}

int ObLogWriterV3::set_group_start_timestamp(FLogPos& cur_pos)
{
  int ret = check_inner_stat();

  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.set_group_start_timestamp(cur_pos)))
  {
    TBSYS_LOG(WARN, "log_generator.set_group_start_timestamp()=>%d", ret);
  }
  return ret;
}

int64_t ObLogWriterV3::get_trans_id(FLogPos& cur_pos)
{
  return log_generator_.get_trans_id(cur_pos);
}

int ObLogWriterV3::write_log(const LogCommand cmd, const char* log_data, const int64_t data_len,
                             FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
{
  int ret = check_inner_stat();

  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.write_log(cmd, log_data, data_len, cur_pos, ref_cnt, max_cmt_id)))
  {
    TBSYS_LOG(WARN, "log_generator.write_log(cmd=%d, buf=%p[%ld])=>%d", cmd, log_data, data_len, ret);
  }

  //TBSYS_LOG(ERROR, "test::zhouhuan write_log cur_pos=[%s]", to_cstring(cur_pos));
  return ret;
}

inline int64_t get_align_padding_size(const int64_t x, const int64_t mask)
{
  return -x & mask;
}

int64_t ObLogWriterV3::to_string(char* buf, const int64_t len) const
{
  int64_t pos = 0;
  databuff_printf(buf, len, pos, "LogWriter(data_writer=%s)",
                 to_cstring(log_writer_));
  return pos;
}

int ObLogWriterV3::write_keep_alive_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
{
  int err = OB_SUCCESS;
  if (OB_SUCCESS != (err = log_generator_.gen_keep_alive(cur_pos, ref_cnt, max_cmt_id)))
  {
    TBSYS_LOG(ERROR, "write_keep_alive_log()=>%d", err);
  }
  return err;
}

int64_t ObLogWriterV3::get_flushed_clog_id()
{
  return slave_mgr_->get_acked_clog_id();
}

/*int ObLogWriterV3::get_filled_cursor(ObLogCursor& log_cursor) const
{
  return log_generator_.get_end_cursor(log_cursor);
}*/

/*bool ObLogWriterV3::is_all_flushed() const
{
  ObLogCursor cursor;
  log_generator_.get_start_cursor(cursor);
  return slave_mgr_->get_acked_clog_id() >= cursor.log_id_;
}*/

int ObLogWriterV3::async_flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer)
{
  int ret = check_inner_stat();
  int send_err = OB_SUCCESS;
  char* buf = NULL;
  int64_t len = 0;
  ObLogCursor start_cursor;
  ObLogCursor end_cursor;
  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.get_log(start_cursor, end_cursor, buf, len, cur_group)))
  {
    TBSYS_LOG(ERROR, "log_generator.get_log()=>%d", ret);
  }
  else if (len <= 0)
  {}
  else if (0 != (len & ObLogGenerator::LOG_FILE_ALIGN_MASK))
  {
    ret = OB_LOG_NOT_ALIGN;
    TBSYS_LOG(ERROR, "len=%ld cursor=[%s,%s], not align", len, to_cstring(start_cursor), to_cstring(end_cursor));
    while(1);
  }
  else
  {
    int64_t store_start_time_us = tbsys::CTimeUtil::getTime();
    if (OB_SUCCESS != (send_err = slave_mgr_->post_log_to_slave(start_cursor, end_cursor, buf, len)))
    {
      TBSYS_LOG(WARN, "slave_mgr.send_data(buf=%p[%ld], %s)=>%d", buf, len, to_cstring(*this), send_err);
    }
    if (OB_SUCCESS != (ret = log_writer_.write(start_cursor, end_cursor,
                                               buf, len + ObLogGenerator::LOG_FILE_ALIGN_SIZE)))
    {
      TBSYS_LOG(ERROR, "log_writer.write_log(buf=%p[%ld], cursor=[%s,%s])=>%d, maybe disk FULL or Broken",
                buf, len, to_cstring(start_cursor), to_cstring(end_cursor), ret);
    }
    else
    {
      last_disk_elapse_ = tbsys::CTimeUtil::getTime() - store_start_time_us;
      if (last_disk_elapse_ > disk_warn_threshold_us_)
      {
        TBSYS_LOG(WARN, "last_disk_elapse_[%ld] > disk_warn_threshold_us[%ld], cursor=[%s,%s], len=%ld",
                  last_disk_elapse_, disk_warn_threshold_us_, to_cstring(start_cursor), to_cstring(end_cursor), len);
      }
    }
  }
  FILL_TRACE_BUF(tlog_buffer, "write_log disk=%ld net=%ld len=%ld log=%ld:%ld",
                 last_disk_elapse_, last_net_elapse_, len,
                 start_cursor.log_id_, end_cursor.log_id_);
  if (OB_SUCCESS != ret)
  {}
  else if (OB_SUCCESS != (ret = log_generator_.commit(cur_group, end_cursor, true)))
  {
    TBSYS_LOG(ERROR, "log_generator.commit(end_cursor=%s)", to_cstring(end_cursor));
  }
  else if (OB_SUCCESS != (ret = write_log_hook(true, start_cursor, end_cursor, buf, len)))
  {
    TBSYS_LOG(ERROR, "write_log_hook(log_id=[%ld,%ld))=>%d", start_cursor.log_id_, end_cursor.log_id_, ret);
  }
  else if (len > 0)
  {
    last_flush_log_time_ = tbsys::CTimeUtil::getTime();
  }
  return ret;
}

int ObLogWriterV3::flush_log(LogGroup* cur_group, TraceLog::LogBuffer &tlog_buffer, const bool sync_to_slave, const bool is_master)
{
  int ret = check_inner_stat();
  int send_err = OB_SUCCESS;
  char* buf = NULL;
  int64_t len = 0;
  ObLogCursor start_cursor;
  ObLogCursor end_cursor;
  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.get_log(start_cursor, end_cursor, buf, len, cur_group)))
  {
    TBSYS_LOG(ERROR, "log_generator.get_log()=>%d", ret);
  }
  else if (len <= 0)
  {}
  else
  {
    int64_t store_start_time_us = tbsys::CTimeUtil::getTime();
    if (sync_to_slave)
    {
      if (OB_SUCCESS != (send_err = slave_mgr_->post_log_to_slave(start_cursor, end_cursor, buf, len)))
      {
        TBSYS_LOG(WARN, "slave_mgr.send_data(buf=%p[%ld], %s)=>%d", buf, len, to_cstring(*this), send_err);
      }
    }
    if (OB_SUCCESS != (ret = log_writer_.write(start_cursor, end_cursor,
                                               buf, len + ObLogGenerator::LOG_FILE_ALIGN_SIZE)))
    {
      TBSYS_LOG(ERROR, "log_writer.write_log(buf=%p[%ld], cursor=[%s,%s])=>%d, maybe disk FULL or Broken",
                buf, len, to_cstring(start_cursor), to_cstring(end_cursor), ret);
    }
    else
    {
      last_disk_elapse_ = tbsys::CTimeUtil::getTime() - store_start_time_us;
      if (last_disk_elapse_ > disk_warn_threshold_us_)
      {
        TBSYS_LOG(TRACE, "last_disk_elapse_[%ld] > disk_warn_threshold_us[%ld], cursor=[%s,%s], len=%ld",
                  last_disk_elapse_, disk_warn_threshold_us_, to_cstring(start_cursor), to_cstring(end_cursor), len);
      }
    }
    if (sync_to_slave)
    {
      int64_t delay = -1;
      if (OB_SUCCESS != (send_err = slave_mgr_->wait_post_log_to_slave(buf, len, delay)))
      {
        TBSYS_LOG(ERROR, "slave_mgr.send_data(buf=%p[%ld], cur_write=[%s,%s], %s)=>%d", buf, len, to_cstring(start_cursor), to_cstring(end_cursor), to_cstring(*this), send_err);
      }
      else if (delay >= 0)
      {
        last_net_elapse_ = delay;
        if (last_net_elapse_ > net_warn_threshold_us_)
        {
          TBSYS_LOG(TRACE, "last_net_elapse_[%ld] > net_warn_threshold_us[%ld]", last_net_elapse_, net_warn_threshold_us_);
        }
      }
    }
  }
  FILL_TRACE_BUF(tlog_buffer, "write_log disk=%ld net=%ld len=%ld log=%ld:%ld",
                 last_disk_elapse_, last_net_elapse_, len,
                 start_cursor.log_id_, end_cursor.log_id_);
  if (OB_SUCCESS != ret)
  {}
  else if (OB_SUCCESS != (ret = log_generator_.commit(cur_group, end_cursor, is_master)))
  {
    TBSYS_LOG(ERROR, "log_generator.commit(end_cursor=%s)", to_cstring(end_cursor));
  }
  else if (OB_SUCCESS != (ret = write_log_hook(is_master, start_cursor, end_cursor, buf, len)))
  {
    TBSYS_LOG(ERROR, "write_log_hook(log_id=[%ld,%ld))=>%d", start_cursor.log_id_, end_cursor.log_id_, ret);
  }
  else if (len > 0)
  {
    last_flush_log_time_ = tbsys::CTimeUtil::getTime();
  }
  return ret;
}

int ObLogWriterV3::switch_log_group(FLogPos& cur_pos, FLogPos& next_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
{
  int64_t switch_flag = 0;
  int ret = check_inner_stat();
  if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.set_log_position(cur_pos, 0, switch_flag, next_pos)))
  {
    TBSYS_LOG(ERROR, "set_log_position()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.write_end_log(cur_pos, ref_cnt, max_cmt_id)))
  {
    TBSYS_LOG(ERROR, "write_end_log()=>%d", ret);
  }

  return ret;
}

int ObLogWriterV3::write_and_end_log(const LogCommand cmd, const char* log_data, const int64_t data_len, FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
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
        cur_pos.rel_offset_ = cur_pos.rel_offset_ + (int32_t)data_len + (int32_t)entry.get_serialize_size();
        cur_group->sync_to_slave_ = true;
        if (OB_SUCCESS != (ret = set_group_start_timestamp(cur_pos)))
        {
          TBSYS_LOG(ERROR, "set_group_start_timestamp()=>%d", ret);
        }
        else if (OB_SUCCESS != (ret = write_log(cmd, log_data, data_len, cur_pos, ref_cnt, max_cmt_id)))
        {
          TBSYS_LOG(ERROR, "write_log(cmd=%d, log_data=%p[%ld])", cmd, log_data, data_len);
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

int ObLogWriterV3::store_log(const char* buf, const int64_t buf_len, const bool sync_to_slave)
{
  int ret = OB_SUCCESS;
  LogGroup* group = log_generator_.get_log_group(0);
  if (OB_SUCCESS != (ret = check_inner_stat()))
  {
    TBSYS_LOG(ERROR, "check_inner_stat()=>%d", ret);
  }
  else if (OB_SUCCESS != (ret = log_generator_.fill_batch(buf, buf_len)))
  {
    TBSYS_LOG(ERROR, "log_generator.fill_batch(%p[%ld])=>%d", buf, buf_len, ret);
  }
  else if (OB_SUCCESS != (ret = flush_log(group,TraceLog::get_logbuffer(), sync_to_slave, false)))
  {
    TBSYS_LOG(ERROR, "flush_log(buf=%p[%ld],sync_slave=%s)=>%d", buf, buf_len, STR_BOOL(sync_to_slave), ret);
  }
  return ret;
}

int ObLogWriterV3::switch_log_file(uint64_t &new_log_file_id, FLogPos& cur_pos, const int64_t max_cmt_id)
{
  int ret = check_inner_stat();
  new_log_file_id = 0;
  LogGroup* cur_group = get_log_group(cur_pos.group_id_);
  TBSYS_LOG(INFO, "log write switch log file");
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
        if (OB_SUCCESS != (ret = log_generator_.set_group_start_timestamp(cur_pos)))
        {
          TBSYS_LOG(ERROR, "set_group_start_timestamp()=>%d", ret);
        }
        else if (OB_SUCCESS != (ret = log_generator_.switch_log_file((int64_t&)new_log_file_id, cur_pos, max_cmt_id)))
        {
          TBSYS_LOG(ERROR, "log_generator.switch_log()=>%d", ret);
        }
        else
        {
          cur_group->sync_to_slave_ = true;
        }
        break;
      }
    }
  }
  return ret;
}

void ObLogWriterV3::set_disk_warn_threshold_us(const int64_t warn_us)
{
  disk_warn_threshold_us_ = warn_us;
}

void ObLogWriterV3::set_net_warn_threshold_us(const int64_t warn_us)
{
  net_warn_threshold_us_ = warn_us;
}

