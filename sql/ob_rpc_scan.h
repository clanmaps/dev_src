/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_rpc_scan.h
 * @brief 用于MS进行全表扫描
 *
 * modified by longfei：add some member variables and some rule for Query Optimization
 *
 * modify by guojinwei, bingo: support REPEATABLE-READ isolation
 * add header files
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 *         guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         bingo <bingxiao@stu.ecnu.edu.cn>
 * @date 2016_01_22
 *       2016_06_16
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
 * ob_rpc_scan.h
 *
 * Authors:
 *   Yu Huang <xiaochu.yh@taobao.com>
 *
 */
#ifndef _OB_RPC_SCAN_H
#define _OB_RPC_SCAN_H 1
#include "ob_phy_operator.h"
#include "ob_sql_expression.h"
#include "common/ob_row.h"
#include "common/ob_hint.h"
#include "common/ob_schema.h"
#include "common/location/ob_tablet_location_cache_proxy.h"
#include "common/ob_string_buf.h"
#include "sql/ob_sql_scan_param.h"
#include "sql/ob_sql_get_param.h"
#include "sql/ob_sql_context.h"
#include "mergeserver/ob_ms_scan_param.h"
#include "mergeserver/ob_ms_sql_scan_request.h"
#include "mergeserver/ob_ms_sql_get_request.h"
#include "mergeserver/ob_ms_rpc_proxy.h"
#include "mergeserver/ob_rs_rpc_proxy.h"
#include "mergeserver/ob_merge_server_service.h"
#include "mergeserver/ob_frozen_data_cache.h"
#include "sql/ob_sql_read_strategy.h"
#include "mergeserver/ob_insert_cache.h"
// add by guojinwei [repeatable read] 20160310:b
#include "common/ob_transaction.h"
// add:e


namespace oceanbase
{
  namespace sql
  {
    // 用于MS进行全表扫描
    class ObRpcScan : public ObPhyOperator
    {
      public:
        ObRpcScan();
        virtual ~ObRpcScan();
        virtual void reset();
        virtual void reuse();
        int set_child(int32_t child_idx, ObPhyOperator &child_operator)
        {
          UNUSED(child_idx);
          UNUSED(child_operator);
          return OB_ERROR;
        }
        int init(ObSqlContext *context, const common::ObRpcScanHint *hint = NULL);
        virtual int open();
        virtual int close();
        virtual ObPhyOperatorType get_type() const { return PHY_RPC_SCAN; }
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;

        //add longfei
        /**
         * @brief set_main_tid: 设置原表tid
         * @param main_tid
         * @return OB_SUCCESS
         */
        int set_main_tid(uint64_t main_tid);
        /**
         * @brief set_is_use_index_for_storing: 设置行描述
         * @param main_tid
         * @param [in] row_desc
         * @return
         */
        int set_is_use_index_for_storing(uint64_t main_tid,common::ObRowDesc &row_desc);
        /**
         * @brief get_next_compact_row_for_index 第一次scan索引表，返回索引表的数据
         * @param row
         * @return error code
         */
        int get_next_compact_row_for_index(const common::ObRow *&row);
        /**
         * @brief set_main_rowkey_info
         * @param rowkey_info
         * @return err code
         */
        int set_main_rowkey_info(const ObRowkeyInfo& rowkey_info);
        /**
         * @brief add_main_output_column: 第二次get原表时的输出列
         * @param expr
         * @return err code
         */
        int add_main_output_column(const ObSqlExpression& expr);
        /**
         * @brief add_main_filter: 第二次get原表时用做过滤
         * @param expr
         * @return err code
         */
        int add_main_filter(ObSqlExpression* expr);
        /**
         * @brief get_other_row_desc: 获得第二次get时的行描述
         * @param row_desc
         * @return err code
         */
        int get_other_row_desc(const common::ObRowDesc *&row_desc);
        /**
         * @brief set_second_rowdesc: 回表时，第二次get原表时用到的行描述
         * @param row_desc
         * @return err code
         */
        int set_second_rowdesc(common::ObRowDesc *row_desc);
        /**
         * @brief fill_read_param_for_first_scan 设置第一次读表参数
         * @param dest_param
         * @return err code
         */
        int fill_read_param_for_first_scan(ObSqlReadParam &dest_param);
        //add:e

        /**
         * 添加一个需输出的column
         *
         * @note 只有通过复合列结算新生成的列才需要new_column_id
         * @param expr [in] 需输出的列（这个列可能是个复合列的结果）
         *
         * @return OB_SUCCESS或错误码
         *
         * NOTE: 如果传入的expr是一个条件表达式，本函数将不做检查，需要调用者保证
         */
        int add_output_column(const ObSqlExpression& expr);

