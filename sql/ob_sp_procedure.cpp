/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_sp_procedure.h
 * @brief procedure instruction and operator physical plan realation class definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_sp_procedure.h"
#include "ob_physical_plan.h"
#include "common/ob_obj_cast.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

/* ========================================================
 *      SpVar Definition
 * =======================================================*/
int64_t SpVar::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "SpVar: ");
  pos += var_name_.to_string(buf + pos, buf_len - pos);

  if( !idx_value_.is_null() )
  {
    databuff_printf(buf, buf_len, pos, ", idx[%s]: ", to_cstring(idx_value_));
  }

  return pos;
}

int SpVar::serialize(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = var_name_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize var_name fail, ret=%d", ret);
  }
  else if( OB_SUCCESS != (ret = serialization::encode_bool(buf, buf_len, pos, !idx_value_.is_null())) )
  {
    TBSYS_LOG(WARN, "serialize idx flag fail, ret=%d", ret);
  }
  else if( !idx_value_.is_null() )
  {
    ret = idx_value_.serialize(buf, buf_len, pos);
  }
  return ret;
}

int SpVar::deserialize(const char *buf, int64_t data_len, int64_t &pos)
{
  int ret = OB_SUCCESS;
  bool has_idx = false;
  if( OB_SUCCESS != (ret = var_name_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize var_name fail, ret=%d", ret);
  }
  else if( OB_SUCCESS != (ret = serialization::decode_bool(buf, data_len, pos, &has_idx)))
  {
    TBSYS_LOG(WARN, "deserialize idx flag fail, ret=%d", ret);
  }
  else if( has_idx )
  {
    ret = idx_value_.deserialize(buf, data_len, pos);
  }
  else
  {
    idx_value_.set_null();
  }
  return ret;
}

int SpVar::assign(const SpVar &other)
{
  var_name_ = other.var_name_;
  idx_value_ = other.idx_value_;
  return OB_SUCCESS;
}


SpVar::~SpVar()
{
}

int64_t SpVarInfo::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  if( var_type_ == VM_TMP_VAR )
  {
    databuff_printf(buf, buf_len, pos, "%.*s(var)", var_name_.length(), var_name_.ptr());
  }
  else if( var_type_ == VM_FUL_ARY )
  {
    databuff_printf(buf, buf_len, pos, "%.*s[%s](arr)", var_name_.length(), var_name_.ptr(),
                    to_cstring(idx_value_));
  }
  else if( var_type_ == VM_DB_TAB )
  {
    databuff_printf(buf, buf_len, pos, "%s(tab)", to_cstring(idx_value_));
  }
  return pos;
}

bool SpVarInfo::conflict(const SpVarInfo &a, const SpVarInfo &b)
{
  bool ret = false;
  if( a.var_type_ == b.var_type_ && a.var_type_ == VM_DB_TAB )
  {
    ret = (a.idx_value_.compare(b.idx_value_) == 0);
  }
  else if ( a.var_type_ == b.var_type_ )
  {
    ret = (a.var_name_.compare(b.var_name_) == 0);
  }
  else if( VM_FUL_ARY == a.var_type_ &&
           VM_TMP_VAR == b.var_type_ &&
           ObVarcharType == a.idx_value_.get_type())
  {
    ObString idx_var;
    a.idx_value_.get_varchar(idx_var);
    ret = (b.var_name_.compare(idx_var) == 0);
  }
  else if( VM_FUL_ARY == b.var_type_ &&
           VM_TMP_VAR == a.var_type_ &&
           ObVarcharType == b.idx_value_.get_type())
  {
    ObString idx_var;
    b.idx_value_.get_varchar(idx_var);
    ret = (a.var_name_.compare(idx_var) == 0);
  }
  return ret;
}

bool SpVarInfo::compare(const SpVarInfo &other) const
{
  bool ret = false;
  if( var_type_ == other.var_type_ )
  {
    if( var_type_ == VM_DB_TAB ) ret = (idx_value_ == other.idx_value_);
    else
    {
      ret = (var_name_.compare(other.var_name_) == 0 );
    }
  }
  return ret;
}

int SpVariableSet::add_tmp_var(const ObString &var_name)
{
  return add_var_info(SpVarInfo(var_name));
}

int SpVariableSet::add_tmp_var(const ObIArray<ObString> &var_set)
{
  int ret = OB_SUCCESS;
  for(int64_t i = 0; OB_SUCCESS == ret && i < var_set.count(); ++i)
  {
    const ObString &var_name = var_set.at(i);
    ret = add_tmp_var(var_name);
  }
  return ret;
}

int SpVariableSet::add_array_var(const ObString &arr_name, const ObObj &idx_value)
{
  return  add_var_info(SpVarInfo(arr_name, idx_value));
}

int SpVariableSet::add_table_id(const uint64_t table_id)
{
  return add_var_info(SpVarInfo(table_id));
}

int SpVariableSet::add_var(const SpVar &var)
{
  int ret = OB_SUCCESS;
  if( var.is_array() )
  {
    ret = add_array_var(var.var_name_, var.idx_value_);
  }
  else
  {
    ret = add_tmp_var(var.var_name_);
  }
  return ret;
}

int SpVariableSet::add_var_info(const SpVarInfo &var_info)
{
  int ret = OB_SUCCESS;
  bool flag = false;
  for(int64_t i = 0; i < var_info_set_.count() && !flag; ++i)
  {
    const SpVarInfo &info = var_info_set_.at(i);
    flag = var_info.compare(info);
  }
  if( !flag ) ret = var_info_set_.push_back(var_info);
  return ret;
}

int SpVariableSet::add_var_info_set(const SpVariableSet &var_set)
{
  int ret = OB_SUCCESS;
  for(int64_t i = 0; OB_SUCCESS == ret && i < var_set.var_info_set_.count(); ++i)
  {
    ret = add_var_info(var_set.var_info_set_.at(i));
  }
  return ret;
}

int64_t SpVariableSet::to_string(char *buf, int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "[ ");
  for(int64_t i = 0; i < var_info_set_.count(); ++i)
  {
    databuff_printf(buf, buf_len, pos, "%s ", to_cstring(var_info_set_.at(i)));
  }
  databuff_printf(buf, buf_len, pos, "]");
  return pos;
}

int SpVariableSet::conflict(const SpVariableSet &in_set, const SpVariableSet &out_set)
{
  int ret = 0;
  for(int64_t i = 0; i < in_set.count(); ++i)
  {
    const SpVarInfo &in_var = in_set.var_info_set_.at(i);
    for(int64_t j = 0; j < out_set.count(); ++j)
    {
      const SpVarInfo &out_var = out_set.var_info_set_.at(j);
      if( SpVarInfo::conflict(in_var, out_var) )
      {
        if( in_var.var_type_ == VM_DB_TAB) ret |= 2;
        else ret |= 1;
      }
    }
  }
  return ret;
}

bool SpVariableSet::exist(const ObString &var_name) const
{
  bool ret = false;
  for(int64_t i = 0; i < var_info_set_.count(); ++i)
  {
    const ObString var = var_info_set_.at(i).var_name_;
    if( var_name.compare(var) == 0 )
    {
      ret = true;
      break;
    }
  }
  return ret;
}

/*===================================================================
 *                       SpInst Definition
 * ==================================================================*/
SpInst::~SpInst()
{
  proc_ = NULL;
}

int SpInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  UNUSED(buf);
  UNUSED(buf_len);
  UNUSED(pos);
  TBSYS_LOG(WARN, "Could not serialize inst[%d]", type_);
  return OB_ERROR;
}

int SpInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  UNUSED(buf);
  UNUSED(data_len);
  UNUSED(pos);
  UNUSED(allocator);
  UNUSED(operators_store);
  UNUSED(op_factory);
  TBSYS_LOG(WARN, "Could not deserialize inst[%d]", type_);
  return OB_ERROR;
}

int SpInst::check_dep(SpInst &inst_in, SpInst &inst_out)
{
  int ret = 0;
  int conflict;
  SpVariableSet in_rs, in_ws, out_rs, out_ws;
  inst_in.get_read_variable_set(in_rs);
  inst_in.get_write_variable_set(in_ws);
  inst_out.get_read_variable_set(out_rs);
  inst_out.get_write_variable_set(out_ws);

  if ( inst_in.get_type() == SP_B_INST )
  {
    if(static_cast<SpRdBaseInst&>(inst_in).get_rw_id() == inst_out.id_)
    {
      ret = Da_True_Dep;
    }
  }

  if( 0 != (conflict = SpVariableSet::conflict(in_rs, out_ws)) )
  {
    if( 2 == (conflict & 2) ) ret |= Tr_Itm_Dep;
    if( 1 == (conflict & 1) ) ret |= Da_Anti_Dep;
  }

  if( 0 != (conflict = SpVariableSet::conflict(in_ws, out_rs)) )
  {
    if( 2 == (conflict &2) ) ret |= Tr_Itm_Dep;
    if( 1 == (conflict &1) ) ret |= Da_True_Dep;
  }

  if( 0 != (conflict = SpVariableSet::conflict(in_ws, out_ws)) )
  {
    if( 2 == (conflict &2) ) ret |= Tr_Itm_Dep;
    if( 1 == (conflict &1) ) ret |= Da_Out_Dep;
  }

  return ret;
}

/* ==============================================
 *    SpExprInst Definition
 * ===============================================*/
SpExprInst::~SpExprInst()
{
//  left_var_.clear();
}

void SpExprInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
}

void SpExprInst::get_write_variable_set(SpVariableSet &write_set) const
{
//  write_set.add_tmp_var(left_var_.var_name_);
  write_set.add_var(left_var_);
}


int SpExprInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = left_var_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize left_var_ fail, ret=%d", ret);
  }
  else if( OB_SUCCESS != (ret = right_val_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize right_val_ fail, ret=%d", ret);
  }
  return ret;
}

int SpExprInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  UNUSED(allocator);
  UNUSED(operators_store);
  UNUSED(op_factory);
  TBSYS_LOG(TRACE, "deserialize expr inst");

  if( OB_SUCCESS != (ret = left_var_.deserialize(buf, data_len, pos)) )
  {
    TBSYS_LOG(WARN, "deserialize left_var_ fail, ret=%d", ret);
  }
  else if( OB_SUCCESS != (ret = right_val_.deserialize(buf, data_len, pos)) )
  {
    TBSYS_LOG(WARN, "deserialize right_val_ fail, ret=%d", ret);
  }
  else
  {
    right_val_.set_owner_op(proc_);
  }
  return ret;
}

int SpExprInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  const SpExprInst *old_expr = static_cast<const SpExprInst*>(inst);

  left_var_.assign(old_expr->left_var_);
//  if( left_var_.idx_value_ != NULL ) left_var_.idx_value_ ->set_owner_op(proc_);
  right_val_ = old_expr->right_val_;
  right_val_.set_owner_op(proc_);
  rs_ = old_expr->rs_;
  return ret;
}

