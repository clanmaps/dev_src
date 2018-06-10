/**
* Copyright (C) 2013-2016 ECNU_DaSE.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file group_queue_thread.h
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
#ifndef OCEANBASE_COMMON_COMMIT_QUEUE_H_
#define OCEANBASE_COMMON_COMMIT_QUEUE_H_
#include "ob_queue_thread.h"
#include "updateserver/ob_session_mgr.h"
#include "ob_log_generator2.h"
#include "updateserver/ob_inc_seq.h"
//#include "ob_icommit_queue.h"
#include "ob_ticket_queue.h"
namespace oceanbase
{
  namespace common
  {
    /**
     * @brief The GroupQueueThread class
     * GroupQueueThread is designed for
     * transaction commit thread
     */
    class GroupQueueThread
    {
       static const int64_t QUEUE_WAIT_TIME = 10 * 1000;
       public:
         /**
          * @brief constructor
          */
         GroupQueueThread();
         /**
         * @brief destructor
         */
         virtual ~GroupQueueThread();
       public:
         int init(const int64_t task_num_limit,
                  const int64_t idle_interval);
         void destroy();
         /**
         * @brief push group to transaction commit threads.
         * @param[in] group_id group's id.
         * @param[in] group group.
         * @return OB_SUCCESS if success.
         */
         int push(int64_t group_id, LogGroup *group);
         virtual void on_push_fail(void *ptr) {UNUSED(ptr);};
         virtual void *on_begin() {return NULL;};
         virtual void on_end(void *ptr) {UNUSED(ptr);};
         virtual void on_idle() {};
         virtual void on_process(void *task, void *pdata){UNUSED(task);UNUSED(pdata);};
         //int handle_unfinished_task();
         /**
          * @brief get queue numbers
          * @return queue numbers
          */
         int64_t get_queued_num() const;
       protected:
         ObSeqQueue seq_queue_;///< transaction commit queue that its members from transaction processing threads
         void* special_item_;///< special group for system free time
       private:
         static void *thread_func_(void *data);
       private:
         bool inited_;
         int64_t limit_;
         pthread_t pd_;
         volatile bool run_flag_;
         int64_t idle_interval_;
         int64_t last_idle_time_;

         //ObTicketQueue flush_queue_;
    }__attribute__ ((aligned (CPU_CACHE_LINE)));
  }
}
#endif
