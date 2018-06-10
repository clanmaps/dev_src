/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure.cpp
 * @brief procedure phsical plan relation class definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_27
 */

#include "ob_procedure.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include "ob_ups_executor.h"
#include "common/ob_common_stat.h"
#include "common/ob_obj_cast.h"
#include "math.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

/*==================================================================
 *							Instruction Execution Strategy
 * ================================================================*/
int SpMsInstExecStrategy::execute_inst(SpInst *inst)
{
  int ret = OB_SUCCESS;
  SpInstType type = inst->get_type();
  switch(type)
  {
  case SP_E_INST:
    ret = execute_expr(static_cast<SpExprInst*>(inst));
    break;
  case SP_B_INST:
    ret = execute_rd_base(static_cast<SpRdBaseInst*>(inst));
    break;
  case SP_D_INST:
    ret = execute_wr_delta(static_cast<SpRwDeltaInst*>(inst));
    break;
  case SP_DE_INST:
    ret = execute_rd_delta(static_cast<SpRwDeltaIntoVarInst*>(inst));
    break;
  case SP_A_INST:
    ret = execute_rw_all(static_cast<SpRwCompInst*>(inst));
    break;
  case SP_PREGROUP_INST:
    ret = execute_pre_group(static_cast<SpPreGroupInsts*>(inst));
    break;
  case SP_GROUP_INST:
    ret = execute_group(static_cast<SpGroupInsts*>(inst));
    break;
  case SP_C_INST:
    ret = execute_if_ctrl(static_cast<SpIfCtrlInsts*>(inst));
    break;
  case SP_L_INST:
    ret = execute_loop(static_cast<SpLoopInst*>(inst));
    break;
  case SP_CW_INST:
    ret = execute_casewhen(static_cast<SpCaseInst*>(inst));
    break;
  case SP_W_INST:
    ret = execute_while(static_cast<SpWhileInst*>(inst));
    break;
  //add by wangdonghui 20160624 :b
  case SP_EXIT_INST:
    ret = execute_exit(static_cast<SpExitInst*>(inst));
    break;
  case SP_SQL_INST:
    ret = execute_plain_sql(static_cast<SpPlainSQLInst*>(inst));
    break;
  //add :e
  default:
    TBSYS_LOG(WARN, "Unsupport execute inst[%d] on mergeserver", type);
    ret = OB_NOT_SUPPORTED;
    break;
  }
  return ret;
}

int64_t SpMsInstExecStrategy::hkey(int64_t sdata_id) const
{
  return SpInstExecStrategy::sdata_mgr_hash(sdata_id, loop_counter_);
}


int SpMsInstExecStrategy::execute_expr(SpExprInst *inst)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(TRACE, "sp expr inst exec()");
  common::ObRow input_row; //fake paramters
  const ObObj *val = NULL;
  if( OB_SUCCESS != (ret = inst->get_val().calc(input_row, val)) )
  {
    TBSYS_LOG(WARN, "sp expr compute failed");
  }
  else if( OB_SUCCESS != (ret = inst->get_ownner()->write_variable(inst->get_var(), *val)))
  {
    TBSYS_LOG(WARN, "write variables fail");
  }
  return ret;
}

