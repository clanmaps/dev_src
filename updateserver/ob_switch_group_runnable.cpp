/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_switch_group_runnable.cpp
 * @brief
 * SwitchGroupThread is designed for switching group periodically .
 *
 *
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_07_11
 */
#include "ob_switch_group_runnable.h"
#include <sys/syscall.h>
#include "ob_update_server_main.h"

#define UPS ObUpdateServerMain::get_instance()->get_update_server()//add by zhouhuan 20160723

namespace oceanbase
{
  namespace updateserver
  {
    void SwitchGroupThread::setThreadParamter(int threadCount, ISwitchGroupHandler *handler, int64_t period)
    {
      setThreadCount(threadCount);
      set_switch_period(period);
      _handler = handler;
    }

    void SwitchGroupThread::run(tbsys::CThread *thread, void *arg)
    {
      UNUSED(thread);
      UNUSED(arg);
      TBSYS_LOG(INFO,"test::zhouhuan1 Switch group thread tid = [%ld]", syscall(SYS_gettid));
      while (!_stop)
      {
        _handler->handleSwitchGroup();
        usleep(static_cast<useconds_t>(switch_period_));
      }
    }
  }//end namespace updateserver
}//end namespace oceanbase
