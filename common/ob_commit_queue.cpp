#include "common/ob_commit_queue.h"

namespace oceanbase
{
  namespace common
  {
    int CommitCallBack::init(ObCond *cond)
    {
      int ret = OB_SUCCESS;
      if(NULL == cond)
      {
        ret = OB_ERROR;
      }
      else
      {
        commit_cond = cond;
      }
      return ret;
    }

    int CommitCallBack::cb_func()
    {
      int ret = OB_SUCCESS;
      if(NULL == commit_cond)
      {
        ret = OB_NOT_INIT;
      }
      else
      {
        commit_cond->signal();
      }
      return ret;
    }

    int ObCommitQueue::init(int64_t task_limit)
    {
      int ret = OB_SUCCESS;
      if(OB_SUCCESS != (ret = task_queue_.init(task_limit)))
      {
        TBSYS_LOG(WARN, "task queue init failed=>%d", ret);
      }
      else
      {
//        flush_ok_callback = &ack_;
        //TBSYS_LOG(ERROR, "test::zhouhuan ack_ pointer %p new val %p", &ack_, flush_ok_callback);
//        session_mgr_ = session_mgr;
//        reserved_ = reserved;
//        lock_mgr_ = lock_mgr;
//        end_func_ = func;
      }
      return ret;
    }

    int ObCommitQueue::submit(ICommitTask *task)
    {
      int ret = OB_SUCCESS;
      //void *data = NULL;
      if(NULL == task)
      {
        ret = OB_ERROR;
      }
      /*else if(0 == task_queue_.get_total())
      {
        ret = task_queue_.push(task);
      }*/
      /*else if(OB_SUCCESS != (ret = task_queue_.pop(data)))
      {
        ret = OB_ERROR;
      }
      else if(task->get_seq() <= ((ICommitTask*)data)->get_seq())
      {
        ret = OB_ERROR;
      }*/
      else
      {
        ret = task_queue_.push(task);
      }
      //TBSYS_LOG(ERROR,"test::zhouhuan task_log_id=%ld,thread_no=%ld, task_queue.size=%ld", task->log_id_, task->thread_no, task_queue_.get_total());
      return ret;
    }

    int ObCommitQueue::next(void *&ptask)
    {

      return task_queue_.next(ptask);
    }

    int ObCommitQueue::pop(void * &ptask)
    {
      return task_queue_.pop(ptask);
    }

    int64_t ObCommitQueue::get_total() const
    {
      return task_queue_.get_total();
    }

//    int ObCommitQueue::handle_unfinished_task(void *host, void *pdata, uint64_t idx)
//    {
//      int ret = OB_SUCCESS;
//      void *ptask = NULL;
//      ICommitTask *task = NULL;
//      UNUSED(idx);
//      while(task_queue_.get_total() > 0)
//      {
//        //TBSYS_LOG(ERROR, "test::zhouhuan handle_unfinished_task start!! task_queue.size=[%ld]", task_queue_.get_total());
//        if(OB_SUCCESS != (ret = task_queue_.next(ptask)))
//        {
//          TBSYS_LOG(ERROR, "failed to next data, %d",ret);
//          break;
//        }
//        else
//        {
//          task = (ICommitTask*)ptask;
//          //TBSYS_LOG(ERROR,"test::zhouhuan task_queue_.next()");
//        }

//       // TBSYS_LOG(ERROR, "test::zhouhuan begin to end_task,reserved_=[%ld] > task.log_id=[%ld] size=%ld thread_no=%ld",
//         //         *reserved_, task->get_seq(),task_queue_.get_total(), idx);
//        if( *reserved_ >= task->get_seq() )
//        {
//          if( OB_SUCCESS != (ret = end_func_(host, ptask, pdata, session_mgr_, lock_mgr_)))
//          {
//            TBSYS_LOG(WARN, "end session failed, ret = %d", ret);
//            break;
//          }
//          else
//          {
//            ret = task_queue_.pop(ptask);
//          }
//        }
//        else
//        {
//          break;
//        }
//     }
//     return ret;
//    }

//    int ObCommitQueue::set_cond(ObCond *cond)
//    {
//      return ack_.init(cond);
//    }

//    void ObCommitQueue::ack()
//    {
//      if(NULL != cond_)
//      {
//        cond_.singal();
//      }
//    }


  }
}
