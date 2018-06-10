/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_tablet_histogram_repo_info.h
* @brief for tablet histogram report info
*
* Created by maoxiaoxiao:operations to report tablet histogram
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_TABLET_HISTOGRAM_REPORT_INFO
#define OB_TABLET_HISTOGRAM_REPORT_INFO
#include <tblog.h>
#include "ob_define.h"
#include "ob_server.h"
#include "ob_array_helper.h"
#include "page_arena.h"
#include "ob_range2.h"
#include "common/ob_tablet_info.h"
#include "common/ob_tablet_histogram.h"


namespace oceanbase
{
  namespace common
  {
    /**
     * @brief The ObTabletHistogramReportInfo struct
     * ObTabletHistogramReportInfo is designed for
     * storing one tablet histogram report information
     */
    struct ObTabletHistogramReportInfo
    {
      ObTabletInfo tablet_info; ///<tablet information
      ObTabletLocation tablet_location; ///<tablet location
      ObTabletHistogram static_index_histogram; ///<tablet histogram information
      //ObObj ptr[OB_MAX_ROWKEY_COLUMN_NUMBER * 2];
      bool operator== (const ObTabletReportInfo &other) const;
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    /**
     * @brief The ObTabletHistogramReportInfoList struct
     * ObTabletHistogramReportInfoList is designed for
     * storing a list of tablet histogram report information
     */
    struct ObTabletHistogramReportInfoList
    {
      ObTabletHistogramReportInfo tablets[OB_MAX_TABLET_LIST_NUMBER]; ///<a list of tablet histogram report information
      ObArrayHelper<ObTabletHistogramReportInfo> tablet_list; ///<array helper
      ModuleArena allocator; ///<module arena
      mutable tbsys::CRWLock lock; ///<lock

      /**
       * @brief constructor
       */
      ObTabletHistogramReportInfoList()
      {
        reset();
      }

      /**
       * @brief reset
       */
      void reset()
      {
        tablet_list.init(OB_MAX_TABLET_LIST_NUMBER, tablets);
        tablet_list.clear();
      }

      /**
       * @brief get_tablet_histogram
       * get one tablet histogram report information with the given index
       * @param tablet
       * @param index
       * @return OB_SUCCESS or other ERROR
       */
      int get_tablet_histogram(ObTabletHistogramReportInfo &tablet, int64_t index)
      {
        int ret = OB_SUCCESS;
        if(index >=0 && index < tablet_list.get_array_index())
        {
          tablet=tablets[index];
          if(index == (tablet_list.get_array_index()-1))
          {
            ret = OB_ITER_END;
          }
        }
        return ret;
      }

      /**
       * @brief add_tablet
       * add one tablet to the tablet_list
       * @param tablet
       * @return OB_SUCCESS or other ERROR
       */
      inline int add_tablet(const ObTabletHistogramReportInfo& tablet)
      {
        int ret = OB_SUCCESS;
        tbsys::CWLockGuard guard(lock);
        if (!tablet_list.push_back(tablet))
        {
          ret = OB_ARRAY_OUT_OF_RANGE;
        }
        //debugb longfei 2016-03-18 11:38:53
        else
        {
          //输出tablet的信息
//          TBSYS_LOG(WARN,"debug::longfei>>>add tablet[%s] into tablet_list!", to_cstring(tablet.tablet_info.range_));
        }
        //debuge

        return ret;
      }
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

  } // end namespace common
} // end namespace oceanbase

#endif // OB_TABLET_HISTOGRAM_REPORT_INFO

