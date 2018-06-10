/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_optimizer.cpp
 * @brief procedure optimizer class definition,used to adjust instactions execution sequence
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_27
 */
#include "ob_procedure_optimizer.h"
#include "ob_procedure.h"
using namespace oceanbase::sql;


#define CHECK_PROC_NAME(S) ( 0 == proc_name.compare( #S ) )

#define ADD_INST(A, B) ((A).push_back(inst_list.at(B)))
#define ADD_INST_WITH_SOUR(A, B, C) ((A).push_back((B).at(C)))
#define PREP_PROC(A) \
  SpInstList &exec_list = (A).exec_list_;  \
  ObIArray<SpInst *> &inst_list = (A).inst_list_;  \
  const ObString &proc_name = proc.get_proc_name();

#define ADD_MULTI_INST(A, B...) \
  do {   \
  const int array[] = { B }; \
  for(uint32_t i = 0; i < sizeof(array)/sizeof(int); ++i) \
  ADD_INST(A, array[i]); \
  }while(0);

#define ADD_MULTI_INST_WITH_SOUR(A, B, C...) \
  do {   \
  const int array[] = { C }; \
  for(uint32_t i = 0; i < sizeof(array)/sizeof(int); ++i) \
  ADD_INST_WITH_SOUR(A, B, array[i]); \
  }while(0);

#define CHECK_TYPE(A, B) \
    A->get_type() == B \

ObProcedureOptimizer::ObProcedureOptimizer()
{

}

int ObProcedureOptimizer::optimize(ObProcedure &proc, bool no_group)
{
  int ret = OB_SUCCESS;

//  if( OB_SUCCESS == (ret = specialize_optimize(proc)) )
//  {
//    TBSYS_LOG(INFO, "[%.*s] use special optimization", proc.get_proc_name().length(), proc.get_proc_name().ptr());
//  }
//  else
  if( no_group )
  {
    if( OB_SUCCESS == (ret = no_optimize(proc)))
    {
      TBSYS_LOG(INFO, "[%.*s] use no optimization", proc.get_proc_name().length(), proc.get_proc_name().ptr());
    }
  }
  else if( OB_SUCCESS == (ret = rule_based_optimize(proc)) )
  {
    TBSYS_LOG(INFO, "[%.*s] use general optimization", proc.get_proc_name().length(), proc.get_proc_name().ptr());
  }

  proc.deter_exec_mode();

  return OB_SUCCESS;
}

int ObProcedureOptimizer::rule_based_optimize(ObProcedure &proc)
{
  PREP_PROC(proc);
  UNUSED(proc_name);
  UNUSED(exec_list);
  //TBSYS_LOG(INFO, "rule_base_optimize: %.*s", proc_name.length(), proc_name.ptr());

  SpInstList expand_list;
  SpInstList seq_list;

  expand_list.clear();

  for(int64_t i = 0; i < inst_list.count(); ++i)
  {
    seq_list.push_back(inst_list.at(i));
  }

  for(int64_t i = 0; i < seq_list.count(); ++i)
  {
    if( seq_list.at(i)->get_type() == SP_C_INST)
    {
      ctrl_split(static_cast<SpIfCtrlInsts*>(seq_list.at(i)), expand_list);
    }
    else if( seq_list.at(i)->get_type() == SP_L_INST &&
             static_cast<SpLoopInst*>(seq_list.at(i))->is_simple_loop() )
    {
      loop_split(static_cast<SpLoopInst*>(seq_list.at(i)), expand_list);
    }
    else
    {
      expand_list.push_back(seq_list.at(i));
    }
  }

  group(proc, expand_list);
  return OB_SUCCESS;
}

int ObProcedureOptimizer::no_optimize(ObProcedure &proc)
{
  PREP_PROC(proc);
  UNUSED(proc_name);

  for(int64_t i = 0; i < inst_list.count(); ++i)
  {
    exec_list.push_back(inst_list.at(i));
  }
  return OB_SUCCESS;
}


