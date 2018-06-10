/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_group.h
 * @brief
 * ObLogGroup is designed for storing log in memory transitorily.
 * Operation should set a position in the ObLogGroup before filling log.
 *
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_03_14
 */
#ifndef __OB_COMMON_OB_LOG_GENERATOR2_H__
#define __OB_COMMON_OB_LOG_GENERATOR2_H__

#include "ob_log_cursor.h"
#include "ob_log_generator.h"
//add hushuang [scalable commit]20160507
#include "ob_commit_queue.h"
#include "ob_bit_set.h"
#include "ob_common_stat.h"
//add e
using namespace oceanbase::common;
namespace oceanbase
{
  namespace common
  {
    namespace types
    {
      struct uint128_t
      {
        uint64_t lo;
        uint64_t hi;
      }
        __attribute__ (( __aligned__( 16 ) ));
    }

    inline bool cas128( volatile types::uint128_t * src, types::uint128_t cmp, types::uint128_t with )
    {
      bool result;
      __asm__ __volatile__
      (
        "\n\tlock cmpxchg16b %1"
        "\n\tsetz %0\n"
        : "=q" ( result ), "+m" ( *src ), "+d" ( cmp.hi ), "+a" ( cmp.lo )
        : "c" ( with.hi ), "b" ( with.lo )
        : "cc"
        );
      return result;
    }

    inline void load128 (__uint128_t& dest, types::uint128_t *src)
    {
      __asm__ __volatile__ ("\n\txor %%rax, %%rax;"
                          "\n\txor %%rbx, %%rbx;"
                          "\n\txor %%rcx, %%rcx;"
                          "\n\txor %%rdx, %%rdx;"
                          "\n\tlock cmpxchg16b %1;\n"
                          : "=&A"(dest)
                          : "m"(*src)
                          : "%rbx", "%rcx", "cc");
    }

    #define CAS128(src, cmp, with) cas128((types::uint128_t*)(src), *((types::uint128_t*)&(cmp)), *((types::uint128_t*)&(with)))
    #define LOAD128(dest, src) load128((__uint128_t&)(dest), (types::uint128_t*)(src))
    /**
    * @brief The FLogPos struct
    * FLogPos is designed for
    * occuping buffer
    */
    struct FLogPos
    {
      int64_t group_id_;
      int32_t rel_id_;
      int32_t rel_offset_;
      FLogPos();
      ~FLogPos();
      int init();
      void reset();
      bool is_valid() const;
      char* to_str() const;
      int64_t to_string(char* buf, const int64_t len) const;
      /**
      * @brief computing buffer size that task needs
      * @param[in] pos current pos
      * @param[in] len task len
      * @param[in] limit limt
      * @param[in] switch_flag switch flag
      * @return success if it succeeds.
      */
      int append(FLogPos& pos, int64_t len, int64_t limit, int64_t& switch_flag);
      bool newer_than(const FLogPos& that) const;
      bool equal(const FLogPos& that) const;
    }__attribute__ (( __aligned__( 16 ) ));
    /**
    * @brief The LogGroup struct
    */
    struct LogGroup
    {
      static const int64_t LOG_FILE_ALIGN_SIZE = 1<<OB_DIRECT_IO_ALIGN_BITS;
      static const int64_t LOG_FILE_ALIGN_MASK = LOG_FILE_ALIGN_SIZE - 1;
      static const int64_t LOG_BUF_RESERVED_SIZE = 2 * LOG_FILE_ALIGN_SIZE; // nop or switch_log + eof
      enum { READY = 1};
      int64_t last_proc_time_;
      int64_t first_set_time_;
      int64_t first_fill_time_;
      int64_t last_fill_time_;
      volatile bool sync_to_slave_;
      volatile bool need_ack_;///<need response to client flag
      volatile int64_t ref_cnt_; ///<the number of filled log
      volatile int64_t len_; ///<the length of log
      volatile int64_t count_; ///<the number of position-set log
      volatile int64_t ts_seq_ CACHE_ALIGNED; ///<if ts_seq_=group_id+READY,then the group can be filled,
                                   ///<if ts_seq_=group_id,then the group can be set position
      volatile int64_t group_id_;
      volatile int64_t start_timestamp_;
      volatile int64_t start_log_id_;
      volatile int64_t last_ts_seq_ CACHE_ALIGNED;
      volatile int64_t last_timestamp_;
      volatile int64_t log_cursor_seq_ CACHE_ALIGNED;///<if log_cursor_seq_=group_id+READY,then the group can be committed
      ObLogCursor start_cursor_;
      ObLogCursor end_cursor_;
      char empty_log_[LOG_FILE_ALIGN_SIZE * 2];///<buf stored log_switch
      char nop_log_[LOG_FILE_ALIGN_SIZE * 2];
      char* buf_;
      //add hushuang [scalable commit]20160507
      CallbackList cb_list;
      //add e
      LogGroup();
      ~LogGroup();
      bool is_clear() const;
      void destroy();
      void clear(int64_t group_size);
      int reset(int64_t group_size);
      int check_state() const;
      bool is_inited() const;
      void set_start_cursor(ObLogCursor& log_cursor);
      void set_end_cursor(ObLogCursor& log_cursor);
      void set_last_proc_time(const int64_t cur_time);
      int64_t get_last_proc_time() const;
      void set_first_set_time(const int64_t cur_time);
      int64_t get_first_set_time() const;
      void set_first_fill_time(const int64_t cur_time);
      int64_t get_first_fill_time() const;
      void set_last_fill_time(const int64_t cur_time);
      int64_t get_last_fill_time() const;

    };

