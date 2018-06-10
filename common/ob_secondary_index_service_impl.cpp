/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_secondary_index_service_impl.cpp
 * @brief for using secondary index in select statement
 *
 * Created by longfei：implementation of secondary index service
 * maybe more function?
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_29
 */

#include "ob_secondary_index_service_impl.h"
#include "ob_postfix_expression.h"
#include "ob_define.h"
#include "ob_schema.h"

using namespace oceanbase;
using namespace common;
using namespace sql;

int ObSecondaryIndexServiceImpl::init(const ObSchemaManagerV2* schema_manager_)
{
  int ret = OB_SUCCESS;
  if(NULL == schema_manager_)
  {
    TBSYS_LOG(ERROR,"sql context isn't init!");
    ret = OB_ERROR;
  }
  this->schema_manager_ = schema_manager_;
  return ret;
}

ObSecondaryIndexServiceImpl::ObSecondaryIndexServiceImpl() :
  schema_manager_(NULL)
{
}

const ObSchemaManagerV2* oceanbase::common::ObSecondaryIndexServiceImpl::getSchemaManager() const
{
  return schema_manager_;
}

void oceanbase::common::ObSecondaryIndexServiceImpl::setSchemaManager(
    const ObSchemaManagerV2* schemaManager)
{
  schema_manager_ = schemaManager;
}

/*************************************************************************
 * find the information in OB's expression
 *************************************************************************/

int ObSecondaryIndexServiceImpl::find_cid(
    ObSqlExpression& sql_expr,
    uint64_t &cid)
{
  int ret = OB_SUCCESS;
  int64_t type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr.get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  int32_t column_count = 0;
  int64_t tmp_cid = OB_INVALID_ID;
  int64_t tid = OB_INVALID_ID;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tid)))
      {
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 2].get_int(tmp_cid)))
      {
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else
      {
        column_count++;
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
      }
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  if (column_count == 1 && ret == OB_SUCCESS)
  {
    cid = tmp_cid;
  }
  else
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "too many columns in one expr");
  }
  return ret;
}

int ObSecondaryIndexServiceImpl::change_tid(
    ObSqlExpression* sql_expr,
    uint64_t& array_index)
{

  int ret = OB_SUCCESS;
  int64_t type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr->get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  int32_t column_count = 0;
  int64_t tmp_index = OB_INVALID_ID;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      tmp_index = idx + 1;
      column_count++;
      idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  if (column_count == 1 && ret == OB_SUCCESS)
    array_index = (uint64_t) tmp_index;
  else
    ret = OB_ERROR;
  return ret;
}

int ObSecondaryIndexServiceImpl::get_cid(
    ObSqlExpression* sql_expr,
    uint64_t& cid)
{
  int ret = OB_SUCCESS;
  int64_t type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr->get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  int32_t column_count = 0;
  int64_t tmp_cid = OB_INVALID_ID;
  int64_t tmp_tid = OB_INVALID_ID;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tmp_tid)))
      {
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 2].get_int(tmp_cid)))
      {
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else
      {
        column_count++;
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
      }
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END || type == sql::ObPostfixExpression::UPS_TIME_OP  || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  if (column_count == 1 && ret == OB_SUCCESS)
    cid = (uint64_t) tmp_cid;
  else
    ret = OB_ERROR;
  return ret;
}


//判断该表达式的所有列是否都在索引表index_tid中
bool ObSecondaryIndexServiceImpl::is_all_expr_cid_in_indextable(
    uint64_t index_tid,
    const sql::ObPostfixExpression& pf_expr,
    const ObSchemaManagerV2 *sm_v2)
{
  bool return_ret = true;
  int ret = OB_SUCCESS;
  int64_t type = 0;
  const ExprArray& expr_ = pf_expr.get_expr(); 
  int64_t count = expr_.count();
  int64_t idx = 0;
  int64_t tmp_cid = OB_INVALID_ID;
  int64_t tid = OB_INVALID_ID;
  //testb longfei 2016-03-12 14:22:03
//  for (int64_t i = 0; i < count; i++)
//  {
//    TBSYS_LOG(WARN, "test::longfei>>>expr_[%ld] is [%s]", i, to_cstring(expr_.at(i)));
//  }
  //teste
  while (idx < count)
  {
    //TBSYS_LOG(WARN, "test::longfei>>>idx[%ld]", idx);
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      return_ret = false;
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {      
      if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tid)))
      {
        return_ret = false;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 2].get_int(tmp_cid)))
      {
        return_ret = false;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else
      {
        const ObColumnSchemaV2* column_schema = NULL;
        if (NULL == (column_schema = sm_v2->get_column_schema(index_tid, tmp_cid)))
        {
          return_ret = false;
          break;
        }
        else
          idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
      }
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW  // ??
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      return_ret = false;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  return return_ret;
}

