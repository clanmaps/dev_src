/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_repeated_log_reader.h
 * @brief support multiple clusters for HA by adding or modifying
 *        some functions, member variables
 *        modify the class ObRepeatedLogReader, output the timestamp info of each logEntry
 *
 * @version CEDAR 0.2 
 * @author liubozhong <51141500077@ecnu.cn>
 *         zhangcd<zhangcd_ecnu@ecnu.cn>
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

#ifndef OCEANBASE_COMMON_OB_REPEATED_LOG_READER_H_
#define OCEANBASE_COMMON_OB_REPEATED_LOG_READER_H_

#include "ob_single_log_reader.h"

namespace oceanbase
{
  namespace common
  {
    class ObRepeatedLogReader : public ObSingleLogReader
    {
    public:
      ObRepeatedLogReader();
      virtual ~ObRepeatedLogReader();

      /**
       * @brief 从操作日志中读取一个更新操作
       * @param [out] cmd 日志类型
       * @param [out] log_seq 日志序号
       * @param [out] log_data 日志内容
       * @param [out] data_len 缓冲区长度
       * @return OB_SUCCESS: 如果成功;
       *         OB_READ_NOTHING: 从文件中没有读到数据
       *         others: 发生了错误.
       */
      int read_log(LogCommand &cmd, uint64_t &log_seq, char *&log_data, int64_t &data_len);

      //add lbzhong [Max Log Timestamp] 20150824:b
      //add lbzhong [Max Log Timestamp] 20150824:b
      /**
       * @brief [overwrite] read log timestamp from commit log
       * @param[out] cmd  log command
       * @param[out] log_seq  LSN
       * @param[out] timestamp  log timestamp
       * @return OB_SUCCESS if success
       */
      int read_log(LogCommand &cmd, uint64_t &log_seq, int64_t& timestamp);
      //add:e
      //add chujiajia [log synchronization][multi_cluster] 20160419:b
      /**
       * @brief [overwrite] read max commited log id from commit log
       * @param[out] cmd  log command
       * @param[out] log_seq  LSN
       * @param[out] cmt_id  max commited log id
       * @return OB_SUCCESS if success
       */
      int read_log_for_cmt_id(LogCommand &cmd, uint64_t &log_seq, int64_t& cmt_id);
      /**
       * @brief [overwrite] read data checksum from commit log
       * @param[out] cmd  log command
       * @param[out] log_seq  LSN
       * @param[out] data_checksum  data checksum
       * @return OB_SUCCESS if success
       */
      int read_log_for_data_checksum(LogCommand &cmd, uint64_t &log_seq, int64_t& data_checksum);
      // add:e
      // add by zhangcd [log_reader] 20151215:b
      int64_t timestamp_;
      // add:e
      //add chujiajia [log synchronization][multi_cluster] 20160326:b
      int64_t max_cmt_id_;  ///< max commited log id
      //add:e
    };
  } // end namespace common
} // end namespace oceanbase

#endif // OCEANBASE_COMMON_OB_REPEATED_LOG_READER_H_
