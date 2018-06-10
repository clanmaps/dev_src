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
*   wangjiahao <wjh2006-1@163.com>
*     - some work details if you want
*/
#ifndef OB_TABLE_LOCK_H
#define OB_TABLE_LOCK_H
#include "common/ob_define.h"
#include "tbsys.h"

#define TEST_PRINT_

using namespace oceanbase::common;
namespace oceanbase
{
  namespace updateserver
  {
      struct TbLock
      {
        enum Status {
          EXCLUSIVE_BIT = 1UL<<31,
          UID_MASK = ~EXCLUSIVE_BIT
        };
        TbLock(): n_ref_(0), uid_(0) {}
        ~TbLock() {}
        volatile uint32_t n_ref_;
        volatile uint32_t uid_;

        static inline bool is_timeout(const int64_t end_time)
        {
#ifdef TEST_PRINT_TRY
          TBSYS_LOG(INFO, "##TEST_PRINT## end_time-now=%ld", end_time - tbsys::CTimeUtil::getTime());
#endif
          return end_time > 0 && tbsys::CTimeUtil::getTime() > end_time;
        }

        static inline uint32_t add_intention_ref(uint32_t* ref)
        {
          return __sync_fetch_and_add(ref, 1);
        }

        static inline uint32_t del_intention_ref(uint32_t* ref)
        {
          return __sync_fetch_and_add(ref, -1);
        }

        int try_intention_lock(uint32_t uid)
        {

          int err = OB_SUCCESS;
          UNUSED(uid);
          if (uid_ & EXCLUSIVE_BIT)
          {
            err = OB_EAGAIN;
          }
          else
          {
            add_intention_ref((uint32_t*)&n_ref_);
            if (uid_ & EXCLUSIVE_BIT)
            {
              err = OB_EAGAIN;
              del_intention_ref((uint32_t*)&n_ref_);
            }
          }
#ifdef TEST_PRINT_TRY
          test_print("Try add IX lock", uid, err);
#endif
          return err;
        }

        int try_intention_unlock(uint32_t uid)
        {
          int err = OB_SUCCESS;
          UNUSED(uid);
          del_intention_ref((uint32_t*)&n_ref_);
#ifdef TEST_PRINT_TRY
          test_print("Try free IX lock", uid, err);
#endif
          return err;
        }
/*
        int try_upgrade_lock(const uint32_t uid)
        {
          int err = OB_SUCCESS;
          if (uid & ~UID_MASK)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else if (!__sync_bool_compare_and_swap(&uid_, 0, uid))
          {
            err = OB_EAGAIN;
          }
          return err;
        }

        int try_upgrade_unlock(const uint32_t uid)
        {
          int err = OB_SUCCESS;
          if (uid != uid_)
          {
            err = OB_LOCK_NOT_MATCH;
          }
          else
          {
            __sync_synchronize();
            uid_ = 0;
          }
          return err;
        }
*/
        int try_exclusive_lock(const uint32_t uid)
        {
          int err = OB_SUCCESS;
          if (uid & ~UID_MASK)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else if (0 != n_ref_
                  || !__sync_bool_compare_and_swap(&uid_, 0, uid|EXCLUSIVE_BIT))
          {
            err = OB_EAGAIN;
          }
          else if (0 != n_ref_)
          {
            err = OB_EAGAIN;
            __sync_synchronize();
            uid_ = 0;
          }
#ifdef TEST_PRINT_TRY
          test_print("Try add X lock", uid, err);
#endif
          return err;
        }

        int try_wait_intention_lock_release(const uint32_t uid)
        {
          int err = OB_SUCCESS;
          if ((uid_ & UID_MASK) != uid)
          {
            err = OB_LOCK_NOT_MATCH;
          }
          else if (0 != n_ref_)
          {
            err = OB_EAGAIN;
          }
          else
          {
            __sync_synchronize();
          }
#ifdef TEST_PRINT_TRY
          test_print("Try wait IX lock release", uid, err);
#endif
          return err;
        }

