/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_fill_values.cpp
 * @brief ObFillValues is designed for update_more
 * created by wangjiahao: fill expressions designed for update_more.
 * @version CEDAR 0.2 
 * @author wangjiahao <51151500051@ecnu.edu.cn>
 * @date 2015_12_30
 */
//add wangjiahao [dev_update_more] 20151204:b
#include "ob_fill_values.h"
#include "common/serialization.h"
#include "ob_physical_plan.h"
#include "common/ob_tsi_factory.h"
#include "ob_raw_expr.h"
#include "ob_expr_values.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;
using namespace oceanbase::common::serialization;

ObFillValues::ObFillValues()
  : op_from_(NULL), op_to_(NULL)
{
}

ObFillValues::~ObFillValues()
{
}

void ObFillValues::reset()
{
  op_from_ = NULL;
  op_to_ = NULL;
}

void ObFillValues::reuse()
{
  op_from_ = NULL;
  op_to_ = NULL;
}

int ObFillValues::set_op(ObPhyOperator *op_from, ObPhyOperator *op_to)
{
  int ret = OB_SUCCESS;
  if (op_from == NULL || op_to == NULL)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(ERROR, "invalid operator");
  }
  else if (NULL != op_from_ || NULL != op_to_)
  {
    ret = OB_INIT_TWICE;
    TBSYS_LOG(ERROR, "op_from operator already init");
  }
  else
  {
    op_from_ = dynamic_cast<ObValues*>(op_from);
    op_to_ = dynamic_cast<ObExprValues*>(op_to);
  }
  return ret;
}

int ObFillValues::set_op_to(ObPhyOperator *op_to)
{
  int ret = OB_SUCCESS;
  if (op_to == NULL)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(ERROR, "invalid operator");
  }
  else if (NULL != op_to_)
  {
    ret = OB_INIT_TWICE;
    TBSYS_LOG(ERROR, "op_from operator already init");
  }
  else
  {
    op_to_ = dynamic_cast<ObExprValues*>(op_to);
  }
  return ret;
}

