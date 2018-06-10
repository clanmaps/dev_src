/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_transaction.h
 * @brief ObTransID
 *     modify by guojinwei, bingo: support REPEATABLE-READ isolation
 *     add isolation and transaction information to ObTransID
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         bingo <bingxiao@stu.ecnu.edu.cn>
 * @date 2016_06_16
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
#ifndef __OB_COMMON_OB_TRANSACTION_H__
#define __OB_COMMON_OB_TRANSACTION_H__
#include "ob_define.h"
#include "ob_server.h"
#include "ob_string.h"
namespace oceanbase
{
  namespace common
  {
    enum IsolationLevel
    {
      NO_LOCK = 1,
      READ_COMMITED = 2,
      REPEATABLE_READ = 3,
      SERIALIZABLE = 4,
    };

    enum TransType
    {
      READ_ONLY_TRANS = 0,
      READ_WRITE_TRANS = 1,
      INTERNAL_WRITE_TRANS = 2, // 不加锁,避免触发trigger
      REPLAY_TRANS = 3, // 不加锁，
      LONG_READ_WRITE_TRANS = 4, // add by qx 20170314 for long transcations
    };

    struct ObTransReq
    {
      ObTransReq(): type_(READ_WRITE_TRANS), isolation_(READ_COMMITED),
                    start_time_(0),
                    timeout_(OB_DEFAULT_SESSION_TIMEOUT),
                    idle_time_(timeout_) {}
      ~ObTransReq() {}
      int32_t type_;
      int32_t isolation_;
      int64_t start_time_; 
      int64_t timeout_;
      int64_t idle_time_;

      int64_t to_string(char* buf, int64_t len) const;
      int set_isolation_by_name(const ObString &isolation);
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    struct ObTransID
    {
      const static uint32_t INVALID_SESSION_ID = 0;
      // modify by guojinwei [repeatable read] 20160310:b
      //ObTransID(): descriptor_(INVALID_SESSION_ID), ups_(), start_time_us_(0) {}
      ObTransID(): descriptor_(INVALID_SESSION_ID), ups_(), start_time_us_(0), trans_start_time_us_(0), isolation_level_(READ_COMMITED) {}
      // modify:e
      ~ObTransID() {}
      void reset();
      bool is_valid()const;
      int64_t to_string(char* buf, int64_t len) const;
      NEED_SERIALIZE_AND_DESERIALIZE;
      uint32_t descriptor_;
      common::ObServer ups_;
      int64_t start_time_us_;
      // add by guojinwei [repeatalbe read] 20160310:b
      int64_t trans_start_time_us_;
      int32_t isolation_level_;
      // add:e
    };

    struct ObEndTransReq
    {
      ObEndTransReq(): trans_id_(), rollback_(false) {}
      ~ObEndTransReq() {}
      int64_t to_string(char* buf, int64_t len) const;
      NEED_SERIALIZE_AND_DESERIALIZE;
      ObTransID trans_id_;
      bool   rollback_;
    };
  }; // end namespace common
}; // end namespace oceanbase

#endif /* __OB_COMMON_OB_TRANSACTION_H__ */
