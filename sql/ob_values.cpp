/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_values.cpp
 * @brief the value physical plan operator
 *
 * mofied by zhutao:add some content for procedure in ObValues class
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
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
 * ob_values.cpp
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#include "ob_values.h"
#include "common/utility.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

//add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
ObValuesKeyInfo::ObValuesKeyInfo(){
    tid_=OB_INVALID_ID;
    cid_=OB_INVALID_ID;
    type_=ObNullType;
    precision_=38;
    scale_=0;
}
ObValuesKeyInfo::~ObValuesKeyInfo(){

}
//add e

ObValues::ObValues() : is_open_(false), static_data_id_(-1),
    //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
    obj_array_(common::OB_MAX_ROWKEY_COLUMN_NUMBER*sizeof(ObValuesKeyInfo), ModulePageAllocator(ObModIds::OB_SQL_EXPR))
    //add e
{
    //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
    is_need_fix_obvalues=false;
    //add e
}

ObValues::~ObValues()
{
}
//add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
void ObValues::set_fix_obvalues(){
    is_need_fix_obvalues=true;
}
//add e
void ObValues::reset()
{
  row_desc_.reset();
  //curr_row_.reset(false, ObRow::DEFAULT_NULL);
  row_store_.clear();

  is_open_ = false; //add zt : 20151107
  ObSingleChildPhyOperator::reset();
  //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
  is_need_fix_obvalues=false;
  //add e
}

void ObValues::reuse()
{
  row_desc_.reset();
  //curr_row_.reset(false, ObRow::DEFAULT_NULL);
  row_store_.clear();

  is_open_ = false; //add by zt 20151107
  static_data_id_ = -1; //add by zt 20160118
  ObSingleChildPhyOperator::reset();
  //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
  is_need_fix_obvalues=false;
  //add e
}

int ObValues::set_row_desc(const common::ObRowDesc &row_desc)
{
  TBSYS_LOG(DEBUG, "DEBUG ObValues set row desc %s", to_cstring(row_desc));
  row_desc_ = row_desc;
  return OB_SUCCESS;
}

int ObValues::add_values(const common::ObRow &value)
{
  const ObRowStore::StoredRow *stored_row = NULL;
  return row_store_.add_row(value, stored_row);
}

int ObValues::open()
{
  int ret = OB_SUCCESS;
  curr_row_.set_row_desc(row_desc_);
  if (NULL != child_op_  && !is_open_ ) //modify zt 20151107
  {
    if (OB_SUCCESS != (ret = load_data()))
    {
      TBSYS_LOG(WARN, "failed to load data from child op, err=%d", ret);
    }
    is_open_ = true; //add zt 20151107
  }
  return ret;
}

int ObValues::close()
{
  row_store_.clear();
  is_open_ = false; //add zt 20151124, avoid reopened by ObUpsExecutor
  return OB_SUCCESS;
}

int ObValues::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = row_store_.get_next_row(curr_row_)))
  {
    if (OB_ITER_END != ret)
    {
      TBSYS_LOG(WARN, "failed to get next row from row store, err=%d", ret);
    }
  }
  else
  {
    row = &curr_row_;
  }
  return ret;
}

int ObValues::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  row_desc = &row_desc_;
  return OB_SUCCESS;
}

//add by zt 20160114:b
int ObValues::get_row_desc_template(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  UNUSED(row_desc);
  if( NULL != child_op_ )
  {
    ret = child_op_->get_row_desc(row_desc);
  }
  else
  {
    row_desc = &row_desc_;
  }
  return ret;
}
//add by zt 20160114:e

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObValues, PHY_VALUES);
  }
}

int64_t ObValues::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "Values(row_store=%s)\n", to_cstring(row_store_));
  if (NULL != child_op_)
  {
    pos += child_op_->to_string(buf+pos, buf_len-pos);
  }
  return pos;
}

PHY_OPERATOR_ASSIGN(ObValues)
{
  int ret = OB_SUCCESS;
  CAST_TO_INHERITANCE(ObValues);
  reset();
  ObRowStore *store_ptr = const_cast<ObRowStore*>(&o_ptr->row_store_);
  if ((ret = row_desc_.assign(o_ptr->row_desc_)) == OB_SUCCESS)
  {
    ObRow row;
    int64_t cur_size_counter;
    store_ptr->reset_iterator();
    while ((ret = store_ptr->get_next_row(row)) == OB_SUCCESS)
    {
      if ((ret = row_store_.add_row(row, cur_size_counter)) != OB_SUCCESS)
      {
        break;
      }
    }
    if (ret == OB_ITER_END)
    {
      ret = OB_SUCCESS;
    }
    store_ptr->reset_iterator();
  }
  static_data_id_ = o_ptr->static_data_id_; //add by zt 20160118
  return ret;
}

DEFINE_SERIALIZE(ObValues)
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
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
  return ret;
}

DEFINE_DESERIALIZE(ObValues)
{
  int ret = OB_SUCCESS;
  int64_t tmp_pos = pos;
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
    pos = tmp_pos;
  }
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObValues)
{
  return (row_desc_.get_serialize_size() + row_store_.get_serialize_size());
}

