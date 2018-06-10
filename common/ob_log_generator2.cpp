/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_group.cpp
 * @brief
 * ObLogGeneratorV2 is designed for storing log in memory transitorily.
 * Operation should set a position in the ObLogGeneratorV2 before filling log.
 *
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_03_29
 */
#include "ob_define.h"
#include "tbsys.h"
#include "ob_malloc.h"
#include "ob_log_generator2.h"
#include "ob_common_stat.h"
namespace oceanbase
{
  namespace common
  {
    FLogPos::FLogPos(): group_id_(0), rel_id_(0), rel_offset_(0)
    {}

    FLogPos::~FLogPos()
    {}

    void FLogPos::reset()
    {
      group_id_ = 0;
      rel_id_ = 0;
      rel_offset_ = 0;
    }

    char* FLogPos::to_str() const
    {
      static char buf[512];
      snprintf(buf, sizeof(buf), "FLogPos{group_id=%ld, rel_id=%d, rel_offset=%d}", group_id_, rel_id_, rel_offset_);
      buf[sizeof(buf)-1] = 0;
      return buf;
    }

    int64_t FLogPos::to_string(char *buf, const int64_t limit) const
    {
      int64_t len = -1;
      if (NULL == buf || 0 >= limit)
      {
        TBSYS_LOG(ERROR, "Null buf");
      }
      else if (0 >= (len = snprintf(buf, limit, "FLogPos{group_id=%ld, rel_id=%d, rel_offset=%d}",
                                    group_id_, rel_id_, rel_offset_)) || len >= limit)
      {
        TBSYS_LOG(ERROR, "Buf not enough, buf=%p[%ld]", buf, limit);
      }
      return len;
    }

    bool FLogPos::newer_than(const FLogPos &that) const
    {
      return group_id_ > that.group_id_ || (group_id_ == that.group_id_ && rel_id_ > that.rel_id_);
    }

    bool FLogPos::equal(const FLogPos &that) const
    {
      return group_id_ == that.group_id_ && rel_id_ == that.rel_id_ && rel_offset_ == that.rel_offset_;
    }

