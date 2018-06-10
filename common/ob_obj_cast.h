/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_obj_cast.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_OBJ_CAST_H
#define _OB_OBJ_CAST_H 1
#include "ob_object.h"
#include "ob_expr_obj.h"

namespace oceanbase
{
  namespace common
  {
    struct ObObjCastParams
    {
      // add params when necessary
      //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        uint32_t precision;
        uint32_t scale;
        bool is_modify;
        ObObjCastParams()
        {
            precision=38;
            scale=0;
            is_modify=false;
        }
      //add:e
    };
    typedef int (*ObObjCastFunc) (const ObObjCastParams &params, const ObExprObj &in, ObExprObj &out);
    extern ObObjCastFunc OB_OBJ_CAST[ObMaxType][ObMaxType];

    // utility function
    int obj_cast(const ObObj &orig_cell, const ObObj &expected_type,
                 ObObj &casted_cell, const ObObj *&res_cell);
    //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
    int obj_cast_for_rowkey(const ObObj &orig_cell, const ObObj &expected_type,
                 ObObj &casted_cell, const ObObj *&res_cell);
    //add e
    int obj_cast(ObObj &cell, const ObObjType expected_type, char* buf, int64_t buf_size, int64_t &used_buf_len);
    int obj_cast(ObObj &cell, const ObObjType expected_type, ObString &cast_buffer);
  } // end namespace common
} // end namespace oceanbase

#endif /* _OB_OBJ_CAST_H */