int SpMsInstExecStrategy::execute_rd_base(SpRdBaseInst *inst)
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
  const ObRow *row = NULL;
  const ObRowStore::StoredRow *stored_row = NULL;

  ObPhyOperator *op = inst->get_rd_op(); //op_ is in the UpsExecutor::inner_plan, that is different from the procedure->my_phy_plan_
  ObPhysicalPlan *phy_plan = inst->get_ownner()->get_phy_plan();
  //table rpc scan is in the ups_executor's inner plan that is different from the current plan
  op->get_phy_plan()->set_curr_frozen_version(phy_plan->get_curr_frozen_version());
  op->get_phy_plan()->set_result_set(phy_plan->get_result_set());

  op->get_phy_plan()->set_group_exec(false);
  op->get_phy_plan()->set_long_trans_exec(false); //add by qx 20170317
  if( OB_SUCCESS !=  (ret = op->open()) )
  {
    TBSYS_LOG(WARN, "rd_base fail, sp rdbase inst exec(proc_op: %p, phy_plan: %p, result_set: %p)", inst->get_ownner(), phy_plan, phy_plan->get_result_set());
    TBSYS_LOG(WARN, "rd plan: %s", to_cstring(*op));
  }
  else if( inst->is_for_group_exec() )
  {
    /**
      if the static data is used for group execution, we need save which into static_store and close the op by self.
        later, it would be sent to the UPS.
      Otherwise, ObUpsExecutor would consume the static data and close the ObValues op.
    */
    int64_t hkey = sdata_mgr_hash(inst->get_sdata_id(), loop_counter_);

    ObRowStore *p_row_store = NULL;
    if( OB_SUCCESS != (ret = inst->get_ownner()->store_static_data(
                         inst->get_sdata_id(),
                         hkey,
                         p_row_store)) )
    {
      TBSYS_LOG(WARN, "fail to store static data");
    }

    while( OB_SUCCESS == ret )
    {
      ret = op->get_next_row(row);
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
        TBSYS_LOG(DEBUG, "load data from child, row=%s", to_cstring(*row));
        if (OB_SUCCESS != (ret = p_row_store->add_row(*row, stored_row)))
        {
          TBSYS_LOG(WARN, "fail to add row:ret[%d]", ret);
        }
      }
    }

    if( OB_SUCCESS != (err = op->close()))
    {
      TBSYS_LOG(WARN, "fail to close rpc scan:err[%d]", err);
      if( OB_SUCCESS == ret )
      {
        ret = err;
      }
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_wr_delta(SpRwDeltaInst *inst)
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
//  inst->get_ownner()->get_phy_plan()->get_result_set()->get_session()->set_autocommit(true); // for what?
  ObPhyOperator *op = inst->get_ups_exec_op();
  if( OB_SUCCESS != (ret = op->open()) )
  {
    TBSYS_LOG(WARN, "execute rw_delta_inst fail");
  }

  if ( OB_SUCCESS != (err = op->close() ))
  {
    TBSYS_LOG(WARN, "failed to close the rw_delta op");
    if( OB_SUCCESS == ret )
    {
      ret = err;
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_plain_sql(SpPlainSQLInst *inst)
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
  ObPhyOperator *op = inst->get_main_query();
  if( OB_SUCCESS != (ret = op->open()) )
  {
    TBSYS_LOG(WARN, "execute plain sql fail");
  }

  if ( OB_SUCCESS != (err = op->close() ))
  {
    TBSYS_LOG(WARN, "failed to close the plain sql");
    if( OB_SUCCESS == ret )
    {
      ret = err;
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_rw_all(SpRwCompInst *inst)
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
  const ObRow *row;
  ObPhyOperator* op = inst->get_rwcomp_op();
  const ObIArray<SpVar> &var_list = inst->get_var_list();
  if( OB_SUCCESS != (ret = op->open()) )
  {
    TBSYS_LOG(WARN, "open rw_com_inst fail");
  }
  else if( OB_SUCCESS != (ret = op->get_next_row(row))) //properly we need to check only one row is got
  {
    ret = OB_SUCCESS; //if we donot get a row here, do not throw error.
    TBSYS_LOG(WARN, "get new_row fail");
  }
  else if( row->get_column_num() == var_list.count() )
  {
    for(int64_t var_it = 0; OB_SUCCESS == ret && var_it < var_list.count(); ++var_it)
    {
      const SpVar &var = var_list.at(var_it);
      const ObObj *cell = NULL;
      if(OB_SUCCESS !=(ret = row->raw_get_cell(var_it, cell)))
      {
        TBSYS_LOG(WARN, "raw_get_cell %ld failed",var_it);
      }
      else if(OB_SUCCESS !=(ret = inst->get_ownner()->write_variable(var, *cell)))
      {
        TBSYS_LOG(WARN, "write into variables fail");
      }
    }
  }
  else
  { //variable number and column number are not consistent
    ret = OB_ERR_SP_WRONG_NO_OF_ARGS;
  }

  if( OB_SUCCESS == ret && OB_ITER_END != op->get_next_row(row) )
  { //more rows reterived
    ret = OB_ERR_TOO_MANY_ROWS;
  }

  if( OB_SUCCESS != (err = op->close()))
  {
    TBSYS_LOG(WARN, "failed to close the select op");
    if( OB_SUCCESS == ret )
    {
      ret = err;
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_rd_delta(SpRwDeltaIntoVarInst *inst)
{
  int ret = OB_SUCCESS;
  int err = OB_SUCCESS;
  const ObRow *row;
  TBSYS_LOG(TRACE, "sp rwintovar inst exec()");
  ObRowDesc fake_desc;
  fake_desc.reset();
  const ObIArray<SpVar> &var_list = inst->get_var_list();
  ObPhyOperator *op = inst->get_ups_exec_op();
  SpProcedure *proc = inst->get_ownner();

  /**
   * we expect the select list has the same length with the variable list
   * ups use a fake desc to deserialize the result from the ups
  */
  for(int64_t i = 0; ret == OB_SUCCESS && i < var_list.count(); ++i)
  {
    if ((ret = fake_desc.add_column_desc(OB_INVALID_ID, OB_APP_MIN_COLUMN_ID + i)) != OB_SUCCESS)
    {
      TBSYS_LOG(WARN, "Generate row descriptor of RwDeltaIntoVar failed, err=%d", ret);
      break;
    }
  }

  if( (OB_SUCCESS == ret) && OB_SUCCESS != (ret = op->open()) )
  {
    TBSYS_LOG(WARN, "open rw_delta_into_inst fail");
  }
  else if( OB_SUCCESS != (ret = (((ObUpsExecutor *)op)->get_next_row_for_sp(row, fake_desc)))) //properly we need to check only one row is got
  {
    if( OB_ITER_END == ret )
    {
      TBSYS_LOG(WARN, "instruction[%s] read nothing", to_cstring(*inst));
      ret = OB_SUCCESS;
    }
    else
    {
      TBSYS_LOG(WARN, "instruction (%s) read row fail, %d", to_cstring(*inst), ret);
    }
  }
  else
  {
    for(int64_t var_it = 0; var_it < var_list.count(); ++var_it)
    {
      const SpVar &var = var_list.at(var_it);
      const ObObj *cell = NULL;
      if(OB_SUCCESS !=(ret=row->raw_get_cell(var_it, cell)))
      {
        TBSYS_LOG(WARN, "raw_get_cell %ld failed",var_it);
      }
      else if(OB_SUCCESS !=(proc->write_variable(var, *cell)))
      {
        TBSYS_LOG(WARN, "write into variables fail");
      }
    }
  }

  if( OB_SUCCESS == ret && OB_ITER_END !=  ((ObUpsExecutor *)op)->get_next_row_for_sp(row, fake_desc) )
  { //more rows reterived
    ret = OB_ERR_TOO_MANY_ROWS;
  }

  if( OB_SUCCESS != (err = op->close()) )
  {
    TBSYS_LOG(WARN, "failed to close the rw_delta op");
    if( OB_SUCCESS == ret )
    {
      ret = err;
    }
  }
  return ret;
}

int SpMsInstExecStrategy::init_physical_plan(ObPhysicalPlan &exec_plan, ObPhysicalPlan &out_plan)
{
  bool start_new_trans = false;
  ObSQLSessionInfo *session = out_plan.get_result_set()->get_session();

  exec_plan.set_result_set(out_plan.get_result_set()); //need when serialize

  start_new_trans = (!session->get_autocommit() && !session->get_trans_id().is_valid());
//  start_new_trans = false; //a hack for payment test
  exec_plan.set_start_trans(start_new_trans);

  //add by qx 210170317 :b
  exec_plan.set_long_trans_exec(out_plan.is_long_trans_exec());
  if (exec_plan.is_long_trans_exec())
  {
    exec_plan.get_trans_req().type_ = LONG_READ_WRITE_TRANS;
  }
  //add :e

//  common::ObTransReq &start_trans_req = exec_plan.get_trans_req();
  return set_trans_params(session, exec_plan.get_trans_req());
}

int SpMsInstExecStrategy::set_trans_params(ObSQLSessionInfo *session, common::ObTransReq &req)
{
  int ret = OB_SUCCESS;
  // get isolation level etc. from session
  ObObj val;
  ObString isolation_str;
  int64_t tx_timeout_val = 0;
  int64_t tx_idle_timeout = 0;
  if (OB_SUCCESS != (ret = session->get_sys_variable_value(ObString::make_string("tx_isolation"), val)))
  {
    TBSYS_LOG(WARN, "failed to get tx_isolation value, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = val.get_varchar(isolation_str)))
  {
    TBSYS_LOG(WARN, "wrong obj type, err=%d", ret);
    ret = OB_ERR_UNEXPECTED;
  }
  else if (OB_SUCCESS != (ret = req.set_isolation_by_name(isolation_str)))
  {
    TBSYS_LOG(WARN, "failed to set isolation level, err=%d", ret);
    ret = OB_ERR_UNEXPECTED;
  }
  else if (OB_SUCCESS != (ret = session->get_sys_variable_value(ObString::make_string("ob_tx_timeout"), val)))
  {
    TBSYS_LOG(WARN, "failed to get tx_timeout value, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = val.get_int(tx_timeout_val)))
  {
    TBSYS_LOG(WARN, "wrong obj type, err=%d", ret);
    ret = OB_ERR_UNEXPECTED;
  }
  else if (OB_SUCCESS != (ret = session->get_sys_variable_value(ObString::make_string("ob_tx_idle_timeout"), val)))
  {
    TBSYS_LOG(WARN, "failed to get tx_idle_timeout value, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = val.get_int(tx_idle_timeout)))
  {
    TBSYS_LOG(WARN, "wrong obj type, err=%d", ret);
    ret = OB_ERR_UNEXPECTED;
  }
  else
  {
    req.timeout_ = tx_timeout_val;
    req.idle_time_ = tx_idle_timeout;
  }
  return ret;
}

int SpMsInstExecStrategy::execute_if_ctrl(SpIfCtrlInsts *inst)
{
  int ret = OB_SUCCESS;
  common::ObRow &fake_row = curr_row_;
  const ObObj *flag = NULL;

  fake_row.clear();
  if(OB_SUCCESS != (ret = inst->get_if_expr().calc(fake_row, flag)) )
  {
    TBSYS_LOG(WARN, "if expr evalute failed");
  }
  else if( flag->is_true() )
  { //execute the then branch
    inst->set_open_flag(1);
    if( OB_SUCCESS != (ret = execute_multi_inst(inst->get_then_block())) )
    {
      TBSYS_LOG(WARN, "execute then block fail");
    }
  }
  else
  { //execute the fail branch
    inst->set_open_flag(0);
    if( OB_SUCCESS != (ret = execute_multi_inst(inst->get_else_block())) )
    {
      TBSYS_LOG(WARN, "execute else block fail");
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_loop(SpLoopInst *inst)
{
  int ret = OB_SUCCESS;
  const ObObj *lowest_value, *highest_value;
  ObObj itr_var;
  common::ObRow &fake_row = curr_row_;

  SpProcedure *proc = inst->get_ownner();

  fake_row.clear();
  //add by wdh 20160624 :b
  if( inst->get_lowest_expr().is_empty() )
  {
    loop_counter_.push_back(0);
    int64_t &counter = loop_counter_.at(loop_counter_.count() - 1);
    while(true)
    {
      ++counter;
      if( OB_SUCCESS != (ret = execute_multi_inst(inst->get_body_block())) )
      {
        break;
      }
      else if( counter % 100 == 0 && inst->get_ownner()->get_phy_plan()->is_terminate(ret) )
      {
        break;
      }
    }
    loop_counter_.pop_back();
    if(ret == OB_SP_EXIT) ret = OB_SUCCESS;
  }
  //add :e
  else if( OB_SUCCESS != (ret = inst->get_lowest_expr().calc(fake_row, lowest_value)) ||  lowest_value->get_type() != ObIntType )
  {
    TBSYS_LOG(WARN, "unsupported loop range type: %d", lowest_value->get_type());
    ret = OB_NOT_SUPPORTED;
  }
  else
  {
    int64_t itr = 0, itr_begin = -1, itr_end = -1, itr_inc = 1;
    if( OB_SUCCESS!= (ret = inst->get_highest_expr().calc(fake_row, highest_value)) )
    {
      TBSYS_LOG(WARN, "highest value calculate failed");
    }
    else if( OB_SUCCESS != lowest_value->get_int(itr_begin) || OB_SUCCESS != highest_value->get_int(itr_end) )
    {
      TBSYS_LOG(WARN, "unsupported loop range type");
      ret = OB_NOT_SUPPORTED;
    }
    else if( SpLoopInst::check_dead_loop(itr_begin, itr_end, inst->get_reverse()) )
    {
      TBSYS_LOG(WARN, "detect dead loop, [%ld .. %ld, %d]", itr_begin, itr_end, inst->get_reverse() ? -1 : 1);
      ret = OB_ERR_SP_BAD_SQLSTAT; //dead loop detected;
    }
    else
    {
      loop_counter_.push_back(0);
      int64_t &counter = loop_counter_.at(loop_counter_.count() - 1);

      if( inst->get_reverse() )
      {
        itr_inc = -1;
      }
      for(itr = itr_begin; itr != (itr_end + itr_inc) && OB_SUCCESS == ret; itr += itr_inc)//modify wdh <= 20160624
      { //itr_end also need to be iterated, so loop ends when itr == itr_end + itr_inc
        ++counter;
        itr_var.set_int(itr);
        if( OB_SUCCESS != (ret = proc->write_variable(inst->get_loop_var(), itr_var )))
        {
          TBSYS_LOG(WARN, "update iterate variable fail");
        }
        else if( OB_SUCCESS != (ret = execute_multi_inst(inst->get_body_block())) )
        {
          TBSYS_LOG(WARN, "execute loop body fail");
        }
      }

      //try to clear local variables
      inst->get_ownner()->clear_variable(inst->get_loop_var());
      loop_counter_.pop_back();
    }
  }
  return ret;
}


//add hjw 20151230:b
int SpMsInstExecStrategy::execute_while(SpWhileInst *inst)
{
  int ret = OB_SUCCESS;
  common::ObRow fake_row;
  const ObObj *flag =NULL;
  loop_counter_.push_back(0);
  int64_t &counter = loop_counter_.at(loop_counter_.count() - 1);
  while(OB_SUCCESS == (ret = inst->get_while_expr().calc(fake_row, flag)) && flag->is_true())//execute do body
  {
    TBSYS_LOG(TRACE,"while expr value: %s", to_cstring(*flag));
    ++counter;
    inst->get_ownner()->debug_status(inst);
    if(OB_SUCCESS != (ret = execute_multi_inst(inst->get_body_block())))
    {
      TBSYS_LOG(WARN,"execute do body block failed");
      break;
    }

    if( counter % 100 == 0 && inst->get_ownner()->get_phy_plan()->is_terminate(ret))
    {
      break;
    }
  }
  loop_counter_.pop_back();
  return ret;
}
//add hjw 20151230:e

//add by wdh 20160624 :b
int SpMsInstExecStrategy::execute_exit(SpExitInst *inst)
{
    int ret = OB_SUCCESS;
    common::ObRow fake_row;
    const ObObj *flag =NULL;
    if(inst->check_when())
    {
        ret =OB_SP_EXIT;
    }
    else if(OB_SUCCESS == (ret = inst->get_when_expr().calc(fake_row, flag)) && flag->is_true())
    {
        TBSYS_LOG(TRACE,"exit when expr value: %s", to_cstring(*flag));
        ret = OB_SP_EXIT;
    }
    return ret;
}
//add :e
int SpMsInstExecStrategy::execute_casewhen(SpCaseInst *inst)
{
  int ret = OB_SUCCESS;
  common::ObRow &fake_row = curr_row_;
  const ObObj *flag = NULL;
  const ObObj *when_value = NULL;
  bool else_flag = true;
  fake_row.clear();
  TBSYS_LOG(TRACE, "execute casewhen instruction");
  if(OB_SUCCESS != (ret = inst->get_case_expr().calc(fake_row, flag)) )
  {
    TBSYS_LOG(WARN, "fail to execute case expr");
  }
  else
  {
    TBSYS_LOG(TRACE, "case expr value: %s", to_cstring(*flag));
    for( int64_t i=0; i < inst->get_when_count(); i++ )
    {
      SpWhenBlock *when_block = inst->get_when_block(i);
      if(OB_SUCCESS != (ret = when_block->get_when_expr().calc(fake_row, when_value)))
      {
        TBSYS_LOG(WARN, "fail to compute when expr at %ld", i);
      }
      else if( when_value->compare(*flag) == 0 )
      {
        TBSYS_LOG(TRACE, "get into when block %ld", i);
        if( OB_SUCCESS != (ret = execute_multi_inst(when_block)) )
        {
          TBSYS_LOG(WARN, "fail to execute when block[%ld]", i);
        }
        else_flag = false;
        break;
      }
      //add by wdh 20160705 :b for numerical type compare
      else if(when_value->is_numerical()&&flag->is_numerical())
      {
          double val1,val2;
          switch(when_value->get_type())
          {
            case ObIntType: int64_t a;when_value->get_int(a);val1=(double)a;break;
            case ObFloatType: float b;when_value->get_float(b);val1=((double)b);break;
            case ObDoubleType: double c;when_value->get_double(c);val1=((double)c);break;
            default: break;
          }
          switch(flag->get_type())
          {
          case ObIntType: int64_t a;flag->get_int(a);val2=((double)a);break;
          case ObFloatType: float b;flag->get_float(b);val2=((double)b);break;
          case ObDoubleType: double c;flag->get_double(c);val2=((double)c);break;
            default: break;
          }
          int cmp=1;
          TBSYS_LOG(DEBUG, "a: %lf, b:%lf",val1,val2);
          bool double_eq = fabs(val1-val2) < DOUBLE_EPSINON;
          if (double_eq)
          {
            cmp = 0;
          }
          if(cmp==0)
          {
              TBSYS_LOG(TRACE, "get into when block %ld", i);
              if( OB_SUCCESS != (ret = execute_multi_inst(when_block)) )
              {
                TBSYS_LOG(WARN, "fail to execute when block[%ld]", i);
              }
              else_flag = false;
              break;
          }
      }
      //add :e
    }
    if( else_flag )
    {
      if( inst->get_else_block()->inst_count() == 0 )
      {
        TBSYS_LOG(WARN, "case-when does not hit a branch");
        ret = OB_ERR_SP_CASE_NOT_FOUND; //if not case hit, return an error
      }
      else if( OB_SUCCESS != (ret = execute_multi_inst(inst->get_else_block())) )
      {
        TBSYS_LOG(WARN, "fail to execute else block");
      }
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_multi_inst(SpMultiInsts *mul_inst)
{
  int ret = OB_SUCCESS;
  int64_t pc = 0;
  for(; pc < mul_inst->inst_count() && OB_SUCCESS == ret; ++pc)
  {
    SpInst *inst = NULL;
    mul_inst->get_inst(pc, inst);
    if( inst != NULL )
    {
      ret = execute_inst(inst);
    }
    else
    {
      ret = OB_ERR_UNEXPECTED;
      TBSYS_LOG(WARN, "does not fetch inst[%ld]", pc);
    }
  }
  return ret;
}

int SpMsInstExecStrategy::execute_pre_group(SpPreGroupInsts *inst)
{
  int ret = OB_SUCCESS;
  //keep execution context;
  SpProcedure *context = inst->get_ownner();
  const SpVariableSet &write_set = inst->get_write_set();
  ObSEArray<ObObj, 16> value_list;
  const ObObj *read_value = NULL;
  ObObj write_value;
  for(int64_t i = 0; i < write_set.count(); ++i)
  {
    const SpVarInfo &info = write_set.get_var_info(i);

    if( VM_TMP_VAR == info.var_type_ )
    {
      context->read_variable(info.var_name_, read_value);
      ob_write_obj(obj_pool_, *read_value, write_value);
    }
    else
    {
      //TODO handle array variable
    }
    value_list.push_back(write_value);
  }

  ret = execute_multi_inst(inst->get_body());

  //restore execution context;
  for(int64_t i = 0; i < write_set.count(); ++i)
  {
    const SpVarInfo &info = write_set.get_var_info(i);

    if( VM_TMP_VAR == info.var_type_ )
    {
      context->write_variable(info.var_name_, value_list.at(i));
    }
  }
  return ret;
}

/**
 * @brief SpGroupInsts::exec
 * important protocal, a group of instructions would be sent to ups
 * @return
 */
int SpMsInstExecStrategy::execute_group(SpGroupInsts *inst)
{
  int ret = OB_SUCCESS;

  /**
    * proc should be serializable and deserializable
    * local_result_ should contains variables calculated by ups
    */
  ObPhysicalPlan exec_plan;
  SpProcedure proc;
  ObUpsResult result;

  ObPhysicalPlan *out_plan = inst->get_ownner()->get_phy_plan();
  if( NULL == out_plan )
  {
    ret = OB_ERR_UNEXPECTED;
    TBSYS_LOG(WARN, "procedure does not have physical plan");
  }
  else
  {
    /**
     *  set the execution context variables for the exec_plan
     */
    init_physical_plan(exec_plan, *out_plan);

    /**
     * build the relationship between block_inst <----> proc <----> exec_plan
     */
    proc.add_inst(inst);
    exec_plan.add_phy_query(&proc, NULL, true);

    //adjust the serialize methods for ObExprValues/ ObPostfixExpression
    out_plan->set_group_exec(true);

    /******************************************************
    * the procedure rpc call protocal should be modified
    *****************************************************/
    int64_t remain_us = 0;
    if (OB_LIKELY(OB_SUCCESS == ret))
    {
      int64_t begin_time_us = tbsys::CTimeUtil::getTime();
      if (out_plan->is_timeout(&remain_us))
      {
        ret = OB_PROCESS_TIMEOUT;
        TBSYS_LOG(WARN, "ups execute timeout. remain_us[%ld]", remain_us);
      }
      else if (OB_UNLIKELY(NULL != out_plan && out_plan->is_terminate(ret)))
      {
        TBSYS_LOG(WARN, "execution was terminated ret is %d", ret);
      }
      else if (OB_SUCCESS != (ret = static_cast<ObProcedure *>(inst->get_ownner())->get_rpc_stub()->ups_plan_execute(remain_us, exec_plan, result)))
      {
        int64_t elapsed_us = tbsys::CTimeUtil::getTime() - begin_time_us;
        OB_STAT_INC(MERGESERVER, SQL_PROC_UPS_EXECUTE_COUNT);
        OB_STAT_INC(MERGESERVER, SQL_PROC_UPS_EXECUTE_TIME, elapsed_us);
        TBSYS_LOG(WARN, "failed to execute plan on updateserver, err=%d", ret);
        if (OB_TRANS_ROLLBACKED == ret)
        {
          TBSYS_LOG(USER_ERROR, "transaction is rolled back");
          // reset transaction id
          ObTransID invalid_trans;
          out_plan->get_result_set()->get_session()->set_trans_id(invalid_trans);
        }
      }
      else if( OB_SUCCESS != handle_group_result(inst->get_ownner(), result))
      {
        TBSYS_LOG(WARN, "failed to handle the group execution result");
      }
      else
      {
        ret = result.get_error_code();
        int64_t elapsed_us = tbsys::CTimeUtil::getTime() - begin_time_us;
        OB_STAT_INC(MERGESERVER, SQL_PROC_UPS_EXECUTE_COUNT);
        OB_STAT_INC(MERGESERVER, SQL_PROC_UPS_EXECUTE_TIME, elapsed_us);
      }
    }
    //adjust the serialize methods for ObExprValues / ObPostfixExpression
    out_plan->set_group_exec(false);
    proc.clear_inst_list(); //avoid destruction of instruction
  }
//  TBSYS_LOG(INFO, "End execution of SpBlockInst, ret=%d", ret);
  return ret;
}

int SpMsInstExecStrategy::handle_group_result(SpProcedure *proc, ObUpsResult &result)
{
  int ret = OB_SUCCESS;
  const ObObj *cell;
  ObString var;

  fake_row_desc_.add_column_desc(SpProcedure::FAKE_TABLE_ID, SpProcedure::ROW_VAR_CID);
  fake_row_desc_.add_column_desc(SpProcedure::FAKE_TABLE_ID, SpProcedure::ROW_VAL_CID);

  curr_row_.clear();
  curr_row_.set_row_desc(fake_row_desc_);
  while( OB_SUCCESS == ret &&
         OB_SUCCESS == (ret = result.get_scanner().get_next_row(curr_row_)))
  {
    curr_row_.get_cell(SpProcedure::FAKE_TABLE_ID, SpProcedure::ROW_VAR_CID, cell);
    cell->get_varchar(var);

    curr_row_.get_cell(SpProcedure::FAKE_TABLE_ID, SpProcedure::ROW_VAL_CID, cell);

    TBSYS_LOG(TRACE, "row: %s", to_cstring(curr_row_));

    if( OB_SUCCESS != (ret = proc->write_variable(var, *cell)) )
    {
      TBSYS_LOG(WARN, "set group execution result failed");
    }
  }

  ObSQLSessionInfo *session = proc->get_phy_plan()->get_result_set()->get_session();
  if( result.get_trans_id().is_valid() && !session->get_autocommit() && !session->get_trans_id().is_valid() )
  {
    session->set_trans_id(result.get_trans_id());
  }

  if( OB_ITER_END == ret ) ret = OB_SUCCESS;

  return ret;
}

/*=================================================
 *           ObProcedure Definition
===================================================*/
ObProcedure::ObProcedure() : long_trans_(false)
{
//  static_data_mgr_.init();
}

ObProcedure::~ObProcedure()
{}


int ObProcedure::add_param(const ObParamDef &proc_param)
{
  return params_.push_back(proc_param);
}

int ObProcedure::add_var_def(const ObVariableDef &def)
{
  return defs_.push_back(def);
}

const ObParamDef& ObProcedure::get_param(int64_t index) const
{
  return params_.at(index);
}

int64_t ObProcedure::get_param_num() const
{
  return params_.count();
}

void ObProcedure::reset()
{
  params_.clear();
  defs_.clear();
  exec_list_.clear();

//  static_data_mgr_.clear();

  SpProcedure::reset();
}

void ObProcedure::reuse()
{
  reset();
}

int ObProcedure::close()
{
  int ret = OB_SUCCESS;
  my_phy_plan_->set_group_exec(false);
  pc_ = 0;

//  static_data_mgr_.clear();
  ret = SpProcedure::close();
  return ret;
}

int ObProcedure::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  UNUSED(row_desc);
  return OB_SUCCESS;
}

int ObProcedure::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  UNUSED(row);
  return ret;
}

/**
 * create declared variables for the procedure
 * @brief ObProcedure::create_variables
 * @return
 */
int ObProcedure::create_variables()
{
  int ret = OB_SUCCESS;
  if( defs_.count() > 0 )
  {
    ObObj casted_cell;
    const ObObj *res_cell;
    ObString varchar;
    varchar.assign_ptr(casted_buff_, OB_MAX_VARCHAR_LENGTH);
    casted_cell.set_varchar(varchar);

    for (int64_t i = 0; i < defs_.count() && OB_SUCCESS == ret; ++i)
    {
      ObObj new_value_obj;
      ObVariableDef &var = defs_.at(i);
      new_value_obj.set_type(var.variable_type_);
      if( var.is_array_ )
      {
        ret = write_variable(var.variable_name_, 0, new_value_obj);
      }
      else if(var.is_default_)
      {
        common::obj_cast(var.default_value_, new_value_obj, casted_cell, res_cell);
        name_pool_.write_obj(*res_cell, &new_value_obj);
        ret = write_variable(var.variable_name_, new_value_obj);
      }
      else
      {
        //        new_value_obj.set_type(var.variable_type_);
        ret = write_variable(var.variable_name_, new_value_obj);
      }
    }
  }
  return ret;
}

//int ObProcedure::clear_variables()
//{
//  int ret = OB_SUCCESS;

//  SpProcedure::clear_var_tab();
//  if( defs_.count() > 0 )
//  {
//    ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();
//    for (int64_t i = 0; i < defs_.count() && OB_SUCCESS == ret; ++i)
//    {
//      ObVariableDef &var= defs_.at(i);

//      if( !var.is_array_ )
//      {
//        if( OB_SUCCESS != (ret=session->remove_variable(var.variable_name_)) )
//        {
//          TBSYS_LOG(WARN, "remove variable from sql_session[%p] fail", session);
//        }
//        else
//        {
//          TBSYS_LOG(TRACE, "remove %.*s from sql_session[%p]",
//                    var.variable_name_.length(),var.variable_name_.ptr(), session);
//        }
//      }
//      else
//      {
//        if( OB_SUCCESS != (ret = session->remove_vararray(var.variable_name_)))
//        {
//          TBSYS_LOG(WARN, "remove vararray from sql_session[%p] fail", session);
//        }
//        else
//        {
//          TBSYS_LOG(TRACE, "remove %.*s[] from sql_session[%p]",
//                    var.variable_name_.length(),var.variable_name_.ptr(), session);
//        }
//      }
//    }
//  }
//  return ret;
//}

int ObProcedure::fill_parameters(ObIArray<ObSqlExpression> &param_expr)
{
  int ret = OB_SUCCESS;

  if( param_expr.count() != params_.count() )
  {
    ret = OB_ERR_WRONG_PARAMCOUNT_TO_PROCEDURE;
  }
  else
  {
    common::ObRow tmp_row;
    const ObObj *result = NULL;
    ObObj casted_cell, tmp;
    ObObj expected_type;
    ObString varchar;
    varchar.assign_ptr(casted_buff_, OB_MAX_VARCHAR_LENGTH);
    casted_cell.set_varchar(varchar);

    for(int64_t i = 0; OB_SUCCESS == ret && i < params_.count(); ++i)
    {
      const ObParamDef &  param = params_.at(i);
      expected_type.set_type(param.param_type_);
      //modified by wdh 20160712
      if( param.out_type_ == OUT_TYPE || param.out_type_ == INOUT_TYPE )
      {
        bool is_var_type = false;
        //save output into default value
        tmp.set_type(param.param_type_);
        if( OB_SUCCESS != (param_expr.at(i).is_var_expr(is_var_type, params_.at(i).out_var_))
            || !is_var_type )
        {
          TBSYS_LOG(WARN, "out parameter must be a variable, %ld,  %s", i, to_cstring(param_expr.at(i)));
          ret = OB_ERR_SP_NOT_VAR_ARGS;
        }
        else if( OB_SUCCESS != (ret = write_variable(param.param_name_, tmp)) )
        {
          TBSYS_LOG(WARN, "fill output variables");
        }
      }

      if( param.out_type_ ==  IN_TYPE || param.out_type_ == INOUT_TYPE )
      {
        ObSqlExpression &expr = param_expr.at(i);

        if( OB_SUCCESS != (ret = expr.calc(tmp_row, result)) )
        {
          TBSYS_LOG(WARN, "fail to calc input expr");
          ret = OB_ERR_WRONG_PARAMETERS_TO_PROCEDURE;
        }
        else if( result->get_type() != param.param_type_ )
        {
          //try to cast input paramter
          if( OB_SUCCESS != (ret = common::obj_cast(*result, expected_type, casted_cell, result)) )
          {
            TBSYS_LOG(WARN, "fail to cast obj, orig: %s, expected type: %d", to_cstring(*result), param.param_type_);
            ret = OB_ERR_WRONG_PARAMETERS_TO_PROCEDURE;
          }
        }

        if( OB_SUCCESS != ret) {}
        else if( OB_SUCCESS != (ret = write_variable(param.param_name_, *result)) )
        {
          TBSYS_LOG(WARN, "fail to set input expr");
        }
      }
    }
  }
  return ret;
}

int ObProcedure::return_paramters()
{
  int ret = OB_SUCCESS;
//  const ObObj *res_cell;
//  ObObj casted_cell;
//  ObObj type;
//  ObString varchar;
//  varchar.assign_ptr(casted_buff_, OB_MAX_VARCHAR_LENGTH);
//  casted_cell.set_varchar(varchar);
  for(int64_t i = 0; i < params_.count(); ++i)
  {
    const ObParamDef &param = params_.at(i);
    const ObObj *val;
    if( param.out_type_ == OUT_TYPE || param.out_type_ == INOUT_TYPE )
    {
      ObString var_name;
//      type.set_type(param.param_type_);
      if( OB_SUCCESS != (ret = param.out_var_.get_varchar(var_name)) )
      {
        TBSYS_LOG(WARN, "out param does not receive variable name");
        ret = OB_ERR_UNEXPECTED;
      }
      else if( OB_SUCCESS != (ret = read_variable(param.param_name_, val)))
      {
        TBSYS_LOG(WARN, "does not read param value, %s", to_cstring(param.param_name_));
      }
//      else if( OB_SUCCESS != (obj_cast(*val, type, casted_cell, res_cell)) )
//      {
//        TBSYS_LOG(WARN, "cast failed, %s", to_cstring(*val));
//      }
      else if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->
                              get_session()->replace_variable(var_name, *val)) )
      {
        TBSYS_LOG(WARN, "fail to return paramter into session, param[%s], var[%s]",
            to_cstring(param.param_name_),
            to_cstring(var_name));
      }
    }
  }
  return ret;
}

int ObProcedure::open()
{
  int ret = OB_SUCCESS;
  SpMsInstExecStrategy strategy;

  strategy_ = &strategy;
  if( OB_SUCCESS != (ret = create_variables()))
  {
    TBSYS_LOG(WARN, "create varialbes fail");
  }
  else
  {
    pc_ = 0;

    bool autoCommit = my_phy_plan_->get_result_set()->get_session()->get_autocommit();
    my_phy_plan_->get_result_set()->get_session()->set_autocommit(!long_trans_);
    for(; pc_ < exec_list_.count() && OB_SUCCESS == ret; ++pc_)
    {
      ret = strategy.execute_inst(exec_list_.at(pc_));
      if( OB_UNLIKELY(TBSYS_LOGGER._level >= TBSYS_LOG_LEVEL_TRACE))
        debug_status(exec_list_.at(pc_));
      if( OB_SUCCESS != ret )
      {
        TBSYS_LOG(WARN, "execution procedure fail at inst[%ld]:\n%s", pc_, to_cstring(*this));
      }
    }
    if( long_trans_ )
    {
      //add by qx 20160825:b
      //avoid  execution error code was covered
      int temp_ret=ret;
      ret = end_trans(OB_SUCCESS != ret);
      if(ret==OB_SUCCESS)
        ret=temp_ret;
      //add :e
    }
    my_phy_plan_->get_result_set()->get_session()->set_autocommit(autoCommit);
  }
  //add by wdh :b
  if(ret == OB_SUCCESS)
  //add :e
  if( OB_SUCCESS != (ret = return_paramters() ))
  {
    TBSYS_LOG(WARN, "fail to return paramters");
  }

  return ret;
}

int ObProcedure::deter_exec_mode()
{
  for(int64_t i = 0; i < inst_store_.count(); ++i)
  {
    if( inst_store_.at(i)->get_type() == SP_B_INST )
    {
      SpRdBaseInst *rd_base = static_cast<SpRdBaseInst*>(inst_store_.at(i));
      rd_base->set_exec_mode();
    }
  }

  long_trans_ = true;
  SpMultiInsts tmp;
  for(int64_t i = 0; i < exec_list_.count(); ++i)
  {
    tmp.add_inst(exec_list_.at(i));
  }

  long_trans_ = (1 < tmp.check_tnode_access());
  tmp.clear();
  return OB_SUCCESS;
}

/*
int ObProcedure::write_variable(const ObString &var_name, const ObObj &val)
{
  ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();

  return session->replace_variable(var_name, val);
}

int ObProcedure::write_variable(const ObString &array_name, int64_t idx_value, const ObObj &val)
{
  int ret = OB_SUCCESS;
//  bool find = false;

  if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->get_session()->replace_vararray(array_name, idx_value, val)))
  {
    TBSYS_LOG(WARN, "update %.*s[%ld] = %s in sqlsession[%p] fail",
              array_name.length(), array_name.ptr(), idx_value,
              to_cstring(val), my_phy_plan_->get_result_set()->get_session());
  }
  return ret;
}

int ObProcedure::write_variable(const SpVar &var, const ObObj &val)
{
  int ret = OB_SUCCESS;
  if( var.is_array() ) //process array variable
  {
    int64_t idx = 0;
    if(OB_SUCCESS != (ret = read_index_value(var.idx_value_, idx)) )
    {
      TBSYS_LOG(WARN, "read index value failed");
    }
    else if (OB_SUCCESS != (ret = write_variable(var.var_name_, idx, val)))
    {
      TBSYS_LOG(WARN, "write %.*s[%ld] = %s failed", var.var_name_.length(), var.var_name_.ptr(), idx, to_cstring(val));
    }
  } //process ordinary variable
  else if( OB_SUCCESS != (ret = write_variable(var.var_name_, val)) )
  {}
  return ret;
}

int ObProcedure::read_variable(const ObString &var_name, const ObObj *&val) const
{
  ObSQLSessionInfo *session = my_phy_plan_->get_result_set()->get_session();

  val = session->get_variable_value(var_name);

  return val == NULL ? OB_ENTRY_NOT_EXIST : OB_SUCCESS;
}

int ObProcedure::read_variable(const ObString &array_name, int64_t idx_value, const ObObj *&val) const
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->get_session()->get_variable_value(array_name, idx_value, val)) )
  {
    TBSYS_LOG(WARN, "read %.*s[%ld] from sql_session_info[%p] fail",
              array_name.length(), array_name.ptr(), idx_value, my_phy_plan_->get_result_set()->get_session());
  }

//  if ( !find ) ret = OB_ERR_VARIABLE_UNKNOWN;
  return ret;
}

int ObProcedure::read_variable(const SpVar &var, const ObObj *&val) const
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
  else
  {
    ret = read_variable(var.var_name_, val);
  }
  return ret;
}

int ObProcedure::read_array_size(const ObString &array_name, int64_t &size) const
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = my_phy_plan_->get_result_set()->
                     get_session()->get_variable_array_size(array_name, size)))
  {
    TBSYS_LOG(WARN, "procedure could not read array %.*s size", array_name.length(), array_name.ptr());
  }
  return ret;
}
*/

/*
int ObProcedure::store_static_data(int64_t sdata_id, int64_t hkey, ObRowStore *&p_row_store)
{
  return static_data_mgr_.store(sdata_id, hkey, p_row_store);
}

int64_t ObProcedure::get_static_data_count() const
{
  return static_data_mgr_.get_static_data_count();
}

int ObProcedure::get_static_data_by_id(int64_t sdata_id, ObRowStore *&p_row_store)
{
  return static_data_mgr_.get(sdata_id, strategy_.hkey(sdata_id), p_row_store);
}

int ObProcedure::get_static_data(int64_t idx, int64_t &sdata_id, int64_t &hkey, const ObRowStore *&p_row_store)
{
  return static_data_mgr_.get(idx, sdata_id, hkey, p_row_store);
}
*/

int64_t ObProcedure::hkey(int64_t sdata_id) const
{
  return strategy_->hkey(sdata_id);
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObProcedure, PHY_PROCEDURE);
  }
}

int ObProcedure::set_inst_op()
{
  int ret = OB_SUCCESS;
  for(int64_t i = 0; i < exec_list_.count(); ++i)
  {
    if( OB_SUCCESS != (ret = set_inst_op(exec_list_.at(i))))
    {
      TBSYS_LOG(WARN, "set inst op fail at idx[%ld]", i);
      break;
    }
  }
  return ret;
}


int ObProcedure::set_inst_op(SpInst *inst)
{
  int ret = OB_SUCCESS;
  switch(inst->get_type())
  {
  case SP_B_INST:
    {
      SpRdBaseInst *rd_inst = static_cast<SpRdBaseInst*>(inst);
      int32_t idx = rd_inst->get_query_id();
      OB_ASSERT(my_phy_plan_->get_phy_query(idx)->get_type() == PHY_UPS_EXECUTOR);
      ObUpsExecutor *ups_exec = (ObUpsExecutor *)my_phy_plan_->get_phy_query(idx);

      ObPhysicalPlan* inner_plan = ups_exec->get_inner_plan();
      OB_ASSERT(inner_plan->get_query_size() == 3);
      for(int32_t i = 0; i < inner_plan->get_query_size(); ++i)
      {
        ObPhyOperator* aux_query = inner_plan->get_phy_query(i);
        const ObPhyOperatorType type = aux_query->get_type();
        if( PHY_VALUES == type )
        {
          rd_inst->set_rdbase_op(aux_query, idx);
          break;
        }
      }
      break;
    }
  case SP_A_INST:
    {
      SpRwCompInst *rw_comp_inst = static_cast<SpRwCompInst*>(inst);
      int32_t idx = rw_comp_inst->get_query_id();
      rw_comp_inst->set_rwcomp_op(my_phy_plan_->get_phy_query(idx), idx);
      break;
    }
  case SP_D_INST:
  case SP_DE_INST:
    {
      SpRwDeltaInst *rw_delta_inst = static_cast<SpRwDeltaInst*>(inst);
      int32_t idx = rw_delta_inst->get_query_id();

      OB_ASSERT(my_phy_plan_->get_phy_query(idx)->get_type() == PHY_UPS_EXECUTOR);
      ObUpsExecutor *ups_exec = (ObUpsExecutor *)my_phy_plan_->get_phy_query(idx);

      rw_delta_inst->set_rwdelta_op(ups_exec->get_inner_plan()->get_main_query());
      rw_delta_inst->set_ups_exec_op(ups_exec, idx);
      break;
    }
   case SP_C_INST:
    {
      SpIfCtrlInsts *if_ctrl_inst = static_cast<SpIfCtrlInsts*>(inst);
      SpMultiInsts *mul_inst = if_ctrl_inst->get_then_block();
      for(int64_t i = 0; i < mul_inst->inst_count(); ++i)
      {
        set_inst_op(mul_inst->get_inst(i));
      }
      mul_inst = if_ctrl_inst->get_else_block();
      for(int64_t i = 0; i < mul_inst->inst_count(); ++i)
      {
        set_inst_op(mul_inst->get_inst(i));
      }
      break;
    }
  case SP_W_INST:
  {
    SpWhileInst *while_inst =static_cast<SpWhileInst*>(inst);
    SpMultiInsts *mul_inst = while_inst->get_body_block();
    for(int64_t i = 0; i < mul_inst->inst_count(); ++i)
    {
      set_inst_op(mul_inst->get_inst(i));
    }
    break;
  }
  case SP_GROUP_INST:
    {
      SpGroupInsts *block_inst = static_cast<SpGroupInsts*>(inst);
      ObIArray<SpInst *> &inst_list = block_inst->get_inst_list();
      for(int64_t i = 0; i < inst_list.count(); ++i)
      {
         set_inst_op(inst_list.at(i));
      }
      break;
    }
  case SP_L_INST:
    {
      SpMultiInsts *loop_body = static_cast<SpLoopInst*>(inst)->get_body_block();
      for(int64_t i = 0; i < loop_body->inst_count(); ++i)
      {
        set_inst_op(loop_body->get_inst(i));
      }
      break;
    }
  case SP_CW_INST:
  {
    SpCaseInst *case_inst = static_cast<SpCaseInst *>(inst);
    for(int64_t i = 0; i < case_inst->get_when_count(); ++i)
    {
      SpMultiInsts *when_block = case_inst->get_when_block(i);
      for(int64_t i = 0 ;i < when_block->inst_count(); ++i)
      {
        set_inst_op(when_block->get_inst(i));
      }
    }
    SpMultiInsts *else_block = case_inst->get_else_block();
    for(int64_t i = 0; i < else_block->inst_count(); ++i)
    {
      set_inst_op(else_block->get_inst(i));
    }
    break;
  }
  default:
    break;
  }
  return ret;
}


/**
 * @brief ObProcedure::assign
 *  reconstruct according to the execution plan
 * @param other
 * @return
 */
int ObProcedure::assign(const ObPhyOperator* other)
{
  int ret = OB_SUCCESS;
  const ObProcedure *old_proc = static_cast<const ObProcedure *>(other);

  reset();
  proc_name_ = old_proc->proc_name_;

  for(int64_t i = 0; i < old_proc->params_.count(); ++i)
  {
    const ObParamDef & param = old_proc->params_.at(i);
    params_.push_back(param); //should alloc memory for name ?
  }
  for(int64_t i = 0; i < old_proc->defs_.count(); ++i)
  {
    const ObVariableDef & def = old_proc->defs_.at(i);
    defs_.push_back(def);
  }

  for(int64_t i = 0; (OB_SUCCESS == ret) && i < old_proc->exec_list_.count(); ++i)
  {
    SpInst *inst = old_proc->exec_list_.at(i);
    SpInst *new_inst = NULL;

    new_inst = create_inst(inst->get_type(), NULL);

    if( new_inst != NULL ) 
		{
			ret = new_inst->assign(inst);
			exec_list_.push_back(new_inst);
		}
  }
  return ret;
}

int64_t ObProcedure::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "proc[ac=%d] %.*s\n", !long_trans_, proc_name_.length(), proc_name_.ptr());
  for(int64_t i = 0; i < exec_list_.count(); ++i)
  {
    SpInst *inst = exec_list_.at(i);
    databuff_printf(buf, buf_len, pos, "inst %ld: ", i);
    pos += inst->to_string(buf + pos, buf_len -pos);
  }
  return pos;
}

int ObProcedure::end_trans(bool rollback)
{
  int ret = OB_SUCCESS;
  ObEndTransReq req;
  req.rollback_ = rollback;
  req.trans_id_ = my_phy_plan_->get_result_set()->get_session()->get_trans_id(); // get trans id at runtime to support prepare commit/rollback
  if (!req.trans_id_.is_valid())
  {
    TBSYS_LOG(WARN, "not in transaction");
  }
  else if (OB_SUCCESS != (ret = rpc_->ups_end_trans(req)))
  {
    TBSYS_LOG(WARN, "failed to end ups transaction, err=%d trans=%s",
              ret, to_cstring(req));
    if (OB_TRANS_ROLLBACKED == ret)
    {
      TBSYS_LOG(USER_ERROR, "transaction is rolled back");
    }
    // reset transaction id
    ObTransID invalid_trans;
    my_phy_plan_->get_result_set()->get_session()->set_trans_id(invalid_trans);
  }
  else
  {
    // reset transaction id
    ObTransID invalid_trans;
    my_phy_plan_->get_result_set()->get_session()->set_trans_id(invalid_trans);
  }
  if (!req.rollback_)
  {
    OB_STAT_INC(OBMYSQL, SQL_COMMIT_COUNT);
  }
  else
  {
    OB_STAT_INC(OBMYSQL, SQL_ROLLBACK_COUNT);
  }
  FILL_TRACE_LOG("trans_id=%s err=%d", to_cstring(req.trans_id_), ret);
  return ret;
}

int ObProcedure::check_semantics() const
{
  int ret = OB_SUCCESS;
  for(int64_t i = 0; OB_SUCCESS == ret && i < inst_list_.count(); ++i)
  {
    SpVariableSet var_set;
    inst_list_.at(i)->get_read_variable_set(var_set);
    inst_list_.at(i)->get_write_variable_set(var_set);

    for(int64_t j = 0; OB_SUCCESS == ret && j < var_set.count(); ++j)
    {
      if( !is_defined(var_set.get_var_info(j)) )
      {
        TBSYS_LOG(WARN, "variable %s used in %s does not define",
                  to_cstring(var_set.get_var_info(j)),
                  to_cstring(*(inst_list_.at(i))));
        ret = OB_ERR_SP_UNDECLARED_VAR;
      }
    }
  }
  return ret;
}

bool ObProcedure::is_defined(const SpVarInfo &info) const
{
  bool ret = false;

  if( info.var_type_ == VM_DB_TAB ) return true;
  if( info.var_type_ == VM_FUL_ARY ) return true;

  for(int64_t i = 0; !ret && i < params_.count(); ++i)
  {
    if( info.var_name_.compare(params_.at(i).param_name_) == 0 ) ret = true;
  }

  for(int64_t i = 0; !ret && i < defs_.count(); ++i)
  {
    if( info.var_name_.compare(defs_.at(i).variable_name_) == 0 ) ret = true;
  }
  return ret;
}
