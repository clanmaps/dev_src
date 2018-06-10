/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_sql_expression.cpp
 * @brief sql expression class
 *
 * modified by longfei：
 * 1.add function: set_table_id()
 * modified by Qiushi FAN: add some functions to create a new expression
 *
 * modified by zhutao:modified alloc and free function
 *
 * @version __DaSE_VERSION
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author Qiushi FAN <qsfan@ecnu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 *
 * @date 2016_07_30
 */
/*
 * (C) 2007-2012 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 * Version:  ob_sql_expression.cpp,  05/28/2012 03:46:40 PM xiaochu Exp $
 *
 * Author:
 *   xiaochu.yh <xiaochu.yh@taobao.com>
 * Description:
 *
 *
 */

#include "ob_sql_expression.h"
#include "common/utility.h"
#include "sql/ob_item_type_str.h"
#include "common/ob_cached_allocator.h"
#include "sql/ob_phy_operator_type.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

ObSqlExpression::ObSqlExpression()
  : column_id_(0), table_id_(0), is_aggr_func_(false), is_distinct_(false)
  , aggr_func_(T_INVALID)
{
  //add weixing [implementation of sub_query]20160106
  use_bloom_filter_ = false;
  //add e
}

ObSqlExpression::~ObSqlExpression()
{
  //add weixing [implementation of sub_query]20160106
  if(use_bloom_filter_)
  {
    bloom_filter_.destroy();
  }
  //add e
}

ObSqlExpression::ObSqlExpression(const ObSqlExpression &other)
  :DLink()
{
  *this = other;
}

ObSqlExpression& ObSqlExpression::operator=(const ObSqlExpression &other)
{
  if (&other != this)
  {
    post_expr_ = other.post_expr_;
    column_id_ = other.column_id_;
    table_id_ = other.table_id_;
    is_aggr_func_ = other.is_aggr_func_;
    is_distinct_ = other.is_distinct_;
    aggr_func_ = other.aggr_func_;
    //add weixing [implementation of sub_query]20160106
    use_bloom_filter_ = other.use_bloom_filter_;
    if(use_bloom_filter_)
    {
      bloom_filter_.deep_copy(other.bloom_filter_);
    }
    //add e
    // @note we do not copy the members of DLink on purpose
  }
  return *this;
}

int ObSqlExpression:: add_expr_obj(const ObObj &obj)
{
  return post_expr_.add_expr_obj(obj);
}

int ObSqlExpression::add_expr_item(const ExprItem &item)
{
  return post_expr_.add_expr_item(item);
}

int ObSqlExpression::add_expr_item_end()
{
  return post_expr_.add_expr_item_end();
}

//add weixing [implementation of sub_query]20160106
void ObSqlExpression::set_has_bloomfilter()
{
    use_bloom_filter_ = true;
    post_expr_.set_has_bloomfilter(&bloom_filter_);
}
//add e

static ObObj OBJ_ZERO;
static struct obj_zero_init
{
  obj_zero_init()
  {
    OBJ_ZERO.set_int(0);
  }
} obj_zero_init;

//mod weixing [implementation of sub_query]20160116
//int ObSqlExpression::calc(const common::ObRow &row, const common::ObObj *&result)
int ObSqlExpression::calc(const common::ObRow &row, const common::ObObj *&result, hash::ObHashMap<common::ObRowkey, common::ObRowkey, common::hash::NoPthreadDefendMode>* hash_map, bool second_check)
//mod e
{
  int err = OB_SUCCESS;
  if (OB_UNLIKELY(is_aggr_func_ && T_FUN_COUNT == aggr_func_ && post_expr_.is_empty()))
  {
    // COUNT(*)
    // point the result to an arbitray non-null cell
    result = &OBJ_ZERO;
  }
  else
  {
    //mod weixing [implementation of sub_query]20160116
    //err = post_expr_.calc(row, result);
    err = post_expr_.calc(row, result, hash_map, second_check);
    //mod e
  }
  return err;
}

int64_t ObSqlExpression::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  if (OB_INVALID_ID == table_id_)
  {
    databuff_printf(buf, buf_len, pos, "expr<NULL,%lu>=", column_id_);
  }
  else
  {
    databuff_printf(buf, buf_len, pos, "expr<%lu,%lu>=", table_id_, column_id_);
  }
  if (is_aggr_func_)
  {
    databuff_printf(buf, buf_len, pos, "%s(%s", ob_aggr_func_str(aggr_func_), is_distinct_ ? "DISTINCT " : "");
  }
  if (post_expr_.is_empty())
  {
    databuff_printf(buf, buf_len, pos, "*");
  }
  else
  {
    databuff_printf(buf, buf_len, pos, "[");
    pos += post_expr_.to_string(buf+pos, buf_len-pos);
    databuff_printf(buf, buf_len, pos, "]");
  }
  if (is_aggr_func_)
  {
    databuff_printf(buf, buf_len, pos, ")");
  }
  return pos;
}