/* ===============================================
 *    SpRdBaseInst Definition
 * ==============================================*/
SpRdBaseInst::~SpRdBaseInst()
{}

void SpRdBaseInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
}

void SpRdBaseInst::get_write_variable_set(SpVariableSet &write_set) const
{
  UNUSED(write_set);
}

int SpRdBaseInst::set_rdbase_op(ObPhyOperator *op, int32_t query_id)
{
  int ret = OB_SUCCESS;
  OB_ASSERT(op->get_type() == PHY_VALUES);
  op_ = op;
  sdata_id_ = proc_->generate_static_data_id();
  static_cast<ObValues*>(op_)->set_static_data_id(sdata_id_);
  query_id_ = query_id;
  return ret;
}

int SpRdBaseInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;

  const SpRdBaseInst *old_inst = static_cast<const SpRdBaseInst*>(inst);

  op_ = NULL; //get the op_ from query_id_'s inner plan
  query_id_ = old_inst->query_id_;
  rs_ = old_inst->rs_;
  table_id_ = old_inst->table_id_;
  for_group_exec_ = old_inst->for_group_exec_;
  return ret;
}

void SpRdBaseInst::set_exec_mode()
{
  SpInst *rw_inst = NULL;
  if( rw_inst_id_ != -1 && OB_SUCCESS == get_ownner()->get_inst_by_id(rw_inst_id_, rw_inst) )
  {
    for_group_exec_ = rw_inst->in_group_exec();
  }
}

/* ========================================================
 *      SpRwDeltaInst Definition
 * =======================================================*/
SpRwDeltaInst::~SpRwDeltaInst()
{}


void SpRwDeltaInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
  read_set.add_table_id(table_id_);
}

void SpRwDeltaInst::get_write_variable_set(SpVariableSet &write_set) const
{
  write_set.add_table_id(table_id_);
}

int SpRwDeltaInst::set_rwdelta_op(ObPhyOperator *op)
{
  int ret = OB_SUCCESS;
//  OB_ASSERT(op->get_type() == PHY_UPS_EXECUTOR);
  op_ = op;
  return ret;
}

int SpRwDeltaInst::set_ups_exec_op(ObPhyOperator *op, int32_t query_id)
{
  int ret = OB_SUCCESS;
  OB_ASSERT(op->get_type() == PHY_UPS_EXECUTOR);
  ups_exec_op_ = op;
  query_id_ = query_id;
  return ret;
}

int SpRwDeltaInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
//  ObUpsExecutor *ups_exec = static_cast<ObUpsExecutor *>(op_);
//  ObPhyOperator *ups_main_query = ups_exec->get_inner_plan()->get_main_query();

  //op_'s phyplan is different with proc_'s phyplan, op_ is in the ups_executor's innerplan
  op_->get_phy_plan()->set_group_exec(true);
  static_cast<ObPhyOperator*>(op_)->get_phy_plan()->set_result_set(ups_exec_op_->get_phy_plan()->get_result_set());
  if( OB_SUCCESS != (ret = proc_->serialize_tree(buf, buf_len, pos, *op_)) )
  {
    TBSYS_LOG(WARN, "Serialize ups main query fail: ret=%d", ret);
  }
  return ret;
}

int SpRwDeltaInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(TRACE, "deserialize rwdelta");
  if( OB_SUCCESS != (ret = proc_->deserialize_tree(buf, data_len, pos, allocator, operators_store, op_factory, op_)) )
  {
    TBSYS_LOG(WARN, "Deserialize rw delta inst query fail: ret=%d", ret);
  }
  return ret;
}

int SpRwDeltaInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  const SpRwDeltaInst *old_inst = static_cast<const SpRwDeltaInst*>(inst);
  op_ = NULL;
  ups_exec_op_ = NULL;
  query_id_ = old_inst->query_id_;
  rs_ = old_inst->rs_;
//  ws_ = old_inst->ws_;
  table_id_ = old_inst->table_id_;
  group_exec_ = old_inst->group_exec_;
  return ret;
}

/* ========================================================
 *      SpRwDeltaIntoInst Definition
 * =======================================================*/
SpRwDeltaIntoVarInst::~SpRwDeltaIntoVarInst()
{
}

void SpRwDeltaIntoVarInst::get_write_variable_set(SpVariableSet &write_set) const
{
  for(int64_t i = 0; i < var_list_.count(); ++i)
  {
    write_set.add_var(var_list_.at(i));
  }
}

int SpRwDeltaIntoVarInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
//  ObUpsExecutor *ups_exec = static_cast<ObUpsExecutor *>(op_);
//  ObPhyOperator *ups_main_query = ups_exec->get_inner_plan()->get_main_query();

  //op_'s phyplan is different with proc_'s phyplan, op_ is in the ups_executor's innerplan
  op_->get_phy_plan()->set_group_exec(true);
  /**
   * serialize the main query[ObUpsModifyWithDmlType] to the ups
   * caution. not the ups_executor
   * */
  if( OB_SUCCESS != (ret = proc_->serialize_tree(buf, buf_len, pos, *op_)) )
  {
    TBSYS_LOG(WARN, "Serialize ups main query fail: ret=%d", ret);
  }
  else if ( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, var_list_.count())) )
  {
    TBSYS_LOG(WARN, "Serialize var list count fail: ret=%d", ret);
  }
  else
  {
    for(int64_t i = 0; i < var_list_.count(); ++i)
    {
      if( OB_SUCCESS != ( ret = var_list_.at(i).serialize(buf, buf_len, pos)) )
      {
        TBSYS_LOG(WARN, "Serialize var list[%ld] fail, ret=%d", i, ret);
        break;
      }
    }
  }
  return ret;
}

int SpRwDeltaIntoVarInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;

  int64_t var_count = 0;
  TBSYS_LOG(TRACE, "deserialize RwDeltaInto");
  if( OB_SUCCESS != (ret = proc_->deserialize_tree(buf, data_len, pos, allocator, operators_store, op_factory, op_)))
  {
    TBSYS_LOG(WARN, "Deserialize main query fail: ret=%d", ret);
  }
  else if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &var_count)))
  {
    TBSYS_LOG(WARN, "Deserialize var count fail: ret=%d", ret);
  }
  else
  {
    var_list_.reserve(var_count);
    TBSYS_LOG(TRACE, "var_count: %ld", var_count);
    for(int64_t i = 0; i < var_count; ++i)
    {
//      ObString var_name;
      SpVar tmp_var;
      var_list_.push_back(tmp_var);
      if( OB_SUCCESS != (ret = var_list_.at(i).deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "Deserialize var list[%ld] fail, ret=%d", i, ret);
        break;
      }
    }
  }
  return ret;
}

int SpRwDeltaIntoVarInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  ret = SpRwDeltaInst::assign(inst);
//  var_list_ = static_cast<const SpRwDeltaIntoVarInst*>(inst)->var_list_;
  const ObIArray<SpVar>& old_var_list = static_cast<const SpRwDeltaIntoVarInst*>(inst)->var_list_;

  int64_t var_count = old_var_list.count();

  var_list_.reserve(var_count);
  for(int64_t i = 0; i < var_count; ++i)
  {
    SpVar tmp_var;
    var_list_.push_back(tmp_var);
    if( OB_SUCCESS != (ret = var_list_.at(i).assign(old_var_list.at(i))) )
    {
      TBSYS_LOG(WARN, "assign failure, at %ld", i);
    }
    else
    {
    }
  }
  return ret;
}

/* ========================================================
 *      SpRwCompInst Definition
 * =======================================================*/
SpRwCompInst::~SpRwCompInst()
{
}

void SpRwCompInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
  read_set.add_table_id(table_id_);
}

void SpRwCompInst::get_write_variable_set(SpVariableSet &write_set) const
{
  for(int64_t i = 0; i < var_list_.count(); ++i)
  {
    write_set.add_var(var_list_.at(i));
  }
}

int SpRwCompInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;

  const SpRwCompInst *old_inst = static_cast<const SpRwCompInst*>(inst);
  op_ = NULL;
  query_id_ = old_inst->query_id_;
  rs_ = old_inst->rs_;
//  ws_ = old_inst->ws_;
  table_id_ = old_inst->table_id_;

  int64_t var_count = old_inst->var_list_.count();

  var_list_.reserve(var_count);
  for(int64_t i = 0; i < var_count; ++i)
  {
    SpVar tmp_var;
    var_list_.push_back(tmp_var);
    if( OB_SUCCESS != (ret = var_list_.at(i).assign(old_inst->var_list_.at(i))) )
    {
      TBSYS_LOG(WARN, "assign failure, at %ld", i);
    }
    else
    {
    }
  }
  return ret;
}

SpPlainSQLInst::~SpPlainSQLInst()
{}

void SpPlainSQLInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
}

void SpPlainSQLInst::get_write_variable_set(SpVariableSet &write_set) const
{
  if(op_->get_type() == PHY_UPS_EXECUTOR)
  { //write sql
    write_set.add_table_id(table_id_);
  }
}

void SpPreGroupInsts::get_read_variable_set(SpVariableSet &read_set) const
{
  inst_list_.get_read_variable_set(read_set);
}

void SpPreGroupInsts::get_write_variable_set(SpVariableSet &write_set) const
{
  //PreGroup executes in isolated execution context
  UNUSED(write_set);
//  inst_list_.get_write_variable_set(write_set);
}

int SpPreGroupInsts::add_inst(SpInst *inst)
{
  inst_list_.add_inst(inst);
  inst->get_write_variable_set(write_set_);
  return OB_SUCCESS;
}


/* ========================================================
 *      SpGroupInsts Definition
 * =======================================================*/
SpGroupInsts::~SpGroupInsts()
{
  //sp_block_insts doesn't really own the memory of inst_list_
}

void SpGroupInsts::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(rs_);
}

void SpGroupInsts::get_write_variable_set(SpVariableSet &write_set) const
{
  write_set.add_var_info_set(ws_);
}

int SpGroupInsts::add_inst(SpInst *inst)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != inst_list_.push_back(inst) )
  {
    TBSYS_LOG(WARN, "add instruction fail");
  }
  inst->get_read_variable_set(rs_);
  inst->get_write_variable_set(ws_);
  inst->set_in_group_exec();
  return ret;
}

