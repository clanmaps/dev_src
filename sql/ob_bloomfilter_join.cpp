/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_bloomfilter_join.cpp
* @brief for operations of bloomfilter join
*
* Created by maoxiaoxiao:do bloomfilter join
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_07_27
*/

#include "ob_bloomfilter_join.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

ObBloomFilterJoin::ObBloomFilterJoin()
  :get_next_row_func_(NULL),
    last_left_row_(NULL),
    last_right_row_(NULL),
    right_cache_is_valid_(false),
    is_right_iter_end_(false),
    bloom_filter_(NULL),
    use_bloom_filter_(false),
    table_filter_expr_(NULL)
{
}

ObBloomFilterJoin::~ObBloomFilterJoin()
{
  char *store_buf = last_join_left_row_store_.ptr();
  if (NULL != store_buf)
  {
    ob_free(store_buf);
    last_join_left_row_store_.assign_ptr(NULL, 0);
  }
}

void ObBloomFilterJoin::reset()
{
  get_next_row_func_ = NULL;
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  char *store_buf = last_join_left_row_store_.ptr();
  if (NULL != store_buf)
  {
    ob_free(store_buf);
    last_join_left_row_store_.assign_ptr(NULL, 0);
  }
  right_cache_.clear();
  row_desc_.reset();
  right_cache_is_valid_ = false;
  is_right_iter_end_ = false;
  equal_join_conds_.clear();
  other_join_conds_.clear();
  left_op_ = NULL;
  right_op_ = NULL;

  bloom_filter_ = NULL;
  use_bloom_filter_ = false;
  ObSqlExpression* ptr = NULL;
  table_filter_expr_->free(ptr);
}

void ObBloomFilterJoin::reuse()
{
  get_next_row_func_ = NULL;
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  char *store_buf = last_join_left_row_store_.ptr();
  if (NULL != store_buf)
  {
    ob_free(store_buf);
    last_join_left_row_store_.assign_ptr(NULL, 0);
  }
  right_cache_.reuse();
  row_desc_.reset();
  right_cache_is_valid_ = false;
  is_right_iter_end_ = false;
  equal_join_conds_.clear();
  other_join_conds_.clear();
  left_op_ = NULL;
  right_op_ = NULL;

  bloom_filter_ = NULL;
  use_bloom_filter_ = false;
  ObSqlExpression* ptr = NULL;
  table_filter_expr_->free(ptr);
}

