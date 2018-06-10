/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file   ob_root_balancer_runnable.cpp
 * @brief  for running data balance
 *
 * modified by Wenghaixing:now it will not balance while secondary index is being constructed
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

/**
 * (C) 2010-2011 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_root_balancer_runnable.cpp
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#include "ob_root_balancer_runnable.h"
#include "tbsys.h"
using namespace oceanbase::rootserver;
using namespace oceanbase::common;

ObRootBalancerRunnable::ObRootBalancerRunnable(ObRootServerConfig &config,
                                               ObRootBalancer &balancer,
                                               common::ObRoleMgr &role_mgr)
  : config_(config), balancer_(balancer), role_mgr_(role_mgr)
{
  balancer_.set_balancer_thread(this);
}

ObRootBalancerRunnable::~ObRootBalancerRunnable()
{
}

void ObRootBalancerRunnable::wakeup()
{
  balance_worker_sleep_cond_.broadcast();
}

bool ObRootBalancerRunnable::is_master() const
{
  return role_mgr_.is_master();
}

void ObRootBalancerRunnable::run(tbsys::CThread *thread, void *arg)
{
  UNUSED(thread);
  UNUSED(arg);
  TBSYS_LOG(INFO, "[NOTICE] balance worker thread start, waiting [%s]",
            config_.migrate_wait_time.str());
  const int wait_second = (int)config_.migrate_wait_time / 1000L / 1000L;
  for (int64_t i = 0; i < wait_second && !_stop; i++)
  {
    sleep(1);
  }

  TBSYS_LOG(INFO, "[NOTICE] balance working");
  while (!_stop)
  {
    if (is_master() || role_mgr_.get_role() == ObRoleMgr::STANDALONE)
    {
      //modify by wenghaixing [secondary index static_index] 20151216
      //balancer_.do_balance_or_load();
      if(balancer_.check_create_index_over())
      {
        balancer_.do_balance_or_load();
      }
      else
      {
         TBSYS_LOG(INFO, "is building index, do not balance");
      }
      //modify e
    }
    else
    {
      TBSYS_LOG(DEBUG, "not the master");
    }

    int64_t sleep_us = config_.balance_worker_idle_time;
    TBSYS_LOG(TRACE, "balance worker idle, sleep [%s]", config_.balance_worker_idle_time.str());
    int sleep_ms = static_cast<int32_t>(sleep_us/1000);
    balance_worker_sleep_cond_.wait(sleep_ms);
  } // end while
  TBSYS_LOG(INFO, "[NOTICE] balance worker thread exit");
}