int SpGroupInsts::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  int64_t count = inst_list_.count();
  int64_t last_pos = pos;

  if( group_proc_name_.compare("neworder") == 0 ) count = 0; //a hack for neworder
  if( group_proc_name_.compare("payment") == 0 ) count = 0; //a hack for payment
  if( group_proc_name_.compare("loopdep") == 0 ) count = 0;
  if( group_proc_name_.compare("amalgamate") == 0 ) count = 0;
  if( group_proc_name_.compare("writecheck") == 0 ) count = 0;
  if( group_proc_name_.compare("sendpayment") == 0 ) count = 0;
  if( group_proc_name_.compare("transactsavings") == 0 ) count = 0;

  if( OB_SUCCESS != (ret = group_proc_name_.serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "serialize group_proc_name fail");
  }
  else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, count)) )
  {
    TBSYS_LOG(WARN, "serialize inst count fail");
  }

  //serialize instructions
  for(int64_t i = 0; i < count && OB_SUCCESS == ret; ++i)
  {
    SpInst *inst = inst_list_.at(i);
    SpInstType type = inst->get_type();
    if( OB_SUCCESS != (ret = serialization::encode_i32(buf, buf_len, pos, type)) )
    {
      TBSYS_LOG(WARN, "serialize inst type fail, %ld", i);
    }
    else if (OB_SUCCESS != (ret = inst->serialize_inst(buf, buf_len,pos)) )
    {
      TBSYS_LOG(WARN, "serialize inst fail, %ld", i);
    }
//    TBSYS_LOG(INFO, "inst %ld, size: %ld", i, pos - last_pos);
    last_pos = pos;
  }

  //serialize read variables
  int64_t rd_var_count = rs_.count();
  int32_t rd_tmp_var_count = 0, rd_array_var_count = 0;
  for(int64_t i = 0; i < rd_var_count; ++i)
  {
    const SpVarInfo & var_info = rs_.get_var_info(i);
    if( var_info.var_type_ == VM_TMP_VAR ) rd_tmp_var_count ++;
    else if( var_info.var_type_ == VM_FUL_ARY ) rd_array_var_count++;
  }

  TBSYS_LOG(TRACE, "Group inst serialization, rd_var count: %ld, tmp_var[%d], array[%d]", rd_var_count, rd_tmp_var_count, rd_array_var_count);
  //serialize temp variables
  if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, rd_tmp_var_count)))
  {
    TBSYS_LOG(WARN, "serialize read temp variables count fail");
  }
  else
  {
    for(int64_t i = 0; OB_SUCCESS == ret && i < rd_var_count; ++i)
    {
      const SpVarInfo &var_info = rs_.get_var_info(i);
      const ObString &var_name = var_info.var_name_;
      if( var_info.var_type_ == VM_TMP_VAR )
      {
        //try to serialize the temp variables
        const ObObj *obj;
        if(OB_SUCCESS != (proc_->read_variable(var_name, obj)) )
        {
          TBSYS_LOG(WARN, "serialize variables [%.*s] fails, does not get value", var_name.length(), var_name.ptr());
        }
        else if( OB_SUCCESS != (ret = var_name.serialize(buf, buf_len, pos)))
        {
          TBSYS_LOG(WARN, "serialzie variable name fail [%.*s]", var_name.length(), var_name.ptr());
        }
        else if ( OB_SUCCESS != (ret = obj->serialize(buf, buf_len, pos)) )
        {
          TBSYS_LOG(WARN, "serialize variable value fail [%s]", to_cstring(*obj));
        }
        else
        {
          TBSYS_LOG(DEBUG, "Group inst seralization, %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(*obj));
        }
      }
    }
  }
  last_pos = pos;
  //serialize array variables
  if( OB_SUCCESS != ret ){}
  else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, rd_array_var_count)) )
  {
    TBSYS_LOG(WARN, "serialize read array variables count fail");
  }
  else
  {
    for(int64_t i = 0; OB_SUCCESS == ret && i < rd_var_count; ++i)
    {
      const SpVarInfo &var_info = rs_.get_var_info(i);
      const ObString &var_name = var_info.var_name_;
      const ObObj *val;
      int64_t size = 0; //array size
      if( var_info.var_type_ == VM_FUL_ARY )
      {
        if( OB_SUCCESS != (ret = proc_->read_array_size(var_name, size)))
        {
          TBSYS_LOG(WARN, "serialize array [%.*s] fails, does not get size", var_name.length(), var_name.ptr());
        }
        else if( OB_SUCCESS != (ret = var_name.serialize(buf, buf_len, pos)))
        {
          TBSYS_LOG(WARN, "serialize variable name fail [%.*s]", var_name.length(), var_name.ptr());
        }
        else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, size)))
        {
          TBSYS_LOG(WARN, "serialize array size fail, %ld", size);
        }
        else
        {
          for(int64_t j = 0; OB_SUCCESS == ret && j < size; ++j)
          {
            if( OB_SUCCESS != (ret = proc_->read_variable(var_name, j, val)))
            {
              TBSYS_LOG(WARN, "serialize %.*s[%ld] fail, can not get value", var_name.length(), var_name.ptr(), j);
            }
            else if( OB_SUCCESS != (ret = val->serialize(buf, buf_len, pos)))
            {
              TBSYS_LOG(WARN, "serialize array values fail %.*s[%ld] = %s", var_name.length(), var_name.ptr(), j, to_cstring(*val));
            }
            else
            {
              TBSYS_LOG(DEBUG, "Group inst serialize, %.*s[%ld] = %s", var_name.length(), var_name.ptr(), j, to_cstring(*val));
            }
          }
        }
      }
    }
  }
  //serialize static data
  int64_t static_data_count = proc_->get_static_data_count();
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, static_data_count)) )
  {
    TBSYS_LOG(WARN, "fail to serialize static data count");
  }
  else
  {
    for(int64_t i = 0; OB_SUCCESS == ret && i < static_data_count; ++i)
    {
      int64_t sdata_id, hkey;
      const ObRowStore *p_row_store = NULL;

      if( OB_SUCCESS != (ret = proc_->get_static_data(i, sdata_id, hkey, p_row_store)) )
      {
        TBSYS_LOG(WARN, "fail to get static data, idx: %ld", i);
      }
      else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, sdata_id)))
      {
        TBSYS_LOG(WARN, "fail to serialize sdata_id");
      }
      else if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, hkey)))
      {
        TBSYS_LOG(WARN, "fail to serialize hkey");
      }
      else if( OB_SUCCESS != (ret = (p_row_store->serialize(buf, buf_len, pos))))
      {
        TBSYS_LOG(WARN, "fail to serialize static data");
      }

      TBSYS_LOG(TRACE, "sid: %ld, hkey: %ld, sdata: %s", sdata_id, hkey, to_cstring(*p_row_store));
    }
  }
  return ret;
}

int SpGroupInsts::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena& allocator,
                                   ObPhysicalPlan::OperatorStore& operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  int64_t count = 0;

  if( OB_SUCCESS != (ret = group_proc_name_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize group_proc_name fail");
  }
  else if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &count)) )
  {
    TBSYS_LOG(WARN, "deserialize inst count fail");
  }
  inst_list_.reserve(count);
  for(int64_t i = 0; i < count && OB_SUCCESS == ret; ++i)
  {
    SpInstType type;
    SpInst *inst = NULL;
    int32_t type_int_value = 0;
    serialization::decode_i32(buf, data_len, pos, &type_int_value);
    type = static_cast<SpInstType>(type_int_value);

    inst = proc_->create_inst(type, NULL);
    if(OB_SUCCESS != (ret = inst->deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory)))
    {
      TBSYS_LOG(WARN, "deserialize inst [%ld] fail, type: %d", i, type_int_value);
    }

    if( OB_SUCCESS == ret )
      add_inst(inst);
    else
    {
      TBSYS_LOG(WARN, "Deserialize instructions %ld fail, ret=%d", i, ret);
      break;
    }
  }

  //Varialbles are saved in ObSqlSessionInfo on ms
  // 	and saved in ObUpsProcedure on ups
  int64_t rd_tmp_var_count = 0;
  int64_t rd_array_var_count = 0;
  if( OB_SUCCESS == ret )
  {
    ret = serialization::decode_i64(buf, data_len, pos, &rd_tmp_var_count);
    TBSYS_LOG(TRACE, "block inst, rd_tmp_var count: %ld", rd_tmp_var_count);
    for(int64_t i = 0; i < rd_tmp_var_count && OB_SUCCESS == ret; ++i)
    {
      ObString var_name;
      ObObj obj;
      if( OB_SUCCESS != (ret = var_name.deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "Deserialize var_name fail");
      }
      else if( OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "Deserialize val fail");
      }
      else if( OB_SUCCESS != (ret = proc_->write_variable(var_name, obj)))
      {
        TBSYS_LOG(WARN, "write variables[%.*s] = %s into table fail", var_name.length(), var_name.ptr(), to_cstring(obj));
      }
      else
      {
        TBSYS_LOG(TRACE, "try to write variables var_name: %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(obj));
      }
    }
  }

  if( OB_SUCCESS == ret && OB_SUCCESS == (ret = serialization::decode_i64(buf, data_len, pos, &rd_array_var_count)) )
  {
    TBSYS_LOG(TRACE, "block inst, rd_array_var: %ld", rd_array_var_count);
    for(int64_t i = 0; i < rd_array_var_count && OB_SUCCESS ==ret; ++i)
    {
      ObString var_name;
      ObObj obj;
      int64_t array_size = 0;
      if( OB_SUCCESS != (ret = var_name.deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "Deserialize var_name fail");
      }
      else if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &array_size)))
      {
        TBSYS_LOG(WARN, "Deserialize array size fail");
      }
      else
      {
        for(int64_t j = 0; j < array_size && OB_SUCCESS == ret; ++j)
        {
          if( OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)) )
          {
            TBSYS_LOG(WARN, "Deserialize values %.*s[%ld] fail", var_name.length(), var_name.ptr(), j);
          }
          else if(OB_SUCCESS != (ret = proc_->write_variable(var_name, j, obj)))
          {
            TBSYS_LOG(WARN, "write array variables %.*s[%ld] = %s into table fail", var_name.length(), var_name.ptr(), j, to_cstring(obj));
          }
        }
      }
    }
  }

  int64_t static_data_count;
  if( OB_SUCCESS == ret && OB_SUCCESS == (ret = serialization::decode_i64(buf, data_len, pos, &static_data_count)) )
  {
    TBSYS_LOG(TRACE, "block inst, static_data_count: %ld", static_data_count);

    for(int64_t i = 0; i < static_data_count && OB_SUCCESS == ret; ++i)
    {
      int64_t sdata_id = 0, hkey = 0;
      ObRowStore *p_row_store = NULL;
      if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &sdata_id)) )
      {
        TBSYS_LOG(WARN, "failed to deserialize static data id");
      }
      else if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &hkey)))
      {
        TBSYS_LOG(WARN, "failed to deserialize hkey");
      }
      else if( OB_SUCCESS != (ret = proc_->store_static_data(sdata_id, hkey, p_row_store)))
      {
        TBSYS_LOG(WARN, "create row store failed");
      }
      else if( OB_SUCCESS != (ret = p_row_store->deserialize(buf, data_len, pos)) )
      {
        TBSYS_LOG(WARN, "failed to deserialize static data");
      }
      TBSYS_LOG(TRACE, "sid: %ld, hkey: %ld, sdata: %s", sdata_id, hkey, to_cstring(*p_row_store));
    }
  }
  return ret;
}

