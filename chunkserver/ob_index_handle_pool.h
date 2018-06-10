/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_handle_pool.h
 * @brief multi-thread's pool, each thread is responsible for one tablet
 *
 * Created by longfei： multi-thread to construct secondary index
 * future work
 *   1.some function need to be realized,see todo list in this page
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */

#ifndef CHUNKSERVER_OB_INDEX_HANDLE_POOL_H_
#define CHUNKSERVER_OB_INDEX_HANDLE_POOL_H_
#include <tbsys.h>
#include <Mutex.h>
#include <Monitor.h>
#include "common/ob_define.h"
#include "common/ob_schema.h"
#include "common/ob_vector.h"
#include "common/thread_buffer.h"
#include "common/ob_rowkey.h"
#include "common/ob_range2.h"
#include "common/location/ob_tablet_location_list.h"
#include "common/ob_schema_manager.h"
//add maoxx
#include "common/ob_mod_define.h"
#include "common/ob_tablet_histogram.h"
#include "common/ob_tablet_histogram_report_info.h"
#include "ob_index_reporter.h"
#include "common/ob_tablet_info.h"
//add e
#include "common/ob_index_black_list.h" // add longfei

namespace oceanbase
{
  namespace chunkserver
  {
    using namespace common;
    class ObTabletManager;
    class ObChunkServer;
    class ObIndexHandler;
    class ObGlobalIndexHandler;
    class ObLocalIndexHandler;
    class ObTablet;

    struct FailRecord
    {
      int8_t if_process_;
      int8_t fail_count_;
    };

    struct TabletRecord
    {
      ObTablet* tablet_;
      int8_t fail_count_;
      int8_t if_process_;
      int8_t work_send_;
      TabletRecord()
      {
        tablet_ = NULL;
        fail_count_ = 0;
        if_process_ = 0;
        work_send_ = 0;

      }
    };

    struct RangeRecord
    {
      ObNewRange range_;
      int64_t fail_count_;
      int8_t if_process_;
      int8_t work_send_;
      RangeRecord()
      {
        fail_count_ = 0;
        if_process_ = 0;
        work_send_ = 0;
      }
    };

    enum ErrNo
    {
      LOCAL_INDEX_SST_BUILD_FAILED = 0,
      LOCAL_INDEX_SST_NOT_FOUND,
      GLOBAL_INDEX_BUILD_FAILED,
    };

