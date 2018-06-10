/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_drop_index.h
 * @brief for physical plan of drop index
 *
 * Created by longfei：drop index physical operator
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_26
 */

#ifndef SQL_OB_DROP_INDEX_H_
#define SQL_OB_DROP_INDEX_H_

#include "sql/ob_no_children_phy_operator.h"
#include "common/ob_strings.h"
#include "sql/ob_drop_table.h"
namespace oceanbase
{
  namespace mergeserver
  {
    class ObMergerRootRpcProxy;
  } // end namespace mergeserver

  namespace sql
  {
    /**
     * @brief The ObDropIndex class
     * ObDropIndex is designed for drop index physical plan
     */
    class ObDropIndex: public ObDropTable
    {
    public:
      /**
       * @brief ObDropIndex: constructor
       */
      ObDropIndex();
      /**
       * @brief ~ObDropIndex: destructor
       */
      virtual ~ObDropIndex();
      /**
       * @brief add_index_name into indexs_
       * @param tname
       * @return
       */
      int add_index_name (const common::ObString &tname);

      /**
       * @brief execute the drop index statement
       * @return
       */
      virtual int open();
      /**
       * @brief close the operator
       * @return
       */
      virtual int close();
      /**
       * @brief get_type
       * @return
       */
      virtual ObPhyOperatorType get_type() const { return PHY_DROP_INDEX; }
      /**
       * @brief to_string
       * @param buf
       * @param buf_len
       * @return
       */
      virtual int64_t to_string(char* buf, const int64_t buf_len) const;

    private:
      common::ObStrings indexs_; ///< store the index we will drops
    };

  } // end namespace sql
} // end namespace oceanbase



#endif /* SQL_OB_DROP_INDEX_H_ */
