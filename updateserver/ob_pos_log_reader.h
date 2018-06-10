/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_pos_log_reader.h
 * @brief ObPosLogReader
 *     modify by liubozhong: support multiple clusters for HA by
 *     adding or modifying some functions, member variables
 *
 * @version CEDAR 0.2 
 * @author liubozhong <51141500077@ecnu.cn>
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

#ifndef OCEANBASE_UPDATESERVER_OB_POS_LOG_READER_H_
#define OCEANBASE_UPDATESERVER_OB_POS_LOG_READER_H_
#include "ob_log_locator.h"
#include "ob_on_disk_log_locator.h"
#include "ob_located_log_reader.h"

namespace oceanbase
{
  namespace updateserver
  {
    class ObPosLogReader
    {
      public:
        ObPosLogReader();
        virtual ~ObPosLogReader();
        int init(const char* log_dir, const bool dio = true);
        // start_location的log_id_必须是有效的, file_id_和offset_如果无效，会被填充为正确值。
        virtual int get_log(const int64_t start_id, ObLogLocation& start_location, ObLogLocation& end_location,
                            char* buf, const int64_t len, int64_t& read_count);
        //add lbzhong [Commit Point] 20150820:b
        /**
         * @brief get commit log from log file
         * @param[in] start_id  the start log id of this read
         * @param[in] start_location  the start location of this read
         * @param[out] end_location  the end location of this read
         * @param[in] buf  the log buffer
         * @param[in] len  the limit of the log buffer
         * @param[out] read_count  the number of bytes of this read
         * @return OB_SUCCESS if success
         */
        virtual int get_log(const int64_t start_id, ObLogLocation& start_location, ObLogLocation& end_location,
                            char* buf, const int64_t len, int64_t& read_count, bool& has_committed_end, const int64_t commit_seq);
        //add:e
      protected:
        bool is_inited() const;
      private:
        char log_dir_[OB_MAX_FILE_NAME_LENGTH];
        ObOnDiskLogLocator on_disk_log_locator_;
        ObLocatedLogReader located_log_reader_;
    };
  }; // end namespace updateserver
}; // end namespace oceanbase
#endif // OCEANBASE_UPDATESERVER_OB_POS_LOG_READER_H_
