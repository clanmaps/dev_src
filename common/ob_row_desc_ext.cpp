/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_row_desc_ext.cpp
 * @brief the ObRowDescExt class definition
 *
 * modified by zhutao:add deserialize and serialize functions
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_29
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
 * ob_row_desc_ext.cpp
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#include "ob_row_desc_ext.h"
using namespace oceanbase::common;

ObRowDescExt::ObRowDescExt()
{
}

ObRowDescExt::~ObRowDescExt()
{
}

ObRowDescExt::ObRowDescExt(const ObRowDescExt &other)
{
  *this = other;
}

void ObRowDescExt::reset()
{
  row_desc_.reset();
  data_type_.clear();
}

ObRowDescExt& ObRowDescExt::operator=(const ObRowDescExt &other)
{
  if (this != &other)
  {
    this->row_desc_ = other.row_desc_;
    this->data_type_.assign(other.data_type_);
  }
  return *this;
}

int ObRowDescExt::get_by_id(const uint64_t table_id, const uint64_t column_id, int64_t &idx, ObObj &data_type) const
{
  int ret = OB_SUCCESS;
  idx = OB_INVALID_INDEX;
  const common::ObObj *cell = NULL;
  if (OB_INVALID_INDEX == (idx = row_desc_.get_idx(table_id, column_id)))
  {
    TBSYS_LOG(WARN, "failed to find column, tid=%lu cid=%lu", table_id, column_id);
    ret = OB_ERR_COLUMN_NOT_FOUND;
  }
  else if (OB_SUCCESS != (ret = data_type_.get_cell(idx, cell)))
  {
    TBSYS_LOG(ERROR, "BUG found column index but no data type");
    ret = OB_ERR_UNEXPECTED;
  }
  else
  {
    data_type = *cell;
  }
  return ret;
}

int ObRowDescExt::get_by_idx(const int64_t idx, uint64_t &table_id, uint64_t &column_id, ObObj &data_type) const
{
  int ret = OB_SUCCESS;
  const common::ObObj *cell = NULL;
  if (OB_SUCCESS != (ret = row_desc_.get_tid_cid(idx, table_id, column_id)))
  {
    TBSYS_LOG(WARN, "failed to find column by index, idx=%ld", idx);
    ret = OB_ERR_COLUMN_NOT_FOUND;
  }
  else if (OB_SUCCESS != (ret = data_type_.get_cell(idx, cell)))
  {
    TBSYS_LOG(ERROR, "BUG found column index but no data type");
    ret = OB_ERR_UNEXPECTED;
  }
  else
  {
    data_type = *cell;
  }
  return ret;
}

int ObRowDescExt::add_column_desc(const uint64_t table_id, const uint64_t column_id, const ObObj &data_type)
{
  int ret = OB_SUCCESS;
  int64_t idx = OB_INVALID_INDEX;
  if (OB_SUCCESS != (ret = row_desc_.add_column_desc(table_id, column_id)))
  {
    TBSYS_LOG(WARN, "failed to add column desc, err=%d", ret);
  }
  else if (OB_INVALID_INDEX == (idx = row_desc_.get_idx(table_id, column_id)))
  {
    TBSYS_LOG(ERROR, "BUG we have just added the column bug failed to get it");
    ret = OB_ERR_UNEXPECTED;
  }
  else if (OB_SUCCESS != (ret = data_type_.set_cell(idx, data_type)))
  {
    TBSYS_LOG(WARN, "failed to set data type, err=%d", ret);
  }
  return ret;
}

//add zt 20151113:b
DEFINE_SERIALIZE(ObRowDescExt)
{
  int ret = OB_SUCCESS;
  const common::ObObj *cell = NULL;
  if( OB_SUCCESS != (ret = row_desc_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize obrow_desc fail");
  }
  else
  {
    for(int64_t i = 0; (OB_SUCCESS == ret) && (i < row_desc_.get_column_num()); ++i)
    {
      data_type_.get_cell(i, cell);
      if( OB_SUCCESS != (ret = cell->serialize(buf, buf_len, pos)) )
      {
        TBSYS_LOG(WARN, "serialize datatype [%ld] fail", i);
      }
    }
  }
  return ret;
}


DEFINE_DESERIALIZE(ObRowDescExt)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = row_desc_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize obrow_desc fail");
  }
  else
  {
    ObObj cell;
    for(int64_t i = 0; (OB_SUCCESS == ret) && (i < row_desc_.get_column_num()); ++i)
    {
      if( OB_SUCCESS != (ret = cell.deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "serialize datatype [%ld] fail", i);
      }
      else
      {
        data_type_.set_cell(i, cell);
      }
    }
  }
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObRowDescExt)
{
  int64_t size = row_desc_.get_serialize_size();
  const common::ObObj *cell = NULL;
  for(int64_t i = 0; i < row_desc_.get_column_num(); ++i)
  {
    data_type_.get_cell(i, cell);
    size += cell->get_serialize_size();
  }
  return size;
}

//add zt 20151113:e
