/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_table_rpc_scan.h
 * @brief table rpc scan operator
 *
 * modified by longfei：
 * add member variables and member function for using index in select
 * modified by Qiushi FAN: add some functions to insert a new expression to scan operator.
 * modified by zhutao: add a get_row_desc_template function
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author Qiushi FAN <qsfan@ecnu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 *
 * @date 2016_07_30
 */

/** * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_table_rpc_scan.h
 *
 * Authors:
 *   Yu Huang <xiaochu.yh@taobao.com>
 *
 */
#ifndef _OB_TABLE_RPC_SCAN_H
#define _OB_TABLE_RPC_SCAN_H 1
#include "ob_table_scan.h"
#include "ob_rpc_scan.h"
#include "ob_sql_expression.h"
#include "ob_table_rename.h"
#include "ob_project.h"
#include "ob_filter.h"
#include "ob_scalar_aggregate.h"
#include "ob_merge_groupby.h"
#include "ob_sort.h"
#include "ob_limit.h"
#include "ob_empty_row_filter.h"
#include "ob_sql_context.h"
#include "common/ob_row.h"
#include "common/ob_hint.h"

namespace oceanbase
{
  namespace sql
  {
    class ObTableRpcScan: public ObTableScan
    {
      public:
        ObTableRpcScan();
        virtual ~ObTableRpcScan();
        virtual void reset();
        virtual void reuse();
        virtual int open();
        virtual int close();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        /**
         * @brief get_row_desc_template
         * get row descriptor
         * @param row_desc returned row descriptor point
         * @return error code
         */
        int get_row_desc_template(const common::ObRowDesc *&row_desc) const; //add by zt 20160114

        virtual ObPhyOperatorType get_type() const;

        int init(ObSqlContext *context, const common::ObRpcScanHint *hint = NULL);

        //add longfei [secondary index select] 20151116 :b
        /**
         * @brief set_main_tid 存原表的tid
         * @param tid
         */
        void set_main_tid(uint64_t tid);
        /**
         * @brief set_is_index_for_storing 设置这次查询是否用到不回表的索引
         * @param is_use
         * @param tid
         */
        void set_is_index_for_storing(bool is_use,uint64_t tid);
        /**
         * @brief set_is_index_without_storing 设置这次查询是否用到回表的索引
         * @param is_use
         * @param tid
         */
        void set_is_index_without_storing(bool is_use,uint64_t tid);
        /**
         * @brief set_is_use_index_without_storing 置位
         */
        void set_is_use_index_without_storing();
        /**
         * @brief set_is_use_index_for_storing 置位 && 设置行描述
         * @param tid
         * @param row_desc
         */
        void set_is_use_index_for_storing(uint64_t tid ,ObRowDesc &row_desc);
        /**
         * @brief add_main_output_column 输出列
         * @param expr
         * @return err code
         */
        int add_main_output_column(const ObSqlExpression& expr);
        /**
         * @brief add_main_filter select语句中的过滤列
         * @param expr
         * @return err code
         */
        int add_main_filter(ObSqlExpression* expr);
        /**
         * @brief add_index_filter 使用索引时的过滤列
         * @param expr
         * @return err code
         */
        int add_index_filter(ObSqlExpression* expr);
        /**
         * @brief set_main_rowkey_info
         * @param rowkey_info
         */
        void set_main_rowkey_info(const ObRowkeyInfo &rowkey_info);
        /**
         * @brief cons_second_row_desc
         * @param row_desc
         * @return err code
         */
        int cons_second_row_desc(ObRowDesc &row_desc);
        /**
         * @brief set_second_row_desc
         * @param row_desc
         * @return err code
         */
        int set_second_row_desc(ObRowDesc *row_desc);
        //add:e
        
        // add by lxb [hash join single] 20170411
        int get_is_index_without_storing(bool &is_use, uint64_t &tid);

        /**
         * 添加一个需输出的column
         *
         * @note 只有通过复合列结算新生成的列才需要new_column_id
         * @param expr [in] 需输出的列（这个列可能是个复合列的结果）
         *
         * @return OB_SUCCESS或错误码
         */
        int add_output_column(const ObSqlExpression& expr);

