/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_tablet_histogram.cpp
* @brief for tablet histogram info
*
* Created by maoxiaoxiao:operations for tablet histogram
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#include "ob_tablet_histogram.h"
#include "ob_tablet_info.h"

namespace oceanbase {
  namespace common {

    void ObTabletHistogramMeta::dump() const
    {
      for (int32_t i = 0; i < common::OB_SAFE_COPY_COUNT; i++)
      {
        TBSYS_LOG(INFO, "server_info_index = %d, timestamp = %ld", report_cs_info[i].server_info_index, report_cs_info[i].report_timestamp);
      }
    }

    void ObTabletSample::dump() const
    {
      TBSYS_LOG(INFO, "sample range = %s", to_cstring(range));
      TBSYS_LOG(INFO, "row_count = %ld", row_count);
    }

    void ObTabletHistogram::dump() const
    {
      for (int32_t i = 0; i < sample_helper_.get_array_index(); i++)
      {
        samples_[i].dump();
      }
    }

    int ObTabletHistogram::get_sample(const int64_t sample_index, ObTabletSample *&return_sample)
    {
      int ret = OB_SUCCESS;
      if (sample_index < 0 || sample_index >= sample_helper_.get_array_index())
      {
        TBSYS_LOG(WARN, "array index out of range.");
        ret = OB_ARRAY_OUT_OF_RANGE;
      }
      else
      {
        return_sample = samples_ + sample_index;
      }
      return ret;
    }

    int ObTabletHistogram::get_sample(const int64_t sample_index, const ObTabletSample *&return_sample) const
    {
      int ret = OB_SUCCESS;
      if (sample_index < 0 || sample_index >= sample_helper_.get_array_index())
      {
        TBSYS_LOG(WARN, "array index out of range.");
        ret = OB_ARRAY_OUT_OF_RANGE;
      }
      else
      {
        return_sample = samples_ + sample_index;
      }
      return ret;
    }

