/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_get_cell_stream_wrapper.h
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
 * ob_get_cell_stream_wrapper.h is for concealing ObMergerRpcProxy
 * initialization details from chunkserver
 *
 * Authors:
 *   wushi <wushi.ly@taobao.com>
 *   huating <huating.zmq@taobao.com>
 *
 */
#ifndef OCEANBASE_CHUNKSERVER_GET_CELL_STREAM_WRAPPER_H_ 
#define OCEANBASE_CHUNKSERVER_GET_CELL_STREAM_WRAPPER_H_

#include "common/ob_client_manager.h"
#include "common/ob_server.h"
#include "common/thread_buffer.h"
#include "ob_rpc_proxy.h"
#include "ob_get_cell_stream.h"
#include "ob_scan_cell_stream.h"
#include "ob_cs_interactive_cell_stream.h" //longfei [cons static index] 151216

namespace oceanbase
{
  namespace chunkserver
  {
    class ObGetCellStreamWrapper
    {
    public:
      /**
       * @param retry_times retry times
       * @param timeout network timeout
       * @param update_server address of update server
       */
      ObGetCellStreamWrapper(ObMergerRpcProxy& rpc_proxy, const int64_t time_out = 0);
      ~ObGetCellStreamWrapper();

      // get cell stream used for join
      ObGetCellStream *get_ups_get_cell_stream();
      // get cell stream used for merge
      ObScanCellStream *get_ups_scan_cell_stream();
      //add longfei [cons static index] 151205:b
      /**
       * @brief get_cs_interactive_cell_stream
       * @return cs_interactive_cell_stream_
       */
      ObCsInteractiveCellStream *get_cs_interactive_cell_stream();
      //add e
    private:
      ObGetCellStream get_cell_stream_;
      ObScanCellStream scan_cell_stream_;
      //add longfei [cons static index] 151205:b
      ObCsInteractiveCellStream cs_interactive_cell_stream_; ///< for a cs get cell from other cs
      //add e
    };
  }
}   

#endif //OCEANBASE_CHUNKSERVER_GET_CELL_STREAM_WRAPPER_H_