        int try_exclusive_unlock(const uint32_t uid)
        {
          int err = OB_SUCCESS;
          uint32_t cur_uid = uid_;
          if (0 == (cur_uid & ~UID_MASK) || (cur_uid & UID_MASK) != uid)
          {
            err = OB_LOCK_NOT_MATCH;
          }
          else
          {
            __sync_synchronize();
            uid_ = 0;
          }
#ifdef TEST_PRINT_TRY
          test_print("Try free X lock", uid, err);
#endif
          return err;
        }

        int intention_lock(const uint32_t uid, const int64_t end_time = -1)
        {
          int err = OB_EAGAIN;
          while(OB_EAGAIN == (err = try_intention_lock(uid)) && !is_timeout(end_time))
          {
            PAUSE();
          }
#ifdef TEST_PRINT
          test_print("add X lock", uid, err);
#endif
          return err;
        }

        int intention_lock(const uint32_t uid, const int64_t end_time, const bool volatile &wait_flag)
        {
          int err = OB_EAGAIN;
          while(OB_EAGAIN == (err = try_intention_lock(uid)) && wait_flag && !is_timeout(end_time))
          {
            PAUSE();
          }
#ifdef TEST_PRINT
          test_print("add IX lock", uid, err);
#endif
          return err;
        }

        int intention_unlock(const uint32_t uid)
        {
          int err = try_intention_unlock(uid);
#ifdef TEST_PRINT
          test_print("free IX lock", uid, err);
#endif
          return err;
        }

        bool is_exclusive_locked_by(const uint32_t uid) const
        {
          return (uid_ & ~UID_MASK) && (uid == (uid_ & UID_MASK));
        }

        uint32_t get_uid() const
        {
          return (uint32_t)(uid_ & UID_MASK);
        }

        // 多个线程拿着同一个uid去加互斥锁也没关系
        // 同一个线程拿着同一个uid反复加锁，只有第一次可以成功
        int exclusive_lock(const uint32_t uid, const int64_t end_time = -1)
        {
          int err = OB_EAGAIN;
          if (uid & ~UID_MASK || uid == 0)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else
          {
            while(1)
            {
              if (__sync_bool_compare_and_swap(&uid_, 0, uid|EXCLUSIVE_BIT))
              {
                err = OB_SUCCESS;
                break;
              }
              if (is_timeout(end_time))
              {
                break;
              }
              PAUSE();
            }
          }
          if (OB_SUCCESS != err)
          {}
          else if (OB_SUCCESS != (err = wait_intention_lock_release(uid, end_time)))
          {
            uid_ = 0;
          }
#ifdef TEST_PRINT
          test_print("add X lock", uid, err);
#endif
          return err;
        }

        int exclusive_lock(const uint32_t uid, const int64_t end_time, const bool volatile &wait_flag)
        {
          int err = OB_EAGAIN;
          if (uid & ~UID_MASK || uid == 0)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else
          {
            while(1)
            {
              if (__sync_bool_compare_and_swap(&uid_, 0, uid|EXCLUSIVE_BIT))
              {
                err = OB_SUCCESS;
                break;
              }
              if (!wait_flag || is_timeout(end_time))
              {
                break;
              }
              PAUSE();
            }
          }
          if (OB_SUCCESS != err)
          {}
          else if (OB_SUCCESS != (err = wait_intention_lock_release(uid, end_time, wait_flag)))
          {
            uid_ = 0;
          }

#ifdef TEST_PRINT
          test_print("add X lock", uid, err);
#endif
          return err;
        }

        int exclusive_unlock(const uint32_t uid)
        {
          int err = try_exclusive_unlock(uid);
#ifdef TEST_PRINT
          test_print("free X lock", uid, err);
#endif
          return err;
        }