bool ObSecondaryIndexServiceImpl::is_have_main_cid(
    sql::ObSqlExpression& sql_expr,
    uint64_t main_column_id)
{
  int ret = OB_SUCCESS;
  bool return_ret = false;
  int64_t type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr.get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  int32_t column_count = 0;
  int64_t cid = OB_INVALID_ID;
  int64_t tid = OB_INVALID_ID;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      return_ret = true;
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tid)))
      {
        return_ret = true;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 2].get_int(cid)))
      {
        return_ret = true;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else
      {
        if (cid == (int64_t) main_column_id)
        {
          return_ret = true;
          break;
        }
        else
        {
          column_count++;
          idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
        }
      }
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      return_ret = true;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  if (column_count > 1)
    return_ret = true;
  return return_ret;
}

int ObSecondaryIndexServiceImpl::get_all_cloumn(
    sql::ObSqlExpression& sql_expr,
    ObArray<uint64_t> &column_index)
{
  int ret = OB_SUCCESS;
  int64_t type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr.get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      column_index.push_back(idx + 1);
      idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END
             || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  return ret;
}

bool ObSecondaryIndexServiceImpl::is_this_expr_can_use_index(
    sql::ObSqlExpression& sql_expr,
    uint64_t &index_tid,
    uint64_t main_tid,
    const ObSchemaManagerV2 *sm_v2)
{
  int ret = OB_SUCCESS;
  bool return_ret = false;
  int64_t type = 0;
  int64_t tmp_type = 0;
  const sql::ObPostfixExpression& pf_expr = sql_expr.get_decoded_expression();
  const ExprArray& expr_ = pf_expr.get_expr();
  int64_t count = expr_.count();
  int64_t idx = 0;
  int32_t column_count = 0;
  int32_t EQ_count = 0;
  int32_t IN_count = 0;
  int64_t cid = OB_INVALID_ID;
  int64_t tid = OB_INVALID_ID;
  while (idx < count)
  {
    if (OB_SUCCESS != (ret = expr_[idx].get_int(type)))
    {
      return_ret = false;
      TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
      break;
    }
    else if (type == sql::ObPostfixExpression::COLUMN_IDX)
    {
      if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tid)))
      {
        return_ret = false;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 2].get_int(cid)))
      {
        return_ret = false;
        TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        break;
      }
      else
      {
        column_count++;
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::COLUMN_IDX);
      }
    }
    else if (type == sql::ObPostfixExpression::OP)
    {
      if (ObIntType != expr_[idx + 1].get_type())
      {
        return_ret = false;
        ret = OB_ERROR;
        break;
      }
      else if (OB_SUCCESS != (ret = expr_[idx + 1].get_int(tmp_type)))
      {
        TBSYS_LOG(WARN, "fail to get int value.err=%d", ret);
        return_ret = false;
        break;
      }
      else if (tmp_type == T_OP_EQ)
      {
        EQ_count++;
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::OP);
      }
      else if (tmp_type == T_OP_IN)
      {
        IN_count++;
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::OP);
      }
      else
      {
        idx = idx + pf_expr.get_type_num(idx, sql::ObPostfixExpression::OP);
      }
    }
    else if (type == sql::ObPostfixExpression::OP || type == sql::ObPostfixExpression::COLUMN_IDX || type == T_OP_ROW
             || type == sql::ObPostfixExpression::CONST_OBJ || type == sql::ObPostfixExpression::END
             || type == sql::ObPostfixExpression::UPS_TIME_OP || type == sql::ObPostfixExpression::PARAM_IDX)//mod wsx[prepare bug]
    {
      idx = idx + pf_expr.get_type_num(idx, type);
    }
    else
    {
      return_ret = false;
      TBSYS_LOG(WARN, "wrong expr type: %ld", type);
      break;
    }
  }
  //mod longfei 2016-03-12 15:21:21 :b
