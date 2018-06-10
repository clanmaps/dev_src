/**
 * Copyright (C) 2013-2016 DaSE .
 * @file     ob_root_timer_task.h
 * @brief    add a timer task class to run
 *           the set_the set_auto_elect_flag task.
 * @version CEDAR 0.2 
 * @author   zhangcd <zhangcd_ecnu@ecnu.cn>
 * @date     2015-12-25
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

#ifndef __OB_ROOTSERVER_OB_ROOT_TIMER_TASK_H__
#define __OB_ROOTSERVER_OB_ROOT_TIMER_TASK_H__

#include "common/ob_timer.h"

namespace oceanbase
{
  namespace rootserver
  {
    class ObRootWorker;
    class ObRootOperationDuty : public common::ObTimerTask
    {
      public:
        ObRootOperationDuty():worker_(NULL)
      {}
        virtual ~ObRootOperationDuty() {}
        int init(ObRootWorker *worker);
        virtual void runTimerTask(void);
      private:
        ObRootWorker* worker_;
    };
    // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
    class ObRootSetAutoElectFlagTask : public common::ObTimerTask
    {
      public:
        ObRootSetAutoElectFlagTask():worker_(NULL)
      {}
        virtual ~ObRootSetAutoElectFlagTask() {}
        int init(ObRootWorker *worker);
        virtual void runTimerTask(void);
      private:
        ObRootWorker* worker_;
    };
    // add:e
  }
}

#endif

