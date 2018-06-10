/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_designer.h
 * @brief design global index distribution, and check if local/global index construction is finished
 *
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

#ifndef OB_INDEX_DESIGNER_H
#define OB_INDEX_DESIGNER_H

#include <tbsys.h>
#include "rootserver/ob_root_meta2.h"
#include "rootserver/ob_root_table2.h"
#include "rootserver/ob_root_server2.h"
#include "common/ob_tablet_histogram.h"
#include "common/ob_tablet_info.h"

#define GET_ROOT_TABLE root_server_->get_root_table()
#define GET_ROOT_TABLE_LOCK root_server_->get_root_table_lock()
#define GET_ROOT_TABLE_BUILD_LOCK root_server_->get_root_table_build_lock()
namespace oceanbase
{
  namespace rootserver
  {
    /**
     * @brief The ObIndexDesigner class
     * ObIndexDesigner is designed for
     * index tablet report info management
     * design global index distribution
     * check if global/local index construction is finished
     */
    class ObIndexDesigner
    {
      public:
        typedef ObTabletHistogramMeta* meta_itr;
        typedef const ObTabletHistogramMeta* const_meta_itr;

        struct ObHistgramSampleIterator
        {
          common::ObRowkey rowkey;
          const_meta_itr iter;
        };

        /**
         * @brief constructor
         */
        explicit ObIndexDesigner(rootserver::ObRootServer2 *rs, ObTabletHistogramManager *hist_manager)
          :hist_manager_(hist_manager), root_server_(rs),
           table_id_(OB_INVALID_ID), index_tid_(OB_INVALID_ID),
           cur_sri_index_(0)
        {
          data_meta_.init(ObTabletHistogramManager::MAX_TABLET_COUNT_PER_TABLE, data_holder_);
        }

        /**
         * @brief destructor
         */
        ~ObIndexDesigner()
        {
          root_server_ = NULL;
          if(NULL != hist_manager_)
          {
            OB_DELETE(ObTabletHistogramManager, ObModIds::OB_STATIC_INDEX, hist_manager_);
          }
          hist_manager_ = NULL;
          reset();
        }

        /**
         * @brief reset env of index designer
         *
         */
        void reset()
        {
          data_meta_.clear();
          sri_array_.clear();
          sorted_hsi_list_.clear();
          table_id_ = OB_INVALID_ID;
          index_tid_ = OB_INVALID_ID;
          cur_sri_index_ = 0;
        }

        /**
         * @brief set index id and original table id .
         * @param tid original table id of index
         * @param index_tid index tid to be constructed
         * @return err code if success or not.
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
         * @brief get original tableid
         * @return original table id.
         */
        uint64_t get_table_id() const
        {
          return table_id_;
        }

        /**
         * @brief get index tid
         * @return index id.
         */
        uint64_t get_index_tid() const
        {
          return index_tid_;
        }

        /**
         * @brief checkout if sorted_hsi_list has next item
         * @return boolean if has next.
         */
        bool has_next()
        {
          return (sorted_hsi_list_.size() > cur_sri_index_) && (0 <= cur_sri_index_);
        }

        /**
         * @brief get histogram manager
         * @return ObTabletHistogramManager.
         */
        ObTabletHistogramManager *get_hist_manager(){return hist_manager_;}

        /**
         * @brief dump all histogram info
         */
        void        dump_histogram()const;

        /**
         * @brief add histogram meta info into data holder array.
         * @param tablet info reported by cs.
         * @param meta_index  output param, return meta index in data hodler.
         * @param server_index cs_index report info
         * @return err code if success or not.
         */
        int         add_hist_meta(const ObTabletInfo &tablet_info,  int64_t &meta_index, const int32_t server_index);

        /**
         * @brief find tablet pos in root_table, info must be found in both data_holder_ and roottable.
         * @param info tablet info we wanted to find.
         * @param inner_pos output param, pos in root_table.
         * @return err code if success or not.
         */
        meta_itr    get_tablet_pos(const ObTabletInfo& info, int64_t &inner_pos);

        /**
         * @brief get table info by meta index in the root table.
         * @param meta_index index of tablet.
         * @param tablet_info output param ,link of ObTabletInfo.
         * @return err code if success or not.
         */
        int         get_rt_tablet_info(const int32_t meta_index, const ObTabletInfo *&tablet_info) const;

        /**
         * @brief get tablet_info pos in rootable.
         * @param tablet_info we want to find.
         * @param meta_index output param, pos index in the root table.
         * @return err code if success or not.
         */
        int         get_root_meta_index(const common::ObTabletInfo &tablet_info, int32_t &meta_index);

        /**
         * @brief get meta by index in the root table.
         * @param meta_index index of meta.
         * @param meta output param, root meta.
         * @return err code if success or not.
         */
        int         get_root_meta(const int32_t meta_index, ObRootTable2::const_iterator &meta);

        /**
         * @brief find an available pos in meta array to add in.
         * @param it meta link to add in.
         * @param server_index cs index that report info.
         * @return available pos, if err occured, return OB_INVALID_INDEX.
         */
        int32_t     find_available_pos(const_meta_itr it, const int32_t server_index) const;

