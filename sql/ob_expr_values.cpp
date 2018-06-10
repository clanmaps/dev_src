/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_expr_values.cpp
 * @brief for operations of expression value
 *
 * modified by zhutao:add define different serialize methods for procedure
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
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
 * ob_expr_values.cpp
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#include "ob_expr_values.h"
#include "ob_duplicate_indicator.h"
#include "common/utility.h"
#include "common/ob_obj_cast.h"
#include "common/hash/ob_hashmap.h"
#include "ob_fill_values.h" //add huangjianwei [foreign key] 20180119
#include "ob_physical_plan.h" //add zt 20151109
#include "ob_sp_procedure.h" //add by zt 20160704

using namespace oceanbase::sql;
using namespace oceanbase::common;
ObExprValues::ObExprValues()
  :values_(OB_TC_MALLOC_BLOCK_SIZE, ModulePageAllocator(ObModIds::OB_SQL_ARRAY)),
   from_deserialize_(false),
   check_rowkey_duplicat_(false),
   do_eval_when_serialize_(false),
   group_exec_(false),
   //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
   is_del_update(false)
   //add e
{
}

ObExprValues::~ObExprValues()
{
}

//add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
void ObExprValues::set_del_upd(){
    is_del_update=true;
}
//add e

void ObExprValues::reset()
{
  row_desc_.reset();
  row_desc_ext_.reset();
  values_.clear();
  row_store_.clear();
  //row_.reset(false, ObRow::DEFAULT_NULL);
  from_deserialize_ = false;
  check_rowkey_duplicat_ = false;
  do_eval_when_serialize_ = false;
  //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
  is_del_update=false;
  //add e
}

void ObExprValues::reuse()
{
  row_desc_.reset();
  row_desc_ext_.reset();
  values_.clear();
  row_store_.clear();
  //row_.reset(false, ObRow::DEFAULT_NULL);
  from_deserialize_ = false;
  check_rowkey_duplicat_ = false;
  do_eval_when_serialize_ = false;
  //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
  is_del_update=false;
  //add e
}

int ObExprValues::set_row_desc(const common::ObRowDesc &row_desc, const common::ObRowDescExt &row_desc_ext)
{
  row_desc_ = row_desc;
  row_desc_ext_ = row_desc_ext;
  return OB_SUCCESS;
}

int ObExprValues::add_value(const ObSqlExpression &v)
{
  int ret = OB_SUCCESS;
  if ((ret = values_.push_back(v)) == OB_SUCCESS)
  {
    values_.at(values_.count() - 1).set_owner_op(this);
  }
  return ret;
}

int ObExprValues::open()
{
  int ret = OB_SUCCESS;

  //add by zt 20160119:b
  if( group_exec_ )
  {
    if( OB_SUCCESS != (ret = prepare_data()))
    {
      TBSYS_LOG(WARN, "prepare_data()=>%d", ret);
    }
  }
  if( OB_SUCCESS != ret ) {}
  else
  //add by zt 20160119:e
  if (from_deserialize_)
  {
    row_store_.reset_iterator();
    row_.set_row_desc(row_desc_);
    // pass
  }
  else if (0 >= row_desc_.get_column_num()
      || 0 >= row_desc_ext_.get_column_num())
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(WARN, "row_desc not init");
  }
  else if (0 >= values_.count())
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(WARN, "values not init");
  }
  else
  {
    row_.set_row_desc(row_desc_);
    row_store_.reuse();
    if (OB_SUCCESS != (ret = eval()))
    {
      TBSYS_LOG(WARN, "failed to eval exprs, err=%d", ret);
    }
  }
  return ret;
}

int ObExprValues::close()
{
  if (from_deserialize_)
  {
    row_store_.reset_iterator();
  }
  else
  {
    row_store_.reuse();
  }
  return OB_SUCCESS;
}

int ObExprValues::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if( group_exec_ )
  {
    ret = get_next_row_template(row);
  }
  else
  if (OB_SUCCESS != (ret = row_store_.get_next_row(row_)))
  {
    if (OB_ITER_END != ret)
    {
      TBSYS_LOG(WARN, "failed to get next row from row store, err=%d", ret);
    }
  }
  else
  {
    row = &row_;
  }
  return ret;
}

