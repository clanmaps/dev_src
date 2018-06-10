/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_transformer.h
 * @brief logical plan --transformer--> physical plan
 *
 * modified by longfei：generate physical plan for create, drop, index in select
 * modified by maoxiaoxiao:add and modify some functions to generate a correct physicl plan if a table with index has a insert, delete, update, replace and alter operation
 * modify some functions to generate a physicl plan for bloomfilter join
 * modified by fanqiushi: add some functions to create an phsical plan for semijoin
 * modified by wangjiahao: add method to generate physical plan for update_more
 * modified by wangdonghui: add some function to generate physical plan for procedure
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author Qiushi FAN <qsfan@ecnu.cn>
 * @author wangjiahao <51151500051@ecnu.edu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_26
 */

/** * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_transformer.h
 *
 * Authors:
 *   Guibin Du <tianguan.dgb@taobao.com>
 *
 */
#ifndef _OB_TRANSFORMER_H
#define _OB_TRANSFORMER_H

#include "ob_phy_operator.h"
#include "ob_logical_plan.h"
#include "ob_multi_phy_plan.h"
#include "ob_multi_logic_plan.h"
#include "ob_sql_context.h"
#include "ob_insert_stmt.h"
#include "ob_show_stmt.h"
#include "ob_values.h"
#include "ob_expr_values.h"
#include "ob_table_rpc_scan.h"
#include "common/ob_list.h"
#include "common/ob_row_desc_ext.h"
#include "common/ob_se_array.h"

//add longfei [secondary index select]
#include "common/ob_secondary_index_service.h"
//add e
//add maoxx
#include "ob_update_stmt.h"
//add e
/*add maoxx [bloomfilter_join] 20160417*/
#include "ob_join.h"
/*add e*/

//add lbzhong [auto_increment] 20161218:b
#include "ob_auto_increment_filter.h"
#include "ob_ups_modify.h"
//add:e

#include "sql/ob_procedure.h" //add by zhutao
#include "ob_procedure_compilation_guard.h" //add by zhutao

namespace oceanbase
{
  namespace sql
  {
    struct ObProcedureCompilationContext;
    class ObWhenFilter;
    class ObTransformer
    {
      public:
        ObTransformer(ObSqlContext &context);
        virtual ~ObTransformer();

        int generate_physical_plans(
            ObMultiLogicPlan &logical_plans,
            ObMultiPhyPlan &physical_plans,
            ErrStat& err_stat);

