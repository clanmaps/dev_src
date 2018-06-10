/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_multiple_get_merge.h
* @brief for operations of merging data
*
* modified by maoxiaoxiao:add functions to reset iterator
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
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
 * ob_multiple_get_merge.h 
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@alipay.com>
 *
 */

#ifndef _OB_MULTIPLE_GET_MERGE_H
#define _OB_MULTIPLE_GET_MERGE_H 1

#include "ob_multiple_merge.h"

namespace oceanbase
{
  namespace sql
  {
    class ObMultipleGetMerge : public ObMultipleMerge
    {
      public:
        int open();
        int close();
        virtual void reset();
        virtual void reuse();
        int get_next_row(const ObRow *&row);
        enum ObPhyOperatorType get_type() const{return PHY_MULTIPLE_GET_MERGE;}
        int64_t to_string(char *buf, int64_t buf_len) const;
        //add maoxx
        /**
         * @brief reset_iterator
         */
        void reset_iterator();
        //add e

        DECLARE_PHY_OPERATOR_ASSIGN;
      private:
        ObRow cur_row_;
    };
  }
}

#endif /* _OB_MULTIPLE_GET_MERGE_H */
  