int ObExprValues::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  row_desc = &row_desc_;
  return OB_SUCCESS;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObExprValues, PHY_EXPR_VALUES);
  }
}

int64_t ObExprValues::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ExprValues(values_num=%ld, values=",
                  values_.count());
  pos += values_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, ", row_desc=");
  pos += row_desc_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, ")\n");
  return pos;
}

int ObExprValues::eval()
{
  int ret = OB_SUCCESS;
  OB_ASSERT(0 < values_.count());
  OB_ASSERT(0 < row_desc_.get_column_num());
  OB_ASSERT(0 == (values_.count() % row_desc_.get_column_num()));
  ModuleArena buf(OB_MALLOC_BLOCK_SIZE, ModulePageAllocator(ObModIds::OB_SQL_TRANSFORMER));
  char* varchar_buff = NULL;
  if (NULL == (varchar_buff = buf.alloc(OB_MAX_VARCHAR_LENGTH)))
  {
    ret = OB_ALLOCATE_MEMORY_FAILED;
    TBSYS_LOG(WARN, "No memory");
  }
  else
  {
    const ObRowStore::StoredRow *stored_row = NULL;
    ObDuplicateIndicator *indicator = NULL;
    int64_t col_num = row_desc_.get_column_num();
    int64_t row_num = values_.count() / col_num;
    // RowKey duplication checking doesn't need while 1 row only
    if (check_rowkey_duplicat_ && row_num > 1)
    {
      void *ind_buf = NULL;
      if (row_desc_.get_rowkey_cell_count() <= 0)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "RowKey is empty, ret=%d", ret);
      }
      else if ((ind_buf = buf.alloc(sizeof(ObDuplicateIndicator))) == NULL)
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "Malloc ObDuplicateIndicator failed, ret=%d", ret);
      }
      else
      {
        indicator = new (ind_buf) ObDuplicateIndicator();
        if ((ret = indicator->init(row_num)) != OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "Init ObDuplicateIndicator failed, ret=%d", ret);
        }
      }
    }
    //add huangjianwei [secondary index maintain] 20161108:b
    if (replace_check_rowkey_duplicat_ && row_num > 1)
    {
      //ergodic from the back forward,give up the forward of rowkey is same with back
      for (int64_t i = values_.count(); OB_SUCCESS == ret && i > 0; i-=col_num)
      {
        ObRow val_row;
        val_row.set_row_desc(row_desc_);
        ObString varchar;
        ObObj casted_cell;
        for (int64_t j = 0; OB_SUCCESS == ret && j < col_num; ++j)
        {
          varchar.assign_ptr(varchar_buff, OB_MAX_VARCHAR_LENGTH);
          casted_cell.set_varchar(varchar); // reuse the varchar buffer
          const ObObj *single_value = NULL;
          uint64_t table_id = OB_INVALID_ID;
          uint64_t column_id = OB_INVALID_ID;
          ObObj tmp_value;
          ObObj data_type;
          ObSqlExpression &val_expr = values_.at(i-col_num+j);
          if ((ret = val_expr.calc(val_row, single_value)) != OB_SUCCESS) // the expr should be a const expr here
          {
            TBSYS_LOG(WARN, "Calculate value result failed, err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = row_desc_ext_.get_by_idx(j, table_id, column_id, data_type)))
          {
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "Failed to get column, err=%d", ret);
          }
          /*
          else if (0 < row_desc_.get_rowkey_cell_count()
                   && j < row_desc_.get_rowkey_cell_count()
                   && single_value->is_null())
          {
            TBSYS_LOG(USER_ERROR, "primary key can not be null");
            ret = OB_ERR_INSERT_NULL_ROWKEY;
          }
          */
           //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
          else
          {

              if(is_del_update)
              {
                  if (OB_SUCCESS != obj_cast(*single_value, data_type, casted_cell, single_value))
                  {
                      TBSYS_LOG(DEBUG, "failed to cast obj, err=%d", ret);
                  }
              }
              else
              {
                  if (OB_SUCCESS != (ret = obj_cast(*single_value, data_type, casted_cell, single_value)))
                  {
                          TBSYS_LOG(WARN, "failed to cast obj, err=%d", ret);
                  }
              }
          }
            //here can't add other code
          //else if (OB_SUCCESS != (ret = ob_write_obj(buf, *single_value, tmp_value))) old code
                  //old code
          /*
             else if (OB_SUCCESS != (ret = obj_cast(*single_value, data_type, casted_cell, single_value)))
          {
            TBSYS_LOG(WARN, "failed to cast obj, err=%d", ret);
          }
          */
          if(OB_SUCCESS!=ret){
          }
          else if (OB_SUCCESS != (ret = ob_write_obj_v2(buf, *single_value, tmp_value)))  //??
              //modify:e
          {
            TBSYS_LOG(WARN, "str buf write obj fail:ret[%d]", ret);
          }
          else if ((ret = val_row.set_cell(table_id, column_id, tmp_value)) != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "Add value to ObRow failed");
          }
          else
          {

            //TBSYS_LOG(DEBUG, "i=%ld j=%ld cell=%s", i, j, to_cstring(tmp_value));
          }
        } // end for
        if (OB_LIKELY(OB_SUCCESS == ret))
        {
          if (indicator)
          {
            const ObRowkey *rowkey = NULL;
            bool is_dup = false;
            if ((ret = val_row.get_rowkey(rowkey)) != OB_SUCCESS)
            {
              TBSYS_LOG(WARN, "Get RowKey failed, err=%d", ret);
            }
            else if ((ret = indicator->have_seen(*rowkey, is_dup)) != OB_SUCCESS)
            {
              TBSYS_LOG(WARN, "Check duplication failed, err=%d", ret);
            }
            else if (is_dup)
            {
              TBSYS_LOG(INFO, "replace check rowkey isdup is %c rowkey=%s", is_dup?'Y':'N', to_cstring(*rowkey));
            }
            else
            {
              if (OB_SUCCESS != (ret = row_store_.add_row(val_row, stored_row)))
              {
                TBSYS_LOG(WARN, "failed to add row into store, err=%d", ret);
              }
            }
          }
        }
      }// end for
    }
    else
    {
    //add:e
      for (int64_t i = 0; OB_SUCCESS == ret && i < values_.count(); i+=col_num) // for each row
      {
        ObRow val_row;
        val_row.set_row_desc(row_desc_);
        ObString varchar;
        ObObj casted_cell;
        for (int64_t j = 0; OB_SUCCESS == ret && j < col_num; ++j)
        {
          varchar.assign_ptr(varchar_buff, OB_MAX_VARCHAR_LENGTH);
          casted_cell.set_varchar(varchar); // reuse the varchar buffer
          const ObObj *single_value = NULL;
          uint64_t table_id = OB_INVALID_ID;
          uint64_t column_id = OB_INVALID_ID;
          ObObj tmp_value;
          ObObj data_type;
          ObSqlExpression &val_expr = values_.at(i+j);
          if ((ret = val_expr.calc(val_row, single_value)) != OB_SUCCESS) // the expr should be a const expr here
          {
            TBSYS_LOG(WARN, "Calculate value result failed, err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = row_desc_ext_.get_by_idx(j, table_id, column_id, data_type)))
          {
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "Failed to get column, err=%d", ret);
          }
          //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
         else
         {
             if(is_del_update)
             {
                 if (OB_SUCCESS != obj_cast(*single_value, data_type, casted_cell, single_value))
                 {
                     TBSYS_LOG(DEBUG, "failed to cast obj, err=%d", ret);
                 }
             }
             else
             {
                 if (OB_SUCCESS != (ret = obj_cast(*single_value, data_type, casted_cell, single_value)))   //??
                 {
                         TBSYS_LOG(WARN, "failed to cast obj, err=%d", ret);
                 }
             }

         }
         if(OB_SUCCESS!=ret)
         {
         }
         else if (OB_SUCCESS != (ret = ob_write_obj_v2(buf, *single_value, tmp_value)))
             //modify:e
         {
             TBSYS_LOG(WARN, "str buf write obj fail:ret[%d]", ret);
         }
         else if ((ret = val_row.set_cell(table_id, column_id, tmp_value)) != OB_SUCCESS)
         {
             TBSYS_LOG(WARN, "Add value to ObRow failed");
         }
         else
         {
         }
        } // end for
        if (OB_LIKELY(OB_SUCCESS == ret))
        {
          if (OB_SUCCESS != (ret = row_store_.add_row(val_row, stored_row)))
          {
            TBSYS_LOG(WARN, "failed to add row into store, err=%d", ret);
          }
          else if (indicator)
          {
            const ObRowkey *rowkey = NULL;
            bool is_dup = false;
            if ((ret = val_row.get_rowkey(rowkey)) != OB_SUCCESS)
            {
              TBSYS_LOG(WARN, "Get RowKey failed, err=%d", ret);
            }
            else if ((ret = indicator->have_seen(*rowkey, is_dup)) != OB_SUCCESS)
            {
              TBSYS_LOG(WARN, "Check duplication failed, err=%d", ret);
            }
            else if (is_dup)
            {
              ret = OB_ERR_PRIMARY_KEY_DUPLICATE;
              TBSYS_LOG(USER_ERROR, "Duplicate entry \'%s\' for key \'PRIMARY\'", to_cstring(*rowkey));
            }
            TBSYS_LOG(INFO, "check rowkey isdup is %c rowkey=%s", is_dup?'Y':'N', to_cstring(*rowkey));
          }
        }
      }// end for
    }
    if (indicator)
    {
      indicator->~ObDuplicateIndicator();
    }
  }
  return ret;
}

