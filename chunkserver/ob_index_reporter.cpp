/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_index_reporter.cpp
* @brief for report index histogram info
*
* Created by maoxiaoxiao:send local index histogram information and send index tablet information
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#include "ob_index_reporter.h"
#include "ob_chunk_server_main.h"
namespace oceanbase
{
    namespace chunkserver
    {
        void ObIndexReporter::reset_report_info()
        {
            static_index_histogram_.init();
            memset(&(static_index_report_info_.tablet_info), 0, sizeof(ObTabletInfo));
            static_index_report_info_.static_index_histogram.init();
            static_index_report_info_.tablet_location.chunkserver_.reset();
            static_index_report_info_.tablet_location.tablet_seq_ = 0;
            static_index_report_info_.tablet_location.tablet_version_ = 0;
        }

        int ObIndexReporter::send_local_index_info(ObTabletManager *tablet_manager, common::ObTabletHistogramReportInfoList* report_info_list)
        {
          int ret = OB_SUCCESS;
          int64_t histogram_index = 0;
          const int64_t REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE = 8;
          int64_t serialize_size = REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE;
          int64_t max_serialize_size = OB_MAX_PACKET_LENGTH - 1024;
          common::ObTabletHistogramReportInfo* report_info = OB_NEW(common::ObTabletHistogramReportInfo, common::ObModIds::OB_TABLET_HISTOGRAM_REPORT);
          common::ObTabletHistogramReportInfoList* report_list = OB_NEW(common::ObTabletHistogramReportInfoList, common::ObModIds::OB_TABLET_HISTOGRAM_REPORT);
          common::ObTabletHistogramReportInfoList* tablet_histogram_report_list = report_info_list;
          if(NULL == report_list || NULL == report_info)
          {
             ret = OB_ALLOCATE_MEMORY_FAILED;
          }
          else
          {
             report_list->reset();
          }
          while (OB_SUCCESS == ret)
          {
            while (OB_SUCCESS == ret && histogram_index  < tablet_histogram_report_list->tablet_list.get_array_index())
            {
              ret = tablet_histogram_report_list->get_tablet_histogram(*report_info , histogram_index);

              if(OB_SUCCESS != ret)
              {
                if (OB_ITER_END != ret)
                {
                  TBSYS_LOG(WARN, "failed to get tablet histogram info, err=%d", ret);
                }
              }
              else
              {
                histogram_index ++;
                ret = report_list->add_tablet(*report_info);
                if (OB_SUCCESS != ret)
                {
                  TBSYS_LOG(WARN, "failed to add tablet histogram info, err=%d", ret);
                }
                else
                {
                  serialize_size += report_info->get_serialize_size();
                  if (serialize_size > max_serialize_size)
                  {
                    break;
                  }
                }
              }
            }
            if(OB_SUCCESS == ret)
            {
              int64_t retry_times = THE_CHUNK_SERVER.get_config().retry_times;
              RPC_RETRY_WAIT(!(tablet_manager->is_stoped()),  retry_times, ret, CS_RPC_CALL_RS(report_tablets_histogram, THE_CHUNK_SERVER.get_self(), *report_list, tablet_manager->get_serving_data_version(), false));
              //ret = tablet_manager_->send_tablet_histogram_report(*report_list, true);
              if(OB_SUCCESS != ret)
              {
                TBSYS_LOG(WARN, "failed to send tablet histogram info, err=%d", ret);
              }
            }
            if (OB_SUCCESS == ret)
            {
              serialize_size = REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE;
              report_list->reset();
            }
          }
          if(OB_ITER_END == ret)
          {
            ret = report_list->add_tablet(*report_info);
            if (OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "failed to add tablet histogram info, err=%d", ret);
            }
            //debugb longfei 2016-03-18 11:51:32
//            TBSYS_LOG(WARN, "debug::longfei>>>tablet info to report 1 = %s,add in info 2 = %s",
//                      to_cstring(report_list->tablets[0].tablet_info.range_), to_cstring(report_list->tablets[1].tablet_info.range_));
//            TBSYS_LOG(WARN, "debug::longfei>>>report_list.index[%ld], size[%ld]",
//                      report_list->tablet_list.get_array_index(), report_list->tablet_list.get_array_size());
            //debuge
            int64_t retry_times = THE_CHUNK_SERVER.get_config().retry_times;
            RPC_RETRY_WAIT(!(tablet_manager->is_stoped()),  retry_times, ret, CS_RPC_CALL_RS(report_tablets_histogram, THE_CHUNK_SERVER.get_self(), *report_list, tablet_manager->get_serving_data_version(), false));
            //ret = tablet_manager_->send_local_index_info(*report_list, false);
            if (OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "failed to send tablet histogram report in last round, ret=%d", ret);
            }
          }
          OB_DELETE(ObTabletHistogramReportInfo,ObModIds::OB_TABLET_HISTOGRAM_REPORT, report_info);
          OB_DELETE(ObTabletHistogramReportInfoList,ObModIds::OB_TABLET_HISTOGRAM_REPORT, report_list);

