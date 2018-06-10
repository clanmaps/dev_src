/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cs_interactive_scan.h
 * @brief father operator of ObIndexLocalAgent and ObIndexInteractiveAgent
 *
 * Created by longfei：scan agent for construct static index
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_07
 */

#ifndef CHUNKSERVER_OB_CS_INTERACTIVE_SCAN_H_
#define CHUNKSERVER_OB_CS_INTERACTIVE_SCAN_H_

#include "sql/ob_double_children_phy_operator.h"

using namespace oceanbase::sql;
namespace oceanbase
{
  namespace chunkserver
  {
    /**
     * @brief The ObCsInteractiveScan class
     * is designed for scan operator to get data in a special range
     */
    class ObCsInteractiveScan: public ObDoubleChildrenPhyOperator
    {
    public:
      /**
       * @brief ObCsInteractiveScan constructor
       */
      ObCsInteractiveScan();
      /**
       * @brief ~ObCsInteractiveScan destructor
       */
      virtual ~ObCsInteractiveScan();
    public:
      /**
       * @brief get_next_row
       * @param row
       * @return err code
       */
      virtual int get_next_row(const common::ObRow *&row);
      /**
       * @brief get_row_desc Not implemented
       * @param row_desc
       * @return error code
       */
      virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
      /**
       * @brief to_string Not implemented
       * @param buf
       * @param buf_len
       * @return err code
       */
      virtual int64_t to_string(char *buf,const int64_t buf_len) const;

    private:
      bool local_idx_scan_finish_; ///< to distinguish left and right child
    };

  } /* namespace chunkserver */
} /* namespace oceanbase */

#endif /* CHUNKSERVER_OB_CS_INTERACTIVE_SCAN_H_ */