int ObBloomFilterJoin::open()
{
  int ret = OB_SUCCESS;
  const ObRowDesc *left_row_desc = NULL;
  const ObRowDesc *right_row_desc = NULL;
  char *store_buf = NULL;
  int64_t equal_join_conds_count = equal_join_conds_.count();

  if (equal_join_conds_count <= 0)
  {
    TBSYS_LOG(WARN, "bloomfilter join can not work without equijoin conditions");
    ret = OB_NOT_SUPPORTED;
    return ret;
  }
  /*1.打开前表运算符*/
  else if (OB_SUCCESS != (ret = left_op_->open()))
  {
    TBSYS_LOG(WARN, "failed to open child ops, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = left_op_->get_row_desc(left_row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get child row desc, err=%d", ret);
  }
  if(OB_SUCCESS == ret)
  {
    table_filter_expr_ = ObSqlExpression::alloc();
    table_filter_expr_->set_has_bloomfilter();
    table_filter_expr_->get_bloom_filter(bloom_filter_);
    if(OB_SUCCESS != (ret = bloom_filter_->init(BLOOMFILTER_ELEMENT_NUM)))
    {
      TBSYS_LOG(WARN,"Problem initialize bloom filter");
    }

    ExprItem item1,item2,item3,item4,item5;
    item1.type_ = T_REF_COLUMN;
    for (int64_t i = 0; i < equal_join_conds_count; i++)
    {
      const ObSqlExpression &expr = equal_join_conds_.at(i);
      ExprItem::SqlCellInfo c1;
      ExprItem::SqlCellInfo c2;
      if (expr.is_equijoin_cond(c1, c2))
      {
        item1.value_.cell_.tid = c2.tid;
        item1.value_.cell_.cid = c2.cid;
        table_filter_expr_->add_expr_item(item1);
      }
    }

    item2.type_ = T_OP_ROW;
    item2.data_type_ = ObMinType;
    item2.value_.int_ = equal_join_conds_.count();

    item3.type_ = T_OP_LEFT_PARAM_END;
    item3.data_type_ = ObMinType;
    item3.value_.int_ = 2;

    table_filter_expr_->add_expr_item(item2);
    table_filter_expr_->add_expr_item(item3);

    /*modify maoxx [bloomfilter_join] 20160722*/
//    item4.type_ = T_REF_QUERY;
//    item4.data_type_ = ObMinType;
//    item4.value_.int_ = 1;
    ObPostfixExpression *pf_expr = table_filter_expr_->get_decoded_expression_v3();
    ObSEArray<ObObj, BASIC_SYMBOL_COUNT> *expr = pf_expr->get_expr_v2();
    ObObj item4_type;
    ObObj obj;
    item4_type.set_int(ObPostfixExpression::QUERY_ID);
    obj.set_int(1);
    if(OB_SUCCESS != (ret = expr->push_back(item4_type)))
    {
      TBSYS_LOG(ERROR,"table_filter_expr_ push back item_type failed");
    }
    if(OB_SUCCESS != (ret = expr->push_back(obj)))
    {
      TBSYS_LOG(ERROR,"table_filter_expr_ push back obj failed");
    }

    item5.type_ = T_OP_IN;
    item5.data_type_ = ObMinType;
    item5.value_.int_ = 2;

//    table_filter_expr_->add_expr_item(item4);
    table_filter_expr_->add_expr_item(item5);
    table_filter_expr_->add_expr_item_end();
    /*modify e*/

    //迭代前表数据，构建Bloomfilter
    const ObRow *row = NULL;
    while (OB_SUCCESS == (ret = left_op_->get_next_row(row)))
    {
      ObObj obj_array[equal_join_conds_count];
      ObRow *temp_row = const_cast<ObRow *>(row);
      for (int64_t i = 0; i < equal_join_conds_count; ++i)
      {
        const ObSqlExpression &expr = equal_join_conds_.at(i);
        ExprItem::SqlCellInfo c1;
        ExprItem::SqlCellInfo c2;
        if (expr.is_equijoin_cond(c1, c2))
        {
          ObObj *obj = NULL;
          if (OB_SUCCESS != (ret = temp_row->get_cell(c1.tid, c1.cid, obj)))
          {
            TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
            break;
          }
          else
          {
            obj_array[i] = *obj;
          }
        }
      }

      ObRowkey rowkey;
      rowkey.assign(obj_array, equal_join_conds_count);
      bloom_filter_->insert(rowkey);

      const ObRowStore::StoredRow *stored_row = NULL;
      left_row_store_.add_row(*row, stored_row);
    }

    if(use_bloom_filter_)
    {
      ObSingleChildPhyOperator *sort_query =  dynamic_cast<ObSingleChildPhyOperator *>(get_child(1));
      ObTableRpcScan * main_query = dynamic_cast<ObTableRpcScan *>(sort_query->get_child(0));
      if(NULL != main_query)
        main_query->add_filter(table_filter_expr_);
    }
  }
  if (OB_SUCCESS != (ret = right_op_->open()))
  {
    TBSYS_LOG(WARN, "failed to open right_op_ operator(s), err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = right_op_->get_row_desc(right_row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get child row desc, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = cons_row_desc(*left_row_desc, *right_row_desc)))
  {
    TBSYS_LOG(WARN, "failed to cons row desc, err=%d", ret);
  }
  else if (NULL == (store_buf = static_cast<char*>(ob_malloc(MAX_SINGLE_ROW_SIZE, ObModIds::OB_SQL_MERGE_JOIN))))
  {
    TBSYS_LOG(ERROR, "no memory");
    ret = OB_ALLOCATE_MEMORY_FAILED;
  }
  else
  {
    OB_ASSERT(left_row_desc);
    OB_ASSERT(right_row_desc);
    curr_row_.set_row_desc(row_desc_);
    left_row_.set_row_desc(*left_row_desc);
    curr_cached_right_row_.set_row_desc(*right_row_desc);

    last_left_row_ = NULL;
    last_right_row_ = NULL;
    right_cache_is_valid_ = false;
    is_right_iter_end_ = false;
    last_join_left_row_store_.assign_buffer(store_buf, MAX_SINGLE_ROW_SIZE);
  }
  return ret;
}

int ObBloomFilterJoin::close()
{
  int ret = OB_SUCCESS;
  char *store_buf = last_join_left_row_store_.ptr();
  if (NULL != store_buf)
  {
    ob_free(store_buf);
    last_join_left_row_store_.assign_ptr(NULL, 0);
  }
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  right_cache_is_valid_ = false;
  is_right_iter_end_ = false;
  row_desc_.reset();

  bloom_filter_ = NULL;
  use_bloom_filter_ = false;
  ObSqlExpression* ptr = NULL;
  table_filter_expr_->free(ptr);

  ret = ObJoin::close();
  return ret;
}

int ObBloomFilterJoin::set_join_type(const ObJoin::JoinType join_type)
{
  int ret = OB_SUCCESS;
  ObJoin::set_join_type(join_type);
  switch(join_type)
  {
  case INNER_JOIN:
    get_next_row_func_ = &ObBloomFilterJoin::inner_get_next_row;
    use_bloom_filter_ = true;
    break;
  case LEFT_OUTER_JOIN:
    get_next_row_func_ = &ObBloomFilterJoin::left_outer_get_next_row;
    use_bloom_filter_ = true;
    break;
  default:
    ret = OB_ERR_UNEXPECTED;
    break;
  }
  return ret;
}

int ObBloomFilterJoin::get_next_row(const ObRow *&row)
{
  OB_ASSERT(get_next_row_func_);
  return (this->*(this->ObBloomFilterJoin::get_next_row_func_))(row);
}

int ObBloomFilterJoin::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(0 >= row_desc_.get_column_num()))
  {
    TBSYS_LOG(ERROR, "not init");
    ret = OB_NOT_INIT;
  }
  else
  {
    row_desc = &row_desc_;
  }
  return ret;
}

