/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_commit_point_runnable.cpp
 * @brief ObCommitPointRunnable
 * (1) create by liubozhong: flush commit point to disk asynchronously
 * (2) modify by guojinwei: modify the judgment in function run()
 *
 * @version CEDAR 0.2 
 * @author liubozhong <51141500077@edu.cn>
 *         guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */
#include "ob_commit_point_runnable.h"
#include "ob_update_server_main.h"
#include <sys/syscall.h>

#define UPS ObUpdateServerMain::get_instance()->get_update_server()

//add lbzhong [Commit Point] 20150820:b
/**
 * @brief write commitpoint thread
 */
namespace oceanbase
{
  namespace updateserver
  {
    ObCommitPointRunnable::ObCommitPointRunnable()
    {
    }

    ObCommitPointRunnable::~ObCommitPointRunnable()
    {
    }

    void ObCommitPointRunnable::run(tbsys::CThread * thread, void * arg)
    {
      UNUSED(thread);
      UNUSED(arg);
      TBSYS_LOG(INFO, "[NOTICE] CommitPointRunnable start");
      TBSYS_LOG(INFO, "test::zhouhuan1 Commit Point Runnable tid = [%ld]", syscall(SYS_gettid));
      while (!_stop)
      {
        // modify by guojinwei [commit point for log replay][multi_cluster] 20151127:b
        //if(UPS.get_log_mgr().is_master_master())
        if(UPS.get_log_mgr().is_master_master()
            && UPS.is_master_lease_valid()
            && (common::OB_COMMIT_POINT_ASYNC == UPS.get_param().commit_point_sync_type))
        // modify:e
        {
          TBSYS_LOG(INFO, "async flush commit point");
            UPS.get_log_mgr().flush_commit_point();
        }
        usleep(100*1000);
      }
    }
  }
}

//add:e