PHY_OPERATOR_ASSIGN(ObExprValues)
{
  int ret = OB_SUCCESS;
  CAST_TO_INHERITANCE(ObExprValues);
  reset();
  row_desc_ = o_ptr->row_desc_;
  row_desc_ext_ = o_ptr->row_desc_ext_;

  values_.reserve(o_ptr->values_.count());
  for (int64_t i = 0; i < o_ptr->values_.count(); i++)
  {
    if ((ret = this->values_.push_back(o_ptr->values_.at(i))) == OB_SUCCESS)
    {
      this->values_.at(i).set_owner_op(this);
    }
    else
    {
      break;
    }
  }
  //add huangjianwei [update more] fix prepare 20180119:b
  /*
  for (int32_t i = 0; i < my_phy_plan_->get_query_size(); ++i)
  {
    ObPhyOperator* query = my_phy_plan_->get_phy_query(i);
    TBSYS_LOG(DEBUG,"huangjianwei test>>plan_query_size = [%d], query:%s", my_phy_plan_->get_query_size(),ob_phy_operator_type_str(query->get_type()));
    if (query->get_type() == PHY_FILL_VALUES)
    {
       TBSYS_LOG(DEBUG, "huangjianwei test>> query type is PHY_FILL_VALUES");
       ObFillValues *op_fill_values =  dynamic_cast<ObFillValues*>(query);
       //ObFillValues *op_fill_values = (ObFillValues*)query;
       op_fill_values->set_op_to((ObPhyOperator *)this);
       break;
    }
  }
  */
  //add:e
  do_eval_when_serialize_ = o_ptr->do_eval_when_serialize_;
  check_rowkey_duplicat_ = o_ptr->check_rowkey_duplicat_;
  // Does not need to assign row_store_, because this function is used by MS only before opening
  return ret;
}