int SpGroupInsts::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  const SpGroupInsts *old_inst = static_cast<const SpGroupInsts*>(inst);

  inst_list_.clear();
  for(int64_t i = 0; i < old_inst->inst_list_.count(); ++i)
  {
    SpInst *inner_inst = old_inst->inst_list_.at(i);

    SpInst *new_inst = proc_->create_inst(inner_inst->get_type(), NULL);

    if( new_inst != NULL )
    {
      new_inst->assign(inner_inst);
      inst_list_.push_back(new_inst);
    }
  }
  rs_ = old_inst->rs_;
  ws_ = old_inst->ws_;
  group_proc_name_ = old_inst->group_proc_name_;
  return ret;
}

/*================================================
 *				SpMultiInsts Definition
 * ===============================================*/
SpMultiInsts::~SpMultiInsts()
{
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    //release the memory, placement allocated, manually release
    inst_list_.at(i)->~SpInst();
  }
}

int SpMultiInsts::get_inst(int64_t idx, SpInst *&inst) const
{
  if( idx < 0 || idx >= inst_list_.count() ) inst = NULL;
  else inst = inst_list_.at(idx);
  return inst == NULL ? OB_ENTRY_NOT_EXIST : OB_SUCCESS;
}

SpInst* SpMultiInsts::get_inst(int64_t idx)
{
  OB_ASSERT(idx >= 0 && idx < inst_list_.count() );
  return inst_list_.at(idx);
}

int SpMultiInsts::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  int64_t count = inst_list_.count();
  if( OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, count)) )
  {
    TBSYS_LOG(WARN, "serialize inst count fail");
  }

  //serialize instructions
  for(int64_t i = 0; i < count && OB_SUCCESS == ret; ++i)
  {
    SpInst *inst = inst_list_.at(i);
    SpInstType type = inst->get_type();
    ret = serialization::encode_i32(buf, buf_len, pos, type);
    inst->serialize_inst(buf, buf_len,pos);
  }
  return ret;
}

int SpMultiInsts::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  int64_t count = inst_list_.count();
  SpProcedure *proc_ = ownner_->get_ownner();
  if( OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &count)) )
  {
    TBSYS_LOG(WARN, "deserialize inst count fail");
  }
  inst_list_.reserve(count);
  for(int64_t i = 0; i < count && OB_SUCCESS == ret; ++i)
  {
    SpInstType type;
    SpInst *inst = NULL;
    int32_t type_int_value = 0;
    serialization::decode_i32(buf, data_len, pos, &type_int_value);
    type = static_cast<SpInstType>(type_int_value);

    inst = proc_->create_inst(type, this);
    ret = inst->deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory);

    if( OB_SUCCESS != ret )
    {
      TBSYS_LOG(WARN, "Deserialize instructions %ld fail, ret=%d", i, ret);
      break;
    }
  }
  return ret;
}

int SpMultiInsts::assign(const SpMultiInsts &mul_inst)
{
  int ret = OB_SUCCESS;
//  SpProcedure *proc = mul_inst.ownner_->get_ownner(); //wtf, why would I ever use the old inst's proc pointer
  SpProcedure *proc = ownner_->get_ownner();
  for(int64_t i = 0; i < mul_inst.inst_list_.count(); ++i)
  {
    SpInst *inner_inst = mul_inst.inst_list_.at(i);
    SpInst *new_inst = NULL;

    new_inst = proc->create_inst(inner_inst->get_type(), this);

    if( new_inst != NULL )
    {
      new_inst->assign(inner_inst);
    }
  }
  return ret;
}

void SpMultiInsts::get_read_variable_set(SpVariableSet &read_set) const
{
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    inst_list_.at(i)->get_read_variable_set(read_set);
  }
}

void SpMultiInsts::get_write_variable_set(SpVariableSet &write_set) const
{
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    inst_list_.at(i)->get_write_variable_set(write_set);
  }
}

int SpMultiInsts::optimize(SpInstList &exec_list)
{
  int ret = OB_SUCCESS;

  for(int64_t inst_itr = 0; inst_itr < inst_list_.count(); ++inst_itr)
  {
    if( inst_list_.at(inst_itr)->get_type() == SP_B_INST ) //clear the B instruction
    {
      exec_list.push_back(inst_list_.at(inst_itr));
      inst_list_.remove(inst_itr);
      inst_itr --;
    }
    else if( inst_list_.at(inst_itr)->get_type() == SP_C_INST )
    {
      static_cast<SpIfCtrlInsts*>(inst_list_.at(inst_itr))->optimize(exec_list);
    }
  }
  return ret;
}

//add by wdh 20160707 :b
void SpMultiInsts::dfs(bool &flag)
{
    for(int64_t inst_itr = 0; inst_itr < inst_list_.count(); ++inst_itr)
    {
        if(flag) return;
        SpInst *inst = inst_list_.at(inst_itr);
        SpInstType type = inst->get_type();
        if(type == SP_C_INST)
        {
            SpIfCtrlInsts *if_inst = static_cast<SpIfCtrlInsts*>(inst);
            if(if_inst->get_then_block()!=NULL)
            {
                if_inst->get_then_block()->dfs(flag);
            }
            if(if_inst->get_else_block()!=NULL)
            {
                if_inst->get_else_block()->dfs(flag);
            }
        }
        else if(type == SP_CW_INST)
        {
            SpCaseInst *case_inst = static_cast<SpCaseInst*>(inst);
            for(int64_t size=0; size < case_inst->get_when_count(); ++size)
            {
                case_inst->get_when_block(size)->dfs(flag);
            }
            if(case_inst->get_else_block()!=NULL)
            {
                case_inst->get_else_block()->dfs(flag);
            }
        }
        else if(type == SP_EXIT_INST)
        {
            flag=true;
            return;
        }
    }
    return ;
}
bool SpMultiInsts::check_exit()
{
    bool flag=false;
    dfs(flag);
    return flag;
}

//add :e

void SpMultiInsts::set_in_group_exec()
{
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    inst_list_.at(i)->set_in_group_exec();
  }
}

CallType SpMultiInsts::get_call_type() const
{
  CallType ret = L_LPC;
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    ret = merge_call_type(ret, inst_list_.at(i)->get_call_type());
  }
  return ret;
}

int64_t SpMultiInsts::check_tnode_access() const
{
  int64_t count = 0;

  for(int64_t i = 0 ; i < inst_list_.count() && count < 2; ++i)
  {
    switch (inst_list_.at(i)->get_type())
    {
      case SP_A_INST:
      case SP_E_INST:
      case SP_B_INST:
      case SP_PREGROUP_INST:
      case SP_UNKOWN:
      case SP_EXIT_INST:
        break;

      case SP_C_INST:
        count += static_cast<SpIfCtrlInsts*>(inst_list_.at(i))->get_then_block()->check_tnode_access();
        count += static_cast<SpIfCtrlInsts*>(inst_list_.at(i))->get_else_block()->check_tnode_access();
        break;
      case SP_CW_INST:
      {
        SpCaseInst *inst = static_cast<SpCaseInst*>(inst_list_.at(i));
        count += inst->get_else_block()->check_tnode_access();
        for(int64_t j = 0; j < inst->get_when_count(); ++j)
        {
          count += inst->get_when_block(j)->check_tnode_access();
        }
        break;
      }
      case SP_L_INST:
        count += 2 * static_cast<SpLoopInst*>(inst_list_.at(i))->get_body_block()->check_tnode_access();
        break;
      case SP_W_INST:
        count += 2 * static_cast<SpWhileInst*>(inst_list_.at(i))->get_body_block()->check_tnode_access();
        break;

      case SP_D_INST:
      case SP_DE_INST:
      case SP_GROUP_INST:
      case SP_SQL_INST:
        ++count;
        break;
    }
  }
  if( count > 2 ) count = 2;
  return count;
}

void SpMultiInsts::clear()
{
  inst_list_.clear();
}

/*================================================
 * 					SpIfContrlInsts Definition
 * ==============================================*/
SpIfBlock::~SpIfBlock()
{}

SpIfCtrlInsts::~SpIfCtrlInsts()
{}

void SpIfCtrlInsts::set_in_group_exec()
{
  then_branch_.set_in_group_exec();
  else_branch_.set_in_group_exec();
}

void SpIfCtrlInsts::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(expr_rs_set_);
  then_branch_.get_read_variable_set(read_set);
  else_branch_.get_read_variable_set(read_set);
}

void SpIfCtrlInsts::get_write_variable_set(SpVariableSet &write_set) const
{
  then_branch_.get_write_variable_set(write_set);
  else_branch_.get_write_variable_set(write_set);
}

int SpIfCtrlInsts::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = if_expr_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize if_expr fail");
  }
  else if( OB_SUCCESS != (ret = then_branch_.serialize_inst(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "serialize then_branch fail");
  }
  else if( OB_SUCCESS != (ret = else_branch_.serialize_inst(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "serialize else_branch fail");
  }
  return ret;
}

int SpIfCtrlInsts::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                                    ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = if_expr_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize i_expr fail");
  }
  else if( OB_SUCCESS != (ret = then_branch_.deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory)))
  {
    TBSYS_LOG(WARN, "deserialize then_branch fail");
  }
  else if( OB_SUCCESS != (ret = else_branch_.deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory)))
  {
    TBSYS_LOG(WARN, "deserialize else_branch fail");
  }
  else
  {
    if_expr_.set_owner_op(proc_);
  }
  return ret;
}

int SpIfCtrlInsts::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;

  const SpIfCtrlInsts *old_inst = static_cast<const SpIfCtrlInsts*>(inst);
  if_expr_ = old_inst->if_expr_;
  if_expr_.set_owner_op(proc_);
  expr_rs_set_ = old_inst->expr_rs_set_;
//  rs_set_ = old_inst->rs_set_;
//  ws_set_ = old_inst->ws_set_;
  if( OB_SUCCESS != (ret = then_branch_.assign(old_inst->then_branch_)) )
  {
    TBSYS_LOG(WARN, "assign then branch fail");
  }
  else if( OB_SUCCESS != (ret = else_branch_.assign(old_inst->else_branch_)) )
  {
    TBSYS_LOG(WARN, "assign else branch fail");
  }
  return ret;
}

int SpIfBlock::optimize(SpInstList &exec_list)
{
  int ret = OB_SUCCESS;

  for(int64_t inst_itr = 0; inst_itr < inst_list_.count(); ++inst_itr)
  {
    if( inst_list_.at(inst_itr)->get_type() == SP_B_INST )
    {
      exec_list.push_back(inst_list_.at(inst_itr));
      inst_list_.remove(inst_itr);
      inst_itr --;
    }
  }
  return ret;
}

/**
 * @brief SpIfCtrlInsts::optimize
 * 	try to optimize the if execution here
 * @param inst_list
 * @return
 */
