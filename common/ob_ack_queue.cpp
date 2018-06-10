/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ack_queue.cpp
 * @brief
 * 1. support multiple clusters for HA by adding or modifying
 *    some functions, member variables
   2. set majority_count_ in class ObAckQueue
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
#include "ob_ack_queue.h"
#include "ob_result.h"
#include "onev_io.h"
#include "ob_client_manager.h"

namespace oceanbase
{
  namespace common
  {
    struct __ACK_QUEUE_UNIQ_TYPE__ {};
    typedef TypeUniqReg<ObAckQueue, __ACK_QUEUE_UNIQ_TYPE__> TUR;

    // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
    //void ObAckQueue::WaitNode::done(int err)
    void ObAckQueue::WaitNode::done(const ObLogPostResponse& response_data, int err)
    // modify:e
    {
      err_ = err;
      receive_time_us_ = tbsys::CTimeUtil::getTime();
      // add by guojinwei [log synchronization][multi_cluster] 20150819:b
      next_flush_log_id_ = response_data.next_flush_log_id_;
      message_residence_time_us_ = response_data.message_residence_time_us_;
      if (response_data.SYNC == response_data.slave_status_)
      {
        slave_status_ = SLAVE_STAT_SYNC;
      }
      else if (response_data.NOTSYNC == response_data.slave_status_)
      {
        slave_status_ = SLAVE_STAT_NOTSYNC;
      }
      else
      {
        slave_status_ = SLAVE_STAT_OFFLINE;
      }
      // add:e
      if (OB_SUCCESS != err)
      {
        TBSYS_LOG(ERROR, "wait response: err=%d, %s", err, to_cstring(*this));
      }
      // else if (receive_time_us_ > send_time_us_ + delay_warn_threshold_us_)
      // {
      //   TBSYS_LOG(WARN, "slow response: %s", to_cstring(*this));
      // }
    }

    int64_t ObAckQueue::WaitNode::to_string(char* buf, const int64_t len) const
    {
      int64_t pos = 0;
      // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
      //databuff_printf(buf, len, pos, "WaitNode: seq=[%ld,%ld], err=%d, server=%s, send_time=%ld, round_time=%ld, timeout=%ld",
      //                start_seq_, end_seq_, err_, to_cstring(server_), send_time_us_, receive_time_us_ - send_time_us_, timeout_us_);
      databuff_printf(buf, len, pos, "WaitNode: seq=[%ld,%ld], err=%d, server=%s, send_time=%ld, round_time=%ld, timeout=%ld, next_flush_log_id=%ld, status=%d",
                      start_seq_, end_seq_, err_, to_cstring(server_), send_time_us_, receive_time_us_ - send_time_us_, timeout_us_, next_flush_log_id_, slave_status_);
      // modify:e
      return pos;
    }

    // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
    //ObAckQueue::ObAckQueue(): callback_(NULL), client_mgr_(NULL), next_acked_seq_(0), lock_(0)
    ObAckQueue::ObAckQueue(): callback_(NULL), client_mgr_(NULL), next_acked_seq_(0), lock_(0), majority_count_(1)
    // modify:e
    {
      OB_ASSERT(*TUR::value() == NULL);
      *TUR::value() = this;
    }

    ObAckQueue::~ObAckQueue()
    {}