        int gen_physical_select(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        ObSqlContext* get_sql_context();

      private:
        DISALLOW_COPY_AND_ASSIGN(ObTransformer);
        void *trans_malloc(const size_t nbyte);
        void trans_free(void* p);

        //add by zhujun 2014-12-05:b
        //code_coverage_zhujun
        /**
         * @brief gen_physical_procedure
         * generate a physical plan PhyOperator of procedure body block based on a logical plan of procedure body block stmt
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store ObProcedure that procedure body block PhyOperator
         * @param err_stat store error status
         * @param query_id is ObProcedureStmt id
         * @param index physical query id
         * @return error code
         */
        int gen_physical_procedure(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        /**
         * @brief gen_physical_procedure_create
         * generate a physical plan PhyOperator of procedure create  based on a logical plan of procedure create stmt
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store ObProcedureCreate that procedure create PhyOperator
         * @param err_stat store error status
         * @param query_id is ObProcedureCreateStmt id
         * @param index physical query id
         * @return error code
         */
        int gen_physical_procedure_create(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        /**
         * @brief gen_physical_procedure_drop
         * generate a physical plan PhyOperator of procedure drop based on a logical plan of procedure drop stmt
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store ObProcedureDrop that procedure drop PhyOperator
         * @param err_stat store error status
         * @param query_id is ObProcedureDropStmt id
         * @param index physical query id
         * @return error code
         */
        int gen_physical_procedure_drop(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        /**
         * @brief gen_physical_procedure_execute
         * generate a physical plan PhyOperator of procedure execute based on a logical plan of procedure execute
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store ObProcedureExecute that procedure execute PhyOperator
         * @param err_stat store error status
         * @param query_id is ObProcedureExecuteStmt id
         * @param index physical query id
         * @return error code
         */
        int gen_physical_procedure_execute(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

      public:
        /**
         * @brief gen_physical_procedure_inst_var_set
         * used to analyze the structure of expression, find out the variables used in the instruction
         * and update the corresponding variable set.
         * @param var_set  the variable set that variables/array added into
         * @param raw_expr_list  the expression list to be analyzed
         * @return error code
         */
        int gen_physical_procedure_inst_var_set(
                        SpVariableSet &var_set,
                        const ObIArray<const ObRawExpr *> & raw_expr_list
                        );
        /**
         * @brief gen_physical_procedure_inst_var_set
         * used to analyze the structure of expression, find out the variables used in the instruction
         * and update the corresponding variable set.
         * @param var_set  the variable set that variables/array added into
         * @param raw_expr  the expression to be analyzed
         * @return error code
         */
        int gen_physical_procedure_inst_var_set(SpVariableSet &var_set,
                        const ObSqlRawExpr *raw_expr);

        /**
         * @brief gen_physical_procedure_inst_var_set
         * used to analyze the structure of expression, find out the variables used in the instruction
         * and update the corresponding variable set.
         * @param var_set  the variable set that variables/array added into
         * @param sql_raw_expr_list  the expression to be analyzed
         * @return error code
         */
        int gen_physical_procedure_inst_var_set(SpVariableSet &var_set,
                        const ObIArray<const ObSqlRawExpr *> &sql_raw_expr_list);
        /**
         * @brief gen_physical_procedure_inst_var_set
         * used to analyze the table used in the sql
         * and update the corresponding variable set.
         * @param var_set  the variable set that variables/array added into
         * @param table_list   the table id that used
         * @return error code
         */
        int gen_physical_procedure_inst_var_set(SpVariableSet &var_set,
                        const ObIArray<uint64_t> &table_list);
        /**
         * @brief ext_var_info_where
         * extract where expression variable information
         * @param raw_expr is raw exprission
         * @param is_rowkey is a flag of rowkey existence
         * @return error code
         */
          private:
        int ext_var_info_where(const ObSqlRawExpr *raw_expr, bool is_rowkey);

        /**
         * @brief ext_var_info_project
         * extract variable information
         * @param raw_expr is raw exprission
         * @return error code
         */
        int ext_var_info_project(const ObSqlRawExpr *raw_expr);
        /**
         * @brief ext_table_id
         * set inst used table id
         * @param table_id is table id
         * @return error code
         */
        int ext_table_id(uint64_t table_id);

        /**
         * @brief gen_physical_procedure_if
         * generate a physical plan if instruction based on a logical plan of if
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureIfStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_if(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_declare
         * generate a physical plan declare instruction based on a logical plan of declare
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param err_stat store error status
         * @param query_id is ObProcedureAssginStmt index
         * @param proc_op point ObProcedure object
         * @return error code
         */
        int gen_physical_procedure_declare(
                        ObLogicalPlan *logical_plan,
                        //			ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op);
        //			int32_t* index);

        /**
         * @brief gen_physical_procedure_assign
         * generate a physical plan assign instruction based on a logical plan of assign
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureAssginStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_assign(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_insert
         * wrap the insert physical plan into the stored procedure instruction
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObInsertStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_insert(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_replace
         * generate a physical plan replace instruction based on a logical plan of replace
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObInsertStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_replace(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_update
         * generate a physical plan update instruction based on a logical plan of update
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObUpdateStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_update(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat &err_stat,
                        const uint64_t &query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        //add by wangdonghui 20160623 :b
        /**
         * @brief gen_physical_procedure_delete
         * generate a physical plan delete instruction based on a logical plan of delete
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObDeleteStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_delete(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat &err_stat,
                        const uint64_t &query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst
                );
        //add :e
        /**
         * @brief gen_physical_procedure_select_into
         * generate a physical plan select_into instruction based on a logical plan of select_into
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureSelectIntoStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_select_into(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_loop
         * generate a physical plan loop instruction based on a logical plan of loop
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureLoopStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_loop(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_inst
         * generate a physical plan instruction of procedure body block based on a logical plan of procedure body block
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure  instructions
         * @param err_stat store error status
         * @param query_id is stmt id
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_inst(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_elseif
         * generate a physical plan elseif instruction based on a logical plan of elseif
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureElseIfStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_elseif(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure *proc_op,
                        SpIfCtrlInsts *&elseif_ctrl,
                        SpMultiInsts *mul_inst = NULL
                        );
        /**
         * @brief gen_physical_procedure_case
         * generate a physical plan case instruction based on a logical plan of case
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureCaseStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_case(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure* proc_op,
                        SpMultiInsts* mul_inst =NULL);
        /**
         * @brief gen_physical_procedure_casewhen
         * generate a physical plan casewhen instruction based on a logical plan of casewhen
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureCaseWhenStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_casewhen(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat& err_stat,
                        const uint64_t& query_id,
                        ObProcedure* proc_op,
                        SpMultiInsts* mul_inst =NULL);
        /**
         * @brief gen_physical_set_array_value
         * generate a physical plan PhyOperator of variable set array based on a logical plan of variable set array stmt
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store variable set array PhyOperator
         * @param err_stat store error status
         * @param query_id is variable set array stmt id
         * @param index physical query id
         * @return error code
         */
        int gen_physical_set_array_value(
                        ObLogicalPlan *logical_plan,
                        ObPhysicalPlan *physical_plan,
                        ErrStat &err_stat,
                        const uint64_t &query_id,
                        int32_t *index
                        );


        //add hjw 20151229:b
        /**
         * @brief gen_physical_procedure_while
         * generate a physical plan while instruction based on a logical plan of while
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureWhileStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_while(
               ObLogicalPlan *logical_plan,
               ObPhysicalPlan *physical_plan,
               ErrStat& err_stat,
               const uint64_t& query_id,
               ObProcedure *proc_op,
               SpMultiInsts* mul_inst = NULL);
        //add hjw 20151229:e

        //add wdh 20160623 :b
        /**
         * @brief gen_physical_procedure_exit
         * generate a physical plan exit instruction based on a logical plan of exit
         * @param logical_plan point a logical plan that used to generate physical plan
         * @param physical_plan point a physical plan that used to store procedure instructions
         * @param err_stat store error status
         * @param query_id is ObProcedureExitStmt index
         * @param proc_op point ObProcedure object
         * @param mul_inst store generated instructions
         * @return error code
         */
        int gen_physical_procedure_exit(
               ObLogicalPlan *logical_plan,
               ObPhysicalPlan *physical_plan,
               ErrStat& err_stat,
               const uint64_t& query_id,
               ObProcedure *proc_op,
               SpMultiInsts* mul_inst = NULL);
        //add :e

        //code_coverage_zhujun
        //add:e
        int generate_physical_plan(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id = common::OB_INVALID_ID,
            int32_t* index = NULL);

        int add_cur_time_plan(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObCurTimeType& type);

        int gen_physical_replace(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_insert_new(
          ObLogicalPlan *logical_plan,
          ObPhysicalPlan *physical_plan,
          ErrStat& err_stat,
          const uint64_t& query_id,
          int32_t* index);
        int gen_physical_delete(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_update(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_explain(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        // longfei [create index]
        /**
         * @brief gen_physical_create_index: generate physical plan for create index
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param query_id
         * @param index
         * @return err code
         */
        int gen_physical_create_index(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        // longfei [drop index]
        /**
         * @brief gen_physical_drop_index generate physical plan for drop index
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param query_id
         * @param index
         * @author longfei <longfei@stu.ecnu.edu.cn>
         * @return err code
         */
        int gen_physical_drop_index(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        int gen_physical_create_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_drop_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_truncate_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index); //add hxlong [truncate table] 20170403
        int gen_physical_alter_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_show(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        //add wangjiahao [table lock] 20160616 :b
        int gen_physical_lock_table(
                    ObLogicalPlan *logical_plan,
                    ObPhysicalPlan *physical_plan,
                    ErrStat& err_stat,
                    const uint64_t& query_id,
                    int32_t* index);
        //add :e
        int gen_phy_tables(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            bool& group_agg_pushed_down,
            bool& limit_pushed_down,
            oceanbase::common::ObList<ObPhyOperator*>& phy_table_list,
            oceanbase::common::ObList<ObBitSet<> >& bitset_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& remainder_cnd_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& none_columnlize_alias);
        int gen_physical_kill_stmt(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan* physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        //add longfei
        /**
         * @brief gen_phy_table_not_back
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param table_op
         * @param group_agg_pushed_down
         * @param limit_pushed_down
         * @param is_use_storing_column
         * @param index_tid
         * @param filter_array
         * @param project_array
         * @return err code
         */
        int gen_phy_table_not_back(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            ObPhyOperator*& table_op,
            bool* group_agg_pushed_down = NULL,
            bool* limit_pushed_down = NULL,
            bool is_use_storing_column = false,
            uint64_t index_tid=OB_INVALID_ID,
            common::Expr_Array *filter_array=NULL,
            common::Expr_Array *project_array=NULL );
        /**
         * @brief gen_phy_table_back
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param table_op
         * @param group_agg_pushed_down
         * @param limit_pushed_down
         * @param index_tid_without_storing
         * @param filter_array
         * @param project_array
         * @return err code
         */
        int gen_phy_table_back(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            ObPhyOperator*& table_op,
            bool* group_agg_pushed_down = NULL,
            bool* limit_pushed_down = NULL,
            uint64_t index_tid_without_storing=OB_INVALID_ID,
            common::Expr_Array * filter_array = NULL,
            common::Expr_Array * project_array = NULL);
        //add e
        int gen_phy_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            ObPhyOperator*& table_op,
            bool* group_agg_pushed_down = NULL,
            bool* limit_pushed_down = NULL);

        //add longfei
        /**
         * @brief handle_index_for_one_table: Query Optimization for using secondaryIndex
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param table_op
         * @param group_agg_pushed_down
         * @param limit_pushed_down
         * @return ture : use index or false : can't use index
         */
        bool handle_index_for_one_table(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            ObPhyOperator*& table_op,
            bool* group_agg_pushed_down = NULL,
            bool* limit_pushed_down = NULL);
        //add:e

        int gen_phy_joins(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            /*add maoxx [bloomfilter_join] 20160417*/
            ObJoin::JoinType join_type,
            /*add e*/
            oceanbase::common::ObList<ObPhyOperator*>& phy_table_list,
            oceanbase::common::ObList<ObBitSet<> >& bitset_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& remainder_cnd_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& none_columnlize_alias,
            int32_t hint_idx = 0);
        //add fanqiushi [semi_join] [0.1] 20150826:b
        /**
        * @brief create a physical plan of semijoin.
        * @param logical plan,statement,array of operator,array of bitset,array of expression.
        * @param physical plan,error state.
        * @return Error code.
        */
        int gen_phy_semi_join(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            oceanbase::common::ObList<ObPhyOperator*>& phy_table_list,
            oceanbase::common::ObList<ObBitSet<> >& bitset_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& remainder_cnd_list,
            oceanbase::common::ObList<ObSqlRawExpr*>& none_columnlize_alias);
        //add:e
        int gen_phy_group_by(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            ObPhyOperator *in_op,
            ObPhyOperator *&out_op);
        int gen_phy_scalar_aggregate(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            ObPhyOperator *in_op,
            ObPhyOperator *&out_op);
        int gen_phy_distinct(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            ObPhyOperator *in_op,
            ObPhyOperator *&out_op);
        int gen_phy_order_by(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            ObPhyOperator *in_op,
            ObPhyOperator *&out_op,
            bool use_generated_id = false);
        int gen_phy_limit(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObSelectStmt *select_stmt,
            ObPhyOperator *in_op,
            ObPhyOperator *&out_op);
        int gen_phy_values(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObInsertStmt *insert_stmt,
            const ObRowDesc& row_desc,
            const ObRowDescExt& row_desc_ext,
            const ObSEArray<int64_t, 64> *row_desc_map,
            ObExprValues& value_op);
        int gen_phy_static_data_scan(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObInsertStmt *insert_stmt,
            const ObRowDesc& row_desc,
            const common::ObSEArray<int64_t, 64> &row_desc_map,
            const uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            ObTableRpcScan &table_scan);
        int gen_phy_show_tables(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        //add longfei
        /**
         * @brief gen_phy_show_index generate physical plan for show index
         * @param physical_plan
         * @param err_stat
         * @param show_stmt
         * @param out_op
         * @return err code
         */
        int gen_phy_show_index(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        //add:e
        int gen_phy_show_columns(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int gen_phy_show_variables(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int gen_phy_show_warnings(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int gen_phy_show_grants(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int gen_phy_show_table_status(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
	   //zhounan unmark:b
       int gen_physical_cursor_declare(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
       int gen_physical_cursor_fetch(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
       int gen_physical_cursor_fetch_into(
			   ObLogicalPlan *logical_plan,
			   ObPhysicalPlan *physical_plan,
			   ErrStat& err_stat,
			   const uint64_t& query_id,
			   int32_t* index);
       int gen_physical_cursor_fetch_prior(
	       ObLogicalPlan *logical_plan,
	       ObPhysicalPlan *physical_plan,
	       ErrStat& err_stat,
	       const uint64_t& query_id,
	       int32_t* index);
       int gen_physical_cursor_fetch_prior_into(
       	       ObLogicalPlan *logical_plan,
       	       ObPhysicalPlan *physical_plan,
       	       ErrStat& err_stat,
       	       const uint64_t& query_id,
       	       int32_t* index);
       int gen_physical_cursor_fetch_first(
	    ObLogicalPlan *logical_plan,
	    ObPhysicalPlan *physical_plan,
	    ErrStat& err_stat,
	    const uint64_t& query_id,
	    int32_t* index);
       int gen_physical_cursor_fetch_first_into(
      			   ObLogicalPlan *logical_plan,
      			   ObPhysicalPlan *physical_plan,
      			   ErrStat& err_stat,
      			   const uint64_t& query_id,
      			   int32_t* index);
       int gen_physical_cursor_fetch_last(
	    ObLogicalPlan *logical_plan,
	    ObPhysicalPlan *physical_plan,
	    ErrStat& err_stat,
	    const uint64_t& query_id,
	    int32_t* index);

       int gen_physical_cursor_fetch_last_into(
             			   ObLogicalPlan *logical_plan,
             			   ObPhysicalPlan *physical_plan,
             			   ErrStat& err_stat,
             			   const uint64_t& query_id,
             			   int32_t* index);

       int gen_physical_cursor_fetch_relative(
	    ObLogicalPlan *logical_plan,
	    ObPhysicalPlan *physical_plan,
	    ErrStat& err_stat,
	    const uint64_t& query_id,
		 int32_t* index);

       int gen_physical_cursor_fetch_relative_into(
       	    ObLogicalPlan *logical_plan,
       	    ObPhysicalPlan *physical_plan,
       	    ErrStat& err_stat,
       	    const uint64_t& query_id,
			int32_t* index);

       int gen_physical_cursor_fetch_absolute(
	    ObLogicalPlan *logical_plan,
	    ObPhysicalPlan *physical_plan,
	    ErrStat& err_stat,
	    const uint64_t& query_id,
	    int32_t* index);

       int gen_physical_cursor_fetch_absolute_into(
                    			   ObLogicalPlan *logical_plan,
                    			   ObPhysicalPlan *physical_plan,
                    			   ErrStat& err_stat,
                    			   const uint64_t& query_id,
                    			   int32_t* index);

       int gen_physical_cursor_fetch_fromto(
	       ObLogicalPlan *logical_plan,
	       ObPhysicalPlan *physical_plan,
	       ErrStat& err_stat,
	       const uint64_t& query_id,
	       int32_t* index);
       int gen_physical_cursor_open(
	       ObLogicalPlan *logical_plan,
	       ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
			
       int gen_physical_cursor_close(
	    ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

//e:add by zhounan

        int gen_phy_show_processlist(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        template <class T>
        int get_stmt(
            ObLogicalPlan *logical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            T *& stmt);
        template <class T>
        int add_phy_query(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            T * stmt,
            ObPhyOperator *phy_op,
            int32_t* index);
        int gen_physical_priv_stmt(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_prepare(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_variable_set(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_execute(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_deallocate(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_phy_table_for_update(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            const ObRowDesc &row_desc,
            const ObRowDescExt &row_desc_ext,
            ObPhyOperator*& table_op);
        //add maoxx
        /**
         * @brief cons_whole_row_desc_for_delete
         * construct row description of the data used by the delete statement if the table has index
         * @param table_id
         * @param desc
         * @param desc_ext
         * @return OB_SUCCESS or other ERROR
         */
        int cons_whole_row_desc_for_delete(uint64_t table_id, ObRowDesc &desc, ObRowDescExt &desc_ext);

        /**
         * @brief cons_whole_row_desc_for_update
         * construct row description of the data used by the update statement if the table has index
         * @param stmt
         * @param table_id
         * @param desc
         * @param desc_ext
         * @return OB_SUCCESS or other ERROR
         */
        int cons_whole_row_desc_for_update(const ObStmt *stmt, uint64_t table_id, ObRowDesc &desc, ObRowDescExt &desc_ext);

        /**
         * @brief cons_whole_row_desc_for_replace
         * construct row description of the data used by the replace statement if the table has index
         * @param stmt
         * @param table_id
         * @param desc
         * @param desc_ext
         * @return OB_SUCCESS or other ERROR
         */
        int cons_whole_row_desc_for_replace(const ObStmt *stmt, uint64_t table_id, ObRowDesc &desc, ObRowDescExt &desc_ext);

        /**
         * @brief column_in_stmt
         * decide if the given column is in the sql query statement
         * @param stmt
         * @param table_id
         * @param cid
         * @param in_stmt_flag
         * @return OB_SUCCESS or other ERROR
         */
        int column_in_stmt(const ObStmt* stmt, uint64_t table_id, uint64_t cid, bool &in_stmt_flag);

        /**
         * @brief gen_phy_table_for_update_new
         * generate physical table if the table with index has update statements
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param rowkey_info
         * @param row_desc
         * @param row_desc_ext
         * @param table_op
         * @return OB_SUCCESS or other ERROR
         */
        int gen_phy_table_for_update_new(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            const ObRowDesc &row_desc,
            const ObRowDescExt &row_desc_ext,
            ObPhyOperator*& table_op);

        /**
         * @brief gen_phy_table_for_delete
         * generate physical table if the table with index has delete statements
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param rowkey_info
         * @param row_desc
         * @param row_desc_ext
         * @param table_op
         * @return OB_SUCCESS or other ERROR
         */
        int gen_phy_table_for_delete(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            const ObRowDesc &row_desc,
            const ObRowDescExt &row_desc_ext,
            ObPhyOperator*& table_op);

        /**
         * @brief gen_physical_replace_new
         * generate physical plan if the table with index has replace statements
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param query_id
         * @param index
         * @return OB_SUCCESS or other ERROR
         */
        int gen_physical_replace_new(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);

        /**
         * @brief gen_phy_static_data_scan_for_replace
         * generate physical static data scan if the table with index has replace statements
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param insert_stmt
         * @param row_desc
         * @param row_desc_map
         * @param table_id
         * @param rowkey_info
         * @param table_scan
         * @return OB_SUCCESS or other ERROR
         */
        int gen_phy_static_data_scan_for_replace(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObInsertStmt *insert_stmt,
            const ObRowDesc& row_desc,
            const ObSEArray<int64_t, 64> &row_desc_map,
            const uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            ObTableRpcScan &table_scan
            //add lbzhong [auto_increment] 20161217:b
            , const uint64_t auto_column_id, const int64_t auto_value
            //add:e
            );

        /**
         * @brief gen_phy_values_for_replace
         * generate physical values if the table with index has replace statements
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param insert_stmt
         * @param row_desc
         * @param row_desc_ext
         * @param row_desc_map
         * @param value_op
         * @return OB_SUCCESS or other ERROR
         */
        int gen_phy_values_for_replace(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObInsertStmt *insert_stmt,
            const ObRowDesc& row_desc,
            const ObRowDescExt& row_desc_ext,
            const ObSEArray<int64_t, 64> *row_desc_map,
            ObExprValues& value_op
            //add lbzhong [auto_increment] 20161217:b
            , const uint64_t auto_column_id, const int64_t auto_value
            //add:e
            );
        //add maoxx [replace bug fix] 20170317
        int get_row_desc_intersect(
            ObRowDesc &row_desc,
            ObRowDescExt &row_desc_ext,
            ObRowDesc row_desc_index,
            ObRowDescExt row_desc_ext_index);

        int gen_phy_values_index(ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObInsertStmt *insert_stmt,
            ObRowDesc& row_desc,
            ObRowDescExt &row_desc_ext,
            const ObRowDesc& row_desc_index,
            const ObRowDescExt& row_desc_ext_index,
            const ObSEArray<int64_t, 64> *row_desc_map,
            ObExprValues& value_op
            //add huangjianwei [auto_increment] 20170703:b
            , const uint64_t auto_column_id, const int64_t auto_value
            //add:e
            );
        //add e

        //add wangjiahao [dev_update_more] 20151204 :b
        /**
         * @brief gen_phy_table_for_update_more is a new method
         * to generate physical plan for update which support update
         * not even given full rowkey condition but also
         * other conditions. In the full rowkey condition
         * physical plan generated will be as the same as
         * the old gen_phy_table_for_update method.
         * @param logical_plan
         * @param physical_plan
         * @param err_stat
         * @param stmt
         * @param table_id
         * @param rowkey_info
         * @param row_desc
         * @param row_desc_ext
         * @param table_op
         * @return OB_SUCCESS
         */
        int gen_phy_table_for_update_more(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            ObStmt *stmt,
            uint64_t table_id,
            const ObRowkeyInfo &rowkey_info,
            const ObRowDesc &row_desc,
            const ObRowDescExt &row_desc_ext,
            ObPhyOperator*& table_op);
       //add :e

        //add huangjianwei [auto_increment] 20170703:b
        int cons_auto_increment_row_desc(const uint64_t table_id,
            const ObStmt *stmt,
            ObRowDescExt &row_desc_ext,
            ObRowDesc &row_desc,
            ErrStat& err_stat);
        //add:b
        int gen_physical_update_new(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan*& physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int cons_row_desc(const uint64_t table_id,
            const ObStmt *stmt,
            ObRowDescExt &row_desc_ext,
            ObRowDesc &row_desc,
            const ObRowkeyInfo *&rowkey_info,
            common::ObSEArray<int64_t, 64> &row_desc_map,
            ErrStat& err_stat);
        int gen_physical_delete_new(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan* physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_start_trans(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan* physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_physical_end_trans(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan* physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int wrap_ups_executor(
            ObPhysicalPlan *physical_plan,
            const uint64_t query_id,
            ObPhysicalPlan*& new_plan,
            int32_t* index,
            ErrStat& err_stat);
        int gen_phy_select_for_update(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int try_push_down_group_agg(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObSelectStmt *select_stmt,
            bool& group_agg_pushed_down,
            ObPhyOperator *& scan_op);
        int try_push_down_limit(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const ObSelectStmt *select_stmt,
            bool& limit_pushed_down,
            ObPhyOperator *scan_op);
        int gen_phy_show_parameters(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int gen_phy_show_create_table(
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            ObShowStmt *show_stmt,
            ObPhyOperator *&out_op);
        int cons_table_definition(
            const common::ObTableSchema& table_schema,
            char* buf,
            const int64_t& buf_len,
            int64_t& pos,
            ErrStat& err_stat);
        int gen_physical_alter_system(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            int32_t* index);
        int gen_phy_when(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            ObPhyOperator& child_op,
            ObWhenFilter *& when_filter);
        int merge_tables_version(ObPhysicalPlan & outer_plan, ObPhysicalPlan & inner_plan);

        bool parse_join_info(const ObString &join_info, TableSchema &table_schema);
        bool check_join_column(const int32_t column_index,
              const char* column_name, const char* join_column_name,
              TableSchema& schema, const ObTableSchema& join_table_schema);
        
        int allocate_column_id(TableSchema & table_schema);
        int gen_physical_change_obi_stmt(
          ObLogicalPlan *logical_plan,
          ObPhysicalPlan* physical_plan,
          ErrStat& err_stat,
          const uint64_t& query_id,
          int32_t* index);
	//add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        int ob_write_obj_for_delete(ModuleArena &allocator, const ObObj &src, ObObj &dst,ObObj type);
         //add e
              
//<<<<<<< HEAD
//=======
        //add lbzhong [auto_increment] 20161126:b
        bool need_auto_increment(
            ObLogicalPlan *logical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id);
        uint64_t get_auto_column_id(const uint64_t table_id);
        int update_and_get_auto_value(const uint64_t table_id, const uint64_t column_id, const int64_t row_count, int64_t& auto_value);
        int gen_phy_auto_increment(
            ObLogicalPlan *logical_plan,
            ObPhysicalPlan *physical_plan,
            ErrStat& err_stat,
            const uint64_t& query_id,
            const uint64_t when_expr_size,
            ObExprValues* value_op,
            ObWhenFilter* when_filter,
            ObAutoIncrementFilter*& auto_increment_filter);
        int check_and_load_auto_value(const uint64_t auto_column_id,
                                     const bool need_modify_index_flag,
                                     ObInsertStmt *insert_stmt,
                                     const int64_t row_count,
                                     int64_t& auto_value,
                                     ObUpsModifyWithDmlType *&ups_modify);
        int add_auto_increment_op(ObPhysicalPlan *physical_plan,
                                  ErrStat& err_stat,
                                  ObAutoIncrementFilter*& auto_increment_filter_op,
                                  ObPhyOperator* parent_op,
                                  ObPhyOperator* child_op);
        //add:e
//>>>>>>> dev

      private:
        common::ObIAllocator *mem_pool_;
        ObSqlContext *sql_context_;
        bool group_agg_push_down_param_;


        //add by zhutao [a switch from normal/procedure compilation]
        bool compile_procedure_;
        ObProcedureCompilationContext context_;
        //add :e
    };

    inline ObSqlContext* ObTransformer::get_sql_context()
    {
      return sql_context_;
    }
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_TRANSFORMER_H */