    /**
     * @brief The ObIndexHandlePool class
     * is designed for thread pool for constructing index
     */
    class ObIndexHandlePool: public tbsys::CDefaultRunnable
    {
    public:
      /**
       * @brief ObIndexHandlePool constructor
       */
      ObIndexHandlePool();
      /**
       * @brief ~ObIndexHandlePool destructor
       */
      ~ObIndexHandlePool(){}
      /**
       * @brief init: Initialization working thread
       * @param manager
       * @return err code
       */
      int init(ObTabletManager *manager);
      /**
       * @brief schedule: Conduct HandlerPool scheduling algorithm based on stage
       * @return return code
       */
      int schedule();
      /**
       * @brief start_round: fetch infomation from rs
       * @return return_code
       */
      int start_round();
      /**
       * @brief set_config_param: Create two hashmap, to save rs segmentation of range
       * @return err code
       */
      int set_config_param();
      /**
       * @brief create_work_thread: Call tbsys::CDefault Runnable's start function to start multiple threads
       * @param max_work_thread_num
       * @return return code
       */
      int create_work_thread(const int64_t max_work_thread_num);
      /**
       * @brief fetch_tablet_info: Retrieve tablet or range information
       * @param which_stage
       * @return return code
       */
      int fetch_tablet_info(common::ConIdxStage which_stage = STAGE_INIT);
      /**
       * @brief is_tablet_need_build_static_index
       * @param tablet
       * @param [out] is_need_index
       * @return return code
       */
      int is_tablet_need_build_static_index(ObTablet* tablet, bool &is_need_index);
      /**
       * @brief construct_tablet_item
       * @param table_id
       * @param start_key
       * @param end_rowkey
       * @param range
       * @param [out]list
       */
      void construct_tablet_item(
          const uint64_t table_id,
          const common::ObRowkey &start_key,
          const common::ObRowkey &end_rowkey,
          common::ObNewRange &range,
          ObTabletLocationList &list);
      /**
       * @brief can_launch_next_round
       * @return true or false
       */
      bool can_launch_next_round();
      /**
       * @brief parse_location_from_scanner: fill range_hash_ or data_multcs_range_hash_
       * @param scanner
       * @param row_key
       * @param table_id
       * @param need_other_cs
       * @return return code
       */
      int parse_location_from_scanner(
          ObScanner &scanner,
          ObRowkey &row_key,
          uint64_t table_id,
          bool need_other_cs = false);
      /**
       * @brief add_tablet_info: for information report
       * @param tablet
       * @return succ or error
       */
      int add_tablet_info(ObTabletReportInfo* tablet);
      /**
       * @brief push_work: push failed work into black list
       * @param [out] list
       * @return ret
       */
      int push_work(BlackList &list);
      /**
       * @brief reset
       */
      void reset();
      /**
       * @brief is_tablet_handle
       * @param [in] tablet
       * @param [out] is_handle
       * @return OB_INVALID_ARGUMENT or succ
       */
      int is_tablet_handle(ObTablet* tablet, bool &is_handle);
      /**
       * @brief try_stop_mission
       * @param index_tid
       * @return ret
       */
      int try_stop_mission(uint64_t index_tid);
      /**
       * @brief check_if_in_processing
       * @param index_tid
       * @return true or false
       */
      bool check_if_in_processing(uint64_t index_tid);
      /**
       * @brief is_work_stoped
       * @return true or false
       */
      inline bool is_work_stoped() const
      {
        return 0 == active_thread_num_ || OB_INVALID_ID == process_idx_tid_;
      }
      /**
       * @brief get_tablet_manager
       * @return tablet_manager_
       */
      inline ObTabletManager* get_tablet_manager()
      {
        return tablet_manager_;
      }
      /**
       * @brief get_round_end
       * @return true or false
       */
      inline bool get_round_end()
      {
        return round_end_ == TABLET_RELEASE;
      }
      /**
       * @brief set_schedule_idx_tid
       * @param index_tid
       * @return succ or already_done
       */
      inline int set_schedule_idx_tid(uint64_t index_tid)
      {
        int ret = OB_SUCCESS;
        if (schedule_idx_tid_ != index_tid)
        {
          schedule_idx_tid_ = index_tid;
        }
        else
        {
          ret = OB_ALREADY_DONE;
        }
        return ret;
      }
      /**
       * @brief get_process_tid
       * @return process_idx_tid_
       */
      inline uint64_t get_process_tid()
      {
        return process_idx_tid_;
      }
      /**
       * @brief set_hist_width
       * @param hist_width
       */
      inline void set_hist_width(int64_t hist_width)
      {
        hist_width_ = hist_width;
      }
      /**
       * @brief get_schedule_idx_tid
       * @return schedule_idx_tid_
       */
      inline uint64_t get_schedule_idx_tid()
      {
        return schedule_idx_tid_;
      }
      /**
       * @brief if_is_building_index: Determine whether there is under construction index
       * @return true or false
       */
      inline bool if_is_building_index()
      {
        //如果process_idx_tid_的值没有有效索引表id，则此时没有建索引的工作
        if (process_idx_tid_ == OB_INVALID_ID)
        {
          return false;
        }
        else
        {
          return true;
        }
      }
      /**
       * @brief check_new_global
       * @return true or false
       */
      inline bool check_new_global()
      {
        return (total_work_start_time_ == 0);
      }
    public:
      /**
       * @brief get_range_info
       * @return data_multcs_range_hash_
       */
      inline hash::ObHashMap <ObNewRange, ObTabletLocationList,
      hash::NoPthreadDefendMode>* get_range_info()
      {
        return &data_multcs_range_hash_;
      }

      // add longfei [cons static index] 151123
    public:
      /**
       * @brief get_which_stage
       * @return which_stage_
       */
      inline common::ConIdxStage get_which_stage()
      {
        return which_stage_;
      }
      /**
       * @brief set_which_stage
       * @param which_stage
       */
      inline void set_which_stage(common::ConIdxStage which_stage)
      {
        which_stage_ = which_stage;
        //设置global阶段的开始时间，用于判断是否是新的global阶段
        if (which_stage_ == GLOBAL_INDEX_STAGE)
        {
          total_work_start_time_ = tbsys::CTimeUtil::getTime();
        }
      }
      // add e

    private:
      const static int64_t MAX_WORK_THREAD = 32;
      const static int32_t TABLET_COUNT_PER_WORK = 10240;
      const static uint32_t MAX_WORK_PER_DISK = 2;
      const static int64_t SLEEP_INTERVAL = 5000000;
      const static int8_t MAX_FAILE_COUNT = 5;

      const static int64_t tablets_num = 200;
      const static int sample_rate = 200;

