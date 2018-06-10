/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_in_memory_cursor.cpp
* @brief this class  is a cursor helper implement
*
* Created by zhounan: support cursor
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#include "ob_in_memory_cursor.h"
#include "common/ob_row_util.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObInMemoryCursor::ObInMemoryCursor()
  :cursor_array_get_pos_(0), row_desc_(NULL)
{
}

ObInMemoryCursor::~ObInMemoryCursor()
{
}

void ObInMemoryCursor::reset()
{
  row_store_.clear();
  cursor_array_.clear();
  cursor_array_get_pos_ = 0;
  row_desc_ = NULL;
}

void ObInMemoryCursor::reuse()
{
  row_store_.clear();
  cursor_array_.clear();
  cursor_array_get_pos_ = 0;
  row_desc_ = NULL;
}

int ObInMemoryCursor::add_row(const common::ObRow &row)
{
  int ret = OB_SUCCESS;
  const common::ObRowStore::StoredRow* stored_row = NULL;
  if (OB_SUCCESS != (ret = row_store_.add_row(row, stored_row)))
  {
    TBSYS_LOG(WARN, "failed to add row into row_store, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = cursor_array_.push_back(stored_row)))
  {
    TBSYS_LOG(WARN, "failed to push back to array, err=%d", ret);
  }
  else if (NULL == row_desc_)
  {
    row_desc_ = row.get_row_desc();
    TBSYS_LOG(DEBUG, "set row desc=%p col_num=%ld", row_desc_, row_desc_->get_column_num());
  }
  return ret;
}



int ObInMemoryCursor::get_next_row(common::ObRow &row)
{
  int ret = OB_SUCCESS;
  if (cursor_array_get_pos_ >= cursor_array_.count())
  {
    ret = OB_ITER_END;
    TBSYS_LOG(DEBUG, "end of the in-memory run");
  }
  else
  {
    OB_ASSERT(row_desc_);
    row.set_row_desc(*row_desc_);
    if (OB_SUCCESS != (ret = common::ObRowUtil::convert(cursor_array_.at(
              static_cast<int32_t>(cursor_array_get_pos_))->get_compact_row(), row)))
    {
      TBSYS_LOG(WARN, "failed to convert row, err=%d", ret);
    }
    else
    {
      ++cursor_array_get_pos_;
    }
  }
  return ret;
}

int ObInMemoryCursor::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS == (ret = get_next_row(curr_row_)))
  {
    row = &curr_row_;
  }
  return ret;
}

int ObInMemoryCursor::get_next_compact_row(ObString &compact_row)
{
  int ret = OB_SUCCESS;
  if (cursor_array_get_pos_ >= cursor_array_.count())
  {
    ret = OB_ITER_END;
    TBSYS_LOG(INFO, "end of the in-memory run");
  }
  else
  {
    compact_row = cursor_array_.at(static_cast<int32_t>(cursor_array_get_pos_))->get_compact_row();
    ++cursor_array_get_pos_;
  }
  return ret;
}

int64_t ObInMemoryCursor::get_row_count() const
{
  return cursor_array_.count();
}

int64_t ObInMemoryCursor::get_used_mem_size() const
{
  return row_store_.get_used_mem_size() + cursor_array_.count()*sizeof(void*);
}




