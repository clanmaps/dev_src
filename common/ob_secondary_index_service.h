/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_secondary_index_service.h
 * @brief interface of determination rule for using secondary index in select
 *
 * Created by longfei：interface of determination rule for using secondary index in select,
 * maybe more in the future
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_29
 */

#ifndef COMMON_OB_SECONDRY_INDEX_SERVICE_H_
#define COMMON_OB_SECONDRY_INDEX_SERVICE_H_

#include "ob_define.h"
#include "ob_string.h"
#include "ob_array.h"
#include "sql/ob_sql_expression.h"
#include "page_arena.h"
#include "ob_se_array.h"

namespace oceanbase
{
  namespace common
  {
    static const int64_t EXPR_COUNT = 64;
    typedef ObSEArray<ObObj, EXPR_COUNT> ExprArray;
    typedef common::ObArray<sql::ObSqlExpression>  Expr_Array;///< storing filter column && project column
    /**
     * @brief The ObSecondaryIndexService class
     * interface of determination rule for using secondary index in selects
     */
    class ObSecondaryIndexService
    {
      public:
        virtual int init(const ObSchemaManagerV2* schema_manager_) = 0;
        virtual ~ObSecondaryIndexService() {}
        virtual bool is_this_table_avalibale(uint64_t tid) = 0;
        virtual bool is_index_table_has_all_cid_V2(uint64_t index_tid, Expr_Array *filter_array, Expr_Array *project_array) = 0;
        virtual int64_t is_cid_in_index_table(uint64_t cid, uint64_t tid) = 0;
        virtual bool is_expr_can_use_storing_V2(sql::ObSqlExpression c_filter,
                                                uint64_t mian_tid, uint64_t &index_tid, Expr_Array * filter_array,
                                                Expr_Array *project_array) = 0;
        virtual bool is_wherecondition_have_main_cid_V2(Expr_Array *filter_array, uint64_t main_cid) = 0;
        virtual bool if_rowkey_in_expr(Expr_Array *filter_array, uint64_t main_tid) = 0;
        virtual bool decide_is_use_storing_or_not_V2(Expr_Array *filter_array,
                                                     Expr_Array *project_array, uint64_t &index_table_id, uint64_t main_tid) = 0;
        virtual bool is_can_use_hint_for_storing_V2(Expr_Array *filter_array,
                                                    Expr_Array *project_array, uint64_t index_table_id) = 0;
        virtual bool is_can_use_hint_index_V2(Expr_Array *filter_ayyay, uint64_t index_table_id) = 0;
      public:
        virtual int find_cid(sql::ObSqlExpression& sql_expr, uint64_t &cid) = 0;
        virtual int change_tid(sql::ObSqlExpression* sql_expr, uint64_t& array_index) = 0;
        virtual int get_cid(sql::ObSqlExpression* sql_expr, uint64_t& cid) = 0;
        virtual bool is_have_main_cid(sql::ObSqlExpression& sql_expr, uint64_t main_column_id) = 0;
        virtual bool is_all_expr_cid_in_indextable(uint64_t index_tid, const sql::ObPostfixExpression& expr_, const ObSchemaManagerV2 *sm_v2) = 0;
        virtual int get_all_cloumn(sql::ObSqlExpression& sql_expr,ObArray<uint64_t> &column_index) = 0;
        virtual bool is_this_expr_can_use_index(sql::ObSqlExpression& sql_expr, uint64_t &index_tid,uint64_t main_tid,const ObSchemaManagerV2 *sm_v2) = 0;
    };
  }
}

#endif /* COMMON_OB_SECONDRY_INDEX_SERVICE_H_ */
