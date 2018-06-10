/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_secondary_index_service_impl.h
 * @brief for using secondary index in select statement
 *
 * Created by longfei：implementation of secondary index service
 * maybe more function?
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_29
 */

#ifndef COMMON_OB_SECONDARY_INDEX_SERVICE_IMPL_H_
#define COMMON_OB_SECONDARY_INDEX_SERVICE_IMPL_H_

#include "ob_secondary_index_service.h"

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief The ObSecondaryIndexServiceImpl class
     * ObSecondaryIndexServiceImpl is implatation of ObSecondaryIndexService
     */
    class ObSecondaryIndexServiceImpl : public ObSecondaryIndexService
    {
      //del:huangjianwei [secondary index maintain] 20161219:b
      // this type copy from "ob_postfix_expression.h"
      /*enum ObPostExprNodeType {
        BEGIN_TYPE = 0,
        COLUMN_IDX,
        CONST_OBJ,
        PARAM_IDX,
        SYSTEM_VAR,
        TEMP_VAR,
        OP,
        CUR_TIME_OP,
        UPS_TIME_OP,
        END,  postfix expression terminator
        END_TYPE
      };*/
      //del:e
    public:
      /**
       * @brief init
       * @param schema_manager_
       * @return err code
       */
      virtual int init(const ObSchemaManagerV2* schema_manager_);
      /**
       * @brief ObSecondaryIndexServiceImpl constructor
       */
      ObSecondaryIndexServiceImpl();
      /**
       * @brief getSchemaManager
       * @return schema_manager_
       */
      const ObSchemaManagerV2* getSchemaManager() const;
      /**
       * @brief setSchemaManager
       * @param schemaManager
       */
      void setSchemaManager(const ObSchemaManagerV2* schemaManager);
    public:
      /**
       * @brief find_cid: 获得表达式中列的cid
       * @param sql_expr
       * @param cid
       * @return err code
       */
      virtual int find_cid(
          sql::ObSqlExpression& sql_expr,
          uint64_t &cid);
      /**
       * @brief change_tid: 获得表达式中记录列的tid的ObObj在ObObj数组里的下标
       * @param sql_expr
       * @param array_index
       * @return err code
       */
      virtual int change_tid(
          sql::ObSqlExpression* sql_expr,
          uint64_t& array_index);
      /**
       * @brief get_cid: 获得表达式中列的cid，如果表达式中有多个列，则报错
       * @param sql_expr
       * @param cid
       * @return err code
       */
      virtual int get_cid(
          sql::ObSqlExpression* sql_expr,
          uint64_t& cid);
      /**
       * @brief is_have_main_cid: 如果表达式中有主表的第一主键，或者表达式中有超过两列的，返回true
       * @param sql_expr
       * @param main_column_id
       * @return err code
       */
      virtual bool is_have_main_cid(
          sql::ObSqlExpression& sql_expr,
          uint64_t main_column_id);
      /**
       * @brief is_all_expr_cid_in_indextable
       * @param index_tid
       * @param expr_
       * @param sm_v2
       * @return err code
       */
      virtual bool is_all_expr_cid_in_indextable(
          uint64_t index_tid,
          const sql::ObPostfixExpression& expr_,
          const ObSchemaManagerV2 *sm_v2);
      /**
       * @brief get_all_cloumn: 获得表达式中所有列的存tid的ObObj在ObObj数组里的下标
       * @param sql_expr
       * @param column_index
       * @return err code
       */
      virtual int get_all_cloumn(
          sql::ObSqlExpression& sql_expr,
          ObArray<uint64_t> &column_index);
      /**
       * @brief is_this_expr_can_use_index: 判断该表达式是否能够使用索引。
       * 如果该表达式只有一列，并且是个等值或in表达式，并且该表达式的列的cid是主表main_tid的某一张索引表的第一主键，则该表达式能够使用索引
       * @param sql_expr
       * @param index_tid
       * @param main_tid
       * @param sm_v2
       * @return err code
       */
      virtual bool is_this_expr_can_use_index(
          sql::ObSqlExpression& sql_expr,
          uint64_t &index_tid,
          uint64_t main_tid,
          const ObSchemaManagerV2 *sm_v2);
    public:
      /**
       * @brief is_this_table_avalibale: 判断tid为参数的表是否是可用的索引表
       * @param tid
       * @return err code
       */
      virtual bool is_this_table_avalibale(uint64_t tid);
      /**
       * @brief is_index_table_has_all_cid_V2: 判断索引表是否包含sql语句中出现的所有列
       * @param index_tid
       * @param filter_array
       * @param project_array
       * @return err code
       */
      virtual bool is_index_table_has_all_cid_V2(
          uint64_t index_tid,
          Expr_Array *filter_array,
          Expr_Array *project_array);
      /**
       * @brief is_cid_in_index_table: 判断该列是否在该索引表中。
       * @param cid
       * @param tid
       * @return 结果是0，表示不在；结果是1，表示该列是索引表的主键；结果是2，表示该列是索引表的非主键
       */
      virtual int64_t is_cid_in_index_table(
          uint64_t cid,
          uint64_t tid);
      /**
       * @brief is_expr_can_use_storing_V2
       * @param c_filter
       * @param mian_tid
       * @param index_tid
       * @param filter_array
       * @param project_array
       * @return 当返回值为true时，index_tid赋值为能用于query的索引表的tid
       */
      virtual bool is_expr_can_use_storing_V2(
          sql::ObSqlExpression c_filter,
          uint64_t mian_tid,
          uint64_t &index_tid,
          Expr_Array * filter_array,
          Expr_Array *project_array);
      /**
       * @brief is_wherecondition_have_main_cid_V2
       * @param filter_array
       * @param main_cid
       * @return 如果where条件的某个表达式有main_cid或where条件中某个表达式有多个列,返回true
       */
      virtual bool is_wherecondition_have_main_cid_V2(
          Expr_Array *filter_array,
          uint64_t main_cid);
      /**
       * @brief if_rowkey_in_expr
       * @param filter_array
       * @param main_tid
       * @return err code
       */
      virtual bool if_rowkey_in_expr(
          Expr_Array *filter_array,
          uint64_t main_tid);
      /**
       * @brief decide_is_use_storing_or_not_V2
       * @param filter_array
       * @param project_array
       * @param index_table_id
       * @param main_tid
       * @return 当返回值为true时，uint64_t &index_table_id 设置为被选中索引表的tid
       */
      virtual bool decide_is_use_storing_or_not_V2(
          Expr_Array *filter_array,
          Expr_Array *project_array,
          uint64_t &index_table_id,
          uint64_t main_tid);
      /**
       * @brief is_can_use_hint_for_storing_V2
       * @param filter_array
       * @param project_array
       * @param index_table_id
       * @return err code
       */
      virtual bool is_can_use_hint_for_storing_V2(
          Expr_Array *filter_array,
          Expr_Array *project_array,
          uint64_t index_table_id);
      /**
       * @brief is_can_use_hint_index_V2
       * @param filter_array
       * @param index_table_id
       * @return err code
       */
      virtual bool is_can_use_hint_index_V2(
          Expr_Array *filter_array,
          uint64_t index_table_id);
    private:
      DISALLOW_COPY_AND_ASSIGN(ObSecondaryIndexServiceImpl);
    private:
      const ObSchemaManagerV2* schema_manager_; ///< index service need schema manager to get some information about index table
    };
  }
}

#endif /* COMMON_OB_SECONDARY_INDEX_SERVICE_IMPL_H_ */