int SpIfCtrlInsts::optimize(SpInstList &exec_list)
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = then_branch_.optimize(exec_list)) )
  {
    TBSYS_LOG(WARN, "optimize then_branch fail");
  }
  else if ( OB_SUCCESS != (ret = else_branch_.optimize(exec_list)) )
  {
    TBSYS_LOG(WARN, "optimize else_branch fail");
  }
  else
  {
    //construct read set
  }
  return ret;
}

CallType SpIfCtrlInsts::get_call_type() const
{
  CallType c1 = then_branch_.get_call_type();
  CallType c2 = else_branch_.get_call_type();
  return merge_call_type(c1, c2);
}

//add hjw 20151231:b

/*=================================================
 *                SpWhileInst Defintion
 * ================================================*/
SpWhileInst::~SpWhileInst()
{}

void SpWhileInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(while_expr_var_set_);
  do_body_.get_read_variable_set(read_set);
}

void SpWhileInst::get_write_variable_set(SpVariableSet &write_set) const
{
 do_body_.get_write_variable_set(write_set);
}

int SpWhileInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
    int ret = OB_SUCCESS;
    if(OB_SUCCESS !=(ret = while_expr_.serialize(buf, buf_len,pos)))
    {
        TBSYS_LOG(WARN, "serialize while_expr fail");
    }
    else if(OB_SUCCESS !=(ret = do_body_.serialize_inst(buf, buf_len, pos)))
    {
        TBSYS_LOG(WARN,"serialize do_body fail");
    }
    return ret;
}

int SpWhileInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                                    ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = while_expr_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize while_expr fail");
  }
  else if( OB_SUCCESS != (ret = do_body_.deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory)))
  {
    TBSYS_LOG(WARN, "deserialize do_body fail");
  }
  else
  {
    while_expr_.set_owner_op(proc_);
  }
  return ret;
}

int SpWhileInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;

  const SpWhileInst*old_inst = static_cast<const SpWhileInst*>(inst);
  while_expr_ = old_inst->while_expr_;
  while_expr_.set_owner_op(proc_);
  while_expr_var_set_ = old_inst->while_expr_var_set_;
//  rs_set_ = old_inst->rs_set_;
//  ws_set_ = old_inst->ws_set_;
  if( OB_SUCCESS != (ret = do_body_.assign(old_inst->do_body_)) )
  {
    TBSYS_LOG(WARN, "assign do body fail");
  }
  return ret;
}

CallType SpWhileInst::get_call_type() const
{
//  return do_body_.get_call_type();
  return T_AND_S; //a trick to prevent optimize while inst
}

//add hjw 20151231:e


/*=================================================
 * 					SpLoopInst Defintion
 * ===============================================*/
SpLoopInst::~SpLoopInst()
{
//  loop_counter_var_.clear();
}

void SpLoopInst::get_read_variable_set(SpVariableSet &read_set) const
{
  int64_t idx = read_set.count();
  read_set.add_tmp_var(loop_counter_var_.var_name_);

  //be careful of the adding order, the loop var must be added in the first.
  //and later be removed
  read_set.add_var_info_set(range_var_set_);
  loop_body_.get_read_variable_set(read_set);
  read_set.remove(idx);
}

void SpLoopInst::get_write_variable_set(SpVariableSet &write_set) const
{
  int64_t idx = write_set.count();
  write_set.add_tmp_var(loop_counter_var_.var_name_);
  loop_body_.get_write_variable_set(write_set);
  write_set.remove(idx);
}

int SpLoopInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = lowest_expr_.deserialize(buf, data_len, pos)))
  {
  }
  else if( OB_SUCCESS != (ret = highest_expr_.deserialize(buf, data_len, pos)))
  {
  }
  else if( OB_SUCCESS != (ret = serialization::decode_bool(buf, data_len, pos, &reverse_)))
  {
  }
  else if( OB_SUCCESS != (ret = loop_counter_var_.deserialize(buf, data_len, pos)) )
  {
//    TBSYS_LOG(WARN, "deserialize loop counter variable failed");
  }
  else if( OB_SUCCESS != (ret = loop_body_.deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory)))
  {
    TBSYS_LOG(WARN, "failed to deserialize loop_body");
  }
  else
  {
    lowest_expr_.set_owner_op(proc_);
    highest_expr_.set_owner_op(proc_);
  }
  return ret;
}


/**
 * @brief SpLoopInst::serialize_inst
 * @param buf
 * @param buf_len
 * @param pos
 * @return
 */
int SpLoopInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = lowest_expr_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize lowest expr_ failed");
  }
  else if( OB_SUCCESS != (ret = highest_expr_.serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "serialize highest expr_ failed");
  }
  else if( OB_SUCCESS != (ret = serialization::encode_bool(buf, buf_len, pos, reverse_)))
  {
    TBSYS_LOG(WARN, "serialize direction failed");
  }
  else if( OB_SUCCESS != (ret = loop_counter_var_.serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "serialize loop counter var fail");
  }
  else if( OB_SUCCESS != (ret = loop_body_.serialize_inst(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "serialize loop body template failed");
  }
  return ret;
}


int SpLoopInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  const SpLoopInst *old_inst = static_cast<const SpLoopInst *>(inst);

  loop_counter_var_.assign(old_inst->loop_counter_var_);

  lowest_expr_ = old_inst->lowest_expr_;
  lowest_expr_.set_owner_op(proc_);

  highest_expr_ = old_inst->highest_expr_;
  highest_expr_.set_owner_op(proc_);


  step_size_ = old_inst->step_size_;
  reverse_ = old_inst->reverse_;

  loop_body_.assign(old_inst->loop_body_);
//  loop_local_inst_ = old_inst->loop_local_inst_;

  return ret;
}

int SpLoopInst::assign_template(const SpLoopInst *old_inst)
{
  loop_counter_var_.assign(old_inst->loop_counter_var_);
  lowest_expr_ = old_inst->lowest_expr_;
  lowest_expr_.set_owner_op(proc_);

  highest_expr_ = old_inst->highest_expr_;
  highest_expr_.set_owner_op(proc_);

  range_var_set_.add_var_info_set(old_inst->range_var_set_);

  step_size_ = old_inst->step_size_;
  reverse_ = old_inst->reverse_;
  return OB_SUCCESS;
}

int SpLoopInst::optimize(SpInstList &exec_list)
{
  int ret = OB_SUCCESS;

  //get out the B instruction
  if( OB_SUCCESS != (ret = loop_body_.optimize(exec_list)) )
  {
    TBSYS_LOG(WARN, "optimize loop body fail");
  }
  else
  {}
  return ret;
}

void SpLoopInst::set_in_group_exec()
{
  loop_body_.set_in_group_exec();
}

CallType SpLoopInst::get_call_type() const
{
  return loop_body_.get_call_type();
}

bool SpLoopInst::check_dead_loop(int64_t begin, int64_t end, bool rever)
{
  if( !rever) return begin > end;
  else return begin < end;
}

bool SpLoopInst::is_simple_loop() const
{
  bool ret = true;

  SpVariableSet write_set;
  loop_body_.get_write_variable_set(write_set);

  //first check whether any insturctions modify the counter
  for(int64_t i = 0; ret && i < write_set.count(); ++i)
  {
    const SpVarInfo &var_info = write_set.get_var_info(i);
    if( var_info.var_type_ == VM_TMP_VAR &&
        var_info.var_name_.compare(loop_counter_var_.var_name_) == 0)
    {
      TBSYS_LOG(INFO, "some instructions try to modify the counter[%s]", to_cstring(loop_counter_var_));
      ret = false;
    }
  }

  //second check whether any array variable using a different counter
  if( ret )
  {
    SpVariableSet read_set;
    loop_body_.get_read_variable_set(read_set);
    for(int64_t i = 0; ret && i < read_set.count(); ++i)
    {
      const SpVarInfo &var_info = read_set.get_var_info(i);
      if( var_info.var_type_ == VM_FUL_ARY )
      {
        ObString idx_var;
        var_info.idx_value_.get_varchar(idx_var);
        if(0 !=  idx_var.compare(loop_counter_var_.var_name_))
        {
          TBSYS_LOG(INFO, "some instrucitons use unstable counter[%s]", to_cstring(var_info));
          ret = false;
        }
      }
    }
  }

  //third check whether exit when exists
  //add by wdh 20160626 :b
  //modify by zhutao 20160707 :be
  if( ret )
  {
    ret = !(lowest_expr_.is_empty() || highest_expr_.is_empty());
//    int64_t i=0;
//    for(; i < loop_body_.inst_count(); i++)
//    {
//      SpInst *tmp = NULL;
//      loop_body_.get_inst(i, tmp);
//      SpInstType type=tmp->get_type();
//      if(type == SP_EXIT_INST)
//      {
//        SpExitInst *inst = static_cast<SpExitInst*>(tmp);
//        if(inst->check_when()==false)
//        {
//          TBSYS_LOG(INFO, "exit when exists");
//          ret=false;
//          break;
//        }
//      }
//    }
  }
  //add :e

  return ret;
}

//add by wdh 20160624:b

/*=================================================
 *                SpExitInst Defintion
 * ================================================*/
SpExitInst::~SpExitInst()
{}

void SpExitInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(when_expr_var_set_);
}

void SpExitInst::get_write_variable_set(SpVariableSet &write_set) const
{
  write_set.add_var_info_set(when_expr_var_set_);
}

int SpExitInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
    int ret = OB_SUCCESS;
    if(OB_SUCCESS !=(ret = when_expr_.serialize(buf, buf_len,pos)))
    {
        TBSYS_LOG(WARN, "serialize when_expr fail");
    }
    return ret;
}

bool SpExitInst::check_when() const
{
    return when_expr_.is_empty();
}

int SpExitInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                                    ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
    UNUSED(allocator);
    UNUSED(operators_store);
    UNUSED(op_factory);
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = when_expr_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(WARN, "deserialize while_expr fail");
  }
  else
  {
    when_expr_.set_owner_op(proc_);
  }
  return ret;
}

int SpExitInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;

  const SpExitInst*old_inst = static_cast<const SpExitInst*>(inst);
  when_expr_ = old_inst->when_expr_;
  when_expr_.set_owner_op(proc_);
  when_expr_var_set_ = old_inst->when_expr_var_set_;
  return ret;
}

CallType SpExitInst::get_call_type() const
{
//    return L_LPC;
  return T_AND_S; //a trick to prevent optimize exit inst
}
//add :e


/*=================================================
             SpWhenBlock Defintion
 * ===============================================*/
SpWhenBlock::~SpWhenBlock()
{}

int SpWhenBlock::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  if(OB_SUCCESS != (ret = when_expr_.serialize(buf, buf_len, pos)) )
  {
    TBSYS_LOG(WARN, "failed to serilize when expr!");
  }
  else if( OB_SUCCESS != (ret = SpMultiInsts::serialize_inst(buf, buf_len, pos)))
  {

  }
  return ret;
}