/*******************************************************************
  *We need to define different serialize methods for procedure
  *execution or physical plan execution.
  * > For procedure execution, exprValues cannot be calculted until
  * 	until precede plans done
  * > For phy_plan execution, exprValues can be calcuted, since there's
  *   no precede plans
  *****************************************************************/

DEFINE_SERIALIZE(ObExprValues)
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
  //add by zt 20160118:b
  if( OB_SUCCESS !=  (ret = serialization::encode_bool(buf, buf_len, tmp_pos, my_phy_plan_->is_group_exec())) )
  {
    TBSYS_LOG(WARN, "failed to serialize proc_exec flag");
  }
  else if( my_phy_plan_->is_group_exec() )
  {
    if( OB_SUCCESS != (ret = serialize_template(buf, buf_len, tmp_pos)) )
    {
      TBSYS_LOG(WARN, "failed to serialize template version");
    }
    else
    {
      pos = tmp_pos;
    }
  }
  else
  {
  //add zt 20160118 :e
    if (do_eval_when_serialize_)
    {
      if (OB_SUCCESS != (ret = (const_cast<ObExprValues*>(this))->open()))
      {
        TBSYS_LOG(WARN, "failed to open expr_values, err=%d", ret);
      }
    }

    if (OB_LIKELY(OB_SUCCESS == ret))
    {
      if (OB_SUCCESS != (ret = row_desc_.serialize(buf, buf_len, tmp_pos)))
      {
        TBSYS_LOG(WARN, "serialize row_desc fail ret=%d buf=%p buf_len=%ld pos=%ld", ret, buf, buf_len, tmp_pos);
      }
      else if (OB_SUCCESS != (ret = row_store_.serialize(buf, buf_len, tmp_pos)))
      {
        TBSYS_LOG(WARN, "serialize row_store fail ret=%d buf=%p buf_len=%ld pos=%ld", ret, buf, buf_len, tmp_pos);
      }
      else
      {
        pos = tmp_pos;
      }
    }
    if (do_eval_when_serialize_)
    {
      (const_cast<ObExprValues*>(this))->close();
    }
  }
  return ret;
}

