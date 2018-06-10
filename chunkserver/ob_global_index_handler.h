/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_global_index_handler.h
 * @brief for global stage of construct secondary index
 *
 * Created by longfei： for global stage of construct secondary index
 * future work
 *   1.some function need to be realized,see todo list in this page
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_05
 */

#ifndef CHUNKSERVER_OB_GLOBAL_INDEX_HANDLER_H_
#define CHUNKSERVER_OB_GLOBAL_INDEX_HANDLER_H_

#include "ob_index_handler.h"
#include "common/ob_range2.h"
#include "common/ob_column_checksum.h"
#include "ob_index_local_agent.h"
#include "ob_index_interactive_agent.h"
#include "ob_get_cell_stream_wrapper.h"
#include "sql/ob_sort.h"

namespace oceanbase
{
  namespace chunkserver
  {
    class ObIndexHandlePool;

    /**
     * @brief The ObGlobalIndexHandler class
     * is designed for global stage,each handler to handle a range
     */
    class ObGlobalIndexHandler: public ObIndexHandler
    {
    public:
      /**
       * @brief ObGlobalIndexHandler: constructor
       * @param pool
       * @param schema_mgr
       * @param tablet_mgr
       */
      ObGlobalIndexHandler(ObIndexHandlePool *pool,common::ObMergerSchemaManager *schema_mgr,ObTabletManager* tablet_mgr);
      /**
       * @brief ~ObGlobalIndexHandler: destructor
       */
      virtual ~ObGlobalIndexHandler();

    public:
      /**
       * @brief start
       * @return ret
       */
      virtual int start();
      /**
       * @brief to_string: Not Implemented
       * @return succ
       */
      virtual int to_string();
      /**
       * @brief init: do check!
       * @return ret code
       */
      virtual int init();
      /**
       * @brief cons_global_index
       * @param range
       * @return ret
       */
      int cons_global_index(ObNewRange *range);
      /**
       * @brief fill_scan_param
       * @param param
       * @return ret
       */
      int fill_scan_param(ObScanParam &param);
      /**
       * @brief write_global_index
       * @return ret
       */
      int write_global_index();
      /**
       * @brief cons_row_desc
       * @param desc
       * @return ret
       */
      int cons_row_desc(ObRowDesc &desc);
      /**
       * @brief cons_row_desc_without_virtual: without virtual column in row desc
       * @param desc
       * @return error code
       */
      int cons_row_desc_without_virtual(ObRowDesc &desc);
      /**
       * @brief calc_tablet_col_checksum_index
       * @param row
       * @param desc
       * @param [out] column_checksum
       * @param tid
       * @return error code
       */
      int calc_tablet_col_checksum_index(
          const ObRow& row,
          ObRowDesc desc,
          char *column_checksum,
          int64_t tid);
      /**
       * @brief construct_index_tablet_info
       * @param tablet
       * @return error code
       */
      int construct_index_tablet_info(ObTablet* tablet);
      /**
       * @brief update_meta_index: put tablet into image
       * @param tablet
       * @param sync_meta
       * @return error code
       */
      int update_meta_index(ObTablet* tablet,const bool sync_meta);
      /**
       * @brief check_tablet: check the tablet can really serve
       * @param tablet
       * @return error code
       */
      int check_tablet(const ObTablet* tablet);
      /**
       * @brief get_failed_fake_range
       * @param range
       * @return OB_INNER_STAT_ERROR or succ
       */
      int get_failed_fake_range(ObNewRange &range);


    public:
      /**
       * @brief set handle_range_
       * @param handle_range
       * @return error code
       */
      inline int set_handle_range(ObNewRange *handle_range);

    private:
      ObNewRange *handle_range_; ///< global stage handle range
      RangeServerHash* range_server_hash_; ///< pointer to data_multcs_range_hash_
      uint64_t table_id_; ///< the table id of the global range
      ObGetCellStreamWrapper ms_wrapper_; ///< wrap ms to communicate with other cs
      ObIndexInteractiveAgent interactive_agent_;///< Physical Operators
      ObIndexLocalAgent local_agent_;///< Physical Operators
      common::ObColumnChecksum cc; ///< column checksum
      common::ObColumnChecksum column_checksum_; ///< column_checksum_
      sql::ObSort sort_; ///< sort operator
    };

    inline int ObGlobalIndexHandler::set_handle_range(ObNewRange *handle_range)
    {
      int ret = OB_SUCCESS;
      handle_range_ = handle_range;
      if (handle_range_ == NULL)
      {
        TBSYS_LOG(ERROR,"failed to assign handle_range_!");
        ret = OB_ERROR;
      }
      return ret;
    }
  } /* namespace chunkserver */
} /* namespace oceanbase */

#endif /* CHUNKSERVER_OB_GLOBAL_INDEX_HANDLER_H_ */
