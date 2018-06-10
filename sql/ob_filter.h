/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_filter.h
* @brief for operations of filter
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
 * ob_filter.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_FILTER_H
#define _OB_FILTER_H 1
#include "ob_single_child_phy_operator.h"
#include "ob_sql_expression.h"
#include "common/dlist.h"
//add maoxx
#include "ob_multiple_get_merge.h"
//add e

namespace oceanbase
{
  namespace sql
  {
    class ObFilter: public ObSingleChildPhyOperator
    {
      public:
        ObFilter();
        virtual ~ObFilter();
        virtual void reset();
        virtual void reuse();
        /**
         * 添加一个filter
         * 多个filter之间为AND关系
         * @param expr [in] 过滤表达式
         *
         * @return OB_SUCCESS或错误码
         */
        int add_filter(ObSqlExpression* expr);
        virtual int open();
        virtual int close();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        virtual ObPhyOperatorType get_type() const;
        //add maoxx
        /**
         * @brief reset_iterator
         */
        void reset_iterator();
        //add e

        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
      private:
        // disallow copy
        ObFilter(const ObFilter &other);
        ObFilter& operator=(const ObFilter &other);
      private:
        // data members
        common::DList filters_;
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_FILTER_H */