int ObProcedureOptimizer::group(ObProcedure &proc, SpInstList &expand_list)
{
  int64_t group_range_start, group_range_end;
  ObProcDepGraph graph;
  ObSEArray<int64_t, ObProcDepGraph::MAX_GRAPH_SIZE> seq;
  SpInstList &exec_list = proc.exec_list_;
  graph.set_insts(expand_list);
  graph.reorder_for_group(seq);

  for(int64_t start = seq.count() - 1; start >= 0; --start)
  {
    if( expand_list.at(seq.at(start))->is_trpc() )
    {
      group_range_start = start;
      group_range_end = start;
      for(int64_t cur = start - 1; cur >= 0; --cur)
      {
        if( S_RPC == (S_RPC & expand_list.at(seq.at(cur))->get_call_type()) )
        {
          break;
        }
        else if( expand_list.at(seq.at(cur))->is_trpc() )
        {
          group_range_end = cur;
        }
      }
      //try to group operations [start_id, end_id]
      if( group_range_start != group_range_end ||
          (expand_list.at(seq.at(start))->get_type() == SP_L_INST))
      {
        SpGroupInsts *group = proc.create_inst<SpGroupInsts>(NULL);
        for(int64_t i = group_range_start; i >= group_range_end; --i)
        {
          ADD_INST_WITH_SOUR(*group, expand_list, seq.at(i));
        }
        exec_list.push_back(group);
        start = group_range_end;
      }
      else
      {
        ADD_INST_WITH_SOUR(exec_list, expand_list, seq.at(start));
      }
    }
    else
    {
      ADD_INST_WITH_SOUR(exec_list, expand_list, seq.at(start));
    }
  }
  return OB_SUCCESS;
}

int ObProcedureOptimizer::loop_split(SpLoopInst *loop_inst, SpInstList &inst_list)
{
  int ret = OB_SUCCESS;

  SpInstList pre_inst, post_inst;

  pre_inst.clear();
  post_inst.clear();

  do_loop_split(loop_inst->get_body_block()->inst_list_, pre_inst, post_inst);

  if( pre_inst.count() != 0 )
  {
    SpPreGroupInsts *pre_loop =loop_inst->get_ownner()->create_inst<SpPreGroupInsts>(NULL);
    SpLoopInst *new_loop = loop_inst->get_ownner()->create_inst<SpLoopInst>(NULL);

    new_loop->assign_template(loop_inst);

    for(int64_t i = 0; i < pre_inst.count(); ++i)
    {
      new_loop->get_body_block()->add_inst(pre_inst.at(i));
    }
    pre_loop->add_inst(new_loop);
    inst_list.push_back(pre_loop);
  }

  loop_inst->get_body_block()->inst_list_.clear();
  for(int64_t i = 0; i < post_inst.count(); ++i)
  {
    loop_inst->get_body_block()->add_inst(post_inst.at(i));
  }

  inst_list.push_back(loop_inst);
  return ret;
}

/**
 * @brief ObProcedureOptimizer::ctrl_split
 * @param inst
 * @param inst_list
 *  upper level inst_list?
 * @return
 */
