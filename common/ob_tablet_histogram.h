/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_tablet_histogram.h
* @brief for tablet histogram info
*
* Created by maoxiaoxiao:operations for tablet histogram
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_TABLET_HITOGRAM
#define OB_TABLET_HITOGRAM

#include <tblog.h>
#include "ob_define.h"
#include "ob_server.h"
#include "ob_array_helper.h"
#include "page_arena.h"
#include "ob_range2.h"
#include "ob_vector.h"
#include "ob_se_array.h"

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief The ReportCSInfo struct
     * ReportCSInfo is designed for
     * recording the report chunkserver information in the histogram information
     */
    struct ReportCSInfo
    {
      int32_t server_info_index; ///<server index
      int64_t report_timestamp; ///<time stamp

      /**
       * @brief constructor
       */
      ReportCSInfo():server_info_index(OB_INVALID_INDEX),report_timestamp(0)
      {}
    };

    /**
     * @brief The ObTabletHistogramMeta struct
     * ObTabletHistogramMeta is designed for
     * recording the location of the tablet which the histogram information belongs to
     */
    struct ObTabletHistogramMeta
    {
      int32_t root_meta_index; ///<index in roottable
      ReportCSInfo report_cs_info[common::OB_SAFE_COPY_COUNT]; ///<chunkserver information
      int64_t hist_index; ///<histogram index

      /**
       * @brief constructor
       */
      ObTabletHistogramMeta(): root_meta_index(OB_INVALID_INDEX), hist_index(OB_INVALID_INDEX)
      {}

      /**
       * @brief dump
       */
      void dump() const;
    };

    /**
     * @brief The ObTabletSample struct
     * ObTabletSample is designed for
     * storing one sample information
     */
    struct ObTabletSample
    {
      ObNewRange range; ///<sample range
      int64_t row_count; ///<row count

      /**
       * @brief constructor
       */
      ObTabletSample():row_count(0){}

      /**
       * @brief set_range
       * @param new_range
       */
      inline void set_range(const ObNewRange &new_range)
      {
        range = new_range;
      }

      /**
       * @brief get_range
       * @return range
       */
      inline const ObNewRange& get_range(void) const
      {
        return range;
      }

      /**
       * @brief set_row_count
       * @param count
       */
      inline void set_row_count(const int64_t count)
      {
        row_count = count;
      }

      /**
       * @brief get_row_count
       * @return row_count
       */
      inline int64_t get_row_count(void) const
      {
        return row_count;
      }

      /**
       * @brief dump
       */
      void dump() const;

      template <typename Allocator>
      /**
       * @brief deep_copy
       * deep copy with external allocator
       * @param allocator
       * @param other other ObTabletSample
       * @return OB_SUCCESS or other ERROR
       */
      int deep_copy(Allocator &allocator, const ObTabletSample &other);
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    template <typename Allocator>
    inline int ObTabletSample::deep_copy(Allocator &allocator, const ObTabletSample &other)
    {
      int ret = OB_SUCCESS;
      this->row_count = other.row_count;

      ret = deep_copy_range(allocator, other.range, this->range);
      return ret;
    }

    /**
     * @brief The ObTabletHistogram class
     * ObTabletHistogram is designed for
     * storing one tablet histogram information
     */
    class ObTabletHistogram
    {
    public:

      /**
       * @brief constructor
       */
      ObTabletHistogram()
        :allocator_(NULL)
      {
        init();
      }

      /**
       * @brief constructor
       * @param allocator
       */
      ObTabletHistogram(common::ModuleArena *allocator)
        :allocator_(allocator)
      {
        init();
      }

      /**
       * @brief destructor
       */
      ~ ObTabletHistogram()
      {
        if(NULL != allocator_ && allocator_flag_)
        allocator_->free();
        allocator_ = NULL;
      }

      /**
       * @brief reset
       */
      void reset()
      {
        sample_helper_.clear();
      }

      /**
       * @brief set_allocator
       * @param allocator
       */
      void set_allocator(common::ModuleArena *allocator){
        allocator_ = allocator;
      }

      /**
       * @brief init
       */
      void init()
      {
        sample_helper_.init(MAX_SAMPLE_BUCKET, samples_);
        allocator_flag_ = true;
      }

      /**
       * @brief get_sample_count
       * @return sample_helper_.get_array_index()
       */
      int64_t get_sample_count() const
      {
        return sample_helper_.get_array_index();
      }

      /**
       * @brief do_not_free
       */
      void do_not_free(){allocator_flag_ = false;}

      /**
       * @brief dump
       */
      void dump() const;

      /**
       * @brief get_sample
       * get the specific sample information with the given index
       * @param sample_index
       * @param return_sample
       * @return OB_SUCCESS or other ERROR
       */
      int get_sample(const int64_t sample_index, ObTabletSample *&return_sample);

      /**
       * @brief get_sample
       * get the specific sample information with the given index constantly
       * @param sample_index
       * @param return_sample
       * @return OB_SUCCESS or other ERROR
       */
      int get_sample(const int64_t sample_index, const ObTabletSample *&return_sample) const;

      /**
       * @brief add_sample
       * @param index_sample
       * @return OB_SUCCESS or other ERROR
       */
      int add_sample(const ObTabletSample &index_sample);

      /**
       * @brief add_sample_with_deep_copy
       * deep copy added sample with internal allocator
       * @param index_sample
       * @return OB_SUCCESS or other ERROR
       */
      int add_sample_with_deep_copy(const ObTabletSample &index_sample);

      template <typename Allocator>
      /**
       * @brief add_sample_with_deep_copy
       * deep copy added sample with external allocator
       * @param allocator
       * @param index_sample
       * @return OB_SUCCESS or other ERROR
       */
      int add_sample_with_deep_copy(Allocator &allocator, const ObTabletSample &index_sample);

      /**
       * @brief deep_copy
       * deep copy with internal allocator
       * @param other other ObTabletHistogram
       * @return OB_SUCCESS or other ERROR
       */
      int deep_copy(const ObTabletHistogram &other);

      template <typename Allocator>
      /**
       * @brief deep_copy
       * deep copy with external allocator
       * @param allocator
       * @param other other ObTabletHistogram
       * @return OB_SUCCESS or other ERROR
       */
      int deep_copy(Allocator &allocator, const ObTabletHistogram &other);

      NEED_SERIALIZE_AND_DESERIALIZE;

    public:
      static const int64_t MAX_SAMPLE_BUCKET = 256;

    private:
      common::ModuleArena *allocator_; ///<module arena
      ObTabletSample samples_[MAX_SAMPLE_BUCKET]; ///<sample information in one tablet
      common::ObArrayHelper<ObTabletSample> sample_helper_; ///<array helper
      bool allocator_flag_; ///<allocator_ can be freed only if the allocator_flag_ is true
    };

    template <typename Allocator>
    int ObTabletHistogram::add_sample_with_deep_copy(Allocator &allocator, const ObTabletSample &index_sample)
    {
      int ret = OB_SUCCESS;
      ObTabletSample inner_sample;

      if (sample_helper_.get_array_index() >= MAX_SAMPLE_BUCKET){
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "too many sample in one histogram, max is %ld, now is %ld.", MAX_SAMPLE_BUCKET, sample_helper_.get_array_index());
      }
      else if(OB_SUCCESS != (ret = inner_sample.deep_copy(allocator, index_sample)))
      {
        TBSYS_LOG(WARN, "deep copy index sample failed!");
      }
      else if (!sample_helper_.push_back(inner_sample)) {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "can not push index_sample into sample_list_");
      }
      return ret;
    }

    /**
     * @brief The ObTabletHistogramManager class
     * ObTabletHistogramManager is designed for
     * management of multiple tablet histogram information
     */
    class ObTabletHistogramManager
    {

      public:

        /**
         * @brief constructor
         */
        ObTabletHistogramManager();

        /**
         * @brief alloc_hist_object
         * @return new ObTabletHistogram()
         */
        ObTabletHistogram* alloc_hist_object();

        /**
         * @brief add_histogram
         * add one tablet histogram information
         * @param histogram
         * @param out_index
         * @return OB_SUCCESS or other ERROR
         */
        int add_histogram(const ObTabletHistogram &histogram, int64_t &out_index);

        /**
         * @brief get_histogram
         * get one tablet histogram information with the given index
         * @param hist_index
         * @param out_hist
         * @return OB_SUCCESS or other ERROR
         */
        int get_histogram(const int32_t hist_index, ObTabletHistogram *&out_hist) const;

        /**
         * @brief dump
         * @param hist_index
         */
        void dump(const int32_t hist_index) const;

        /**
         * @brief set_table_id
         * @param tid
         * @param index_tid
         * @return OB_SUCCESS or other ERROR
         */
        int set_table_id(const uint64_t tid, const uint64_t index_tid)
        {
          int ret = OB_SUCCESS;
          if (OB_INVALID_ID == tid || OB_INVALID_ID == index_tid)
          {
            ret = OB_INVALID_ARGUMENT;
            TBSYS_LOG(WARN, "invalid tid, tid=%lu, index_tid=%lu", tid, index_tid);
          }
          else
          {
            table_id_ = tid;
            index_tid_ = index_tid;
          }
          return ret;
        }

        /**
         * @brief get_table_id
         * @return table_id_
         */
        uint64_t get_table_id() const
        {
          return table_id_;
        }

        /**
         * @brief get_index_tid
         * @return index_tid_
         */
        uint64_t get_index_tid() const
        {
          return index_tid_;
        }

        /**
         * @brief reuse
         */
        void reuse();


      public:
        static const int64_t MAX_TABLET_COUNT_PER_TABLE = 4 * 1024;
        common::ModulePageAllocator mod_; ///<module page allocator
        common::ModuleArena module_arena_; ///<module arena

      private:
        ObTabletHistogram* histograms_[MAX_TABLET_COUNT_PER_TABLE]; ///<multiple tablet histogram information
        common::ObArrayHelper<ObTabletHistogram*> histogram_helper_; ///<array helper


        uint64_t table_id_; ///<table id
        uint64_t index_tid_; ///<index table id
        mutable tbsys::CThreadMutex alloc_mutex_; ///<mutex for thread
    };

    inline ObTabletHistogramManager::ObTabletHistogramManager()
      :mod_(ObModIds::OB_RS_TABLET_MANAGER),
      module_arena_(ModuleArena::DEFAULT_PAGE_SIZE, mod_),
      table_id_(OB_INVALID_ID), index_tid_(OB_INVALID_ID)
    {
      histogram_helper_.init(MAX_TABLET_COUNT_PER_TABLE, histograms_);
    }
  }
}

#endif // OB_TABLET_HITOGRAM

