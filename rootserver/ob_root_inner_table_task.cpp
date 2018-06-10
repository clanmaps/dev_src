/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_inner_table_task.cpp
 * @brief support multiple clusters for HA by adding or modifying
 * some functions, member variables
 * add the CHANGE_MASTER_CLUSTER_ROOTSERVER task_type to the process.
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

#include "ob_root_inner_table_task.h"
#include "ob_root_async_task_queue.h"
#include "ob_root_sql_proxy.h"

using namespace oceanbase::common;
using namespace oceanbase::rootserver;

ObRootInnerTableTask::ObRootInnerTableTask():cluster_id_(-1), timer_(NULL), queue_(NULL), proxy_(NULL)
{
}

ObRootInnerTableTask::~ObRootInnerTableTask()
{
}

int ObRootInnerTableTask::init(const int cluster_id, ObRootSQLProxy & proxy, ObTimer & timer,
    ObRootAsyncTaskQueue & queue)
{
  int ret = OB_SUCCESS;
  if (cluster_id < 0)
  {
    TBSYS_LOG(WARN, "check init param failed:cluster_id[%d]", cluster_id);
    ret = OB_INVALID_ARGUMENT;
  }
  else
  {
    cluster_id_ = cluster_id;
    timer_ = &timer;
    queue_ = &queue;
    proxy_ = &proxy;
  }
  return ret;
}

int ObRootInnerTableTask::modify_all_server_table(const ObRootAsyncTaskQueue::ObSeqTask & task)
{
  int ret = OB_SUCCESS;
  // write server info to internal table
  char buf[OB_MAX_SQL_LENGTH] = "";
  int32_t server_port = task.server_.get_port();
  char ip_buf[OB_MAX_SERVER_ADDR_SIZE] = "";
  if (false == task.server_.ip_to_string(ip_buf, sizeof(ip_buf)))
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "convert server ip to string failed:ret[%d]", ret);
  }
  else
  {
    switch (task.type_)
    {
      case SERVER_ONLINE:
      {
        const char * sql_temp = "REPLACE INTO __all_server(cluster_id, svr_type,"
          " svr_ip, svr_port, inner_port, svr_role, svr_version)"
          " VALUES(%d,\'%s\',\'%s\',%u,%u,%d,\'%s\');";
        snprintf(buf, sizeof (buf), sql_temp, cluster_id_, print_role(task.role_),
            ip_buf, server_port, task.inner_port_, task.server_status_, task.server_version_);
        break;
      }
      case SERVER_OFFLINE:
      {
        const char * sql_temp = "DELETE FROM __all_server WHERE svr_type=\'%s\' AND"
          " svr_ip=\'%s\' AND svr_port=%d AND cluster_id=%d;";
        snprintf(buf, sizeof (buf), sql_temp, print_role(task.role_),
            ip_buf, server_port, cluster_id_);
        break;
      }
      case ROLE_CHANGE:
      {
        const char * sql_temp = "REPLACE INTO __all_server(cluster_id, svr_type,"
          " svr_ip, svr_port, inner_port, svr_role) VALUES(%d,\'%s\',\'%s\',%u,%u,%d);";
        snprintf(buf, sizeof (buf), sql_temp, cluster_id_, print_role(task.role_),
            ip_buf, server_port, task.inner_port_, task.server_status_);
        break;
      }
      default:
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(WARN, "check input param failed:task_type[%d]", task.type_);
      }
    }
  }
  if (OB_SUCCESS == ret)
  {
    ObString sql;
    sql.assign_ptr(buf, static_cast<ObString::obstr_size_t>(strlen(buf)));
    ret = proxy_->query(true, RETRY_TIMES, TIMEOUT, sql);
    if (OB_SUCCESS == ret)
    {
      TBSYS_LOG(INFO, "process inner task succ:task_id[%lu], timestamp[%ld], sql[%s]",
          task.get_task_id(), task.get_task_timestamp(), buf);
    }
    // add by zcd [multi_cluster] 20150416:b
    /// output the error message and the failed SQL
    else
    {
      TBSYS_LOG(INFO, "process inner task failed:task_id[%lu], timestamp[%ld], sql[%s]",
          task.get_task_id(), task.get_task_timestamp(), buf);
    }
  }
  else
  {
    TBSYS_LOG(INFO, "modify the __all_server failed! task_id[%lu], timestamp[%ld], sql[%s]", task.get_task_id(), task.get_task_timestamp(), buf);
  }
  // add:e
  return ret;
}