int SpWhenBlock::deserialize_inst(const char *buf, int64_t buf_len, int64_t &pos, ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  if(OB_SUCCESS != (ret = when_expr_.deserialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN,"failed to deserilize when expr!");
  }
  else if( OB_SUCCESS != (ret = SpMultiInsts::deserialize_inst(buf, buf_len, pos, allocator, operators_store, op_factory)))
  {
  }
  else
  {
    when_expr_.set_owner_op(ownner_->get_ownner());
  }
  return ret;
}

int SpWhenBlock::assign(const SpWhenBlock &block)
{
  int ret = OB_SUCCESS;

  when_expr_ = block.when_expr_;
  when_expr_.set_owner_op(ownner_->get_ownner());

  ret = SpMultiInsts::assign(block);
  return ret;
}

/*=================================================
             SpCaseInst Defintion
 * ===============================================*/
SpCaseInst::~SpCaseInst()
{}

void SpCaseInst::get_read_variable_set(SpVariableSet &read_set) const
{
  read_set.add_var_info_set(case_expr_var_set_);

  for(int64_t i = 0; i < when_list_.count(); ++i )
  {
    //correct compile error
    read_set.add_var_info_set(when_list_.at(i).when_expr_var_set_);
    when_list_.at(i).get_read_variable_set(read_set);
  }
  else_branch_.get_read_variable_set(read_set);
}

void SpCaseInst::get_write_variable_set(SpVariableSet &write_set) const
{
  for(int64_t i = 0; i < when_list_.count(); ++i)
  {
    when_list_.at(i).get_write_variable_set(write_set);
  }
  else_branch_.get_write_variable_set(write_set);
}

int SpCaseInst::serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = case_expr_.serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "failed to serilize case_expr!");
  }
  else if(OB_SUCCESS != (ret = else_branch_.serialize_inst(buf, buf_len, pos)))
  {
    TBSYS_LOG(WARN, "failed to serilize else_branch!");
  }
  else
  {
    serialization::encode_i64(buf, buf_len, pos, when_list_.count());
    for(int64_t i = 0; i < when_list_.count(); i++ )
    {
     if(OB_SUCCESS != when_list_.at(i).serialize_inst(buf, buf_len, pos))
     {
       TBSYS_LOG(WARN, "failed to serilize the when %ld", i+1);
     }
    }
  }
  return ret;
}

int SpCaseInst::deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = case_expr_.deserialize(buf, data_len, pos )))
  {
    TBSYS_LOG(WARN, "failed to serilize case_expr!");
  }
  else if(OB_SUCCESS != else_branch_.deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory))
  {
    TBSYS_LOG(WARN, "failed to serilize else_branch!");
  }
  else
  {
    int64_t count = 0;
    //correct compile error 
//    serialization::decode_i64(buf, data_len, pos, count);
    serialization::decode_i64(buf, data_len, pos, &count);

    SpWhenBlock block;
    when_list_.reserve(count);
    for(int64_t i = 0; i < count; i++ )
    {
      when_list_.push_back(block);
     if(OB_SUCCESS != when_list_.at(i).deserialize_inst(buf, data_len, pos, allocator, operators_store, op_factory))
     {
       TBSYS_LOG(WARN, "failed to serilize the statement when %ld", i+1);
     }
    }
    case_expr_.set_owner_op(proc_);
  }
  return ret;
}

int SpCaseInst::assign(const SpInst *inst)
{
  int ret = OB_SUCCESS;
  const SpCaseInst *old_inst = static_cast<const SpCaseInst*>(inst);
  case_expr_ = old_inst->case_expr_;
  SpWhenBlock block(this);
  for(int64_t i = 0; i < old_inst->when_list_.count(); i ++ )
  {
    when_list_.push_back(block);
    when_list_.at(i).assign(old_inst->when_list_.at(i));
  }
  else_branch_.assign(old_inst->else_branch_);
  return ret;
}

void SpCaseInst::set_in_group_exec()
{
  else_branch_.set_in_group_exec();
  for(int64_t i = 0; i < when_list_.count(); ++i)
  {
    when_list_.at(i).set_in_group_exec();
  }
}

CallType SpCaseInst::get_call_type() const
{
  CallType ret = L_LPC;
  for(int64_t i = 0; i < when_list_.count(); ++i)
  {
    ret = merge_call_type(ret, when_list_.at(i).get_call_type());
  }

  //if( ret != L_LPC ) ret = S_RPC; //a trick to prevent from optimizing case
  ret = T_AND_S;
  return ret;
}

int64_t SpInstExecStrategy::sdata_mgr_hash(int64_t sdata_id, const ObLoopCounter &counter)
{
  uint64_t seed = (uint64_t) sdata_id;
  int64_t len = counter.count();

  const uint64_t m = 0xc6a4a7935bd1e995;
  const int r = 47;

  uint64_t h = seed ^ (len * 8 * m);

  for(int64_t i = 0 ; i < counter.count(); ++i)
  {
    uint64_t k = (uint64_t) counter.at(i);

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;
  }


  h ^= h >> r;
  h *= m;
  h ^= h >> r;

  return (int64_t)h;
}

/*=================================================
             SpProcedure Defintion
 * ===============================================*/

SpProcedure::SpProcedure() : static_data_id_gen_(0),
  block_allocator_(SMALL_BLOCK_SIZE, common::OB_MALLOC_BLOCK_SIZE),
  var_name_val_map_allocer_(SMALL_BLOCK_SIZE, ObWrapperAllocator(&block_allocator_)),
  name_pool_()
{
  create_variable_table();
  static_data_mgr_.init();
}

SpProcedure::~SpProcedure()
{
  reset();
}

int SpProcedure::set_proc_name(const ObString &proc_name)
{
  return ob_write_string(arena_, proc_name, proc_name_);
}

int SpProcedure::close()
{
  clear_var_tab();
  static_data_mgr_.clear();
  return OB_SUCCESS;
}

void SpProcedure::reset()
{
  for(int64_t i = 0 ; i < inst_list_.count(); ++i)
  {
    inst_list_.at(i)->~SpInst();
  }
  inst_list_.clear();
  inst_store_.clear();
  clear_var_tab();
  static_data_mgr_.clear();
  arena_.free();
  static_data_id_gen_ = 0;
}

void SpProcedure::clear_var_tab()
{
  for(int64_t i = 0; i < array_table_.count(); ++i)
  {
    array_table_.at(i).array_values_.clear();
  }
  array_table_.clear();

  var_name_val_map_.clear();

  name_pool_.clear();
}

void SpProcedure::clear_variable(const SpVar &var)
{
  if( HASH_EXIST != var_name_val_map_.erase(var.var_name_) )
  {
    TBSYS_LOG(WARN, "%.*s does not exist in the var_map", var.var_name_.length(), var.var_name_.ptr());
  }
}

int SpProcedure::create_variable_table()
{
  return var_name_val_map_.create(hash::cal_next_prime(16), &var_name_val_map_allocer_, &block_allocator_);
}

int SpProcedure::write_variable(const ObString &var_name, const ObObj &val)
{
  int ret = OB_SUCCESS;
  ObString tmp_var = var_name;
  ObObj tmp_val;
  const ObObj *res_cell = &val;

  if (var_name.length() <= 0)
  {
    ret = OB_ERROR;
//    TBSYS_LOG(ERROR, "Empty variable name");
  }

  const ObObj* old = var_name_val_map_.get(var_name);

  if( NULL == old )
  { //new variable name
    ret = name_pool_.write_string(var_name, &tmp_var);
  }
  else if( old->get_type() != val.get_type() )
  { //cast type
    ObString varchar;
    varchar.assign_ptr(casted_buff_, OB_MAX_VARCHAR_LENGTH);
    tmp_val.set_varchar(varchar);
    common::obj_cast(val, *old, tmp_val, res_cell);
    TBSYS_LOG(TRACE, "before: %s; after: %s", to_cstring(val), to_cstring(*res_cell));
  }

  if ( (ret = name_pool_.write_obj(*res_cell, &tmp_val)) != OB_SUCCESS
           || ((ret = var_name_val_map_.set(tmp_var, tmp_val, 1)) != hash::HASH_INSERT_SUCC
               && ret != hash::HASH_OVERWRITE_SUCC) )
  {
    ret = OB_ERROR;
//    TBSYS_LOG(ERROR, "Add variable %.*s error", var_name.length(), var_name.ptr());
  }
  else
  {
    TBSYS_LOG(TRACE, "write variable %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(val));
    ret = OB_SUCCESS;
  }
  return ret;
}

int SpProcedure::write_variable(const SpVar &var, const ObObj &val)
{
  int ret = OB_SUCCESS;
  if( !var.is_array() )
  {
    ret = write_variable(var.var_name_, val);
  }
  else //write array variables
  {
    int64_t idx = 0;
    if( OB_SUCCESS != (ret = read_index_value(var.idx_value_, idx)))
    {
//      TBSYS_LOG(WARN, "read index value failed");
    }
    else if( OB_SUCCESS != (ret = write_variable(var.var_name_, idx, val)))
    {
      TBSYS_LOG(WARN, "write %.*s[%ld] = %s failed", var.var_name_.length(), var.var_name_.ptr(), idx, to_cstring(val));
    }
  }
  return ret;
}

int SpProcedure::write_variable(const ObString &array_name, int64_t idx_value, const ObObj &val)
{  //TODO be careful of the variable type
  int ret = OB_SUCCESS;
  bool find = false;

  //check array existence
  const ObObj *array_idx_obj;
  if( idx_value < 0 )
  {
    ret = OB_ERR_ILLEGAL_INDEX;
  }
  else if( OB_SUCCESS != (ret = read_variable(array_name, array_idx_obj)) )
  {
    //array is not created
  }
  else
  {
    int64_t array_idx = -1;
    find = true;
    array_idx_obj->get_int(array_idx);
    ObProcedureArray &array = array_table_.at(array_idx);

    if( idx_value >= array.array_values_.count() )
    {
      while( OB_SUCCESS == ret && idx_value >= array.array_values_.count() )
      {
        ObObj tmp_obj;
        tmp_obj.set_null();
        ret = array.array_values_.push_back(tmp_obj);
      }
    }
    if( OB_SUCCESS == ret )
    {
      array.array_values_.at(idx_value) = val;
    }
  }

  if ( !find && OB_ERR_VARIABLE_UNKNOWN == ret )
  {
    ObProcedureArray tmp_array;
    ObObj loc;
    array_table_.push_back(tmp_array);
    loc.set_int(array_table_.count() - 1);
    ObProcedureArray &array = array_table_.at(array_table_.count() - 1);
    if( OB_SUCCESS != (ret = write_variable(array_name, loc)))
    {
      //udpate array_name location fail
    }
    else if( idx_value < 0 )
    {
      ret = OB_ERR_ILLEGAL_INDEX;
    }
    else if( idx_value >= array.array_values_.count() )
    {
      while( OB_SUCCESS == ret && idx_value >= array.array_values_.count() )
      {
        ObObj tmp_obj;
        tmp_obj.set_null();
        ret = array.array_values_.push_back(tmp_obj);
      }
    }

    if( OB_SUCCESS == ret )
    {
      array.array_values_.at(idx_value) = val;
    }
  }
  return ret;
}

int SpProcedure::read_variable(const ObString &var_name, const ObObj *&val) const
{
  int ret = OB_SUCCESS;
  if ((val=var_name_val_map_.get(var_name)) == NULL)
  {
//		TBSYS_LOG(WARN, "var does not exist");
    ret = OB_ERR_VARIABLE_UNKNOWN;
  }
  else
  {
    TBSYS_LOG(TRACE, "read var %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(*val));
  }
  return ret;
}

int SpProcedure::read_variable(const ObString &array_name, int64_t idx_value, const ObObj *&val) const
{
  int ret = OB_SUCCESS;

  if( my_phy_plan_->get_result_set() != NULL )
  {
    if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->get_session()->get_variable_value(array_name, idx_value, val)))
    {
      TBSYS_LOG(WARN, "failed to read array value");
    }
  }
  else if( OB_SUCCESS != (ret = read_variable(array_name, val)))
  {
    //table may not exist
  }
  else
  {
    int64_t i = -1;
    val->get_int(i);
    const ObProcedureArray &arr = array_table_.at(i);
    if( idx_value >= 0 && idx_value < arr.array_values_.count() )
    {
      val = & (arr.array_values_.at(idx_value));
    }
    else
    {
      TBSYS_LOG(WARN, "array index is invalid, %ld", idx_value);
      ret = OB_ERR_ILLEGAL_INDEX;
    }
  }
  return ret;
}

