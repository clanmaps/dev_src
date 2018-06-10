/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_timer_task.cpp
 * @brief ObUpsSlaveMgr
 *     modify by zhangcd: add a timertask ObUpsSetMajorityCountTask
 *     to execute setting majority_count
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

#include "ob_ups_timer_task.h"
#include "ob_update_server_main.h"
namespace oceanbase
{
  namespace updateserver
  {
    ObUpsCheckKeepAliveTask::ObUpsCheckKeepAliveTask()
    {
    }

    ObUpsCheckKeepAliveTask::~ObUpsCheckKeepAliveTask()
    {
    }

    void ObUpsCheckKeepAliveTask::runTimerTask(void)
    {
      ObUpdateServerMain *ups_main = ObUpdateServerMain::get_instance();
      if (NULL == ups_main)
      {
        TBSYS_LOG(WARN, "get ups_main fail");
      }
      else
      {
        ObUpdateServer &ups = ups_main->get_update_server();
        ups.submit_check_keep_alive();
      }
    }

    void ObUpsGrantKeepAliveTask::runTimerTask()
    {
      ObUpdateServerMain *ups_main = ObUpdateServerMain::get_instance();
      if (NULL == ups_main)
      {
        TBSYS_LOG(WARN, "get ups_main fail");
      }
      else
      {
        ObUpdateServer &ups = ups_main->get_update_server();
        ups.submit_grant_keep_alive();
      }
    }

    void ObUpsLeaseTask::runTimerTask()
    {
      ObUpdateServerMain *ups_main = ObUpdateServerMain::get_instance();
      if (NULL == ups_main)
      {
        TBSYS_LOG(WARN, "get ups_main fail");
      }
      else
      {
        ObUpdateServer &ups = ups_main->get_update_server();
        ups.submit_lease_task();
      }
    }

    // add by zhangcd [majority_count_init] 20151118:b
    void ObUpsSetMajorityCountTask::runTimerTask()
    {
      ObUpdateServerMain *ups_main = ObUpdateServerMain::get_instance();
      if (NULL == ups_main)
      {
        TBSYS_LOG(WARN, "get ups_main fail");
      }
      else
      {
        TBSYS_LOG(INFO, "begin runTimerTask of upsSetMajorityCountTask");
        ObUpdateServer &ups = ups_main->get_update_server();
        int ret = OB_SUCCESS;
        if(OB_SUCCESS != (ret = ups.ups_set_majority_count()))
        {
          TBSYS_LOG(WARN, "ups_set_majority_count failed, ret = %d", ret);
        }
      }
    }
    // add:e
  }
}
