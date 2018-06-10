/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_interactive_agent.h
 * @brief define rpc interface between chunkserver like this :) cs <== rpc ==> cs
 *
 * Created by longfei：  interactive agent is for the global stage of constructing static index,
 *  get the data of the range when those datas in other chunkserver.
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */

#ifndef CHUNKSERVER_OB_INDEX_INTERACTIVE_AGENT_H_
#define CHUNKSERVER_OB_INDEX_INTERACTIVE_AGENT_H_

#include "sql/ob_no_children_phy_operator.h"
#include "ob_cs_interactive_cell_stream.h"
#include "ob_index_local_agent.h"

namespace oceanbase
{
  using namespace common;
  using namespace sql;
  namespace chunkserver
  {
    /**
     * @brief The ObIndexInteractiveAgent class
     * is designed for scan range in other cs
     */
    class ObIndexInteractiveAgent: public ObNoChildrenPhyOperator
    {
    public:
      /**
       * @brief ObIndexInteractiveAgent constructor
       */
      ObIndexInteractiveAgent();
      /**
       * @brief ~ObIndexInteractiveAgent destructor
       */
      virtual ~ObIndexInteractiveAgent(){}
      /**
       * @brief reset
       */
      virtual void reset();
      /**
       * @brief reuse: Not implemented
       */
      virtual void reuse();
      /**
       * @brief open: an issue(todo item)
       * @return success
       */
      virtual int open();
      /**
       * @brief close: Not implemented
       * @return success
       */
      virtual int close();
      /**
       * @brief get_type
       * @return PHY_INDEX_INTERACTIVE_AGENT
       */
      virtual ObPhyOperatorType get_type() const
      {
        return PHY_INDEX_INTERACTIVE_AGENT;
      }
      /**
       * @brief get_next_row
       * @param [out] row
       * @return err code
       */
      virtual int get_next_row(const ObRow *&row);
      /**
       * @brief to_string Not implemented
       * @param buf
       * @param buf_len
       * @return err code
       */
      virtual int64_t to_string(char* buf, const int64_t buf_len) const;
      /**
       * @brief get_row_desc
       * @param row_desc
       * @return OB_NOT_SUPPORTED
       */
      virtual int get_row_desc(const ObRowDesc *&row_desc) const
      {
        row_desc = NULL;
        return common::OB_NOT_SUPPORTED;
      }

    public:
      /**
       * @brief get_next_row
       * @param [out] row
       * @return succ, iter_end or fail
       */
      int get_next_row(ObRow &row);
      /**
       * @brief set_row_desc
       * @param desc
       */
      void set_row_desc(const ObRowDesc &desc);
      /*add this function so that index builder can return failed fake range*/
      /**
       * @brief set_failed_fake_range
       * @param range
       */
      void set_failed_fake_range(const ObNewRange &range);
      /**
       * @brief start_agent
       * @param scan_param
       * @param cs_stream
       * @param hash
       * @return err code
       */
      int start_agent(
          ObScanParam &scan_param,
          ObCsInteractiveCellStream &cs_stream,
          const RangeServerHash *hash);
      /**
       * @brief stop_agent
       */
      void stop_agent();
      /**
       * @brief get_failed_fake_range
       * @param range
       * @return OB_INNER_STAT_ERROR or succ
       */
      int get_failed_fake_range(ObNewRange &range);
    public:
      /**
       * @brief get_cell
       * @param cell
       * @return err code
       */
      int get_cell(common::ObCellInfo** cell);
      /**
       * @brief get_cell
       * @param cell
       * @param is_row_changed
       * @return err code
       */
      int get_cell(common::ObCellInfo** cell, bool* is_row_changed);
      /**
       * @brief next_cell
       * @return err code
       */
      int next_cell();

    private:
      ObCsInteractiveCellStream *interactive_cell_; ///< stream to get data from other cs
      common::ObScanParam *scan_param_; ///< scan parameter
      bool inited_; ///< has been inited?
      const RangeServerHash *range_server_hash_; ///< data_multics_range_hash!
      int32_t hash_index_; ///< for Traversal of range_server_hash_
      int64_t column_count_; ///< column number
      ObNewRange failed_fake_range_; ///< failed index table's range
      ObRow curr_row_; ///< current row
      ObRowDesc row_desc_; ///< row desc

    };

  }
}

#endif /* CHUNKSERVER_OB_INDEX_INTERACTIVE_AGENT_H_ */