//  if ((column_count == 1 && EQ_count == 1)
//      || (column_count == 1 && IN_count == 1))
  if(column_count == 1)
  //mode
  {
    uint64_t tmp_index_tid[OB_MAX_INDEX_NUMS];
    for (int32_t m = 0; m < OB_MAX_INDEX_NUMS; m++)
    {
      tmp_index_tid[m] = OB_INVALID_ID;
    }
    if (sm_v2->is_cid_in_index(cid, main_tid, tmp_index_tid))
    {
      index_tid = tmp_index_tid[0];
      return_ret = true;
    }
  }
  return return_ret;
}

/*************************************************************************
 * for transformer to generate physical plan
 *************************************************************************/

bool ObSecondaryIndexServiceImpl::is_this_table_avalibale(uint64_t tid)
{
  bool ret = false;
  const ObTableSchema *main_table_schema = NULL;
  if (NULL == (main_table_schema = schema_manager_->get_table_schema(tid)))
  {
    //ret = OB_ERR_ILLEGAL_ID;
    TBSYS_LOG(WARN, "fail to get table schema for table[%ld]", tid);
  }
  else
  {
    if (main_table_schema->get_index_status() == AVALIBALE)
    {
      ret = true;
    }
  }
  return ret;
}

bool ObSecondaryIndexServiceImpl::is_index_table_has_all_cid_V2(
    uint64_t index_tid,
    Expr_Array *filter_array,
    Expr_Array *project_array)
{
  bool ret = true;
  if (is_this_table_avalibale(index_tid))
  {
    int64_t w_num = project_array->count();
    for (int32_t i = 0; i < w_num; i++)
    {
      ObSqlExpression col_expr = project_array->at(i);
      const sql::ObPostfixExpression& postfix_pro_expr =
          col_expr.get_decoded_expression();
      if (!is_all_expr_cid_in_indextable(index_tid, postfix_pro_expr,
                                         schema_manager_))
      {
        ret = false;
        break;
      }
    }
    int64_t c_num = filter_array->count();
    for (int32_t j = 0; j < c_num; j++)
    {

      ObSqlExpression c_filter = filter_array->at(j);
      const sql::ObPostfixExpression& postfix_fil_expr =
          c_filter.get_decoded_expression();
      if (!is_all_expr_cid_in_indextable(index_tid, postfix_fil_expr,
                                         schema_manager_))
      {
        ret = false;
        break;
      }
    }
  }
  return ret;
}

int64_t ObSecondaryIndexServiceImpl::is_cid_in_index_table(
    uint64_t cid,
    uint64_t tid)
{
  int64_t return_ret = 0;
  int ret = OB_SUCCESS;
  bool is_in_rowkey = false;
  bool is_in_other_column = false;
  const ObTableSchema *index_table_schema = NULL;
  if (NULL == (index_table_schema = schema_manager_->get_table_schema(tid)))
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "Fail to get table schema for table[%ld]", tid);
  }
  else
  {
    uint64_t tmp_cid = OB_INVALID_ID;
    int64_t rowkey_column = index_table_schema->get_rowkey_info().get_size();
    for (int64_t j = 0; j < rowkey_column; j++)
    {
      if (OB_SUCCESS
          != (ret = index_table_schema->get_rowkey_info().get_column_id(j,
                                                                        tmp_cid)))
      {
        TBSYS_LOG(ERROR, "get column schema failed,cid[%ld]", tmp_cid);
        ret = OB_SCHEMA_ERROR;
      }
      else
      {
        if (tmp_cid == cid)
        {
          is_in_rowkey = true;
          break;
        }
      }
    }
    if (!is_in_rowkey)
    {
      const ObColumnSchemaV2* index_column_schema = NULL;
      index_column_schema = schema_manager_->get_column_schema(tid, cid);
      if (index_column_schema != NULL)
      {
        is_in_other_column = true;
      }
    }
  }
  if (is_in_rowkey)
    return_ret = 1;
  else if (is_in_other_column)
    return_ret = 2;
  return return_ret;
}