        /**
         * 设置table_id
         * @note 只有基本表被重命名的情况才会使两个不相同id，其实两者相同时base_table_id可以给个默认值。
         * @param table_id [in] 输出的table_id
         * @param base_table_id [in] 被访问表的id
         *
         * @return OB_SUCCESS或错误码
         */
        int set_table(const uint64_t table_id, const uint64_t base_table_id);
        /**
         * 添加一个filter
         *
         * @param expr [in] 过滤表达式
         *
         * @return OB_SUCCESS或错误码
         */
        int add_filter(ObSqlExpression *expr);
        //add fanqiushi [semi_join] [0.1] 20150910:b
        /**
        * @brief insert a expression to a scan operator.
        * @param an expression.
        * @param void.
        * @return Error code.
        */
        int add_filter_set_for_semijoin(ObSqlExpression *expr);
        //add:e
        int add_group_column(const uint64_t tid, const uint64_t cid);
        int add_aggr_column(const ObSqlExpression& expr);

        /**
         * 指定limit/offset
         *
         * @param limit [in]
         * @param offset [in]
         *
         * @return OB_SUCCESS或错误码
         */
        int set_limit(const ObSqlExpression& limit, const ObSqlExpression& offset);
        int64_t to_string(char* buf, const int64_t buf_len) const;
        void set_phy_plan(ObPhysicalPlan *the_plan);
        int32_t get_child_num() const;

        void set_rowkey_cell_count(const int64_t rowkey_cell_count)
        {
          rpc_scan_.set_rowkey_cell_count(rowkey_cell_count);
        }

        inline void set_need_cache_frozen_data(bool need_cache_frozen_data)
        {
          rpc_scan_.set_need_cache_frozen_data(need_cache_frozen_data);
        }
        inline void set_cache_bloom_filter(bool cache_bloom_filter)
        {
          rpc_scan_.set_cache_bloom_filter(cache_bloom_filter);
        }

        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
      private:
        // disallow copy
        ObTableRpcScan(const ObTableRpcScan &other);
        ObTableRpcScan& operator=(const ObTableRpcScan &other);
      private:
        // data members
        ObRpcScan rpc_scan_;
        ObFilter select_get_filter_;
        ObScalarAggregate *scalar_agg_; // very big
        ObMergeGroupBy *group_; // very big
        ObSort group_columns_sort_;
        ObLimit limit_;
        ObEmptyRowFilter empty_row_filter_;
        bool has_rpc_;
        bool has_scalar_agg_;
        bool has_group_;
        bool has_group_columns_sort_;
        bool has_limit_;
        bool is_skip_empty_row_;
        int32_t read_method_;
        //add longfei
        bool is_use_index_rpc_scan_;  ///< 判断是否使用了回表的索引
        bool is_use_index_for_storing_; ///< 判断是否使用了不回表的索引
        //ObRpcScan index_rpc_scan_;
        ObProject main_project_;   ///< 存第二次get原表时的输出列
        ObFilter main_filter_;     ///< 存第二次get原表时filter
        uint64_t main_tid_;         ///< 原表的tid
        bool is_use_index_for_storing_for_tostring_; ///< 不回表物理计划标志位
        uint64_t index_tid_for_storing_for_tostring_; ///< 不回表使用索引tid
        bool is_use_index_without_storing_for_tostring_; ///< 回表物理计划标志位
        uint64_t index_tid_without_storing_for_tostring_; ///< 回表使用索引tid
        //add:e
    };
    inline void ObTableRpcScan::set_phy_plan(ObPhysicalPlan *the_plan)
    {
      ObPhyOperator::set_phy_plan(the_plan);
      rpc_scan_.set_phy_plan(the_plan);
      select_get_filter_.set_phy_plan(the_plan);
      group_columns_sort_.set_phy_plan(the_plan);
      limit_.set_phy_plan(the_plan);
      limit_.set_phy_plan(the_plan);
    }
    inline int32_t ObTableRpcScan::get_child_num() const
    {
      return 0;
    }

    //add by zt 20160114:b
    inline int ObTableRpcScan::get_row_desc_template(const ObRowDesc *&row_desc) const
    {
      return rpc_scan_.get_row_desc(row_desc);
    }
    //add by zt 20160114:e
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_TABLE_RPC_SCAN_H */
