/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_expr_values.h
 * @brief for operations of expression value
 *
 * modified by maoxiaoxiao:add functions to reset iterator
 * modified by zhutao:add define different serialize methods for procedure
 * @version __DaSE_VERSION
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
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
 * ob_expr_values.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_EXPR_VALUES_H
#define _OB_EXPR_VALUES_H 1
#include "sql/ob_no_children_phy_operator.h"
#include "common/ob_row_store.h"
#include "sql/ob_sql_expression.h"
#include "common/ob_array.h"
#include "common/ob_row_desc_ext.h"


namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObExprValues class
     * ObExprValues is used by ObIncScan and ObInsertDBSemFilter and ObUpsModify(Replace semantics)
     * > ObIncScan, uses ObExprValues to get the rowkey info and store into the get_param.
     *	When serialization, get_param (instead of the ObExprValues) would be serialize. Serialize get_param
     *  is safe, since the rowkey info would not change over execution.
     * > ObUpsModify, uses ObExprValues as a child, when serialization, ObExprValues would be direct serialized,
     *   we should serialize the raw ObSqlExpression.
     * > ObInsertDBSemFilter, uses ObExprValues as a subquery, when serialization, ObExprValues would be direct serialize,
     *   we should serialize the raw ObSqlExpression
     */
    class ObExprValues: public ObNoChildrenPhyOperator
    {
      public:
        ObExprValues();
        virtual ~ObExprValues();

        int set_row_desc(const common::ObRowDesc &row_desc, const common::ObRowDescExt &row_desc_ext);
        int add_value(const ObSqlExpression &v);

        void clear_value() { values_.clear(); } //add by zhutao, debug range update

        void reserve_values(int64_t num) {values_.reserve(num);}
        void set_check_rowkey_duplicate(bool flag) { check_rowkey_duplicat_ = flag; }
        void set_do_eval_when_serialize(bool v) { do_eval_when_serialize_ = v;}
        //add huangjianwei [secondary index maintain] 20161108:b
        void set_replace_check_rowkey_duplicate(bool flag) { replace_check_rowkey_duplicat_ = flag;}
        //add:e
        ObExpressionArray &get_values() {return values_;}
        virtual int open();
        virtual int close();
        virtual void reset();
        virtual void reuse();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        enum ObPhyOperatorType get_type() const {return PHY_EXPR_VALUES;}
        //add maoxx
        /**
         * @brief reset_iterator
         */
        void reset_iterator() { row_store_.reset_iterator();}
        //add e

        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        void set_del_upd();
        //add e
        //add by zt 20160119:b
        /**
         * @brief serialize_template
         * serialize expression for procedure
         * @param buf buffer
         * @param buf_len buffer length
         * @param pos location flag
         * @return error code
         */
        int serialize_template(char *buf, const int64_t buf_len, int64_t &pos) const;
        /**
         * @brief deserialize_template
         * serialize expression for procedure
         * @param buf buffer
         * @param data_len buffer length
         * @param pos location flag
         * @return error code
         */
        int deserialize_template(const char *buf, const int64_t data_len, int64_t& pos);
        /**
         * @brief prepare_data
         * prepare set expression inx  0
         * @return error code
         */
        int prepare_data();
        /**
         * @brief get_next_row_template
         * get next row
         * @param row return ObRow object point
         * @return error code
         */
        int get_next_row_template(const common::ObRow *&row);
        //add by zt 20160119:e
      private:
        // types and constants
      private:
        // disallow copy
        ObExprValues(const ObExprValues &other);
        ObExprValues& operator=(const ObExprValues &other);
        // function members
        int eval();
      private:
        // data members
        ObExpressionArray values_;
        common::ObRowStore row_store_;
        common::ObRowDesc row_desc_;
        common::ObRowDescExt row_desc_ext_;
        common::ObRow row_;
        bool from_deserialize_;
        bool check_rowkey_duplicat_;
        bool do_eval_when_serialize_;
        bool group_exec_;
        int64_t expr_idx_;
        //add huangjianwei [secondary index maintain] 20161108:b
        bool replace_check_rowkey_duplicat_;
        //add:e
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        bool is_del_update;
        //add e
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_EXPR_VALUES_H */