bool ObSecondaryIndexServiceImpl::is_expr_can_use_storing_V2(
    ObSqlExpression c_filter,
    uint64_t mian_tid,
    uint64_t &index_tid,
    Expr_Array * filter_array,
    Expr_Array *project_array)
{
  bool ret = false;
  uint64_t expr_cid = OB_INVALID_ID;
  uint64_t tmp_index_tid = OB_INVALID_ID;
  uint64_t index_tid_array[OB_MAX_INDEX_NUMS];
  for (int32_t k = 0; k < OB_MAX_INDEX_NUMS; k++)
  {
    index_tid_array[k] = OB_INVALID_ID;
  }
  if (OB_SUCCESS == find_cid(c_filter, expr_cid)) //获得表达式中存的列的column id:expr_cid。如果表达式中有多列，返回ret不等于OB_SUCCESS
  {
    //TBSYS_LOG(WARN, "test::longfei>>>expr_cid[%ld]", expr_cid);
    if (schema_manager_->is_cid_in_index(expr_cid, mian_tid, index_tid_array)) //根据原表的tid，找到该表的��?有的第一主键为expr_cid的索引表，存到index_tid_array里面
    {
      for (int32_t i = 0; i < OB_MAX_INDEX_NUMS; i++)  //对每��?张符合条件的索引��?
      {
        //uint64_t tmp_tid=index_tid_array[i];
        if (index_tid_array[i] != OB_INVALID_ID)
        {
          //判断是否��?有在sql语句里面出现的列，都在这张索引表��?
          if (is_index_table_has_all_cid_V2(index_tid_array[i], filter_array,
                                            project_array))
          {
            tmp_index_tid = index_tid_array[i];
            ret = true;
            break;
          }
        }
      }
      index_tid = tmp_index_tid;
    }
  }
  return ret;
}

bool ObSecondaryIndexServiceImpl::is_wherecondition_have_main_cid_V2(
    Expr_Array *filter_array,
    uint64_t main_cid)
{
  bool return_ret = false;
  int ret = OB_SUCCESS;

  int64_t c_num = filter_array->count();
  int32_t i = 0;
  for (; ret == OB_SUCCESS && i < c_num; i++)
  {
    ObSqlExpression c_filter = filter_array->at(i);
    if (is_have_main_cid(c_filter, main_cid))
    {
      return_ret = true;
      break;
    }
  }
  return return_ret;
}

bool ObSecondaryIndexServiceImpl::if_rowkey_in_expr(
    Expr_Array *filter_array,
    uint64_t main_tid)
{
  bool return_ret = false;
  uint64_t tid = main_tid;
  const ObTableSchema *mian_table_schema = NULL;
  if (NULL == (mian_table_schema = schema_manager_->get_table_schema(tid)))
  {
    TBSYS_LOG(WARN, "Fail to get table schema for table[%ld]", tid);
  }
  else
  {
    const ObRowkeyInfo *rowkey_info = &mian_table_schema->get_rowkey_info();
    uint64_t main_cid = OB_INVALID_ID;
    rowkey_info->get_column_id(0, main_cid);
    return_ret = is_wherecondition_have_main_cid_V2(filter_array, main_cid);

  }
  return return_ret;
}

bool ObSecondaryIndexServiceImpl::decide_is_use_storing_or_not_V2(
    Expr_Array *filter_array,
    Expr_Array *project_array,
    uint64_t &index_table_id,
    uint64_t main_tid)
{
  bool return_ret = false;
  int ret = OB_SUCCESS;

  uint64_t tid = main_tid;
  uint64_t index_tid = OB_INVALID_ID;
  const ObTableSchema *mian_table_schema = NULL;
  if (NULL == (mian_table_schema = schema_manager_->get_table_schema(tid)))
  {
    TBSYS_LOG(WARN, "Fail to get table schema for table[%ld]", tid);
  }
  else
  {
    const ObRowkeyInfo *rowkey_info = &mian_table_schema->get_rowkey_info();
    //TBSYS_LOG(WARN,"test::longfei>>>main_name[%s]",mian_table_schema->get_table_name());
    uint64_t main_cid = OB_INVALID_ID;//第一主键column id
    rowkey_info->get_column_id(0, main_cid);
    if (!is_wherecondition_have_main_cid_V2(filter_array, main_cid))
    {
      int64_t c_num = filter_array->count();
      int32_t i = 0;
      for (; ret == OB_SUCCESS && i < c_num; i++)
      {
        ObSqlExpression c_filter = filter_array->at(i);
        //判断该表达式能否使用不回表的索引
        if (is_expr_can_use_storing_V2(c_filter, tid, index_tid, filter_array,
                                       project_array))
        {
          index_table_id = index_tid;
          //TBSYS_LOG(WARN, "test::longfei>>>not back to original table, index_tid[%ld]",index_table_id);
          return_ret = true;
          break;
        }
      }
    }
  }
  return return_ret;
}

