/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_row_desc_ext.h
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
 * ob_row_desc_ext.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_ROW_DESC_EXT_H
#define _OB_ROW_DESC_EXT_H 1
#include "ob_row_desc.h"
#include "ob_raw_row.h"

namespace oceanbase
{
  namespace common
  {
    // ObRowDesc with column data type information
    class ObRowDescExt
    {
      public:
        ObRowDescExt();
        ~ObRowDescExt();

        ObRowDescExt(const ObRowDescExt &other);
        ObRowDescExt& operator=(const ObRowDescExt &other);
        void reset();
        int get_by_id(const uint64_t table_id, const uint64_t column_id, int64_t &idx, ObObj &data_type) const;
        int get_by_idx(const int64_t idx, uint64_t &table_id, uint64_t &column_id, ObObj &data_type) const;
        int64_t get_column_num() const;
        
        int add_column_desc(const uint64_t table_id, const uint64_t column_id, const ObObj &data_type);

        //add zt 20151113:b
        /**
         * @brief get_row_desc
         * get row descriptor
         * @return row descriptor
         */
        const ObRowDesc & get_row_desc() const { return row_desc_; }
        /**
         * @brief serialize and deserialize function
         * @param buf
         * @param buf_len
         * @param pos
         * @return
         */
        NEED_SERIALIZE_AND_DESERIALIZE;
        //add zt 20151113:e
      private:
        // data members
        ObRowDesc row_desc_;
        ObRawRow data_type_;
    };

    inline int64_t ObRowDescExt::get_column_num() const
    {
      return row_desc_.get_column_num();
    }
  } // end namespace common
} // end namespace oceanbase

#endif /* _OB_ROW_DESC_EXT_H */