        /**
         * @brief sort all histogram info after collect all sample.
         * @return err code if success or not.
         */
        int         sort_all_sample();

        /**
         * @brief get next global tablet for index design.
         * @param sample_num number of sample list.
         * @param tablet_info output param, global index tablet info.
         * @param server_index output param, chunkserver index who will get this tablet info
         * @param copy_count tablet copy count
         * @return err code if success or not.
         */
        int         get_next_global_tablet(const int64_t sample_num, ObTabletInfo &tablet_info, int32_t *server_index, const int32_t copy_count);

        /**
         * @brief allocate chunkserver for tablet copy.
         * @param server_index output param cs allocated.
         * @param copy_count index tablet copy count.
         * @param begin_index start pos of global tablet
         * @param end_index end pos of global index tablet
         * @return err code if success or not.
         */
        int         allocate_chunkserver(int32_t *server_index, const int32_t copy_count, const int32_t begin_index, const int32_t end_index);

        /**
         * @brief set hist index by meta_index.
         * @param meta_index pos to set hist index.
         * @param hist_index value to set.
         * @return err code if success or not.
         */
        int         set_meta_index(int64_t meta_index, int64_t hist_index);

      public:
        /**
         * @brief begin function for data_holder iterator
         * @return iterator return .
         */
        inline meta_itr begin()
        {
          return &(data_holder_[0]);
        }

        /**
         * @brief end function for data_holder iterator
         * @return iterator return .
         */
        inline meta_itr end()
        {
          return begin() + data_meta_.get_array_index();
        }

        /**
         * @brief begin function for data_holder iterator with const
         * @return iterator return .
         */
        inline const_meta_itr begin()const
        {
          return &(data_holder_[0]);
        }

        /**
         * @brief end function for data_holder iterator with const
         * @return iterator return .
         */
        inline const_meta_itr end() const
        {
          return begin() + data_meta_.get_array_index();
        }

      public:
        /**
         * @brief check if a original table tablet's report info has recieved by designer.
         * @param compare_tablet tablet need check, it is a link.
         * @param is_finished output param, if this tablet has recieved.
         * @return err code if success or not.
         */
        int check_report_info(const ObTabletInfo *compare_tablet, bool &is_finished) const;

        /**
         * @brief check local index stage mission has been finished.
         * @param index_tid  index id to be checked
         * @param output param is this index local stage finished.
         * @param histogran manager mutex lock
         * @return err code if success or not.
         */
        int check_local_index_build_done(const uint64_t index_tid, bool &is_finished, tbsys::CThreadMutex &mutex);

        /**
         * @brief use a global index tablet info , server index, and copy count to add in tablet info list.
         * @param tablet_info_list a link to info list array
         * @param list_size array size.
         * @param tablet_info tablet to add in, get by get_next_global_tablet()
         * @param server_index cs index which will own tablet
         * @param copy_count index tablet copy count
         * @return err code if success or not.
         */
        int fill_tablet_info_list_by_server_index(ObTabletInfoList *tablet_info_list[], const int32_t list_size,
                                                  const ObTabletInfo &tablet_info, int32_t *server_index, const int32_t copy_count);

        /**
         * @brief design global index .
         * @param sample_num sample number
         * @param mutex histogram manager's lock
         * @return if success or not.
         */
        int design_global_index(const int64_t sample_num, tbsys::CThreadMutex &mutex);

        /**
         * @brief check global index stage mission has been finished.
         * @param index_tid  index id to be checked
         * @param is_finished output param is this index local stage finished.
         * @return err code if success or not.
         */
        int check_global_index_build_done(const uint64_t index_tid, bool &is_finished) const;

        /**
         * @brief balance one index tablet copy to safe copy count.
         * @param index_tid index id to be balanced.
         * @return err code if success or not.
         */
        int balance_index(const uint64_t index_tid);

      private:
        /**
         * @brief push meta into data_holder_.
         * @param meta meta to be pushed.
         * @return err code if success or not.
         */
        int add_meta(const ObTabletHistogramMeta &meta);

      private:
        ObTabletHistogramMeta data_holder_[ObTabletHistogramManager::MAX_TABLET_COUNT_PER_TABLE];                       ///< array stored histogram meta
        common::ObArrayHelper<ObTabletHistogramMeta> data_meta_;                                                        ///< array helper of data_holder
        common::ObSEArray<ObHistgramSampleIterator, ObTabletHistogramManager::MAX_TABLET_COUNT_PER_TABLE> sri_array_;   ///< simple array , hold the iterator link of sample
        common::ObSortedVector<ObHistgramSampleIterator *> sorted_hsi_list_;                                            ///< sorted vectir, sort and hold iterator link
        ObTabletHistogramManager *hist_manager_;                                                                        ///< a link field of hist manager
        rootserver::ObRootServer2* root_server_;                                                                        ///< a link field of root server
        uint64_t table_id_;                                                                                             ///< original table id
        uint64_t index_tid_;                                                                                            ///< index id
        int32_t  cur_sri_index_;                                                                                        ///< current sri array index

    };
  }
}



#endif // OB_INDEX_DESIGNER

