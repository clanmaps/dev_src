/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_designer.cpp
 * @brief design global index distribution, and check if local/global index construction is finished
 *
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */
#include "rootserver/ob_index_designer.h"

namespace oceanbase
{
  namespace rootserver
  {

    void ObIndexDesigner::dump_histogram() const
    {
      TBSYS_LOG(INFO, "begin to dump histogram.size = %d", (int32_t)data_meta_.get_array_index());
      if (NULL != hist_manager_ && NULL != root_server_)
      {
        for (int32_t i = 0; i < data_meta_.get_array_index(); i++)
        {
          TBSYS_LOG(INFO, "dump the %d-th tablet info", i);
          root_server_->dump_root_table(data_holder_[i].root_meta_index);
          TBSYS_LOG(INFO, "dump the %d-th tablet info end", i);

          TBSYS_LOG(INFO, "dump the %d-th histogram info", i);
          hist_manager_->dump((int32_t)data_holder_[i].hist_index);
          data_holder_[i].dump();
          TBSYS_LOG(INFO, "dump the %d-th histogram info end", i);
        }
      }
      TBSYS_LOG(INFO, "dump hist table complete");
    }

    bool compare_iter(const ObIndexDesigner::ObHistgramSampleIterator *l_itr, const ObIndexDesigner::ObHistgramSampleIterator *r_itr)
    {
      return  (l_itr->rowkey.compare(r_itr->rowkey) < 0);
    }

    bool unique_iter(const ObIndexDesigner::ObHistgramSampleIterator *l_itr, const ObIndexDesigner::ObHistgramSampleIterator *r_itr)
    {
      return (l_itr->rowkey.compare(r_itr->rowkey) == 0);
    }

    int ObIndexDesigner::get_root_meta_index(const ObTabletInfo &tablet_info, int32_t &meta_index)
    {
      int ret = OB_SUCCESS;
      if(OB_UNLIKELY(NULL == root_server_))
      {
        ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(WARN, "root server is null");
      }
      else
      {
        tbsys::CRWLock& lock = GET_ROOT_TABLE_LOCK;
        tbsys::CRLockGuard guard(lock);
        ObRootTable2* root_table = GET_ROOT_TABLE;
        if(NULL == root_table)
        {
          ret = OB_INNER_STAT_ERROR;
          TBSYS_LOG(WARN, "root table is null");
        }
        else
        {
          ret = root_table->get_root_meta_index(tablet_info, meta_index);
        }
      }
      return ret;
    }

    int ObIndexDesigner::get_root_meta(const int32_t meta_index, ObRootTable2::const_iterator &meta)
    {
      int ret = OB_SUCCESS;
      if(OB_UNLIKELY(NULL == root_server_))
      {
        ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(WARN, "root server is null");
      }
      else
      {
        tbsys::CRWLock& lock = GET_ROOT_TABLE_LOCK;
        tbsys::CRLockGuard guard(lock);
        ObRootTable2* root_table = GET_ROOT_TABLE;
        if(NULL == root_table)
        {
          ret = OB_INNER_STAT_ERROR;
          TBSYS_LOG(WARN, "root table is null");
        }
        else
        {
          ret = root_table->get_root_meta(meta_index, meta);
        }
      }
      return ret;
    }

    int ObIndexDesigner::add_hist_meta(const ObTabletInfo &tablet_info, int64_t &meta_index, const int32_t server_index)
    {
      int ret = OB_SUCCESS;
      meta_itr pos = NULL;
      ObTabletHistogramMeta hist_meta;
      int64_t  inner_idx = 0;
      //hist_meta.hist_index = meta_index;
      if(NULL == root_server_)
      {
        ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(WARN, "root server cannot be null");
      }
      else if(NULL == (pos = get_tablet_pos(tablet_info, inner_idx)))
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "get tablet pos failed");
      }
      else if(pos == end())
      {
        ///new tablet info,should add in meta index
        hist_meta.report_cs_info[0].server_info_index = server_index;
        hist_meta.report_cs_info[0].report_timestamp = tbsys::CTimeUtil::getTime();

        if(OB_SUCCESS != (ret = get_root_meta_index(tablet_info, hist_meta.root_meta_index)))
        {
          TBSYS_LOG(WARN, "can not get hist_meta.root_meta_ from root table.ret = %d", ret);
        }
        else if(OB_SUCCESS != (ret = add_meta(hist_meta)))
        {
          TBSYS_LOG(WARN, "failed to add hist meta,ret = %d", ret);
        }
        meta_index = data_meta_.get_array_index() - 1;
      }
      else
      {
        int32_t server_index_pos = find_available_pos(pos, server_index);
        if (server_index_pos >= 0 && server_index_pos < common::OB_SAFE_COPY_COUNT)
        {
          pos->report_cs_info[server_index_pos].server_info_index = server_index;
          pos->report_cs_info[server_index_pos].report_timestamp = tbsys::CTimeUtil::getTime();
        }
        meta_index = inner_idx;
      }

