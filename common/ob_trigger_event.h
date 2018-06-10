/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_trigger_event.h
 * @brief log all trigger event
 *
 * modified by longfei：change trigger event timeout from 2s to 5s
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_21
 */

/*
 * (C) 2007-2012 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version:  ob_trigger_event.h,  12/17/2012 11:23:00 AM xiaochu Exp $
 *
 * Author:
 *   xiaochu.yh <xiaochu.yh@taobao.com>
 * Description:
 *
 *
 */
#ifndef __OCEANBASE_COMMON_OB_TRIGGER_EVENT_H__
#define __OCEANBASE_COMMON_OB_TRIGGER_EVENT_H__
#include "roottable/ob_ms_provider.h"
#include "ob_general_rpc_stub.h"
#include "ob_define.h"
#include "ob_server.h"

namespace oceanbase
{
  namespace common
  {
    class ObTriggerEvent
    {
      public:
        ObTriggerEvent(){}
        ~ObTriggerEvent(){}

        int set_rpc_stub(ObGeneralRpcStub *rpc_stub)
        {
          int ret = OB_SUCCESS;
          if (NULL == (rpc_stub_ = rpc_stub))
          {
            TBSYS_LOG(WARN, "invalid rpc stub. null pointer.");
            ret = OB_INVALID_ARGUMENT;
          }
          return ret;
        }

        int set_ms_provider(ObMsProvider *provider)
        {
          int ret = OB_SUCCESS;
          if (NULL == (ms_provider_ = provider))
          {
            TBSYS_LOG(WARN, "invalid ms provider. null pointer.");
            ret = OB_INVALID_ARGUMENT;
          }
          return ret;
        }

        int execute_sql(ObString &sql_str)
        {
          int ret = OB_SUCCESS;
          if (NULL == ms_provider_ || NULL ==rpc_stub_)
          {
            ret = OB_NOT_INIT;
          }
          else
          {
            ObServer ms;
            if (OB_SUCCESS != (ret = ms_provider_->get_ms(ms)))
            {
              TBSYS_LOG(WARN, "fail to get ms from ms_provider. ret=%d", ret);
            }
            else if (OB_SUCCESS != (ret = rpc_stub_->execute_sql(timeout, ms, sql_str)))
            {
              TBSYS_LOG(WARN, "fail to execute sql on ms(%s). ret=%d", to_cstring(ms), ret);
            }
          }
          return ret;
        }
      private:
        //mod longfei 160118:b
        //debug:trigger event执行replace时间超过2s
        //static const int64_t timeout = 2 * 1000L * 1000L; // 2s
        static const int64_t timeout = 5 * 1000L * 1000L; ///<  change trigger event timeout from 2s to 5s
        //mod e

        ObMsProvider *ms_provider_;
        ObGeneralRpcStub *rpc_stub_;
    };
  }; // end namepsace common
}; // end namespace oceanbase
#endif //__OCEANBASE_COMMON_OB_TRIGGER_EVENT_H__