    /*
     * fleet copy
     */
    int ObTabletHistogram::add_sample(const ObTabletSample &index_sample)
    {
      int ret = OB_SUCCESS;
      if (sample_helper_.get_array_index() >= MAX_SAMPLE_BUCKET)
      {
        //debugb longfei 2016-03-18 13:19:23
        //打印sample_helper_信息
//        for (int64_t i = 0; i < sample_helper_.get_array_index(); i++)
//        {
//          TBSYS_LOG(WARN, "debug::longfei>>>sample_helper_[%ld].range is [%s], row_count is [%ld]",
//                    i,
//                    to_cstring(sample_helper_.at(i)->range),
//                    sample_helper_.at(i)->row_count);
//        }
        //debuge
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "too many sample in one histogram, max is %ld, now is %ld.", MAX_SAMPLE_BUCKET, sample_helper_.get_array_index());
      }
      else if (!sample_helper_.push_back(index_sample))
      {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "can not push index_sample into sample_list_");
      }
      return ret;
    }

    /*
     * deep copy add sample with internal allocator_
     */
    int ObTabletHistogram::add_sample_with_deep_copy(const ObTabletSample &index_sample)
    {
      int ret = OB_SUCCESS;
      ObTabletSample inner_sample;

      if (NULL == allocator_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "allocator_ is null.");
      }
      else if (sample_helper_.get_array_index() >= MAX_SAMPLE_BUCKET)
      {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "too many sample in one histogram, max is %ld, now is %ld.", MAX_SAMPLE_BUCKET, sample_helper_.get_array_index());
      }
      else if(OB_SUCCESS != (ret = inner_sample.deep_copy(*allocator_, index_sample)))
      {
        TBSYS_LOG(WARN, "deep copy index sample failed!");
      }
      else if (!sample_helper_.push_back(inner_sample))
      {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "can not push index_sample into sample_list_");
      }
      return ret;
    }

    /*
     * deep copy with internal allocator_
     */
    int ObTabletHistogram::deep_copy(const ObTabletHistogram &other)
    {
      int ret = OB_SUCCESS;
      int64_t size = other.get_sample_count();
      for (int64_t i = 0; i < size; i++)
      {
        const ObTabletSample *sample = NULL;
        if (OB_SUCCESS != (ret = other.get_sample(i, sample)))
        {
          TBSYS_LOG(WARN, "get sample failed");
          break;
        }
        else if (NULL == sample)
        {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "sample is NULL");
          break;
        }
        else if (OB_SUCCESS != (ret = add_sample_with_deep_copy(*sample)))
        {
          TBSYS_LOG(WARN, "add sample failed");
          break;
        }
      }

      return ret;
    }

    /*
     * deep copy with specific allocator
     */
    template <typename Allocator>
    int ObTabletHistogram::deep_copy(Allocator &allocator, const ObTabletHistogram &other)
    {
      int ret = OB_SUCCESS;
      int64_t size = other.get_sample_count();
      for (int64_t i = 0; i < size; i++)
      {
        const ObTabletSample *sample = NULL;
        if (OB_SUCCESS != (ret = other.get_sample(i, sample)))
        {
          TBSYS_LOG(WARN, "get sample failed");
          break;
        }
        else if (NULL == sample)
        {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "sample is NULL");
          break;
        }
        else if (OB_SUCCESS != (ret = add_sample_with_deep_copy(allocator, *sample)))
        {
          TBSYS_LOG(WARN, "add sample failed");
          break;
        }
      }

      return ret;
    }

    void ObTabletHistogramManager::reuse()
    {
      tbsys::CThreadGuard mutex_gard(&alloc_mutex_);
      module_arena_.reuse();
      histogram_helper_.clear();
      table_id_ = OB_INVALID_ID;
      index_tid_ = OB_INVALID_ID;
    }


    int ObTabletHistogramManager::get_histogram(const int32_t hist_index, ObTabletHistogram *&out_hist) const
    {
      int ret = OB_SUCCESS;
      out_hist = NULL;
      if (hist_index < 0 || hist_index >= histogram_helper_.get_array_index())
      {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "index out of range, index=%d, histogram_helper_.size()=%ld", hist_index, histogram_helper_.get_array_index());
      }
      else
      {
        out_hist = histograms_[hist_index];
      }
      return ret;
    }

    int ObTabletHistogramManager::add_histogram(const ObTabletHistogram &histogram, int64_t &out_index)
    {
      int ret = OB_SUCCESS;
      out_index = OB_INVALID_INDEX;
      const ObTabletSample *first_sample = NULL;
      if (histogram.get_sample_count() < 1)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "added histogram is null.");
      }
      else if (OB_SUCCESS != (ret = histogram.get_sample(0, first_sample)))
      {
        TBSYS_LOG(WARN, "get first sample failed.");
      }
      else if (NULL == first_sample)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "first sample is NULL.");
      }
      else if (index_tid_ != first_sample->range.table_id_)
      {
        ret = OB_INVALID_DATA;
        TBSYS_LOG(WARN, "index tid is not consistent. index_tid_[%lu], range tid[%lu]"
                  , index_tid_, first_sample->range.table_id_);
      }
      else if (histogram_helper_.get_array_index() >= MAX_TABLET_COUNT_PER_TABLE)
      {
        ret = OB_ARRAY_OUT_OF_RANGE;
        TBSYS_LOG(WARN, "too many histogram in hist manager, max is %ld, now is %ld.", MAX_TABLET_COUNT_PER_TABLE, histogram_helper_.get_array_index());
      }
      else
      {
        ObTabletHistogram* inner_hist = alloc_hist_object();
        if (NULL == inner_hist)
        {
          ret = OB_ERROR ;
          TBSYS_LOG(WARN, "alloc hist object failed.");
        }
        else if (OB_SUCCESS != (ret = inner_hist->deep_copy(module_arena_, histogram)))
        {
          ret = OB_ARRAY_OUT_OF_RANGE;
          TBSYS_LOG(WARN, "deep copy histogram with specific allocator failed.");
        }
        else if (!histogram_helper_.push_back(inner_hist))
        {
          ret = OB_ARRAY_OUT_OF_RANGE;
          TBSYS_LOG(WARN, "failed to push hist into hist manager.");
        }
      }
      if (OB_SUCCESS == ret)
      {
        out_index = histogram_helper_.get_array_index() - 1;
      }
      return ret;
    }

    ObTabletHistogram* ObTabletHistogramManager::alloc_hist_object()
    {
      tbsys::CThreadGuard mutex_gard(&alloc_mutex_);
      ObTabletHistogram* hist = NULL;
      char* ptr = module_arena_.alloc_aligned(sizeof(ObTabletHistogram));
      if (NULL != ptr)
      {
        hist = new (ptr) ObTabletHistogram();
      }
      return hist;
    }

    void ObTabletHistogramManager::dump(const int32_t hist_index) const
    {
      if (hist_index >=0 && hist_index < histogram_helper_.get_array_index())
      {
        histograms_[hist_index]->dump();
      }
    }

    DEFINE_SERIALIZE(ObTabletSample)
    {
      int ret = OB_SUCCESS;
      ret = serialization::encode_vi64(buf, buf_len, pos, row_count);

      if (ret == OB_SUCCESS)
        ret = range.serialize(buf, buf_len, pos);

      return ret;
    }


    DEFINE_DESERIALIZE(ObTabletSample)
    {
      int ret = OB_SUCCESS;
      ret = serialization::decode_vi64(buf, data_len, pos, &row_count);

      if (OB_SUCCESS == ret)
      {
        ret = range.deserialize(buf, data_len, pos);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "fail to deserialize range, ret=%d, buf=%p, data_len=%ld, pos=%ld",
              ret, buf, data_len, pos);
        }
      }

      return ret;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObTabletSample)
    {
      int64_t total_size = 0;

      total_size += serialization::encoded_length_vi64(row_count);
      total_size += range.get_serialize_size();

      return total_size;
    }

    DEFINE_SERIALIZE(ObTabletHistogram)
    {
      int ret = OB_SUCCESS;
      int64_t size = sample_helper_.get_array_index();
      if (ret == OB_SUCCESS)
      {
        ret = serialization::encode_vi64(buf, buf_len, pos, size);
      }

      if (ret == OB_SUCCESS)
      {
        for (int64_t i = 0; i < size; ++i)
        {
          ret = samples_[i].serialize(buf, buf_len, pos);
          if (ret != OB_SUCCESS)
            break;
        }
      }

      return ret;
    }

    DEFINE_DESERIALIZE(ObTabletHistogram)
    {
      int ret = OB_SUCCESS;
      int64_t size = 0;
      ObObj* ptr = NULL;

      if (NULL == allocator_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "allocator_ not set, deseriablize failed.");
      }
      else
      {
        if (ret == OB_SUCCESS)
        {
          ret = serialization::decode_vi64(buf, data_len, pos, &size);
          //debugb longfei 2016-03-18 11:58:50
//          TBSYS_LOG(WARN, "debug::longfei>>>decode size[%ld]",size);
          //debuge
        }
      }
      if (ret == OB_SUCCESS && size > 0)
      {
        for (int64_t i=0; i<size; ++i)
        {
          ObTabletSample sample;
          ptr = reinterpret_cast<ObObj*>(allocator_->alloc(sizeof(ObObj) * OB_MAX_ROWKEY_COLUMN_NUMBER * 2));
          sample.range.start_key_.assign(ptr, OB_MAX_ROWKEY_COLUMN_NUMBER);
          sample.range.end_key_.assign(ptr + OB_MAX_ROWKEY_COLUMN_NUMBER, OB_MAX_ROWKEY_COLUMN_NUMBER);
          ret = sample.deserialize(buf, data_len, pos);
          //debugb longfei 2016-03-18 12:06:39
          if (i == 0)
          {
            //第一次清空一下sample_helper_
            sample_helper_.clear();
          }
          //debuge
          if (OB_SUCCESS != ret)
          {
            TBSYS_LOG(WARN, "fail to deserialize ObStaticIndexSample.");
            break;
          }
          else if (OB_SUCCESS != (ret = add_sample(sample)))//shallow copy
          {
            TBSYS_LOG(WARN, "fail to add sample into histogram.");
            break;
          }
        }
      }

      return ret;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObTabletHistogram)
    {
      int64_t total_size = 0;
      int64_t size = sample_helper_.get_array_index();
      total_size += serialization::encoded_length_vi64(size);
      if (size > 0)
      {
        for (int64_t i=0; i<size; ++i)
          total_size += samples_[i].get_serialize_size();
      }
      return total_size;
    }
  }

}