int ObProcedureOptimizer::ctrl_split(SpIfCtrlInsts *if_inst, SpInstList &inst_list)
{
  int ret = OB_SUCCESS;


  SpInstList then_post_inst, then_pre_inst;
  SpInstList else_post_inst, else_pre_inst;

  if( if_inst->get_then_block()->inst_count() != 0 )
  {
    do_split(if_inst->get_then_block()->inst_list_, then_pre_inst, then_post_inst);
  }
  if( if_inst->get_else_block()->inst_count() != 0 )
  {
    do_split(if_inst->get_else_block()->inst_list_, else_pre_inst, else_post_inst);
  }

  if( (if_inst->get_then_block()->inst_count() == 0 || then_pre_inst.count() != 0) &&
      (if_inst->get_else_block()->inst_count() == 0 || else_pre_inst.count() != 0) )
  {
    SpPreGroupInsts *pre_then_group = if_inst->get_ownner()->create_inst<SpPreGroupInsts>(NULL);
    SpPreGroupInsts *pre_else_group = if_inst->get_ownner()->create_inst<SpPreGroupInsts>(NULL);

    for(int64_t i = 0; i < then_pre_inst.count(); ++i)
    {
      pre_then_group->add_inst(then_pre_inst.at(i));
    }

    for(int64_t i = 0; i < else_pre_inst.count(); ++i)
    {
      pre_else_group->add_inst(else_pre_inst.at(i));
    }

    if_inst->get_then_block()->inst_list_.clear();
    for(int64_t i = 0; i < then_post_inst.count(); ++i)
    {
      if_inst->get_then_block()->inst_list_.push_back(then_post_inst.at(i));
    }

    if_inst->get_else_block()->inst_list_.clear();
    for(int64_t i = 0; i < else_post_inst.count(); ++i)
    {
      if_inst->get_else_block()->inst_list_.push_back(else_post_inst.at(i));
    }
    inst_list.push_back(pre_then_group);
    inst_list.push_back(pre_else_group);

  }
  inst_list.push_back(if_inst);
  return ret;
}

int ObProcedureOptimizer::do_split(SpInstList &inst_list, SpInstList &pre_inst, SpInstList &post_inst)
{
  int ret = OB_SUCCESS;
  ObSEArray<int64_t, ObProcDepGraph::MAX_GRAPH_SIZE> seq;
  int64_t pre_count = 0;
  ObProcDepGraph graph;
  graph.set_insts(inst_list);
  graph.split(seq, pre_count);
  //pre-execute all s-rpc
  /**
   * Maybe we need use a speical instructions to wrap
   */
  if( pre_count == inst_list.count() )
  {
    pre_count = 0;
  }
  for(int64_t i = 0; i < pre_count; ++i)
  {
    pre_inst.push_back(inst_list.at(seq.at(i)));
  }
  for(int64_t i = pre_count; i < seq.count(); ++i)
  {
    post_inst.push_back(inst_list.at(seq.at(i)));
  }
  return ret;
}

int ObProcedureOptimizer::do_loop_split(SpInstList &inst_list, SpInstList &pre_inst, SpInstList &post_inst)
{
  int ret = OB_SUCCESS;
  ObSEArray<int64_t, ObProcDepGraph::MAX_GRAPH_SIZE> seq;
  int64_t pre_count = 0;
  ObProcDepGraph graph;

  SpInstList expand_list;

  for(int64_t i = 0; i < inst_list.count(); ++i)
  {
    expand_list.push_back(inst_list.at(i));
  }

  for(int64_t i = 0; i < inst_list.count(); ++i)
  {
    expand_list.push_back(inst_list.at(i));
  }

  graph.set_insts(expand_list);
  graph.split(seq, pre_count);

  if( pre_count == 2 * inst_list.count() )
  {
    pre_count = 0;
  }

  for(int64_t i = 0; i < pre_count; ++i)
  {
    if( seq.at(i) < inst_list.count() )
    {
      pre_inst.push_back(inst_list.at(seq.at(i)));
    }
  }

  for(int64_t i = pre_count; i < seq.count(); ++i)
  {
    if( seq.at(i) < inst_list.count() )
    {
      post_inst.push_back(inst_list.at(seq.at(i)));
    }
  }
  return ret;
}

int ObProcDepGraph::check_dep(SpInst *in_node, SpInst *out_node)
{
  //check dependence between instructions
  return SpInst::check_dep(*in_node, *out_node);
}

