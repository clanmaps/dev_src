/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_switch_group_runnable.h
 * @brief
 * SwitchGroupThread is designed for switching group periodically .
 *
 *
 *
 * @version __DaSE_VERSION
 * @author zhouhuan <zhouhuan@stu.ecnu.edu.cn>
 * @date 2016_07_11
 */
#ifndef OB_SWITCH_GROUP_RUNNABLE
#define OB_SWITCH_GROUP_RUNNABLE
#include "tbsys.h"

namespace oceanbase
{
  namespace updateserver
  {
    class ISwitchGroupHandler{
      public:
        virtual ~ ISwitchGroupHandler(){}
        virtual int handleSwitchGroup() = 0;
    };
    class SwitchGroupThread : public tbsys::CDefaultRunnable
    {
      public:
        SwitchGroupThread() : switch_period_(-1) {};
        virtual ~SwitchGroupThread() {};
        void setThreadParamter(int threadCount, ISwitchGroupHandler *handler, int64_t period);
        virtual void run(tbsys::CThread* thread, void* arg);
        void set_switch_period(const int64_t switch_period)
        {
          switch_period_ = switch_period;
        };
      private:
        int64_t switch_period_;
        ISwitchGroupHandler *_handler;
    };
  }
}

#endif // OB_SWITH_GROUP_RUNNABLE