int ObRootInnerTableTask::modify_all_cluster_table(const ObRootAsyncTaskQueue::ObSeqTask & task)
{
  int ret = OB_SUCCESS;
  // write cluster info to internal table
  char buf[OB_MAX_SQL_LENGTH] = "";

  if (task.type_ == LMS_ONLINE)
  {
    char ip_buf[OB_MAX_SERVER_ADDR_SIZE] = "";
    if (false == task.server_.ip_to_string(ip_buf, sizeof(ip_buf)))
    {
      ret = OB_INVALID_ARGUMENT;
      TBSYS_LOG(WARN, "convert server ip to string failed:ret[%d]", ret);
    }
    else
    {
      const char * sql_temp = "REPLACE INTO %s"
        "(cluster_id, cluster_vip, cluster_port)"
        "VALUES(%d, \'%s\',%u);";
      snprintf(buf, sizeof (buf), sql_temp, OB_ALL_CLUSTER, cluster_id_, ip_buf,
               task.server_.get_port());
    }
  }
  else if (task.type_ == OBI_ROLE_CHANGE)
  {
    if (OB_SUCCESS == ret)
    {
      // modify by zcd [multi_cluster] 20150406:b
      // 在修改__all_cluster这个表时
      // 也要修改记录的cluster_flow_percent这个字段的值
      // 这个字段表示流量分配比例，主集群是100，备集群是0
      const char * sql_temp = "REPLACE INTO %s"
        "(cluster_id, cluster_role, cluster_flow_percent)"
        "VALUES(%d, %d, %d);";
      snprintf(buf, sizeof (buf), sql_temp, OB_ALL_CLUSTER, cluster_id_, task.cluster_role_, task.cluster_role_ == 1 ? 100 : 0);
      // modify:e
    }
  }
  else
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "check input param failed:task_type[%d]", task.type_);
  }
  if (OB_SUCCESS == ret)
  {
    ObString sql;
    sql.assign_ptr(buf, static_cast<ObString::obstr_size_t>(strlen(buf)));
    ret = proxy_->query(true, RETRY_TIMES, TIMEOUT, sql);
    if (OB_SUCCESS == ret)
    {
      TBSYS_LOG(INFO, "process inner task succ:task_id[%lu], timestamp[%ld], sql[%s]",
          task.get_task_id(), task.get_task_timestamp(), buf);
    }
  // add by zcd [multi_cluster] 20150406:b
    // 在失败的时候也要输出执行错误的sql
    else
    {
      TBSYS_LOG(INFO, "process inner task failed:task_id[%lu], timestamp[%ld], sql[%s]",
          task.get_task_id(), task.get_task_timestamp(), buf);
    }
  }
  else
  {
    TBSYS_LOG(INFO, "modify the __all_cluster failed! task_id[%lu], timestamp[%ld], sql[%s]", task.get_task_id(), task.get_task_timestamp(), buf);
  }
  // add:e
  return ret;
}
// add by zcd [multi_cluster] 20150405:b
// 修改__all_sys_config中master_root_server_ip
// 和master_root_server_port的值的函数
int ObRootInnerTableTask::modify_master_cluster_root_server_ip(const ObRootAsyncTaskQueue::ObSeqTask & task)
{
  int ret = OB_SUCCESS;
  // write cluster info to internal table
  char buf[OB_MAX_SQL_LENGTH] = "";

  if (task.type_ == CHANGE_MASTER_CLUSTER_ROOTSERVER)
  {
    char ip_buf[OB_MAX_SERVER_ADDR_SIZE] = "";
    if (false == task.server_.ip_to_string(ip_buf, sizeof(ip_buf)))
    {
      ret = OB_INVALID_ARGUMENT;
      TBSYS_LOG(WARN, "convert server ip to string failed:ret[%d]", ret);
    }
    else
    {
      const char * sql_temp = "alter system set master_root_server_ip='%s' "
                              "server_type=rootserver,master_root_server_port=%d server_type=rootserver;";
      snprintf(buf, sizeof (buf), sql_temp, ip_buf,
               task.server_.get_port());
    }
  }
  else
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "check input param failed:task_type[%d]", task.type_);
  }
  if (OB_SUCCESS == ret)
  {
    ObString sql;
    sql.assign_ptr(buf, static_cast<ObString::obstr_size_t>(strlen(buf)));
    ret = proxy_->query(true, RETRY_TIMES, TIMEOUT, sql);
    if (OB_SUCCESS == ret)
    {
      TBSYS_LOG(INFO, "process inner task succ:task_id[%lu], timestamp[%ld], sql[%s]",
          task.get_task_id(), task.get_task_timestamp(), buf);
    }
  }
  return ret;
}
// add:e