void ObProcDepGraph::build_dep_graph(GraphType type)
{
  int dep_type = 0;
  type_ = type;
  for(int64_t i = 0; i < inst_list_.count(); ++i)
  {
    SpInst *dep_node = inst_list_.at(i);
    for(int64_t j = 0; j < i; ++j)
    {
      SpInst *pre_node = inst_list_.at(j);
      if( 0 != (dep_type = check_dep(pre_node, dep_node)) ) //check whether dep_node depends on the output of pre_node
      {
        add_edge(j, i, dep_type);
      }
    }
  }
  //TBSYS_LOG(INFO, "%s", to_cstring(*this));
  inited_ = true;
}

//build edge i ---> j  forward
//build edge i <--- j  backward
void ObProcDepGraph::add_edge(int64_t i, int64_t j, int dep_type)
{
  if( type_ == Backward )
  {
    int64_t tmp = i;
    i = j;
    j = tmp;
  }
  SpNode &node = graph_.at(i);
  SpNode *new_node = (SpNode*)arena_.alloc(sizeof(SpNode));
  new_node->id_ = j;
  new_node->dep_type_ = dep_type;
  new_node->next_ = node.next_;
  node.next_ = new_node;
  degree_.at(j)++;

  if( dep_type == Da_True_Dep )
  {
    cover_trpc_.at(i) = cover_trpc_.at(j) || cover_trpc_.at(i);
  }
}

int ObProcDepGraph::set_insts(ObIArray<SpInst *> &insts)
{
  graph_.clear();
  degree_.clear();
  cover_trpc_.clear();
  flow_srpc_.clear();
  inst_list_.clear();
  arena_.reuse();
  inited_ = false;
  for(int64_t i = 0; i < insts.count(); ++i)
  {
    inst_list_.push_back(insts.at(i));
    graph_.push_back(SpNode(i, NULL));
    degree_.push_back(0);
    cover_trpc_.push_back((T_RPC & inst_list_.at(i)->get_call_type()) == T_RPC);
    flow_srpc_.push_back(false);
  }

  TBSYS_LOG(TRACE, "set_insts: %ld, inner_list_: %ld, graph_: %ld, degree_: %ld", insts.count(), inst_list_.count(),
            graph_.count(), degree_.count());
  active_node_count_ = insts.count();
  return OB_SUCCESS;
}

bool ObProcDepGraph::is_leaf(int64_t id) const
{
  return graph_.at(id).next_ == NULL;
}

bool ObProcDepGraph::is_root(int64_t id) const
{
  return degree_.at(id) == 0;
}

bool ObProcDepGraph::is_done(int64_t id) const
{
  return degree_.at(id) == -1;
}

bool ObProcDepGraph::cover_tnode(int64_t id) const
{
  return cover_trpc_.at(id);
}

void ObProcDepGraph::mark_done(int64_t id, GraphType type)
{
  OB_ASSERT(type = type_);
  degree_.at(id) = -1;

  --active_node_count_;
  const SpNode *node = &(graph_.at(id));
  const SpNode *nxt = node->next_;
  while( nxt != NULL)
  {
    degree_.at(nxt->id_)--;
    nxt = nxt->next_;
  }
}

bool ObProcDepGraph::is_stype(int64_t id) const
{
  return inst_list_.at(id)->is_srpc();
}

bool ObProcDepGraph::is_ttype(int64_t id) const
{
  return inst_list_.at(id)->is_trpc();
}