bool ObSecondaryIndexServiceImpl::is_can_use_hint_for_storing_V2(
    Expr_Array *filter_array,
    Expr_Array *project_array,
    uint64_t index_table_id)
{
  bool cond_has_main_cid = false;
  bool can_use_hint_for_storing = false;
  const ObTableSchema *index_table_schema = NULL;
  if (NULL == (index_table_schema = schema_manager_->get_table_schema(index_table_id)))
  {
    TBSYS_LOG(WARN, "Fail to get table schema for table[%ld]", index_table_id);
  }
  else if (schema_manager_->is_this_table_avalibale(index_table_id))
  {
    const ObRowkeyInfo& rowkey_info = index_table_schema->get_rowkey_info();
    uint64_t index_key_cid = OB_INVALID_ID;
    // 获得索引表的第一主键的column id
    if (OB_SUCCESS != rowkey_info.get_column_id(0, index_key_cid))
    {
      TBSYS_LOG(WARN,
                "Fail to get column id, index_table name:[%s], index_table id: [%ld]",
                index_table_schema->get_table_name(),
                index_table_schema->get_table_id());
      cond_has_main_cid = false;
    }
    // 判断where条件的表达式中是否包含索引表的第一主键，每个表达式都只有一列且其中有一列是索引表的第一主键时返回true
    else if (!is_wherecondition_have_main_cid_V2(filter_array, index_key_cid))
    {
      cond_has_main_cid = false;
    }
    else
    {
      cond_has_main_cid = true;
    }
  }

  if (cond_has_main_cid)
  {
    // 如果where条件中包含索引表的第一主键再判断这些表达式中的列和select的输出列是不是都在索引表冗余列中
    can_use_hint_for_storing = is_index_table_has_all_cid_V2(
                                 index_table_id,
                                 filter_array, project_array);
  }
  // 如果对于where条件不能使用主键索引的情况则认为不能使用索引表的storing列
  else
  {
    can_use_hint_for_storing = false;
  }

  return can_use_hint_for_storing;
}

bool ObSecondaryIndexServiceImpl::is_can_use_hint_index_V2(
    Expr_Array *filter_array,
    uint64_t index_table_id)
{
  bool can_use_hint_index = false;
  bool cond_has_main_cid = false;
  const ObTableSchema *index_table_schema = NULL;
  if (NULL
      == (index_table_schema = schema_manager_->get_table_schema(index_table_id)))
  {
    TBSYS_LOG(WARN, "Fail to get table schema for table[%ld]", index_table_id);
  }
  else
  {
    const ObRowkeyInfo& rowkey_info = index_table_schema->get_rowkey_info();
    uint64_t index_key_cid = OB_INVALID_ID;
    // 获得索引表的第一主键的column id
    if (OB_SUCCESS != rowkey_info.get_column_id(0, index_key_cid))
    {
      TBSYS_LOG(WARN,
                "Fail to get column id, index_table name:[%s], index_table id: [%ld]",
                index_table_schema->get_table_name(),
                index_table_schema->get_table_id());
      cond_has_main_cid = false;
    }
    // 判断where条件的表达式中是否包含索引表的第一主键，每个表达式都只有一列且其中有一列是索引表的第一主键时返回true
    else if (!is_wherecondition_have_main_cid_V2(filter_array, index_key_cid))
    {
      cond_has_main_cid = false;
    }
    else
    {
      cond_has_main_cid = true;
    }
  }

  if (cond_has_main_cid)
  {
    can_use_hint_index = true;
  }
  if (!schema_manager_->is_this_table_avalibale(index_table_id))
  {
    can_use_hint_index = false;
  }
  return can_use_hint_index;
}
