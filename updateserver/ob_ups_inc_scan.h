/**
* Copyright (C) 2013-2016 ECNU_DaSE.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_ups_inc_scan.h
* @brief for operations of update server increment scan
*
* modified by maoxiaoxiao:add functions to reset iterator
*
* modified by zhutao:add some context for procedure group execution
*
* @version __DaSE_VERSION
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @author zhutao <zhutao@stu.ecnu.edu.cn>
*
* @date 2016_07_30
*/

/**
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * Authors:
 *   yuanqi <yuanqi.xhf@taobao.com>
 *     - some work details if you want
 */
#ifndef __OB_UPDATESERVER_OB_INC_SCAN_IMPL_H__
#define __OB_UPDATESERVER_OB_INC_SCAN_IMPL_H__
#include "sql/ob_inc_scan.h"
#include "ob_table_list_query.h"
#include "ob_ups_utils.h"
#include "sql/ob_expr_values.h" //add by zt 20160113

namespace oceanbase
{
  namespace updateserver
  {
    class ObRowDescPrepare : public RowkeyInfoCache
    {
      public:
        ObRowDescPrepare() {}
        virtual ~ObRowDescPrepare() {}
      protected:
        int set_rowkey_size(ObUpsTableMgr* table_mgr, ObRowDesc* row_desc);
    };

    class ObIncGetIter:  public sql::ObPhyOperator, public ObTableListQuery, public ObRowDescPrepare
    {
      public:
        ObIncGetIter(): lock_flag_(sql::LF_NONE), get_param_(NULL), result_(NULL), last_cell_idx_(0)
      {}
      public:
        virtual void reset()
        {
          row_desc_.reset();
          lock_flag_ = sql::LF_NONE;
          get_param_ = NULL;
          result_ = NULL;
          last_cell_idx_ = 0;
          ObTableListQuery::reset();
        }
        virtual void reuse()
        {
          row_desc_.reset();
          lock_flag_ = sql::LF_NONE;
          get_param_ = NULL;
          result_ = NULL;
          last_cell_idx_ = 0;
          ObTableListQuery::reset();
        }
        int open(BaseSessionCtx* session_ctx, ObUpsTableMgr* table_mgr,
                 const common::ObGetParam* get_param, const sql::ObLockFlag lock_flag, sql::ObPhyOperator*& result);
        int open(){ return common::OB_SUCCESS; }
        int close();
        virtual sql::ObPhyOperatorType get_type() const { return sql::PHY_INC_GET_ITER; }
        int set_child(int32_t child_idx, sql::ObPhyOperator &child_operator)
        {
          int err = OB_NOT_IMPLEMENT;
          UNUSED(child_idx);
          UNUSED(child_operator);
          return err;
        }
        int get_next_row(const common::ObRow *&row);
        int get_row_desc(const common::ObRowDesc *&row_desc) const;
        int64_t to_string(char* buf, const int64_t buf_len) const { return snprintf(buf, buf_len, "%s", "inc_get_iter"); }
        //add maoxx
        /**
         * @brief reset_iterator
         */
        void reset_iterator() { last_cell_idx_ = 0;}
        //add e
      private:
        common::ObRowDesc row_desc_;
        sql::ObLockFlag lock_flag_;
        const common::ObGetParam* get_param_;
        ObPhyOperator* result_;
        int64_t last_cell_idx_;
        ObSingleTableGetQuery get_query_;
    };

    class ObIncScanIter:  public ObTableListQuery, public ObRowDescPrepare
    {
      public:
        int open(BaseSessionCtx* session_ctx, ObUpsTableMgr* table_mgr,
                 const common::ObScanParam* scan_param, sql::ObPhyOperator*& result);
        void reset()
        {
          row_desc_.reset();
          ObTableListQuery::reset();
        }
      private:
        common::ObRowDesc row_desc_;
        ObSingleTableScanQuery scan_query_;
    };

    class ObUpsIncScan: public sql::ObIncScan
    {
      public:
        ObUpsIncScan(BaseSessionCtx& session_ctx): session_ctx_(&session_ctx), result_(NULL)
        {}
        ObUpsIncScan(): session_ctx_(NULL), result_(NULL)
        {}
        void set_session_ctx(BaseSessionCtx *session_ctx) {session_ctx_ = session_ctx;}
        virtual ~ObUpsIncScan() {}
        void reset()
        {
          session_ctx_ = NULL;
          result_ = NULL;
          get_iter_.reset();
          scan_iter_.reset();

          input_values_.reset();
        }
      public:
        int open();
        int close();
        int64_t to_string(char* buf, const int64_t buf_len) const;
        //add maoxx
        /**
         * @brief reset_iterator
         */
        virtual void reset_iterator() { get_iter_.reset_iterator();}
        //add e
      public:
        int get_next_row(const common::ObRow *&row);
        int get_row_desc(const common::ObRowDesc *&row_desc) const;

        //add by zt 20160113:b
        /**
         * @brief deserialize
         * deserialize object
         * @param buf
         * @param data_len
         * @param pos
         * @return error code
         */
        int deserialize(const char *buf, const int64_t data_len, int64_t &pos);
        //add by zt 20160113:e

      protected:
        virtual ObUpsTableMgr* get_table_mgr(); // for test

        //add by zt 20160113:b
      private:
        /**
         * @brief prepare_data
         * prepare data for group execution
         * @return error code
         */
        int prepare_data();
        //add by zt 20160113:e
      private:
        BaseSessionCtx *session_ctx_;
        sql::ObPhyOperator* result_;
        ObIncGetIter get_iter_;
        ObIncScanIter scan_iter_;

        //add by zt 20160113
        sql::ObExprValues input_values_;  ///<  input variable values
        bool group_exec_mode_;  ///< group exection flag
    };
  } // end namespace updateserver
} // end namespace oceanbase

#endif /* __OB_UPDATESERVER_OB_INC_SCAN_IMPL_H__ */