int ObFillValues::open()
{
  int ret = OB_SUCCESS;
  const common::ObRow *cur_row;
  const ObRowDesc *row_desc = NULL;
  uint64_t cid = OB_INVALID_ID;
  uint64_t tid = OB_INVALID_ID;
  const ObObj *cell = NULL;
  bool has_data = false;
//modified wjhh2008 [update more] for prepare update more :begin
  if (NULL == op_to_) {
    //this branch will be involved only for preparing SQL statment.
    //op_to_ is a ObExprValues operator and we search the plan tree to get it.
    for (int32_t i = 0; i < my_phy_plan_->get_query_size(); ++i)
    {
      ObPhyOperator* query = my_phy_plan_->get_phy_query(i);
      TBSYS_LOG(DEBUG,"huangjianwei test>>plan_query_size = [%d], query:%s", my_phy_plan_->get_query_size(),ob_phy_operator_type_str(query->get_type()));
      if (query->get_type() == PHY_EXPR_VALUES)
      {
         TBSYS_LOG(DEBUG, "huangjianwei test>> query type is PHY_EXPR_VALUES");
         set_op_to(query);
         break;
      }
    }
  }
//end
  if (NULL == op_from_)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(ERROR, "ObFillValues has not init, ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = op_to_->get_row_desc(row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get row_desc, err=%d", ret);
  }
  else
  {
    int64_t cell_num = row_desc->get_column_num();
    bool is_empty = false;
    int64_t remain_us = 0;

    //fill data
    while(OB_SUCCESS == ret && !this->my_phy_plan_->is_terminate(ret) && !this->my_phy_plan_->is_timeout(&remain_us))
    {
      ret = op_from_->get_next_row(cur_row);
      if (OB_ITER_END == ret)
      {
        ret = OB_SUCCESS;
        break;
      }
      else if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "fail to get next row from rpc scan");
      } 
      else if (OB_SUCCESS != cur_row->get_is_row_empty(is_empty))
      {
        TBSYS_LOG(WARN, "fail to check row empty.");
      }
      else if (!is_empty)
      {
        has_data = true;
        for (int64_t i = 0; i < cell_num; ++i)
        {
          ObConstRawExpr col_expr;
          if (OB_SUCCESS != (ret = cur_row->raw_get_cell(i, cell, tid, cid)))
          {
            TBSYS_LOG(WARN, "failed to get cell, err=%d i=%ld", ret, i);
            break;
          }
          else if (i < rowkey_info_.get_size()) // rowkey column
          {  
            if (OB_SUCCESS != (ret = col_expr.set_value_and_type(*cell)))
            {
              TBSYS_LOG(WARN, "failed to set value, err=%d", ret);
              break;
            }
          }
          else
          {
            ObObj null_obj;
            col_expr.set_value_and_type(null_obj);
          }

          ObSqlRawExpr col_raw_expr(
                common::OB_INVALID_ID,
                tid,
                cid,
                &col_expr);
          ObSqlExpression output_expr;
          if ((ret = col_raw_expr.fill_sql_expression(
                 output_expr,
                 NULL,
                 NULL,
                 NULL)) != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "Add table output columns failed. err=%d", ret);
            break;
          }
          else if (OB_SUCCESS != (ret = op_to_->add_value(output_expr)))
          {
            TBSYS_LOG(WARN, "Failed to add cell into get param, err=%d", ret);
            break;
          }
        } // end for
      }      
      TBSYS_LOG(DEBUG, "Update more fill the scanned rows from cs for ups plan %s", to_cstring(*cur_row));
    }
    //TBSYS_LOG(INFO, "wjh_test %s", to_cstring(*op_to_));
    if (!has_data) //if no data to update, return OB_NO_RESULT to terminate the plan, do not send to ups.
    {
      ret = OB_NO_RESULT;
    }
    //query canceled
    if (OB_ERR_QUERY_INTERRUPTED == ret || OB_ERR_SESSION_INTERRUPTED == ret)
    {
      ret = OB_SUCCESS;
    }
    if (remain_us < 0)
    {
      ret = OB_SUCCESS;
    }
  }
  return ret;
}

int ObFillValues::close()
{
  int ret = OB_SUCCESS;
  op_to_->clear_value(); //add by zhutao, fix bugs
  return ret;
}
int64_t ObFillValues::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObFillValues(op_from_=%p, op_to_=%p)\n", &op_from_, &op_to_);
  return pos;
}


//add :e

PHY_OPERATOR_ASSIGN(ObFillValues)
{
  TBSYS_LOG(DEBUG, "huangjianwei test>>into fill values assgin");
  int ret = OB_SUCCESS;
  CAST_TO_INHERITANCE(ObFillValues);
  reset();

  if (!my_phy_plan_)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(WARN, "ObPhysicalPlan/allocator is not set, ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = rowkey_info_.assign(o_ptr->rowkey_info_)))
  {
    TBSYS_LOG(WARN, "Assign rowkey info failed, ret=%d", ret);
  }
  else
  {
    for (int32_t i = 0; i < my_phy_plan_->get_query_size(); ++i)
    {
      ObPhyOperator* query = my_phy_plan_->get_phy_query(i);
      TBSYS_LOG(DEBUG,"huangjianwei test>>plan_query_size = [%d], main_query:%s", my_phy_plan_->get_query_size(),ob_phy_operator_type_str(query->get_type()));
      if (query->get_type() == PHY_VALUES)
      {
         TBSYS_LOG(DEBUG, "huangjianwei test>> query type is PHY_VALUES");
         op_from_ =  dynamic_cast<ObValues*>(query);
      }
    }
  }
  return ret;
}

//add huangjianwei [update more] fix prepare 20171127:b
namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObFillValues, PHY_FILL_VALUES);
  }
}

//add:e