          return ret;
        }

        int ObIndexReporter::send_index_info(ObTabletManager* tablet_manager, uint64_t index_tid)
        {
          int err = OB_SUCCESS;
          ObTablet* tablet = NULL;
          ObTabletReportInfo tablet_info;
          int64_t num = OB_MAX_TABLET_LIST_NUMBER;
          /**
           * FIXME: in order to avoid call report_info_list->get_serialize_size()
           * too many times, we caculate the serialize size incremently here. the
           * report info list will serialize the tablet count and it occupy 8 bytes
           * at most, so we reserve 8 bytes for it.
           */
          const int64_t REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE = 8;
          int64_t max_serialize_size = OB_MAX_PACKET_LENGTH - 1024;
          int64_t serialize_size = REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE;
          int64_t cur_tablet_brother_cnt = 0;
          ObMergerSchemaManager *merger_schema_manager = THE_CHUNK_SERVER.get_schema_manager();
          int64_t frozen_version = merger_schema_manager->get_latest_version();
          const ObSchemaManagerV2 *schema_manager = merger_schema_manager->get_user_schema(frozen_version);
          ObTabletReportInfoList *report_info_list_first = GET_TSI_MULT(ObTabletReportInfoList, TSI_CS_TABLET_REPORT_INFO_LIST_1);
          ObTabletReportInfoList *report_info_list_second = GET_TSI_MULT(ObTabletReportInfoList, TSI_CS_TABLET_REPORT_INFO_LIST_2);
          ObTabletReportInfoList *report_info_list = report_info_list_first;
          ObTabletReportInfoList *report_info_list_rollback = NULL;
          if (NULL == report_info_list_first || NULL == report_info_list_second)
          {
            err = OB_ALLOCATE_MEMORY_FAILED;
          }
          else
          {
            report_info_list_first->reset();
            report_info_list_second->reset();
          }

          bool is_version_changed = false;
          ObMultiVersionTabletImage& image = tablet_manager->get_serving_tablet_image();
          if (OB_SUCCESS == err)
          {
            err = image.begin_scan_tablets();
          }
          if (OB_ITER_END == err)
          {
            image.end_scan_tablets();
            // iter ends
          }
          else if (OB_SUCCESS != err)
          {
            TBSYS_LOG(WARN, "failed to scan tablets, tablet=%p, err=%d",
                      tablet, err);
            err = OB_ERROR;
          }
          else
          {
            while (OB_SUCCESS == err)
            {
              while (OB_SUCCESS == err && num > 0)
              {

                err = image.get_next_tablet(tablet);
                if (OB_ITER_END == err)
                {
                  // iter ends
                }
                else if (OB_SUCCESS != err || NULL == tablet)
                {
                  TBSYS_LOG(WARN, "failed to get next tablet, err=%d, tablet=%p",
                      err, tablet);
                  err = OB_ERROR;
                }
                else if (tablet->get_data_version() != image.get_serving_version())
                {
                  image.release_tablet(tablet);
                  is_version_changed = true;
                  continue;
                }
                else if (tablet->is_removed())
                {
                  TBSYS_LOG(WARN, "report: ignore removed range = <%s>", to_cstring(tablet->get_range()));
                  image.release_tablet(tablet);
                  continue;
                }
                else
                {

                  if (!tablet->get_range().is_left_open_right_closed())
                  {
                    TBSYS_LOG(WARN, "report illegal tablet range = <%s>", to_cstring(tablet->get_range()));
                  }
                  if(OB_SUCCESS == err)
                  {
                    const ObTableSchema *table_schema = schema_manager->get_table_schema(tablet->get_range().table_id_);
                    if((OB_INVALID_ID == table_schema->get_original_table_id()) || (tablet->get_range().table_id_ != index_tid))
                    {
                        err = OB_SUCCESS;
                        image.release_tablet(tablet);
                        continue;
                    }
                  }
                  tablet_manager->fill_tablet_info(*tablet, tablet_info);
                  TBSYS_LOG(INFO, "add tablet <%s>, row count:[%ld], size:[%ld], "
                      "crc:[%ld] row_checksum:[%lu] version:[%ld] sequence_num:[%ld] to report list",
                      to_cstring(tablet_info.tablet_info_.range_),
                      tablet_info.tablet_info_.row_count_,
                      tablet_info.tablet_info_.occupy_size_,
                      tablet_info.tablet_info_.crc_sum_,
                      tablet_info.tablet_info_.row_checksum_,
                      tablet_info.tablet_location_.tablet_version_,
                      tablet_info.tablet_location_.tablet_seq_);
                  err = report_info_list->add_tablet(tablet_info);
                  if (OB_SUCCESS != err)
                  {
                    TBSYS_LOG(WARN, "failed to add tablet info, num=%ld, err=%d", num, err);
                  }
                  else
                  {
                    serialize_size += tablet_info.get_serialize_size();
                    if (!tablet->is_with_next_brother()
                        && serialize_size <= max_serialize_size)
                    {
                      cur_tablet_brother_cnt = 0;
                    }
                    else
                    {
                      cur_tablet_brother_cnt ++;
                    }
                    --num;
                  }

                  if (OB_SUCCESS != image.release_tablet(tablet))
                  {
                    TBSYS_LOG(WARN, "failed to release tablet, tablet=%p", tablet);
                    err = OB_ERROR;
                  }

                  if (serialize_size > max_serialize_size)
                  {
                    break;
                  }
                }
              }

              if (OB_SUCCESS == err && cur_tablet_brother_cnt > 0)
              {
                if (serialize_size > max_serialize_size)
                {
                  /**
                   * FIXME: it's better to ensure the tablets splited from one
                   * tablet are reported in one packet, in this case, one tablet
                   * splits more than 1024 tablets, after add the last tablet, the
                   * serialize size is greater than packet size, we need rollback
                   * the last tablet, and report the tablets more than one packet,
                   * rootserver is also handle this case.
                   */
                  TBSYS_LOG(WARN, "one tablet splited more than %ld tablets, "
                                  "and the serialize size is greater than packet size, "
                                  "rollback the last tablet, and can't report in one packet "
                                  "atomicly, serialize_size=%ld, max_serialize_size=%ld",
                    OB_MAX_TABLET_LIST_NUMBER, serialize_size, max_serialize_size);
                  cur_tablet_brother_cnt = 1;
                }
                if (cur_tablet_brother_cnt < OB_MAX_TABLET_LIST_NUMBER)
                {
                  if (report_info_list == report_info_list_first)
                  {
                    report_info_list_rollback = report_info_list_second;
                  }
                  else
                  {
                    report_info_list_rollback = report_info_list_first;
                  }
                  report_info_list_rollback->reset();
                  err = report_info_list->rollback(*report_info_list_rollback, cur_tablet_brother_cnt);
                  if (OB_SUCCESS != err)
                  {
                    TBSYS_LOG(WARN, "failed to rollback tablet info report list, err=%d", err);
                  }
                  else if (report_info_list->get_serialize_size() > max_serialize_size)
                  {
                    TBSYS_LOG(ERROR, "report_info_list serialize_size: %ld still greater than %ld",
                        report_info_list->get_serialize_size(), max_serialize_size);
                    err = OB_ERROR;
                  }
                }
                else
                {
                  /**
                   * FIXME: it's better to ensure the tablets splited from one
                   * tablet are reported in one packet, in this case, rootserver
                   * can ensure the atomicity. we only report 1024 tablets to
                   * rootserver each time, if one tablet splits more than 1024
                   * tablets, we can't report all the tablets in one packet, we
                   * report the tablets more than one packet, rootserver is also
                   * handle this case.
                   */
                  TBSYS_LOG(WARN, "one tablet splited more than %ld tablets, "
                                  "can't report in one packet atomicly",
                    OB_MAX_TABLET_LIST_NUMBER);
                  cur_tablet_brother_cnt = 0;
                }
              }

              if (OB_SUCCESS == err)
              {
                err = tablet_manager->send_tablet_report(*report_info_list, true);
                if (OB_SUCCESS != err)
                {
                  TBSYS_LOG(WARN, "failed to send tablet info report, err=%d", err);
                }
              }

              if (OB_SUCCESS == err)
              {
                serialize_size = REPORT_INFO_LIST_RESERVED_SERIALIZE_SIZE;
                num = OB_MAX_TABLET_LIST_NUMBER;
                report_info_list->reset();
              }

              if (OB_SUCCESS == err && cur_tablet_brother_cnt > 0)
              {
                num = OB_MAX_TABLET_LIST_NUMBER - cur_tablet_brother_cnt;
                report_info_list = report_info_list_rollback;
                serialize_size = report_info_list->get_serialize_size();
              }
            }
            image.end_scan_tablets();
          }

          if (OB_ITER_END == err)
          {
            err = tablet_manager->send_tablet_report(*report_info_list, false);
            if (OB_SUCCESS != err)
            {
              TBSYS_LOG(WARN, "failed to send tablet report in last round, ret=%d", err);
            }
          }

          if (is_version_changed)
          {
            err = OB_CS_EAGAIN;
          }

          //mod wenghaixing [realse schema while function end!] 20160128:b
          //if(OB_SUCCESS == err)
          if(NULL != schema_manager)
          //mod e
          {
            if(OB_SUCCESS != (err = merger_schema_manager->release_schema(schema_manager)))
            {
              TBSYS_LOG(WARN, "failed to releas schema_managerV2, err = %d",err);
            }
            else
            {
              schema_manager = NULL; //add wenghaixing [realse schema while function end!] 20160128:e
            }
          }

          return err;
        }
    }
}
