/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_ups_lock_table.cpp
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */
#include "ob_ups_lock_table.h"
#include "stdlib.h"

using namespace oceanbase::common;
using namespace oceanbase::sql;

int64_t ObUpsLockTable::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObUpsUpsLockTable(table_name=%.*s)\n", table_name_.length(), table_name_.ptr());
  return pos;
}
int ObUpsLockTable::serialize(char* buf, const int64_t buf_len, int64_t& pos) const
{
  int err = OB_SUCCESS;
  int64_t new_pos = pos;
  if (OB_SUCCESS != (err = table_name_.serialize(buf, buf_len, new_pos)))
  {
    TBSYS_LOG(ERROR, "serialize(buf=%p[%ld-%ld])=>%d", buf, new_pos, buf_len, err);
  }
  else if (OB_SUCCESS != (err = serialization::encode_i64(buf, buf_len, new_pos, table_id_)))
  {
    TBSYS_LOG(ERROR, "serialize(buf=%p[%ld-%ld])=>%d", buf, new_pos, buf_len, err);
  }
  else
  {
    pos = new_pos;
  }
  return err;
}
int ObUpsLockTable::deserialize(const char* buf, const int64_t buf_len, int64_t& pos)
{
  int err = OB_SUCCESS;
  int64_t new_pos = pos;
  if (OB_SUCCESS != (err = table_name_.deserialize(buf, buf_len, new_pos)))
  {
    TBSYS_LOG(ERROR, "deserialize(buf=%p[%ld-%ld])=>%d", buf, new_pos, buf_len, err);
  }
  else if (OB_SUCCESS != (err = serialization::decode_i64(buf, buf_len, new_pos, (int64_t*)&table_id_)))
  {
    TBSYS_LOG(ERROR, "deserialize(buf=%p[%ld-%ld])=>%d", buf, new_pos, buf_len, err);
  }
  else
  {
    pos = new_pos;
  }
  return err;
}

int64_t ObUpsLockTable::get_serialize_size(void) const
{
  return table_name_.get_serialize_size();
}

int ObUpsLockTable::open()
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(INFO, "zcd::test get_lock_table_name() => %.*s table_id=%ld", get_lock_table_name().length(), get_lock_table_name().ptr(), get_lock_table_id());

  return ret;
}

int ObUpsLockTable::close()
{
  int ret = OB_SUCCESS;
  return ret;
}
void ObUpsLockTable::reset()
{

}
void ObUpsLockTable::reuse()
{

}
int ObUpsLockTable::get_next_row(const ObRow *&row)
{
  int ret = OB_ITER_END;
  UNUSED(row);
  return ret;
}
int ObUpsLockTable::get_row_desc(const ObRowDesc *&row_desc) const
{
  int ret = OB_ITER_END;
  UNUSED(row_desc);
  return ret;
}
