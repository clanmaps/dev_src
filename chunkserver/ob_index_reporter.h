/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_index_reporter.h
* @brief for report index histogram info
*
* Created by maoxiaoxiao:send local index histogram information and send index tablet information
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_INDEX_REPORTER_H
#define OB_INDEX_REPORTER_H

#include "common/ob_tablet_histogram.h"
#include "common/ob_tablet_histogram_report_info.h"
#include "common/ob_mod_define.h"
//#include "ob_tablet_manager.h"


namespace oceanbase
{
  namespace chunkserver
  {
    class ObTabletManager;

    /**
     * @brief The ObIndexReporter class
     * ObIndexReporter is designed for
     * report index histogram information
     */
    class ObIndexReporter
    {
    public:

      /**
       * @brief constructor
       */
      ObIndexReporter()
      {

      }

      //void init(ObTabletManager* tablet_manager){tablet_manager_ = tablet_manager;}

      /**
       * @brief get_tablet_histogram
       * @return &static_index_histogram_
       */
      common::ObTabletHistogram *get_tablet_histogram()
      {
        return &static_index_histogram_;
      }

      /**
       * @brief get_tablet_histogram_report_info
       * @return &static_index_report_info_
       */
      common::ObTabletHistogramReportInfo *get_tablet_histogram_report_info()
      {
        return &static_index_report_info_;
      }

      /**
       * @brief get_tablet_histogram_report_info_list
       * @return static_index_report_info_list_
       */
      common::ObTabletHistogramReportInfoList *&get_tablet_histogram_report_info_list()
      {
        return static_index_report_info_list_;
      }

      /**
       * @brief reset_report_info
       */
      void reset_report_info();

      /**
       * @brief send_local_index_info
       * send local index histogram information
       * @param tablet_manager
       * @param report_info_list
       * @return OB_SUCCESS or other ERROR
       */
      static int send_local_index_info(ObTabletManager *tablet_manager, common::ObTabletHistogramReportInfoList* report_info_list);

      /**
       * @brief send_index_info
       * send index tablet information
       * @param tablet_manager
       * @param index_tid
       * @return OB_SUCCESS or other ERROR
       */
      static int  send_index_info(ObTabletManager *tablet_manager, uint64_t index_tid);

    private:
      common::ObTabletHistogram static_index_histogram_; ///<index histogram information
      common::ObTabletHistogramReportInfo static_index_report_info_; ///<index report information
      common::ObTabletHistogramReportInfoList *static_index_report_info_list_; ///<index report information list
      //ObTabletManager *tablet_manager_;
    };
  }
}

#endif // OB_INDEX_REPORTER_H