// add by guojinwei [obi role switch][multi_cluster] 20150916:b
int ObRootInnerTableTask::modify_all_cluster_table_with_id(const ObRootAsyncTaskQueue::ObSeqTask & task)
{
  int ret = OB_SUCCESS;
  // write cluster info to internal table
  char buf[OB_MAX_SQL_LENGTH] = "";

  const char * sql_temp = "REPLACE INTO %s"
    "(cluster_id, cluster_role, cluster_flow_percent)"
    "VALUES(%d, %d, %d);";
  snprintf(buf, sizeof (buf), sql_temp, OB_ALL_CLUSTER, task.cluster_id_, task.cluster_role_, task.cluster_role_ == 1 ? 100 : 0);

  ObString sql;
  sql.assign_ptr(buf, static_cast<ObString::obstr_size_t>(strlen(buf)));
  ret = proxy_->query(true, RETRY_TIMES, TIMEOUT, sql);
  if (OB_SUCCESS == ret)
  {
    TBSYS_LOG(INFO, "process inner task succ:task_id[%lu], timestamp[%ld], sql[%s]",
        task.get_task_id(), task.get_task_timestamp(), buf);
  }
  else
  {
    TBSYS_LOG(INFO, "process inner task failed:task_id[%lu], timestamp[%ld], sql[%s]",
        task.get_task_id(), task.get_task_timestamp(), buf);
  }

  return ret;
}
// add:e

void ObRootInnerTableTask::runTimerTask(void)
{
  if (check_inner_stat() != true)
  {
    TBSYS_LOG(WARN, "check inner stat failed");
  }
  else
  {
    int ret = OB_SUCCESS;
    int64_t cur_time = tbsys::CTimeUtil::getTime();
    int64_t end_time = cur_time + MAX_TIMEOUT;
    while ((OB_SUCCESS == ret) && (end_time > cur_time))
    {
      ret = process_head_task();
      if ((ret != OB_SUCCESS) && (ret != OB_ENTRY_NOT_EXIST))
      {
        TBSYS_LOG(WARN, "process head inner task failed:ret[%d]", ret);
      }
      else
      {
        cur_time = tbsys::CTimeUtil::getTime();
      }
    }
  }
}

int ObRootInnerTableTask::process_head_task(void)
{
  // process the head task
  ObRootAsyncTaskQueue::ObSeqTask task;
  int ret = queue_->head(task);
  if (OB_SUCCESS == ret)
  {
    switch (task.type_)
    {
    case ROLE_CHANGE:
    case SERVER_ONLINE:
    case SERVER_OFFLINE:
      {
        ret = modify_all_server_table(task);
        break;
      }
    case OBI_ROLE_CHANGE:
    case LMS_ONLINE:
      {
        ret = modify_all_cluster_table(task);
        break;
      }
    // add by zcd [multi_cluster] 20150405:b
    case CHANGE_MASTER_CLUSTER_ROOTSERVER:
      {
        ret = modify_master_cluster_root_server_ip(task);
      }
      break;
    // add:e
    // add by guojinwei [obi role switch][multi_cluster] 20150916:b
    case CLUSTER_OBI_ROLE_CHANGE:
      {
        ret = modify_all_cluster_table_with_id(task);
      }
      break;
    // add:e
    default:
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "not supported task right now");
        break;
      }
    }
  }
  // pop the succ task
  if (OB_SUCCESS == ret)
  {
    ObRootAsyncTaskQueue::ObSeqTask pop_task;
    ret = queue_->pop(pop_task);
    if ((ret != OB_SUCCESS) || (pop_task != task))
    {
      ret = OB_ERROR;
      TBSYS_LOG(ERROR, "pop the succ task failed:ret[%d]", ret);
      pop_task.print_info();
      task.print_info();
    }
  }
  return ret;
}
