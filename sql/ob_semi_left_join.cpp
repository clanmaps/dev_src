/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file     ob_semi_left_join.cpp
 * @brief    semi left join operator
 * created by yu shengjuan: sort row from child_op get_next_row(),use std::sort it will use at logical plan transform to physical plan
 * @version  CEDAR 0.2 
 * @author   yu shengjuan <51141500090@ecnu.cn>
 * @date     2015_08_29
 */
//add yushengjuan [semi_join] [0.1] 20150829:b
#include "common/ob_define.h"
#include "common/ob_malloc.h"
#include "common/ob_row.h"
#include "common/ob_row_util.h"
#include "ob_semi_left_join.h"
#include "ob_physical_plan.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObSemiLeftJoin::ObSemiLeftJoin()
  :got_first_row_(false)
{
}

ObSemiLeftJoin::~ObSemiLeftJoin()
{
}

void ObSemiLeftJoin::reset()
{
  got_first_row_ = false;
  sort_columns_.clear();
  sorted_array_.clear();
  left_table_element_stored_.clear();
  in_mem_sort_.reset();
  ObSingleChildPhyOperator::reset();
}

void ObSemiLeftJoin::reuse()
{
  got_first_row_ = false;
  sort_columns_.clear();
  sorted_array_.clear();
  left_table_element_stored_.clear();
  in_mem_sort_.reuse();
  ObSingleChildPhyOperator::reuse();
}

int ObSemiLeftJoin::open()
{
  int ret = OB_SUCCESS;
  if(OB_SUCCESS != (ret = ObSingleChildPhyOperator::open()))
  {
    TBSYS_LOG(WARN,"failed to open child_op, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = do_sort()))
  {
    TBSYS_LOG(WARN, "failed to sort input data, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = do_distinct()))
  {
    TBSYS_LOG(WARN, "failed to , err=%d", ret);
  }
  return ret;
}

int ObSemiLeftJoin::colse()
{
  int ret = OB_SUCCESS;
  in_mem_sort_.reset();
  ret = ObSingleChildPhyOperator::close();
  return ret;
}

int ObSemiLeftJoin::get_next_row(const common::ObRow *&row)
{
  return in_mem_sort_.get_next_row(row);
}

int ObSemiLeftJoin::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(NULL == child_op_))
  {
    TBSYS_LOG(ERROR, "child op is NULL");
    ret = OB_NOT_INIT;
  }
  else
  {
    ret = child_op_->get_row_desc(row_desc);
  }
  return ret;
}

int ObSemiLeftJoin::set_sort_columns(uint64_t tab_id_, uint64_t col_id_)
{
  int ret = OB_SUCCESS;
  ObSortColumn sort_column;
  sort_column.table_id_ = tab_id_;
  sort_column.column_id_ = col_id_;
  sort_column.is_ascending_ = true;
  if(OB_SUCCESS != (ret = sort_columns_.push_back(sort_column)))
  {
    TBSYS_LOG(WARN,"failed to add reserved column, err = %d", ret);
  }
  return ret;
}

//add fanqiushi [semi_join] [0.1] 20150829:b
int64_t ObSemiLeftJoin::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObSemiLeftJoin(table_id_=%lu c_id_=%lu", sort_columns_.at(0).table_id_,sort_columns_.at(0).column_id_);
  //pos += insert_values_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, ")\n");
  if (NULL != child_op_)
  {
    pos += child_op_->to_string(buf+pos, buf_len-pos);
  }
  return pos;
}
//add:e

int ObSemiLeftJoin::do_sort()
{
  int ret = OB_SUCCESS;
  const common::ObRow *input_row = NULL;
  if (OB_SUCCESS != (ret = in_mem_sort_.set_sort_columns(sort_columns_)))
  {
    TBSYS_LOG(WARN, "fail to set sort columns for in_mem_sort. ret=%d", ret);
  }
  else
  {
    while(OB_SUCCESS == ret && OB_SUCCESS == (ret = child_op_->get_next_row(input_row)))
    {
      //TBSYS_LOG(ERROR,"test::yusj the input row to_string is %s",to_cstring(*input_row));
      if(OB_SUCCESS != (ret = in_mem_sort_.add_row(*input_row)))
      {
        TBSYS_LOG(WARN,"failed to add row, err=%d",ret);
      }
    }//end while
    if(OB_ITER_END == ret)
    {
      ret = OB_SUCCESS;
    }
    if(OB_SUCCESS == ret)
    {
      //TBSYS_LOG(WARN,"test::yusj enter into ObSemiLeftJoin sort_rows()");
      if(OB_SUCCESS != (ret = in_mem_sort_.sort_rows()))
      {
        TBSYS_LOG(WARN,"failed to sort,err=%d",ret);
      }
    }
  }
  return ret;
}

int ObSemiLeftJoin::do_distinct()
{
  int ret = OB_SUCCESS;
  const common::ObRowStore::StoredRow *store_row_ = NULL;
  bool got_distinct_row = false;
  sorted_array_ = in_mem_sort_.get_sorted_element();
  for(int64_t i = 0;i < sorted_array_.count();i++)
  {
    if(got_first_row_)
    {
      if(OB_SUCCESS == (ret = compare_equal(sorted_array_.at(i-1),sorted_array_.at(i),got_distinct_row)))
      {
        if(got_distinct_row)
        {
          store_row_ = sorted_array_.at(i);
          const common::ObObj *stored_obj = NULL;
          stored_obj = store_row_->reserved_cells_;
          if(OB_SUCCESS != (ret = left_table_element_stored_.push_back(*stored_obj)))
          {
            TBSYS_LOG(WARN,"push back the store_row_ into left_table_element_stored_ error, ret=%d",ret);
          }
        }
        else
        {
        }
      }
    }
    else
    {
      store_row_ = sorted_array_.at(i);
      const common::ObObj *stored_obj = NULL;
      stored_obj = store_row_->reserved_cells_;
      if(OB_SUCCESS != (ret = left_table_element_stored_.push_back(*stored_obj)))
      {
        TBSYS_LOG(WARN,"push back the first row into left_table_element_stored_ error, ret=%d",ret);
      }
      got_first_row_ = true;
    }
  }
  return ret;
}

int ObSemiLeftJoin::compare_equal(const common::ObRowStore::StoredRow* this_row, const common::ObRowStore::StoredRow* last_row, bool &result) const
{
  int ret = OB_SUCCESS;
  const ObObj *this_obj = NULL;
  const ObObj *last_obj = NULL;
  bool cmp_val = false;

  result = false;

  if(NULL == this_row || NULL == last_row)
  {
    TBSYS_LOG(WARN, "compared row invalid. this_row=%p, last_row=%p", this_row, last_row);
    ret = OB_INVALID_ARGUMENT;
  }
  else
  {
    this_obj = this_row->reserved_cells_;
    last_obj = last_row->reserved_cells_;
    if(NULL == this_obj || NULL == last_obj)
    {
      TBSYS_LOG(WARN, "compared Obj is null. this_obj=%p, last_obj=%p", this_row, last_row);
      ret = OB_ERROR;
    }
  }

  if(OB_SUCCESS ==ret && ((*this_obj) != (*last_obj)))
  {
    cmp_val = true;
    result = cmp_val;
  }
  return ret;
}
//add:e