int SpProcedure::read_variable(const SpVar &var, const ObObj *&val) const
{
  int ret = OB_SUCCESS;

  if( var.is_array() )
  {
    int64_t idx = 0;
    if( OB_SUCCESS != (ret = read_index_value(var.idx_value_, idx)))
    {
      TBSYS_LOG(WARN, "read index value failed");
    }
    else if( OB_SUCCESS != (ret = read_variable(var.var_name_, idx, val)))
    {
      TBSYS_LOG(WARN, "read %.*s[%ld] failed", var.var_name_.length(), var.var_name_.ptr(), idx);
    }
  }
  else if( OB_SUCCESS != (ret = read_variable(var.var_name_, val)))
  {
    TBSYS_LOG(WARN, "read %.*s failed", var.var_name_.length(), var.var_name_.ptr());
  }
  return ret;
}

int SpProcedure::read_array_size(const ObString &array_name, int64_t &size) const
{
  int ret = OB_SUCCESS;

  const ObObj *idx;

  if( my_phy_plan_->get_result_set() != NULL )
  {
    if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->
                       get_session()->get_variable_array_size(array_name, size)))
    {
      TBSYS_LOG(WARN, "procedure could not read array %.*s size", array_name.length(), array_name.ptr());
    }
  }
  else if( OB_SUCCESS != (ret = read_variable(array_name, idx)))
  {
    TBSYS_LOG(WARN, "fail to read array idx");
  }
  else
  {
    int64_t i = -1;
    idx->get_int(i);
    if( 0 <= i && i < array_table_.count() )
    {
      const ObProcedureArray &arr = array_table_.at(i);
      size = arr.array_values_.count();
    }
    else
    {
      ret = OB_ERR_ILLEGAL_INDEX;
    }
  }
  return ret;
}

int SpProcedure::read_index_value(const ObObj &obj, int64_t &idx_val) const
{
  int ret = OB_SUCCESS;

  if( ObIntType == obj.get_type() )
  {
    obj.get_int(idx_val);
  }
  else if( ObVarcharType == obj.get_type() )
  {
    ObString idx_var_name;
    const ObObj  *idx_obj_val;
    obj.get_varchar(idx_var_name);
    if( OB_SUCCESS != (ret = read_variable(idx_var_name, idx_obj_val)))
    {
      TBSYS_LOG(WARN, "read index variable failed, %.*s", idx_var_name.length(), idx_var_name.ptr());
    }
    else if( ObIntType != idx_obj_val->get_type() )
    {
      TBSYS_LOG(WARN, "index variable has wrong type[%d], %s",
                idx_obj_val->get_type(), to_cstring(*idx_obj_val));
    }
    else
    {
      idx_obj_val->get_int(idx_val);
    }
  }
  else
  {
    TBSYS_LOG(WARN, "index object need to be int or varchar, index obj: %s", to_cstring(obj));
    ret = OB_ERR_ILLEGAL_INDEX;
  }
  return ret;
}

int SpProcedure::store_static_data(int64_t sdata_id, int64_t hkey, ObRowStore *&p_row_store)
{
  return static_data_mgr_.store(sdata_id, hkey, p_row_store);
}

int64_t SpProcedure::get_static_data_count() const
{
  return static_data_mgr_.get_static_data_count();
}

int SpProcedure::get_static_data_by_id(int64_t sdata_id, ObRowStore *&p_row_store)
{
  return static_data_mgr_.get(sdata_id, hkey(sdata_id), p_row_store);
}

int SpProcedure::get_static_data(int64_t idx, int64_t &sdata_id, int64_t &hkey, const ObRowStore *&p_row_store)
{
  return static_data_mgr_.get(idx, sdata_id, hkey, p_row_store);
}

int64_t SpProcedure::hkey(int64_t sdata_id) const
{
  UNUSED(sdata_id);
  return 0;
}

/*
int SpProcedure::store_static_data(int64_t sdata_id,
                                   int64_t hkey,
                                    ObRowStore *&p_row_store)
{
  UNUSED(sdata_id);
  UNUSED(hkey);
  UNUSED(p_row_store);
  return OB_NOT_SUPPORTED;
}

int64_t SpProcedure::get_static_data_count() const
{
  return 0;
}

int SpProcedure::get_static_data(int64_t idx,
                                 int64_t &sdata_id,
                                 int64_t &hkey,
                                 const ObRowStore *&p_row_store)
{
  UNUSED(idx);
  UNUSED(sdata_id);
  UNUSED(hkey);
  UNUSED(p_row_store);
  return OB_NOT_SUPPORTED;
}

int SpProcedure::get_static_data_by_id(int64_t sdata_id, ObRowStore *&p_row_store)
{
  UNUSED(sdata_id);
  UNUSED(p_row_store);
  return OB_NOT_SUPPORTED;
}
*/

SpInst* SpProcedure::create_inst(SpInstType type, SpMultiInsts *mul_inst)
{
  SpInst *new_inst = NULL;
  switch(type)
  {
  case SP_E_INST:
    new_inst = create_inst<SpExprInst>(mul_inst);
    break;
  case SP_B_INST:
    new_inst = create_inst<SpRdBaseInst>(mul_inst);
    break;
  case SP_A_INST:
    new_inst = create_inst<SpRwCompInst>(mul_inst);
    break;
  case SP_C_INST:
    new_inst = create_inst<SpIfCtrlInsts>(mul_inst);
    break;
  case SP_D_INST:
    new_inst = create_inst<SpRwDeltaInst>(mul_inst);
    break;
  case SP_DE_INST:
    new_inst = create_inst<SpRwDeltaIntoVarInst>(mul_inst);
    break;
  case SP_PREGROUP_INST:
    new_inst = create_inst<SpPreGroupInsts>(mul_inst);
    break;
  case SP_GROUP_INST:
    new_inst = create_inst<SpGroupInsts>(mul_inst);
    break;
  case SP_L_INST:
    new_inst = create_inst<SpLoopInst>(mul_inst);
    break;
  case SP_CW_INST:
    new_inst = create_inst<SpCaseInst>(mul_inst);
    break;
  case SP_W_INST:
    new_inst = create_inst<SpWhileInst>(mul_inst);
    break;
  //add by wdh 20160624 :b
  case SP_EXIT_INST:
    new_inst = create_inst<SpExitInst>(mul_inst);
    break;
  //add :e
  case SP_SQL_INST:
    new_inst = create_inst<SpPlainSQLInst>(mul_inst);
    break;
  case SP_UNKOWN:
    new_inst = NULL;
    TBSYS_LOG(WARN, "unknown type here");
    break;
  }
  return new_inst;
}

int SpProcedure::debug_status(const SpInst *inst) const
{
  int ret = OB_SUCCESS;

  if( inst != NULL && inst->get_ownner() == this )
  {
    SpVariableSet rs, ws;
    inst->get_read_variable_set(rs);
    inst->get_write_variable_set(ws);
    char debug_buf[1024]; //TODO try to handle the buffer overflow problem
    int64_t buf_len = 1024, pos = 0;

    databuff_printf(debug_buf, buf_len, pos, "\ninst %ld\n", pc_);
    databuff_printf(debug_buf, buf_len, pos, "\tread set:\n");
    for(int64_t i = 0; i < rs.count(); ++i)
    {
      const SpVarInfo &var_info = rs.get_var_info(i);
      const ObObj *val;
      if( var_info.var_type_ == VM_TMP_VAR )
      {
        //read temp variable
        if( OB_SUCCESS == read_variable(var_info.var_name_, val) )
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] = %s\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr(), to_cstring(*val));
        }
        else
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] not found.\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
        }
      }
      else if( var_info.var_type_ == VM_FUL_ARY )
      {
        //read full array
        int64_t arr_size = 0;
        if( OB_SUCCESS == read_array_size(var_info.var_name_, arr_size) )
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] = [",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
          for(int64_t j = 0; j <  arr_size; ++j)
          {
            read_variable(var_info.var_name_, j, val);
            databuff_printf(debug_buf, buf_len, pos, "%s ", to_cstring(*val));
          }
          pos -= 1; //elimate the last white space
          databuff_printf(debug_buf, buf_len, pos, "]\n");
        }
        else
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] not found.\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
        }
      }
    }
    databuff_printf(debug_buf, buf_len, pos, "\twrite set:\n");
    for(int64_t i = 0; i < ws.count(); ++i)
    {
      const ObObj *val;
      const SpVarInfo &var_info = ws.get_var_info(i);
      if( var_info.var_type_ == VM_TMP_VAR )
      {
        //read temp variable
        if( OB_SUCCESS == read_variable(var_info.var_name_, val) )
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] = %s\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr(), to_cstring(*val));
        }
        else
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] not found.\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
        }
      }
      else if( var_info.var_type_ == VM_FUL_ARY )
      {
        //read full array
        int64_t arr_size = 0;
        if( OB_SUCCESS == read_array_size(var_info.var_name_, arr_size) )
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar  [%.*s] = [",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
          for(int64_t j = 0; j <  arr_size; ++j)
          {
            read_variable(var_info.var_name_, j, val);
            databuff_printf(debug_buf, buf_len, pos, "%s ", to_cstring(*val));
          }
          pos -= 1; //elimate the last white space
          databuff_printf(debug_buf, buf_len, pos, "]\n");
        }
        else
        {
          databuff_printf(debug_buf, buf_len, pos, "\tvar [%.*s] not found.\n",
                          var_info.var_name_.length(), var_info.var_name_.ptr());
        }
      }
    }
    TBSYS_LOG(TRACE, "%s", debug_buf);
  }
  return ret;
}

