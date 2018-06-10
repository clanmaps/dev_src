/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_fill_values.h
 * @brief ObFillValues is designed for update_more
 * created by wangjiahao: fill expressions designed for update_more.
 * @version CEDAR 0.2 
 * @author wangjiahao <51151500051@ecnu.edu.cn>
 * @date 2015_12_30
 */
//add wangjiahao [dev_update_more] 20151204:b
#ifndef OBFILLVALUES_H
#define OBFILLVALUES_H 1

//#include "ob_phy_operator.h"
#include "common/ob_ups_row.h"
#include "common/ob_schema.h"
#include "ob_values.h"
#include "ob_no_children_phy_operator.h"

using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {

    class ObExprValues;
    /**
     * @brief this class is designed for the purpose of
     * supporting new update operation which can update
     * not even given full rowkey condition but also
     * other conditions. We need to fill the values from
     * ObValues contained the conditions to ObExprValues.
     */
    class ObFillValues : public ObNoChildrenPhyOperator
    {
      public:

        ObFillValues();
        virtual ~ObFillValues();
        virtual void reset();
        virtual void reuse();

        /**
         * @brief open Read data from ObValues operator and
         * Convert the data to expration and fill them into
         * operator ObExprValues.
         * @return OB_SUCCESS on success, OB_NO_RESULT means
         * no data need to update, otherwise on failed.
         */
        virtual int open();
        virtual int close();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;

        /**
         * @brief set_op set local operators.
         * @param op_from need a ObValues typed operator.
         * @param op_to need ObExprValues typed operator.
         * @return OB_SUCCESS.
         */
        int set_op(ObPhyOperator *op_from, ObPhyOperator *op_to);
        int set_op_to(ObPhyOperator *op_to);

        /**
         * @brief set_row_desc set row description
         * @param row_desc
         * @return OB_SUCCESS.
         */
        int set_row_desc(const common::ObRowDesc &row_desc);
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        /**
         * @brief set_rowkey_info set rowkey info for get row key.
         * @param rowkey_info
         */
        void set_rowkey_info(const common::ObRowkeyInfo &rowkey_info);
        enum ObPhyOperatorType get_type() const{return PHY_FILL_VALUES;}

        DECLARE_PHY_OPERATOR_ASSIGN;
        //VIRTUAL_NEED_SERIALIZE_AND_DESERIALIZE;

      protected:
        ObValues *op_from_; ///< source operator
        ObExprValues *op_to_; ///< target operator
        common::ObRowkeyInfo rowkey_info_; ///< rowken info

    };

    inline void ObFillValues::set_rowkey_info(const common::ObRowkeyInfo &rowkey_info)
    {
      rowkey_info_ = rowkey_info;
    }

    inline int ObFillValues::get_next_row(const common::ObRow *&row)
    {
      row = NULL;
      return common::OB_ITER_END;
    }
    inline int ObFillValues::get_row_desc(const common::ObRowDesc *&row_desc) const
    {
      row_desc = NULL;
      return common::OB_NOT_SUPPORTED;
    }
  }
}


#endif // OBFILLVALUES_H
//add :e