    // modify by guojinwei [log synchronization][multi_cluster] 20151117:b
    //int ObAckQueue::init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len)
    // modify by zhangcd [majority_count_init] 20151118:b
    //int ObAckQueue::init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len, int32_t slave_count)
    int ObAckQueue::init(IObAsyncClientCallback* callback, const ObClientManager* client_mgr, int64_t queue_len)
    // modify:e
    // modify:e
    {
      int err = OB_SUCCESS;
      if (NULL == callback || NULL == client_mgr || queue_len <= 0)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "invalid argument: callback=%p, client_mgr=%p, queue_len=%ld", callback, client_mgr, queue_len);
      }
      else if (OB_SUCCESS != (err = wait_queue_.init(queue_len)))
      {
        TBSYS_LOG(ERROR, "wait_queue.init()=>%d", err);
      }
      else
      {
        callback_ = callback;
        client_mgr_ = client_mgr;
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        // slave_count=>majority_count: 0=>1, 1=>2, 2=>2, 3=>3, 4=>3, ......
        // modify by zhangcd [majority_count_init] 20151118:b
        /**
         * set the default value of majority_count_ as INT32_MAX,
         * if the majority_count_ hasn't been set later, any log committion would not been done.
         **/
        // majority_count_ = (slave_count + 1) / 2 + 1;
        majority_count_ = INT32_MAX;
        // modify:e
        // add:e
      }
      return err;
    }

    int64_t ObAckQueue::get_next_acked_seq()
    {
      int err = OB_SUCCESS;
      SeqLockGuard guard(lock_);
      int64_t old_ack_seq = next_acked_seq_;
      int64_t seq = 0;
      WaitNode node;
      while(OB_SUCCESS == err)
      {
        if (OB_SUCCESS != (err = wait_queue_.pop(seq, node))
            && OB_PROCESS_TIMEOUT != err && OB_EAGAIN != err)
        {
          TBSYS_LOG(ERROR, "wait_queue.pop()=>%d", err);
        }
        else if (OB_PROCESS_TIMEOUT == err)
        {
          // add by guojinwei [log synchronization][multi_cluster] 20150819:b
          node.slave_status_ = SLAVE_STAT_OFFLINE;
          // add:e
          callback_->handle_response(node);
          err = OB_SUCCESS;
        }
        // add by guojinwei [log synchronization][multi_cluster] 20151211:b
        else if (OB_EAGAIN != err)
        {
          update_slave_array(node);
        }
        // add:e
        // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
        //if (OB_SUCCESS == err)
        //{
        //  next_acked_seq_ = node.start_seq_;
        //}
        if (is_single()
            || (OB_SUCCESS == err && SLAVE_STAT_OFFLINE != node.slave_status_))
        {
          // delete by guojinwei [log synchronization][multi_cluster] 20151211:b
          //update_slave_array(node);
          // delete:e
          int64_t acked_num = 0;
          get_acked_num(acked_num);
          if(acked_num > next_acked_seq_)
          {
            next_acked_seq_ = acked_num;
            //TBSYS_LOG(INFO, "next_acked_seq_ is %ld.", next_acked_seq_);
          }
          //TBSYS_LOG(ERROR,"test::zhouhuan get_next_ack, old_ack_seq=%ld, acked_num=%ld, next_acked_seq_=%ld",
                   // old_ack_seq, acked_num, next_flush_seq_);
        }
        // delete by guojinwei [log synchronization][multi_cluster] 20151211:b
        //else if (OB_SUCCESS == err && SLAVE_STAT_OFFLINE == node.slave_status_)
        //{
        //  update_slave_array(node);
        //}
        // delete:e
        // modify:e
      }

      //TBSYS_LOG(ERROR,"test::zhouhuan get_next_ack, old_ack_seq=%ld, next_acked_seq_=%ld", old_ack_seq, next_acked_seq_);
      if (old_ack_seq != next_acked_seq_)
      {
        callback_->on_ack(node);
      }
      return next_acked_seq_;
    }

    int ObAckQueue::many_post(const ObServer* servers, int64_t n_server, int64_t start_seq, int64_t end_seq,
                                    const int32_t pcode, const int64_t timeout_us, const ObDataBuffer& pkt_buffer, int64_t idx)
    {
      int err = OB_SUCCESS;
      // add by guojinwei [log synchronization][multi_cluster] 20150819:b
      next_flush_seq_ = end_seq;
      //TBSYS_LOG(ERROR,"test::zhouhuan: next_flush_seq_=%ld, end_seq=%ld", next_flush_seq_, end_seq);
      // add:e
      int64_t send_time_us = tbsys::CTimeUtil::getTime();
      ObServer null_server;
      if (NULL == servers || n_server < 0 || start_seq < 0 || end_seq < start_seq || timeout_us <= TIMEOUT_DELTA)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "invalid argument: servers=%p[%ld], seq=[%ld,%ld], timeout=%ld", servers, n_server, start_seq, end_seq, timeout_us);
      }
      for(int64_t i = 0; OB_SUCCESS == err && i < n_server; i++)
      {
        if (OB_SUCCESS != (err = post(servers[i], start_seq, end_seq, send_time_us, pcode, timeout_us - TIMEOUT_DELTA, pkt_buffer, idx)))
        {
          TBSYS_LOG(ERROR, "post(%s)=>%d", to_cstring(servers[i]), err);
        }
      }
      // delete by guojinwei [log synchronization][multi_cluster] 20150819:b
      //if (OB_SUCCESS != err)
      //{}
      //else if (OB_SUCCESS != (err = post(null_server, end_seq, end_seq, send_time_us, pcode, timeout_us -TIMEOUT_DELTA, pkt_buffer, -1)))
      //{
      //  TBSYS_LOG(ERROR, "post(fake)=>%d", err);
      //}
      //if (OB_SUCCESS == err && n_server <= 0)
      //{
      //  get_next_acked_seq();
      //}
      // delete:e
      // add by guojinwei [log synchronization][multi_cluster] 20151211:b
      if ((OB_SUCCESS == err)
          && is_single())
      {
        get_next_acked_seq();
      }
      // add:e
      return err;
    }

    int ObAckQueue::post(const ObServer server, int64_t start_seq, int64_t end_seq, int64_t send_time_us,
                           const int32_t pcode, const int64_t timeout_us, const ObDataBuffer& pkt_buffer, int64_t idx)
    {
      int err = OB_SUCCESS;
      int64_t wait_idx = 0;
      WaitNode wait_node;
      if (start_seq < 0 || end_seq < 0 || timeout_us <= TIMEOUT_DELTA)
      {
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(ERROR, "invalid argument: servers=%s, seq=[%ld,%ld], timeout=%ld", to_cstring(server), start_seq, end_seq, timeout_us);
      }
      else
      {
        wait_node.start_seq_ = start_seq;
        wait_node.end_seq_ = end_seq;
        wait_node.server_ = server;
        wait_node.send_time_us_ = send_time_us;
        wait_node.timeout_us_ = timeout_us;
      }
      while(OB_SUCCESS == err)
      {
        if (OB_SUCCESS != (err = wait_queue_.push(wait_idx, wait_node))
            && OB_EAGAIN != err)
        {
          TBSYS_LOG(ERROR, "wait_queue_.push()=>%d", err);
        }
        else if (OB_EAGAIN == err)
        {
          get_next_acked_seq();
          err = OB_SUCCESS;
        }
        else
        {
          if (idx >= 0 && OB_SUCCESS != (err = client_mgr_->post_request_using_dedicate_thread(server, pcode, RPC_VERSION, timeout_us, pkt_buffer,
                                                                                               static_callback<TUR>, (void*)wait_idx, (int)idx)))
          {
            TBSYS_LOG(ERROR, "post_request()=>%d", err);
          }
          if (idx < 0 || OB_SUCCESS != err)
          {
            // add by guojinwei [log synchronization][multi_cluster] 20150819:b
            ObLogPostResponse response_data;
            response_data.next_flush_log_id_ = 0;
            response_data.slave_status_ = response_data.OFFLINE;
            // add:e
            // modify by guojiwnei
            //wait_queue_.done(wait_idx, wait_node, err);
            wait_queue_.done(wait_idx, response_data, wait_node, err);
            // modify:e
            callback_->handle_response(wait_node);
          }
          break;
        }
      }
      return err;
    }

    int ObAckQueue::callback(void* data)
    {
      int ret = ONEV_OK;
      int err = OB_SUCCESS;
      onev_request_e* r = (onev_request_e*)data;
      if (NULL == r || NULL == r->ms)
      {
        TBSYS_LOG(WARN, "request is null or r->ms is null");
      }
      else if (NULL == r->user_data)
      {
        TBSYS_LOG(WARN, "request user_data == NULL");
      }
      else
      {
        ObPacket* packet = NULL;
        ObDataBuffer* response_buffer = NULL;
        ObResultCode result_code;
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        ObLogPostResponse response_data;
        // add:e
        int64_t pos = 0;
        if (NULL == (packet =(ObPacket*)r->ipacket))
        {
          err = OB_RESPONSE_TIME_OUT;
          TBSYS_LOG(WARN, "receive NULL packet, timeout");
        }
        else if (NULL == (response_buffer = packet->get_buffer()))
        {
          err = OB_INVALID_ARGUMENT;
          TBSYS_LOG(WARN, "response has NULL buffer");
        }
        else if (OB_SUCCESS != (err = result_code.deserialize(response_buffer->get_data() + response_buffer->get_position(), packet->get_data_length(), pos)))
        {
          TBSYS_LOG(ERROR, "deserialize result_code failed:pos[%ld], err[%d], server=%s", pos, err, inet_ntoa_r(get_onev_addr(r)));
        }
        // add by guojinwei [log synchronization][multi_cluster] 20150819:b
        else if (OB_SUCCESS != (err = response_data.deserialize(response_buffer->get_data() + response_buffer->get_position(), packet->get_data_length(), pos)))
        {
          TBSYS_LOG(ERROR, "deserialize response_data failed:pos[%ld], err[%d], server=%s", pos, err, inet_ntoa_r(get_onev_addr(r)));
        }
        // add:e
        else
        {
          err = result_code.result_code_;
          // add by guojinwei [log synchronization][multi_cluster] 20151211:b
          if ((OB_SUCCESS == err)
              && (response_data.next_flush_log_id_ > max_received_seq_))
          {
            max_received_seq_ = response_data.next_flush_log_id_;
          }
          // add:e
        }
        WaitNode node;
        int tmperr = OB_SUCCESS;
        // modify by guojinwei [log synchronization][multi_cluster] 20150819:b
        //if (OB_SUCCESS != (tmperr = wait_queue_.done((int64_t)r->user_data, node, err))
        //    && OB_ALREADY_DONE != tmperr)
        if (OB_SUCCESS != (tmperr = wait_queue_.done((int64_t)r->user_data, response_data, node, err))
            && OB_ALREADY_DONE != tmperr)
        // modify:e
        {
          TBSYS_LOG(ERROR, "wait_queue.done()=>%d", tmperr);
        }
        else if (OB_SUCCESS == tmperr)
        {
          callback_->handle_response(node);
          get_next_acked_seq();
        }
      }
      if (NULL != r && NULL != r->ms)
      {
        onev_destroy_session(r->ms);
      }
      return ret;
    }

    // add by guojinwei [log synchronization][multi_cluster] 20150819:b
    int ObAckQueue::get_acked_num(int64_t& acked_num)
    {
      int err = OB_SUCCESS;
      if (is_single()
          || (FIRST_TOLERATE_LOG_NUM >= next_flush_seq_))
      {
        acked_num = next_flush_seq_;
        //TBSYS_LOG(ERROR,"test::zhouhuan acked_num=%ld next_flush_seq=%ld",acked_num, next_flush_seq_);
      }
      // add by guojinwei [log synchronization][multi_custer] 20151211:b
      // if system is 3 cluster
      else if (2 >= majority_count_)
      {
        acked_num = max_received_seq_;
      }
      // add:e
      else if (OB_SUCCESS != (err = get_acked_num_from_slaves(acked_num)))
      {
        TBSYS_LOG(ERROR, "get_acked_num_from_slaves()=>%d", err);
      }
      return err;
    }

    int ObAckQueue::get_acked_num_from_slaves(int64_t& acked_num_from_slaves)
    {
      int err = OB_SUCCESS;
      reset_flag_slave_array();
      int64_t max_seq_temp = 0;
      for (int32_t i = 1; i < majority_count_; i++)
      {
        max_seq_temp = 0;
        int32_t index_temp = -1;
        for (int32_t j = 0; j < SLAVE_COUNT; j++)
        {
          if ((max_seq_temp <= slave_array_[j].next_flush_log_id_) && (false == slave_array_[j].query_flag_))
          {
            max_seq_temp = slave_array_[j].next_flush_log_id_;
            index_temp = j;
          }
        }
        if (-1 == index_temp)
        {
          // there are no available slaves exist, which means the count
          // of responsing slaves have not reach at majority_count_.
          // set the error code as OB_ERROR temporarily.
          TBSYS_LOG(ERROR, "The amount of slaves is less than majority_count.");
          err = OB_ERROR;
          break;
        }
        else if (true == slave_array_[index_temp].query_flag_)
        {
          // normaly the process should not be here
          err = OB_ERROR;
          break;
        }
        else
        {
          slave_array_[index_temp].query_flag_ = true;
        }
      }
      acked_num_from_slaves = max_seq_temp;
      reset_flag_slave_array();
      return err;
    }

    void ObAckQueue::reset_flag_slave_array()
    {
      for(int32_t i = 0; i < SLAVE_COUNT; i++)
      {
        slave_array_[i].query_flag_ = false;
      }
    }

    void ObAckQueue::set_majority_count(int32_t majority_count)
    {
      majority_count_ = majority_count;
    }

    // add by zhangcd [majority_count_init] 20151118:b
    int32_t ObAckQueue::get_majority_count()
    {
      return majority_count_;
    }
    // add:e

    int ObAckQueue::update_slave_array(const WaitNode& wait_node)
    {
      int err = OB_SUCCESS;
      int index = -1;
      if (!is_wait_node_valid(wait_node))
      {
        // do nothing
        err = OB_INVALID_ARGUMENT;
        TBSYS_LOG(WARN, "wait_node is invalid.");
      }
      else if (-1 == (index = find_slave_index(wait_node.server_)))
      {
        // not exit in slave_array
        int32_t j = -1;
        for (j = 0; j < SLAVE_COUNT; ++j)
        {
          if (SLAVE_STAT_OFFLINE == slave_array_[j].slave_status_)
          {
            slave_array_[j].server_ = wait_node.server_;
            slave_array_[j].next_flush_log_id_ = wait_node.next_flush_log_id_;
            slave_array_[j].slave_status_ = wait_node.slave_status_;
            break;
          }
        }
        if (j < 0 || j >= SLAVE_COUNT)
        {
          // 没找到空闲的数组下标,可能slave_array满了
          err = OB_ARRAY_OUT_OF_RANGE;
          TBSYS_LOG(ERROR, "idle_array_index:%d is a wrong number. ", j);
        }
      }
      else if (-1 > index || SLAVE_COUNT <= index)
      {
        // index is beyong the range
        // 正常运行进入不到该分支
        err = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(ERROR, "can not find reasonable index in slave_array: i = %d. ", index);
      }
      else
      {
        slave_array_[index].next_flush_log_id_ = wait_node.next_flush_log_id_;
        slave_array_[index].slave_status_ = wait_node.slave_status_;
      }
      return err;
    }

    int ObAckQueue::find_slave_index(const ObServer &addr) const
    {
      int ret = -1;
      for (int32_t i = 0; i < SLAVE_COUNT; ++i)
      {
        if (slave_array_[i].server_ == addr )  // && UPS_SLAVE_STAT_OFFLINE != slave_array_[i].ups_status_)
        {
          ret = i;
          break;
        }
      }
      return ret;
    }

    bool ObAckQueue::is_wait_node_valid(const WaitNode& wait_node)
    {
      bool ret = true;
      ObServer null_server;
      if (null_server == wait_node.server_)
      {
        ret = false;
      }
      return ret;
    }

    bool ObAckQueue::is_single()
    {
      bool ret = false;
      if (SLAVE_COUNT_ZERO >= (majority_count_ - 1))
      {
        ret = true;
      }
      return ret;
    }
    // add:e
  }; // end namespace common
}; // end namespace oceanbase