DEFINE_DESERIALIZE(ObExprValues)
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
  //add zt 20151109:b
  if( OB_SUCCESS != (ret = serialization::decode_bool(buf, data_len, tmp_pos, &group_exec_)) )
  {
    TBSYS_LOG(WARN, "fail to deseriazlie proc_exec flag");
  }
  else if ( group_exec_ )
  {
    if ( OB_SUCCESS != (ret = deserialize_template(buf, data_len, tmp_pos)))
    {
      TBSYS_LOG(WARN, "fail to deserialize template version");
    }
    else
    {
      from_deserialize_ = true;
      pos = tmp_pos;
    }
  }
  else
  //add zt 20151109:e
  if (OB_SUCCESS != (ret = row_desc_.deserialize(buf, data_len, tmp_pos)))
  {
    TBSYS_LOG(WARN, "serialize row_desc fail ret=%d buf=%p data_len=%ld pos=%ld", ret, buf, data_len, tmp_pos);
  }
  else if (OB_SUCCESS != (ret = row_store_.deserialize(buf, data_len, tmp_pos)))
  {
    TBSYS_LOG(WARN, "serialize row_store fail ret=%d buf=%p data_len=%ld pos=%ld", ret, buf, data_len, tmp_pos);
  }
  else
  {
    from_deserialize_ = true;
    pos = tmp_pos;
  }
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObExprValues)
{
  //delete by zt 20151109 :b
  //  return (row_desc_.get_serialize_size() + row_store_.get_serialize_size());
  //delete by zt 20151109 :e
  bool group_exec = false;
  if( NULL != my_phy_plan_ && my_phy_plan_->is_group_exec() ) group_exec = true;
  if( !group_exec )
  {
    return (row_desc_.get_serialize_size() + row_store_.get_serialize_size() + sizeof(bool));
  }
  else
  {
    int64_t size = sizeof(bool) + sizeof(int64_t) + row_desc_ext_.get_serialize_size();
    for(int64_t i = 0; i < values_.count(); ++i)
    {
      size += values_.at(i).get_serialize_size();
    }
    return size;
  }
}

//add by zt 20160119:b
int ObExprValues::serialize_template(char *buf, const int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
  if ( OB_SUCCESS != (ret = row_desc_.serialize(buf, buf_len, tmp_pos)))
  {
    TBSYS_LOG(WARN, "serialize row_desc fail");
  }
  else if ( OB_SUCCESS != (ret = row_desc_ext_.serialize(buf, buf_len, tmp_pos)))
  {
    TBSYS_LOG(WARN, "serialzie row_desc ext fail");
  }
  else
  {
    serialization::encode_i64(buf, buf_len, tmp_pos, values_.count());
    for (int64_t i = 0; OB_SUCCESS == ret && i < values_.count(); ++i)
    {
      if ( OB_SUCCESS != (ret = values_.at(i).serialize(buf, buf_len, tmp_pos)))
      {
        TBSYS_LOG(WARN, "Fail to serialize expr[%ld], ret=%d", i, ret);
        break;
      }
    }
  }
  if( OB_SUCCESS == ret )
  {
    pos = tmp_pos;
  }
  return ret;
}

