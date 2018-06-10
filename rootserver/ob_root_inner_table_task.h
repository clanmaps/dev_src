/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_inner_table_task.h
 * @brief support multiple clusters for HA by adding or modifying
 *        some functions, member variables
 *        add the modify root_table task to function.
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
  *   zhidong <xielun.szd@taobao.com>
  *     - some work details if you want
  */

#ifndef OB_ROOT_INNER_TABLE_TASK_H_
#define OB_ROOT_INNER_TABLE_TASK_H_

#include "common/ob_timer.h"
#include "ob_root_async_task_queue.h"

namespace oceanbase
{
  namespace rootserver
  {
    class ObRootSQLProxy;
    class ObRootInnerTableTask: public common::ObTimerTask
    {
    public:
      ObRootInnerTableTask();
      virtual ~ObRootInnerTableTask();
    public:
      int init(const int cluster_id, ObRootSQLProxy & proxy, common::ObTimer & timer, ObRootAsyncTaskQueue & queue);
      void runTimerTask(void);
    private:
      // check inner stat
      bool check_inner_stat(void) const;
      // process head task
      int process_head_task(void);
      // update all server table
      int modify_all_server_table(const ObRootAsyncTaskQueue::ObSeqTask & task);
      // update all cluster table
      int modify_all_cluster_table(const ObRootAsyncTaskQueue::ObSeqTask & task);
      // add by zcd [multi_cluster] 20150405:b
      // update the master_cluster_root_server_ip
      int modify_master_cluster_root_server_ip(const ObRootAsyncTaskQueue::ObSeqTask & task);
      // add:e
      // add by guojinwei [obi role switch][multi_cluster] 20150916:b
      /**
       * @brief update inner table __all_cluster with cluster id
       * @param[in] task  inner table task
       * @return OB_SUCCESS if success
       */
      int modify_all_cluster_table_with_id(const ObRootAsyncTaskQueue::ObSeqTask & task);
      // add:e
    private:
      // every run process task timeout
      const static int64_t MAX_TIMEOUT = 2000000; // 2s
      const static int64_t TIMEOUT = 1000000; // 1s
      const static int64_t RETRY_TIMES = 1;
      int cluster_id_;
      common::ObTimer * timer_;
      ObRootAsyncTaskQueue * queue_;
      ObRootSQLProxy * proxy_;
    };
    inline bool ObRootInnerTableTask::check_inner_stat(void) const
    {
      return ((timer_ != NULL) && (queue_ != NULL) && (NULL != proxy_));
    }
  }
}

#endif //OB_ROOT_INNER_TABLE_TASK_H_
