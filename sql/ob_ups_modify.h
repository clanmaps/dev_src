/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_ups_modify.h
 *
 * Authors:
 *   Li Kai <yubai.lk@alipay.com>
 *
 */
#ifndef _OB_UPS_MODIFY_H
#define _OB_UPS_MODIFY_H 1

#include "ob_husk_filter.h"
#include "ob_no_children_phy_operator.h" //add wangjiahao [table lock] 20160616

namespace oceanbase
{
  using namespace common;

  namespace sql
  {
    namespace test
    {
    }

    /**
     * add by zt
     * @brief The ObUpsModify class
     * I feel the ObUpsModify almost the same as the ObUpsModifyWithDmlType
     * the mainly different is serialization. The former does not serialize anything,
     * the later serialize the dmltype, it just save the dmltype flag when serialize
     * replace plan. Is it necessary to desgin two classes just for such savings ?
     */
    class ObUpsModify : public ObHuskFilter<PHY_UPS_MODIFY>
    {
      public:
        ObDmlType get_dml_type() const {return OB_DML_REPLACE;};
        int64_t get_table_id() const {return table_id_;}
        void set_table_id(int64_t table_id) {table_id_ = table_id;}
        //add lbzhong [auto_increment] 20161215:b
        int64_t get_auto_value() const {return auto_value_;}
        void set_auto_value(int64_t auto_value) {auto_value_ = auto_value;}
        //add:e
      private:
        int64_t table_id_;
        //add lbzhong [auto_increment] 20161215:b
        int64_t auto_value_;
        //add:e
    };

    class ObUpsModifyWithDmlType: public ObHuskFilter<PHY_UPS_MODIFY_WITH_DML_TYPE>
    {
      public:
        ObUpsModifyWithDmlType(): dml_type_(OB_DML_REPLACE), table_id_(OB_INVALID_ID)
        //add lbzhong [auto_increment] 20161215:b
        , auto_value_(OB_INVALID_AUTO_INCREMENT_VALUE)
        //add:e
        {} //modify wangjiahao [table lock] 20160616
        virtual ~ObUpsModifyWithDmlType() {}
      public:
        ObDmlType get_dml_type() const {return dml_type_;};
        void set_dml_type(const ObDmlType dml_type) {dml_type_ = dml_type;}
        //add wangjiahao [table lock] 20160616 :b
        int64_t get_table_id() const {return table_id_;}
        void set_table_id(int64_t table_id) {table_id_ = table_id;}
        //add :e
        //add lbzhong [auto_increment] 20161215:b
        int64_t get_auto_value() const {return auto_value_;}
        void set_auto_value(int64_t auto_value) {auto_value_ = auto_value;}
        //add:e
        int64_t to_string(char* buf, const int64_t buf_len) const;
        int serialize(char* buf, const int64_t buf_len, int64_t& pos) const;
        int deserialize(const char* buf, const int64_t data_len, int64_t& pos);
        int64_t get_serialize_size(void) const;
        DECLARE_PHY_OPERATOR_ASSIGN
        {
          int ret = OB_SUCCESS;
          const ObUpsModifyWithDmlType *sub_other = dynamic_cast<const ObUpsModifyWithDmlType*>(other);
          if (NULL == sub_other)
          {
            TBSYS_LOG(ERROR, "dynamic cast phy operator fail, other=%p", other);
            ret = OB_ERR_UNEXPECTED;
          }
          else
          {
            dml_type_ = sub_other->dml_type_;
            //add huangjianwei [auto_increment debug] 20180111:b
            table_id_ = sub_other->table_id_;
            auto_value_ = sub_other->auto_value_;
            //add :e
          }
          return ret;
        }
      private:
        ObDmlType dml_type_;
        int64_t table_id_; //add wangjiahao [table lock] 20160616
        //add lbzhong [auto_increment] 20161215:b
        int64_t auto_value_;
        //add:e
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_UPS_MODIFY_H */
