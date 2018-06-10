/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_mem_sstable_scan.cpp
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
 * ob_mem_sstable_scan.cpp
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@alipay.com>
 *
 */

#include "ob_mem_sstable_scan.h"
#include "ob_table_rpc_scan.h"
#include "common/ob_common_stat.h"

#include "ob_sp_procedure.h"

using namespace oceanbase;
using namespace common;
using namespace sql;

ObMemSSTableScan::ObMemSSTableScan()
  :from_deserialize_(false), tmp_table_subquery_(common::OB_INVALID_ID)
{
}

ObMemSSTableScan::~ObMemSSTableScan()
{
}

void ObMemSSTableScan::reset()
{
  //cur_row_.reset(false, ObRow::DEFAULT_NULL);
  cur_row_desc_.reset();
  row_store_.clear();
  from_deserialize_ = false;
  tmp_table_subquery_ = common::OB_INVALID_ID;
}

void ObMemSSTableScan::reuse()
{
  cur_row_desc_.reset();
  row_store_.clear();
  from_deserialize_ = false;
  tmp_table_subquery_ = common::OB_INVALID_ID;
}

int ObMemSSTableScan::open()
{
  int ret = OB_SUCCESS;
  if (!from_deserialize_)
  {
    ret = OB_ERR_UNEXPECTED;
    TBSYS_LOG(WARN, "this operator can only open after deserialize");
  }
  else
  {
    cur_row_.set_row_desc(cur_row_desc_);

    //add by zt 20160114:b
    if( proc_exec_ )
    {
      if( OB_SUCCESS != (ret = prepare_data() ))
      {
        TBSYS_LOG(WARN, "prepare_data()=>%d", ret);
      }
    }
    //add by zt 20160114:e
  }
  return ret;
}

int ObMemSSTableScan::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  int64_t start_ts = tbsys::CTimeUtil::getTime();
//  ret = row_store_.get_next_row(cur_row_);  //delete by zt 20160114
  ret = row_store_ptr_->get_next_row(cur_row_); //add by zt 20160114, for template execution
  if (OB_ITER_END == ret)
  {
    TBSYS_LOG(DEBUG, "end of iteration");
  }
  else if (OB_SUCCESS != ret)
  {
    TBSYS_LOG(WARN, "fail to get next row from row store:ret[%d]", ret);
  }
  else
  {
    row = &cur_row_;
    TBSYS_LOG(DEBUG, "[MemSSTableScan] %s", to_cstring(cur_row_));
  }
  OB_STAT_INC(UPDATESERVER, UPS_EXEC_MEM_SSTABLE, tbsys::CTimeUtil::getTime() - start_ts);
  return ret;
}

int ObMemSSTableScan::close()
{
  return OB_SUCCESS;
}

int ObMemSSTableScan::set_child(int32_t child_idx, ObPhyOperator &child_operator)
{
  UNUSED(child_idx);
  UNUSED(child_operator);
  return OB_NOT_IMPLEMENT;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObMemSSTableScan, PHY_MEM_SSTABLE_SCAN);
  }
}

int64_t ObMemSSTableScan::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "MemSSTableScan(static_data_subquery=%lu", tmp_table_subquery_);
  databuff_printf(buf, buf_len, pos, ", row_desc=");
  pos += cur_row_desc_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, ", row_store=");
  pos += row_store_.to_string(buf+pos, buf_len-pos);
  databuff_printf(buf, buf_len, pos, ", from_deserialize=%c", from_deserialize_?'Y':'N');
  databuff_printf(buf, buf_len, pos, ")\n");
  return pos;
}

int ObMemSSTableScan::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (cur_row_desc_.get_column_num() <= 0)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(WARN, "cur_row_desc_ is empty");
  }
  else
  {
    row_desc = &cur_row_desc_;
  }
  return ret;
}

ObPhyOperatorType ObMemSSTableScan::get_type() const
{
  return PHY_MEM_SSTABLE_SCAN;
}

PHY_OPERATOR_ASSIGN(ObMemSSTableScan)
{
  int ret = OB_SUCCESS;
  CAST_TO_INHERITANCE(ObMemSSTableScan);
  reset();
  tmp_table_subquery_ = o_ptr->tmp_table_subquery_;
  return ret;
}

