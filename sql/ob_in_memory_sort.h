/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file     ob_stmt.h
 * @brief    sort 
 * modified by yu shengjuan: sort row from child_op get_next_row(),use std::sort . it will use at logical plan transform to physical plan
 * @version  CEDAR 0.2 
 * @author   yu shengjuan <51141500090@ecnu.cn>
 * @date     2015_08_19
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
 * ob_in_memory_sort.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_IN_MEMORY_SORT_H
#define _OB_IN_MEMORY_SORT_H 1
#include "common/ob_array.h"
#include "common/ob_string.h"
#include "common/ob_object.h"
#include "common/ob_row.h"
#include "ob_sort_helper.h"
#include "common/ob_row_store.h"

namespace oceanbase
{
  namespace sql
  {
    struct ObSortColumn
    {
      uint64_t table_id_;
      uint64_t column_id_;
      bool is_ascending_;

      ObSortColumn()
        :table_id_(common::OB_INVALID_ID), column_id_(common::OB_INVALID_ID), is_ascending_(true)
      {
      }

      NEED_SERIALIZE_AND_DESERIALIZE;
    };
    class ObInMemorySort: public ObSortHelper
    {
      public:
        ObInMemorySort();
        virtual ~ObInMemorySort();
        virtual void reset();
        virtual void reuse();
        int set_sort_columns(const common::ObArray<ObSortColumn> &sort_columns);
        int add_row(const common::ObRow &row);
        int sort_rows();
        // @pre sort_rows()
        virtual int get_next_row(const common::ObRow *&row);
        int get_next_compact_row(common::ObString &compact_row);
        int get_next_row(common::ObRow &row);
        const common::ObRowDesc* get_row_desc() const;
        //add yushengjuan [semi_join] [0.1] 20150829:b
        common::ObArray<const common::ObRowStore::StoredRow*>& get_sorted_element();
        //add:end

        int64_t get_row_count() const;
        int64_t get_used_mem_size() const;
      private:
        // types
        struct Comparer;
      private:
        // disallow copy
        ObInMemorySort(const ObInMemorySort &other);
        ObInMemorySort& operator=(const ObInMemorySort &other);
      private:
        // data members
        const common::ObArray<ObSortColumn> *sort_columns_;
        common::ObRowStore row_store_;
        common::ObArray<const common::ObRowStore::StoredRow*> sort_array_;
        int64_t sort_array_get_pos_;
        common::ObRow curr_row_;
        const common::ObRowDesc *row_desc_;
    };

    inline const common::ObRowDesc* ObInMemorySort::get_row_desc() const
    {
      return row_desc_;
    }

    //add yushengjuan [semi_join] [0.1] 20150829:b
    inline common::ObArray<const common::ObRowStore::StoredRow*>& ObInMemorySort::get_sorted_element()
	{
    	return sort_array_;
	}
    //add:end
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_IN_MEMORY_SORT_H */

