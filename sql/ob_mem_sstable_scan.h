/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_mem_sstable_scan.h
 * @brief for operations of memory sstable scan
 *
 * modified by maoxiaoxiao:add functions to reset iterator
 * modified by zhutao:add serialize deserialize for procedure
 * @version __DaSE_VERSION
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 *
 * @date 2016_07_27
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
 * ob_mem_sstable_scan.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@alipay.com>
 *
 */

#ifndef _OB_MEM_SSTABLE_SCAN_H
#define _OB_MEM_SSTABLE_SCAN_H 1

#include "common/ob_row.h"
#include "common/ob_row_desc.h"
#include "common/ob_row_store.h"
#include "ob_phy_operator.h"
#include "ob_values.h"
namespace oceanbase
{
  namespace sql
  {
    class ObMemSSTableScan : public ObPhyOperator
    {
      public:
        ObMemSSTableScan();
        virtual ~ObMemSSTableScan();
        virtual void reset();
        virtual void reuse();
        virtual int set_child(int32_t child_idx, ObPhyOperator &child_operator);
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;

        virtual int open();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int close();
        virtual ObPhyOperatorType get_type() const;
        void set_tmp_table(uint64_t subquery_id) {tmp_table_subquery_ = subquery_id;}
        //add maoxx
        /**
         * @brief reset_iterator
         */
        void reset_iterator() { row_store_.reset_iterator();}
        //add e

        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;

        //add by zt 20160114:b
        /**
         * @brief serialize_template
         * serialize static data
         * @param buf buffer
         * @param buf_len buffer length
         * @param pos position flag
         * @return error code
         */
        int serialize_template(char *buf, const int64_t buf_len, int64_t &pos) const;
        /**
         * @brief deserialize_template
         * deserialize static data
         * @param buf buffer
         * @param data_len buffer length
         * @param pos position flag
         * @return error code
         */
        int deserialize_template(const char* buf, const int64_t data_len, int64_t& pos);
      private:
        /**
         * @brief prepare_data
         * read static data from the ObUpsProcedure here
         * @return error code
         */
        int prepare_data();
        //add by zt 20160114:e
      private:
        common::ObRow cur_row_;
        common::ObRowDesc cur_row_desc_;
        common::ObRowStore row_store_;
        bool from_deserialize_;
        uint64_t tmp_table_subquery_;

        //add by zt 20160113:b
        common::ObRowStore *row_store_ptr_;  ///<  used to seperate row_store from the operator
        bool proc_exec_;  ///<  whether  procedure execution flag
        //add by zt 20160113:e
    };
  }
}

#endif /* _OB_MEM_SSTABLE_SCAN_H */