int64_t ObBloomFilterJoin::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "BloomFilter ");
  pos += ObJoin::to_string(buf + pos, buf_len - pos);
  return pos;
}

//INNER JOIN
int ObBloomFilterJoin::inner_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  //const ObRow *left_row = NULL;
  const ObRow *right_row = NULL;
  // fetch the next left row
  if (NULL != last_left_row_)
  {
    left_row_ = *last_left_row_;
    last_left_row_ = NULL;
  }
  else
  {
    ret = left_row_store_.get_next_row(left_row_);
  }

  while(OB_SUCCESS == ret)
  {
    if (right_cache_is_valid_)
    {
      OB_ASSERT(!right_cache_.is_empty());
      int cmp = 0;
      if (OB_SUCCESS != (ret = left_row_compare_equijoin_cond(left_row_, last_join_left_row_, cmp)))
      {
        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
        break;
      }
      if (0 == cmp)
      {
        // fetch the next right row from right_cache
        if (OB_SUCCESS != (ret = right_cache_.get_next_row(curr_cached_right_row_)))
        {
          if (OB_UNLIKELY(OB_ITER_END != ret))
          {
            TBSYS_LOG(WARN, "failed to get next row from right_cache, err=%d", ret);
          }
          else
          {
            right_cache_.reset_iterator(); // continue
            // fetch the next left row
            ret = left_row_store_.get_next_row(left_row_);
          }
        }
        else
        {
          bool is_qualified = false;
          if (OB_SUCCESS != (ret = join_rows(left_row_, curr_cached_right_row_)))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
          {
            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
          }
          else if (is_qualified)
          {
            // output
            row = &curr_row_;
            last_left_row_ = &left_row_;
            break;
          }
          else
          {
            // continue with the next cached right row
            OB_ASSERT(NULL == last_left_row_);
            //OB_ASSERT(NULL != left_row);
          }
        }
      }
      else
      {
        // left_row > last_join_left_row_ on euqijoin conditions
        right_cache_is_valid_ = false;
        right_cache_.clear();
      }
    }
    else
    {
      // fetch the next right row
      if (OB_UNLIKELY(is_right_iter_end_))
      {
        ret = OB_ITER_END;
        break;
      }
      else if (NULL != last_right_row_)
      {
        right_row = last_right_row_;
        last_right_row_ = NULL;
      }
      else
      {
        ret = right_op_->get_next_row(right_row);
        if (OB_SUCCESS != ret)
        {
          if (OB_ITER_END == ret)
          {
            TBSYS_LOG(DEBUG, "end of right child op");
            is_right_iter_end_ = true;
            if (!right_cache_.is_empty())
            {
              // no more right rows and the right cache is not empty, we SHOULD look at the next left row
              right_cache_is_valid_ = true;
              OB_ASSERT(NULL == last_right_row_);
              OB_ASSERT(NULL == last_left_row_);
              ret = left_row_store_.get_next_row(left_row_);
            }
            continue;
          }
          else
          {
            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
            break;
          }
        }
      }
      //OB_ASSERT(left_row);
      OB_ASSERT(right_row);
      int cmp = 0;
      if (OB_SUCCESS != (ret = compare_equijoin_cond(left_row_, *right_row, cmp)))
      {
        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
        break;
      }
      if (0 == cmp)
      {
        if (right_cache_.is_empty())
        {
          // store the joined left row
          last_join_left_row_store_.assign_buffer(last_join_left_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
          if (OB_SUCCESS != (ret = ObRowUtil::convert(left_row_, last_join_left_row_store_, last_join_left_row_)))
          {
            TBSYS_LOG(WARN, "failed to store left row, err=%d", ret);
            break;
          }
        }
        bool is_qualified = false;
        const ObRowStore::StoredRow *stored_row = NULL;
        if (OB_SUCCESS != (ret = right_cache_.add_row(*right_row, stored_row)))
        {
          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = join_rows(left_row_, *right_row)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          last_left_row_ = &left_row_;
          OB_ASSERT(NULL == last_right_row_);
          break;
        }
        else
        {
          // continue with the next right row
          //OB_ASSERT(NULL != left_row);
          OB_ASSERT(NULL == last_left_row_);
          OB_ASSERT(NULL == last_right_row_);
        }
      } // end 0 == cmp
      else if (cmp < 0)
      {
        // left_row < right_row on equijoin conditions
        if (!right_cache_.is_empty())
        {
          right_cache_is_valid_ = true;
        }
        last_right_row_ = right_row;
        OB_ASSERT(NULL == last_left_row_);
        ret = left_row_store_.get_next_row(left_row_);
      }
      else
      {
        // left_row > right_row on euqijoin conditions
        // continue with the next right row
        //OB_ASSERT(NULL != left_row);
        OB_ASSERT(NULL == last_left_row_);
        OB_ASSERT(NULL == last_right_row_);
      }
    }
  } // end while
  return ret;
}

