/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_get_cell_stream_wrapper.cpp
 * @brief for concealing ObMergerRpcProxy initialization details from chunkserver
 *
 * modified by longfei：add Member variable cs_interactive_cell_stream_
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_19
 */

/**
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * version 2 as published by the Free Software Foundation. 
 *  
 * ob_get_cell_stream_wrapper.cpp is for concealing 
 * ObMergerRpcProxy initialization details from chunkserver 
 *
 * Authors:
 *   wushi <wushi.ly@taobao.com>
 *   huating <huating.zmq@taobao.com>
 *
 */
#include "ob_get_cell_stream_wrapper.h"
#include "common/ob_define.h"
#include "tbsys.h"

namespace oceanbase
{
  namespace chunkserver
  {
    using namespace oceanbase::common;

    ObGetCellStreamWrapper::ObGetCellStreamWrapper(
        ObMergerRpcProxy& rpc_proxy, const int64_t time_out)
    : get_cell_stream_(&rpc_proxy, CHUNK_SERVER, time_out), 
      scan_cell_stream_(&rpc_proxy, CHUNK_SERVER, time_out)
    //add longfei [cons static index] 151205:b
    ,cs_interactive_cell_stream_(&rpc_proxy, CHUNK_SERVER, time_out)
    //add e
    {
    }
    
    ObGetCellStreamWrapper::~ObGetCellStreamWrapper()
    {

    }
    
    ObGetCellStream *ObGetCellStreamWrapper::get_ups_get_cell_stream()
    {
      return &get_cell_stream_;
    }
    
    ObScanCellStream *ObGetCellStreamWrapper::get_ups_scan_cell_stream()
    {
      return &scan_cell_stream_;
    }

    //add longfei [cons static index] 151205:b
    ObCsInteractiveCellStream *ObGetCellStreamWrapper::get_cs_interactive_cell_stream()
    {
      return &cs_interactive_cell_stream_;
    }
    //add e
  } // end namespace chunkserver
} // end namespace oceanbase
