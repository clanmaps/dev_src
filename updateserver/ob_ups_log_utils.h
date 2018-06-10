/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_log_utils.h
 * @brief utilities for commit log of ups
 *     modify by liubozhong, zhangcd: support multiple clusters
 *     for HA by adding or modifying some functions, member variables
 *
 * @version CEDAR 0.2 
 * @author liubozhong <51141500077@ecnu.cn>
 *         zhangcd <zhangcd_ecnu@ecnu.cn>
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
 *   yuanqi <yuanqi.xhf@taobao.com>
 *     - some work details if you want
 */

#ifndef __OB_UPDATESERVER_OB_UPS_LOG_UTILS_H__
#define __OB_UPDATESERVER_OB_UPS_LOG_UTILS_H__

#include "common/ob_define.h"
#include "common/ob_log_entry.h"
#include "common/ob_log_cursor.h"
//add lbzhong [Commit Point] 20150820:b
#include "common/file_utils.h"
#include "common/file_directory_utils.h"

using namespace oceanbase::common;
//add:e

namespace oceanbase
{
  namespace updateserver
  {
    //add lbzhong [Commit Point] 20150820:b
    //
    // @brief write commitpoint file
    //
    // class ObFileForLog is modify by zhangcd 20151215, change the write function
    // from repeatly openning and closing file to just writting content into file.
    // The file is just openned at the initialization ObFileForlog object while
    // new write function is just writting content to the file, and closing the
    // file in the object destory function.
    //
    /**
     * @brief ObFileForLog
     * This class is designed for special file read/write, e.g. was_master, commit_point.
     * The initialization opens this special file, while write/read function writes/reads
     * content to the file, and destuctor function closes this file.
     */
    template<typename T>
    class ObFileForLog
    {
    public:
      /**
       * @brief constructor
       */
      ObFileForLog(): file_dir_(NULL), file_name_(NULL){}

      /**
       * @brief destructor
       */
      ~ObFileForLog();

      /**
       * @brief open the file
       * @param[in] file_dir  the directory containing the file
       * @param[in] file_name  the file name
       * @return OB_SUCCESS if success
       */
      int init(const char* file_dir, const char* file_name);

      /**
       * @brief write to the file
       * write the data from the first line of the file,
       * overwrite the previous content
       * @param[in] data  the data to the file
       * @return OB_SUCCESS if success
       */
      int write(const T data);

      /**
       * @brief read from the file
       * @param[out] data  the data from the file
       * @return OB_SUCCESS if success
       */
      int get(T& data);
    private:
      const char* file_dir_;     ///< the directory
      const char* file_name_;    ///< the file name
      FileUtils file_file;       ///< the file utils
    };

