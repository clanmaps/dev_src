/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_log_post.cpp
 * @brief slave response message
 * This file is designed for slave to response commit log from master.
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */

#include "ob_define.h"
#include "serialization.h"
#include "tbsys.h"
#include "ob_log_post.h"

namespace oceanbase
{
  namespace common
  {
    ObLogPostResponse::ObLogPostResponse(): next_flush_log_id_(-1),
                                            message_residence_time_us_(-1),
                                            slave_status_(OFFLINE)
    {}

    ObLogPostResponse::~ObLogPostResponse()
    {}

    bool ObLogPostResponse::is_valid() const
    {
      return next_flush_log_id_ >= 0 && slave_status_ >= 0 && slave_status_ <= 2;
    }

    void ObLogPostResponse::reset()
    {
      next_flush_log_id_ = 0;
      slave_status_ = OFFLINE;
    }

    int ObLogPostResponse::serialize(char* buf, int64_t len, int64_t& pos) const
    {
      int err = OB_SUCCESS;
      int64_t tmp_pos = pos;
      if (NULL == buf || 0 > len || 0 > pos || pos > len)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if(OB_SUCCESS != (err = serialization::encode_i64(buf, len, tmp_pos, next_flush_log_id_)))
      {
        TBSYS_LOG(ERROR, "encode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, len, tmp_pos, next_flush_log_id_, err);
      }
      else if(OB_SUCCESS != (err = serialization::encode_i64(buf, len, tmp_pos, message_residence_time_us_)))
      {
        TBSYS_LOG(ERROR, "encode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, len, tmp_pos, message_residence_time_us_, err);
      }
      else if(OB_SUCCESS != (err = serialization::encode_i32(buf, len, tmp_pos, slave_status_)))
      {
    	TBSYS_LOG(ERROR, "encode_i32(buf=%p, len=%ld, pos=%ld, i=%d)=>%d", buf, len, tmp_pos, slave_status_, err);
      }
      else
      {
        pos = tmp_pos;
      }
      return err;
    }

    int ObLogPostResponse::deserialize(const char* buf, int64_t len, int64_t& pos) const
    {
      int err = OB_SUCCESS;
      int64_t tmp_pos = pos;
      if (NULL == buf || 0 > len || 0 > pos || pos > len)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if(OB_SUCCESS != (err = serialization::decode_i64(buf, len, tmp_pos, (int64_t*)&next_flush_log_id_)))
      {
        TBSYS_LOG(ERROR, "decode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, len, tmp_pos, next_flush_log_id_, err);
      }
      else if(OB_SUCCESS != (err = serialization::decode_i64(buf, len, tmp_pos, (int64_t*)&message_residence_time_us_)))
      {
        TBSYS_LOG(ERROR, "decode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, len, tmp_pos, message_residence_time_us_, err);
      }
      else if(OB_SUCCESS != (err = serialization::decode_i32(buf, len, tmp_pos, (int32_t*)&slave_status_)))
      {
        TBSYS_LOG(ERROR, "decode_i32(buf=%p, len=%ld, pos=%ld, i=%d)=>%d", buf, len, tmp_pos, slave_status_, err);
      }
      else
      {
        pos = tmp_pos;
      }
      return err;
    }

    char* ObLogPostResponse::to_str() const
    {
      static char buf[512];
      snprintf(buf, sizeof(buf), "ObLogPostResponse{next_flush_log_id_=%ld, "
                                 "message_residence_time_us_=%ld, "
                                 "slave_status_=%d}",
                                 next_flush_log_id_,
                                 message_residence_time_us_,
                                 slave_status_);
      buf[sizeof(buf)-1] = 0;
      return buf;
    }

    int64_t ObLogPostResponse::to_string(char* buf, const int64_t limit) const
    {
      int64_t len = -1;
      if (NULL == buf || 0 >= limit)
      {
        TBSYS_LOG(ERROR, "Null buf");
      }
      else if (0 >= (len = snprintf(buf, limit, "ObLogCursor{next_flush_log_id_=%ld, "
                                                "message_residence_time_us_ = %ld, "
                                                "slave_status_=%d}",
                                                next_flush_log_id_,
                                                message_residence_time_us_,
                                                slave_status_)) || len >= limit)
      {
        TBSYS_LOG(ERROR, "Buf not enough, buf=%p[%ld]", buf, limit);
      }
      return len;
    }

  } // end namespace common
} // end namespace oceanbase
