/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_tablet_histogram_repo_info.cpp
* @brief for tablet histogram report info
*
* Created by maoxiaoxiao:operations to report tablet histogram
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#include <algorithm>
#include "tbsys.h"
#include "utility.h"
#include "ob_tablet_histogram_report_info.h"
#include "ob_tablet_info.h"
#include "ob_tablet_histogram.h"

namespace oceanbase
{
  namespace common
  {
    // ObTabletHistogramReportInfo
    DEFINE_SERIALIZE(ObTabletHistogramReportInfo)
    {
      int ret = OB_SUCCESS;
      ret = tablet_info.serialize(buf, buf_len, pos);
      if (ret == OB_SUCCESS)
        ret = tablet_location.serialize(buf, buf_len, pos);
      if (ret == OB_SUCCESS)
          ret = static_index_histogram.serialize(buf,buf_len,pos);
      return ret;
    }

    DEFINE_DESERIALIZE(ObTabletHistogramReportInfo)
    {
      int ret = OB_SUCCESS;
      ret = tablet_info.deserialize(buf, data_len, pos);
      if (ret == OB_SUCCESS)
        ret = tablet_location.deserialize(buf, data_len, pos);
      if (ret == OB_SUCCESS)
          ret = static_index_histogram.deserialize(buf,data_len,pos);

      return ret;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObTabletHistogramReportInfo)
    {
      int64_t total_size = 0;

      total_size += tablet_info.get_serialize_size();
      total_size += tablet_location.get_serialize_size();
      total_size += static_index_histogram.get_serialize_size();

      return total_size;
    }
    // ObTabletHistogramReportInfoList
    DEFINE_SERIALIZE(ObTabletHistogramReportInfoList)
    {
      int ret = OB_ERROR;

      int64_t size = tablet_list.get_array_index();
      ret = serialization::encode_vi64(buf, buf_len, pos, size);
      TBSYS_LOG(DEBUG, "test::liumz, serialize size=%ld", size);

      if (ret == OB_SUCCESS)
      {
        for (int64_t i = 0; i < size; ++i)
        {
          ret = tablets[i].serialize(buf, buf_len, pos);
          if (ret != OB_SUCCESS)
            break;
        }
      }

      return ret;
    }

    DEFINE_DESERIALIZE(ObTabletHistogramReportInfoList)
    {
      int ret = OB_ERROR;
      ObObj* ptr = NULL;

      int64_t size = 0;
      ObTabletHistogramReportInfo tablet;
      ret = serialization::decode_vi64(buf, data_len, pos, &size);
      //debugb longfei 2016-03-18 11:56:55
//      TBSYS_LOG(WARN, "debug::longfei>>>deserialize size=%ld", size);
      //debuge

      if (ret == OB_SUCCESS && size > 0)
      {
        for (int64_t i = 0; i < size; ++i)
        {
          ptr = reinterpret_cast<ObObj*>(allocator.alloc(sizeof(ObObj) * OB_MAX_ROWKEY_COLUMN_NUMBER * 2));
          if (NULL == ptr)
          {
            ret = OB_ALLOCATE_MEMORY_FAILED;
          }
          else
          {
            tablet.static_index_histogram.set_allocator(&allocator);
            tablet.static_index_histogram.do_not_free();
            tablet.tablet_info.range_.start_key_.assign(ptr, OB_MAX_ROWKEY_COLUMN_NUMBER);
            tablet.tablet_info.range_.end_key_.assign(ptr + OB_MAX_ROWKEY_COLUMN_NUMBER, OB_MAX_ROWKEY_COLUMN_NUMBER);
            ret = tablet.deserialize(buf, data_len, pos);
          }
          if (ret != OB_SUCCESS)
            break;

          tablet_list.push_back(tablet);
        }
      }

      return ret;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObTabletHistogramReportInfoList)
    {
      int64_t total_size = 0;
      int64_t size = tablet_list.get_array_index();
      total_size += serialization::encoded_length_vi64(size);

      if (size > 0)
      {
        for (int64_t i=0; i<size; ++i)
          total_size += tablets[i].get_serialize_size();
      }

      return total_size;
    }
  }
}