        // 多个线程拿着同一个uid去加互斥锁也没关系
        // 同一个线程拿着同一个uid反复加锁，只有第一次可以成功
        int intention2exclusive_lock(const uint32_t uid, const int64_t end_time = -1)
        {
          int err = OB_EAGAIN;
          uint32_t cur_uid = 0;
          if (uid & ~UID_MASK || uid == 0)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else
          {
            while(1)
            {
              cur_uid = uid_;
              if (cur_uid != 0 || n_ref_ == 0)
              {
                err = OB_EAGAIN;
                break;
              }
              else if (__sync_bool_compare_and_swap(&uid_, cur_uid, uid|EXCLUSIVE_BIT))
              {
                err = OB_SUCCESS;
                break;
              }
              if (is_timeout(end_time))
              {
                break;
              }
              PAUSE();
            }
          }
          if (OB_SUCCESS != err)
          {}
          else if (OB_SUCCESS != (err = intention_unlock(uid))) // 这一步不可能失败
          {}
          else if (OB_SUCCESS != (err = wait_intention_lock_release(uid, end_time)))
          {
            add_intention_ref((uint32_t*)&n_ref_);
            uid_ = cur_uid;
          }
#ifdef TEST_PRINT
          test_print("add IX->X lock", uid, err);
#endif
          return err;
        }

        int intention2exclusive_lock(const uint32_t uid, const int64_t end_time, const bool volatile &wait_flag)
        {
          int err = OB_EAGAIN;
          uint32_t cur_uid = 0;
          if (uid & ~UID_MASK || uid == 0)
          {
            err = OB_INVALID_ARGUMENT;
          }
          else
          {
            while(1)
            {
              cur_uid = uid_;
              if (cur_uid != 0 || n_ref_ == 0)
              {
                err = OB_EAGAIN;
                TBSYS_LOG(WARN, "intention2exclusive_lock failed, uid=%u, n_ref=%u", uid_, n_ref_);
                break;
              }
              else if (__sync_bool_compare_and_swap(&uid_, cur_uid, uid|EXCLUSIVE_BIT))
              {
                err = OB_SUCCESS;
                break;
              }
              if (!wait_flag || is_timeout(end_time))
              {
                break;
              }
              PAUSE();
            }
          }
          if (OB_SUCCESS != err)
          {}
          else if (OB_SUCCESS != (err = intention_unlock(uid))) // 这一步不可能失败
          {}
          else if (OB_SUCCESS != (err = wait_intention_lock_release(uid, end_time, wait_flag)))
          {
            add_intention_ref((uint32_t*)&n_ref_);
            uid_ = cur_uid;
          }
#ifdef TEST_PRINT
          test_print("add IX->X lock", uid, err);
#endif
          return err;
        }

        int exclusive2intention_lock(const uint32_t uid)
        {
          int err = OB_EAGAIN;
          add_intention_ref((uint32_t*)&n_ref_);
          if (OB_SUCCESS != (err = exclusive_unlock(uid)))
          {
            del_intention_ref((uint32_t*)&n_ref_);
          }
#ifdef TEST_PRINT
          test_print("add X->IX lock [NEVER USED]", uid, err);
#endif
          return err;
        }

        int wait_intention_lock_release(const uint32_t uid, const int64_t end_time = -1)
        {
          int err = OB_EAGAIN;
          while(OB_EAGAIN == (err = try_wait_intention_lock_release(uid)) && !is_timeout(end_time))
          {
            PAUSE();
          }
#ifdef TEST_PRINT
          test_print("wait IX lock release", uid, err);
#endif
          return err;
        }

        int wait_intention_lock_release(const uint32_t uid, const int64_t end_time, const bool volatile &wait_flag)
        {
          int err = OB_EAGAIN;
          while(OB_EAGAIN == (err = try_wait_intention_lock_release(uid)) && wait_flag && !is_timeout(end_time))
          {
            PAUSE();
          }
#ifdef TEST_PRINT
          test_print("wait IX lock release", uid, err);
#endif
          return err;
        }
        void test_print(const char* lock_name, uint64_t uid=0, int64_t err=0)
        {
          //
          if (OB_SUCCESS == err)
          {
            TBSYS_LOG(INFO, "##TEST_PRINT##\t %s succ.  session_id=%ld uid=%u n_ref=%u", lock_name, uid, uid_, n_ref_);
          }
          else
          {
            TBSYS_LOG(INFO, "##TEST_PRINT##\t %s failed.  session_id=%ld uid=%u n_ref=%u", lock_name, uid, uid_, n_ref_);
          }
          //
        }
      };
  }
}
#endif // OB_TABLE_LOCK_H
