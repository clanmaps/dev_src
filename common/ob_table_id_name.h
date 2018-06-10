/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_table_id_name.h
 * @brief get all the table schema in the database
 *
 * modified by longfei：get "__all_secondary_index"
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2016_01_21
 */

/**
 * (C) 2010-2011 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_table_id_name.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *
 */

#ifndef _OB_TABLE_ID_NAME_H
#define _OB_TABLE_ID_NAME_H

#include "common/ob_string.h"
#include "common/nb_accessor/ob_nb_accessor.h"

namespace oceanbase
{
  namespace common
  {
    struct ObTableIdName
    {
      ObString table_name_;
      uint64_t table_id_;

      ObTableIdName() : table_id_(0) { }
    };

    /* 获得系统所有表单名字和对应的表单id */
    /// @note not thread-safe
    class ObTableIdNameIterator
    {
      public:
        ObTableIdNameIterator();
        virtual ~ObTableIdNameIterator();

        int init(ObScanHelper* client_proxy, bool only_core_tables);
        virtual int next();

        /* 获得内部table_info的指针 */
        virtual int get(ObTableIdName** table_info);

        /* 释放内存 */
        void destroy();

      private:
        bool check_inner_stat();
        int normal_get(ObTableIdName** table_info, bool index = false);
        int internal_get(ObTableIdName** table_info);
      protected:
        bool need_scan_;
        bool only_core_tables_;
        // longfei [create index] for schema in secondary index core table
        bool index; ///< flag for get row from res2
        int32_t table_idx_;
        nb_accessor::ObNbAccessor nb_accessor_;
        ObScanHelper* client_proxy_;
        nb_accessor::QueryRes* res_;
        //longfei [create index] for schema in secondary index core table
        nb_accessor::QueryRes* res2_; ///< for schema in secondary index core table
        ObTableIdName table_id_name_;
    };
  }
}

#endif /* _OB_TABLE_ID_NAME_H */