    class ObLogGeneratorV2
    {
      public:
        //static const int64_t GROUP_ARRAY_SIZE = 5;
        static const int64_t LOG_FILE_ALIGN_SIZE = 1<<OB_DIRECT_IO_ALIGN_BITS;
        static const int64_t LOG_FILE_ALIGN_MASK = LOG_FILE_ALIGN_SIZE - 1;
        static const int64_t LOG_BUF_RESERVED_SIZE = 2 * LOG_FILE_ALIGN_SIZE; // nop or switch_log + eof
      public:
        /**
         * @brief ObLogGeneratorV2 constructor
         */
        ObLogGeneratorV2();
        /**
         * @brief ObLogGeneratorV2 destructor
         */
        ~ObLogGeneratorV2();
        int group_alloc(int64_t log_size);
        int init(int64_t log_buf_size, int64_t log_file_max_size, const ObServer* id=NULL, int64_t group_size = GROUP_ARRAY_SIZE);
        int reset();
        bool is_log_start(int64_t group_id);
        bool is_clear() const;
        int64_t to_string(char* buf, const int64_t len) const;
        int start_log(const ObLogCursor& start_cursor);
        int init_group(int64_t size);//add hushuang [scalable commit]20160630
        /**
         * @brief ObLogGroup check the log size
         * @return true = log size is suitable
         */
        bool check_log_size(const int64_t size) const;
        LogGroup* get_log_group(int64_t group_id);
        /**
         * @brief ObLogGroup set position by CAS operator
         * @return OB_SUCCESS
         */
        int set_log_position(FLogPos& cur_pos, int64_t len, int64_t& switch_flag, FLogPos& next_pos);
        bool switch_group(FLogPos& cur_pos);
        int set_group_start_timestamp(FLogPos& cur_pos);
        int64_t get_trans_id(FLogPos& cur_pos);
        int fill_batch(const char* buf, int64_t len);
        int write_log(const LogCommand cmd, const char* log_data, const int64_t data_len, const FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
        int write_end_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
        template<typename T>
        int write_log(const LogCommand cmd, const T& data, const FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
        int get_log(ObLogCursor& start_cursor, ObLogCursor& end_cursor, char*& buf, int64_t& len, LogGroup* cur_group);
        int commit(LogGroup* cur_group, const ObLogCursor& end_cursor, bool is_master);
        int switch_log_file(int64_t& new_file_id, FLogPos& cur_pos, const int64_t max_cmt_id);
        int gen_keep_alive(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
        int get_start_log_cursor(ObLogCursor& log_cursor, int64_t group_id);
        static bool is_eof(const char* buf, int64_t len);
        void set_group_as_slave(const int64_t log_id, const int64_t timestamp);
        void reset_next_pos();
        FLogPos get_next_pos();
      protected:
        bool is_inited() const;
        int check_state() const;
        //bool has_log() const;
        int do_write_log(const LogCommand cmd, const char* log_data, const int64_t data_len,
                         const FLogPos& cur_pos, const int64_t reserved_len, int64_t& ref_cnt, const int64_t max_cmt_id);
        //int check_log_file_size();
        int write_nop(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id, bool force_write=false);
        int switch_log(FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id);
        int append_eof(const FLogPos& cur_pos, bool switch_flag, int64_t& ref_cnt);

      public:
        static char eof_flag_buf_[LOG_FILE_ALIGN_SIZE] __attribute__ ((aligned(DIO_ALIGN_SIZE)));
      private:
        DebugLog debug_log_;
        int64_t log_file_max_size_;
        int64_t log_buf_len_;
        //modify hushuang [scalablecommit]20160630
        //LogGroup log_buf_[GROUP_ARRAY_SIZE];
        LogGroup *log_buf_;
        int64_t group_size_;
        //modify e
        FLogPos next_pos_;
    };

    template<typename T>
    int generate_log2(char* buf, const LogCommand cmd, const T& data, const FLogPos& cur_pos,
                      const int64_t log_id, int64_t len, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      ObLogEntry entry;
      int64_t new_pos, data_pos, end_pos;
      if(0 == cur_pos.rel_offset_)
      {
        new_pos = cur_pos.rel_offset_;
      }
      else
      {
        new_pos = cur_pos.rel_offset_ - entry.get_serialize_size() - data.get_serialize_size();
      }
      data_pos = new_pos + entry.get_serialize_size();
      end_pos = data_pos;
      if (NULL == buf || 0 >= len || new_pos > len)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "generate_log(buf=%p, len=%ld, pos=%ld)=>%d",buf, len, new_pos, err);
      }
      else if (OB_SUCCESS != (err = data.serialize(buf, len, end_pos)))
      {
        TBSYS_LOG(ERROR, "data.serialize()=>%d", err);
      }
      else if (OB_SUCCESS != (err = entry.set_entry(buf + data_pos, end_pos - data_pos, cmd, log_id + (int64_t)cur_pos.rel_id_, max_cmt_id)))
      {
        TBSYS_LOG(ERROR, "cur_pos[%s].set_entry()=>%d", to_cstring(cur_pos), err);
      }
      else if (OB_SUCCESS != (err = entry.serialize(buf, new_pos + entry.get_serialize_size(), new_pos)))
      {
        TBSYS_LOG(ERROR, "serialize_log_entry(buf=%p, len=%ld, entry[id=%ld], data_len=%ld)=>%d",
                  buf, len, entry.seq_, end_pos - data_pos, err);
      }
      else if(end_pos != cur_pos.rel_offset_)
      {
        err = OB_ERROR;
        //TBSYS_LOG(ERROR, "test::zhouhuan cal size = %ld,real size = %ld", end_pos - data_pos, data.get_serialize_size());
        //TBSYS_LOG(ERROR, "generate_log pos=[%ld] != cur_pos.rel_offset[%d]", end_pos, cur_pos.rel_offset_);
      }
      //TBSYS_LOG(INFO,"test::zhouhuan log_id = [%ld] rel_id = [%d]", log_id, cur_pos.rel_id_);
      //TBSYS_LOG(INFO,"test::zhouhuan log_id = [%ld] seq = [%ld] len=%ld", (log_id + (int64_t)cur_pos.rel_id_), entry.seq_,new_pos);
      return err;

    }

