/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_wait_queue.h
 * @brief support multiple clusters for HA by adding or modifying
 *        some functions, member variables
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
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
#ifndef __OB_COMMON_OB_WAIT_QUEUE_H__
#define __OB_COMMON_OB_WAIT_QUEUE_H__
#include "ob_define.h"
#include "tbsys.h"
#include "ob_malloc.h"
#include "utility.h"
#include "ob_log_post.h"

namespace oceanbase
{
  namespace common
  {
    template<typename DataT>
    class ObFixedArray
    {
      typedef DataT Item;
      enum {MIN_ARRAY_LEN = 32};
      public:
        ObFixedArray(): pos_mask_(0), items_(NULL) {}
        ~ObFixedArray() { destroy(); }
        int init(int64_t len)
        {
          int err = OB_SUCCESS;
          if (len < MIN_ARRAY_LEN || 0 != ((len-1) & len))
          {
            err = OB_INVALID_ARGUMENT;
            TBSYS_LOG(ERROR, "INVALID_ARGUMENT: len=%ld", len);
          }
          else if (NULL != items_)
          {
            err = OB_INIT_TWICE;
          }
          else if (NULL == (items_ = (Item*)ob_malloc(sizeof(Item) * len, ObModIds::OB_SEQ_QUEUE)))
          {
            err = OB_MEM_OVERFLOW;
            TBSYS_LOG(ERROR, "ob_malloc(%ld): failed", sizeof(Item) * len);
          }
          for(int64_t i = 0; OB_SUCCESS == err && i < len; i++)
          {
            new(items_ + i)Item(i);
          }
          if (OB_SUCCESS != err)
          {
            destroy();
          }
          else
          {
            pos_mask_ = len - 1;
          }
          return err;
        }

        Item* get(int64_t seq){ return items_? items_ + (seq & pos_mask_): NULL; }
        int64_t len() const { return pos_mask_ + 1; }
      protected:
        void destroy()
        {
          if (NULL != items_)
          {
            for(int64_t i = 0; i < pos_mask_ + 1; i++)
            {
              items_[i].~Item();
            }
            ob_free(items_);
            items_ = NULL;
          }
        }
      protected:
        int64_t pos_mask_;
        Item* items_;
    };

    template<typename DataT>
    class ObWaitQueue
    {
      public:
        enum
        {
          REGISTERED = 1,
          DONE = 2,
        };
        struct Item
        {
          Item(int64_t seq): lock_(0), wait_seq_(seq), data_() {}
          ~Item(){}
          volatile uint64_t lock_;
          volatile int64_t wait_seq_;
          DataT data_;
        };
      public:
        ObWaitQueue(): lock_(0), push_(0), pop_(0) {}
        ~ObWaitQueue(){}
      public:
        int init(int64_t len)
        {
          return array_.init(len);
        }
        
        int push(int64_t& seq, DataT& data)
        {
          int err = OB_SUCCESS;
          Item* item = NULL;
          SeqLockGuard guard(lock_);
          int64_t push = push_;
          if (NULL == (item = array_.get(push)))
          {
            err = OB_NOT_INIT;
          }
          else
          {
            SeqLockGuard guard(item->lock_);
            if (item->wait_seq_ != push)
            {
              err = OB_EAGAIN;
              TBSYS_LOG(WARN, "wait_queue is full: push=%ld pop=%ld", push_, pop_);
            }
            else
            {
              item->data_ = data;
              seq = push;
              __sync_synchronize();
              item->wait_seq_ = push + REGISTERED;
              push_++;
            }
          }
          return err;
        }
        // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
        //int done(int64_t seq, DataT& data, int handle_err)
        int done(int64_t seq, const ObLogPostResponse& response_data, DataT& data, int handle_err)
        // modify:e
        {
          int err = OB_SUCCESS;
          Item* item = NULL;
          if (NULL == (item = array_.get(seq)))
          {
            err = OB_NOT_INIT;
          }
          else
          {
            SeqLockGuard guard(item->lock_);
            if (item->wait_seq_ > seq + REGISTERED)
            {
              err = OB_ALREADY_DONE;
              TBSYS_LOG(WARN, "someone already mark done seq=%ld real_seq=%ld", seq, item->wait_seq_);
            }
            else if (item->wait_seq_ < seq + REGISTERED)
            {
              err = OB_ERR_UNEXPECTED;
              TBSYS_LOG(ERROR, "unexpected error: done before registered, done_seq=%ld real_seq=%ld", seq, item->wait_seq_);
            }
            else
            {
              // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
              //item->data_.done(handle_err);
              item->data_.done(response_data, handle_err);
              // modify:e
              data = item->data_;
              __sync_synchronize();
              item->wait_seq_ = seq + DONE;
            }
          }
          return err;
        }
        int pop(int64_t& seq, DataT& data)
        {
          int err = OB_SUCCESS;
          Item* item = NULL;
          SeqLockGuard guard(lock_);
          int64_t pop = pop_;
          if (NULL == (item = array_.get(pop)))
          {
            err = OB_NOT_INIT;
          }
          else
          {
            SeqLockGuard guard(item->lock_);
            if (pop + DONE == item->wait_seq_)
            {}
            else if (pop + REGISTERED == item->wait_seq_ && item->data_.is_timeout())
            {
              err = OB_PROCESS_TIMEOUT;
              // add by guojinwei [log synchronization][multi_cluster] 20150819:b
              ObLogPostResponse response_data;
              response_data.next_flush_log_id_ = 0;
              response_data.message_residence_time_us_ = -1;
              response_data.slave_status_ = response_data.OFFLINE;
              // add:e
              // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
              //item->data_.done(OB_PROCESS_TIMEOUT);
              item->data_.done(response_data, OB_PROCESS_TIMEOUT);
              // modify:e
            }
            else
            {
              err = OB_EAGAIN;
            }
            if (OB_SUCCESS == err || OB_PROCESS_TIMEOUT == err)
            {
              seq = pop;
              data = item->data_;
              __sync_synchronize();
              item->wait_seq_ = pop + array_.len();
              pop_++;
            }
          }
          return err;
        }
      protected:
        ObFixedArray<Item> array_;
        volatile uint64_t lock_;
        volatile int64_t push_;
        volatile int64_t pop_;
    };

    template<typename T, typename uniq_id>
    struct TypeUniqReg
    {
      static T** value()
      {
        static T* reg = NULL;
        return &reg;
      }
    };

    template <typename reg>
    int static_callback(onev_request_e* arg)
    {
      return (*reg::value())->callback(arg);
    }
  }; // end namespace common
}; // end namespace oceanbase

#endif /* __OB_COMMON_OB_WAIT_QUEUE_H__ */
