/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_create_table_stmt.cpp
 * @brief for logical plan of create index
 *
 * Created by longfei：for create index
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_21
 */

#include "ob_create_index_stmt.h"
#include "ob_schema_checker.h"
#include "common/ob_strings.h"
#include "common/ob_postfix_expression.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

const ObString& ObCreateIndexStmt::get_original_table_name() const
{
  return original_table_name_;
}

int64_t ObCreateIndexStmt::get_index_columns_count() const
{
  return index_columns_.count();
}
const ObString& ObCreateIndexStmt::get_index_columns(int64_t index) const
{
  return index_columns_.at(index);
}
bool ObCreateIndexStmt::has_storing()
{
  return has_storing_col_;
}

int64_t ObCreateIndexStmt::get_storing_columns_count() const
{
  return storing_columns_.count();
}
const ObString& ObCreateIndexStmt::get_storing_columns(int64_t index) const
{
  return storing_columns_.at(index);
}

bool ObCreateIndexStmt::is_rowkey_hit(uint64_t cid)
{
  bool ret = false;
  //uint64_t rk=common::OB_INVALID_ID;
  for(int64_t i = 0; i < hit_rowkey_.count(); i++)
  {
    if(cid == hit_rowkey_.at(i))
    {
      ret=true;
      break;
    }
  }
  return ret;
}

int ObCreateIndexStmt::push_hit_rowkey(uint64_t cid)
{
  int ret = common::OB_SUCCESS;
  if(common::OB_SUCCESS != (ret = hit_rowkey_.push_back(cid)))
  {
    TBSYS_LOG(WARN,"put cid[%ld] rowkey in hit arrary error",cid);
  }
  return ret;
}

int ObCreateIndexStmt::set_original_table_name(ResultPlan& result_plan, const ObString& original_table_name)
{
  int ret = OB_SUCCESS;
  ObSchemaChecker* schema_checker = NULL;
  uint64_t table_id = OB_INVALID_ID;
  bool is_index_full = false;
  schema_checker = static_cast<ObSchemaChecker*>(result_plan.schema_checker_);
  if (NULL == schema_checker)
  {
    ret = OB_ERR_SCHEMA_UNSET;
    snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
             "Schema(s) are not set");
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_INVALID_ID == (table_id = schema_checker->get_table_id(original_table_name)))
    {
      ret = OB_ERR_ALREADY_EXISTS;
      snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
               "the table to create index '%.*s' is not exist", original_table_name.length(), original_table_name.ptr());
    }
    if(OB_SUCCESS == ret)
    {
      if(OB_SUCCESS != (ret = schema_checker->is_index_full(table_id,is_index_full)))
      {
      }
      else if(is_index_full)
      {
        //mod huangjianwei [secondary index debug] 20170314:b
        //ret = OB_ERROR;
        ret = OB_ERROR_INDEX_ALREADY_FULL;
        snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
                 "table %.*s index number is full!", original_table_name.length(), original_table_name.ptr());
        //mod:e
      }
    }
  }
  if ((OB_SUCCESS == ret) && !is_index_full && OB_SUCCESS != (ret = ob_write_string(*name_pool_, original_table_name, original_table_name_)))
  {
    TBSYS_LOG(ERROR, "Make space for %.*s failed", original_table_name.length(), original_table_name.ptr());
  }
  return ret;
}

int ObCreateIndexStmt::generate_inner_index_table_name(ObString& index_name, ObString& original_table_name, char *out_buff, int64_t& str_len)
{
  int ret = OB_SUCCESS;
  char str[OB_MAX_TABLE_NAME_LENGTH];
  char raw[OB_MAX_TABLE_NAME_LENGTH];
  memset(str,0,OB_MAX_TABLE_NAME_LENGTH);
  memset(raw,0,OB_MAX_TABLE_NAME_LENGTH);
  if(index_name.length() > OB_MAX_TABLE_NAME_LENGTH || original_table_name.length() > OB_MAX_TABLE_NAME_LENGTH)
  {
    TBSYS_LOG(WARN,"buff is not enough to generate index table name");
    ret=OB_ERROR;
  }
  else
  {
    strncpy(str, index_name.ptr(), index_name.length());
    strncpy(raw, original_table_name.ptr(), original_table_name.length());
    int wlen = snprintf(out_buff, OB_MAX_TABLE_NAME_LENGTH, "___%s_%s",  raw, str);
    if((size_t)wlen > (size_t)OB_MAX_TABLE_NAME_LENGTH)
    {
      ret = OB_ERROR;
    }
    str_len = wlen;
  }
  return ret;
}

int ObCreateIndexStmt::set_index_columns(ResultPlan& result_plan,const common::ObString& table_name,const common::ObString& column_name)
{
  /*
   * first,you should check if index columns of source table are exist;
   * then,you should check if there are duplicate columns;
   */
  int ret = OB_SUCCESS;
  ObString str;
  ObSchemaChecker* schema_checker = NULL;
  uint64_t col_id = OB_INVALID_ID;
  schema_checker = static_cast<ObSchemaChecker*>(result_plan.schema_checker_);
  if (schema_checker == NULL)
  {
    ret = common::OB_ERR_SCHEMA_UNSET;
    snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
             "Schema(s) are not set");
  }
  if(OB_SUCCESS == ret)
  {
    col_id = schema_checker->get_column_id(table_name, column_name);
    if(OB_INVALID_ID == col_id)
    {
      ret = OB_ERR_COLUMN_NOT_FOUND;
      snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
               "Index Columns %.*s are not found",column_name.length(), column_name.ptr());
    }
  }
  if(OB_SUCCESS == ret)
  {
    if(index_columns_.count()>0)
    {
      for (int32_t i = 0; i < index_columns_.count(); i++)
      {
        if (index_columns_.at(i) == column_name)
        {
          ret = OB_ERR_COLUMN_DUPLICATE;
          snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
                   "Not unique index_colums_: '%.*s'", column_name.length(), column_name.ptr());
          break;
        }
      }
    }
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(ERROR,"create index stmt error ret=[%d]", ret);
    }
    else if (OB_SUCCESS != (ret = ob_write_string(*name_pool_, column_name, str)))
    {
      TBSYS_LOG(ERROR,"Make space for %.*s failed", column_name.length(), column_name.ptr());
    }
    else
    {
      index_columns_.push_back(str);
    }
  }
  return ret;
}

