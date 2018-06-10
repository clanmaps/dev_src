/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cs_interactive_scan.cpp
 * @brief father operator of ObIndexLocalAgent and ObIndexInteractiveAgent
 *
 * Created by longfei：scan agent for construct static index
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_07
 */

#include "ob_cs_interactive_scan.h"
#include "tbsys.h"

namespace oceanbase
{
  using namespace common;
  namespace chunkserver
  {
    ObCsInteractiveScan::ObCsInteractiveScan() :
      local_idx_scan_finish_(false)
    {
    }

    ObCsInteractiveScan::~ObCsInteractiveScan()
    {
    }

    int ObCsInteractiveScan::get_next_row(const common::ObRow *&row)
    {
      int ret = OB_SUCCESS;
      //@(longfei):need to set local_idx_scan_finish_
      if (!local_idx_scan_finish_)
      {
        if (OB_SUCCESS != (ret = (left_op_->get_next_row(row))))
        {
          if(OB_ITER_END == ret)
          {
            local_idx_scan_finish_ = true;
          }
          else
          {
            TBSYS_LOG(ERROR, "get local next row failed");
          }
        }
      }
      if(local_idx_scan_finish_)
      {
        if (OB_SUCCESS != (ret = (right_op_->get_next_row(row))))
        {
          if(OB_ITER_END == ret)
          {
          }
          else
          {
            TBSYS_LOG(ERROR, "get interactive next row failed");
          }
        }
      }
      return ret;
    }

    int ObCsInteractiveScan::get_row_desc(const ObRowDesc *& row_desc) const
    {
      int ret = OB_SUCCESS;
      UNUSED(row_desc);
      return ret;
    }

    int64_t ObCsInteractiveScan::to_string(char *buf, const int64_t buf_len) const
    {
      int ret = OB_SUCCESS;
      UNUSED(buf);
      UNUSED(buf_len);
      return ret;
    }
  } /* namespace chunkserver */
} /* namespace oceanbase */
