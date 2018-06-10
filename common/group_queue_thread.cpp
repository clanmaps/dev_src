/**
* Copyright (C) 2013-2016 ECNU_DaSE.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file group_queue_thread.cpp
* @brief GroupQueueThread
*        design for scalable commit's transaction commit thread by adding or
*        modifying some functions, member variables
*
* Created by hushuang
*
* @version __DaSE_VERSION
* @author hushuang <51151500017@stu.ecnu.edu.cn>
* @date 2016_07_25
*/
#include "group_queue_thread.h"
#include "ob_queue_thread.h"
#include <sys/syscall.h>
#include <sys/types.h>


namespace oceanbase
{
  namespace common
  {
    GroupQueueThread:: GroupQueueThread() : inited_(false),
                                      pd_(0),
                                      run_flag_(true),
                                      idle_interval_(INT64_MAX),
                                      last_idle_time_(0)
    {
    }

    GroupQueueThread::~GroupQueueThread()
    {
      destroy();
    }

    int GroupQueueThread :: init(const int64_t task_num_limit,
                              const int64_t idle_interval)
    {
      int ret = OB_SUCCESS;
      int temp_ret = 0;
      run_flag_ = true;
	  TBSYS_LOG(INFO, "test::zhouhuan1 group_queue_thread tid = [%ld]",syscall(SYS_gettid));
      if(inited_)
      {
        ret = OB_INIT_TWICE;
      }
      else if(OB_SUCCESS != (ret = seq_queue_.init(task_num_limit)))
      {
        TBSYS_LOG(WARN, "icommit_queue init fail, ret=%d task_num_limit=%ld", ret, task_num_limit);
      }
      else if(0 != (ret = seq_queue_.start(1)))
      {
        TBSYS_LOG(ERROR, "icommit_queue_.start(1)=>%d", ret);
      }
      else if (0 !=  (temp_ret = pthread_create(&pd_, NULL, thread_func_, this)))
      {
        ret = OB_ERR_UNEXPECTED;
        TBSYS_LOG(WARN, "pthread_create fail, ret=%d", temp_ret);
      }
      else
      {
        inited_ = true;
        idle_interval_ = idle_interval;
        last_idle_time_ = 0;
      }
      if (OB_SUCCESS != ret)
      {
        destroy();
      }
      return ret;
    }

    void GroupQueueThread::destroy()
    {
      if(0 != pd_)
      {
        run_flag_ = false;
        pthread_join(pd_, NULL);
        pd_ = 0;
      }
      inited_ =false;
    }


    void *GroupQueueThread::thread_func_(void *data)
    {
      int err = OB_SUCCESS;
      GroupQueueThread *const host = (GroupQueueThread*)data;
      if (NULL == host)
      {
        TBSYS_LOG(WARN, "thread_func param null pointer");
      }
      else
      {
        void *pdata = host->on_begin();
        int64_t group_seq = 0;
        while (host->run_flag_)
        {
          void *task =NULL;
          if (NULL != host->special_item_)
          {
            task = host->special_item_;
            host->on_process(task, NULL);
            host->special_item_ = NULL;
            task = NULL;
          }
          if (OB_SUCCESS != (err = host->seq_queue_.get(group_seq, task, QUEUE_WAIT_TIME))&& OB_EAGAIN != err)
          {
            TBSYS_LOG(ERROR, "get(task=%p)=>%d", task, err);
            break;
          }
          else if (OB_SUCCESS == err)
          {
            host->on_process(task, pdata);
          }
          else if ((host->last_idle_time_ + host->idle_interval_) <= tbsys::CTimeUtil::getTime())
          {
            host->on_idle();
            host->last_idle_time_ = tbsys::CTimeUtil::getTime();
          }
        }
        host->on_end(pdata);
      }
      return NULL;
    }

    int GroupQueueThread::push(int64_t group_id, LogGroup *group)
    {
      //UNUSED(group_id);
      int ret = OB_SUCCESS;
      if(!inited_)
      {
        ret = OB_NOT_INIT;
      }
      else if (NULL == group)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if(OB_INVALID_INDEX != group_id)
      {
        int64_t cur_time = tbsys::CTimeUtil::getTime();
        OB_STAT_INC(UPDATESERVER, UPS_STAT_GROUPS_FTIME, cur_time - group->get_last_proc_time());
        group->set_last_proc_time(cur_time);
        group->group_id_ = group_id;
        //TBSYS_LOG(INFO, "test::zhouhuan push group group_id =>%ld cur_pos=%ld", group->group_id_, group_id);
        ret = seq_queue_.add(group_id + 1,group);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(ERROR, "icommit_queue_.add(group=%p)=>%d", group, ret);
        }
      }
      else
      {
        //on_process(group,NULL);

        special_item_ = group;
      }
      if(OB_SUCCESS != ret)
      {
         on_push_fail(group);
      }
      return ret;
    }

    int64_t GroupQueueThread::get_queued_num() const
    {
      return seq_queue_.next_is_ready()?1:0;
    }


  }
}