int ObProcDepGraph::reorder_for_group(ObIArray<int64_t> &seq)
{
  build_dep_graph(Backward);
  int t_iter = -1, s_iter = -1;
  while( t_iter != 0 || s_iter != 0 )
  {
    bool find = true;
    t_iter = 0;
    s_iter = 0;
    //execute t-op at last
    while( active_node_count_ != 0 && find )
    {
      find = false;
      for(int64_t i = inst_list_.count() - 1; i >= 0; --i)
      {
        if( is_done(i) || is_stype(i) || !is_root(i) ) continue;
        else
        {
          seq.push_back(i);
          mark_done(i, Backward);
          t_iter ++;
          find = true;
          break;
        }
      }
    }

    find = true;
    //execute s-op that covered by some t-op
    while( active_node_count_ != 0 && find )
    {
      find = false;
      for(int64_t i = inst_list_.count() - 1; i >= 0; --i)
      {
        if( is_done(i) || is_ttype(i) || !is_root(i) || !cover_tnode(i) ) continue;
        else
        {
          seq.push_back(i);
          mark_done(i, Backward);
          s_iter ++;
          find = true;
          break;
        }
      }
    }
  }
  //firstly execute s-op that does not covered by any t-op
  //for such op, we can use more aggressive optimization
  while( active_node_count_ != 0 )
  {
    for(int64_t i = inst_list_.count() - 1; i >= 0; --i)
    {
      if( is_done(i) || !is_root(i) ) continue;
      else
      {
        seq.push_back(i);
        mark_done(i, Backward);
        break;
      }
    }
  }
  return OB_SUCCESS;
}

bool ObProcDepGraph::could_split() const
{
  bool ret = true;
  for(int64_t i = 0; ret && i < cover_trpc_.count(); ++i)
  {
    /**
      * If no s_rpc is covered by any t_rpc, we can use preexecute optimization
      */
    if( cover_trpc_.at(i) && (S_RPC == (inst_list_.at(i)->get_call_type() & S_RPC)))
    {
      TBSYS_LOG(TRACE, "can not split[%ld, %d, %d]", i, cover_trpc_.at(i), inst_list_.at(i)->get_call_type());
      ret = false;
    }
  }
  return ret;
}

int ObProcDepGraph::split(ObIArray<int64_t> &seq, int64_t &pre_count)
{
  int ret = OB_SUCCESS;
  if( !inited_ ) build_dep_graph(Backward);
  pre_count = 0;
  if( could_split() )
  {
    for(int64_t i = 0; i < inst_list_.count(); ++i)
    {
      if( inst_list_.at(i)->get_call_type() == S_RPC )
      {
        tag_true_flow(i);
      }
    }

    for(int64_t i = 0; i < inst_list_.count(); ++i)
    {
      //TBSYS_LOG(INFO, "%ld: %d", i, flow_srpc_.at(i));
      if( flow_srpc_.at(i) )
      {
        seq.push_back(i);
        pre_count ++;
      }
    }

    for(int64_t i = 0; i < inst_list_.count(); ++i)
    {
      if( inst_list_.at(i)->get_call_type() != S_RPC )
      {
        seq.push_back(i);
      }
    }
  }
  else
  {
    pre_count = 0;
    for(int64_t i = 0; i < inst_list_.count(); ++i)
    {
      seq.push_back(i);
    }
  }
  return ret;
}

int ObProcDepGraph::tag_true_flow(int64_t id)
{
  int ret = OB_SUCCESS;

  if( !flow_srpc_.at(id) )
  {
    flow_srpc_.at(id) = true;
    SpNode *nxt = graph_.at(id).next_;
    while( NULL != nxt )
    {
      if( Da_True_Dep == (nxt->dep_type_ & Da_True_Dep) )
      {
        tag_true_flow(nxt->id_);
      }
      nxt = nxt->next_;
    }
  }
  return ret;
}

int64_t ObProcDepGraph::to_string(char *buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "Dump dependence graph\n");
  for(int64_t i = 0; i < graph_.count(); ++i)
  {
    const SpNode &node = graph_.at(i);
    databuff_printf(buf, buf_len, pos, "%ld[%d, s:%d, t:%d]: ", node.id_, degree_.at(node.id_), is_stype(i), is_ttype(i));
    const SpNode *nxt = node.next_;
    while(nxt != NULL)
    {
      databuff_printf(buf, buf_len, pos, "%ld, ", nxt->id_);
      nxt = nxt->next_;
    }
    databuff_printf(buf, buf_len, pos, "\n");
  }
  return pos;
}
