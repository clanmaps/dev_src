/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_timer_task.cpp
 * @brief add a timer task class to run
 *        the set_the set_auto_elect_flag task.
 *
 * @version CEDAR 0.2 
 * @author zhangcd <zhangcd_ecnu@ecnu.cn>
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
 *   rongxuan <rongxuan.lc@taobao.com>
 *     - some work details if you want
 */

#include "rootserver/ob_root_timer_task.h"
#include "rootserver/ob_root_worker.h"
#include  "common/ob_define.h"
namespace oceanbase
{
  namespace rootserver
  {
    int ObRootOperationDuty::init(ObRootWorker *worker)
    {
      int ret = OB_SUCCESS;
      if (NULL == worker)
      {
        TBSYS_LOG(WARN, "invalid argument. worker=NULL");
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        worker_ = worker;
      }
      return ret;
    }
    void ObRootOperationDuty::runTimerTask(void)
    {
      worker_->submit_check_task_process();
    }

    // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
    int ObRootSetAutoElectFlagTask::init(ObRootWorker *worker)
    {
      int ret = OB_SUCCESS;
      if (NULL == worker)
      {
        TBSYS_LOG(WARN, "invalid argument. worker=NULL");
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        worker_ = worker;
      }
      return ret;
    }
    void ObRootSetAutoElectFlagTask::runTimerTask(void)
    {
      int ret = worker_->set_auto_elect_flag(true);
      if(OB_SUCCESS != ret)
      {
        if(OB_SUCCESS != (ret = worker_->schedule_set_auto_elect_flag_task()))
        {
          TBSYS_LOG(WARN, "worker_->schedule_set_auto_elect_flag_task() failed.");
        }
      }
    }
    // add:e
  }
}

