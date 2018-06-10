#include "ob_transformer.h"
#include "ob_ups_executor.h"
#include "ob_procedure_compilation_guard.h"

using namespace oceanbase::sql;


void ObProcedureCompilationContext::clear()
{
  rw_delta_inst_ = NULL;
  rd_base_inst_ = NULL;
  rd_all_inst_ = NULL;
  sql_inst_ = NULL;

  key_where_.clear();
  nonkey_where_.clear();
  value_project_.clear();
  access_tids_.clear();

  is_full_key_ = true;
  using_index_ = false;
}

void ObProcedureCompilationContext::fill_variable_info(ObTransformer *trans)
{
  //save variables used in where clause
  if( rw_delta_inst_ != NULL )
  {
    trans->gen_physical_procedure_inst_var_set(rw_delta_inst_->cons_read_var_set(), nonkey_where_);
    trans->gen_physical_procedure_inst_var_set(rw_delta_inst_->cons_read_var_set(), key_where_);
    trans->gen_physical_procedure_inst_var_set(rw_delta_inst_->cons_read_var_set(), value_project_);
    trans->gen_physical_procedure_inst_var_set(rw_delta_inst_->cons_read_var_set(), access_tids_);
    if( access_tids_.count() > 0 )
      rw_delta_inst_->set_tid(access_tids_.at(0));
  }

  if( rd_base_inst_ != NULL )
  {
    trans->gen_physical_procedure_inst_var_set(rd_base_inst_->cons_read_var_set(), key_where_);
    if( access_tids_.count() > 0 )
      rd_base_inst_->set_tid(access_tids_.at(0));
  }

  if( rd_all_inst_ != NULL )
  {
    trans->gen_physical_procedure_inst_var_set(rd_all_inst_->cons_read_var_set(), nonkey_where_);
    trans->gen_physical_procedure_inst_var_set(rd_all_inst_->cons_read_var_set(), key_where_);
    trans->gen_physical_procedure_inst_var_set(rd_all_inst_->cons_read_var_set(), value_project_);
    trans->gen_physical_procedure_inst_var_set(rd_all_inst_->cons_read_var_set(), access_tids_);
    if( access_tids_.count() > 0 )
      rd_all_inst_->set_tid(access_tids_.at(0));
  }

  if( sql_inst_ != NULL )
  {
    trans->gen_physical_procedure_inst_var_set(sql_inst_->cons_read_var_set(), nonkey_where_);
    trans->gen_physical_procedure_inst_var_set(sql_inst_->cons_read_var_set(), key_where_);
    trans->gen_physical_procedure_inst_var_set(sql_inst_->cons_read_var_set(), value_project_);
    trans->gen_physical_procedure_inst_var_set(sql_inst_->cons_read_var_set(), access_tids_);
    if( access_tids_.count() > 0 )
      sql_inst_->set_tid(access_tids_.at(0));
  }
}

void ObProcedureCompilationContext::bind_ups_executor(ObPhyOperator *ups_exec, int32_t idx)
{
  OB_ASSERT(ups_exec->get_type() == PHY_UPS_EXECUTOR);
  ObPhysicalPlan* inner_plan = static_cast<ObUpsExecutor*>(ups_exec)->get_inner_plan();
  OB_ASSERT(inner_plan->get_query_size() == 3);
  for(int32_t i = 0; i < inner_plan->get_query_size(); ++i)
  {
    ObPhyOperator* aux_query = inner_plan->get_phy_query(i);
    const ObPhyOperatorType type = aux_query->get_type();
    if( PHY_VALUES == type )
    {
      rd_base_inst_->set_rdbase_op(aux_query, idx);
      break;
    }
  }
  rw_delta_inst_->set_rwdelta_op(inner_plan->get_main_query());
  rw_delta_inst_->set_ups_exec_op(ups_exec, idx);
  rd_base_inst_->set_rw_id(rw_delta_inst_->get_id());
}

