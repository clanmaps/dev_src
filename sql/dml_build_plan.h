/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file dml_build_plan.h
 * @brief resolve some dml operation
 *
 * modified by longfei：
 * 1.generate inner index table name
 * 2.resolve user's hint for using secondary index in select
 *
 * modified by zhutao: add a resolve_array_expr function
 * @version __DaSE_VERSION
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_24
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
 * dml_build_plan.h
 *
 * Authors:
 *   Guibin Du <tianguan.dgb@taobao.com>
 *
 */

#ifndef DML_BUILD_PLAN_H_
#define DML_BUILD_PLAN_H_

#include "parse_node.h"
#include "parse_malloc.h"
#include "sql/ob_logical_plan.h"
#include "common/ob_vector.h"
#include <stdint.h>

//add wenghaixing [secondary index create fix]20141226
#include "common/ob_strings.h"
//add e
/*
  * Expressions from different scope have different limitations,
  * we need a flage to distinguish where they are from.
  */
#define T_NONE_LIMIT      0
#define T_WHERE_LIMIT     1
#define T_GROUP_LIMIT     2
#define T_HAVING_LIMIT    3
#define T_INSERT_LIMIT    4
#define T_UPDATE_LIMIT    5
#define T_AGG_LIMIT       6
#define T_VARIABLE_VALUE_LIMIT 7
#define T_WHEN_LIMIT      8

extern int resolve_select_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
extern int resolve_delete_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
extern int resolve_insert_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
extern int resolve_update_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
extern int resolve_table(
    ResultPlan * result_plan,
    oceanbase::sql::ObStmt* stmt,
    ParseNode* node,
    uint64_t& table_id);
extern int resolve_and_exprs(
    ResultPlan * result_plan,
    oceanbase::sql::ObStmt* stmt,
    ParseNode* node,
    oceanbase::common::ObVector<uint64_t>& and_exprs,
    int32_t expr_scope_type = T_NONE_LIMIT);
extern int resolve_independ_expr(
    ResultPlan * result_plan,
    oceanbase::sql::ObStmt* stmt,
    ParseNode* node,
    uint64_t& expr_id,
    int32_t expr_scope_type = T_NONE_LIMIT);
extern int resolve_table_columns(
    ResultPlan * result_plan,
    oceanbase::sql::ObStmt* stmt,
    oceanbase::sql::TableItem& table_item,
    int64_t num_columns = -1);
// add longfei 20151105
/**
 * @brief generate_inner_index_table_name
 * @param index_name
 * @param original_table_name
 * @param out_buff
 * @param str_len
 * @return
 */
extern int generate_inner_index_table_name(
    ObString& index_name,
    ObString& original_table_name,
    char *out_buff,
    int64_t& str_len);
// add e

//add zt 20151207:b
extern int resolve_array_expr(
    ResultPlan * result_plan,
    ParseNode* node,
    ObString &array_name,
    ObObj &idx_value);
//add zt 20151207:e

#endif //DML_BUILD_PLAN_H_

