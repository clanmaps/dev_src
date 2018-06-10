/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_local_agent.h
 * @brief get range data in cs itself
 *
 * Created by longfei： local agent is for the global stage of constructing static index,
 *  get the data of the range when those datas in myself.
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */

#ifndef CHUNKSERVER_OB_INDEX_LOCAL_AGENT_H_
#define CHUNKSERVER_OB_INDEX_LOCAL_AGENT_H_

#include "common/ob_schema.h"
#include "ob_cell_stream.h"
#include "ob_scan_cell_stream.h"
#include "ob_rpc_proxy.h"
#include "common/ob_scan_param.h"
#include "sql/ob_no_children_phy_operator.h"
#include "sql/ob_sstable_scan.h"

namespace oceanbase
{
  using namespace common;
  using namespace sql;
  namespace chunkserver
  {
    typedef hash::ObHashMap<ObNewRange, ObTabletLocationList,
    hash::NoPthreadDefendMode> RangeServerHash;
    typedef hash::ObHashMap<ObNewRange, ObTabletLocationList,
    hash::NoPthreadDefendMode>::const_iterator HashIterator;

    /**
     * @brief The ObIndexLocalAgent class
     * is designed for scan range in cs itself
     */
    class ObIndexLocalAgent: public ObNoChildrenPhyOperator
    {
    public:
      /**
       * @brief ObIndexLocalAgent
       * constructor
       */
      ObIndexLocalAgent();
      /**
       * @brief ~ObIndexLocalAgent
       * destructor
       */
      virtual ~ObIndexLocalAgent();
      /**
       * @brief reset
       */
      virtual void reset();
      /**
       * @brief reuse
       */
      virtual void reuse();
      /**
       * @brief open
       * @return err code
       */
      virtual int open();
      /**
       * @brief close
       * @return err code
       */
      virtual int close();
      /**
       * @brief get_type
       * @return err code
       */
      virtual ObPhyOperatorType get_type() const
      {
        return PHY_INDEX_LOCAL_AGENT;
      }
      /**
       * @brief get_next_row
       * @param row [out]
       * @return err code
       */
      virtual int get_next_row(const ObRow *&row);
      /**
       * @brief get_row_desc
       * @param row_desc [out]
       * @return err code
       */
      virtual int get_row_desc(const ObRowDesc *&row_desc) const;
      /**
       * @brief to_string: Not implemented
       * @param buf
       * @param buf_len
       * @return success
       */
      virtual int64_t to_string(char* buf, const int64_t buf_len) const;

    public:
      /**
       * @brief set_row_desc
       * @param desc
       */
      void set_row_desc(const ObRowDesc &desc);
      /**
       * @brief set_scan_param
       * @param scan_param
       * @return success or fail
       */
      int set_scan_param(ObScanParam *scan_param);
      /**
       * @brief set_range_server_hash
       * @param range_server_hash
       * @return success or fail
       */
      int set_range_server_hash(const chunkserver::RangeServerHash *range_server_hash);
      /**
       * @brief build_sst_scan_param: build sstable scan parameter according to scan_param_
       * @return success or fail
       */
      int build_sst_scan_param();
      /**
       * @brief get_next_local_row
       * @param [out] row
       * @return success or fail
       */
      int get_next_local_row(const ObRow *&row);
      /**
       * @brief scan
       * 1.get_next_local_range
       * 2.open_scan_context_local_idx
       * 3.init_sstable_scanner_for_local_idx
       * @return success, iter_end or fail
       */
      int scan();
      /**
       * @brief get_next_local_range: get next range in the cs itself
       * @param range
       * @return success, iter_end or fail
       */
      int get_next_local_range(ObNewRange &range);
      /**
       * @brief set_server
       * @param server
       */
      void set_server(ObServer server)
      {
        self_ = server;
      }
      /**
       * @brief set_scan_context
       * @param sc
       */
      void set_scan_context(ScanContext &sc)
      {
        sc_ = sc;
      }
      DECLARE_PHY_OPERATOR_ASSIGN;

    private:
      ObRowDesc row_desc_; ///< row desc
      ObRow cur_row_; ///< cur row for get_next_row()
      ObSSTableScan sst_scan_; ///< sstable scan operator
      ObScanParam *scan_param_; ///< scan parameter
      sstable::ObSSTableScanParam sst_scan_param_; ///< sstable scan parameter
      const chunkserver::RangeServerHash *range_server_hash_; ///< range which needs to be constructed
      ObNewRange fake_rage_; ///< range recieved from rs
      int64_t hash_index_; ///< index of range_server_hash
      bool local_idx_scan_finish_; ///< is local index scan finished?
      bool local_idx_block_end_; ///< is all local index has been dealt with?
      bool first_scan_; ///< is first scan?
      ObServer self_; ///< cs itself
      ScanContext sc_; ///< sstable scan context
    };

  } //end chunkserver

} //end oceanbase

#endif /* CHUNKSERVER_OB_INDEX_LOCAL_AGENT_H_ */