      return ret;
    }

    int ObIndexDesigner::add_meta(const ObTabletHistogramMeta &meta)
    {
      int ret = OB_SUCCESS;
      if (data_meta_.get_array_index() >= ObTabletHistogramManager::MAX_TABLET_COUNT_PER_TABLE)
      {
        TBSYS_LOG(WARN, "too many histogram, max is %ld now is %ld", ObTabletHistogramManager::MAX_TABLET_COUNT_PER_TABLE, data_meta_.get_array_index());
        ret = OB_ARRAY_OUT_OF_RANGE;
      }
      else if (!data_meta_.push_back(meta))
      {
        TBSYS_LOG(WARN, "failed to push hist meta into meta_table_");
        ret = OB_ARRAY_OUT_OF_RANGE;
      }
      return ret;
    }

    int32_t ObIndexDesigner::find_available_pos(const_meta_itr it, const int32_t server_index) const
    {
      int find_index = OB_INVALID_INDEX;
      if(NULL == it)
      {
        TBSYS_LOG(WARN, "invalid paramater");
      }
      else
      {
        for (int32_t i = 0; i < common::OB_SAFE_COPY_COUNT; i++)
        {
          if (it->report_cs_info[i].server_info_index == server_index)
          {
            find_index = i;
            break;
          }
        }
        if (OB_INVALID_INDEX == find_index)
        {
          for (int32_t i = 0; i < common::OB_SAFE_COPY_COUNT; i++)
          {
            if (it->report_cs_info[i].server_info_index == server_index)
            {
              find_index = i;
              break;
            }
          }
        }
        if (OB_INVALID_INDEX == find_index)
        {
          int64_t old_timestamp = ((((uint64_t)1) << 63) - 1);
          for (int32_t i = 0; i < common::OB_SAFE_COPY_COUNT; i++)
          {
            if (it->report_cs_info[i].report_timestamp < old_timestamp)
            {
              old_timestamp = it->report_cs_info[i].report_timestamp;
              find_index = i;
            }
          }
        }
      }
      return find_index;
    }

    int ObIndexDesigner::get_rt_tablet_info(const int32_t meta_index, const ObTabletInfo *&tablet_info) const
    {
      int ret = OB_SUCCESS;
      if(NULL == root_server_)
      {
        ret = OB_INNER_STAT_ERROR;
      }
      else
      {
        tablet_info = NULL;
        ObRootTable2* root_table = GET_ROOT_TABLE;
        if (NULL == root_table)
        {
         ret = OB_INNER_STAT_ERROR;
         TBSYS_LOG(WARN, "root_table_ is null.");
        }
        else if (NULL == (tablet_info = root_table->get_tablet_info(meta_index)))
        {
         ret = OB_INVALID_DATA;
        }
      }
      return ret;
    }

    ObIndexDesigner::meta_itr ObIndexDesigner::get_tablet_pos(const ObTabletInfo &info, int64_t &inner_pos)
    {
      meta_itr pos = NULL;
      bool val = inner_pos == -1 ? false : true;
      if(NULL == root_server_)
      {
        //ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(ERROR, "root server should not be null");
      }
      else
      {
        for(pos = begin(); pos != end(); pos++)
        {
          const ObTabletInfo *compare = NULL;
          if(val)inner_pos++;
          if(OB_SUCCESS != (get_rt_tablet_info(pos->root_meta_index, compare)))//root_meta_index:在root table中的索引
          {
            TBSYS_LOG(WARN, "failed to get tablet info for compare");
          }
          else if(NULL == compare)
          {
            pos = NULL;
            break;
          }
          else if(compare->equal(info))
          {
            break;
          }

        }
      }
      return pos;
    }

    int ObIndexDesigner::sort_all_sample()
    {
      int ret = OB_SUCCESS;
      if(NULL == hist_manager_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "hist manager should not be NULL");
      }
      else
      {
        ObHistgramSampleIterator l_wrapper;
        ObHistgramSampleIterator r_wrapper;
        ObTabletHistogram* hist = NULL;
        ObTabletSample* sample =NULL;
        for(const_meta_itr it = begin(); it != end() && OB_SUCCESS == ret; it++)
        {
          l_wrapper.iter = it;
          r_wrapper.iter = it;
          if (OB_SUCCESS != (ret = hist_manager_->get_histogram((int32_t)it->hist_index, hist)))
          {
            TBSYS_LOG(WARN, "get histogram failed %d", ret);
          }
          else
          {
            for(int64_t i = 0; i < hist->get_sample_count() && OB_SUCCESS == ret; i++)
            {
              if (OB_SUCCESS != (ret = hist->get_sample(i, sample)))
              {
                TBSYS_LOG(WARN, "get sample from histogram failed. sample index = %ld", i);
              }
              else if (NULL == sample)
              {
                ret = OB_ERROR;
                TBSYS_LOG(WARN, "p_sample is NULL");
              }
              else
              {
                l_wrapper.rowkey = sample->get_range().start_key_;
                r_wrapper.rowkey = sample->get_range().end_key_;
                int64_t count = sri_array_.count();
                if (OB_SUCCESS != (ret = sri_array_.push_back(l_wrapper)) || OB_SUCCESS != (ret = sri_array_.push_back(r_wrapper)))
                {
                  ret = OB_ARRAY_OUT_OF_RANGE;
                  TBSYS_LOG(WARN, "samples overflow");
                }
                else
                {
                  ObHistgramSampleIterator &l_wrapper_2 = sri_array_.at(count);
                  ObHistgramSampleIterator &r_wrapper_2 = sri_array_.at(count + 1);
                  common::ObSortedVector<ObHistgramSampleIterator *>::iterator insert_pos = NULL;
                  if (OB_SUCCESS != (ret = sorted_hsi_list_.insert_unique(&l_wrapper_2, insert_pos, compare_iter, unique_iter)))
                  {
                    TBSYS_LOG(DEBUG, "can not insert left srw into sorted_srws_");
                  }
                  if (OB_CONFLICT_VALUE == ret)
                  {
                    ret = OB_SUCCESS;
                    TBSYS_LOG(DEBUG, "duplicate sample range.");
                  }
                  if (OB_SUCCESS == ret && OB_SUCCESS != (ret = sorted_hsi_list_.insert_unique(&r_wrapper_2, insert_pos, compare_iter, unique_iter)))
                  {
                    TBSYS_LOG(DEBUG, "can not insert right srw into sorted_srws_");
                  }
                  if (OB_CONFLICT_VALUE == ret)
                  {
                    ret = OB_SUCCESS;
                    TBSYS_LOG(DEBUG, "duplicate sample range.");
                  }
                }
              }
            }
          }
        }

      }
      return ret;
    }

    int ObIndexDesigner::get_next_global_tablet(const int64_t sample_num, ObTabletInfo &tablet_info, int32_t *server_index, const int32_t copy_count)
    {
      int ret = OB_SUCCESS;
      ObHistgramSampleIterator *sample = NULL;
      int start = cur_sri_index_;
      if(sample_num <=0 || sample_num > ObTabletHistogram::MAX_SAMPLE_BUCKET || NULL == server_index || sorted_hsi_list_.size() <= cur_sri_index_ || 0 > cur_sri_index_)
      {
        ret = OB_INNER_STAT_ERROR;
        TBSYS_LOG(WARN, "wrong argument,sample num = %d, server_index = %p, sort_list size = %d cur_sri_index = %d", (int32_t)sample_num, server_index, sorted_hsi_list_.size(), cur_sri_index_);
      }
      else
      {
        int64_t gap = static_cast<int64_t>(sorted_hsi_list_.size())/sample_num;
        gap = gap < 0 ? 1 : gap;
        tablet_info.range_.border_flag_.unset_inclusive_start();

        if(0 == cur_sri_index_)
        {
          tablet_info.range_.start_key_ = common::ObRowkey::MIN_ROWKEY;
        }
        else if(NULL != (sample = sorted_hsi_list_.at(cur_sri_index_)))
        {
          tablet_info.range_.start_key_ = sample->rowkey;
        }
        else
        {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "failed to get one sample");
        }

        if(OB_SUCCESS == ret)
        {
          cur_sri_index_ += static_cast<int32_t>(gap);
          if(cur_sri_index_ < sorted_hsi_list_.size() - 1)
          {
            if(NULL != (sample = sorted_hsi_list_.at(cur_sri_index_)))
            {
              tablet_info.range_.end_key_ = sample->rowkey;
              tablet_info.range_.border_flag_.set_inclusive_end();
            }
            else
            {
              ret = OB_ERROR;
              TBSYS_LOG(WARN, "failed to get one sample");
            }
          }
          else
          {
            tablet_info.range_.end_key_ = common::ObRowkey::MAX_ROWKEY;
            tablet_info.range_.border_flag_.unset_inclusive_end();
          }
        }
      }

      if(OB_SUCCESS == ret)
      {
        tablet_info.range_.table_id_ = get_index_tid();
        int32_t end = cur_sri_index_ < sorted_hsi_list_.size() ? cur_sri_index_ : sorted_hsi_list_.size();
        ret = allocate_chunkserver(server_index, copy_count, start, end);
      }
      return ret;
    }

    int ObIndexDesigner::allocate_chunkserver(int32_t *server_index, const int32_t copy_count, const int32_t begin_index, const int32_t end_index)
    {
      int ret = OB_SUCCESS;
      ObHistgramSampleIterator *sample = NULL;
      if (NULL == server_index || begin_index > end_index || begin_index < 0 || end_index > sorted_hsi_list_.size())
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "wrong parameters, begin_index=%d, end_index=%d, server_index=%p.", begin_index, end_index, server_index);
      }
      else
      {
        int idx = 0;
        for(int i = begin_index; i < end_index && OB_SUCCESS == ret ; i++)
        {
          if(NULL == (sample = sorted_hsi_list_.at(i)) || NULL == sample->iter)
          {
            ret = OB_INNER_STAT_ERROR;
            TBSYS_LOG(WARN, "failed to get a histogram sample");
          }
          else
          {
            int server_idx = -1;
            for (int32_t index = 0; index < common::OB_SAFE_COPY_COUNT && idx < copy_count; index++)
            {
              if (OB_INVALID_INDEX != (server_idx = sorted_hsi_list_.at(i)->iter->report_cs_info[index].server_info_index))
              {
                bool exist = false;
                for (int32_t found_index = 0; found_index < idx; found_index++)
                {
                  if (server_index[found_index] == server_idx)
                  {
                    exist = true;
                    break;
                  }
                }
                if (!exist)
                {
                  server_index[idx++] = server_idx;
                }
              }
            }//end for
          }
        }  //end for

        for(int i = begin_index; i < end_index && idx < copy_count&& OB_SUCCESS == ret; i++)
        {
          if(NULL == root_server_ || NULL == sorted_hsi_list_.at(i) || NULL == sorted_hsi_list_.at(i)->iter)
          {
            ret = OB_INNER_STAT_ERROR;
            TBSYS_LOG(WARN, "null pointer of rootserver");
          }
          else
          {
            rootserver::ObRootTable2::const_iterator meta_data = NULL;
            if(OB_SUCCESS != (ret = get_root_meta(sorted_hsi_list_.at(i)->iter->root_meta_index, meta_data)))
            {
              TBSYS_LOG(WARN, "failed to get meta data in root table, ret = %d", ret);
            }
            else if(NULL == meta_data)
            {
              TBSYS_LOG(WARN, "meta data is null");
              ret = OB_INNER_STAT_ERROR;
            }
            else
            {
              int32_t server_idx = OB_INVALID_INDEX;
              for (int32_t index = 0; index < common::OB_SAFE_COPY_COUNT && idx < copy_count; index++)
              {
                if (OB_INVALID_INDEX != (server_idx = meta_data->server_info_indexes_[index]))
                {
                  bool exist = false;
                  for (int32_t found_index = 0; found_index < idx; found_index++)
                  {
                    if (server_index[found_index] == server_idx)
                    {
                      exist = true;
                      break;
                    }
                  }
                  if (!exist)
                  {
                    server_index[idx++] = server_idx;
                  }
                }
              }//end for
            }
          }
        }
      }
      return ret;
    }

    int ObIndexDesigner::set_meta_index(int64_t meta_index, int64_t hist_index)
    {
      int ret = OB_SUCCESS;
      //int index = static_cast<int>(meta_index);
      if(meta_index < 0 || meta_index >= data_meta_.get_array_index())
      {
        TBSYS_LOG(WARN,"can't push into array.input_index[%ld],array index[%ld]",meta_index,data_meta_.get_array_index());
        ret = OB_ERROR;
      }
      else
        data_meta_.at(meta_index)->hist_index = hist_index;
      return  ret;
    }

    int ObIndexDesigner::check_report_info(const ObTabletInfo *compare_tablet, bool &is_finished) const
    {
      is_finished = false;
      int ret = OB_SUCCESS;
      if (NULL == compare_tablet)
      {
        ret = OB_INVALID_DATA;
        TBSYS_LOG(WARN, "compared tablet info is null.");
      }
      else if (NULL == root_server_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(WARN, "root_server_ is null.");
      }
      else if (data_meta_.get_array_index() > 0)
      {
        //first set ret = OB_NO_MONITOR_DATA, otherwise, if meta_table_'s size() == 0, return OB_SUCCESS anyway
        ///check pass even if cs has not reported!!!
        for (int64_t i = 0; i < data_meta_.get_array_index(); i++)
        {
          const ObTabletInfo * tablet_info = NULL;
          if (OB_SUCCESS != (ret = root_server_->get_rt_tablet_info(data_holder_[i].root_meta_index, tablet_info)))
          {
            TBSYS_LOG(WARN, "get_rt_tablet_info failed. ret=%d", ret);
            break;
          }
          else if (NULL == tablet_info)
          {
            ret = OB_INVALID_DATA;
            TBSYS_LOG(WARN, "tablet info is null. ret=%d", ret);
            break;
          }
          else if (tablet_info->equal(*compare_tablet))
          {
            is_finished = true;
            break;
          }
        }
      }
      return ret;

    }

    int ObIndexDesigner::check_local_index_build_done(const uint64_t index_tid, bool &is_finished, tbsys::CThreadMutex &mutex)
    {
      int ret = OB_SUCCESS;
      if(index_tid_ != index_tid || root_server_ == NULL)
      {
        TBSYS_LOG(WARN, "index id [%ld] is not equal inner argument [%ld]", index_tid, index_tid_);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        common::ObRowkey min_rowkey;
        min_rowkey.set_min_row();
        common::ObRowkey max_rowkey;
        max_rowkey.set_max_row();
        common::ObNewRange start_range;
        start_range.table_id_ = table_id_;
        start_range.end_key_ = min_rowkey;
        common::ObNewRange end_range;
        end_range.table_id_ = table_id_;
        end_range.end_key_ = max_rowkey;
        tbsys::CThreadMutex& rt_build_lock = GET_ROOT_TABLE_BUILD_LOCK;
        tbsys::CRWLock& rt_rw_lock = GET_ROOT_TABLE_LOCK;
        tbsys::CThreadGuard mutex_gard(&rt_build_lock);
        tbsys::CRLockGuard guard(rt_rw_lock);
        tbsys::CThreadGuard hist_mutex_gard(&mutex);
        ObRootTable2* root_table = GET_ROOT_TABLE;

        ObRootTable2::const_iterator begin_pos = root_table->find_pos_by_range(start_range);
        ObRootTable2::const_iterator end_pos = root_table->find_pos_by_range(end_range);

        if (NULL == begin_pos || NULL == end_pos)
        {
          ret = OB_NOT_INIT;
          TBSYS_LOG(WARN, "find_pos_by_range failed, begin_pos or end_pos is NULL. ret=%d", ret);
        }
        else if (begin_pos > end_pos)
        {
          ret = OB_ERR_UNEXPECTED;
          TBSYS_LOG(WARN, "find_pos_by_range failed, out of range. begin():[%p], end():[%p], begin:[%p], end:[%p], ret=%d",
                    root_table->begin(), root_table->end(), begin_pos, end_pos, ret);
        }
        for (ObRootTable2::const_iterator it = begin_pos; it <= end_pos && OB_LIKELY(OB_SUCCESS == ret); it++)
        {
          const ObTabletInfo* compare_tablet = NULL;
          compare_tablet = root_table->get_tablet_info(it);
          if (NULL == compare_tablet)
          {
            ret = OB_INVALID_DATA;
            TBSYS_LOG(WARN, "compared tablet info is null. ret=%d", ret);
          }
          else if (OB_SUCCESS != (ret = check_report_info(compare_tablet, is_finished)))
          {
            TBSYS_LOG(INFO, "build local static index has not finished. ret=%d", ret);
          }
        }
      }
      return ret;
    }

    int ObIndexDesigner::check_global_index_build_done(const uint64_t index_tid, bool &is_finished) const
    {
      int ret = OB_SUCCESS;
      if(index_tid_ != index_tid || root_server_ == NULL)
      {
        TBSYS_LOG(WARN, "index id [%ld] is not equal inner argument [%ld]", index_tid, index_tid_);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        common::ObRowkey min_rowkey;
        min_rowkey.set_min_row();
        common::ObRowkey max_rowkey;
        max_rowkey.set_max_row();
        common::ObNewRange start_range;
        start_range.table_id_ = index_tid;
        start_range.end_key_ = min_rowkey;
        common::ObNewRange end_range;
        end_range.table_id_ = index_tid;
        end_range.end_key_ = max_rowkey;
        tbsys::CThreadMutex& rt_build_lock = GET_ROOT_TABLE_BUILD_LOCK;
        tbsys::CRWLock& rt_rw_lock = GET_ROOT_TABLE_LOCK;
        tbsys::CThreadGuard mutex_gard(&rt_build_lock);
        tbsys::CRLockGuard guard(rt_rw_lock);
        ObRootTable2* root_table = GET_ROOT_TABLE;
        ObRootTable2::const_iterator begin_pos = root_table->find_pos_by_range(start_range);
        ObRootTable2::const_iterator end_pos = root_table->find_pos_by_range(end_range);
        if (NULL == begin_pos || NULL == end_pos)
        {
          ret = OB_NOT_INIT;
          TBSYS_LOG(WARN, "find_pos_by_range failed, begin_pos or end_pos is NULL. ret=%d", ret);
        }
        else if (begin_pos > end_pos)
        {
          ret = OB_ERR_UNEXPECTED;
          TBSYS_LOG(WARN, "find_pos_by_range failed, out of range. begin():[%p], end():[%p], begin:[%p], end:[%p], ret=%d",
                    root_table->begin(), root_table->end(), begin_pos, end_pos, ret);
        }
        for (ObRootTable2::const_iterator it = begin_pos; it <= end_pos; it++)
        {
          is_finished = false;
          for (int32_t idx = 0; idx < OB_SAFE_COPY_COUNT; idx++)
          {
            if (OB_INVALID_INDEX != it->server_info_indexes_[idx] && OB_INVALID_VERSION != it->tablet_version_[idx])
            {
              is_finished = true;
              break;
            }
          }
          if (!is_finished)
          {
            TBSYS_LOG(INFO, "create global static index has not finished");
            break;
          }
        }
      }
      return ret;
    }

    int ObIndexDesigner::balance_index(const uint64_t index_tid)
    {
      int ret = OB_SUCCESS;
      if(index_tid_ != index_tid || root_server_ == NULL)
      {
        TBSYS_LOG(WARN, "index id [%ld] is not equal inner argument [%ld]", index_tid, index_tid_);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        common::ObRowkey min_rowkey;
        min_rowkey.set_min_row();
        common::ObRowkey max_rowkey;
        max_rowkey.set_max_row();
        common::ObNewRange start_range;
        start_range.table_id_ = index_tid;
        start_range.end_key_ = min_rowkey;
        common::ObNewRange end_range;
        end_range.table_id_ = index_tid;
        end_range.end_key_ = max_rowkey;
        tbsys::CThreadMutex& rt_build_lock = GET_ROOT_TABLE_BUILD_LOCK;
        tbsys::CRWLock& rt_rw_lock = GET_ROOT_TABLE_LOCK;
        tbsys::CThreadGuard mutex_gard(&rt_build_lock);
        tbsys::CRLockGuard guard(rt_rw_lock);
        ObRootTable2* root_table = GET_ROOT_TABLE;
        ObRootTable2::const_iterator begin_pos = root_table->find_pos_by_range(start_range);
        ObRootTable2::const_iterator end_pos = root_table->find_pos_by_range(end_range);

        if (NULL == begin_pos || NULL == end_pos)
        {
          ret = OB_NOT_INIT;
          TBSYS_LOG(WARN, "find_pos_by_range failed, begin_pos or end_pos is NULL. ret=%d", ret);
        }
        else if (root_table->end() == begin_pos || root_table->end() == end_pos || begin_pos > end_pos)
        {
          ret = OB_ARRAY_OUT_OF_RANGE;
          TBSYS_LOG(WARN, "find_pos_by_range failed, out of range. begin():[%p], end():[%p], begin:[%p], end:[%p], ret=%d",
                    root_table->begin(), root_table->end(), begin_pos, end_pos, ret);
        }
        for (ObRootTable2::const_iterator it = begin_pos; it <= end_pos && OB_LIKELY(OB_SUCCESS == ret); it++)
        {
          for (int32_t idx = 0; idx < OB_SAFE_COPY_COUNT; idx++)
          {
            if (OB_INVALID_INDEX != it->server_info_indexes_[idx] && OB_INVALID_VERSION == it->tablet_version_[idx])
            {
              //modify to OB_INVALID_INDEX, trigger balance
              atomic_exchange((uint32_t*) &(it->server_info_indexes_[idx]), OB_INVALID_INDEX);
            }
          }
        }
      }
      return ret;
    }

    int ObIndexDesigner::fill_tablet_info_list_by_server_index(ObTabletInfoList *tablet_info_list[], const int32_t list_size, const ObTabletInfo &tablet_info, int32_t *server_index, const int32_t copy_count)
    {
      int ret = OB_SUCCESS;
      if (NULL == tablet_info_list || NULL == server_index)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "ObTabletInfoList* tablet_info_list[%p] or server_index[%p] is null.", tablet_info_list, server_index);
      }
      for (int32_t i = 0; i < copy_count && OB_SUCCESS == ret; i++)
      {
        int32_t idx = server_index[i];
        if (OB_INVALID_INDEX != idx)
        {
          if (idx < 0 || idx >= list_size)
          {
            ret = OB_ARRAY_OUT_OF_RANGE;
            TBSYS_LOG(WARN, "list_size:[%d], server_info_index:[%d]. ret=%d", list_size, idx, ret);
          }
          else if (NULL != tablet_info_list[idx])
          {
            if (OB_SUCCESS != (ret = tablet_info_list[idx]->add_tablet(tablet_info)))
            {
              TBSYS_LOG(WARN, "fail add tablet report info. ret=%d", ret);
            }
          }
          else if (NULL != (tablet_info_list[idx] = OB_NEW(ObTabletInfoList,  ObModIds::OB_BUFFER)))
          {
            if (OB_SUCCESS != (ret = tablet_info_list[idx]->add_tablet(tablet_info)))
            {
              TBSYS_LOG(WARN, "fail add tablet report info. ret=%d", ret);
            }
          }
          else
          {
            ret = OB_ALLOCATE_MEMORY_FAILED;
            TBSYS_LOG(WARN, "new ObTabletInfoList failed. ret=%d", ret);
          }
        }
      }
      return ret;
    }

    int ObIndexDesigner::design_global_index(const int64_t sample_num, tbsys::CThreadMutex &mutex)
    {
      int ret = OB_SUCCESS;
      int32_t server_count = 0;
      tbsys::CThreadGuard hist_mutex_gard(&mutex);
      //mod　longfei 2016-06-21 [Bug 145] 22:01:13
      ObTabletInfoList **p_info_list;
      //      ObTabletInfoList* p_info_list[server_count];
      //mod e
      if(NULL == root_server_)
      {
        TBSYS_LOG(WARN, "root server cannot be null");
        ret = OB_INNER_STAT_ERROR;
      }
      else
      {
        server_count = static_cast<int32_t>(root_server_->get_server_manager().size());
        TBSYS_LOG(INFO, "server count is %d", server_count);
        //add longfei 2016-06-21 [Bug 145] 22:14:57
        p_info_list = static_cast<ObTabletInfoList**>(ob_malloc(sizeof(ObTabletInfoList*) * server_count, ObModIds::OB_BUFFER));
        //add e
        for (int32_t i = 0; i< server_count; i++)
        {
          p_info_list[i] = NULL;
        }
      }
      if(OB_SUCCESS == ret)
      {
        {
          TBSYS_LOG(INFO, "Dragon: design global index"); //add dragon 2017-3-20
          while(has_next() && OB_SUCCESS == ret)
          {
            ObTabletInfo tablet_info;
            int32_t copy_count = OB_SAFE_COPY_COUNT;
            int32_t server_index[copy_count];
            for (int32_t i = 0; i < copy_count; i++)
            {
              server_index[i] = OB_INVALID_INDEX;
            }
            TBSYS_LOG(DEBUG,"sample_num is [%ld], copy_count = [%d]", sample_num, copy_count);
            // tablet_info --> 切分后的全局tablet     server_index --> 各副本所在的cs
            if (OB_SUCCESS == (ret = get_next_global_tablet(sample_num, tablet_info, server_index, copy_count)))
            {
              if (OB_SUCCESS != (ret = fill_tablet_info_list_by_server_index(
                                   p_info_list,
                                   server_count,
                                   tablet_info,
                                   server_index,
                                   copy_count)))
              {
                TBSYS_LOG(WARN, "fill tablet info list error. ret=%d", ret);
              }
              //add dragon [] 2017-3-29 b
              //write this tablet info to root table
              if (OB_SUCCESS == ret)
              {
                TBSYS_LOG(INFO, "will write global index range into rt for query, index table id = [%lu]", index_tid_);
                //mod dragon [] 2017-3-29 b
                if (OB_SUCCESS != (ret = root_server_->write_tablet_info_list_to_rt_v2(
                                     p_info_list, server_index, server_count)))
                  //if (OB_SUCCESS != (ret = root_server_->write_tablet_info_list_to_rt(p_info_list, server_count)))
                  //mod dragon e
                {
                  TBSYS_LOG(WARN, "write tablet info list to rt error. ret=%d", ret);
                }
                else
                {
                  TBSYS_LOG(INFO, "write global index range into rt succ, index table id = [%lu]", index_tid_);
                }
              }

              //为下个全局tablet做准备
              for(int32_t i = 0; i < server_count; i++)
              {
                if(p_info_list[i] != NULL)
                  p_info_list[i]->tablet_list.clear();
              }
              //add dragon e
            }
            else
            {
              TBSYS_LOG(WARN, "fail get tablet info. ret=%d", ret);
            }
          }//end while
        }
        //del dragon [] b
        /*
            if (OB_SUCCESS == ret)
            {
              TBSYS_LOG(INFO, "will write global index range into rt for query, index table id = [%lu]", index_tid_);
              //mod dragon [] 2017-3-29 b
              if (OB_SUCCESS != (ret = root_server_->write_tablet_info_list_to_rt_v2(
                                   p_info_list, server_index, server_count)))
              //if (OB_SUCCESS != (ret = root_server_->write_tablet_info_list_to_rt(p_info_list, server_count)))
              //mod dragon e
              {
                TBSYS_LOG(WARN, "write tablet info list to rt error. ret=%d", ret);
              }
              else
              {
                TBSYS_LOG(INFO, "write global index range into rt succ, index table id = [%lu]", index_tid_);
              }
            }
            */
        //del e

        for (int32_t i = 0; i< server_count; i++)
        {
          OB_DELETE(ObTabletInfoList, ObModIds::OB_BUFFER, p_info_list[i]);
        }
        ob_free(p_info_list, ObModIds::OB_BUFFER); //add longfei 2016-06-21 [Bug 145] 22:37:57
      }
      return ret;
    }

  }  //end of rootserver
}  //end of oceanbase