      const static int8_t ROUND_TRUE = 1;
      const static int8_t ROUND_FALSE = 0;
      const static int8_t TABLET_RELEASE = 2;
    private:
      /**
       * @brief create_all_index_handlers: Allocates space for each handler
       * @return ret
       */
      int create_all_index_handlers();
      /**
       * @brief create_index_handlers
       * @param global_handler
       * @param local_handler
       * @param size
       * @return ret
       */
      int create_index_handlers(
          ObGlobalIndexHandler** global_handler,
          ObLocalIndexHandler** local_handler,
          const int64_t size);
      /**
       * @brief run: multi-thread will execute run() after created
       * @param thread
       * @param arg
       */
      virtual void run(tbsys::CThread* thread, void *arg);
      /**
       * @brief construct_index:thread really do work
       * @param thread_no
       */
      void construct_index(const int64_t thread_no);
      /**
       * @brief get_tablets_ranges: give a signal to awake thread
       * @param tablet
       * @param range
       * @param [out] err
       * @return err code
       */
      int get_tablets_ranges(
          TabletRecord* &tablet,
          RangeRecord* &range,
          int &err);
      /**
       * @brief finish_phase1: finish local stage, send report infomation
       * @param reported
       * @return ret
       */
      int finish_phase1(bool &reported);
      /**
       * @brief finish_phase2: finish global stage, send report infomation
       * @param total_reported
       * @return ret
       */
      int finish_phase2(bool &total_reported);
      //add longfei [cons static index] 151220:b
      /**
       * @brief get_global_index_handler
       * @param thread_no
       * @param [out] global_handler
       * @return ret
       */
      int get_global_index_handler(const int64_t thread_no, ObGlobalIndexHandler* &global_handler);
      //add e
      //add maoxx
      /**
       * @brief get_local_index_handler
       * @param thread_no
       * @param [out] local_handler
       * @return ret
       */
      int get_local_index_handler(const int64_t thread_no, ObLocalIndexHandler* &local_handler);
      //add e
      /**
       * @brief check_if_tablet_range_failed
       * @param is_local_index
       * @param [out] tablet
       * @param [out] range
       * @return err code
       */
      bool check_if_tablet_range_failed(
          bool is_local_index,
          TabletRecord* &tablet,
          RangeRecord* &range);
      /**
       * @brief retry_failed_work: if a tablet/range construct failed in this cs over 3 times,then send to other cs
       * @param level
       * @param tablet
       * @param range
       * @return err code
       */
      int retry_failed_work(
          ErrNo level,
          const ObTablet* tablet,
          ObNewRange range);
      /**
       * @brief is_local_stage_need_end
       * @return true or false
       */
      bool is_local_stage_need_end();
      /**
       * @brief is_global_stage_need_end
       * @return true or false
       */
      bool is_global_stage_need_end();
      /**
       * @brief release_tablet_array: release all tablet && set round_end_
       * @return ret
       */
      int release_tablet_array();
      /**
       * @brief inc_get_tablet_count
       */
      void inc_get_tablet_count();
      /**
       * @brief inc_get_range_count
       */
      void inc_get_range_count();
    private:
      common::ObTabletHistogramReportInfoList *static_index_report_infolist; ///< collect histogram report info
      volatile bool inited_; ///< is inited?
      int64_t thread_num_; ///< thread number
      uint64_t process_idx_tid_; ///< The current process id of the index table
      uint64_t schedule_idx_tid_; ///< Plan to be processed
      int64_t tablet_index_; ///< index of tablet_array_
      int64_t range_index_; ///< index of range_array_
      int64_t hist_width_; ///< histogram's width
      volatile int64_t tablets_have_got_; ///< successed tablets number
      volatile int64_t range_have_got_; ///< successed range number
      volatile int64_t active_thread_num_; ///< active thread number
      int64_t min_work_thread_num_; ///< the min number of work thread is 1
      volatile int8_t round_start_; ///< 1 round  is start, 0 round is not start
      volatile int8_t round_end_; ///< 1 round  is end, 0 round is not start, 2 round is end and all tablet is release
      // add longfei [cons static index] 151122
      common::ConIdxStage which_stage_; ///< index's stage
      // add longfei e
      ObTabletManager *tablet_manager_; ///< tablet manager
      common::ObMergerSchemaManager *schema_mgr_; ///< schema manager
      volatile int64_t local_work_start_time_;    ///< this version start time
      volatile int64_t total_work_start_time_;    ///< this version start time
      volatile int64_t total_work_last_end_time_; ///< if root server mean to stop mission, change it not to be zero
      pthread_cond_t cond_; ///< condition
      pthread_mutex_t mutex_; ///< mutex
      pthread_mutex_t stage_mutex_; ///< stage mutex
      pthread_mutex_t tablet_range_mutex_; ///< tablet range mutex for increse counter
      ObArray<TabletRecord> tablet_array_;///< tablet_array stored all the tablet data
      ObArray<RangeRecord> range_array_;///<  the range is actually to be doing
      //ObIndexHandler *handler_[MAX_WORK_THREAD];
      ObGlobalIndexHandler *global_handler_[MAX_WORK_THREAD]; ///< global working thread
      ObLocalIndexHandler *local_handler_[MAX_WORK_THREAD]; ///< local working thread
      hash::ObHashMap <ObNewRange, ObTabletLocationList, hash::NoPthreadDefendMode> data_multcs_range_hash_; ///< all tablet infomation
      hash::ObHashMap <ObNewRange, ObTabletLocationList, hash::NoPthreadDefendMode> range_hash_; ///< the range need to be construct
      ObTabletReportInfoList report_info_; ///< for cs report
      common::ModuleArena report_allocator_; ///< memory allocator
      CharArena allocator_; ///< for rowkey deep-copy
      BlackListArray black_list_array_; ///< failed list
    };
  }
}
#endif /* CHUNKSERVER_OB_INDEX_HANDLE_POOL_H_ */