int SpProcedure::serialize_tree(char *buf, int64_t buf_len, int64_t &pos, const ObPhyOperator &root) const
{
  int ret = OB_SUCCESS;

  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = serialization::encode_vi32(buf, buf_len, pos, root.get_type())))
    {
      TBSYS_LOG(WARN, "fail to encode op type:ret[%d]", ret);
    }
    else if (OB_SUCCESS != (ret = root.serialize(buf, buf_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to serialize root:ret[%d] type=%d op=%s", ret, root.get_type(), to_cstring(root));
    }
    else
    {
      TBSYS_LOG(DEBUG, "serialize operator succ, type=%d", root.get_type());
    }
  }

  for (int64_t i=0;OB_SUCCESS == ret && i<root.get_child_num();i++)
  {
    if (NULL != root.get_child(static_cast<int32_t>(i)) )
    {
      if (OB_SUCCESS != (ret = serialize_tree(buf, buf_len, pos, *(root.get_child(static_cast<int32_t>(i))))))
      {
        TBSYS_LOG(WARN, "fail to serialize tree:ret[%d]", ret);
      }
    }
    else
    {
      ret = OB_ERR_UNEXPECTED;
      TBSYS_LOG(WARN, "this operator should has child:type[%d]", root.get_type());
    }
  }
  return ret;
}

int SpProcedure::deserialize_tree(const char *buf, int64_t data_len, int64_t &pos, ModuleArena &allocator,
                                     ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory, ObPhyOperator *&root)
{
  int ret = OB_SUCCESS;
  int32_t phy_operator_type = 0;
  if (NULL == op_factory)
  {
    ret = OB_NOT_INIT;
    TBSYS_LOG(ERROR, "op_factory == NULL");
  }
  else if (OB_SUCCESS != (ret = serialization::decode_vi32(buf, data_len, pos, &phy_operator_type)))
  {
    TBSYS_LOG(WARN, "fail to decode phy operator type:ret[%d]", ret);
  }

  if (OB_SUCCESS == ret)
  {
    root = op_factory->get_one(static_cast<ObPhyOperatorType>(phy_operator_type), allocator);
    if (NULL == root)
    {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      TBSYS_LOG(WARN, "get operator fail:type[%d]", phy_operator_type);
    }
    else
    {
      root->set_phy_plan(my_phy_plan_); //set the phyplan, used when expr calculation
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = root->deserialize(buf, data_len, pos)))
    {
      TBSYS_LOG(WARN, "fail to deserialize operator:ret[%d]", ret);
    }
    else if (OB_SUCCESS != (ret = operators_store.push_back(root)))
    {
      TBSYS_LOG(WARN, "fail to push operator to operators_store:ret[%d]", ret);
    }
  }

  if (OB_SUCCESS == ret)
  {
    if (root->get_type() <= PHY_INVALID || root->get_type() >= PHY_END)
    {
      ret = OB_ERR_UNEXPECTED;
      TBSYS_LOG(WARN, "invalid operator type:[%d]", root->get_type());
    }
  }

  if (OB_SUCCESS == ret)
  {
    for (int32_t i=0; OB_SUCCESS == ret && i<root->get_child_num(); i++)
    {
      ObPhyOperator *child = NULL;
      if (OB_SUCCESS != (ret = deserialize_tree(buf, data_len, pos, allocator, operators_store, op_factory, child)))
      {
        TBSYS_LOG(WARN, "fail to deserialize tree:ret[%d]", ret);
      }
      else if (OB_SUCCESS != (ret = root->set_child(i, *child)))
      {
        TBSYS_LOG(WARN, "fail to set child:ret[%d]", ret);
      }
    }
  }
  return ret;
}

DEFINE_SERIALIZE(SpProcedure)
{
  //must be only one block inst
  int ret = OB_SUCCESS;
  if( inst_list_.at(0)->get_type() != SP_GROUP_INST )
  {
    TBSYS_LOG(WARN, "unexpected ups procedure execution");
  }
  else
  {
    inst_list_.at(0)->serialize_inst(buf, buf_len, pos);
  }
  TBSYS_LOG(TRACE, "group plan size: %ld", pos);
  return ret;
}

DEFINE_DESERIALIZE(SpProcedure)
{
  //must be only one block inst
  int ret = OB_SUCCESS;
  SpGroupInsts* block_inst = create_inst<SpGroupInsts>(NULL);
  if( OB_SUCCESS != (ret = block_inst->deserialize_inst(
                       buf, data_len, pos, *my_phy_plan_->allocator_,
                       my_phy_plan_->operators_store_,  my_phy_plan_->op_factory_)) )
  {
    TBSYS_LOG(WARN, "deserialize instruction fail");
  }
  else
  {
    my_phy_plan_->set_group_exec(true); //help the ups to determine the execution strategy
  }
  return ret;
}

DEFINE_GET_SERIALIZE_SIZE(SpProcedure)
{
  OB_ASSERT(0);
  TBSYS_LOG(WARN, "do not get here, i do not implement");
  return 0;
}

int64_t SpProcedure::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "proc[seq] %.*s\n", proc_name_.length(), proc_name_.ptr());
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    SpInst *inst = inst_list_.at(i);
    databuff_printf(buf, buf_len, pos, "inst %ld: ", i);
    pos += inst->to_string(buf + pos, buf_len -pos);
  }
  return pos;
}


/*=======================================================================
 * 			to_string methods
 * =====================================================================*/
int64_t SpExprInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set, read_set;
  get_write_variable_set(write_set);
  get_read_variable_set(read_set);
  databuff_printf(buf, buf_len, pos, "type [E], ws: %s, rs: %s\n", to_cstring(write_set), to_cstring(read_set));
  return pos;
}

int64_t SpRdBaseInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set, read_set;
  get_write_variable_set(write_set);
  get_read_variable_set(read_set);
  databuff_printf(buf, buf_len, pos, "type [B], ws: %s, rs: %s, tid[%ld] mod[%s]\n", to_cstring(write_set), to_cstring(read_set), table_id_, for_group_exec_? "Group" : "Normal");
  return pos;
}

int64_t SpRwDeltaInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set, read_set;
  get_write_variable_set(write_set);
  get_read_variable_set(read_set);
  databuff_printf(buf, buf_len, pos, "type [W], ws: %s, rs: %s, tid[%ld] mod[%s]\n", to_cstring(write_set), to_cstring(read_set), table_id_, group_exec_? "Group" : "Normal");
  return pos;
}


int64_t SpRwCompInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set, read_set;
  get_write_variable_set(write_set);
  get_read_variable_set(read_set);
  databuff_printf(buf, buf_len, pos, "type [A], ws: %s, rs: %s, tid[%ld], op[%p]\n", to_cstring(write_set), to_cstring(read_set), table_id_, op_);
  return pos;
}

int64_t SpRwDeltaIntoVarInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set, read_set;
  get_write_variable_set(write_set);
  get_read_variable_set(read_set);
  databuff_printf(buf, buf_len, pos, "type [R], ws: %s, rs: %s, tid[%ld] mod[%s]\n", to_cstring(write_set), to_cstring(read_set), table_id_, group_exec_? "Group" : "Normal");
  return pos;
}

int64_t SpGroupInsts::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [Group] name:%.*s\n", group_proc_name_.length(), group_proc_name_.ptr());
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    SpInst *inst = inst_list_.at(i);
    databuff_printf(buf, buf_len, pos, "\t sub-inst %ld: ", i);

    pos += inst->to_string(buf + pos, buf_len - pos);
  }
  databuff_printf(buf, buf_len, pos, "End Group\n");
  return pos;
}

int64_t SpMultiInsts::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    SpInst *inst = inst_list_.at(i);
    databuff_printf(buf, buf_len, pos, "\t\tsub-inst %ld: ", i);
    pos += inst->to_string(buf + pos, buf_len -pos);
  }
  return pos;
}

int64_t SpIfCtrlInsts::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [If], rs: %s\n", to_cstring(expr_rs_set_));
  databuff_printf(buf, buf_len, pos, "\tThen\n");

  pos += then_branch_.to_string(buf + pos, buf_len - pos);

  databuff_printf(buf, buf_len, pos, "\tElse\n");

  pos += else_branch_.to_string(buf + pos, buf_len - pos);

  databuff_printf(buf, buf_len, pos, "End If\n");
  return pos;
}

int64_t SpLoopInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [for], begin: %s, end: %s, ", to_cstring(lowest_expr_), to_cstring(highest_expr_));
  databuff_printf(buf, buf_len, pos, "rs: %s\n", to_cstring(range_var_set_));
  databuff_printf(buf, buf_len, pos, "\tLoop Body\n");

  pos += loop_body_.to_string(buf + pos, buf_len - pos);
  databuff_printf(buf, buf_len, pos, "\tEnd loop\n");
  return pos;
}

int64_t SpCaseInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [case], rs: %s\n", to_cstring(case_expr_));
  for(int64_t i = 0; i < when_list_.count(); ++i)
  {
   //compile error corrected
    databuff_printf(buf, buf_len, pos, "\twhen %ld, expr: %s, then\n", i, to_cstring(when_list_.at(i).when_expr_));
    pos += when_list_.at(i).to_string(buf + pos, buf_len - pos);
  }
  databuff_printf(buf, buf_len, pos, "\telse\n");
  pos += else_branch_.to_string(buf + pos, buf_len - pos);
  databuff_printf(buf, buf_len, pos, "End Case\n");
  return pos;
}

int64_t SpWhileInst::to_string(char *buf, const int64_t buf_len) const
{
    int64_t pos = 0;
    databuff_printf(buf, buf_len, pos, "type [While], rs: %s\n", to_cstring(while_expr_));
    databuff_printf(buf, buf_len, pos, "\tDo\n");

    pos += do_body_.to_string(buf +pos, buf_len - pos);

    databuff_printf(buf, buf_len, pos, "End While\n");
    return pos;
}


//add by wangdonghui 20160624 :b
int64_t SpExitInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [Exit], rs: %s\n", to_cstring(when_expr_));
  databuff_printf(buf, buf_len, pos, "\tDo\n");
  return pos;
}

int64_t SpPreGroupInsts::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "type [PreGroup]\n");
  pos += inst_list_.to_string(buf + pos, buf_len - pos);
  databuff_printf(buf, buf_len, pos, "End PreGroup\n");
  return pos;
}

int64_t SpPlainSQLInst::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  SpVariableSet write_set;
  get_write_variable_set(write_set);
  databuff_printf(buf, buf_len, pos, "type [SQL], rs: %s, ws: %s\n", to_cstring(rs_), to_cstring(write_set));
  return pos;
}