DEFINE_SERIALIZE(ObSqlExpression)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = serialize_basic_param(buf, buf_len, pos)))
  {
	TBSYS_LOG(WARN, "fail to serialize basic param. ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = post_expr_.serialize(buf, buf_len, pos)))
  {
	TBSYS_LOG(WARN, "fail to serialize postfix expression. ret=%d", ret);
  }
  else
  {
    //add weixing [implementation of sub_query]20160106
    ObObj use_bloom_filter;
    use_bloom_filter.set_bool(use_bloom_filter_);
    if(OB_SUCCESS != (ret = use_bloom_filter.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize sub query num. ret=%d", ret);
    }
    else if(use_bloom_filter_)
    {
      if(OB_SUCCESS != (ret = bloom_filter_.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "fail to eserialize bloomfilter. ret=%d", ret);
      }
      TBSYS_LOG(WARN, "serialize bloomfilter success");
    }
    //add e
	// success
	//TBSYS_LOG(INFO, "success serialize one ObSqlExpression. pos=%ld", pos);
  }
  return ret;
}

DEFINE_DESERIALIZE(ObSqlExpression)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = deserialize_basic_param(buf, data_len, pos)))
  {
	TBSYS_LOG(WARN, "fail to serialize basic param. ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = post_expr_.deserialize(buf, data_len, pos)))
  {
	TBSYS_LOG(WARN, "fail to serialize postfix expression. ret=%d", ret);
  }
  else
  {
    //add weixing [implementation of sub_query]20160106
    ObObj use_bloom_filter;
    if(OB_SUCCESS != (ret = use_bloom_filter.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to deserialize sub query num. ret=%d", ret);
    }
    else if (OB_SUCCESS != (ret = use_bloom_filter.get_bool(use_bloom_filter_)))
    {
      TBSYS_LOG(WARN, "fail to get expr_count. ret=%d", ret);
    }
    else if(use_bloom_filter_)
    {
      if(OB_SUCCESS != (ret = bloom_filter_.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "fail to serialize bloomfilter. ret=%d", ret);
      }
      else
      {
        //pass bloomfilter address to postfix ....
        set_has_bloomfilter();
      }
    }
    //add e
	// success
  }
  return ret;
}

int ObSqlExpression::serialize_basic_param(char* buf, const int64_t buf_len, int64_t& pos) const
{
  int ret = OB_SUCCESS;
  ObObj obj;
  if (OB_SUCCESS == ret)
  {
    obj.set_int((int64_t)column_id_);
    if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
  }
  if (OB_SUCCESS == ret)
  {
    obj.set_int((int64_t)table_id_);
    if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
  }
  if (OB_SUCCESS == ret)
  {
    obj.set_bool(is_aggr_func_);
    if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
  }
  if (OB_SUCCESS == ret)
  {
    obj.set_bool(is_distinct_);
    if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
  }
  if (OB_SUCCESS == ret)
  {
    obj.set_int((int64_t)aggr_func_);
    if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
  }
  return ret;
}

int ObSqlExpression::deserialize_basic_param(const char* buf, const int64_t data_len, int64_t& pos)
{
  int ret = OB_SUCCESS;
  ObObj obj;
  int64_t val = 0;
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
    if (OB_SUCCESS != (ret = obj.get_int(val)))
    {
      TBSYS_LOG(WARN, "fail to get int value. ret=%d, column_id_=%lu", ret, column_id_);
    }
    else
    {
      column_id_ = (uint64_t)val;
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
    if (OB_SUCCESS != (ret = obj.get_int(val)))
    {
      TBSYS_LOG(WARN, "fail to get int value. ret=%d, table_id_=%lu", ret, table_id_);
    }
    else
    {
      table_id_ = val;
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
    if (OB_SUCCESS != (ret = obj.get_bool(is_aggr_func_)))
    {
      TBSYS_LOG(WARN, "fail to get int value. ret=%d, is_aggr_func_=%d", ret, is_aggr_func_);
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
    if (OB_SUCCESS != (ret = obj.get_bool(is_distinct_)))
    {
      TBSYS_LOG(WARN, "fail to get int value. ret=%d, is_distinct_=%d", ret, is_distinct_);
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize obj. ret=%d", ret);
    }
    if (OB_SUCCESS != (ret = obj.get_int(val)))
    {
      TBSYS_LOG(WARN, "fail to get int value. ret=%d, aggr_func_=%d", ret, aggr_func_);
    }
    else
    {
      aggr_func_ = (ObItemType)val;
    }
  }
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObSqlExpression)
{
  int64_t size = 0;
  size += get_basic_param_serialize_size();
  size += post_expr_.get_serialize_size();
  //add weixing [implementation of sub_query]20160106
  ObObj use_bloom_filter;
  use_bloom_filter.set_bool(use_bloom_filter_);
  size += use_bloom_filter.get_serialize_size();
  if(use_bloom_filter_)
  {
    size += bloom_filter_.get_serialize_size();
    TBSYS_LOG(WARN, "add bloonfilter size in get serialize size %ld",bloom_filter_.get_serialize_size());
  }
  //add e
  return size;
}

int64_t ObSqlExpression::get_basic_param_serialize_size() const
{
  int64_t size = 0;
  ObObj obj;
	obj.set_int((int64_t)column_id_);
	size += obj.get_serialize_size();
  obj.set_int((int64_t)table_id_);
	size += obj.get_serialize_size();
	obj.set_bool(is_aggr_func_);
	size += obj.get_serialize_size();
	obj.set_bool(is_distinct_);
	size += obj.get_serialize_size();
	obj.set_int((int64_t)aggr_func_);
	size += obj.get_serialize_size();
  return size;
}

int ObSqlExpressionUtil::make_column_expr(const uint64_t tid, const uint64_t cid, ObSqlExpression &expr)
{
  int ret = OB_SUCCESS;
  ExprItem item;

  item.type_ = T_REF_COLUMN;
  item.value_.cell_.tid = tid;
  item.value_.cell_.cid = cid;
  if (OB_SUCCESS != (ret = expr.add_expr_item(item)))
  {
    TBSYS_LOG(WARN, "fail to add expr item. ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = expr.add_expr_item_end()))
  {
    TBSYS_LOG(WARN, "fail to add expr item. ret=%d", ret);
  }
  return ret;
}

//static ObCachedAllocator<ObSqlExpression> SQL_EXPR_ALLOC;  //delete by zt, 20160419
static ObCachedAllocator<ObSqlExpression> SQL_EXPR_ALLOC[16]; //add by zt, 20160419
static volatile uint64_t ALLOC_TIMES = 0;
static volatile uint64_t FREE_TIMES = 0;

//add by zt, b
namespace oceanbase{
  namespace sql{
  #include <sys/syscall.h>
  long int gettid()
  {
    return syscall(SYS_gettid);
  }
  //add by zt, e

  ObSqlExpression* ObSqlExpression::alloc()
  {
    //add by zt, b
    int64_t tid = (gettid() & 0xf); //redistribute to different alloc
    ObSqlExpression *ret = SQL_EXPR_ALLOC[tid].alloc();
    //add by zt, e
    //  ObSqlExpression *ret = SQL_EXPR_ALLOC.alloc(); //delete by zt
    if (OB_UNLIKELY(NULL == ret))
    {
      TBSYS_LOG(ERROR, "failed to allocate expression object");
    }
    else
    {
      atomic_inc(&ALLOC_TIMES);
    }
    if (ALLOC_TIMES % 1000000 == 0)
    {
      TBSYS_LOG(INFO, "[EXPR] alloc %p, times=%ld cached=%d alloc_num=%d",
                ret, ALLOC_TIMES, SQL_EXPR_ALLOC[tid].get_cached_count(), SQL_EXPR_ALLOC[tid].get_allocated_count());
      ob_print_phy_operator_stat();
    }
    return ret;
  }

  void ObSqlExpression::free(ObSqlExpression* ptr)
  {
    //add by zt, b
    int64_t tid = (gettid() & 0xf);
    SQL_EXPR_ALLOC[tid].free(ptr);
    //add by zt, e
    //  SQL_EXPR_ALLOC.free(ptr); //delete by zt
    atomic_inc(&FREE_TIMES);
    if (FREE_TIMES % 1000000 == 0)
    {
      TBSYS_LOG(INFO, "[EXPR] free %p, times=%ld cached=%d alloc_num=%d",
                ptr, FREE_TIMES, SQL_EXPR_ALLOC[tid].get_cached_count(), SQL_EXPR_ALLOC[tid].get_allocated_count());
    }
  }
}
}