int ObExprValues::deserialize_template(const char *buf, const int64_t data_len, int64_t &pos)
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
  int64_t expr_value_count;
  if( OB_SUCCESS != (ret = row_desc_.deserialize(buf, data_len, tmp_pos)))
  {
    TBSYS_LOG(WARN, "deserialize the row_desc fail");
  }
  else if( OB_SUCCESS != (ret = row_desc_ext_.deserialize(buf, data_len, tmp_pos)) )
  {
    TBSYS_LOG(WARN, "deserialize the row_desc_ext fail");
  }
  else if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, tmp_pos, &expr_value_count)) )
  {
    TBSYS_LOG(WARN, "deserialize the expr values count fail, ret=%d", ret);
  }
  else
  {
    values_.reserve(expr_value_count);
    ObSqlExpression expr;
    for(int64_t i = 0; OB_SUCCESS == ret && i < expr_value_count; ++i)
    {
      values_.push_back(expr);
      if( OB_SUCCESS != (ret = values_.at(i).deserialize(buf, data_len, tmp_pos)) )
      {
        TBSYS_LOG(WARN, "Fail to deserialize expr[%ld], ret=%d", i, ret);
      }
      else
      {
        values_.at(i).set_owner_op(this);
      }
    }
  }
  if( OB_SUCCESS == ret )
  {
    pos = tmp_pos;
  }
  return ret;
}

int ObExprValues::prepare_data()
{
  int ret = OB_SUCCESS;
//  row_store_.reuse();
//  ret = eval();
  expr_idx_ = 0;
  return ret;
}

int ObExprValues::get_next_row_template(const common::ObRow *&row)
{
  int ret = expr_idx_ < values_.count() ? OB_SUCCESS : OB_ITER_END;
  int64_t col_num = row_desc_.get_column_num();
  char *varchar_buff = NULL;
  ObStringBuf *buf = NULL;
  //try to get casted buf
  if( my_phy_plan_->get_main_query()->get_type() == PHY_PROCEDURE )
  {
    SpProcedure *proc = static_cast<SpProcedure*>(my_phy_plan_->get_main_query());
    proc->get_casted_buf(varchar_buff, buf);
  }
  else
  {
    ret = OB_NOT_SUPPORTED;
  }

  ObObj casted_cell;
  ObString varchar;
  for (int64_t j = 0; OB_SUCCESS == ret && j < col_num; ++j)
  {
    varchar.assign_ptr(varchar_buff, OB_MAX_VARCHAR_LENGTH);
    casted_cell.set_varchar(varchar);
    const ObObj *single_value = NULL;
    uint64_t table_id = OB_INVALID_ID;
    uint64_t column_id = OB_INVALID_ID;
    ObObj tmp_value;
    ObObj data_type;
    ObSqlExpression &val_expr = values_.at(expr_idx_+j);
    if ((ret = val_expr.calc(row_, single_value)) != OB_SUCCESS) // the expr should be a const expr here
    {
      TBSYS_LOG(WARN, "Calculate value result failed, err=%d", ret);
    }
    else if (OB_SUCCESS != (ret = row_desc_ext_.get_by_idx(j, table_id, column_id, data_type)))
    {
      ret = OB_ERR_UNEXPECTED;
      TBSYS_LOG(WARN, "Failed to get column, err=%d", ret);
    }
    else if (OB_SUCCESS != (ret = obj_cast(*single_value, data_type, casted_cell, single_value)))
    {
      TBSYS_LOG(WARN, "incorrect data type, err=%d", ret);
    }
    else if (OB_SUCCESS != (ret = buf->write_obj(*single_value, &tmp_value)))
    {
      TBSYS_LOG(WARN, "str buf write obj fail:ret[%d]", ret);
    }
    else if ((ret = row_.set_cell(table_id, column_id, tmp_value)) != OB_SUCCESS)
    {
      TBSYS_LOG(WARN, "Add value to ObRow failed");
    }
  }
  if( OB_SUCCESS == ret )
  {
    row = &row_;
    expr_idx_ += row_desc_.get_column_num();
  }
  return ret;
}

//add by zt 20160119:e
