/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_rpc_stub.h
 * @brief ObUpsSlaveMgr
 *     modify by zhangcd: modify the class ObUpsSlaveMgr to add
 *     the majority_count setting process
 *
 * @version CEDAR 0.2 
 * @author zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_12_25
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
 *   rongxuan <rongxuan.lc@taobao.com>
 *     - some work details if you want
 */

#ifndef OCEANBASE_UPDATESERVER_OB_SLAVE_MGR_H_
#define OCEANBASE_UPDATESERVER_OB_SLAVE_MGR_H_

#include "common/ob_slave_mgr.h"
#include "common/ob_ack_queue.h"
#include "common/ob_log_generator2.h"
#include "ob_ups_role_mgr.h"

using namespace oceanbase::common;
namespace oceanbase
{
  namespace updateserver
  {
    class ObUpsSlaveMgr : private common::ObSlaveMgr
    {
      public:
        static const int32_t RPC_VERSION = 1;
        static const int64_t MAX_SLAVE_NUM = 8;
        static const int64_t DEFAULT_ACK_QUEUE_LEN = 2048;  //modify by qx 20161207 old =1024 for avoid ack_queue fulled
        ObUpsSlaveMgr();
        virtual ~ObUpsSlaveMgr();

        /// @brief 初始化
        // modify by guojinwei [log synchronization][multi_cluster] 20151117:b
        //int init(IObAsyncClientCallback* callback, ObUpsRoleMgr *role_mgr, ObCommonRpcStub *rpc_stub,
        //    int64_t log_sync_timeout);
        // modify by zhangcd [majority_count_init] 20151118:b
        //int init(IObAsyncClientCallback* callback, ObUpsRoleMgr *role_mgr, ObCommonRpcStub *rpc_stub,
        //    int64_t log_sync_timeout, int32_t slave_count);
        int init(IObAsyncClientCallback* callback, ObUpsRoleMgr *role_mgr, ObCommonRpcStub *rpc_stub,
            int64_t log_sync_timeout);
        // modify:e
        // modify:e

        /// @brief 向各台Slave发送数据
        /// 目前依次向各台Slave发送数据, 并且等待Slave的成功返回
        /// Slave返回操作失败或者发送超时的情况下, 将Slave下线并等待租约(Lease)超时
        /// @param [in] data 发送数据缓冲区
        /// @param [in] length 缓冲区长度
        /// @retval OB_SUCCESS 成功
        /// @retval OB_PARTIAL_FAILED 同步Slave过程中有失败
        /// @retval otherwise 其他错误
        int send_data(const char* data, const int64_t length);
        int set_log_sync_timeout_us(const int64_t timeout);
        int post_log_to_slave(const common::ObLogCursor& start_cursor, const common::ObLogCursor& end_cursor, const char* data, const int64_t length);
        int wait_post_log_to_slave(const char* data, const int64_t length, int64_t& delay);
        //mod by chujiajia [log synchronization][multi_cluster] 20160627:b
        //int64_t get_acked_clog_id() const;
        int64_t get_acked_clog_id();
        //mod:e
        //add by chujiajia [log synchronization][multi_cluster] 20160627:b
        inline int64_t get_acked_clog_id_without_update()
        {
          return acked_clog_id_;
        }
        //add:e
        int get_slaves(ObServer* slaves, int64_t limit, int64_t& slave_count);

        int grant_keep_alive();
        int add_server(const ObServer &server);
        int delete_server(const ObServer &server);
        int reset_slave_list();
        int set_send_log_point(const ObServer &server, const uint64_t send_log_point);
        int get_num() const;
        void print(char *buf, const int64_t buf_len, int64_t& pos);
        // add by zhangcd [majority_count_init] 20151118:b
        void set_ack_queue_majority_count(int32_t majority_count);
        int32_t get_ack_queue_majority_count();
        // add:e
      private:
        int64_t n_slave_last_post_;
        ObUpsRoleMgr *role_mgr_;
        ObAckQueue ack_queue_;
        //add chujiajia [log synchronization][multi_cluster] 20160627:b
        int64_t acked_clog_id_;
        //add:e
    };
  } // end namespace common
} // end namespace oceanbase

#endif // OCEANBASE_COMMON_OB_SLAVE_MGR_H_