    int FLogPos::append(FLogPos &pos, int64_t len, int64_t limit, int64_t& switch_flag)
    {
      int err = OB_SUCCESS;

      if ( len < 0 || limit <= 0 )
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "append(len=%ld, limit=%ld)=>%d", len, limit, err);
      }
      else if (0 != len && rel_offset_ + len <= limit)
      {
        pos.group_id_ = group_id_;
        pos.rel_id_ = rel_id_ + 1;
        pos.rel_offset_ = rel_offset_ + (int32_t)len;
      }
      else
      {
//        if (0 != len && rel_offset_ + len > limit)
//        {
//          TBSYS_LOG(INFO,"test::zhouhuan append buffer is full so switch!!!!!");
//        }
        pos.group_id_ = group_id_;
        pos.rel_id_ = rel_id_;
        pos.rel_offset_ = rel_offset_;
        switch_flag = OB_GROUP_SWITCHED;
        //TBSYS_LOG(INFO, "test::zhouhuan Group[%ld] switched", group_id_);
      }
      return err;

    }

	LogGroup::LogGroup(): last_proc_time_(0), first_set_time_(0), first_fill_time_(0), last_fill_time_(0),
                          sync_to_slave_(false), need_ack_(false), ref_cnt_(0), len_(0), count_(0), ts_seq_(0),group_id_(0),
                          start_timestamp_(0), start_log_id_(0), last_ts_seq_(0),
                          last_timestamp_(0), log_cursor_seq_(0), start_cursor_(), end_cursor_(),
                          buf_(NULL)
    {
      memset(empty_log_, 0, sizeof(empty_log_));
      memset(nop_log_, 0, sizeof(nop_log_));

    }

    LogGroup::~LogGroup()
    {
      destroy();
    }

    bool LogGroup::is_clear() const
    {
      return 0 == ref_cnt_ && 0 == len_ && 0 == count_ && 0 == start_timestamp_ && start_cursor_.equal(end_cursor_);
    }

    void LogGroup::destroy()
    {
      if (NULL != buf_)
      {
        free(buf_);
        buf_ = NULL;
      }
    }

    void LogGroup::clear(int64_t group_size)
    {
      //TBSYS_LOG(INFO,"test::zhouhuan log_group clear start! group[%ld].start_log_id=[%ld]", group_id_,start_log_id_);
      sync_to_slave_ = false;
      need_ack_ = false;
      ref_cnt_ = 0;
      len_ = 0;
      count_ = 0;
      start_timestamp_ = 0;
      start_log_id_ = 0;
      last_ts_seq_ = 0;
      last_timestamp_ = 0;
      log_cursor_seq_ = 0;
      group_id_ += group_size;
      __sync_synchronize();
      ts_seq_ = group_id_;
      //TBSYS_LOG(INFO,"test::zhouhuan log_group clear after! group[%ld].start_log_id=[%ld],ts_seq_=%ld", group_id_,start_log_id_, ts_seq_);

    }

    int LogGroup::reset(int64_t group_size)
    {
      int err = OB_SUCCESS;
      if (!is_clear())
      {
        err = OB_LOG_NOT_CLEAR;
        TBSYS_LOG(ERROR, "group_not_clear, start_cursor=[%s], end_cursor=[%s] len=%ld, start_log_id=%ld, start_timestamp_=%ld",
                  to_cstring(start_cursor_),  to_cstring(end_cursor_),len_, start_log_id_, start_timestamp_);
      }
      else
      {
        clear(group_size);
        start_cursor_.reset();
        end_cursor_.reset();
      }
      return err;
    }

    int LogGroup::check_state() const
    {
      int err = OB_SUCCESS;
      if (!is_inited())
      {
        err = OB_NOT_INIT;
      }
      return err;
    }

    bool LogGroup::is_inited() const
    {
      return NULL != buf_;
    }

    void LogGroup::set_start_cursor(ObLogCursor& log_cursor)
    {
      start_cursor_ = log_cursor;
    }

    void LogGroup::set_end_cursor(ObLogCursor& log_cursor)
    {
      end_cursor_ = log_cursor;
    }

    void LogGroup::set_last_proc_time(const int64_t proc_time)
    {
      last_proc_time_ = proc_time;
    }

    int64_t LogGroup::get_last_proc_time() const
    {
      return last_proc_time_;
    }

    void LogGroup::set_first_set_time(const int64_t proc_time)
    {
      first_set_time_ = proc_time;
    }

    int64_t LogGroup::get_first_set_time() const
    {
      return first_set_time_;
    }

    void LogGroup::set_first_fill_time(const int64_t cur_time)
    {
      first_fill_time_ = cur_time;
    }
    int64_t LogGroup::get_first_fill_time() const
    {
      return first_fill_time_;
    }
    void LogGroup::set_last_fill_time(const int64_t cur_time)
    {
      last_fill_time_ = cur_time;
    }
    int64_t LogGroup::get_last_fill_time() const
    {
      return last_fill_time_;
    }

    inline int64_t get_align_padding_size(const int64_t x, const int64_t mask)
    {
      return -x & mask;
    }

    static bool is_aligned(int64_t x, int64_t mask)
    {
      return !(x & mask);
    }

    static int64_t calc_align_log_len(int64_t pos, int64_t min_log_size)
    {
      ObLogEntry entry;
      int64_t header_size = entry.get_serialize_size();
      return get_align_padding_size(pos + header_size + min_log_size, ObLogGeneratorV2::LOG_FILE_ALIGN_MASK) + min_log_size;
    }

    char ObLogGeneratorV2::eof_flag_buf_[LOG_FILE_ALIGN_SIZE] __attribute__ ((aligned(DIO_ALIGN_SIZE)));
    static class EOF_FLAG_BUF_CONSTRUCTOR
    {
      public:
        EOF_FLAG_BUF_CONSTRUCTOR() {
          const char* mark_str = "end_of_log_file";
          memset(ObLogGeneratorV2::eof_flag_buf_, 0, sizeof(ObLogGeneratorV2::eof_flag_buf_));
          for(int64_t i = 0; i + strlen(mark_str) < sizeof(ObLogGeneratorV2::eof_flag_buf_); i += strlen(mark_str))
          {
            strcpy(ObLogGeneratorV2::eof_flag_buf_ + i, mark_str);
          }
        }
        ~EOF_FLAG_BUF_CONSTRUCTOR() {}
    } eof_flag_buf_constructor_;

    ObLogGeneratorV2::ObLogGeneratorV2():log_file_max_size_(1<<24), log_buf_len_(0), log_buf_(NULL), next_pos_()

    {
      /*for (int i = 0; i < group_size_; ++i)
      {
        log_buf_[i].group_id_ = i;
      }*/
    }

    ObLogGeneratorV2:: ~ObLogGeneratorV2()
    {
      for (int i = 0; i < group_size_; ++i)
      {
        log_buf_[i].destroy();
      }
    }

    int ObLogGeneratorV2::group_alloc(int64_t log_buf_size)
    {
      int err = OB_SUCCESS;
      int sys_err = 0;
      for(int i = 0; i < group_size_; ++i)
      {
        if(0 != (sys_err = posix_memalign((void**)&log_buf_[i].buf_, LOG_FILE_ALIGN_SIZE, log_buf_size + LOG_FILE_ALIGN_SIZE)))
        {
          err = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(ERROR, "block(%d)-posix_memalign(%ld):%s", i, log_buf_size, strerror(sys_err));
          break;
        }
        else
        {
          log_buf_[i].ts_seq_ = i;
        }
      }
      return err;
    }

    int ObLogGeneratorV2::init(int64_t log_buf_size, int64_t log_file_max_size, const ObServer *server, int64_t group_size)
    {
      int err = OB_SUCCESS;
      if (is_inited())
      {
        err = OB_INIT_TWICE;
      }
      else if(log_buf_size <= 0 || log_file_max_size <= 0 || log_file_max_size < 2 * LOG_FILE_ALIGN_SIZE)
      {
        err = OB_INVALID_ARGUMENT;
      }
      //add hushuang [scalble commit]20160630
      else if (OB_SUCCESS != (err = init_group(group_size)))
      {
        err = OB_ERROR;
      }
      //add e
      else if (OB_SUCCESS != (err = group_alloc(log_buf_size)))
      {
        err = OB_ALLOCATE_MEMORY_FAILED;
        TBSYS_LOG(ERROR, "group_alloc(%ld)=>%d", log_buf_size, err);
      }
      else
      {
        if (NULL != server)
        {
          debug_log_.server_ = *server;
        }
        log_file_max_size_ = log_file_max_size;
        log_buf_len_ = log_buf_size + LOG_FILE_ALIGN_SIZE;
        TBSYS_LOG(INFO, "log_generator.init(log_buf_size=%ld, log_file_max_size=%ld)", log_buf_size, log_file_max_size);
      }
      return err;
    }

    int ObLogGeneratorV2::reset()
    {
      int err = OB_SUCCESS;
      log_file_max_size_ = 0;
      log_buf_len_ = 0;
      for (int i = 0; i < group_size_; ++i)
      {
        if(OB_SUCCESS != (err = log_buf_[i].reset(group_size_)))
        {
          err = OB_ERROR;
          TBSYS_LOG(ERROR, "group[%d].reset() => %d", i ,err);
          break;
        }
      }
      return err;
    }

    bool ObLogGeneratorV2::is_log_start(int64_t group_id)
    {
      LogGroup* log_group = get_log_group(group_id);
      return log_group->start_cursor_.is_valid();
    }

    bool ObLogGeneratorV2::is_clear() const
    {
      bool ret = true;
      for (int i = 0; i < group_size_; ++i)
      {
        if (!log_buf_[i].is_clear())
        {
          ret = false;
          break;
        }
      }
      return ret;
    }

    /*int64_t ObLogGeneratorV2::to_string(char* buf, const int64_t len) const
    {
      int64_t pos = 0;
      databuff_printf(buf, len, pos, "LogGenerator([%s,%s], len=%ld, frozen=%s)",
                      to_cstring(start_cursor_), to_cstring(end_cursor_), pos_, STR_BOOL(is_frozen_));
      return pos;
    }*/

    int ObLogGeneratorV2::start_log(const ObLogCursor &log_cursor)
    {
      int err = OB_SUCCESS;
      FLogPos cur_pos = get_next_pos();
      LogGroup* log_group = get_log_group(cur_pos.group_id_);
      if (!log_cursor.is_valid())
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "start_log_cursor.is_valid()=>false");
      }
      else if (log_group->start_cursor_.is_valid())
      {
        err = OB_INIT_TWICE;
        TBSYS_LOG(ERROR, "group[0].cusor=[%ld] already inited.", log_group->start_cursor_.log_id_);
      }
      else
      {
        log_group->start_log_id_ = log_cursor.log_id_ - 1;
        log_group->last_timestamp_ = tbsys::CTimeUtil::getTime();
        log_group->start_cursor_ = log_cursor;
        log_group->end_cursor_ = log_cursor;
        __sync_synchronize();
        //log_group->log_cursor_seq_ = next_pos_.group_id_ + log_group->READY;
        log_group->last_ts_seq_ = cur_pos.group_id_ + log_group->READY;
        TBSYS_LOG(INFO, "ObLogGeneratorV2::start_log(log_cursor=%s)", to_cstring(log_cursor));
      }
      return err;
    }

    //add hushuang [scalable commit]20160630
    int ObLogGeneratorV2::init_group(int64_t size)
    {
      int ret = OB_SUCCESS;
      char* ptr = NULL;
      if(0 >= size)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if(NULL == (ptr = reinterpret_cast <char*>(ob_malloc((sizeof(LogGroup)) * size, ObModIds::BLOCK_ALLOC))))
      {
        ret = OB_ERROR;
      }
      else
      {
        group_size_ = size;
        log_buf_ = reinterpret_cast <LogGroup*>(ptr);
        for (int64_t i = 0; i < size; i++)
        {
          LogGroup* group = new(ptr + i * sizeof(LogGroup))LogGroup();
          if(NULL == group)
          {
            ret = OB_ERROR;
          }
        }
        for (int i = 0; i < group_size_; ++i)
        {
          log_buf_[i].group_id_ = i;
        }

      }
      return ret;
    }
    //add e

    bool ObLogGeneratorV2::check_log_size(const int64_t size) const
    {
      ObLogEntry entry;
      bool ret = (size > 0 && size + LOG_BUF_RESERVED_SIZE + entry.get_serialize_size() <= log_buf_len_);
      if (!ret)
      {
        TBSYS_LOG(ERROR, "log_size[%ld] + reserved[%ld] + header[%ld] > log_buf_len[%ld]",
                  size, LOG_BUF_RESERVED_SIZE, entry.get_serialize_size(), log_buf_len_);
      }
      return ret;
    }

    LogGroup* ObLogGeneratorV2::get_log_group(int64_t group_id)
    {
      //TBSYS_LOG(ERROR, "test::zhouhuan seq=>%ld, group_id = %ld", log_buf_[group_id % GROUP_ARRAY_SIZE].ts_seq_,group_id);
      return &log_buf_[group_id % group_size_];
    }

    int ObLogGeneratorV2::set_log_position(FLogPos& cur_pos, int64_t len, int64_t& switch_flag, FLogPos& next_pos)
    {
      int err = OB_SUCCESS;
      FLogPos old;
      FLogPos tmp;

      while(true)
      {
        LOAD128(old, &next_pos_);
        //TBSYS_LOG(ERROR, "test::zhouhuan before set_log_position cur_pos=[%s], next_pos=[%s], old=[%s]",
          //        to_cstring(cur_pos),  to_cstring(next_pos), to_cstring(old));
        ///calculate the Pos
        if (OB_SUCCESS != (err = old.append(cur_pos, len, log_buf_len_ - LOG_BUF_RESERVED_SIZE, switch_flag)))
        {
          TBSYS_LOG(ERROR, "append()=>%d", err);
          //break;
        }
        else if (OB_GROUP_SWITCHED == switch_flag)
        {
          if (0 != len)
          {
            next_pos.group_id_ = cur_pos.group_id_ + 1;
            next_pos.rel_id_ = 1;
            next_pos.rel_offset_ = (int32_t) len;
          }
          else
          {
            next_pos.group_id_ = cur_pos.group_id_ + 1;
            next_pos.rel_id_ = 0;
            next_pos.rel_offset_ = 0;
          }
          tmp = next_pos;
        }
        else
        {
          tmp = cur_pos;
        }

       // next_pos_ = tmp;

        ///set the position
        if (CAS128(&next_pos_, old, tmp))
        {
          break;
        }
      }
//     TBSYS_LOG(ERROR, "test::zhouhuan set_log_position success! cur_pos=[%s], next_pos=[%s], next_pos_=[%s]",
//                to_cstring(cur_pos),  to_cstring(next_pos), to_cstring(next_pos_));
      return err;
    }

    bool ObLogGeneratorV2::switch_group(FLogPos& cur_pos)
    {
      int ret = false;
      FLogPos next_pos;
      next_pos.group_id_ = cur_pos.group_id_ + 1;
      next_pos.rel_id_ = 0;
      next_pos.rel_offset_ = 0;
      if (CAS128(&next_pos_, cur_pos, next_pos))
      {
        ret = true;
      }
      return ret;
    }

    int ObLogGeneratorV2::set_group_start_timestamp(FLogPos& cur_pos)
    {
      int err = OB_SUCCESS;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      if (OB_SUCCESS != (err = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else
      {
        while(true)
        {
         /*TBSYS_LOG(INFO, "test::zhouhuan group_id[%p]: %ld, last_ts_seq_[%p]: %ld, cur_pos[%p]: %d",
                  &(cur_pos.group_id_), cur_pos.group_id_,
                  &(cur_group->last_ts_seq_), cur_group->last_ts_seq_,
                  &(cur_pos.rel_id_), cur_pos.rel_id_);*/

          if (cur_group->last_ts_seq_ != cur_pos.group_id_ + cur_group->READY)
          {
            usleep(1);
          }
          else
          {
            //add by zhouhuan for __all_server_stat 20160530
            int64_t cur_time = tbsys::CTimeUtil::getTime();
            if (cur_group->last_proc_time_ == 0)
            {
              cur_group->set_last_proc_time(cur_time);
            }
            else
            {
              OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_UTIME, cur_time - cur_group->get_last_proc_time());
              //OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_STIME, cur_time - cur_group->get_first_set_time());
              cur_group->set_last_proc_time(cur_time);
            }
            __sync_synchronize();
            //add:e
            int64_t trans_id = tbsys::CTimeUtil::getTime();
            cur_group->start_timestamp_ = (cur_group->last_timestamp_ + 1) > trans_id ? cur_group->last_timestamp_ + 1 : trans_id;
            __sync_synchronize();
            cur_group->ts_seq_ = cur_pos.group_id_ + cur_group->READY;
            break;
          }
        }
        //add:e
      }

      return err;
    }

    int64_t ObLogGeneratorV2::get_trans_id(FLogPos& cur_pos)
    {
      int64_t trans_id;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      while(true)
      {
        /*TBSYS_LOG(INFO, "test::zhouhuan group_id[%p]: %ld, ts_seq_[%p]: %ld, cur_pos[%p]: %d",
                  &(cur_pos.group_id_), cur_pos.group_id_,
                  &(cur_group->ts_seq_), cur_group->ts_seq_,
                  &(cur_pos.rel_id_), cur_pos.rel_id_);*/
        if (cur_group->ts_seq_ != cur_pos.group_id_ + cur_group->READY)
        {
          usleep(1);
        }
        else
        {
          trans_id = cur_group->start_timestamp_ + cur_pos.rel_id_;
          break;
        }
      }
      return trans_id;
    }

//    void ObLogGeneratorV2::switch_next_pos()
//    {
//      ++next_pos_.group_id_;
//      next_pos_.rel_id_ = 0;
//      next_pos_.rel_offset_ = 0;
//    }

    static int parse_log_buffer(const char* log_data, int64_t data_len, const ObLogCursor& start_cursor, ObLogCursor& end_cursor, bool check_data_integrity = false)
    {
      int err = OB_SUCCESS;
      int64_t pos = 0;
      int64_t tmp_pos = 0;
      int64_t file_id = 0;
      ObLogEntry log_entry;
      end_cursor = start_cursor;
      if (NULL == log_data || data_len <= 0 || !start_cursor.is_valid())
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "invalid argument, log_data=%p, data_len=%ld, start_cursor=%s",
                  log_data, data_len, to_cstring(start_cursor));
      }

      while (OB_SUCCESS == err && pos < data_len)
      {
        if (OB_SUCCESS != (err = log_entry.deserialize(log_data, data_len, pos)))
        {
          TBSYS_LOG(ERROR, "log_entry.deserialize(log_data=%p, data_len=%ld, pos=%ld)=>%d", log_data, data_len, pos, err);
        }
        else if (pos + log_entry.get_log_data_len() > data_len)
        {
          err = OB_LAST_LOG_RUINNED;
          TBSYS_LOG(ERROR, "last_log broken, cursor=[%s,%s]", to_cstring(start_cursor), to_cstring(end_cursor));
        }
        else if (check_data_integrity && OB_SUCCESS != (err = log_entry.check_data_integrity(log_data + pos)))
        {
          TBSYS_LOG(ERROR, "log_entry.check_data_integrity()=>%d", err);
        }
        else
        {
          tmp_pos = pos;
        }

        if (OB_SUCCESS != err)
        {}
        else if (OB_LOG_SWITCH_LOG == log_entry.cmd_
                 && !(OB_SUCCESS == (err = serialization::decode_i64(log_data, data_len, tmp_pos, (int64_t*)&file_id)
                                     && start_cursor.log_id_ == file_id)))
        {
          TBSYS_LOG(ERROR, "decode switch_log failed(log_data=%p, data_len=%ld, pos=%ld)=>%d", log_data, data_len, tmp_pos, err);
        }
        else
        {
          pos += log_entry.get_log_data_len();
          if (OB_SUCCESS != (err = end_cursor.advance(log_entry)))
          {
            TBSYS_LOG(ERROR, "end_cursor[%ld].advance(%ld)=>%d", end_cursor.log_id_, log_entry.seq_, err);
          }
        }
      }
      if (OB_SUCCESS == err && pos != data_len)
      {
        err = OB_ERR_UNEXPECTED;
        TBSYS_LOG(ERROR, "pos[%ld] != data_len[%ld]", pos, data_len);
      }

      if (OB_SUCCESS != err)
      {
        hex_dump(log_data, static_cast<int32_t>(data_len), TBSYS_LOG_LEVEL_WARN);
      }
      return err;
    }

    int ObLogGeneratorV2:: fill_batch(const char* buf, int64_t len)
    {
      int err = OB_SUCCESS;
      ObLogCursor start_cursor, end_cursor;
      int64_t reserved_len = LOG_FILE_ALIGN_SIZE;
      start_cursor = log_buf_[0].end_cursor_;
      if (OB_SUCCESS != (err = check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else if (NULL == buf || len <= 0)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if (0 != (len & LOG_FILE_ALIGN_MASK))
      {
        err = OB_LOG_NOT_ALIGN;
        TBSYS_LOG(ERROR, "len[%ld] is not align[mask=%lx]", len, LOG_FILE_ALIGN_SIZE);
      }
      else if (log_buf_[0].len_ != 0)
      {
        err = OB_LOG_NOT_CLEAR;
        TBSYS_LOG(ERROR, "fill_batch(len[%ld] != 0, end_cursor=%s, buf=%p[%ld])",
                  log_buf_[0].len_, to_cstring(log_buf_[0].end_cursor_), buf, len);
      }
      else if (len + reserved_len > log_buf_len_)
      {
        err = OB_BUF_NOT_ENOUGH;
        TBSYS_LOG(ERROR, "len[%ld] + reserved_len[%ld] > log_buf_len[%ld]",
                  len, reserved_len, log_buf_len_);
      }
      else if (OB_SUCCESS != (err = parse_log_buffer(buf, len, start_cursor, end_cursor)))
      {
        TBSYS_LOG(ERROR, "parse_log_buffer(buf=%p[%ld], cursor=%s)=>%d", buf, len, to_cstring(log_buf_[0].end_cursor_), err);
      }
      else
      {
        //TBSYS_LOG(WARN,"test::zhouhuan Replay fill batch start_cursor=[%s] end_cursor=[%s]", to_cstring(start_cursor), to_cstring(end_cursor));
        memcpy(log_buf_[0].buf_, buf, len);
        log_buf_[0].len_ = len;
        log_buf_[0].end_cursor_ = end_cursor;
        if(len + (int64_t)sizeof(eof_flag_buf_) > log_buf_len_)
        {
          err = OB_ERR_UNEXPECTED;
          TBSYS_LOG(ERROR, "no buf to append eof flag, len=%ld, log_buf_len=%ld", len, log_buf_len_);
        }
        else
        {
          memcpy(log_buf_[0].buf_ + len, eof_flag_buf_, sizeof(eof_flag_buf_));
        }

      }
      return err;
    }

    int ObLogGeneratorV2::write_log(const LogCommand cmd, const char* log_data, const int64_t data_len,
                                    const FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      if (OB_SUCCESS != (err = check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else if (NULL == log_data || data_len <= 0)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_SUCCESS != (err = do_write_log(cmd, log_data, data_len, cur_pos, LOG_BUF_RESERVED_SIZE, ref_cnt, max_cmt_id)))
      {
        TBSYS_LOG(WARN, "do_write_log(cmd=%d, len=%ld)=>%d", cmd, data_len, err);
      }
      return err;
    }

    int ObLogGeneratorV2::write_end_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      bool switch_flag = false;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      if (OB_SUCCESS != (err = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else
      {
        while(true)
        {
          if(cur_group->ts_seq_ != cur_pos.group_id_ + cur_group->READY)
          {
            usleep(1);
          }
          else
          {
            if (cur_group->start_cursor_.offset_ + log_buf_len_ <= log_file_max_size_)
            {
              if(OB_SUCCESS != (err = write_nop(cur_pos, ref_cnt, max_cmt_id, true)))
              {
                TBSYS_LOG(ERROR, "write_nop()=>%d", err);
              }
            }
            else
            {
              if (OB_SUCCESS != (err = switch_log(cur_pos, ref_cnt, max_cmt_id)))
              {
                TBSYS_LOG(ERROR, "switch_log()=>%d", err);
              }
              else
              {
                switch_flag = true;
              }
            }

            if(OB_SUCCESS != err)
            {
              TBSYS_LOG(ERROR, "write_end_log()=>%d", err);
            }
            else if (OB_SUCCESS != (err = append_eof(cur_pos, switch_flag, ref_cnt)))
            {
              TBSYS_LOG(ERROR, "write_eof()=>%d", err);
            }
            break;
          }
        }

      }
      return err;
    }

    int ObLogGeneratorV2::get_log(ObLogCursor& start_cursor, ObLogCursor& end_cursor, char*& buf, int64_t& len, LogGroup* cur_group)
    {
      int err = OB_SUCCESS;
      if (OB_SUCCESS != (err = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else
      {
        start_cursor = cur_group->start_cursor_;
        end_cursor = cur_group->end_cursor_;
        buf = cur_group->buf_;
        len = cur_group->len_;
      }
      //TBSYS_LOG(ERROR, "test::zhouhuan:get_log finished! start_cursor=[%s], end_cusror=[%s], len=%ld",
        //        to_cstring(cur_group->start_cursor_), to_cstring(cur_group->end_cursor_), cur_group->len_);
      return err;
    }

    int ObLogGeneratorV2::commit(LogGroup* cur_group, const ObLogCursor& end_cursor, bool is_master)
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != (ret = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", ret);
      }
      else if (!end_cursor.equal(cur_group->end_cursor_))
      {
        ret = OB_ERR_UNEXPECTED;
        TBSYS_LOG(ERROR, "end_cursor[%ld] != end_cursor_[%ld]", end_cursor.log_id_, cur_group->end_cursor_.log_id_);
      }
      else
      {
        //TBSYS_LOG(WARN,"test::zhouhuan commit! group_id=[%ld], group_len=[%f], group_size_ =[%ld]",cur_group->group_id_, float(cur_group->len_) /(1024*1024),  group_size_);
        //TBSYS_LOG(INFO, "test::zhouhuan master commit group=[%ld] is_master=%d", cur_group->group_id_, is_master);
        cur_group->start_cursor_ = cur_group->end_cursor_;
        cur_group->len_ = 0;
        if (is_master)
        {
          cur_group->clear(group_size_);
        }
        //TBSYS_LOG(ERROR, "test::zhouhuan:Group reuse!! group[%ld]->ts_seq=[%ld]", cur_group->group_id_, cur_group->ts_seq_ );
      }
      return ret;
    }

    int ObLogGeneratorV2::switch_log_file(int64_t& new_file_id, FLogPos& cur_pos, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      int64_t ref_cnt = 0;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      if (OB_SUCCESS != (err = check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else if (OB_SUCCESS != (err = switch_log(cur_pos, ref_cnt, max_cmt_id)))
      {
        TBSYS_LOG(ERROR, "switch_log()=>%d", err);
      }
      else if (OB_SUCCESS != (err = append_eof(cur_pos, true, ref_cnt)))
      {
        TBSYS_LOG(ERROR, "append_eof()=>%d", err);
      }
      else
      {
        new_file_id = cur_group->start_cursor_.file_id_ + 1;
      }
      return err;
    }

    int ObLogGeneratorV2::gen_keep_alive(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int ret = OB_SUCCESS;
      //LogGroup* cur_group = get_log_group(cur_pos.group_id_);
//      while(true)
//      {
//        if(cur_group->ts_seq_ != cur_pos.group_id_)
//        {
//          usleep(1);
//        }
//        else
//        {
          if (0 == cur_pos.rel_id_)
          {
            if (OB_SUCCESS != (ret = set_group_start_timestamp(cur_pos)))
            {
              TBSYS_LOG(ERROR, "set_group_start_timestamp()=>%d", ret);
              //break;
            }
          }
          if (OB_SUCCESS == ret)
          {
            if (OB_SUCCESS != (ret = write_end_log(cur_pos, ref_cnt, max_cmt_id)))
            {
              TBSYS_LOG(ERROR, "write_end_log()=>%d", ret);
            }
          }
//          break;
//        }
//      }
      return ret;
    }

    int ObLogGeneratorV2::get_start_log_cursor(ObLogCursor& log_cursor, int64_t group_id)
    {
      int err = OB_SUCCESS;
      LogGroup* cur_group = get_log_group(group_id);
      if (OB_SUCCESS != (err = check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else
      {
        log_cursor = cur_group->start_cursor_;
      }
      return err;
    }

    bool ObLogGeneratorV2::is_inited() const
    {
      //bool err = true;
      //for (int i =0; i < group_size_; ++i)
      //{

        //if(!( NULL != log_buf_[i].buf_ && log_buf_len_ > 0))
        //if(NULL != log_buf_[i] && log_buf_len_ > 0)
        //{
          //err = false;
          //break;
        //}
     // }
      //return err;
    return NULL != log_buf_ && log_buf_len_ > 0;
    }

    int ObLogGeneratorV2::check_state() const
    {
      int err = OB_SUCCESS;
      if (!is_inited())
      {
        err = OB_NOT_INIT;
      }
      return err;
    }

    static int serialize_log_entry2(char* buf, int64_t& pos, ObLogEntry& entry, const char* log_data, const int64_t data_len, const int64_t buf_len)
    {
      int err = OB_SUCCESS;
      if (NULL == buf  || 0 > pos || NULL == log_data || 0 >= data_len || 0 >= buf_len)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "serialize_log_entry(buf=%p, pos=%ld, log_data=%p, data_len=%ld, buf_len=%ld)=>%d",
                  buf, pos, log_data, data_len, buf_len, err);
      }
      else if (OB_SUCCESS != (err = entry.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(ERROR, "entry.serialize(buf=%p, pos=%ld, capacity=%ld)=>%d",
                  buf, pos, buf_len, err);
      }
      else
      {
        memcpy(buf + pos, log_data, data_len);
        pos += data_len;
      }
      return err;
    }

    static int generate_log2(char* buf, const LogCommand cmd, const char* log_data, const int64_t data_len,
                            const FLogPos& cur_pos, const int64_t log_id, int64_t buf_len, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      ObLogEntry entry;
      int64_t pos = (int64_t)cur_pos.rel_offset_ - entry.get_serialize_size() - data_len;
      if (NULL == buf || 0 > pos || NULL == log_data || 0 >= data_len || 0 >= buf_len)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "generate_log(buf=%p, pos=%ld, log_data=%p, data_len=%ld, buf_len=%ld)=>%d",
                  buf, pos, log_data, data_len, buf_len, err);
      }
      else if (OB_SUCCESS != (err = entry.set_entry(log_data, data_len, cmd, log_id + cur_pos.rel_id_, max_cmt_id)))
      {
        TBSYS_LOG(ERROR, "cur_pos[%s].set_entry()=>%d", to_cstring(cur_pos), err);
      }
      else if (OB_SUCCESS != (err = serialize_log_entry2(buf, pos, entry, log_data, data_len, buf_len)))
      {
        TBSYS_LOG(DEBUG, "serialize_log_entry(buf=%p, entry[id=%ld], data_len=%ld, buf_len=%ld)=>%d",
                  buf, entry.seq_, data_len, buf_len, err);
      }
      else if (pos != cur_pos.rel_offset_)
      {
        err = OB_ERROR;
        TBSYS_LOG(ERROR, "generate_log pos=[%ld] != cur_pos.rel_offset[%d]", pos, cur_pos.rel_offset_);
      }
      //TBSYS_LOG(INFO,"test::zhouhuan log_id + cur_pos.rel_id_ =%ld  data_len=%ld buf_len=%ld log_id=%ld cur_pos.rel_id_=%d",
      //          log_id + cur_pos.rel_id_,data_len,buf_len ,log_id,cur_pos.rel_id_);
      return err;
    }

    int ObLogGeneratorV2::do_write_log(const LogCommand cmd, const char* log_data, const int64_t data_len,
                    const FLogPos& cur_pos, const int64_t reserved_len, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      if (OB_SUCCESS != (err = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else if (NULL == log_data || data_len <= 0)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        while(true)
        {
          if (cur_group->ts_seq_ != cur_pos.group_id_ + cur_group->READY)
          {
            usleep(1);
          }
          else
          {
            if(OB_SUCCESS != (err = generate_log2(cur_group->buf_, cmd, log_data, data_len, cur_pos,
                                                            cur_group->start_log_id_, log_buf_len_ - reserved_len, max_cmt_id)))
            {
              TBSYS_LOG(WARN, "generate_log()=>%d", err);
            }
            else
            {
             // TBSYS_LOG(ERROR, "test::zhouhuan:before atomic_inc cur_group->ref_cnt_=%ld, ref_cnt=%d", cur_group->ref_cnt_, ref_cnt);
              ref_cnt = ATOMIC_INC(&(cur_group->ref_cnt_));
             //TBSYS_LOG(INFO, "test::zhouhuan ref_cnt group_id=%ld, ref_cnt=%ld, rel_id=%d", cur_group->group_id_, ref_cnt, cur_pos.rel_id_);
            }
            break;
          }
        }
      }
      return err;
    }

    int ObLogGeneratorV2::write_nop(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id, bool force_write)
    {
      int err = OB_SUCCESS;
      int64_t pos = 0;
      int64_t data_len = 0;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      ObLogEntry entry;

      TBSYS_LOG(TRACE, "try write nop: force_write=%s", STR_BOOL(force_write));
      if (is_aligned((int64_t)cur_pos.rel_offset_, LOG_FILE_ALIGN_MASK) && !force_write)
      {
        TBSYS_LOG(TRACE, "The log is aligned");
      }
      else if (OB_SUCCESS != (err = debug_log_.advance()))
      {
        TBSYS_LOG(ERROR, "debug_log.advance()=>%d", err);
      }
      else if (OB_SUCCESS != (err = debug_log_.serialize(cur_group->nop_log_, sizeof(cur_group->nop_log_), pos)))
      {
        TBSYS_LOG(ERROR, "serialize_nop_log()=>%d", err);
      }
      else
      {
        data_len = calc_align_log_len((int64_t)cur_pos.rel_offset_, pos);
        cur_pos.rel_offset_ += (int32_t)entry.get_serialize_size() + (int32_t)data_len;
        ++cur_pos.rel_id_;
        if (OB_SUCCESS != (err = do_write_log(OB_LOG_NOP, cur_group->nop_log_, data_len , cur_pos, 0, ref_cnt, max_cmt_id)))
        {
          TBSYS_LOG(ERROR, "write_log(OB_LOG_NOP, len=%ld)=>%d", data_len, err);
        }
      }

      return err;
    }

    int ObLogGeneratorV2::switch_log(FLogPos &cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      int64_t buf_pos = 0;
      int64_t data_len = 0;
      LogGroup* log_group = get_log_group(cur_pos.group_id_);
      ObLogEntry entry;
      if (OB_SUCCESS != (err = check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
      }
      else if (OB_SUCCESS != (err = serialization::encode_i64(log_group->empty_log_, sizeof(log_group->empty_log_), buf_pos, log_group->start_cursor_.file_id_ + 1)))
      {
        TBSYS_LOG(ERROR, "encode_i64(file_id_=%ld)=>%d", log_group->start_cursor_.file_id_, err);
      }
      else
      {
        data_len = calc_align_log_len((int64_t)cur_pos.rel_offset_, buf_pos);
        cur_pos.rel_offset_ += (int32_t)entry.get_serialize_size() + (int32_t)data_len;
        ++cur_pos.rel_id_;
        if (OB_SUCCESS != (err = do_write_log(OB_LOG_SWITCH_LOG, log_group->empty_log_, data_len, cur_pos, 0, ref_cnt, max_cmt_id)))
        {
          TBSYS_LOG(ERROR, "write(OB_LOG_SWITCH_LOG, len=%ld)=>%d", data_len, err);
        }
      }
      return err;
    }

    int ObLogGeneratorV2::append_eof(const FLogPos& cur_pos, bool switch_flag, int64_t& ref_cnt)
    {
      int err = OB_SUCCESS;
      int64_t next_group_id = cur_pos.group_id_ + 1;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      LogGroup* next_group = get_log_group(next_group_id);
     // printBuf();
      //TBSYS_LOG(ERROR, "test::zhouhuan group[%ld].ts_seq=%ld", cur_group->group_id_, cur_group->ts_seq_);
      if ((int64_t)cur_pos.rel_offset_ + (int64_t)sizeof(eof_flag_buf_) > log_buf_len_)
      {
        err = OB_ERR_UNEXPECTED;
        TBSYS_LOG(ERROR, "no buf to append eof flag, pos=%d, log_buf_len=%ld", cur_pos.rel_offset_, log_buf_len_);
      }
      else
      {
        memcpy(cur_group->buf_ + cur_pos.rel_offset_, eof_flag_buf_, sizeof(eof_flag_buf_));
      }

      if(OB_SUCCESS == err)
      {
        //set len_ and count_ of cur_group
        //cur_group->len_ = (int64_t)cur_pos.rel_offset_;
        //cur_group->count_ = (int64_t)cur_pos.rel_id_;
        //TBSYS_LOG(ERROR, "test::zhouhuan cur group id %ld, count = %ld", cur_group->group_id_, cur_group->count_);
        if(true != switch_flag)
        {
          cur_group->end_cursor_.file_id_ = cur_group->start_cursor_.file_id_;
          cur_group->end_cursor_.log_id_ = cur_group->start_cursor_.log_id_ + (int64_t)cur_pos.rel_id_;
          cur_group->end_cursor_.offset_ = cur_group->start_cursor_.offset_ + (int64_t)cur_pos.rel_offset_;
        }
        else
        {
          cur_group->end_cursor_.file_id_ = cur_group->start_cursor_.file_id_ + 1;
          cur_group->end_cursor_.log_id_ = cur_group->start_cursor_.log_id_ + (int64_t)cur_pos.rel_id_;
          cur_group->end_cursor_.offset_ = 0;
        }
        cur_group->log_cursor_seq_ = cur_pos.group_id_ + cur_group->READY;
//        TBSYS_LOG(INFO, "test::zhouhuan set end_cusor,group[%ld].ts_seq=%ld, next_group[%ld].ts_seq=%ld, next_group_id=%ld",
//                  cur_group->group_id_,cur_group->ts_seq_, next_group->group_id_, next_group->ts_seq_, next_group_id);

        //set start_log_id last_ts_seq start_log_cursor of next_group
        while(true)
        {
          if (next_group->ts_seq_ != next_group_id)
          {
            usleep(1);
          }
          else
          {
//            //add by zhouhuan for __all_server_stat 20160531
//            int64_t cur_time = tbsys::CTimeUtil::getTime();
//            OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_ATIME, cur_time - next_group->get_last_proc_time());
//            next_group->set_last_proc_time(cur_time);
//            //add:e
            next_group->start_log_id_ = cur_group->start_log_id_ + (int64_t)cur_pos.rel_id_;
            next_group->last_timestamp_ = cur_group->start_timestamp_ + (int64_t)cur_pos.rel_id_;
            next_group->start_cursor_ = cur_group->end_cursor_ ;
            __sync_synchronize();
            next_group->last_ts_seq_ = next_group_id + next_group->READY;

            //TBSYS_LOG(ERROR, "test::zhouhuan set next group message,next_group[%ld],next_cursor[%s,%s]", next_group->group_id_,
             //        to_cstring(next_group->start_cursor_), to_cstring(next_group->end_cursor_));
            break;
          }
        }
        cur_group->len_ = (int64_t)cur_pos.rel_offset_;
        cur_group->count_ = (int64_t)cur_pos.rel_id_ + 1;
        ref_cnt = ATOMIC_INC(&(cur_group->ref_cnt_));
        //TBSYS_LOG(INFO, "test::zhouhuan ref_cnt group_id=%ld ref_cnt=%ld count=%ld rel_id=%d",cur_group->group_id_, ref_cnt, cur_group->count_, cur_pos.rel_id_);
        //TBSYS_LOG(ERROR, "test::zhouhuan append_eof:cur_gid_lid_cnt=[%ld,%ld,%ld] next_group_id=[%ld] next_start_log_id=[%ld], next_group->ts_seq=[%ld]",
         //         cur_pos.group_id_, cur_group->start_log_id_, cur_group->count_, next_group_id, next_group->start_log_id_, next_group->ts_seq_);
      }
      //printBuf();
      return err;
    }

    bool ObLogGeneratorV2::is_eof(const char* buf, int64_t len)
    {
      return NULL != buf && len >= LOG_FILE_ALIGN_SIZE && 0 == memcmp(buf, eof_flag_buf_, LOG_FILE_ALIGN_SIZE);
    }

    void ObLogGeneratorV2::set_group_as_slave(const int64_t log_id, const int64_t timestamp)
    {
      FLogPos cur_pos = get_next_pos(); 
      LogGroup* log_group = get_log_group(cur_pos.group_id_);
      LogGroup* slave_log_group = get_log_group(0);
      log_group->start_log_id_ = log_id;
      log_group->last_timestamp_ = timestamp;
      log_group->group_id_ = cur_pos.group_id_;
      __sync_synchronize();
      log_group->last_ts_seq_ = cur_pos.group_id_ + log_group->READY;
      log_group->ts_seq_ = cur_pos.group_id_;
      log_group->set_start_cursor(slave_log_group->end_cursor_);
      log_group->set_end_cursor(slave_log_group->end_cursor_);
//      TBSYS_LOG(INFO,"test::zhouhuan slave swtich to master! start_log_id=%ld, group_id=%ld, rel_id=%d ts_seq=%ld, start_cursor=[%s]",
//                log_id, cur_pos.group_id_, cur_pos.rel_id_,log_group->ts_seq_, to_cstring(log_group->start_cursor_));
    }

    void ObLogGeneratorV2::reset_next_pos()
    {
      //get_next_pos().reset();
      next_pos_.reset();
    }

    FLogPos ObLogGeneratorV2::get_next_pos()
    {
      FLogPos ret;
      LOAD128(ret, &next_pos_);
      return ret;
    }
  }// end namespace common
}// end namespace oceanbase
