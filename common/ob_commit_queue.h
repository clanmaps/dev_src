/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_commit_queue.h
 * @brief
 * ObCommitQueue: A non-spinlock queue used to handle commited task
 *
 *
 *
 * @version __DaSE_VERSION
 * @author hushuang<kindaichweng@gmail.com>
 * @date 2016_04_15
 */

#ifndef _OB_COMMIT_QUEUE_H
#define _OB_COMMIT_QUEUE_H
#include "common/ob_fixed_queue.h"
#include "common/ob_bit_set.h"
#include "common/ob_transaction.h"

namespace oceanbase
{
  namespace common
  {
    class ObCond
    {
      static const int64_t SPIN_WAIT_NUM = 0;
      static const int64_t BUSY_INTERVAL = 1000;
      public:
        ObCond(const int64_t spin_wait_num = SPIN_WAIT_NUM);
        ~ObCond();
      public:
        void signal();
        int timedwait(const int64_t time_us);
      private:
        const int64_t spin_wait_num_;
        volatile bool bcond_;
        int64_t last_waked_time_;
        pthread_cond_t cond_;
        pthread_mutex_t mutex_;
    } __attribute__ ((aligned (64)));

    /**
    * @brief The ISessionMgr class
    * ISessionMgr is designed for SessionMgr's base class
    */
    class ISessionMgr
    {
      public:
        virtual int update_commited_trans_id(int64_t trans_id) = 0;\
        //virtual int get_group_id(uint32_t session_descriptor, int64_t &group_id) = 0;
        virtual int update_published_trans_id(const int64_t session_descriptor) = 0;
    };

    /**
    * @brief The IAckCallback class
    * IAckCallback is designed for callback
    */
    class IAckCallback
    {
      public:
        /**
        * @brief destructor
        */
        virtual ~IAckCallback(){};
      public:
        virtual int cb_func()=0;
    };

    /**
    * @brief The ICommitTask struct
    * ICommitTask is designed for callback
    */
    struct ICommitTask
    {
      int64_t     log_id_;
//      uint64_t    thread_no;

      int64_t get_seq() { return log_id_;}
    };

//    struct ICommitTask
//    {
//      public:
//      void *reserved_ptr_;
//      void *reserved_mgr_;
//      public:
////        virtual int end_session() = 0;
////        virtual int64_t get_seq() = 0;
//        void set_ptr(void* ptr){reserved_ptr_ = ptr;}
//        void set_mgr(void* ptr){reserved_mgr_ = ptr;}
//    };

    struct CommitCallBack : public IAckCallback
    {
      ObCond *commit_cond;
      int init(ObCond *cond);
      virtual int cb_func();
    };

    /*struct AckCallbackInfo
    {
      CallbackList* head;
      CallbackList* list;
      ObBitSet<>    bit_set;
      AckCallbackInfo()
      {
        head = NULL;
        list = NULL;
      }
      int add_callback(uint64_t no, IAckCallback* cb)
      {
        int ret = OB_SUCCESS;

        if(NULL == head && NULL == list)
        {
          list = cb;
          head = list;
          bit_set.add_member((int32_t)no);
        }
        else if(!bit_set.has_member((int32_t)no))
        {
          list->
        }
        return ret;
      }
    };*/

    struct CallbackList
    {
      const static int64_t MAX_CB_SIZE = 256;
      int64_t index;
      IAckCallback* callback[MAX_CB_SIZE];
      ObBitSet<>bit_set;
      CallbackList()
      {
        index = -1;
        bit_set.clear();
      }
      void reset()
      {
        index = -1;
        bit_set.clear();
      }
      int add_cb_func(uint64_t idx, IAckCallback *cb)
      {
        int ret = OB_SUCCESS;
        if(NULL == cb)
        {
          ret = OB_ERROR;
        }
        else if(!bit_set.has_member((int32_t)idx))
        {
          index ++;
          callback[index] = cb;
          bit_set.add_member((int32_t)idx);
        }
        return ret;
      }
    };

    class ObCommitQueue
    {
      public:
//        typedef int (*end_session_func)(void *host, void *ptask, void *pdata, ISessionMgr *mgr, void *lock_mgr);

        int init(int64_t task_limit);
        int submit(ICommitTask *task);
//        int handle_unfinished_task(void *host, void *pdata, uint64_t idx);
//        int set_cond(ObCond *cond);
        //void ack();

        int next(void * &ptask);
        int64_t get_total() const;
        int pop(void * &ptask);

      private:

        ObFixedQueue<void> task_queue_;
//        CommitCallBack     ack_;
//        ISessionMgr*       mgr_;
//        int64_t *          reserved_; //you can used it for tbd
        //ObCond *cond_;
//        end_session_func end_func_;
//        ISessionMgr *session_mgr_;
//        void *lock_mgr_;
   };


  }  //end of common
}    //end of oceanbase




#endif // OB_COMMIT_QUEUE

