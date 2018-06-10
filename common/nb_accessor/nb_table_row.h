/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file nb_table_row.h
 * @brief add an interface to dump all scanner info
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author
 *   Weng Haixing <wenghaixing@ecnu.cn>
 * @date  20160124
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
 * nb_table_row.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *
 */

#ifndef _NB_TABLE_ROW_H
#define _NB_TABLE_ROW_H 1

#include "common/hash/ob_hashmap.h"
#include "common/ob_common_param.h"

namespace oceanbase
{
namespace common
{
  namespace nb_accessor
  {
    //用于scan和get操作的结果，表示表单中的一行数据
    class TableRow
    {
    public:
      TableRow();
      virtual ~TableRow();

      int init(hash::ObHashMap<const char*, int64_t>* cell_map, ObCellInfo* cells, int64_t cell_count);

      virtual ObCellInfo* get_cell_info(int64_t index) const;
      //通过列名获取当前行的cell
      virtual ObCellInfo* get_cell_info(const char* column_name) const;

      int set_cell(ObCellInfo* cell, int64_t index);
      int64_t count() const;
      void dump() const;
      //add wenghaixing [secondary index.static_index]20160117
      void dump_test() const;
      //add e

    private:
      bool check_inner_stat() const;

      hash::ObHashMap<const char*,int64_t>* cell_map_;
      ObCellInfo* cells_;
      int64_t cell_count_;
    };
  }
}
}

#endif /* _NB_TABLE_ROW_H */

