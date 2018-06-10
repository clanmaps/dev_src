/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cs_interactive_cell_stream.h
 * @brief define rpc interface between chunk like this :) cs <== rpc ==> cs
 *
 * Created by longfei：provide cell stream for operator ObCsInteractiveAgent
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_04
 */

#ifndef CHUNKSERVER_OB_CS_INTERACTIVE_CELL_STREAM_H_
#define CHUNKSERVER_OB_CS_INTERACTIVE_CELL_STREAM_H_

#include "common/ob_read_common_data.h"
#include "ob_cell_stream.h"

namespace oceanbase
{
  namespace chunkserver
  {
    /**
     * @brief The ObCsInteractiveCellStream class
     * is designed for get data in other cs
     */
    class ObCsInteractiveCellStream: public ObCellStream
    {
    public:
      /**
       * @brief ~ObCsInteractiveCellStream destructor
       */
      virtual ~ObCsInteractiveCellStream();
      /**
       * @brief ObCsInteractiveCellStream constructor
       * @param rpc_proxy
       * @param server_type
       * @param time_out
       */
      ObCsInteractiveCellStream(
          ObMergerRpcProxy * rpc_proxy,
          const common::ObServerType server_type = common::MERGE_SERVER,
          const int64_t time_out = 0);
    public:
      /**
       * @brief next_cell retrieve cell from cur_result_
       * @return error code
       */
      virtual int next_cell();
      /**
       * @brief scan fill cur_result_
       * @param param
       * @return error code
       */
      virtual int scan(const common::ObScanParam & param);
      /**
       * @brief get_data_version
       * get the current scan data version, this function must
       * be called after next_cell()
       * @return data version
       */
      virtual int64_t get_data_version() const;
      /**
       * @brief set_chunkserver
       * @param server
       */
      void set_chunkserver(const ObTabletLocationList server);
      /**
       * @brief reset_inner_stat
       */
      void reset_inner_stat();
      /**
       * @brief set_self
       * @param self
       */
      inline void set_self(ObServer self)
      {
        self_ = self;
      }
      /**
       * @brief get_self
       * @return self_
       */
      inline ObServer get_self()
      {
        return self_;
      }
    private:
      /**
       * @brief check_finish_scan:
       * check whether finish scan, if finished return server's servering tablet range
       * @param param current scan param
       * @return error code
       */
      int check_finish_scan(const common::ObScanParam & param);
      /**
       * @brief get_next_cell: get next cell from cur_result_
       * @return error code
       */
      int get_next_cell(void);
      /**
       * @brief scan_row_data: store result in cur_result_
       * @return error code
       */
      int scan_row_data();
      /**
       * @brief check_inner_stat: check rpc_proxy_
       * @return true or false
       */
      bool check_inner_stat(void) const;
      DISALLOW_COPY_AND_ASSIGN(ObCsInteractiveCellStream);
    private:
      bool finish_; ///< finish all scan routine status
      common::ObMemBuf range_buffer_; ///< for modify param range
      const common::ObScanParam * scan_param_; ///<  orignal scan param
      common::ObScanParam cur_scan_param_; ///< current scan param
      ObTabletLocationList chunkserver_; ///<  Choose CS to be sent
      int64_t cur_rep_index_; ///< Index of the current copy
      ObServer self_; ///< this cs itself
    };

    // check inner stat
    inline bool ObCsInteractiveCellStream::check_inner_stat(void) const
    {
      //finish_ = false;
      return (ObCellStream::check_inner_stat() && (NULL != scan_param_));
    }

    // reset inner stat
    inline void ObCsInteractiveCellStream::reset_inner_stat()
    {
      ObCellStream::reset_inner_stat();
      finish_ = false;
      scan_param_ = NULL;
      cur_scan_param_.reset();
      cur_rep_index_ = 0;
    }

    inline void ObCsInteractiveCellStream::set_chunkserver(
        const ObTabletLocationList server)
    {
      chunkserver_ = server;
    }

  } /* namespace chunkserver */
} /* namespace oceanbase */

#endif /* CHUNKSERVER_OB_CS_INTERACTIVE_CELL_STREAM_H_ */