int ObValues::load_data()
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
  const ObRow *row = NULL;
  const ObRowDesc *row_desc = NULL;
  const ObRowStore::StoredRow *stored_row = NULL;

  if (OB_SUCCESS != (ret = child_op_->open()))
  {
    TBSYS_LOG(WARN, "fail to open rpc scan:ret[%d]", ret);
  }

  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = child_op_->get_row_desc(row_desc)))
    {
      TBSYS_LOG(WARN, "fail to get row_desc:ret[%d]", ret);
    }
    else
    {
      row_desc_ = *row_desc;
    }
  }

  while (OB_SUCCESS == ret)
  {
    ret = child_op_->get_next_row(row);
    if (OB_ITER_END == ret)
    {
      ret = OB_SUCCESS;
      break;
    }
    else if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "fail to get next row from rpc scan");
    }
    else
    {
        //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        /*
        TBSYS_LOG(DEBUG, "load data from child, row=%s", to_cstring(*row));
        if (OB_SUCCESS != (ret = row_store_.add_row(*row, stored_row)))
        {
          TBSYS_LOG(WARN, "fail to add row:ret[%d]", ret);
        }
        */
        TBSYS_LOG(DEBUG, "load data from child, row=%s", to_cstring(*row));
        if(is_need_fix_obvalues)
        {
          //TBSYS_LOG(INFO,"xushilei,is_need_fix_obvalues!");
          int ret_v2=OB_SUCCESS;
          ModuleArena row_alloc(OB_MAX_ROW_LENGTH, ModulePageAllocator(ObModIds::OB_SQL_RPC_SCAN));
          ObRow row_v2=*row;
          //const ObRowkey* rowkey = NULL;
          // if(OB_SUCCESS!=(ret_v2=row->get_rowkey(rowkey)))
          //{
          //TBSYS_LOG(ERROR,"test in get_rowkey");
          //}
          // else
          // {
            uint64_t table_id = OB_INVALID_ID;
            uint64_t column_id = OB_INVALID_ID;
            const ObObj *ori_cell = NULL;
            for(int64_t j=0;j<row_v2.get_column_num();j++)
            {
                if(OB_SUCCESS!=(ret_v2=row_v2.raw_get_cell(j, ori_cell, table_id, column_id)))
                {}
                else if(ori_cell!=NULL)
                {
                    //for(int32_t i=0;i<rowkey->get_obj_cnt();i++)
                    if(is_rowkey_column(table_id, column_id))
                    {
                        //ObObj cell = rowkey->get_obj_ptr()[i];
                        ObObjType schema_type;
                        uint32_t schema_p;
                        uint32_t schema_s;
                        if(OB_SUCCESS!=(ret_v2=get_rowkey_schema(table_id,column_id,schema_type,schema_p,schema_s)))
                        {}
                        else
                        {
                          ObObj result_cell;
                          ObObj tmp_cell;
                          if(ObDecimalType==schema_type&&ori_cell->get_type()==ObDecimalType)
                          {
                             //modify xsl ECNU_DECIMAL 2016_12
                             uint64_t *t1 = NULL;
                             t1 = ori_cell->get_ttint();
                             tmp_cell.set_decimal(t1,schema_p,schema_s,ori_cell->get_vscale(),ori_cell->get_nwords());
                             //modify e
                             if(OB_SUCCESS!=(ret_v2=ob_write_obj_v2(row_alloc,tmp_cell,result_cell)))
                             {}
                             else if(OB_SUCCESS!=(ret_v2=row_v2.set_cell(table_id,column_id,result_cell)))
                             {}
                          }
                        }
                    }
                }
            }
          //}
          if(OB_SUCCESS==ret_v2)
          {
            if (OB_SUCCESS != (ret = row_store_.add_row(row_v2, stored_row)))
            {
                TBSYS_LOG(WARN, "fail to add row:ret[%d]", ret);
            }
          }
          else if (OB_SUCCESS != (ret = row_store_.add_row(*row, stored_row)))
          {
              TBSYS_LOG(WARN, "fail to add row:ret[%d]", ret);
          }
          row_alloc.free();
        }
        else
        {
          if (OB_SUCCESS != (ret = row_store_.add_row(*row, stored_row)))
          {
              TBSYS_LOG(WARN, "fail to add row:ret[%d]", ret);
          }
        }
          //modify e
    }
  }
  if (OB_SUCCESS != (err = child_op_->close()))
  {
    TBSYS_LOG(WARN, "fail to close rpc scan:err[%d]", err);
    if (OB_SUCCESS == ret)
    {
      ret = err;
    }
  }
  return ret;
}
//add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
void ObValues::add_rowkey_array(uint64_t tid,uint64_t cid,common::ObObjType type,uint32_t p,uint32_t s){

       ObValuesKeyInfo oki;
       oki.set_key_info(tid,cid,p,s,type);
       obj_array_.push_back(oki);
}

bool ObValues::is_rowkey_column(uint64_t tid,uint64_t cid){
    ObValuesKeyInfo oki;
    bool ret=false;
    for(int i=0;i<obj_array_.count();i++){
        obj_array_.at(i,oki);
        if(oki.is_rowkey(tid,cid)){
                ret=true;
                break;
        }
    }
    return ret;

}

int ObValues::get_rowkey_schema(uint64_t tid,uint64_t cid,common::ObObjType& type,uint32_t& p,uint32_t& s){
    int ret=OB_ERROR;
    ObValuesKeyInfo oki;
    for(int i=0;i<obj_array_.count();i++){
        obj_array_.at(i,oki);
        if(oki.is_rowkey(tid,cid)){
            oki.get_type(type);
            oki.get_key_info(p,s);
            ret=OB_SUCCESS;
            break;
        }

    }
    if(OB_SUCCESS!=ret){
        TBSYS_LOG(WARN,"can not find rowkey info in obvalues!tid=%ld,cid=%ld",tid,cid);
    }
    return ret;
}
//add e
