/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_ups_lock_table.h
 *
 * Authors:
 *   wangjiahao <wjh2006-1@163.com>
 *
 */
#ifndef _OB_UPS_LOCK_TABLE_H
#define _OB_UPS_LOCK_TABLE_H 1

#include "ob_no_children_phy_operator.h"
#include "common/ob_string.h"


namespace oceanbase
{
  using namespace common;

  namespace sql
  {
    class ObUpsLockTable: public ObNoChildrenPhyOperator
    {
      public:
        ObUpsLockTable()
        {
          table_name_.assign_buffer(table_name_buf_, OB_MAX_TABLE_NAME_LENGTH);
          table_id_ = 0;
        }
        virtual ~ObUpsLockTable() {}
      public:
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        virtual int serialize(char* buf, const int64_t buf_len, int64_t& pos) const;
        virtual int deserialize(const char* buf, const int64_t buf_len, int64_t& pos);
        virtual int64_t get_serialize_size(void) const;
        virtual int open();
        virtual int close();
        virtual void reset();
        virtual void reuse();
        virtual int get_next_row(const ObRow *&row);
        virtual int get_row_desc(const ObRowDesc *&row_desc) const;
        virtual ObPhyOperatorType get_type() const {return PHY_UPS_LOCK_TABLE;}

        int set_lock_table_name(const ObString& table_name)
        {
          int ret = OB_SUCCESS;
          ret = table_name_.write(table_name.ptr(), table_name.length()) > 0 ? OB_SUCCESS : OB_BUF_NOT_ENOUGH;
          return ret;
        }
        const ObString& get_lock_table_name()
        {
          return table_name_;
        }

        const int64_t& get_lock_table_id()
        {
          return table_id_;
        }
        int set_lock_table_id(const int64_t table_id)
        {
          table_id_ = table_id;
          return OB_SUCCESS;
        }


      private:
        ObString table_name_;
        int64_t table_id_;
        char table_name_buf_[OB_MAX_TABLE_NAME_LENGTH];
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_UPS_LOCK_TABLE_H */