//LEFT JOIN
int ObBloomFilterJoin::left_outer_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  //const ObRow *left_row = NULL;
  const ObRow *right_row = NULL;
  // fetch the next left row
  if (NULL != last_left_row_)
  {
    left_row_ = *last_left_row_;
    last_left_row_ = NULL;
  }
  else
  {
    ret = left_row_store_.get_next_row(left_row_);
  }

  while(OB_SUCCESS == ret)
  {
    if (right_cache_is_valid_)
    {
      OB_ASSERT(!right_cache_.is_empty());
      int cmp = 0;
      if (OB_SUCCESS != (ret = left_row_compare_equijoin_cond(left_row_, last_join_left_row_, cmp)))
      {
        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
        break;
      }
      if (0 == cmp)
      {
        // fetch the next right row from right_cache
        if (OB_SUCCESS != (ret = right_cache_.get_next_row(curr_cached_right_row_)))
        {
          if (OB_UNLIKELY(OB_ITER_END != ret))
          {
            TBSYS_LOG(WARN, "failed to get next row from right_cache, err=%d", ret);
          }
          else
          {
            right_cache_.reset_iterator(); // continue
            // fetch the next left row
            ret = left_row_store_.get_next_row(left_row_);
          }
        }
        else
        {
          bool is_qualified = false;
          if (OB_SUCCESS != (ret = join_rows(left_row_, curr_cached_right_row_)))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
          {
            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
          }
          else if (is_qualified)
          {
            // output
            row = &curr_row_;
            last_left_row_ = &left_row_;
            break;
          }
          else
          {
            // continue with the next cached right row
            OB_ASSERT(NULL == last_left_row_);
            //OB_ASSERT(NULL != left_row);
          }
        }
      }
      else
      {
        // left_row > last_join_left_row_ on euqijoin conditions
        right_cache_is_valid_ = false;
        right_cache_.clear();
      }
    }
    else
    {
      // fetch the next right row
      if (OB_UNLIKELY(is_right_iter_end_))
      {
        // no more right rows, but there are left rows left
        //OB_ASSERT(left_row);
        bool is_qualified = false;
        if (OB_SUCCESS != (ret = left_join_rows(left_row_)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          OB_ASSERT(NULL == last_left_row_);
          OB_ASSERT(NULL == last_right_row_);
          break;
        }
        else
        {
          // continue with the next left row
          OB_ASSERT(NULL == last_left_row_);
          OB_ASSERT(NULL == last_right_row_);
          ret = left_row_store_.get_next_row(left_row_);
          continue;
        }
      }
      else if (NULL != last_right_row_)
      {
        right_row = last_right_row_;
        last_right_row_ = NULL;
      }
      else
      {
        ret = right_op_->get_next_row(right_row);
        if (OB_SUCCESS != ret)
        {
          if (OB_ITER_END == ret)
          {
            TBSYS_LOG(INFO, "end of right child op");
            is_right_iter_end_ = true;
            if (!right_cache_.is_empty())
            {
              // no more right rows and the right cache is not empty, we SHOULD look at the next left row
              right_cache_is_valid_ = true;
              OB_ASSERT(NULL == last_right_row_);
              OB_ASSERT(NULL == last_left_row_);
              ret = left_row_store_.get_next_row(left_row_);
            }
            else
            {
              ret = OB_SUCCESS;
            }
            continue;
          }
          else
          {
            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
            break;
          }
        }
      }
      //OB_ASSERT(left_row);
      OB_ASSERT(right_row);
      int cmp = 0;
      if (OB_SUCCESS != (ret = compare_equijoin_cond(left_row_, *right_row, cmp)))
      {
        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
        break;
      }
      if (0 == cmp)
      {
        if (right_cache_.is_empty())
        {
          // store the joined left row
          last_join_left_row_store_.assign_buffer(last_join_left_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
          if (OB_SUCCESS != (ret = ObRowUtil::convert(left_row_, last_join_left_row_store_, last_join_left_row_)))
          {
            TBSYS_LOG(WARN, "failed to store left row, err=%d", ret);
            break;
          }
        }
        bool is_qualified = false;
        const ObRowStore::StoredRow *stored_row = NULL;
        if (OB_SUCCESS != (ret = right_cache_.add_row(*right_row, stored_row)))
        {
          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = join_rows(left_row_, *right_row)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          last_left_row_ = &left_row_;
          OB_ASSERT(NULL == last_right_row_);
          break;
        }
        else
        {
          // continue with the next right row
          //OB_ASSERT(NULL != left_row);
          OB_ASSERT(NULL == last_left_row_);
          OB_ASSERT(NULL == last_right_row_);
        }
      } // end 0 == cmp
      else if (cmp < 0)
      {
        // left_row < right_row on equijoin conditions
        if (!right_cache_.is_empty())
        {
          right_cache_is_valid_ = true;
          OB_ASSERT(NULL == last_left_row_);
          last_right_row_ = right_row;
          ret = left_row_store_.get_next_row(left_row_);
        }
        else
        {
          bool is_qualified = false;
          if (OB_SUCCESS != (ret = left_join_rows(left_row_)))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
          {
            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
          }
          else if (is_qualified)
          {
            // output
            row = &curr_row_;
            OB_ASSERT(NULL == last_left_row_);
            last_right_row_ = right_row;
            break;
          }
          else
          {
            // continue with the next left row
            OB_ASSERT(NULL == last_left_row_);
            ret = left_row_store_.get_next_row(left_row_);
            last_right_row_ = right_row;
          }
        }
      }
      else
      {
        // left_row > right_row on euqijoin conditions
        // continue with the next right row
        //OB_ASSERT(NULL != left_row);
        OB_ASSERT(NULL == last_left_row_);
        OB_ASSERT(NULL == last_right_row_);
      }
    }
  } // end while
  return ret;
}

int ObBloomFilterJoin::compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const
{
  int ret = OB_SUCCESS;
  cmp = 0;
  const ObObj *res1 = NULL;
  const ObObj *res2 = NULL;
  ObExprObj obj1;
  ObExprObj obj2;
  for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
  {
    const ObSqlExpression &expr = equal_join_conds_.at(i);
    ExprItem::SqlCellInfo c1;
    ExprItem::SqlCellInfo c2;
    if (expr.is_equijoin_cond(c1, c2))
    {
      if (OB_SUCCESS != (ret = r1.get_cell(c1.tid, c1.cid, res1)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, res2)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
        break;
      }
      else
      {
        obj1.assign(*res1);
        obj2.assign(*res2);
        if (OB_SUCCESS != obj1.compare(obj2, cmp))
        {
          if (obj1.is_null())
          {
            // NULL vs obj2
            cmp = -10;
          }
          else
          {
            // obj1 cmp NULL
            cmp = 10;
          }
        }
        else if (0 != cmp)
        {
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "invalid equijoin condition");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
  }
  return ret;
}

int ObBloomFilterJoin::left_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const
{
  int ret = OB_SUCCESS;
  cmp = 0;
  const ObObj *res1 = NULL;
  const ObObj *res2 = NULL;
  ObExprObj obj1;
  ObExprObj obj2;
  for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
  {
    const ObSqlExpression &expr = equal_join_conds_.at(i);
    ExprItem::SqlCellInfo c1;
    ExprItem::SqlCellInfo c2;
    if (expr.is_equijoin_cond(c1, c2))
    {
      if (OB_SUCCESS != (ret = r1.get_cell(c1.tid, c1.cid, res1)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else if (OB_SUCCESS != (ret = r2.get_cell(c1.tid, c1.cid, res2)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else
      {
        obj1.assign(*res1);
        obj2.assign(*res2);
        if (OB_SUCCESS != obj1.compare(obj2, cmp))
        {
          cmp = -10;            // @todo NULL
        }
        else if (0 != cmp)
        {
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "invalid equijoin condition");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
  }
  return ret;
}

int ObBloomFilterJoin::curr_row_is_qualified(bool &is_qualified)
{
  int ret = OB_SUCCESS;
  is_qualified = true;
  const ObObj *res = NULL;
  for (int64_t i = 0; i < other_join_conds_.count(); ++i)
  {
    ObSqlExpression &expr = other_join_conds_.at(i);
    if (OB_SUCCESS != (ret = expr.calc(curr_row_, res)))
    {
      TBSYS_LOG(WARN, "failed to calc expr, err=%d", ret);
    }
    else if (!res->is_true())
    {
      is_qualified = false;
      break;
    }
  }
  return ret;
}

int ObBloomFilterJoin::cons_row_desc(const ObRowDesc &rd1, const ObRowDesc &rd2)
{
  int ret = OB_SUCCESS;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  for (int64_t i = 0; i < rd1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = rd1.get_tid_cid(i, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = row_desc_.add_column_desc(tid, cid)))
    {
      TBSYS_LOG(WARN, "failed to add column desc, err=%d", ret);
      break;
    }
  }
  for (int64_t i = 0; OB_SUCCESS == ret && i < rd2.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = rd2.get_tid_cid(i, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch");
    }
    else if (OB_SUCCESS != (ret = row_desc_.add_column_desc(tid, cid)))
    {
      TBSYS_LOG(WARN, "failed to add column desc, err=%d", ret);
    }
  }
  return ret;
}

int ObBloomFilterJoin::join_rows(const ObRow& r1, const ObRow& r2)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t i = 0;
  for (; i < r1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = r1.raw_get_cell(i, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  } // end for
  for (int64_t j = 0; OB_SUCCESS == ret && j < r2.get_column_num(); ++j)
  {
    if (OB_SUCCESS != (ret = r2.raw_get_cell(j, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i+j, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  } // end for
  return ret;
}

int ObBloomFilterJoin::left_join_rows(const ObRow& r1)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t i = 0;
  for (; i < r1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = r1.raw_get_cell(i, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  } // end for
  int64_t right_row_column_num = row_desc_.get_column_num() - r1.get_column_num();
  ObObj null_cell;
  null_cell.set_null();
  for (int64_t j = 0; OB_SUCCESS == ret && j < right_row_column_num; ++j)
  {
    if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i+j, null_cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  } // end for
  return ret;
}

int ObBloomFilterJoin::right_join_rows(const ObRow& r2)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t left_row_column_num = row_desc_.get_column_num() - r2.get_column_num();
  ObObj null_cell;
  null_cell.set_null();
  for (int64_t i = 0; i < left_row_column_num; ++i)
  {
    if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, null_cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  } // end for
  for (int64_t j = 0; OB_SUCCESS == ret && j < r2.get_column_num(); ++j)
  {
    if (OB_SUCCESS != (ret = r2.raw_get_cell(j, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(left_row_column_num+j, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  } // end for
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObBloomFilterJoin, PHY_BLOOMFILTER_JOIN);
  }
}

PHY_OPERATOR_ASSIGN(ObBloomFilterJoin)
{
  int ret = OB_SUCCESS;
  CAST_TO_INHERITANCE(ObBloomFilterJoin);
  reset();
  if ((ret = set_join_type(o_ptr->join_type_)) != OB_SUCCESS)
  {
    TBSYS_LOG(WARN, "Fail to set join type, ret=%d", ret);
  }
  else
  {
    ret = ObJoin::assign(other);
  }
  return ret;
}
