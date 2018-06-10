/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_drop_table.h
 * @brief physical operator for drop table
 *
 * modified by longfei：if this table has index, drop table will drop index first
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_22
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_drop_table.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_DROP_TABLE_H
#define _OB_DROP_TABLE_H 1
#include "sql/ob_no_children_phy_operator.h"
#include "common/ob_strings.h"
namespace oceanbase
{
  namespace mergeserver
  {
    class ObMergerRootRpcProxy;
  } // end namespace mergeserver

  namespace sql
  {
    class ObDropTable: public ObNoChildrenPhyOperator
    {
      public:
        ObDropTable();
        virtual ~ObDropTable();
        virtual void reset();
        virtual void reuse();
        // init
        void set_rpc_stub(mergeserver::ObMergerRootRpcProxy* rpc);

        void set_if_exists(bool if_exists);
        int add_table_name(const common::ObString &tname);
        // longfei [drop index]
        /**
         * @brief get_if_exists
         * @return if_exists_
         */
        const bool get_if_exists() const;

        mergeserver::ObMergerRootRpcProxy* get_rpc_stub() const;

        /// execute the insert statement
        virtual int open();
        virtual int close();
        virtual ObPhyOperatorType get_type() const { return PHY_DROP_TABLE; }
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;

        /// @note always return OB_ITER_END
        virtual int get_next_row(const common::ObRow *&row);
        /// @note always return OB_NOT_SUPPORTED
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;

        // longfei [drop index]
        /**
         * @brief isHasIndexs
         * @return all_indexs_
         */
        bool isHasIndexs() const;
        /**
         * @brief setHasIndexs
         * @param hasIndexs
         */
        void setHasIndexs(bool hasIndexs);
        /**
         * @brief is_all_indexs_empty all_indexs_ empty?
         * @return true or false
         */
        bool is_all_indexs_empty() const;
        /**
         * @brief add_all_indexs add idxname into all_indexs_
         * @param idxname
         * @return error code
         */
        int add_all_indexs(const common::ObString &idxname);


      private:
        // types and constants
      private:
        // disallow copy
        ObDropTable(const ObDropTable &other);
        ObDropTable& operator=(const ObDropTable &other);
        // function members
      private:
        // data members
        bool if_exists_;
        common::ObStrings tables_;
        mergeserver::ObMergerRootRpcProxy* rpc_;

        // add longfei [drop index] 20151028
        bool has_indexs_; ///< table has index?
        common::ObStrings all_indexs_; ///< store all indexs on all tables
    };

    inline int ObDropTable::get_next_row(const common::ObRow *&row)
    {
      row = NULL;
      return common::OB_ITER_END;
    }

    inline int ObDropTable::get_row_desc(const common::ObRowDesc *&row_desc) const
    {
      row_desc = NULL;
      return common::OB_NOT_SUPPORTED;
    }

    inline void ObDropTable::set_rpc_stub(mergeserver::ObMergerRootRpcProxy* rpc)
    {
      rpc_ = rpc;
    }

    inline bool ObDropTable::isHasIndexs() const
    {
      return has_indexs_;
    }

    inline void ObDropTable::setHasIndexs(bool hasIndexs)
    {
      has_indexs_ = hasIndexs;
    }

  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_DROP_TABLE_H */
