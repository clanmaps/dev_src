/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ack_queue.h
 * @brief support multiple clusters for HA by adding or modifying
 *        some functions, member variables
          set majority_count in class ObAckQueue.
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
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
#ifndef __OB_COMMON_OB_ACK_QUEUE_H__
#define __OB_COMMON_OB_ACK_QUEUE_H__
#include "tbsys.h"
#include "ob_wait_queue.h"
#include "ob_log_post.h"
#include "ob_log_generator2.h"

namespace oceanbase
{
  namespace common
  {
    class IObAsyncClientCallback;
    class ObClientManager;
    class ObAckQueue
    {
      public:
        static const int RPC_VERSION = 1;
        enum { TIMEOUT_DELTA = 10000, DEFAULT_DELAY_WARN_THRESHOLD_US = 50000};
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        /**
         * @brief slave status enum
         */
        enum ObSlaveStatus
        {
          SLAVE_STAT_OFFLINE = 0,     ///< the slave is offline
          SLAVE_STAT_NOTSYNC = 1,     ///< slave & not sync
          SLAVE_STAT_SYNC = 2,        ///< slave & sync
        };
        // add:e
        struct WaitNode
        {
          // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
          WaitNode(): start_seq_(-1), end_seq_(-1), err_(OB_SUCCESS), server_(),
                      send_time_us_(0),
          //            timeout_us_(0), receive_time_us_(0) {}
                      timeout_us_(0), receive_time_us_(0),
                      next_flush_log_id_(-1), message_residence_time_us_(-1),
                      slave_status_(SLAVE_STAT_OFFLINE) {}
          // modify:e
          ~WaitNode() {}
          bool is_timeout() const { return timeout_us_ > 0 && send_time_us_ + timeout_us_ < tbsys::CTimeUtil::getTime(); }
          // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
          //void done(int err);
          void done(const ObLogPostResponse& response_data, int err);
          // modify:e
          int64_t get_delay(){ return receive_time_us_ - send_time_us_; }
          int64_t to_string(char* buf, const int64_t len) const;
          int64_t start_seq_;
          int64_t end_seq_;
          int err_;
          ObServer server_;
          int64_t send_time_us_;
          int64_t timeout_us_;
          int64_t receive_time_us_;
          // add by guojinwei [log synchronization][multi_cluster] 20150819:b
          int64_t next_flush_log_id_;
          int64_t message_residence_time_us_;
          ObSlaveStatus slave_status_;
          // add:e
        };
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        /**
         * @brief SlaveNode
         * This structure is designed for recording information of a slave.
         */
        struct SlaveNode
        {
          SlaveNode(): server_(), next_flush_log_id_(-1), query_flag_(false),
                       slave_status_(SLAVE_STAT_OFFLINE)
          {}
          ~SlaveNode() {}
          ObServer server_;               ///< ip and port of the slave
          int64_t next_flush_log_id_;     ///< the next flush LSN in slave
          bool query_flag_;               ///< used for get_acked_num_from_slaves()
          ObSlaveStatus slave_status_;    ///< the status of the slave
        };
        // add:e
      public:
        ObAckQueue();
        ~ObAckQueue();
        // modify by guojinwei [log synchronization][multi_cluster] 20151117:b
        //int init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len);
        // modify by zhangcd [majority_count_init] 20151118:b
        // int init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len, int32_t slave_count);
        int init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len);
        // modify:e
        // modify:e
         int64_t get_next_acked_seq();
        int many_post(const ObServer* servers, int64_t n_server, int64_t start_seq, int64_t end_seq,
                            const int32_t pcode, const int64_t timeout_us, const ObDataBuffer& pkt_buffer, int64_t idx=0);
        int post(const ObServer servers, int64_t start_seq, int64_t end_seq, int64_t send_time_us,
                 const int32_t pcode, const int64_t timeout_us, const ObDataBuffer& pkt_buffer, int64_t idx);
        int callback(void* arg);
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        /**
         * @brief set the number of majority
         * @param[in] majority_count  the number of majority
         */
        void set_majority_count(int32_t majority_count);
        // add by zhangcd [majority_count_init] 20151118:b
        /**
         * @brief get the number of majority
         * @return the number of majority
         */
        int32_t get_majority_count();
        // add:e
        // add:e
      private:
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        int get_acked_num(int64_t& acked_num);
        int get_acked_num_from_slaves(int64_t& acked_num_from_slaves);
        void reset_flag_slave_array();
        int update_slave_array(const WaitNode& wait_node);
        int find_slave_index(const ObServer& addr) const;
        bool is_wait_node_valid(const WaitNode& wait_node);
        bool is_single();
        // add:e
      private:
        IObAsyncClientCallback* callback_;
        const ObClientManager* client_mgr_;
        ObWaitQueue<WaitNode> wait_queue_;
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        volatile int64_t next_flush_seq_;           ///< the next flush LSN in local
        // add:e
        // add by guojinwei [log synchronization][multi_cluster] 20151211:b
        volatile int64_t max_received_seq_;         ///< used for 3-cluster
        // add:e
        volatile int64_t next_acked_seq_;
        volatile uint64_t lock_;
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        static const int32_t SLAVE_COUNT_ZERO = 0;
        static const int32_t SLAVE_COUNT = 6;
        static const int32_t FIRST_TOLERATE_LOG_NUM = 800;
        int32_t majority_count_;                    ///< the number of majority
        SlaveNode slave_array_[SLAVE_COUNT];        ///< salve array storing slave information
        // add:e
    };
    class IObAsyncClientCallback
    {
      public:
        IObAsyncClientCallback(){}
        virtual ~IObAsyncClientCallback(){}
        virtual int handle_response(ObAckQueue::WaitNode& node) = 0; // 删除失败的slave
        virtual int on_ack(ObAckQueue::WaitNode& node) = 0; // 唤醒等待线程
    };
  }; // end namespace common
}; // end namespace oceanbase

#endif /* __OB_COMMON_OB_ACK_QUEUE_H__ */