    template<typename T>
    int ObFileForLog<T>::init(const char* file_dir, const char* file_name)
    {
      int err = OB_SUCCESS;
      file_dir_ = file_dir;
      file_name_ = file_name;
      int len = 0;
      char file_fn[OB_MAX_FILE_NAME_LENGTH];
      int32_t open_ret = OB_SUCCESS;
      if (NULL == file_dir)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "Arguments are invalid[file_dir=%p]", file_dir);
      }
      else if ((len = snprintf(file_fn, sizeof(file_fn), "%s/%s", file_dir, file_name) < 0)
             && len >= (int64_t)sizeof(file_fn))
      {
        err = OB_BUF_NOT_ENOUGH;
        TBSYS_LOG(ERROR, "generate_data_fn()=>%d", err);
      }
      // modify by zhangcd [majority_count_init] 20160108:b
      else if (0 > (open_ret = file_file.open(file_fn, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)))
      //else if (0 > (open_ret = file_file.open(file_fn, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)))
      // modify:e
      {
        err = OB_FILE_NOT_EXIST;
        TBSYS_LOG(ERROR, "open file[\"%s\"] error[%s]", file_fn, strerror(errno));
      }
      return err;
    }

    template<typename T>
    int ObFileForLog<T>::write(const T data)
    {
      return write_file_for_log_func(data, &file_file);
    }

    template<typename T>
    ObFileForLog<T>::~ObFileForLog()
    {
      file_file.close();
    }

    template<typename T>
    int ObFileForLog<T>::get(T& data)
    {
      return load_file_for_log_func(file_dir_, file_name_, data);
    }

    template<typename T>
    int load_file_for_log_func(const char* file_dir, const char* file_name, T& data)
    {
      int err = 0;
      int64_t len = 0;
      int open_ret = 0;
      const int UINT64_MAX_LEN = 20;
      char file_fn[OB_MAX_FILE_NAME_LENGTH];
      char file_str[UINT64_MAX_LEN];
      int file_str_len = 0;
      FileUtils file_file;

      if (NULL == file_dir)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "Arguments are invalid[file_dir=%p]", file_dir);
      }
      else if ((len = snprintf(file_fn, sizeof(file_fn), "%s/%s", file_dir, file_name) < 0)
             && len >= (int64_t)sizeof(file_fn))
      {
        err = OB_ERROR;
        TBSYS_LOG(ERROR, "generate_data_fn()=>%d", err);
      }
      else if (!FileDirectoryUtils::exists(file_fn))
      {
        err = OB_SUCCESS;
        data = 0;
        TBSYS_LOG(INFO, "file_for_log file[\"%s\"] does not exist, skip it", file_fn);
      }
      else if (0 > (open_ret = file_file.open(file_fn, O_RDONLY)))
      {
        err = OB_IO_ERROR;
        TBSYS_LOG(WARN, "open file[\"%s\"] error[%s]", file_fn, strerror(errno));
      }
      else if (0 > (file_str_len = static_cast<int32_t>(file_file.read(file_str, sizeof(file_str))))
             || file_str_len >= (int)sizeof(file_str))
      {
        err = file_str_len < 0 ? OB_IO_ERROR: OB_BUF_NOT_ENOUGH;
        TBSYS_LOG(ERROR, "read error[%s] or file contain invalid data[len=%d]", strerror(errno), file_str_len);
      }
      else
      {
        file_str[file_str_len] = '\0';
        const int STRTOUL_BASE = 10;
        char* endptr;
        data = static_cast<T>(strtoul(file_str, &endptr, STRTOUL_BASE));
        if ('\0' != *endptr)
        {
          err = OB_INVALID_DATA;
          TBSYS_LOG(ERROR, "non-digit exist in file_for_log file[file_str=%.*s]", file_str_len, file_str);
        }
        else if (ERANGE == errno)
        {
          err = OB_INVALID_DATA;
          TBSYS_LOG(ERROR, "data contained in file_for_log file is out of range");
        }
      }
      if (0 > open_ret)
      {
        file_file.close();
      }
      return err;
    }

    template<typename T>
    int write_file_for_log_func(const T data, FileUtils *file_file)
    {
      int err = OB_SUCCESS;
      const int UINT64_MAX_LEN = 20;
      char file_str[UINT64_MAX_LEN];
      int64_t file_str_len = 0;

      if ((file_str_len = snprintf(file_str, sizeof(file_str), "%ld", static_cast<int64_t>(data))) < 0
             || file_str_len >= (int64_t)sizeof(file_str))
      {
        err = OB_BUF_NOT_ENOUGH;
        TBSYS_LOG(ERROR, "snprintf file_str error[%s][data=%ld]", strerror(errno), static_cast<int64_t>(data));
      }
      // modify by zhangcd [majority_count_init] 20160120:b
      //else if (0 > (file_str_len = file_file->pwrite(file_str, file_str_len, 0, false)))
      else if (0 > (file_str_len = file_file->pwrite(file_str, file_str_len, 0, true)))
      // modify:e
      {
        err = OB_ERR_SYS;
        TBSYS_LOG(ERROR, "write error[%s][file_str=%p file_str_len=%ld]", strerror(errno), file_str, file_str_len);
      }
      return err;
    }
    //add:e
    enum ReplayType
    {
      RT_LOCAL = 0,
      RT_APPLY = 1,
    };

    class IObLogApplier
    {
      public:
        IObLogApplier(){}      
        virtual ~IObLogApplier(){}
        virtual int apply_log(const common::LogCommand cmd, const uint64_t seq,
                              const char* log_data, const int64_t data_len, const ReplayType replay_type) = 0;
    };

    class ObUpsTableMgr;
    class ObUpsMutator;
    class CommonSchemaManagerWrapper;
    class ObLogReplayWorker;

    class ObLogReplayPoint
    {
      public:
        static const char* REPLAY_POINT_FILE;
      public:
        ObLogReplayPoint(): log_dir_(NULL) {}
        ~ObLogReplayPoint(){}
        int init(const char* log_dir);
        int write(const int64_t replay_point);
        int get(int64_t& replay_point);
      private:
        const char* log_dir_;
    };

    int64_t set_counter(tbsys::CThreadCond& cond, volatile int64_t& counter, const int64_t new_counter);
    int64_t wait_counter(tbsys::CThreadCond& cond, volatile int64_t& counter, const int64_t limit, const int64_t timeout_us);

    int get_local_max_log_cursor_func(const char* log_dir, const common::ObLogCursor& start_cursor,
                                      common::ObLogCursor& end_cursor);
    int get_local_max_log_cursor_func(const char* log_dir, const uint64_t log_file_id_by_sst, common::ObLogCursor& log_cursor);

    int replay_single_log_func(ObUpsMutator& mutator, CommonSchemaManagerWrapper& schema, ObUpsTableMgr* table_mgr, common::LogCommand cmd, const char* log_data, int64_t data_len, const int64_t commit_id, const ReplayType replay_type);
    int replay_local_log_func(const volatile bool& stop, const char* log_dir,
                              const common::ObLogCursor& start_cursor, common::ObLogCursor& end_cursor,
                              ObLogReplayWorker& replay_worker
                              //add lbzhong [Commit Point] 20150930:b
                              ,const int64_t commit_seq
                              //add:e
                              );
    int replay_local_log_func(const volatile bool& stop, const char* log_dir, const common::ObLogCursor& start_cursor,
                              common::ObLogCursor& end_cursor, IObLogApplier* log_applier);
    int replay_log_in_buf_func(const char* log_data, int64_t data_len, IObLogApplier* log_applier);

    //add chujiajia [log synchronization][multi_cluster] 20160524:b
    /**
     * @brief get tmp log data checksum
     * @param[in] log_dir  log directory
     * @param[out] seq  log id
     * @param[out] tmp_data_checksum  uncertain log data checksum
     * @param[in] start_cursor  start cursor
     * @param[out] end_cursor  end cursor
     * @return OB_SUCCESS if success
     */
    int get_tmp_log_data_checksum(const char* log_dir,
                                 uint64_t &seq,
                                 int64_t &tmp_data_checksum,
                                 const ObLogCursor& start_cursor,
                                 ObLogCursor& end_cursor);
    //add:e

    int serialize_log_entry(char* buf, const int64_t len, int64_t& pos, common::ObLogEntry& entry,
                            const char* log_data, const int64_t data_len);
    int generate_log(char* buf, const int64_t len, int64_t& pos, common::ObLogCursor& cursor, const common::LogCommand cmd,
                     const char* log_data, const int64_t data_len);
    int set_entry(common::ObLogEntry& entry, const int64_t seq, const common::LogCommand cmd,
                  const char* log_data, const int64_t data_len);
    int serialize_log_entry(char* buf, const int64_t len, int64_t& pos, const common::LogCommand cmd, const int64_t seq,
                            const char* log_data, const int64_t data_len);

    int parse_log_buffer(const char* log_data, const int64_t len,
                         int64_t& start_id, int64_t& end_id
                         //add chujiajia [log synchronization][multi_cluster] 20160530:b
                         , int64_t& master_cmt_log_id
                         //add:e
                         );
    int trim_log_buffer(const char* log_data, const int64_t len, int64_t& end_pos,
                        int64_t& start_id, int64_t& end_id
                        //add chujiajia [log synchronization][multi_cluster] 20160530:b
                        , int64_t& master_cmt_log_id
                        //add:e
                        );
    int trim_log_buffer(const char* log_data, const int64_t len, int64_t& end_pos,
                        int64_t& start_id, int64_t& end_id,
                        //add chujiajia [log synchronization][multi_cluster] 20160530:b
                        int64_t& master_cmt_log_id,
                        //add:e
                        bool& is_file_end);
    int trim_log_buffer(const int64_t offset, const int64_t align_bits,
                        const char* log_data, const int64_t len, int64_t& end_pos,
                        int64_t& start_id, int64_t& end_id, bool& is_file_end);
    //add lbzhong [Commit Point] 20150522:b
    /**
     * @brief trim log buffer
     * @param[in] offset  the offset of log buffer
     * @param[in] align_bits  the number of bits of aligment
     * @param[in] log_data  the log buffer
     * @param[in] len  the limit of log buffer
     * @param[out] end_pos  the end position of this trim
     * @param[out] start_id  the start log id of this trim
     * @param[out] end_id  the end log id of this trim
     * @param[out] is_file_end  whether to the end of log file
     * @param[out] has_committed_end  whether to the commit point
     * @param[in] commit_seq  the commit point
     * @return OB_SUCCESS if success
     */
    int trim_log_buffer(const int64_t offset, const int64_t align_bits,
                        const char* log_data, const int64_t len, int64_t& end_pos,
                        int64_t& start_id, int64_t& end_id, bool& is_file_end, bool& has_committed_end, const int64_t commit_seq);

    int get_log_timestamp(common::LogCommand cmd, const char* buf, int64_t len, int64_t& timestamp);

    /**
     * @brief get max log timestamp from local log file
     * @param[in] log_dir  the directory containing log files
     * @param[in] start_cursor  the start cursor of this get
     * @param[out] end_cursor  the end cursor of this get
     * @param[out] timestamp  the max log timestamp of this get
     * @return OB_SUCCESS if success
     */
    int get_local_max_log_timestamp_func(const char* log_dir, const common::ObLogCursor& start_cursor, common::ObLogCursor& end_cursor, int64_t& timestamp);

    /**
     * @brief get max log timestamp from local log file
     * @param[in] log_dir  the directory containing log files
     * @param[in] log_file_id_by_sst  the log file id from sstable
     * @param[out] timestamp  the max log timestamp of this get
     * @return OB_SUCCESS if success
     */
    int get_local_max_log_timestamp_func(const char* log_dir, const uint64_t log_file_id_by_sst, int64_t& max_timestamp);

    /**
     * @brief get max log timestamp from local log file
     * @param[in] log_data  the log buffer
     * @param[in] data_len  the limit of the log buffer
     * @param[in] start_cursor  the start cursor of this get
     * @param[out] timestamp  the max log timestamp of this get
     * @return OB_SUCCESS if success
     */
    int get_max_timestamp_from_log_buffer(const char* log_data, const int64_t data_len, const common::ObLogCursor& start_cursor, int64_t& max_timestamp);
    //add:e
    //add chujiajia [log synchronization][multi_cluster] 20150419:b
    int get_local_max_cmt_id_func(const char* log_dir, const uint64_t log_file_id_by_sst, int64_t& cmt_id, ObLogCursor &tmp_end_cursor);
    int get_local_max_cmt_id_func(const char* log_dir, const common::ObLogCursor& start_cursor, common::ObLogCursor& end_cursor, int64_t& cmt_id);
    int get_cursor_by_log_id(const char* log_dir, const int64_t log_id, const common::ObLogCursor& start_cursor, common::ObLogCursor& end_cursor);
    int get_checksum_by_log_id(const char* log_dir, const int64_t log_id, const common::ObLogCursor& start_cursor, common::ObLogCursor& end_cursor, int64_t &checksum);
    //add:e
 } // end namespace updateserver
} // end namespace oceanbase
#endif /* __OB_UPDATESERVER_OB_UPS_LOG_UTILS_H__ */
