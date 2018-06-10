/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_post.h
 * @brief slave response message
 * This file is designed for slave to response commit log from master.
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */

#ifndef __OCEANBASE_UPS_OB_LOG_POST_H__
#define __OCEANBASE_UPS_OB_LOG_POST_H__
#include "ob_define.h"
#include "serialization.h"
#include "tbsys.h"

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief ObLogPostResponse
     * This structure is designed for slave to response commit log from master.
     */
    struct ObLogPostResponse
    {
        int64_t next_flush_log_id_;              ///< next flush LSN in slave
        int64_t message_residence_time_us_;      ///< processing time in slave
        enum SlaveSyncStatus
        {
          SYNC = 0,
          NOTSYNC = 1,
          OFFLINE = 2
        } slave_status_;                         ///< status of slave

        /**
         * @brief constructor
         */
        ObLogPostResponse();

        /**
         * @brief destructor
         */
        ~ObLogPostResponse();

        /**
         * @brief whether the response is valid
         * @return true if the response is valid
         */
        bool is_valid() const;

        /**
         * @brief reset members of the response
         */
        void reset();

        /**
         * @brief serialize the instance of response
         * @return OB_SUCCESS if success, OB_ERROR if not success
         */
        int serialize(char* buf, int64_t len, int64_t& pos) const;

        /**
         * @brief deserialize the instance of response
         * @return OB_SUCCESS if success, OB_ERROR if not success
         */
        int deserialize(const char* buf, int64_t len, int64_t& pos) const;

        /**
         * @brief get string of the response
         * @return the pointer of the string
         */
        char* to_str() const;

        /**
         * @brief get string of the response
         * @param[out] buf  the buffer of the string
         * @param[in] len  the limit of the buffer
         * @return the length of the string
         */
        int64_t to_string(char* buf, const int64_t len) const;
    };
  }  // end namespace common
}  // end namespace oceanbase
#endif // __OCEANBASE_UPS_OB_LOG_POST_H__