        /**
         * 设置table_id
         * @param table_id [in] 被访问表的id
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
        int add_filter(ObSqlExpression* expr);

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

        //void set_data_version(int64_t data_version);
        int set_scan_range(const ObNewRange &range)
        {
          UNUSED(range);
          return OB_ERROR;
        }

        void set_rowkey_cell_count(const int64_t rowkey_cell_count)
        {
          cur_row_desc_.set_rowkey_cell_count(rowkey_cell_count);
        }
        inline void set_need_cache_frozen_data(bool need_cache_frozen_data)
        {
          need_cache_frozen_data_ = need_cache_frozen_data;
        }
        inline void set_cache_bloom_filter(bool cache_bloom_filter)
        {
          cache_bloom_filter_ = cache_bloom_filter;
        }

        int64_t to_string(char* buf, const int64_t buf_len) const;

        DECLARE_PHY_OPERATOR_ASSIGN;
      private:
        // disallow copy
        ObRpcScan(const ObRpcScan &other);
        ObRpcScan& operator=(const ObRpcScan &other);

        // member method
        void destroy();
        int cast_range(ObNewRange &range);
        int create_scan_param(ObSqlScanParam &scan_param);
        int get_next_compact_row(const common::ObRow*& row);
        int cons_scan_range(ObNewRange &range);
        int cons_row_desc(const ObSqlGetParam &sql_get_param, ObRowDesc &row_desc);
        int fill_read_param(ObSqlReadParam &dest_param);
        int get_min_max_rowkey(const ObArray<ObRowkey> &rowkey_array, ObObj *start_key_objs_, ObObj *end_key_objs_,int64_t rowkey_size);

        int create_get_param(ObSqlGetParam &get_param);
        int cons_get_rows(ObSqlGetParam &get_param);
        void set_hint(const common::ObRpcScanHint &hint);
        void cleanup_request();

        //add longfei
        /**
         * @brief create_get_param_for_index: 生成第二次get原表时的get_param
         * @param get_param
         * @return error code
         */
        int create_get_param_for_index(ObSqlGetParam &get_param);
        /**
         * @brief fill_read_param_for_index
         * @param dest_param
         * @return err code
         */
        int fill_read_param_for_index(ObSqlReadParam &dest_param);
        /**
         * @brief cons_get_rows_for_index: 根据第一次scan索引表返回的数据，构造第二次get原表的主键的范围
         * @param get_param
         * @return error code
         */
        int cons_get_rows_for_index(ObSqlGetParam &get_param);
        /**
         * @brief reset_read_param_for_index 重置一下read_param
         * @return error code
         */
        int reset_read_param_for_index();
        /**
         * @brief cons_index_row_desc: 生成第二次get原表时用到的行描述
         * @param sql_get_param
         * @param row_desc
         * @return err code
         */
        int cons_index_row_desc(const ObSqlGetParam &sql_get_param, ObRowDesc &row_desc);
        //add:e

      private:
        static const int64_t REQUEST_EVENT_QUEUE_SIZE = 8192;
        // 等待结果返回的超时时间
        int64_t timeout_us_;
        mergeserver::ObMsSqlScanRequest *sql_scan_request_;
        mergeserver::ObMsSqlGetRequest *sql_get_request_;

        //add longfei
        //mergeserver::ObMsSqlGetRequest *index_sql_get_request_;
        bool is_use_index_;         ///< 是否使用回表的索引
        bool is_use_index_for_storing_;  ///< 是否使用不回表的索引
        uint64_t main_table_id_;    ///< 主表的tid
        int64_t get_next_row_count_;    ///< 判断是否是第一次调用get_next_row
        ObProject main_project;     ///< 第二次get原表时的输出列
        ObFilter main_filter_;      ///< where条件中如果有不在索引表中的列的表达式，就把它存到这里，第二次get原表时用做过滤。
        common::ObRowkeyInfo main_rowkey_info_;    ///< 主表的主键信息
        common::ObRowDesc second_row_desc_;  ///< 回表时，第二次get原表时用到的行描述
        //add:e

        ObSqlScanParam *scan_param_;
        ObSqlGetParam *get_param_;
        ObSqlReadParam *read_param_;
        bool is_scan_;
        bool is_get_;
        ObRowDesc get_row_desc_;
        common::ObRowkeyInfo rowkey_info_;
        common::ObTabletLocationCacheProxy * cache_proxy_;
        mergeserver::ObMergerAsyncRpcStub * async_rpc_;
        ObSQLSessionInfo *session_info_;
        const oceanbase::mergeserver::ObMergeServerService *merge_service_;
        common::ObRpcScanHint hint_;
        common::ObObj start_key_objs_[OB_MAX_ROWKEY_COLUMN_NUMBER];
        common::ObObj end_key_objs_[OB_MAX_ROWKEY_COLUMN_NUMBER];
        common::ObRow cur_row_;

        //add longfei
        common::ObRow cur_row_for_storing_;  ///< 在不回表的查询中，需要对CS返回的数据修改tid，修改后的行存到cur_row_for_storing_里面
        common::ObRowDesc cur_row_desc_for_storing; ///< cur_row_for_storing_行描述
        //add:e

        common::ObRowDesc cur_row_desc_;
        uint64_t table_id_;
        uint64_t base_table_id_;
        char* start_key_buf_;
        char* end_key_buf_;
        ObSqlReadStrategy sql_read_strategy_;
        ObSEArray<ObRowkey, OB_PREALLOCATED_NUM> get_rowkey_array_;
        bool is_rpc_failed_;
        bool need_cache_frozen_data_;
        mergeserver::ObFrozenDataKey    frozen_data_key_;
        mergeserver::ObFrozenDataKeyBuf frozen_data_key_buf_;
        mergeserver::ObFrozenData       frozen_data_;
        mergeserver::ObCachedFrozenData cached_frozen_data_;
        bool cache_bloom_filter_;
        bool rowkey_not_exists_;
        ObObj obj_row_not_exists_;
        ObRow row_not_exists_;
        int insert_cache_iter_counter_;
        mergeserver::InsertCacheWrapValue value_;
        common::ObStringBuf tablet_location_list_buf_;
        //ObSEArray<bool, OB_PREALLOCATED_NUM> rowkeys_exists_;
        bool insert_cache_need_revert_;
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_TABLE_RPC_SCAN_H */