DEFINE_SERIALIZE(ObMemSSTableScan)
{
  int ret = OB_SUCCESS;
  const common::ObRowDesc *row_desc = NULL;
  ObValues *tmp_table = NULL;
  //add by zt 20160114:b
  TBSYS_LOG(TRACE, "ups exec mode: %d", my_phy_plan_->is_group_exec());
  if( OB_SUCCESS != serialization::encode_bool(buf, buf_len, pos, my_phy_plan_->is_group_exec() ))
  {
    TBSYS_LOG(WARN, "failed to serialize proc_exec flag");
  }
  else if( my_phy_plan_->is_group_exec() )
  {
    if( OB_SUCCESS != (ret = serialize_template(buf, buf_len, pos)) )
    {
      TBSYS_LOG(WARN, "failed to serialize template version");
    }
  }
  //add by zt 20160114:e
  else
  if (common::OB_INVALID_ID == tmp_table_subquery_)
  {
    TBSYS_LOG(WARN, "tmp_table_ is NULL");
    ret = OB_NOT_INIT;
  }
  else if (NULL == (tmp_table = dynamic_cast<ObValues*>(my_phy_plan_->get_phy_query_by_id(tmp_table_subquery_))))
  {
    TBSYS_LOG(ERROR, "invalid subqeury id=%lu", tmp_table_subquery_);
  }
  //add zt 20151203:b
  //make sure the tmp_table is opened
  //delete by zt 20160115 not necessary any more
//  else if( ! tmp_table->is_opened() )
//  {
//    ret = const_cast<ObValues*>(tmp_table)->open();
//  }

//  if( OB_SUCCESS != ret ) {}
  //add zt 20151203:e
  else if (OB_SUCCESS != (ret = tmp_table->get_row_desc(row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get row desc, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = row_desc->serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to serialize row desc:ret[%d]", ret);
  }
  else if (OB_SUCCESS != (ret = tmp_table->get_row_store().serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to serialize row store:ret[%d]", ret);
  }
  return ret;
}

DEFINE_DESERIALIZE(ObMemSSTableScan)
{
  int ret = OB_SUCCESS;
  int64_t start_ts = tbsys::CTimeUtil::getTime();
  //add by zt 20160114:b
  if( OB_SUCCESS != (ret = serialization::decode_bool(buf, data_len, pos, &proc_exec_)))
  {
    TBSYS_LOG(WARN, "fail to deserialize proc_exec flag");
  }
  else if( proc_exec_ )
  {
    if( OB_SUCCESS != (ret = deserialize_template(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to deserialize template version");
    }
    else
    {
      from_deserialize_ = true;
      row_store_ptr_ = NULL; //row_store should be got from the ObUpsProcedure.
    }
  }
  else
  //add by zt 20160114:e
  if (OB_SUCCESS != (ret = cur_row_desc_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to deserialize row desc:ret[%d]", ret);
  }
  else if (OB_SUCCESS != (ret = row_store_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to deserialize row_store:ret[%d]", ret);
  }
  else
  {
    from_deserialize_ = true;
    row_store_ptr_ = &row_store_; //add by zt 20160114
  }
  TBSYS_LOG(TRACE, "success[%p], mode [%d], row_desc [%s], static_id [%ld]",
            this,
            proc_exec_,
            to_cstring(cur_row_desc_),
            tmp_table_subquery_);
  OB_STAT_INC(UPDATESERVER, UPS_GEN_MEM_SSTABLE, tbsys::CTimeUtil::getTime() - start_ts);
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObMemSSTableScan)
{
  int64_t size = 0;
  size += cur_row_desc_.get_serialize_size();
  size += row_store_.get_serialize_size();
  return size;
}

//add by zt 20160114:b
int ObMemSSTableScan::serialize_template(char *buf, const int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  const common::ObRowDesc *row_desc = NULL;
  ObValues *tmp_table = NULL;

  if (common::OB_INVALID_ID == tmp_table_subquery_)
  {
    TBSYS_LOG(WARN, "tmp_table_ is NULL");
    ret = OB_NOT_INIT;
  }
  else if (NULL == (tmp_table = dynamic_cast<ObValues*>(my_phy_plan_->get_phy_query_by_id(tmp_table_subquery_))))
  {
    TBSYS_LOG(ERROR, "invalid subqeury id=%lu", tmp_table_subquery_);
  }
  else if (OB_SUCCESS != (ret = tmp_table->get_row_desc_template(row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get row desc, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = row_desc->serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to serialize row desc:ret[%d]", ret);
  }
  else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, tmp_table->get_static_data_id())) )
  {
    TBSYS_LOG(WARN, "failed to serialize static data id");
  }
  return ret;
}

int ObMemSSTableScan::deserialize_template(const char *buf, const int64_t data_len, int64_t &pos)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = cur_row_desc_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "fail to deserialize row desc:ret[%d]", ret);
  }
  else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, (int64_t *)&tmp_table_subquery_)))
  {
    TBSYS_LOG(WARN, "failed to deserialize static data id");
  }
  return ret;
}

int ObMemSSTableScan::prepare_data()
{
  //try to read static data from the ObUpsProcedure here
  int ret = OB_SUCCESS;
  SpProcedure *proc = dynamic_cast<SpProcedure*>(my_phy_plan_->get_main_query());
  if( NULL == proc )
  {
    TBSYS_LOG(WARN, "does not support data preparation outsize of procedure execution");
    ret = OB_NOT_INIT;
  }
  else if( OB_SUCCESS != (ret = proc->get_static_data_by_id(tmp_table_subquery_, row_store_ptr_)))
  {
    TBSYS_LOG(WARN, "can not get static data[%ld]", tmp_table_subquery_);
  }
  else
  {
    TBSYS_LOG(TRACE, "get static data[%p]: %ld", row_store_ptr_, tmp_table_subquery_);
  }
  return ret;
}
//add by zt 20160114:e