    template<typename T>
    int ObLogGeneratorV2::write_log(const LogCommand cmd, const T& data, const FLogPos& cur_pos, int64_t& ref_cnt, const int64_t max_cmt_id)
    {
      int err = OB_SUCCESS;
      LogGroup* cur_group = get_log_group(cur_pos.group_id_);
      //TBSYS_LOG(ERROR, "test::zhouhuan: write_log group[%ld].start_log_id=[%ld], ts_seq=[%ld]", cur_pos.group_id_, cur_group->start_log_id_, cur_group->ts_seq_);
      if (OB_SUCCESS != (err = cur_group->check_state()))
      {
        TBSYS_LOG(ERROR, "check_state()=>%d", err);
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
            //TBSYS_LOG(ERROR, "test::zhouhuan: write_log group_id=[%ld], start_log_id=[%ld], ts_seq=[%ld]",
              //        cur_pos.group_id_, cur_group->start_log_id_, cur_group->ts_seq_);
            err = generate_log2(cur_group->buf_, cmd, data, cur_pos, cur_group->start_log_id_,
                                log_buf_len_ - LOG_BUF_RESERVED_SIZE, max_cmt_id);
            break;
          }
        }
      }

      if (OB_SUCCESS != err)
      {
        TBSYS_LOG(WARN, "generate_log2(pos=%d)=>%d", cur_pos.rel_offset_, err);
      }
      else
      {
        ref_cnt = ATOMIC_INC(&(cur_group->ref_cnt_));
        //add by zhouhuan for __all_sys_stat
        if (ref_cnt == 1)
        {
          int64_t cur_time = tbsys::CTimeUtil::getTime();
          cur_group->set_first_fill_time(cur_time);
        }
        //add:e
       //TBSYS_LOG(ERROR, "test::zhouhuan ref_cnt group_id = %ld ref_cnt=%ld rel_id=%d", cur_group->group_id_, ref_cnt, cur_pos.rel_id_);
      }
      return err;
    }


  }//end namespace common
}//end namespace oceanbase


#endif /* __OB_COMMON_OB_LOG_GENERATOR2_H__ */