int ObCreateIndexStmt::set_storing_columns(ResultPlan& result_plan, const common::ObString& table_name, const ObString& storing_column)
{
  int ret = common::OB_SUCCESS;
  common::ObString str;
  ObSchemaChecker* schema_checker = NULL;
  uint64_t col_id = common::OB_INVALID_ID;
  schema_checker = static_cast<ObSchemaChecker*>(result_plan.schema_checker_);
  if (NULL == schema_checker)
  {
    ret = common::OB_ERR_SCHEMA_UNSET;
    snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
             "Schema(s) are not set");
  }
  if(common::OB_SUCCESS == ret)
  {
    col_id = schema_checker->get_column_id(table_name,storing_column);
    if(common::OB_INVALID_ID == col_id)
    {
      ret = common::OB_ERR_COLUMN_NOT_FOUND;
      snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
               "Storing Columns %.*s are not found",storing_column.length(), storing_column.ptr());
    }
  }
  if(common::OB_SUCCESS == ret)
  {
    // 冗余列不能是原表主键列
    if(schema_checker->is_rowkey_column(table_name,storing_column))
    {
      ret = common::OB_ERR_COLUMN_NOT_FOUND;
      snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
               "Storing Columns %.*s can not be rowkey!",storing_column.length(), storing_column.ptr());
    }
  }
  if(common::OB_SUCCESS == ret)
  {
    if(storing_columns_.count()>0)
    {
      for (int32_t i = 0; i < storing_columns_.count(); i++)
      {
        if (storing_columns_.at(i) == storing_column)
        {
          ret = common::OB_ERROR;
          snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
                   "Not unique index_colums_: '%.*s'", storing_column.length(), storing_column.ptr());
          break;
        }
      }
    }
  }
  if(common::OB_SUCCESS == ret)
  {
    for(int32_t i = 0; i < index_columns_.count(); i++)
    {
      if (index_columns_.at(i) == storing_column)
      {
        ret = common::OB_ERROR;
        snprintf(result_plan.err_stat_.err_msg_, MAX_ERROR_MSG,
                 "There is same column in storing columns and index_columns: '%.*s'", storing_column.length(), storing_column.ptr());
        break;
      }
    }
  }
  if(common::OB_SUCCESS == ret)
  {
    if (common::OB_SUCCESS != (ret = ob_write_string(*name_pool_, storing_column, str)))
    {
      TBSYS_LOG(ERROR,"Make space for %.*s failed", storing_column.length(), storing_column.ptr());
    }
    else
    {
      storing_columns_.push_back(str);
    }
  }
  return ret;
}

int ObCreateIndexStmt::generate_expire_col_list(ObString& input, ObStrings& out)
{
  int ret = OB_SUCCESS;
  ObExpressionParser parser;
  ObArrayHelper<ObObj> obj_array;
  ObObj sym_list_[OB_MAX_COMPOSITE_SYMBOL_COUNT];
  obj_array.init(OB_MAX_COMPOSITE_SYMBOL_COUNT, sym_list_);
  ObString val;
  int i = 0;
  int64_t type = 0;
  int64_t postfix_size = 0;
  if(OB_SUCCESS != (ret = (parser.parse(input,obj_array))))
  {
    TBSYS_LOG(ERROR,"generate_expire_col_list parse error,ret[%d]",ret);
  }
  else
  {
    postfix_size=obj_array.get_array_index();
  }
  if(OB_SUCCESS == ret)
  {
    i=0;
    while(i<postfix_size-1)
    {
      if(OB_SUCCESS != obj_array.at(i)->get_int(type))
      {
        TBSYS_LOG(WARN, "unexpected data type. int expected, but actual type is %d",
                  obj_array.at(i)->get_type());
        ret = OB_ERR_UNEXPECTED;
        break;
      }
      else
      {
        if(ObExpression::COLUMN_IDX == type)
        {
          if (OB_SUCCESS != obj_array.at(i+1)->get_varchar(val))
          {
            TBSYS_LOG(WARN, "unexpected data type. varchar expected, "
                            "but actual type is %d",
                      obj_array.at(i+1)->get_type());
            ret = OB_ERR_UNEXPECTED;
            break;
          }
          else
          {
            out.add_string(val);;
          }
        }
      }
      i += 2;
    }
  }
  return ret;
}

bool ObCreateIndexStmt::is_expire_col_in_storing(common::ObString& col)
{
  bool ret = false;
  for(int64_t i = 0; i < storing_columns_.count(); i++)
  {
    if(col == storing_columns_.at(i))
    {
      ret = true;
      break;
    }
  }
  for(int64_t i = 0; i < index_columns_.count(); i++)
  {
    if(col == index_columns_.at(i))
    {
      ret=true;
      break;
    }
  }
  return ret;
}

int ObCreateIndexStmt::set_storing_columns_simple(const common::ObString storing_name)
{
  return storing_columns_.push_back(storing_name);
}
