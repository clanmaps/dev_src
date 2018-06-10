/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file build_plan.cpp
 * @brief resolve or destory logical plan
 *
 * modified by longfei：
 * 1.add resolve function of create, drop and show index
 * modified by zhujun：add build procedure logic plan method
 *
 * modified by zhutao
 * modified by wangdonghui
 * @version __DaSE_VERSION
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author zhujun <51141500091@ecnu.edu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_24
 */
 
#include "sql_parser.tab.h"
#include "build_plan.h"
#include "dml_build_plan.h"
#include "priv_build_plan.h"
#include "ob_raw_expr.h"
#include "common/ob_bit_set.h"
#include "ob_select_stmt.h"
#include "ob_multi_logic_plan.h"
#include "ob_insert_stmt.h"
#include "ob_delete_stmt.h"
#include "ob_update_stmt.h"
#include "ob_schema_checker.h"
#include "ob_explain_stmt.h"
#include "ob_create_table_stmt.h"
#include "ob_drop_table_stmt.h"
#include "ob_truncate_table_stmt.h" //add hxlong [Truncate Table]:20170403
#include "ob_show_stmt.h"
#include "ob_create_user_stmt.h"
#include "ob_prepare_stmt.h"
#include "ob_variable_set_stmt.h"
#include "ob_execute_stmt.h"
#include "ob_deallocate_stmt.h"
#include "ob_start_trans_stmt.h"
#include "ob_end_trans_stmt.h"
#include "ob_column_def.h"
#include "ob_alter_table_stmt.h"
#include "ob_alter_sys_cnf_stmt.h"
#include "ob_kill_stmt.h"
//zhounan unmark:b
#include "ob_cursor_fetch_stmt.h"
#include "ob_cursor_fetch_into_stmt.h"
#include "ob_cursor_fetch_prior_stmt.h"
#include "ob_cursor_fetch_prior_into_stmt.h"
#include "ob_cursor_fetch_first_stmt.h"
#include "ob_cursor_fetch_first_into_stmt.h"
#include "ob_cursor_fetch_last_stmt.h"
#include "ob_cursor_fetch_last_into_stmt.h"
#include "ob_cursor_fetch_relative_stmt.h"
#include "ob_cursor_fetch_relative_into_stmt.h"
#include "ob_cursor_fetch_absolute_stmt.h"
#include "ob_cursor_fetch_abs_into_stmt.h"
#include "ob_cursor_fetch_fromto_stmt.h"
#include "ob_cursor_declare_stmt.h"
#include "ob_cursor_open_stmt.h"
#include "ob_cursor_close_stmt.h"
//add:e
#include "parse_malloc.h"
#include "common/ob_define.h"
#include "common/ob_array.h"
#include "common/ob_string_buf.h"
#include "common/utility.h"
#include "common/ob_schema_service.h"
#include "common/ob_obi_role.h"
#include "ob_change_obi_stmt.h"
#include <stdint.h>

//longfei [create index]
#include "ob_create_index_stmt.h"
//longfei [drop index]
#include "ob_drop_index_stmt.h"

//add by zhujun:b
#include "ob_procedure_create_stmt.h"
#include "ob_procedure_drop_stmt.h"
#include "ob_procedure_stmt.h"
#include "ob_procedure_execute_stmt.h"
#include "ob_procedure_if_stmt.h"
#include "ob_procedure_elseif_stmt.h"
#include "ob_procedure_else_stmt.h"
#include "ob_procedure_declare_stmt.h"
#include "ob_procedure_assgin_stmt.h"
#include "ob_procedure_while_stmt.h"
#include "ob_procedure_loop_stmt.h"
#include "ob_procedure_exit_stmt.h"
#include "ob_procedure_case_stmt.h"
#include "ob_procedure_casewhen_stmt.h"
#include "ob_procedure_select_into_stmt.h"
#include "ob_variable_set_array_value_stmt.h"
#include "ob_transformer.h"
#include "ob_deallocate.h"
#include "ob_cursor_close.h"
#include "ob_result_set.h"
#include "ob_physical_plan.h"
#include <vector>
//add:e

//add wangjiahao [table lock] :b
#include "ob_lock_table_stmt.h"
//add :e
using namespace oceanbase::common;
using namespace oceanbase::sql;
using namespace std;

int resolve_multi_stmt(ResultPlan* result_plan, ParseNode* node);
int resolve_explain_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_const_value(
		ResultPlan * result_plan,
		ParseNode *def_node,
		ObObj& default_value);
int resolve_column_definition(
		ResultPlan * result_plan,
		ObColumnDef& col_def,
		ParseNode* node,
		bool *is_primary_key = NULL);
int resolve_table_elements(
		ResultPlan * result_plan,
		ObCreateTableStmt& create_table_stmt,
		ParseNode* node);
int resolve_create_table_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_drop_table_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);

//longfei [create index] [drop index]
int resolve_create_index_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_drop_index_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
        );


int resolve_show_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_prepare_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_variable_set_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_execute_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_deallocate_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
//zhounan unmark:b
int resolve_cursor_declare_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_open_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_prior_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_prior_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_first_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_first_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_last_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_last_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_relative_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_relative_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_absolute_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_absolute_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_fetch_fromto_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_cursor_close_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
//add:e
int resolve_alter_sys_cnf_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_kill_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);
int resolve_change_obi(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id);

//add by zhujun:b
//code_coverage_zhujun
int resolve_procedure_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_procedure_create_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_procedure_drop_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_procedure_execute_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_procedure_declare_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt *ps_stmt);
int resolve_procedure_assign_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt *ps_stmt);
int resolve_procedure_if_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt);
int resolve_procedure_elseif_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt);
int resolve_procedure_else_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt);
int resolve_procedure_while_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt);
//add by wdh 20160623:b
int resolve_procedure_exit_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt* ps_stmt);
//add :e
int resolve_procedure_case_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt);
int resolve_procedure_casewhen_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
//	uint64_t case_value,
	ObProcedureStmt* ps_stmt
	);
int resolve_procedure_select_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
int resolve_procedure_proc_block_stmt(
    ResultPlan *result_plan,
    ParseNode *node,
    ObProcedureStmt *stmt);
int resolve_procedure_loop_stmt(
    ResultPlan *result_plan,
    ParseNode *node,
    uint64_t &query_id,
    ObProcedureStmt *stmt);
int resolve_procedure_inner_stmt(
    ResultPlan *result_plan,
    ParseNode *node,
    uint64_t &query_id,
    ObProcedureStmt *stmt);
int resolve_variable_set_array_stmt(
    ResultPlan *result_plan,
    ParseNode *node,
    uint64_t &query_id);
//code_coverage_zhujun
//add:e
//add wangjiahao [table lock] 20160616 :b
int resolve_lock_table_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id);
//add :e
int resolve_multi_stmt(ResultPlan* result_plan, ParseNode* node)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node && node->type_ == T_STMT_LIST);
	if(node->num_child_ == 0)
	{
		ret = OB_ERROR;
	}
	else
	{
		result_plan->plan_tree_ = NULL;
		ObMultiLogicPlan* multi_plan = (ObMultiLogicPlan*)parse_malloc(sizeof(ObMultiLogicPlan), result_plan->name_pool_);
		if (multi_plan != NULL)
		{
			multi_plan = new(multi_plan) ObMultiLogicPlan;
			for(int32_t i = 0; i < node->num_child_; ++i)
			{
				ParseNode* child_node = node->children_[i];
				if (child_node == NULL)
					continue;

				if ((ret = resolve(result_plan, child_node)) != OB_SUCCESS)
				{
					multi_plan->~ObMultiLogicPlan();
					parse_free(multi_plan);
					multi_plan = NULL;
					break;
				}
				if(result_plan->plan_tree_ == NULL)
					continue;

				if ((ret = multi_plan->push_back((ObLogicalPlan*)(result_plan->plan_tree_))) != OB_SUCCESS)
				{
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Can not add logical plan to ObMultiLogicPlan");
					break;
				}
				result_plan->plan_tree_ = NULL;
			}
			result_plan->plan_tree_ = multi_plan;
		}
		else
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc space for ObMultiLogicPlan");
		}
	}
	return ret;
}

int resolve_explain_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node && node->type_ == T_EXPLAIN && node->num_child_ == 1);
	ObLogicalPlan* logical_plan = NULL;
	ObExplainStmt* explain_stmt = NULL;
	query_id = OB_INVALID_ID;


	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		explain_stmt = (ObExplainStmt*)parse_malloc(sizeof(ObExplainStmt), result_plan->name_pool_);
		if (explain_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObExplainStmt");
		}
		else
		{
			explain_stmt = new(explain_stmt) ObExplainStmt();
			query_id = logical_plan->generate_query_id();
			explain_stmt->set_query_id(query_id);
			ret = logical_plan->add_query(explain_stmt);
			if (ret != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObDeleteStmt to logical plan");
			}
			else
			{
				if (node->value_ > 0)
					explain_stmt->set_verbose(true);
				else
					explain_stmt->set_verbose(false);

				uint64_t sub_query_id = OB_INVALID_ID;
				switch (node->children_[0]->type_)
				{
				case T_SELECT:
					ret = resolve_select_stmt(result_plan, node->children_[0], sub_query_id);
					break;
				case T_DELETE:
					ret = resolve_delete_stmt(result_plan, node->children_[0], sub_query_id);
					break;
				case T_INSERT:
					ret = resolve_insert_stmt(result_plan, node->children_[0], sub_query_id);
					break;
				case T_UPDATE:
					ret = resolve_update_stmt(result_plan, node->children_[0], sub_query_id);
					break;
				default:
					ret = OB_ERR_PARSER_SYNTAX;
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Wrong statement in explain statement");
					break;
				}
				if (ret == OB_SUCCESS)
					explain_stmt->set_explain_query_id(sub_query_id);
			}
		}
	}
	return ret;
}

int resolve_column_definition(
		ResultPlan * result_plan,
		ObColumnDef& col_def,
		ParseNode* node,
		bool *is_primary_key)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node->type_ == T_COLUMN_DEFINITION);
	OB_ASSERT(node->num_child_ >= 3);
	if (is_primary_key)
		*is_primary_key = false;

	col_def.action_ = ADD_ACTION;
	OB_ASSERT(node->children_[0]->type_== T_IDENT);
	col_def.column_name_.assign_ptr(
			(char*)(node->children_[0]->str_value_),
			static_cast<int32_t>(strlen(node->children_[0]->str_value_))
	);

	ParseNode *type_node = node->children_[1];
	OB_ASSERT(type_node != NULL);
	switch(type_node->type_)
	{
	case T_TYPE_INTEGER:
		col_def.data_type_ = ObIntType;
		break;
	case T_TYPE_DECIMAL:
		col_def.data_type_ = ObDecimalType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.precision_ = type_node->children_[0]->value_;
		if (type_node->num_child_ >= 2 && type_node->children_[1] != NULL)
			col_def.scale_ = type_node->children_[1]->value_;
        //add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        /*
         *建表的时候要对用户输入的decimal的参数做正确性检查
         * report wrong info when input precision<scale
         * */
        if (col_def.precision_ <= col_def.scale_||col_def.precision_>MAX_DECIMAL_DIGIT||col_def.scale_>MAX_DECIMAL_SCALE||col_def.precision_<=0||type_node->num_child_==0)
        {
            ret = OB_ERR_WRONG_DYNAMIC_PARAM;
            snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                     "You have an error in your SQL syntax; check the param of decimal! precision = %ld, scale = %ld!",
                     col_def.precision_, col_def.scale_);
        }
        //add:e
		break;
	case T_TYPE_BOOLEAN:
		col_def.data_type_ = ObBoolType;
		break;
	case T_TYPE_FLOAT:
		col_def.data_type_ = ObFloatType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.precision_ = type_node->children_[0]->value_;
		break;
	case T_TYPE_DOUBLE:
		col_def.data_type_ = ObDoubleType;
		break;
	case T_TYPE_DATE:
		col_def.data_type_ = ObPreciseDateTimeType;
		break;
	case T_TYPE_TIME:
		col_def.data_type_ = ObPreciseDateTimeType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.precision_ = type_node->children_[0]->value_;
		break;
	case T_TYPE_TIMESTAMP:
		col_def.data_type_ = ObPreciseDateTimeType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.precision_ = type_node->children_[0]->value_;
		break;
	case T_TYPE_CHARACTER:
		col_def.data_type_ = ObVarcharType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.type_length_= type_node->children_[0]->value_;
		break;
	case T_TYPE_VARCHAR:
		col_def.data_type_ = ObVarcharType;
		if (type_node->num_child_ >= 1 && type_node->children_[0] != NULL)
			col_def.type_length_= type_node->children_[0]->value_;
		break;
	case T_TYPE_CREATETIME:
		col_def.data_type_ = ObCreateTimeType;
		break;
	case T_TYPE_MODIFYTIME:
		col_def.data_type_ = ObModifyTimeType;
		break;
	default:
		ret = OB_ERR_ILLEGAL_TYPE;
		snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
				"Unsupport data type of column definiton, column name = %s", node->children_[0]->str_value_);
		break;
	}

	ParseNode *attrs_node = node->children_[2];
	for(int32_t i = 0; ret == OB_SUCCESS && attrs_node && i < attrs_node->num_child_; i++)
	{
		ParseNode* attr_node = attrs_node->children_[i];
		switch(attr_node->type_)
		{
		case T_CONSTR_NOT_NULL:
			col_def.not_null_ = true;
			break;
		case T_CONSTR_NULL:
			col_def.not_null_ = false;
			break;
		case T_CONSTR_AUTO_INCREMENT:
			if (col_def.data_type_ != ObIntType && col_def.data_type_ != ObFloatType
					&& col_def.data_type_ != ObDoubleType && col_def.data_type_ != ObDecimalType)
			{
				ret = OB_ERR_PARSER_SYNTAX;
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Incorrect column specifier for column '%s'", node->children_[0]->str_value_);
				break;
			}
			col_def.atuo_increment_ = true;
			break;
		case T_CONSTR_PRIMARY_KEY:
			if (is_primary_key != NULL)
			{
				*is_primary_key = true;
			}
			break;
		case T_CONSTR_DEFAULT:
			ret = resolve_const_value(result_plan, attr_node, col_def.default_value_);
			break;
		default:  // won't be here
		ret = OB_ERR_PARSER_SYNTAX;
		snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
				"Wrong column constraint");
		break;
		}
		if (ret == OB_SUCCESS && col_def.default_value_.get_type() == ObNullType
				&& (col_def.not_null_ || col_def.primary_key_id_ > 0))
		{
			ret = OB_ERR_ILLEGAL_VALUE;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Invalid default value for '%s'", node->children_[0]->str_value_);
		}
	}
	return ret;
}

int resolve_const_value(
		ResultPlan * result_plan,
		ParseNode *def_node,
		ObObj& default_value)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	if (def_node != NULL)
	{
		ParseNode *def_val = def_node;
		if (def_node->type_ == T_CONSTR_DEFAULT)
			def_val = def_node->children_[0];
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		ObString str;
		ObObj val;
		switch (def_val->type_)
		{
		case T_INT:
			default_value.set_int(def_val->value_);
			break;
		case T_STRING:
		case T_BINARY:
			if ((ret = ob_write_string(*name_pool,
					ObString::make_string(def_val->str_value_),
					str)) != OB_SUCCESS)
			{
				PARSER_LOG("Can not malloc space for default value");
				break;
			}
			default_value.set_varchar(str);
			break;
		case T_DATE:
			default_value.set_precise_datetime(def_val->value_);
			break;
		case T_FLOAT:
			default_value.set_float(static_cast<float>(atof(def_val->str_value_)));
			break;
		case T_DOUBLE:
			default_value.set_double(atof(def_val->str_value_));
			break;
		case T_DECIMAL: // set as string
		if ((ret = ob_write_string(*name_pool,
				ObString::make_string(def_val->str_value_),
				str)) != OB_SUCCESS)
		{
			PARSER_LOG("Can not malloc space for default value");
			break;
		}
		default_value.set_varchar(str);
		default_value.set_type(ObDecimalType);
		break;
		case T_BOOL:
			default_value.set_bool(def_val->value_ == 1 ? true : false);
			break;
		case T_NULL:
			default_value.set_type(ObNullType);
			break;
		default:
			ret = OB_ERR_ILLEGAL_TYPE;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Illigeal type of default value");
			break;
		}
	}
	return ret;
}

int resolve_table_elements(
		ResultPlan * result_plan,
		ObCreateTableStmt& create_table_stmt,
		ParseNode* node)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node->type_ == T_TABLE_ELEMENT_LIST);
	OB_ASSERT(node->num_child_ >= 1);

	ParseNode *primary_node = NULL;
	for(int32_t i = 0; ret == OB_SUCCESS && i < node->num_child_; i++)
	{
		ParseNode* element = node->children_[i];
		if (OB_LIKELY(element->type_ == T_COLUMN_DEFINITION))
		{
			ObColumnDef col_def;
			bool is_primary_key = false;
			col_def.column_id_ = create_table_stmt.gen_column_id();
			if ((ret = resolve_column_definition(result_plan, col_def, element, &is_primary_key)) != OB_SUCCESS)
			{
				break;
			}
			else if (is_primary_key)
			{
				if (create_table_stmt.get_primary_key_size() > 0)
				{
					ret = OB_ERR_PRIMARY_KEY_DUPLICATE;
					PARSER_LOG("Multiple primary key defined");
					break;
				}
				else if ((ret = create_table_stmt.add_primary_key_part(col_def.column_id_)) != OB_SUCCESS)
				{
					PARSER_LOG("Add primary key failed");
					break;
				}
				else
				{
					col_def.primary_key_id_ = create_table_stmt.get_primary_key_size();
				}
			}
			ret = create_table_stmt.add_column_def(*result_plan, col_def);
		}
		else if (element->type_ == T_PRIMARY_KEY)
		{
			if (primary_node == NULL)
			{
				primary_node = element;
			}
			else
			{
				ret = OB_ERR_PRIMARY_KEY_DUPLICATE;
				PARSER_LOG("Multiple primary key defined");
			}
		}
		else
		{
			/* won't be here */
			OB_ASSERT(0);
		}
	}

	if (ret == OB_SUCCESS)
	{
		if (OB_UNLIKELY(create_table_stmt.get_primary_key_size() > 0 && primary_node != NULL))
		{
			ret = OB_ERR_PRIMARY_KEY_DUPLICATE;
			PARSER_LOG("Multiple primary key defined");
		}
		else if (primary_node != NULL)
		{
			ParseNode *key_node = NULL;
			for(int32_t i = 0; ret == OB_SUCCESS && i < primary_node->children_[0]->num_child_; i++)
			{
				key_node = primary_node->children_[0]->children_[i];
				ObString key_name;
				key_name.assign_ptr(
						(char*)(key_node->str_value_),
						static_cast<int32_t>(strlen(key_node->str_value_))
				);
				ret = create_table_stmt.add_primary_key_part(*result_plan, key_name);
			}
		}
	}

	return ret;
}

int resolve_create_table_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node && node->type_ == T_CREATE_TABLE && node->num_child_ == 4);
	ObLogicalPlan* logical_plan = NULL;
	ObCreateTableStmt* create_table_stmt = NULL;
	query_id = OB_INVALID_ID;


	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		create_table_stmt = (ObCreateTableStmt*)parse_malloc(sizeof(ObCreateTableStmt), result_plan->name_pool_);
		if (create_table_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObExplainStmt");
		}
		else
		{
			create_table_stmt = new(create_table_stmt) ObCreateTableStmt(name_pool);
			query_id = logical_plan->generate_query_id();
			create_table_stmt->set_query_id(query_id);
			ret = logical_plan->add_query(create_table_stmt);
			if (ret != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObCreateTableStmt to logical plan");
			}
		}
	}

	if (ret == OB_SUCCESS)
	{
		if (node->children_[0] != NULL)
		{
			OB_ASSERT(node->children_[0]->type_ == T_IF_NOT_EXISTS);
			create_table_stmt->set_if_not_exists(true);
		}
		OB_ASSERT(node->children_[1]->type_ == T_IDENT);
		ObString table_name;
		table_name.assign_ptr(
				(char*)(node->children_[1]->str_value_),
				static_cast<int32_t>(strlen(node->children_[1]->str_value_))
		);
		if ((ret = create_table_stmt->set_table_name(*result_plan, table_name)) != OB_SUCCESS)
		{
			//snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
			//    "Add table name to ObCreateTableStmt failed");
		}
	}

	if (ret == OB_SUCCESS)
	{
		OB_ASSERT(node->children_[2]->type_ == T_TABLE_ELEMENT_LIST);
		ret = resolve_table_elements(result_plan, *create_table_stmt, node->children_[2]);
	}

	if (ret == OB_SUCCESS && node->children_[3])
	{
		OB_ASSERT(node->children_[3]->type_ == T_TABLE_OPTION_LIST);
		ObString str;
		ParseNode *option_node = NULL;
		int32_t num = node->children_[3]->num_child_;
		for (int32_t i = 0; ret == OB_SUCCESS && i < num; i++)
		{
			option_node = node->children_[3]->children_[i];
			switch (option_node->type_)
			{
			case T_JOIN_INFO:
				str.assign_ptr(
						const_cast<char*>(option_node->children_[0]->str_value_),
						static_cast<int32_t>(option_node->children_[0]->value_));
				if ((ret = create_table_stmt->set_join_info(str)) != OB_SUCCESS)
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Set JOIN_INFO failed");
				break;
			case T_EXPIRE_INFO:
				str.assign_ptr(
						(char*)(option_node->children_[0]->str_value_),
						static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
				);
				if ((ret = create_table_stmt->set_expire_info(str)) != OB_SUCCESS)
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Set EXPIRE_INFO failed");
				break;
			case T_TABLET_MAX_SIZE:
				create_table_stmt->set_tablet_max_size(option_node->children_[0]->value_);
				break;
			case T_TABLET_BLOCK_SIZE:
				create_table_stmt->set_tablet_block_size(option_node->children_[0]->value_);
				break;
			case T_TABLET_ID:
				create_table_stmt->set_table_id(
						*result_plan,
						static_cast<uint64_t>(option_node->children_[0]->value_)
				);
				break;
			case T_REPLICA_NUM:
				create_table_stmt->set_replica_num(static_cast<int32_t>(option_node->children_[0]->value_));
				break;
			case T_COMPRESS_METHOD:
				str.assign_ptr(
						(char*)(option_node->children_[0]->str_value_),
						static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
				);
				if ((ret = create_table_stmt->set_compress_method(str)) != OB_SUCCESS)
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Set COMPRESS_METHOD failed");
				break;
			case T_USE_BLOOM_FILTER:
				create_table_stmt->set_use_bloom_filter(option_node->children_[0]->value_ ? true : false);
				break;
			case T_CONSISTENT_MODE:
				create_table_stmt->set_consistency_level(option_node->value_);
				break;
			case T_COMMENT:
				str.assign_ptr(
						(char*)(option_node->children_[0]->str_value_),
						static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
				);
				if ((ret = create_table_stmt->set_comment_str(str)) != OB_SUCCESS)
					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
							"Set COMMENT failed");
				break;
			default:
				/* won't be here */
				OB_ASSERT(0);
				break;
			}
		}
	}
	return ret;
}

int resolve_alter_table_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_ALTER_TABLE && node->num_child_ == 2);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObAlterTableStmt* alter_table_stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, alter_table_stmt)))
	{
	}
	else
	{
		alter_table_stmt->set_name_pool(static_cast<ObStringBuf*>(result_plan->name_pool_));
		OB_ASSERT(node->children_[0]);
		OB_ASSERT(node->children_[1] && node->children_[1]->type_ == T_ALTER_ACTION_LIST);
		int32_t name_len= static_cast<int32_t>(strlen(node->children_[0]->str_value_));
		ObString table_name(name_len, name_len, node->children_[0]->str_value_);
		if ((ret = alter_table_stmt->init()) != OB_SUCCESS)
		{
			PARSER_LOG("Init alter table stmt failed, ret=%d", ret);
		}
		else if ((ret = alter_table_stmt->set_table_name(*result_plan, table_name)) == OB_SUCCESS)
		{
			for (int32_t i = 0; ret == OB_SUCCESS && i < node->children_[1]->num_child_; i++)
			{
				ParseNode *action_node = node->children_[1]->children_[i];
				if (action_node == NULL)
					continue;
				ObColumnDef col_def;
				switch (action_node->type_)
				{
				case T_TABLE_RENAME:
				{
					int32_t len = static_cast<int32_t>(strlen(action_node->children_[0]->str_value_));
					ObString new_name(len, len, action_node->children_[0]->str_value_);
					ret = alter_table_stmt->set_new_table_name(*result_plan, new_name);
					break;
				}
				case T_COLUMN_DEFINITION:
				{
					bool is_primary_key = false;
					if ((ret = resolve_column_definition(
							result_plan,
							col_def,
							action_node,
							&is_primary_key)) != OB_SUCCESS)
					{
					}
					else if (is_primary_key)
					{
						ret = OB_ERR_MODIFY_PRIMARY_KEY;
						PARSER_LOG("New added column can not be primary key");
					}
					else
					{
						ret = alter_table_stmt->add_column(*result_plan, col_def);
					}
					break;
				}
				case T_COLUMN_DROP:
				{
					int32_t len = static_cast<int32_t>(strlen(action_node->children_[0]->str_value_));
					ObString table_name(len, len, action_node->children_[0]->str_value_);
					col_def.action_ = DROP_ACTION;
					col_def.column_name_ = table_name;
					switch (action_node->value_)
					{
					case 0:
						col_def.drop_behavior_ = NONE_BEHAVIOR;
						break;
					case 1:
						col_def.drop_behavior_ = RESTRICT_BEHAVIOR;
						break;
					case 2:
						col_def.drop_behavior_ = CASCADE_BEHAVIOR;
						break;
					default:
						break;
					}
					ret = alter_table_stmt->drop_column(*result_plan, col_def);
					break;
				}
				case T_COLUMN_ALTER:
				{
					int32_t table_len = static_cast<int32_t>(strlen(action_node->children_[0]->str_value_));
					ObString table_name(table_len, table_len, action_node->children_[0]->str_value_);
					col_def.action_ = ALTER_ACTION;
					col_def.column_name_ = table_name;
					OB_ASSERT(action_node->children_[1]);
					switch (action_node->children_[1]->type_)
					{
					case T_CONSTR_NOT_NULL:
						col_def.not_null_ = true;
						break;
					case T_CONSTR_NULL:
						col_def.not_null_ = false;
						break;
					case T_CONSTR_DEFAULT:
						ret = resolve_const_value(result_plan, action_node->children_[1], col_def.default_value_);
						break;
					default:
						/* won't be here */
						ret = OB_ERR_RESOLVE_SQL;
						PARSER_LOG("Unkown alter table alter column action type, type=%d",
								action_node->children_[1]->type_);
						break;
					}
					ret = alter_table_stmt->alter_column(*result_plan, col_def);
					break;
				}
				case T_COLUMN_RENAME:
				{
					int32_t table_len = static_cast<int32_t>(strlen(action_node->children_[0]->str_value_));
					ObString table_name(table_len, table_len, action_node->children_[0]->str_value_);
					int32_t new_len = static_cast<int32_t>(strlen(action_node->children_[1]->str_value_));
					ObString new_name(new_len, new_len, action_node->children_[1]->str_value_);
					col_def.action_ = RENAME_ACTION;
					col_def.column_name_ = table_name;
					col_def.new_column_name_ = new_name;
					ret = alter_table_stmt->rename_column(*result_plan, col_def);
					break;
				}
				default:
					/* won't be here */
					ret = OB_ERR_RESOLVE_SQL;
					PARSER_LOG("Unkown alter table action type, type=%d", action_node->type_);
					break;
				}
			}
		}
	}
	return ret;
}

int resolve_drop_table_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	OB_ASSERT(node && node->type_ == T_DROP_TABLE && node->num_child_ == 2);
	ObLogicalPlan* logical_plan = NULL;
	ObDropTableStmt* drp_tab_stmt = NULL;
	query_id = OB_INVALID_ID;


	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		drp_tab_stmt = (ObDropTableStmt*)parse_malloc(sizeof(ObDropTableStmt), result_plan->name_pool_);
		if (drp_tab_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObDropTableStmt");
		}
		else
		{
			drp_tab_stmt = new(drp_tab_stmt) ObDropTableStmt(name_pool);
			query_id = logical_plan->generate_query_id();
			drp_tab_stmt->set_query_id(query_id);
			if ((ret = logical_plan->add_query(drp_tab_stmt)) != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObDropTableStmt to logical plan");
			}
		}
	}

	if (ret == OB_SUCCESS && node->children_[0])
	{
		drp_tab_stmt->set_if_exists(true);
	}
	if (ret == OB_SUCCESS)
	{
		OB_ASSERT(node->children_[1] && node->children_[1]->num_child_ > 0);
		ParseNode *table_node = NULL;
		ObString table_name;
		for (int32_t i = 0; i < node->children_[1]->num_child_; i ++)
		{
			table_node = node->children_[1]->children_[i];
			table_name.assign_ptr(
					(char*)(table_node->str_value_),
					static_cast<int32_t>(strlen(table_node->str_value_))
			);
			if (OB_SUCCESS != (ret = drp_tab_stmt->add_table_name_id(*result_plan, table_name)))
			{
				break;
			}
		}
	}
	return ret;
}
//add hxlong [Truncate Table]:20170403:b
int resolve_truncate_table_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  OB_ASSERT(node && node->type_ == T_TRUNCATE_TABLE && node->num_child_ == 3);
  ObLogicalPlan* logical_plan = NULL;
  ObTruncateTableStmt* trun_tab_stmt = NULL;
  query_id = OB_INVALID_ID;


  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
  if (result_plan->plan_tree_ == NULL)
  {
    logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
    if (logical_plan == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not malloc ObLogicalPlan");
    }
    else
    {
      logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
      result_plan->plan_tree_ = logical_plan;
    }
  }
  else
  {
    logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
  }

  if (ret == OB_SUCCESS)
  {
    trun_tab_stmt = (ObTruncateTableStmt*)parse_malloc(sizeof(ObTruncateTableStmt), result_plan->name_pool_);
    if (trun_tab_stmt == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not malloc ObTruncateTableStmt");
    }
    else
    {
      trun_tab_stmt = new(trun_tab_stmt) ObTruncateTableStmt(name_pool);
      query_id = logical_plan->generate_query_id();
      trun_tab_stmt->set_query_id(query_id);
      if ((ret = logical_plan->add_query(trun_tab_stmt)) != OB_SUCCESS)
      {
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not add ObDropTableStmt to logical plan");
      }
    }
  }
  if (ret == OB_SUCCESS && node->children_[0])
  {
    trun_tab_stmt->set_if_exists(true);
  }
  if (ret == OB_SUCCESS)
  {
      OB_ASSERT(node->children_[1] && node->children_[1]->num_child_ > 0);
      ParseNode *table_node = NULL;
      ObString table_name;
      for (int32_t i = 0; i < node->children_[1]->num_child_; i ++)
      {
          table_node = node->children_[1]->children_[i];
          table_name.assign_ptr(
                  (char*)(table_node->str_value_),
                  static_cast<int32_t>(strlen(table_node->str_value_))
          );
          if (OB_SUCCESS != (ret = trun_tab_stmt->add_table_name_id(*result_plan, table_name)))
          {
              break;
          }
      }
  }
  return ret;
  }
//add:e
int resolve_create_index_stmt(ResultPlan* result_plan, ParseNode* node, uint64_t& query_id)
{
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  OB_ASSERT(node && node->type_ == T_CREATE_INDEX && node->num_child_ == 6);
  ObLogicalPlan* logical_plan = NULL;
  ObCreateIndexStmt* create_index_stmt = NULL;
  query_id = OB_INVALID_ID;
  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
  ObString original_table_name_;
  ObString index_table_name_;
  uint64_t original_table_id_ = OB_INVALID_ID;
  if (result_plan->plan_tree_ == NULL)
  {
    logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
    if (logical_plan == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "Can not malloc ObLogicalPlan");
    }
    else
    {
      logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
      result_plan->plan_tree_ = logical_plan;
    }
  }
  else
  {
    logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
  }
  if (ret == OB_SUCCESS)
  {
    create_index_stmt = (ObCreateIndexStmt*)parse_malloc(sizeof(ObCreateIndexStmt), result_plan->name_pool_);
    if (create_index_stmt == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "Can not malloc ObCreateIndexStmt");
    }
    else
    {
      create_index_stmt = new(create_index_stmt) ObCreateIndexStmt(name_pool);
      query_id = logical_plan->generate_query_id();
      create_index_stmt->set_query_id(query_id);
      ret = logical_plan->add_query(create_index_stmt);
      if (ret != OB_SUCCESS)
      {
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                 "Can not add ObCreateIndexStmt to logical plan");
      }
    }
  }
  if (ret == OB_SUCCESS)
  {
    // check (if not exists)
    if (node->children_[0] != NULL)
    {
      OB_ASSERT(node->children_[0]->type_ == T_IF_NOT_EXISTS);
      create_index_stmt->set_if_not_exists(true);
    }
  }
  if (ret == OB_SUCCESS)
  {
    OB_ASSERT(node->children_[1]->type_ == T_IDENT);
    original_table_name_.assign_ptr(
          (char*)(node->children_[1]->str_value_),
        static_cast<int32_t>(strlen(node->children_[1]->str_value_))
        );
    if ((ret = create_index_stmt->set_original_table_name(*result_plan, original_table_name_)) != OB_SUCCESS)
    {
      //snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
      //    "Add table name to ObCreateTableStmt failed");
    }
    else
    {
      TBSYS_LOG(WARN,"original_table_name_ = %s",original_table_name_.ptr());
      ObSchemaChecker* schema_checker = NULL;
      schema_checker = static_cast<ObSchemaChecker*>(result_plan->schema_checker_);
      if (NULL == schema_checker)
      {
        ret = OB_ERR_SCHEMA_UNSET;
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                 "Schema(s) are not set");
      }
      if (OB_SUCCESS == ret)
      {
        if((original_table_id_ = schema_checker->get_table_id(original_table_name_)) == OB_INVALID_ID)
        {
          ret = OB_ENTRY_NOT_EXIST;
          snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                   "the table to create index '%.*s' is not exist", original_table_name_.length(), original_table_name_.ptr());
        }
      }
    }
  }
  if (ret == OB_SUCCESS)
  {
    OB_ASSERT(node->children_[3] != NULL);
    OB_ASSERT(node->children_[2]->type_ == T_IDENT);
    char str[OB_MAX_TABLE_NAME_LENGTH];
    memset(str,0,OB_MAX_TABLE_NAME_LENGTH);
    int64_t str_len = 0;
	int max_index_name = OB_MAX_COLUMN_NAME_LENGTH-4;
    index_table_name_.assign_ptr(
          (char*)(node->children_[2]->str_value_),
        static_cast<int32_t>(strlen(node->children_[2]->str_value_))
        );
		 //add zhuyanchao[secondary index bug]
    if(index_table_name_.length()>=max_index_name)
    {
        ret = OB_ERR_INVALID_INDEX_NAME;
        return ret;
    }
    if((ret = create_index_stmt->generate_inner_index_table_name(index_table_name_, original_table_name_, str, str_len)) != OB_SUCCESS)
    {
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "failed to generate index table name!");
    }
    else
    {
      index_table_name_.reset();
      index_table_name_.assign_ptr(str,static_cast<int32_t>(str_len));
      TBSYS_LOG(WARN,"inner_index_table_name = %s",index_table_name_.ptr());
    }
    if((ret = create_index_stmt->set_table_name(*result_plan, index_table_name_)) != OB_SUCCESS)
    {
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "failed to set index table name!");
    }
  }
  if (ret == OB_SUCCESS)
  {
    OB_ASSERT(node->children_[3]->type_ == T_COLUMN_LIST);
    ParseNode *column_node = NULL;
    ObString index_column;
    for(int32_t i = 0; i < node->children_[3]->num_child_; i ++)
    {
      column_node = node->children_[3]->children_[i];
      index_column.reset();
      index_column.assign_ptr(
            (char*)(column_node->str_value_),
            static_cast<int32_t>(strlen(column_node->str_value_))
            );
      if(OB_SUCCESS != (ret = create_index_stmt->set_index_columns(*result_plan, original_table_name_, index_column)))
      {
        //mod longfei 151201
        //        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
        //        						"failed to add_index_colums!");
        TBSYS_LOG(ERROR, "failed to set_index_columns!");
        //mod e
        break;
      }
    }
  }
  if (ret == OB_SUCCESS)
  {
    ObString storing_column; // ������
    ParseNode *column_node = NULL;
    if(NULL == node->children_[4])
    {
      create_index_stmt->set_has_storing(false);
    }
    else
    {
      for(int32_t i = 0; i < node->children_[4]->num_child_; i++)
      {
        column_node = node->children_[4]->children_[i];
        storing_column.reset();
        storing_column.assign_ptr(
              (char*)(column_node->str_value_),
              static_cast<int32_t>(strlen(column_node->str_value_))
              );
        if(OB_SUCCESS != (ret = create_index_stmt->set_storing_columns(*result_plan, original_table_name_, storing_column)))
        {
          //mod longfei 151201
          //					snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          //							"failed to add_index_colums!");
          TBSYS_LOG(WARN, "failed to set_storing_columns!");
          //mod e
          break;
        }
      }
    }
  }
  if (ret == OB_SUCCESS && node->children_[5])
  {
    OB_ASSERT(node->children_[5]->type_ == T_INDEX_OPTION_LIST);
    ObString str;
    ParseNode *option_node = NULL;
    int32_t num = node->children_[5]->num_child_;
    if(NULL == node->children_[5])
    {
      create_index_stmt->set_has_option_list(false);
    }
    else
    {
      for (int32_t i = 0; ret == OB_SUCCESS && i < num; i++)
      {
        option_node = node->children_[5]->children_[i];
        switch (option_node->type_)
        {
          case T_JOIN_INFO:
            str.assign_ptr(
                  const_cast<char*>(option_node->children_[0]->str_value_),
                static_cast<int32_t>(option_node->children_[0]->value_));
            if ((ret = create_index_stmt->set_join_info(str)) != OB_SUCCESS)
              snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                       "Set JOIN_INFO failed");
            break;
          case T_EXPIRE_INFO:
            str.assign_ptr(
                  (char*)(option_node->children_[0]->str_value_),
                static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
                );
            if ((ret = create_index_stmt->set_expire_info(str)) != OB_SUCCESS)
              snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                       "Set EXPIRE_INFO failed");
            break;
          case T_TABLET_MAX_SIZE:
            create_index_stmt->set_tablet_max_size(option_node->children_[0]->value_);
            break;
          case T_TABLET_BLOCK_SIZE:
            create_index_stmt->set_tablet_block_size(option_node->children_[0]->value_);
            break;
          case T_TABLET_ID:
            create_index_stmt->set_table_id(
                  *result_plan,
                  static_cast<uint64_t>(option_node->children_[0]->value_)
                );
            break;
          case T_REPLICA_NUM:
            create_index_stmt->set_replica_num(static_cast<int32_t>(option_node->children_[0]->value_));
            break;
          case T_COMPRESS_METHOD:
            str.assign_ptr(
                  (char*)(option_node->children_[0]->str_value_),
                static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
                );
            if ((ret = create_index_stmt->set_compress_method(str)) != OB_SUCCESS)
              snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                       "Set COMPRESS_METHOD failed");
            break;
          case T_USE_BLOOM_FILTER:
            create_index_stmt->set_use_bloom_filter(option_node->children_[0]->value_ ? true : false);
            break;
          case T_CONSISTENT_MODE:
            create_index_stmt->set_consistency_level(option_node->value_);
            break;
          case T_COMMENT:
            str.assign_ptr(
                  (char*)(option_node->children_[0]->str_value_),
                static_cast<int32_t>(strlen(option_node->children_[0]->str_value_))
                );
            if ((ret = create_index_stmt->set_comment_str(str)) != OB_SUCCESS)
              snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                       "Set COMMENT failed");
            break;
          default:
            /* won't be here */
            OB_ASSERT(0);
            break;
        }
      }
    }
  }
  return ret;
}

//add longfei [drop index] 20151026
int resolve_drop_index_stmt(ResultPlan *result_plan, ParseNode *node, uint64_t &query_id)
{
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  OB_ASSERT(node && node->type_ == T_DROP_INDEX && node->num_child_ == 3);
  ObLogicalPlan* logical_plan = NULL;
  ObDropIndexStmt* drp_idx_stmt = NULL;
  query_id = OB_INVALID_ID;
  uint64_t data_table_id = OB_INVALID_ID;
  ObSchemaChecker* schema_checker=NULL;
  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
  char tname_str[OB_MAX_TABLE_NAME_LENGTH];
  int64_t str_len=0;
  if (NULL == result_plan->plan_tree_)
  {
    logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
    if (NULL == logical_plan)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "Can not malloc ObLogicalPlan");
    }
    else
    {
      logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
      result_plan->plan_tree_ = logical_plan;
    }
  }
  else
  {
    logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
  }
  if (OB_SUCCESS == ret)
  {
    drp_idx_stmt = (ObDropIndexStmt*)parse_malloc(sizeof(ObDropIndexStmt), result_plan->name_pool_);
    if (NULL == drp_idx_stmt)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "Can not malloc ObDropIndexStmt");
    }
    else
    {
      drp_idx_stmt = new(drp_idx_stmt) ObDropIndexStmt(name_pool);
      query_id = logical_plan->generate_query_id();
      drp_idx_stmt->set_query_id(query_id);
      if (OB_SUCCESS != (ret = logical_plan->add_query(drp_idx_stmt)))
      {
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                 "Can not add ObDropIndexStmt to logical plan");
      }
    }
  }
  if (ret == OB_SUCCESS && node->children_[0])
  {
    drp_idx_stmt->set_if_exists(true);
  }
  if(OB_SUCCESS == ret)
  {
    OB_ASSERT(node->children_[2]);
    ParseNode *table_node = NULL;
    ObString table_name;
    table_node =node->children_[2];
    table_name.assign_ptr(
          (char*)(table_node->str_value_),
          static_cast<int32_t>(strlen(table_node->str_value_))
          );
    schema_checker = static_cast<ObSchemaChecker*>(result_plan->schema_checker_);
    if (schema_checker == NULL)
    {
      ret = OB_ERR_SCHEMA_UNSET;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "Schema(s) are not set");
    }
    else if((data_table_id=schema_checker->get_table_id(table_name))==OB_INVALID_ID)
    {
      ret = OB_ERR_TABLE_UNKNOWN;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
               "table '%.*s' does not exist", table_name.length(), table_name.ptr());
    }
    drp_idx_stmt->setOriTabName(table_name);
  }
  if (OB_SUCCESS == ret)
  {
    ParseNode *table_node = NULL;
    ObString table_name;
    ObString ori_tab_name;
    ObString table_lable;
    if (node->children_[1])
    {
      for (int32_t i = 0; i < node->children_[1]->num_child_; i++)
      {
        table_node = node->children_[1]->children_[i];
        table_name.assign_ptr(
              (char*)(table_node->str_value_),
              static_cast<int32_t>(strlen(table_node->str_value_))
              );
        //add longfei [secondary index ecnu_opencode] 20150822:b
        table_lable.assign_ptr(
              (char*)(node->children_[2]->str_value_),
            static_cast<int32_t>(strlen(node->children_[2]->str_value_))
            );
        // add 20150822:e
        //generate index name here
        memset(tname_str,0,OB_MAX_TABLE_NAME_LENGTH);
        // mod longfei 20150822:b
        //if(OB_SUCCESS != (ret = generate_index_table_name(table_name,data_table_id,tname_str,str_len)))
        if(OB_SUCCESS != (ret = drp_idx_stmt->generate_inner_index_table_name(table_name, table_lable, tname_str, str_len)))
          //mod :e
        {
          snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                   "generate index name failed '%.*s'", table_name.length(), table_name.ptr());
        }
        else
        {
          table_name.reset();
          table_name.assign_ptr(tname_str,(int32_t)str_len);
        }
        if (OB_SUCCESS == ret && OB_SUCCESS != (ret = drp_idx_stmt->add_table_name_id(*result_plan, table_name)))
        {
          //mod huangjianwei [secondary index debug] 20140314:b
          //snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
                   //"failed to do add_table_name_id");
          break;
          //mod:e
        }
      }
    }
    else
    {
      drp_idx_stmt->setDrpAll(true);
    }
  }
  return ret;
}
//add e


int resolve_show_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	uint64_t  sys_table_id = OB_INVALID_ID;
	ParseNode *show_table_node = NULL;
	ParseNode *condition_node = NULL;
	OB_ASSERT(node && node->type_ >= T_SHOW_TABLES && node->type_ <= T_SHOW_PROCESSLIST);
	query_id = OB_INVALID_ID;

	ObLogicalPlan* logical_plan = NULL;
	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		ObShowStmt* show_stmt = (ObShowStmt*)parse_malloc(sizeof(ObShowStmt), result_plan->name_pool_);
		if (show_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObShowStmt");
		}
		else
		{
			ParseNode sys_table_name;
			sys_table_name.type_ = T_IDENT;
			switch (node->type_)
			{
			case T_SHOW_TABLES:
				OB_ASSERT(node->num_child_ == 1);
				condition_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_TABLES);
				sys_table_name.str_value_ = OB_TABLES_SHOW_TABLE_NAME;
				break;
      case T_SHOW_INDEX:
        OB_ASSERT(node->num_child_ == 2);
        show_table_node = node->children_[0];
        condition_node = node->children_[1];
        show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_INDEX);
        sys_table_name.str_value_ = OB_INDEX_SHOW_TABLE_NAME;
        break;
			case T_SHOW_VARIABLES:
				OB_ASSERT(node->num_child_ == 1);
				condition_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_VARIABLES);
				show_stmt->set_global_scope(node->value_ == 1 ? true : false);
				sys_table_name.str_value_ = OB_VARIABLES_SHOW_TABLE_NAME;
				break;
			case T_SHOW_COLUMNS:
				OB_ASSERT(node->num_child_ == 2);
				show_table_node = node->children_[0];
				condition_node = node->children_[1];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_COLUMNS);
				sys_table_name.str_value_ = OB_COLUMNS_SHOW_TABLE_NAME;
				break;
			case T_SHOW_SCHEMA:
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_SCHEMA);
				sys_table_name.str_value_ = OB_SCHEMA_SHOW_TABLE_NAME;
				break;
			case T_SHOW_CREATE_TABLE:
				OB_ASSERT(node->num_child_ == 1);
				show_table_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_CREATE_TABLE);
				sys_table_name.str_value_ = OB_CREATE_TABLE_SHOW_TABLE_NAME;
				break;
			case T_SHOW_TABLE_STATUS:
				OB_ASSERT(node->num_child_ == 1);
				condition_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_TABLE_STATUS);
				sys_table_name.str_value_ = OB_TABLE_STATUS_SHOW_TABLE_NAME;
				break;
			case T_SHOW_SERVER_STATUS:
				OB_ASSERT(node->num_child_ == 1);
				condition_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_SERVER_STATUS);
				sys_table_name.str_value_ = OB_SERVER_STATUS_SHOW_TABLE_NAME;
				break;
			case T_SHOW_WARNINGS:
				OB_ASSERT(node->num_child_ == 0 || node->num_child_ == 1);
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_WARNINGS);
				break;
			case T_SHOW_GRANTS:
				OB_ASSERT(node->num_child_ == 1);
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_GRANTS);
				break;
			case T_SHOW_PARAMETERS:
				OB_ASSERT(node->num_child_ == 1);
				condition_node = node->children_[0];
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_PARAMETERS);
				sys_table_name.str_value_ = OB_PARAMETERS_SHOW_TABLE_NAME;
				break;
			case T_SHOW_PROCESSLIST:
				show_stmt = new(show_stmt) ObShowStmt(name_pool, ObBasicStmt::T_SHOW_PROCESSLIST);
				show_stmt->set_full_process(node->value_ == 1? true: false);
				show_stmt->set_show_table(OB_ALL_SERVER_SESSION_TID);
				break;
			default:
				/* won't be here */
				break;
			}
			if (node->type_ >= T_SHOW_TABLES && node->type_ <= T_SHOW_SERVER_STATUS
					&& (ret = resolve_table(result_plan, show_stmt, &sys_table_name, sys_table_id)) == OB_SUCCESS)
			{
				show_stmt->set_sys_table(sys_table_id);
				query_id = logical_plan->generate_query_id();
				show_stmt->set_query_id(query_id);
			}
			if (ret == OB_SUCCESS && (ret = logical_plan->add_query(show_stmt)) != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObShowStmt to logical plan");
			}
			if (ret != OB_SUCCESS && show_stmt != NULL)
			{
				show_stmt->~ObShowStmt();
			}
		}

		if (ret == OB_SUCCESS && sys_table_id != OB_INVALID_ID)
		{
			TableItem *table_item = show_stmt->get_table_item_by_id(sys_table_id);
			ret = resolve_table_columns(result_plan, show_stmt, *table_item);
		}

		// mod longfei [show index] 20151019 :b
		//if (ret == OB_SUCCESS && (node->type_ == T_SHOW_COLUMNS || node->type_ == T_SHOW_CREATE_TABLE))
		if (ret == OB_SUCCESS && (node->type_ == T_SHOW_INDEX || node->type_ == T_SHOW_COLUMNS || node->type_ == T_SHOW_CREATE_TABLE))
		// mod e
		{
			OB_ASSERT(show_table_node);
			ObSchemaChecker *schema_checker = static_cast<ObSchemaChecker*>(result_plan->schema_checker_);
			if (schema_checker == NULL)
			{
				ret = OB_ERR_SCHEMA_UNSET;
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG, "Schema(s) are not set");
			}
			int32_t len = static_cast<int32_t>(strlen(show_table_node->str_value_));
			ObString table_name(len, len, show_table_node->str_value_);
			uint64_t show_table_id = schema_checker->get_table_id(table_name);
			//TBSYS_LOG(INFO,"longfei:table name = %s;table id = %d", table_name.ptr(), static_cast<int>(show_table_id));
			if (show_table_id == OB_INVALID_ID)
			{
				ret = OB_ERR_TABLE_UNKNOWN;
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Unknown table \"%s\"", show_table_node->str_value_);
			}
			else
			{
				show_stmt->set_show_table(show_table_id);
			}
		}

		// mod longfei [show index] 2015109 :b
		// add node->type_ == T_SHOW_INDEX into if
		if (ret == OB_SUCCESS && condition_node
				&& (node->type_ == T_SHOW_TABLES || node->type_ == T_SHOW_INDEX || node->type_ == T_SHOW_VARIABLES || node->type_ == T_SHOW_COLUMNS
						|| node->type_ == T_SHOW_TABLE_STATUS || node->type_ == T_SHOW_SERVER_STATUS
						|| node->type_ == T_SHOW_PARAMETERS))
		// mod e
		{
			if (condition_node->type_ == T_OP_LIKE && condition_node->num_child_ == 1)
			{
				OB_ASSERT(condition_node->children_[0]->type_ == T_STRING);
				ObString  like_pattern;
				like_pattern.assign_ptr(
						(char*)(condition_node->children_[0]->str_value_),
						static_cast<int32_t>(strlen(condition_node->children_[0]->str_value_))
				);
				ret = show_stmt->set_like_pattern(like_pattern);
			}
			else
			{
				ret = resolve_and_exprs(
						result_plan,
						show_stmt,
						condition_node->children_[0],
						show_stmt->get_where_exprs(),
						T_WHERE_LIMIT
				);
			}
		}

		if (ret == OB_SUCCESS && node->type_ == T_SHOW_WARNINGS)
		{
			show_stmt->set_count_warnings(node->value_ == 1 ? true : false);
			if (node->num_child_ == 1 && node->children_[0] != NULL)
			{
				ParseNode *limit = node->children_[0];
				OB_ASSERT(limit->num_child_ == 2);
				int64_t offset = limit->children_[0] == NULL ? 0 : limit->children_[0]->value_;
				int64_t count = limit->children_[1] == NULL ? -1 : limit->children_[1]->value_;
				show_stmt->set_warnings_limit(offset, count);
			}
		}

		if (ret == OB_SUCCESS && node->type_ == T_SHOW_GRANTS)
		{
			if (node->children_[0] != NULL)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Can not malloc space for user name");
				}
				else
				{
					show_stmt->set_user_name(name);
				}
			}
		}
	}
	return ret;
}

int resolve_prepare_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_PREPARE && node->num_child_ == 2);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObPrepareStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if (ret == OB_SUCCESS)
		{
			OB_ASSERT(node->children_[0]);
			ObString name;
			if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
			{
				PARSER_LOG("Can not malloc space for stmt name");
			}
			else
			{
				stmt->set_stmt_name(name);
			}
		}
		if (ret == OB_SUCCESS)
		{
			uint64_t sub_query_id = OB_INVALID_ID;
			switch (node->children_[1]->type_)
			{
			case T_SELECT:
				ret = resolve_select_stmt(result_plan, node->children_[1], sub_query_id);
				break;
			case T_DELETE:
				ret = resolve_delete_stmt(result_plan, node->children_[1], sub_query_id);
				break;
			case T_INSERT:
				ret = resolve_insert_stmt(result_plan, node->children_[1], sub_query_id);
				break;
			case T_UPDATE:
				ret = resolve_update_stmt(result_plan, node->children_[1], sub_query_id);
				break;
			default:
				ret = OB_ERR_PARSER_SYNTAX;
				PARSER_LOG("Wrong statement type in prepare statement");
				break;
			}
			if (ret == OB_SUCCESS)
				stmt->set_prepare_query_id(sub_query_id);
		}
	}
	return ret;
}

int resolve_variable_set_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_VARIABLE_SET);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObVariableSetStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		ParseNode* set_node = NULL;
		ObVariableSetStmt::VariableSetNode var_node;
		for (int32_t i = 0; ret == OB_SUCCESS && i < node->num_child_; i++)
		{
			set_node = node->children_[i];
			OB_ASSERT(set_node->type_ == T_VAR_VAL);
			switch (set_node->value_)
			{
			case 1:
				var_node.scope_type_ = ObVariableSetStmt::GLOBAL;
				break;
			case 2:
				var_node.scope_type_ = ObVariableSetStmt::SESSION;
				break;
			case 3:
				var_node.scope_type_ = ObVariableSetStmt::LOCAL;
				break;
			default:
				var_node.scope_type_ = ObVariableSetStmt::NONE_SCOPE;
				break;
			}

      ParseNode* var = set_node->children_[0];
      OB_ASSERT(var);
      var_node.is_system_variable_ = (var->type_ == T_SYSTEM_VARIABLE) ? true : false;

      //add zt 20151208:b
      //TODO
      if( T_ARRAY == var->type_ )
      {
        TBSYS_LOG(WARN, "does not support array item as left value now");
        ret = OB_NOT_SUPPORTED;
      }
      else
      //add zt 20151208:e
      if ((ret = ob_write_string(*name_pool, ObString::make_string(var->str_value_),
                                  var_node.variable_name_)) != OB_SUCCESS)
      {
        PARSER_LOG("Can not malloc space for variable name");
        break;
      }

			OB_ASSERT(node->children_[1]);
			if ((ret = resolve_independ_expr(result_plan, NULL, set_node->children_[1], var_node.value_expr_id_,
					T_VARIABLE_VALUE_LIMIT)) != OB_SUCCESS)
			{
				//PARSER_LOG("Resolve set value error");
				break;
			}

			if ((ret = stmt->add_variable_node(var_node)) != OB_SUCCESS)
			{
				PARSER_LOG("Add set entry failed");
				break;
			}
		}
	}
	return ret;
}

//add zt 20151202:b
/**
 * @brief resolve_variable_set_array_stmt
 * parse procedure's variable_set_array statement syntax tree and create ObVariableSetArrayValueStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of variable_set_array statement syntax tree
 * @param query_id is variable_set_array_stmt's id
 * @return error code
 */
int resolve_variable_set_array_stmt(ResultPlan *result_plan, ParseNode *node, uint64_t &query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_VAR_ARRAY_VAL );
  int &ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObVariableSetArrayValueStmt *stmt = NULL;
  ObObj value;
  if( OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }

  if (OB_SUCCESS ==ret && NULL != node->children_[0])
  {
    ObString var_name;
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);

    if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_),
                               var_name)) != OB_SUCCESS)
    {
      PARSER_LOG("Can not malloc space for variable name");
    }
    stmt->set_var_name(var_name);
  }

  if( OB_SUCCESS == ret && NULL != node->children_[1] )
  {
    ParseNode *val_nodes = node->children_[1];
    for(int32_t i = 0; OB_SUCCESS == ret && i < val_nodes->num_child_; ++i)
    {
      if( OB_SUCCESS != (ret = resolve_const_value(result_plan, val_nodes->children_[i], value)) )
      {
        TBSYS_LOG(WARN, "resolve const value of array list fail");
      }
      else
      {
        stmt->add_value(value);
      }
    }
  }
  return ret;
}
//add zt 20151202:e

int resolve_execute_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_EXECUTE && node->num_child_ == 2);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObExecuteStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if (ret == OB_SUCCESS)
		{
			OB_ASSERT(node->children_[0]);
			ObString name;
			if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
			{
				PARSER_LOG("Can not malloc space for stmt name");
			}
			else
			{
				stmt->set_stmt_name(name);
			}
		}
		if (ret == OB_SUCCESS && NULL != node->children_[1])
		{
			OB_ASSERT(node->children_[1]->type_ == T_ARGUMENT_LIST);
			ParseNode *arguments = node->children_[1];
			for (int32_t i = 0; ret == OB_SUCCESS && i < arguments->num_child_; i++)
			{
				OB_ASSERT(arguments->children_[i] && arguments->children_[i]->type_ == T_TEMP_VARIABLE);
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable_name(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
			}
		}
	}
	return ret;
}

int resolve_deallocate_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_DEALLOCATE && node->num_child_ == 1);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObDeallocateStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
		TBSYS_LOG(WARN, "fail to prepare resolve stmt. ret=%d", ret);
	}
	else
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		OB_ASSERT(node->children_[0]);
		ObString name;
		if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
		{
			PARSER_LOG("Can not malloc space for stmt name");
		}
		else
		{
			stmt->set_stmt_name(name);
		}
	}
	return ret;
}

int resolve_start_trans_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_BEGIN && node->num_child_ == 0);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObStartTransStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		stmt->set_with_consistent_snapshot(0 != node->value_);
	}
	return ret;
}

int resolve_commit_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_COMMIT && node->num_child_ == 0);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObEndTransStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		stmt->set_is_rollback(false);
	}
	return ret;
}

int resolve_rollback_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_ROLLBACK && node->num_child_ == 0);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObEndTransStmt *stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
	{
	}
	else
	{
		stmt->set_is_rollback(true);
	}
	return ret;
}

int resolve_alter_sys_cnf_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_ALTER_SYSTEM && node->num_child_ == 1);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObAlterSysCnfStmt* alter_sys_cnf_stmt = NULL;
	if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, alter_sys_cnf_stmt)))
	{
	}
	else if ((ret = alter_sys_cnf_stmt->init()) != OB_SUCCESS)
	{
		PARSER_LOG("Init alter system stmt failed, ret=%d", ret);
	}
	else
	{
		OB_ASSERT(node->children_[0] && node->children_[0]->type_ == T_SYTEM_ACTION_LIST);
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		for (int32_t i = 0; ret == OB_SUCCESS && i < node->children_[0]->num_child_; i++)
		{
			ParseNode *action_node = node->children_[0]->children_[i];
			if (action_node == NULL)
				continue;
			OB_ASSERT(action_node->type_ == T_SYSTEM_ACTION && action_node->num_child_ == 5);
			ObSysCnfItem sys_cnf_item;
			ObString param_name;
			ObString comment;
			ObString server_ip;
			sys_cnf_item.config_type_ = static_cast<ObConfigType>(action_node->value_);
			if ((ret = ob_write_string(
					*name_pool,
					ObString::make_string(action_node->children_[0]->str_value_),
					sys_cnf_item.param_name_)) != OB_SUCCESS)
			{
				PARSER_LOG("Can not malloc space for param name");
				break;
			}
			else if (action_node->children_[2] != NULL
					&& (ret = ob_write_string(
							*name_pool,
							ObString::make_string(action_node->children_[2]->str_value_),
							sys_cnf_item.comment_)) != OB_SUCCESS)
			{
				PARSER_LOG("Can not malloc space for comment");
				break;
			}
			else if ((ret = resolve_const_value(
					result_plan,
					action_node->children_[1],
					sys_cnf_item.param_value_)) != OB_SUCCESS)
			{
				break;
			}
			else if (action_node->children_[4] != NULL)
			{
				if (action_node->children_[4]->type_ == T_CLUSTER)
				{
					sys_cnf_item.cluster_id_ = action_node->children_[4]->children_[0]->value_;
				}
				else if (action_node->children_[4]->type_ == T_SERVER_ADDRESS)
				{
					if ((ret = ob_write_string(
							*name_pool,
							ObString::make_string(action_node->children_[4]->children_[0]->str_value_),
							sys_cnf_item.server_ip_)) != OB_SUCCESS)
					{
						PARSER_LOG("Can not malloc space for IP");
						break;
					}
					else
					{
						sys_cnf_item.server_port_ = action_node->children_[4]->children_[1]->value_;
					}
				}
			}
			OB_ASSERT(action_node->children_[3]);
			switch (action_node->children_[3]->value_)
			{
			case 1:
				sys_cnf_item.server_type_ = OB_ROOTSERVER;
				break;
			case 2:
				sys_cnf_item.server_type_ = OB_CHUNKSERVER;
				break;
			case 3:
				sys_cnf_item.server_type_ = OB_MERGESERVER;
				break;
			case 4:
				sys_cnf_item.server_type_ = OB_UPDATESERVER;
				break;
			default:
				/* won't be here */
				ret = OB_ERR_RESOLVE_SQL;
				PARSER_LOG("Unkown server type");
				break;
			}
			if ((ret = alter_sys_cnf_stmt->add_sys_cnf_item(*result_plan, sys_cnf_item)) != OB_SUCCESS)
			{
				// PARSER_LOG("Add alter system config item failed");
				break;
			}
		}
	}
	return ret;
}

int resolve_change_obi(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	UNUSED(query_id);
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_CHANGE_OBI && node->num_child_ >= 2);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObChangeObiStmt* change_obi_stmt = NULL;
	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	ObLogicalPlan *logical_plan = NULL;
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		change_obi_stmt = (ObChangeObiStmt*)parse_malloc(sizeof(ObChangeObiStmt), result_plan->name_pool_);
		if (change_obi_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObChangeObiStmt");
		}
		else
		{
			change_obi_stmt = new(change_obi_stmt) ObChangeObiStmt(name_pool);
			query_id = logical_plan->generate_query_id();
			change_obi_stmt->set_query_id(query_id);
			if ((ret = logical_plan->add_query(change_obi_stmt)) != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObChangeObiStmt to logical plan");
			}
			else
			{
				OB_ASSERT(node->children_[0]->type_ == T_SET_MASTER
						|| node->children_[0]->type_ == T_SET_SLAVE
						|| node->children_[0]->type_ == T_SET_MASTER_SLAVE);
				OB_ASSERT(node->children_[1]&& node->children_[1]->type_ == T_STRING);
				change_obi_stmt->set_target_server_addr(node->children_[1]->str_value_);
				if (node->children_[0]->type_ == T_SET_MASTER)
				{
					change_obi_stmt->set_target_role(ObiRole::MASTER);
				}
				else if (node->children_[0]->type_ == T_SET_SLAVE)
				{
					change_obi_stmt->set_target_role(ObiRole::SLAVE);
				}
				else // T_SET_MASTER_SLAVE
				{
					if (node->children_[2] != NULL)
					{
						OB_ASSERT(node->children_[2]->type_ == T_FORCE);
						change_obi_stmt->set_force(true);
					}
				}
			}
		}
	}
	return ret;
}
int resolve_kill_stmt(
		ResultPlan* result_plan,
		ParseNode* node,
		uint64_t& query_id)
{
	OB_ASSERT(result_plan);
	OB_ASSERT(node && node->type_ == T_KILL && node->num_child_ == 3);
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	ObKillStmt* kill_stmt = NULL;
	ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	ObLogicalPlan *logical_plan = NULL;
	if (result_plan->plan_tree_ == NULL)
	{
		logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
		if (logical_plan == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObLogicalPlan");
		}
		else
		{
			logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
			result_plan->plan_tree_ = logical_plan;
		}
	}
	else
	{
		logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
	}

	if (ret == OB_SUCCESS)
	{
		kill_stmt = (ObKillStmt*)parse_malloc(sizeof(ObKillStmt), result_plan->name_pool_);
		if (kill_stmt == NULL)
		{
			ret = OB_ERR_PARSER_MALLOC_FAILED;
			snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
					"Can not malloc ObKillStmt");
		}
		else
		{
			kill_stmt = new(kill_stmt) ObKillStmt(name_pool);
			query_id = logical_plan->generate_query_id();
			kill_stmt->set_query_id(query_id);
			if ((ret = logical_plan->add_query(kill_stmt)) != OB_SUCCESS)
			{
				snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
						"Can not add ObKillStmt to logical plan");
			}
		}
	}
	if (OB_SUCCESS == ret)
	{
		OB_ASSERT(node->children_[0]&& node->children_[0]->type_ == T_BOOL);
		OB_ASSERT(node->children_[1]&& node->children_[1]->type_ == T_BOOL);
		OB_ASSERT(node->children_[2]);
		kill_stmt->set_is_global(node->children_[0]->value_ == 1? true: false);
		kill_stmt->set_thread_id(node->children_[2]->value_);
		kill_stmt->set_is_query(node->children_[1]->value_ == 1? true: false);
	}
	return ret;
}
//zhounan unmark:b
/**
 * @brief resolve_cursor_declare_stmt
 * parse procedure's cursor_declare statement syntax tree and create ObCursorDeclareStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_declare statement syntax tree
 * @param query_id is cursor_declare_stmt's id
 * @return errorcode.
 */
int resolve_cursor_declare_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_DECLARE && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorDeclareStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
        PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
        stmt->set_cursor_name(name);
      }
    }
    if (ret == OB_SUCCESS)
    {
      uint64_t sub_query_id = OB_INVALID_ID; 
      ret = resolve_select_stmt(result_plan, node->children_[1], sub_query_id);
      if (ret == OB_SUCCESS)
      stmt->set_declare_query_id(sub_query_id);
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_open_stmt
 * parse procedure's cursor_open statement syntax tree and create ObCursorOpenStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_open statement syntax tree
 * @param query_id is cursor_open_stmt's id
 * @return errorcode.
 */
int resolve_cursor_open_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_OPEN && node->num_child_ == 1); int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorOpenStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}
/**
 * @brief resolve_cursor_fetch_stmt
 * parse procedure's cursor_fetch statement syntax tree and create ObCursorFetchStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch statement syntax tree
 * @param query_id is cursor_fetch_stmt's id
 * @return
 */
int resolve_cursor_fetch_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && (node->type_ == T_CURSOR_FETCH||node->type_ == T_CURSOR_FETCH_NEXT) && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_first_into_stmt
 * parse procedure's cursor_fetch_first_into statement syntax tree and create ObCursorFetchFirstIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_first_into statement syntax tree
 * @param query_id is cursor_fetch_first_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_first_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_FIRST_INTO && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchFirstIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_first_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_into_stmt
 * parse procedure's cursor_fetch_into statement syntax tree and create ObCursorFetchIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_into statement syntax tree
 * @param query_id is cursor_fetch_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && (node->type_ == T_CURSOR_FETCH_INTO||node->type_ == T_CURSOR_FETCH_NEXT_INTO) && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_prior_stmt
 * parse procedure's cursor_fetch_into statement syntax tree and create ObFetchPriorStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_prior statement syntax tree
 * @param query_id is cursor_fetch_into_stmt's id
 * @return error code
 */
int resolve_cursor_fetch_prior_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_PRIOR && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchPriorStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_prior_into_stmt
 * parse procedure's cursor_fetch_prior_into statement syntax tree and create ObCursorFetchPriorIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_prior_into statement syntax tree
 * @param query_id is cursor_fetch_prior_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_prior_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_PRIOR_INTO && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchPriorIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_prior_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_first_stmt
 * parse procedure's cursor_fetch_first statement syntax tree and create ObFetchFirstStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_first statement syntax tree
 * @param query_id is cursor_fetch_first_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_first_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_FIRST && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchFirstStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
	    stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_last_stmt
 * parse procedure's cursor_fetch_last statement syntax tree and create ObFetchLastStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_last statement syntax tree
 * @param query_id is cursor_fetch_last_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_last_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_LAST && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchLastStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_last_into_stmt
 * parse procedure's cursor_fetch_last_into statement syntax tree and create ObCursorFetchLastIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_last_into statement syntax tree
 * @param query_id is cursor_fetch_last_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_last_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_LAST_INTO && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchLastIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_last_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}


/**
 * @brief resolve_cursor_fetch_relative_stmt
 * parse procedure's cursor_fetch_relative statement syntax tree and create ObFetchRelativeStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_relative statement syntax tree
 * @param query_id is cursor_fetch_relative_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_relative_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_RELATIVE && node->num_child_ == 3);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchRelativeStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
      OB_ASSERT(node->children_[1]);
      if (node->children_[1]->value_ == 0)
      {
    	 stmt->set_is_next(0);
      }
      else
      {
       	stmt->set_is_next(1);
      }
      OB_ASSERT(node->children_[2]);
      stmt->set_fetch_count(node->children_[2]->value_);
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_relative_into_stmt
 * parse procedure's cursor_fetch_relative_into statement syntax tree and create ObCursorFetchRelativeIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_relative_into statement syntax tree
 * @param query_id is cursor_fetch_relative_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_relative_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_RELATIVE_INTO && node->num_child_ == 3);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchRelativeIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		/*resolve fetch relative into argument list*/
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		//resolve fetch relative into's fetch
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_relative_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_absolute_stmt
 * parse procedure's cursor_fetch_absolute statement syntax tree and create ObFetchAbsoluteStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_absolute statement syntax tree
 * @param query_id is cursor_fetch_absolute_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_absolute_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_ABSOLUTE && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchAbsoluteStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
      OB_ASSERT(node->children_[1]);
      stmt->set_fetch_count(node->children_[1]->value_);
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_absolute_into_stmt
 * parse procedure's cursor_fetch_absolute_into statement syntax tree and create ObCursorFetchAbsIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_absolute_into statement syntax tree
 * @param query_id is cursor_fetch_absolute_into_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_absolute_into_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_ABS_INTO && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorFetchAbsIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
	if (ret == OB_SUCCESS)
	{
		ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
		/*resolve fetch abs into argument list*/
		if(node->children_[1]!=NULL)
		{
			ParseNode* arguments=node->children_[1];
			for (int32_t i = 0;i < arguments->num_child_; i++)
			{
				ObString name;
				if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
				{
					PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
				}
				else if ((ret = stmt->add_variable(name)) != OB_SUCCESS)
				{
					PARSER_LOG("Add Using variable failed");
				}
				else
				{
					TBSYS_LOG(INFO, "add_variable_name is %s  get is %s",name.ptr(),stmt->get_variable((int64_t)i).ptr());
				}
			}
		}
		//resolve fetch abs into's fetch stmt
		if(node->children_[0]!=NULL)
		{
			uint64_t sub_query_id = OB_INVALID_ID;

			if((ret =resolve_cursor_fetch_absolute_stmt(result_plan, node->children_[0], sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "resolve_select_stmt error");
			}
			else if((ret=stmt->set_cursor_id(sub_query_id))!=OB_SUCCESS)
			{
				TBSYS_LOG(WARN, "set_declare_id error");
			}

		}
	}
  }
  return ret;
}

/**
 * @brief resolve_cursor_fetch_fromto_stmt
 * parse procedure's cursor_fetch_fromto statement syntax tree and create ObFetchFromtoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_fetch_fromto statement syntax tree
 * @param query_id is cursor_fetch_fromto_stmt's id
 * @return errorcode.
 */
int resolve_cursor_fetch_fromto_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_FETCH_FROMTO && node->num_child_ == 3);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObFetchFromtoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
    	PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
    	stmt->set_cursor_name(name);
      }
      OB_ASSERT(node->children_[1]);
      stmt->set_count_f(node->children_[1]->value_);
      OB_ASSERT(node->children_[2]);
      stmt->set_count_t(node->children_[2]->value_);
    }
  }
  return ret;
}

/**
 * @brief resolve_cursor_open_stmt
 * parse procedure's cursor_close statement syntax tree and create ObCursorCloseStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of cursor_close statement syntax tree
 * @param query_id is cursor_close_stmt's id
 * @return errorcode.
 */
int resolve_cursor_close_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id
)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_CURSOR_CLOSE && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObCursorCloseStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(WARN, "fail to prepare resolve stmt. ret=%d", ret);
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    if (ret == OB_SUCCESS)
    {
      OB_ASSERT(node->children_[0]);
      ObString name;
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), name)) != OB_SUCCESS)
      {
        PARSER_LOG("Can not malloc space for stmt name");
      }
      else
      {
        stmt->set_cursor_name(name);
      }
    }
  }
  return ret;
}
//add:e

//add by zhujun:b
//code_coverage_zhujun
/**
 * @brief resolve_procedure_select_into_stmt
 * parse procedure's select_into statement syntax tree and create ObProcedureSelectIntoStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of select_into statement syntax tree
 * @param query_id is select_into_stmt's id
 * @param ps_stmt is a point of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_select_into_stmt(
                ResultPlan* result_plan,
                ParseNode* node,
                uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_SELECT_INTO && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureSelectIntoStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(INFO, "prepare_resolve_stmt have ERROR!");
  }
  else
  {
    if (ret == OB_SUCCESS)
    {
      ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
      /*resolve the left vaule list*/
      if (node->children_[0]!=NULL)
      {
        ParseNode* arguments=node->children_[0];
        for (int32_t i = 0;i < arguments->num_child_; i++)
        {
          SpRawVar raw_var;
          if( arguments->children_[i]->type_ == T_TEMP_VARIABLE )
          {
            if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), raw_var.var_name_)) != OB_SUCCESS)
            {
              PARSER_LOG("copy variable name %s error", arguments->children_[i]->str_value_);
            }
            else if ((ret = stmt->add_variable(raw_var)) != OB_SUCCESS)
            {
              TBSYS_LOG(WARN, "add variables into stmt fail");
            }
          }
          else if (arguments->children_[i]->type_ == T_ARRAY)
          {
            if( OB_SUCCESS != (ret = resolve_array_expr(result_plan, arguments->children_[i], raw_var.var_name_, raw_var.idx_value_)))
            {
              TBSYS_LOG(WARN, "resolve array expr failed");
            }
            else
            {
              stmt->add_variable(raw_var);
            }
          }
          else
          {
            TBSYS_LOG(WARN, "unsupported variables type here");
          }
        }
        //TODO we need to check is these variables defined.
      }
      //resolve select clause
      if(node->children_[1]!=NULL)
      {
        uint64_t sub_query_id = OB_INVALID_ID;

//        ObLogicalPlan *logic_plan = get_logical_plan(result_plan);
//        int32_t expr_itr = logic_plan->get_raw_expr_count();
        if ((ret =resolve_select_stmt(result_plan, node->children_[1], sub_query_id))!=OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "resolve_select_stmt error");
        }
        else if ((ret=stmt->set_declare_id(sub_query_id))!=OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "set_declare_id error");
        }
        else
        {
//          int32_t expr_new_itr = logic_plan->get_raw_expr_count();
//          ObSelectStmt* sel_stmt = (ObSelectStmt*) logic_plan->get_query(sub_query_id);
//          for(; expr_itr < expr_new_itr; ++expr_itr)
//          {
//            ObItemType raw_type = logic_plan->get_raw_expr(expr_itr)->get_expr_type();
//            if( T_SYSTEM_VARIABLE == raw_type || T_TEMP_VARIABLE == raw_type || T_ARRAY == raw_type)
//            sel_stmt->add_raw_var_expr(logic_plan->get_raw_expr(expr_itr));
//          }
        }
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_procedure_declare_stmt
 * parse procedure's declare statement syntax tree and create ObProcedureDeclareStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of declare statement syntax tree
 * @param query_id is declare_stmt's id
 * @param ps_stmt is a point of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_declare_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_DECLARE && node->num_child_ == 4);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureDeclareStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
    TBSYS_LOG(WARN, "prepare_resolve_stmt have ERROR!");
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);

    ObObjType var_type = ObNullType;
    ObObj default_value;
    bool has_default = false;
    bool is_array = false;

    //variable data type
    if (node->children_[1] != NULL)
    {
      switch(node->children_[1]->type_)
      {
      case T_TYPE_INTEGER:
        var_type = ObIntType;
        break;
      case T_TYPE_FLOAT:
        var_type = ObFloatType;
        break;
      case T_TYPE_DOUBLE:
        var_type = ObDoubleType;
        break;
      case T_TYPE_DECIMAL:
        var_type = ObDecimalType;
        break;
      case T_TYPE_BOOLEAN:
        var_type = ObBoolType;
        break;
      case T_TYPE_DATETIME:
      case T_TYPE_DATE:
      case T_TYPE_TIME:
      case T_TYPE_TIMESTAMP:
        var_type = ObPreciseDateTimeType;
        break;
      case T_TYPE_CHARACTER:
      case T_TYPE_VARCHAR:
        var_type = ObVarcharType;
        break;
      default:
        TBSYS_LOG(WARN, "data type[%d] is not supported", var_type);
        ret = OB_NOT_SUPPORTED;
        break;
      }
    }

    //default value
    if (OB_SUCCESS == ret && node->children_[2] != NULL)
    {
      default_value.set_type(var_type);
      if ((ret = resolve_const_value(result_plan, node->children_[2], default_value)) != OB_SUCCESS)
      {
        TBSYS_LOG(WARN, "resolve_procedure_declare_stmt resolve_const_value error");
      }
      else
      {
        has_default = true;

        //quite strange code here, checking something here
        //node->children_[2]->str_value_;1233123123123.1111121312
        if( node->children_[1]->type_ == T_TYPE_DECIMAL)
        {
          int64_t leftint = node->children_[1]->children_[0]->value_;

          int64_t rightint = node->children_[1]->children_[1]->value_;

          int64_t preint=rightint-leftint;

          char *p=(char*)node->children_[2]->str_value_;
          int32_t index=0;

          for(;*p!='.';)
          {
            p++;
            index++;
          }
          if(preint<index)
          {
            ret=OB_ERR_ILLEGAL_VALUE;
            TBSYS_LOG(USER_ERROR, "decimal range error");
          }
          TBSYS_LOG(TRACE, "preint=%ld,leftint=%ld,rightint=%ld,index=%d",preint,leftint,rightint,index);
        }
      }
    }

    //array identifier
    if (OB_SUCCESS == ret && node->children_[3] != NULL)
    {
      if (node->children_[3]->value_ == 1)
      {
        is_array = true;
      }
      else
      {
        TBSYS_LOG(WARN, "array identifier get wrong parse value");
      }
    }

    //argument list
    if ( OB_SUCCESS == ret && node->children_[0] != NULL )
    {
      ParseNode* var_node=node->children_[0];
      OB_ASSERT(var_node->type_==T_ARGUMENT_LIST);

      for (int32_t i = 0; ret == OB_SUCCESS && i < var_node->num_child_; i++)
      {
        ObVariableDef var;
        var.is_default_ = has_default;
        var.variable_type_ = var_type;
        var.is_array_ = is_array;
        if ( has_default ) var.default_value_ = default_value;

        if (OB_SUCCESS != (ret=ob_write_string(*name_pool, ObString::make_string(var_node->children_[i]->str_value_),
                                              var.variable_name_)))
        {
          PARSER_LOG("Can not malloc space for variable name");
        }
        else if ((ret=stmt->add_proc_var(var))!=OB_SUCCESS) //add into the declare stmt
        {
          TBSYS_LOG(WARN, "add_proc_param have ERROR!");
        }
        else if ((ret=ps_stmt->add_declare_var(var.variable_name_))!=OB_SUCCESS) //add into the procedure stmt
        {
          TBSYS_LOG(WARN, "add_declare_var have ERROR!");
        }
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_procedure_assign_stmt
 * parse procedure's assign statement syntax tree and create ObProcedureAssginStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of assign statement syntax tree
 * @param query_id is assign_stmt's id
 * @param ps_stmt is a point of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_assign_stmt(
                ResultPlan* result_plan,
                ParseNode* node,
                uint64_t& query_id,
                ObProcedureStmt *ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_ASSGIN && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureAssginStmt *stmt = NULL;

  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
    TBSYS_LOG(ERROR, "prepare_resolve_stmt have ERROR!");
  }
  else if ( node->children_[0] != NULL)
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    /*解析assign语句参数*/
    ParseNode* var_val_list_node=node->children_[0];
    OB_ASSERT(var_val_list_node->type_==T_VAR_VAL_LIST);

    ObString var_name;
    //genereate logical plan for each assign
    for (int32_t i = 0; ret == OB_SUCCESS && i < var_val_list_node->num_child_; i++)
    {
      ParseNode* var_val_node = var_val_list_node->children_[i];
      uint64_t expr_id = 0;
      //analyze the right expr
      //modified by wdh 20160630 :b T_NONE_LIMIT T_VARIABLE_VALUE_LIMIT
      if (OB_SUCCESS != (ret = resolve_independ_expr(result_plan, NULL,var_val_node->children_[1],
                                                     expr_id,T_VARIABLE_VALUE_LIMIT)))
      {
        TBSYS_LOG(WARN, "resolve assignment expression error");
      }
      //analyze the left variable
      else if ( var_val_node->children_[0]->type_ == T_TEMP_VARIABLE )
      {
        ObRawVarAssignVal var_val;
        var_val.val_expr_id_ = expr_id;
        if((ret=ob_write_string(*name_pool, ObString::make_string(var_val_node->children_[0]->str_value_),
                                var_val.var_name_))!=OB_SUCCESS)
        {
          PARSER_LOG("Can not malloc space for variable name");
        }
        else
        {
          stmt->add_var_val(var_val);
          var_name = var_val.var_name_;
        }
      }
      else if ( var_val_node->children_[0]->type_ == T_ARRAY )
      {
        ObRawVarAssignVal arr_val;
        arr_val.val_expr_id_ = expr_id;
        if( OB_SUCCESS != (ret = resolve_array_expr(result_plan, var_val_node->children_[0], arr_val.var_name_, arr_val.idx_value_)) )
        {
          TBSYS_LOG(WARN, "resolve array expr failed");
        }
        else
        {
          stmt->add_var_val(arr_val);
          var_name = arr_val.var_name_;
        }
      }
      else
      {
        TBSYS_LOG(WARN, "unsupported left value");
        ret = OB_NOT_SUPPORTED;
      }

      if (ret == OB_SUCCESS)
      {
          bool find = false;
          //does the variable existence check make sense here?
          //the variable used in the expr is not checked
          for (int64_t j = 0; j < ps_stmt->get_declare_var_size(); j++)
          {
            const ObString &declare_var=ps_stmt->get_declare_var(j);
            if ( var_name.compare(declare_var) == 0 ) //check existence
            {
              find = true;
              break;
            }
          }
          for (int64_t j = 0;  !find && j < ps_stmt->get_param_size(); j++)
          {
            const ObParamDef& def=ps_stmt->get_param(j);
            if ( var_name.compare(def.param_name_)==0 )
            {
              find = true;
              break;
            }
          }
          if ( !find ) //error means the variable is not defined in variables or paramters
          {
            ret=OB_ERR_SP_UNDECLARED_VAR;
            TBSYS_LOG(ERROR, "Variable %.*s does not declare", var_name.length(), var_name.ptr());
            break;
          }
      }
    }
  }
  return ret;
}

/**
 * @brief resolve_procedure_case_stmt
 * parse procedure's case statement syntax tree and create ObProcedureCaseStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of case statement syntax tree
 * @param query_id is case_stmt's id
 * @param ps_stmt is a point of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_case_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_CASE && node->num_child_ == 3);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureCaseStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(ERROR, "resolve_procedure_case_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
    if (ret == OB_SUCCESS)
    {
    	/*获取case的表达式节点*/
        uint64_t expr_id;//modified by wdh 20160708 T_VARIABLE_VALUE_LIMIT
        if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
    	{
    		TBSYS_LOG(ERROR, "resolve_independ_expr error");
    	}
      else if ((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
    	{
    		TBSYS_LOG(ERROR, "set_expr_id error");
    	}
    	else
    	{
			//-----------------------------case when list------------------------------------------
        if (node->children_[1]!=NULL)
    		{
				ParseNode* casewhen_node = node->children_[1];/*case when 节点的*/

				//foreach cae when node's child node
				for (int32_t i = 0; ret == OB_SUCCESS && i < casewhen_node->num_child_; i++)
				{
					uint64_t casewhen_query_id = OB_INVALID_ID;
                    if((ret=resolve_procedure_casewhen_stmt(result_plan, casewhen_node->children_[i], casewhen_query_id,ps_stmt))!=OB_SUCCESS)
					{
						TBSYS_LOG(ERROR, "resolve_procedure_casewhen_stmt error!");
						break;
					}
          else if (ret==OB_SUCCESS&&(ret=stmt->add_case_when_stmt(casewhen_query_id))!=OB_SUCCESS)
					{
						TBSYS_LOG(ERROR, "add_case_when_stmt error!");
						break;
					}
				}
    		}
        if (ret==OB_SUCCESS)
    		{
				//-----------------------------check exist else------------------------------------------
        if ( node->children_[2]!=NULL && node->children_[2]->children_[0]!=NULL )
				{
                    //OB_ASSERT(node->children_[2]->children_[0]->type_ == T_PROCEDURE_ELSE);
					OB_ASSERT(node->children_[2]->type_ == T_PROCEDURE_ELSE);
					uint64_t else_query_id = OB_INVALID_ID;
					ParseNode* else_node = node->children_[2];

          if ((ret = resolve_procedure_else_stmt(result_plan, else_node, else_query_id,ps_stmt))!=OB_SUCCESS)
					{
						TBSYS_LOG(ERROR, "resolve_procedure_else_stmt error!");
					}
					else
					{
						stmt->set_else_stmt(else_query_id);
						stmt->set_have_else(true);
					}
				}
				else
				{
					ret=stmt->set_have_else(false);
				}
    		}
    	}
    }
  }
  return ret;
}
#define CREATE_RAW_EXPR(expr, type_name, result_plan)    \
({    \
  ObLogicalPlan* logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_); \
  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);  \
  expr = (type_name*)parse_malloc(sizeof(type_name), name_pool);   \
  if (expr != NULL) \
  { \
    expr = new(expr) type_name();   \
    if (OB_SUCCESS != logical_plan->add_raw_expr(expr))    \
    { \
      expr = NULL;  /* no memory leak, bulk dealloc */ \
    } \
  } \
  if (expr == NULL)  \
  { \
    result_plan->err_stat_.err_code_ = OB_ERR_PARSER_MALLOC_FAILED; \
    TBSYS_LOG(WARN, "out of memory"); \
    snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,  \
        "Fail to malloc new raw expression"); \
  } \
  expr; \
})

/**
 * @brief resolve_procedure_casewhen_stmt
 * parse procedure's casewhen statement syntax tree and create ObProcedureCaseWhenStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of casewhen statement syntax tree
 * @param query_id is casewhen_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_casewhen_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt* ps_stmt
	)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_CASE_WHEN && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureCaseWhenStmt *stmt = NULL;
  ParseNode* vector_node = node->children_[1];
  uint64_t expr_id;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
    TBSYS_LOG(ERROR, "resolve_procedure_casewhen_stmt prepare_resolve_stmt have ERROR!");
  }
  // modified by wdh 20160708
  else  if ((ret = resolve_independ_expr(result_plan, NULL, node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
  {
    TBSYS_LOG(WARN, "resolve_independ_expr  ERROR");
  }
  else if ((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
  {
    TBSYS_LOG(ERROR, "set_expr_id have ERROR!");
  }
  else if ( NULL != vector_node )
  {
    //-----------------------------resolve then stmt block-----------------------------
    for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
    {
      uint64_t sub_query_id = OB_INVALID_ID;

      if ( vector_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
      {
        TBSYS_LOG(WARN, "case when should not contain declare stmt");
        ret = OB_ERROR; //change to not_support code
      }
      else if ( OB_SUCCESS != (ret = resolve_procedure_inner_stmt(result_plan, vector_node->children_[i], sub_query_id, ps_stmt)) )
      {
        TBSYS_LOG(WARN, "resolve then stmt [%d] failed", i);
      }
      else if ( OB_SUCCESS !=  (ret = stmt->add_then_stmt(sub_query_id)) )
      {
        TBSYS_LOG(WARN, "add then stmt failed");
      }
    }
  }
  else
  {
    TBSYS_LOG(WARN, "case-when resolve err, then block should not be empty");
    ret = OB_ERR_SP_BADSTATEMENT;
  }
  return ret;
}


/**
 * @brief resolve_procedure_if_stmt
 * parse procedure's if statement syntax tree and create ObProcedureIfStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of if statement syntax tree
 * @param query_id is if_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_if_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_IF && node->num_child_ == 4);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureIfStmt *stmt = NULL;
  ParseNode *vector_node = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(ERROR, "resolve_procedure_if_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
    /* resolve if then block */
    uint64_t expr_id;
    // modified by wdh 20160708
    if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "resolve_procedure_if_stmt resolve_independ_expr error");
    }
    else if((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "resolve_procedure_if_stmt set_expr_id error");
    }
    else if ( NULL != (vector_node = node->children_[1]) && vector_node->num_child_ != 0)
    {
      for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
      {
        uint64_t sub_query_id = OB_INVALID_ID;

        if ( vector_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
        {
          TBSYS_LOG(WARN, "does not support stmt type[%d] in elseif", vector_node->children_[i]->type_);
          ret = OB_ERR_SP_BADSTATEMENT;
        }
        if ( OB_SUCCESS != (ret = resolve_procedure_inner_stmt(result_plan, vector_node->children_[i], sub_query_id, ps_stmt)) )
        {
          TBSYS_LOG(WARN, "resolve body stmt fail at [%d]", i);
        }
        else if ( OB_SUCCESS != (ret = stmt->add_then_stmt(sub_query_id)) )
        {
          TBSYS_LOG(ERROR, "foreach else if children_[1] error!");
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(WARN, "then block does not contain any statement");
      ret = OB_ERR_SP_BADSTATEMENT;
    }
  }

  if(ret==OB_SUCCESS)
  {
    //-----------------------------check exist else if------------------------------------------
    if (node->children_[2]!=NULL)
    {
      ParseNode* elseif_node = node->children_[2];/*elseif node*/

      for (int32_t i = 0; ret == OB_SUCCESS && i < elseif_node->num_child_; i++)
      {
        uint64_t elseif_query_id = OB_INVALID_ID;
        if(OB_SUCCESS != (ret = resolve_procedure_elseif_stmt(result_plan, elseif_node->children_[i], elseif_query_id,ps_stmt)) )
        {
          TBSYS_LOG(ERROR, "resolve_procedure_elseif_stmt error!");
        }
        else if (ret==OB_SUCCESS&&(ret=stmt->add_else_if_stmt(elseif_query_id))!=OB_SUCCESS)
        {
          TBSYS_LOG(ERROR, "foreach else if children_[2] error!");
          break;
        }
      }
      if ( ret == OB_SUCCESS )
      {
        ret=stmt->set_have_elseif(true);
      }
    }
    else if ((ret=stmt->set_have_elseif(false))!=OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "set_have_elseif  error!");
    }
  }

  //-----------------------------check exist else------------------------------------------
  if ( OB_SUCCESS == ret )
  {
    if ( node->children_[3] != NULL )
    {
      OB_ASSERT(node->children_[3]->type_ == T_PROCEDURE_ELSE);
      uint64_t else_query_id = OB_INVALID_ID;
      ParseNode* else_node = node->children_[3];

      if ((ret = resolve_procedure_else_stmt(result_plan, else_node, else_query_id, ps_stmt)) != OB_SUCCESS)
      {
        TBSYS_LOG(ERROR, "resolve_procedure_else_stmt error!");
      }
      else if ((ret=stmt->set_else_stmt(else_query_id))!=OB_SUCCESS)
      {
        TBSYS_LOG(ERROR, "set_else_stmt(else_query_id) error!");
      }
      else
      {
        ret=stmt->set_have_else(true);
      }
    }
    else if ((ret=stmt->set_have_else(false)) != OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "set_have_else  error!");
    }
  }

  return ret;
}
/**
 * @brief resolve_procedure_elseif_stmt
 * parse procedure's elseif statement syntax tree and create ObProcedureElseIfStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of elseif statement syntax tree
 * @param query_id is elseif_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_elseif_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
	ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_ELSEIF && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureElseIfStmt *stmt = NULL;
  ParseNode *vector_node = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(ERROR, "resolve_procedure_elseif_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
    uint64_t expr_id;
    // modified by wdh 20160708
    if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "resolve_independ_expr  ERROR");
    }
    else if ((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "set_expr_id have ERROR!");
    }
    else if ( NULL != (vector_node = node->children_[1]) && vector_node->num_child_ != 0 )
    {
      //--------------------------else if then--------------------------
      ParseNode* vector_node = node->children_[1];

      for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
      {
        uint64_t sub_query_id = OB_INVALID_ID;

        if ( vector_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
        {
          TBSYS_LOG(WARN, "does not support stmt type[%d] in elseif", vector_node->children_[i]->type_);
          ret = OB_ERR_PARSE_SQL;
        }
        if ( OB_SUCCESS != (ret = resolve_procedure_inner_stmt(result_plan, vector_node->children_[i], sub_query_id, ps_stmt)) )
        {
          TBSYS_LOG(WARN, "resolve body stmt fail at [%d]", i);
        }
        else if ( OB_SUCCESS != (ret = stmt->add_elseif_then_stmt(sub_query_id)) )
        {
          TBSYS_LOG(ERROR, "foreach else if children_[1] error!");
          break;
        }
      }
    }
    else
    {
      ret = OB_ERR_SP_BADSTATEMENT;
    }
  }
  return ret;
}

/**
 * @brief resolve_procedure_loop_stmt
 * parse procedure's loop statement syntax tree and create ObProcedureLoopStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of loop statement syntax tree
 * @param query_id is loop_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_loop_stmt(ResultPlan *result_plan, ParseNode *node, uint64_t &query_id, ObProcedureStmt *proc_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_LOOP && node->num_child_ == 5);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureLoopStmt *loop_stmt = NULL;
  TBSYS_LOG(TRACE, "enter resolve_procedure_loop_stmt");
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, loop_stmt)))
  {
    TBSYS_LOG(ERROR, "resolve_procedure_loop_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);

    //resolve loop_counter
    //TODO we need to solve the variable name conflict problem
    //we can check whether the variable name is declared in the procedure
    //add by wdh 20160624 :b
    if( node->children_[0] == NULL)
    {
    }
    //add :e
    else if ( node->children_[0] != NULL && node->children_[0]->type_ == T_TEMP_VARIABLE )
    {
      ObString loop_counter_name;
      if( OB_SUCCESS != (ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), loop_counter_name)) )
      {
        PARSER_LOG("Can not malloc space for loop counter name");
      }
      else if ( !proc_stmt->check_var_exist(loop_counter_name) )
      {
        proc_stmt->add_declare_var(loop_counter_name);//add by wdh 20160714
        loop_stmt->set_loop_count_name(loop_counter_name);
      }
      else
      {
        //variable conflict with declared ones or paramters
        ret = OB_ERR_SP_DUP_VAR;
      }
    }
    else
    {
      ret = OB_ERR_PARSE_SQL;
    }

    //resolve reverse flag
    if ( OB_SUCCESS == ret && node->children_[1] != NULL && node->children_[1]->type_ == T_BOOL )
    {
      loop_stmt->set_reverse(true);
    }

    //resolve lowest value
    //add by wdh 20160624 :b
    if (node->children_[2]==NULL)
    {
        uint64_t lowest_expr_id = (uint64_t)-1;
        loop_stmt->set_lowest_expr(lowest_expr_id);
    }
    //add :e
    else if ( OB_SUCCESS == ret && node->children_[2] != NULL )
    {
      uint64_t lowest_expr_id;
      if(OB_SUCCESS != (ret = resolve_independ_expr(result_plan, NULL, node->children_[2], lowest_expr_id, T_VARIABLE_VALUE_LIMIT)) )
      {
        TBSYS_LOG(WARN, "resolve loop's lowest expression fail");
      }
      else
      {
        loop_stmt->set_lowest_expr(lowest_expr_id);
      }
    }

    //resolve highest value
    if ( OB_SUCCESS == ret && node->children_[3] != NULL )
    {
      uint64_t highest_expr_id;
      if (OB_SUCCESS != (ret = resolve_independ_expr(result_plan, NULL, node->children_[3], highest_expr_id, T_VARIABLE_VALUE_LIMIT)) )
      {
        TBSYS_LOG(WARN, "resolve loop's highest expression fail");
      }
      else
      {
        loop_stmt->set_highest_expr(highest_expr_id);
      }
    }

    //resolve loop body
    if (ret == OB_SUCCESS && node->children_[4] != NULL )
    {
      ParseNode* loop_body_node = node->children_[4];

      TBSYS_LOG(TRACE, "loop body num_child_=%d", loop_body_node->num_child_);

      for (int32_t i = 0; ret == OB_SUCCESS && i < loop_body_node->num_child_; i++)
      {
        uint64_t sub_query_id = OB_INVALID_ID;

        //filter some stmt here
        if ( loop_body_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
        {
          TBSYS_LOG(WARN, "dose not support stmt type[%d] in loop", loop_body_node->children_[i]->type_);
          ret = OB_ERR_PARSE_SQL;
        }
        else if ( OB_SUCCESS != ( ret = resolve_procedure_inner_stmt(result_plan, loop_body_node->children_[i], sub_query_id, proc_stmt)))
        {
          TBSYS_LOG(WARN, "resolve body stmt fail at [%d]", i);
        }
        else if ( OB_SUCCESS != (ret = loop_stmt->add_loop_stmt(sub_query_id)))
        {
          TBSYS_LOG(WARN, "loop body add stmt fail at [%d]", i);
        }
      }
    }
    if (ret == OB_SUCCESS)
    {
        proc_stmt->delete_var();//add by wdh 20160714
    }
  }
  return ret;
}


//add hjw 20151229:b
/**
 * @brief resolve_procedure_while_stmt
 * parse procedure's while statement syntax tree and create ObProcedureWhileStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of while statement syntax tree
 * @param query_id is while_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_while_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_WHILE && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureWhileStmt *stmt = NULL;
  ParseNode *vector_node = NULL;
  uint64_t expr_id;
  TBSYS_LOG(INFO, "enter resolve_procedure_while_stmt");
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
      TBSYS_LOG(ERROR, "resolve_procedure_while_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
    if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "resolve_independ_expr  ERROR");
    }
    else if ((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
    {
      TBSYS_LOG(ERROR, "set_expr_id have ERROR!");
    }
    else if ( NULL != (vector_node = node->children_[1]) )
    {
      //-----------------------------while do-----------------------------
      TBSYS_LOG(INFO, "while do num_child_=%d",vector_node->num_child_);
      /*handle the loop body*/
      for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
      {
        uint64_t sub_query_id = OB_INVALID_ID;

        if ( vector_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
        {
          TBSYS_LOG(WARN, "while do should not contain declare stmt");
          ret = OB_ERROR; //change to not_support code
        }
        else if ( OB_SUCCESS != (ret = resolve_procedure_inner_stmt(result_plan, vector_node->children_[i], sub_query_id, ps_stmt)) )
        {
          TBSYS_LOG(WARN, "resolve do stmt [%d] failed", i);
        }
        else if ( OB_SUCCESS !=  (ret = stmt->add_do_stmt(sub_query_id)) )
        {
          TBSYS_LOG(WARN, "add do stmt failed");
        }
        else
        {
          TBSYS_LOG(INFO, "add_do_stmt stmt_id=%ld",sub_query_id);
        }
      }
    }
    else
    {
      TBSYS_LOG(WARN, "while block must contain some statement");
      ret = OB_ERR_SP_BADSTATEMENT;
    }
  }
  return ret;
}
//add hjw 20151229:e

//add by wangdonghui 20160623 :b
int resolve_procedure_exit_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt* ps_stmt)
{
  UNUSED(ps_stmt);
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_EXIT && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureExitStmt *stmt = NULL;
  TBSYS_LOG(INFO, "enter resolve_procedure_exit_stmt");
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
      TBSYS_LOG(ERROR, "resolve_procedure_exit_stmt prepare_resolve_stmt have ERROR!");
  }
  else
  {
        uint64_t expr_id;
        if (node->children_[0]==NULL)
        {
            TBSYS_LOG(DEBUG, "EXIT expr id = -1");
            expr_id = (uint64_t)-1;
            stmt->set_expr_id(expr_id);
        }
        else if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,node->children_[0],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
        {
            TBSYS_LOG(ERROR, "resolve_independ_expr  ERROR");
        }
        else if ((ret=stmt->set_expr_id(expr_id))!=OB_SUCCESS)
        {
            TBSYS_LOG(ERROR, "set_expr_id have ERROR!");
        }
    }
  return ret;
}

/**
 * @brief resolve_procedure_else_stmt
 * parse procedure's else statement syntax tree and create ObProcedureElseStmt object
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of else statement syntax tree
 * @param query_id is else_stmt's id
 * @param ps_stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_else_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id,
    ObProcedureStmt* ps_stmt)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_ELSE && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureElseStmt *stmt = NULL;
  TBSYS_LOG(TRACE, "enter resolve_procedure_else_stmt");
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(ERROR, "resolve_procedure_else_stmt prepare_resolve_stmt have ERROR!");
  }
  else if ( NULL != node->children_[0] )
  {
    ParseNode* vector_node = node->children_[0];
    TBSYS_LOG(DEBUG, "vector_node->num_child_=%d",vector_node->num_child_);

    for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
    {
      uint64_t sub_query_id = OB_INVALID_ID;

      //filter some stmt here
      if ( vector_node->children_[i]->type_ == T_PROCEDURE_DECLARE )
      {
        TBSYS_LOG(WARN, "dose not support stmt type[%d] in else branch", vector_node->children_[i]->type_);
        ret = OB_ERR_PARSE_SQL;
      }
      else if ( OB_SUCCESS != ( ret = resolve_procedure_inner_stmt(result_plan, vector_node->children_[i], sub_query_id, ps_stmt)))
      {
        TBSYS_LOG(WARN, "resolve else branch stmt fail at [%d]", i);
      }
      else if ( OB_SUCCESS != (ret = stmt->add_else_stmt(sub_query_id)))
      {
        TBSYS_LOG(WARN, "else branch body add stmt fail at [%d]", i);
      }
    }
  }
  else
  {
    TBSYS_LOG(WARN, "else block does not contain any statement");
    ret = OB_ERR_SP_BADSTATEMENT;
  }
  return ret;
}

//fix bug: add by zhujun [20150910]

/**
 * @brief resolve_procedure_create_stmt
 * parse create procedure statements syntax tree and generate a create procedure logic plan
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of create procedure statements syntax tree
 * @param query_id is index of create procedure statements stored in ObVector<ObasicStmt> stms of ObLogicalPlan class data member
 * @return errorcode.
 */
int resolve_procedure_create_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_CREATE && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  bool is_prepare_plan = result_plan->is_prepare_;
  if ( !is_prepare_plan )
  {
    ObProcedureCreateStmt *stmt = NULL; //create stmt
    if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
    {
      TBSYS_LOG(WARN, "prepare_resolve_stmt have ERROR!");
    }
    else
    {
      ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
      ObString proc_name;
      ObString proc_source_code;
      OB_ASSERT(node->children_[0]->children_[0]);
      if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->children_[0]->str_value_), proc_name)) != OB_SUCCESS)
      {
        PARSER_LOG("Can not malloc space for stmt name");
      }
      //add by wangdonghui 20160121 :b
      else if ((ret = ob_write_string(*name_pool, ObString::make_string(result_plan->input_sql_), proc_source_code)) != OB_SUCCESS)
      {
        PARSER_LOG("Can not malloc space for stmt source code");
      }
      //add :e
      else if ((ret=stmt->set_proc_name(proc_name))!=OB_SUCCESS)
      {
        TBSYS_LOG(WARN, "set_proc_name have ERROR!");
      }
      //add by wangdonghui 20160121 :b
      else if ((ret = stmt->set_proc_source_code(proc_source_code))!=OB_SUCCESS)
      {
        TBSYS_LOG(WARN, "set_proc_source_code have ERROR!");
      }
      //add :e
      else
      {
        uint64_t proc_query_id = OB_INVALID_ID;
        if ((ret = resolve_procedure_stmt(result_plan, node, proc_query_id))!=OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "resolve_procedure_stmt have ERROR!");
        }
        else
        {
          //the tree structure is built thourgh query_id ref
          ret=stmt->set_proc_id(proc_query_id); //logical plan for proc function
        }
      }
      //delete by wangdonghui 20160128 :b
//      if(ret==OB_SUCCESS)
//      {
//        /*为存储过程创建一个把存储过程源码插入到表的语句*/
//        //modified by wangdonghui 20151223 因为修改了__all_procedure表的字段，重写insert
//        ParseResult parse_result;
//        uint64_t insert_query_id = OB_INVALID_ID;
//        std::string proc_insert_sql="insert into __all_procedure values('{1}','{2}','{3}','{4}')";

//        size_t pos_1 = proc_insert_sql.find("{1}");
//        //proc_insert_sql.replace(pos_1,3,proc_name.ptr());
//        proc_insert_sql.replace(pos_1,3,node->children_[0]->children_[0]->str_value_); //proc name


//        size_t pos_2 = proc_insert_sql.find("{2}");
//        //把'替换为\'
//        TBSYS_LOG(INFO, "input sql:%s length:%lu",result_plan->input_sql_, strlen(result_plan->input_sql_));
//        char *p=new char[strlen(result_plan->input_sql_)+1000];

//        int j=0;
//        for(uint32_t i=0;i<strlen(result_plan->input_sql_);i++)
//        {
//          if(result_plan->input_sql_[i]=='\'')
//          {
//            p[j]='\\';
//            p[j+1]=result_plan->input_sql_[i];
//            j+=2;
//          }
//          else
//          {
//            p[j]=result_plan->input_sql_[i];
//            j++;
//          }
//        }
//        for(uint32_t i=j;i<strlen(p);i++)
//        {
//          p[i]='\0';
//        }
//        //add some char after @
//        replaceArray(p, "@", "@__"); //is the way to solve the variable name conflict bug?

//        TBSYS_LOG(TRACE, "p:%s j:%d length:%lu",p,j,strlen(p));

//        proc_insert_sql.replace(pos_2,3,p);

//        size_t pos_3 = proc_insert_sql.find("{3}");
//        proc_insert_sql.replace(pos_3,3,"procedure");

//        size_t pos_4 = proc_insert_sql.find("{4}");
//        proc_insert_sql.replace(pos_4,3,"");

//        ObString insertstmt=ObString::make_string(proc_insert_sql.c_str());
//        parse_result.malloc_pool_=result_plan->name_pool_;

//        TBSYS_LOG(INFO, "the insert stmt is %s", insertstmt.ptr());

//        if (OB_SUCCESS != (ret = parse_init(&parse_result)))
//        {
//          TBSYS_LOG(WARN, "parser init err");
//          ret = OB_ERR_PARSER_INIT;
//        }
//        if (parse_sql(&parse_result, insertstmt.ptr(), static_cast<size_t>(insertstmt.length())) != 0
//                        || NULL == parse_result.result_tree_)
//        {
//          TBSYS_LOG(WARN, "parser procedure insert sql err, sql is %*s",insertstmt.length(),insertstmt.ptr());
//          ret=OB_ERR_PARSE_SQL;
//        }
//        else if((ret = resolve_insert_stmt(result_plan, parse_result.result_tree_->children_[0], insert_query_id))!=OB_SUCCESS)
//        {
//          TBSYS_LOG(WARN, "resolve_insert_stmt err");
//        }
//        else
//        {
//          ret=stmt->set_proc_insert_id(insert_query_id);
//        }
//        delete p;
//      }
      //delete :e
    }
  }
  else
  {
    if ((ret = resolve_procedure_stmt(result_plan, node, query_id))!=OB_SUCCESS)
    {
      TBSYS_LOG(WARN, "resolve_procedure_stmt have ERROR!");
    }
  }
  return ret;
}

/**
 * @brief resolve_procedure_drop_stmt
 * parse drop procedure statements syntax tree and generate a drop procedure logic plan
 * @param result_plan point generated drop procedure logical plan
 * @param node is root node of drop procedure statements syntax tree
 * @param query_id is index of drop procedure statements stored in ObVector<ObasicStmt> stms of ObLogicalPlan class data member
 * @return errorcode.
 */
int resolve_procedure_drop_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  //OB_ASSERT(node && node->type_ == T_PROCEDURE_DROP && node->num_child_ == 1);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureDropStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(INFO, "prepare_resolve_stmt have ERROR!");
  }
  else
  {
	  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	  ObString proc_name;
	  OB_ASSERT(node->children_[0]);
	  if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), proc_name)) != OB_SUCCESS)
	  {
	       PARSER_LOG("Can not malloc space for stmt name");
	  }
	  else if((ret=stmt->set_proc_name(proc_name))!=OB_SUCCESS)
	  {
		  TBSYS_LOG(ERROR, "set_proc_name have ERROR!");
	  }
//delete by wdh 20160629 :b
//	  else
//	  {

//		  /*构建一个删除存储过程的逻辑结构*/
//          //modified by wangdonghui
//		  ParseResult parse_result;
//		  uint64_t delete_query_id = OB_INVALID_ID;
//          std::string proc_delete_sql="delete from __all_procedure where proc_name='{1}'";
//		  size_t pos_1 = proc_delete_sql.find("{1}");

//		  proc_delete_sql.replace(pos_1,3,node->children_[0]->str_value_);



//		  TBSYS_LOG(INFO, "proc_delete_sql is %s",proc_delete_sql.c_str());

//		  ObString deletestmt=ObString::make_string(proc_delete_sql.c_str());
//		  parse_result.malloc_pool_=result_plan->name_pool_;
//		  if (0 != (ret = parse_init(&parse_result)))
//		  {
//			  TBSYS_LOG(WARN, "parser init err");
//			  ret = OB_ERR_PARSER_INIT;
//		  }
//		  if (parse_sql(&parse_result, deletestmt.ptr(), static_cast<size_t>(deletestmt.length())) != 0
//				|| NULL == parse_result.result_tree_)
//		  {
//			  TBSYS_LOG(WARN, "parser prco delete sql err");
//		  }
//		  else if((ret = resolve_delete_stmt(result_plan, parse_result.result_tree_->children_[0], delete_query_id))!=OB_SUCCESS)
//		  {
//			  TBSYS_LOG(WARN, "resolve_delete_stmt err");
//		  }
//		  else
//		  {
//			  ret=stmt->set_proc_delete_id(delete_query_id);
//		  }
//	  }
//delete :e
	  if(node->num_child_==2)//表示用的是 DROP IF EXISTS 语法
	  {
		  stmt->set_if_exists(true);
	  }
	  else
	  {
		  stmt->set_if_exists(false);
	  }
  }
  return ret;
}

/**
 * @brief resolve_procedure_stmt
 * parse create procedure parameters and construction ObProcedureStmt and call function to parse procedure body block of begin and end
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of create procedure statements syntax tree
 * @param query_id is index of create procedure statements stored in ObVector<ObasicStmt> stms of ObLogicalPlan class data member
 * @return errorcode.
 */
int resolve_procedure_stmt(
                ResultPlan* result_plan,
                ParseNode* node,
                uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_CREATE && node->num_child_ == 2);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
    TBSYS_LOG(INFO, "prepare_resolve_stmt have ERROR!");
  }
  else
  {
    //add by wdh 20160705 :b
    stmt->set_flag(true);
    //add :e
    ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
    ObString proc_name;
    OB_ASSERT(node->children_[0]->children_[0]);
    if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->children_[0]->str_value_), proc_name)) != OB_SUCCESS)
    {
      PARSER_LOG("Can not malloc space for stmt name");
    }
    else if((ret=stmt->set_proc_name(proc_name))!=OB_SUCCESS)
    {
      TBSYS_LOG(WARN, "set_proc_name have ERROR!");
    }
    else
    {
      /*解析存储过程参数*/
      if(node->children_[0]->children_[1]!=NULL)
      {
        ParseNode* param_node=node->children_[0]->children_[1];
        OB_ASSERT(param_node->type_==T_PARAM_LIST);
        for (int32_t i = 0; ret == OB_SUCCESS && i < param_node->num_child_; i++)
        {
          ObParamDef param;
          /*参数输出类型*/
          switch(param_node->children_[i]->type_)
          {
          case T_PARAM_DEFINITION:
            TBSYS_LOG(TRACE, "param %d out type is DEFAULT_TYPE",i);
            param.out_type_=DEFAULT_TYPE;
            break;
          case T_IN_PARAM_DEFINITION:
            TBSYS_LOG(TRACE, "param %d out type is IN_TYPE",i);
            param.out_type_=IN_TYPE;
            break;
          case T_OUT_PARAM_DEFINITION:
            TBSYS_LOG(TRACE, "param %d out type is OUT_TYPE",i);
            param.out_type_=OUT_TYPE;
            break;
          case T_INOUT_PARAM_DEFINITION:
            TBSYS_LOG(TRACE, "param %d out type is INOUT_TYPE",i);
            param.out_type_=INOUT_TYPE;
            break;
          default:
            break;
          }

          if( param_node->children_[i]->children_[2] != NULL  && param_node->children_[i]->children_[2]->value_ )
          {
            param.is_array = true;
          }
          else
          {
            param.is_array = false;
          }

          /*参数数据类型*/
          switch(param_node->children_[i]->children_[1]->type_)
          {
          case T_TYPE_INTEGER:
            TBSYS_LOG(TRACE, "param %d data type is ObIntType",i);
            param.param_type_=ObIntType;
            break;
          case T_TYPE_FLOAT:
            TBSYS_LOG(TRACE, "param %d data type is ObFloatType",i);
            param.param_type_=ObFloatType;
            break;
          case T_TYPE_DOUBLE:
            TBSYS_LOG(TRACE, "param %d data type is ObDoubleType",i);
            param.param_type_=ObDoubleType;
            break;
          case T_TYPE_DECIMAL:
            TBSYS_LOG(TRACE, "param %d data type is ObDecimalType",i);
            param.param_type_=ObDecimalType;
            break;
          case T_TYPE_BOOLEAN:
            TBSYS_LOG(TRACE, "param %d data type is ObBoolType",i);
            param.param_type_=ObBoolType;
            break;
          case T_TYPE_DATETIME:
          case T_TYPE_DATE:
          case T_TYPE_TIME:
          case T_TYPE_TIMESTAMP:
            TBSYS_LOG(TRACE, "param %d data type is ObDateTimeType",i);
            param.param_type_=ObPreciseDateTimeType;
            break;
          case T_TYPE_CHARACTER:
          case T_TYPE_VARCHAR:
            TBSYS_LOG(TRACE, "param %d data type is ObVarcharType",i);
            param.param_type_=ObVarcharType;
            break;
          default:
            TBSYS_LOG(WARN, "param %d data type is not supported",i);
            ret = OB_NOT_SUPPORTED;
            break;
          }
          if( OB_SUCCESS != ret) {}
          else if((ret=ob_write_string(*name_pool, ObString::make_string(param_node->children_[i]->children_[0]->str_value_), param.param_name_))!=OB_SUCCESS)
          {
            PARSER_LOG("Can not malloc space for param name");
          }
          else if((ret=stmt->add_proc_param(param))!=OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "add_proc_param have ERROR!");
          }
        }
      }
      if( OB_SUCCESS != ret )
      {}
      else if(node->children_[1]!=NULL)
      {
        ParseNode* vector_node = node->children_[1];
        /*遍历右子树的节点*/
        ret=resolve_procedure_proc_block_stmt(result_plan, vector_node, stmt);
      }
    }
  }
  return ret;
}
/**
 * @brief resolve_procedure_proc_block_stmt
 * parse create procedure body block syntax tree and set ObProcedureStmt's proc_block_
 * @param result_plan point generated create procedure logical plan
 * @param node is root node of create procedure body block syntax tree
 * @param stmt is a pointer of ObProcedureStmt object that own ObArray<uint64_t> proc_block_ data member that store all block's statement query_id
 * @return errorcode.
 */
int resolve_procedure_proc_block_stmt(
                ResultPlan* result_plan,
                ParseNode* node,
                ObProcedureStmt *stmt
                )
{
  ParseNode *vector_node = node;
  int ret = OB_SUCCESS;
  for (int32_t i = 0; ret == OB_SUCCESS && i < vector_node->num_child_; i++)
  {
    uint64_t sub_query_id = OB_INVALID_ID;  
    TBSYS_LOG(TRACE, "resovle stmt[%d] type[%d]", i, vector_node->children_[i]->type_);
    if(vector_node->children_[i]->type_!=T_PROCEDURE_DECLARE)
    {
        stmt->set_flag(false);
    }
    switch(vector_node->children_[i]->type_)
    {
      //sql
    case T_SELECT:
      TBSYS_LOG(DEBUG, "type = T_SELECT");
      ret = resolve_select_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_DELETE:
      TBSYS_LOG(DEBUG, "type = T_DELETE");
      ret = resolve_delete_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_INSERT:
      TBSYS_LOG(DEBUG, "type = T_INSERT");
      ret = resolve_insert_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_UPDATE:
      TBSYS_LOG(DEBUG, "type = T_UPDATE");
      ret = resolve_update_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;

      //control flow
    case T_PROCEDURE_IF:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_IF");
      ret = resolve_procedure_if_stmt(result_plan, vector_node->children_[i], sub_query_id,stmt);
      break;
    case T_PROCEDURE_DECLARE:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_DECLARE");
      if(stmt->get_flag()==false)
      {
          ret = OB_ERR_SP_BADSTATEMENT;
      }
      else
      {
          ret = resolve_procedure_declare_stmt(result_plan, vector_node->children_[i], sub_query_id,stmt);
      }
      break;
    case T_PROCEDURE_ASSGIN:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_ASSGIN");
      ret = resolve_procedure_assign_stmt(result_plan, vector_node->children_[i], sub_query_id,stmt);
      break;
    case T_PROCEDURE_WHILE:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_WHILE");
      ret = resolve_procedure_while_stmt(result_plan, vector_node->children_[i], sub_query_id,stmt);
      break;
    case T_PROCEDURE_CASE:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_CASE");
      ret = resolve_procedure_case_stmt(result_plan, vector_node->children_[i], sub_query_id,stmt);
      break;
    case T_SELECT_INTO: //select and assign
      TBSYS_LOG(DEBUG, "type = T_SELECT_INTO");
      ret = resolve_procedure_select_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_PROCEDURE_LOOP:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_LOOP");
      ret = resolve_procedure_loop_stmt(result_plan, vector_node->children_[i], sub_query_id, stmt);
      break;

      //cursor support
    case T_CURSOR_DECLARE:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_DECLARE");
      ret = resolve_cursor_declare_stmt(result_plan,vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_OPEN:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_OPEN");
      ret = resolve_cursor_open_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_CLOSE:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_CLOSE");
      ret = resolve_cursor_close_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_INTO");
      ret = resolve_cursor_fetch_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_NEXT_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_NEXT_INTO");
      ret = resolve_cursor_fetch_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_PRIOR_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_PRIOR_INTO");
      ret = resolve_cursor_fetch_prior_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_FIRST_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_FIRST_INTO");
      ret = resolve_cursor_fetch_first_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_LAST_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_LAST_INTO");
      ret = resolve_cursor_fetch_last_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_ABS_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_ABS_INTO");
      ret = resolve_cursor_fetch_absolute_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;
    case T_CURSOR_FETCH_RELATIVE_INTO:
      TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_RELATIVE_INTO");
      ret = resolve_cursor_fetch_relative_into_stmt(result_plan, vector_node->children_[i], sub_query_id);
      break;

    default:
      ret=OB_ERR_PARSE_SQL;
      TBSYS_LOG(DEBUG, "type = ERROR");
      break;
    }
    if(ret==OB_SUCCESS)
    {
      if((ret=stmt->add_stmt(sub_query_id))!=OB_SUCCESS)  //here add the stmt into the procedure block
      {
        TBSYS_LOG(ERROR, "add stmt into the procedure block failed");
        break;
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "resolve_stmt error");
      break;
    }
  }
  return ret;
}


//add zt 20151128:b
/**
 * The name is an accident, better to be resolve_procedure_stmt which has be used
 * Use this function to resolve a stmt inside procedure,
 * But the caller should filter some stmt by himself
 * For example, in if-then block, there should be no declare_stmt
 ***/
/**
 * @brief resolve_procedure_inner_stmt
 * The name is an accident, better to be resolve_procedure_stmt which has be used
 * Use this function to resolve a stmt inside procedure,
 * But the caller should filter some stmt by himself
 * For example, in if-then block, there should be no declare_stmt
 * @param result_plan
 * @param node
 * @param query_id
 * @param stmt
 * @return error code
 */
int resolve_procedure_inner_stmt(
                ResultPlan *result_plan,
                ParseNode *node,
                uint64_t &query_id,
                ObProcedureStmt *stmt)
{
  int ret = OB_SUCCESS;
  switch(node->type_)
  {
  case T_SELECT:
    TBSYS_LOG(DEBUG, "type = T_SELECT");
    ret = resolve_select_stmt(result_plan, node, query_id);
    break;
  case T_DELETE:
    TBSYS_LOG(DEBUG, "type = T_DELETE");
    ret = resolve_delete_stmt(result_plan, node, query_id);
    break;
  case T_INSERT:
    TBSYS_LOG(DEBUG, "type = T_INSERT");
    ret = resolve_insert_stmt(result_plan, node, query_id);
    break;
  case T_UPDATE:
    TBSYS_LOG(DEBUG, "type = T_UPDATE");
    ret = resolve_update_stmt(result_plan, node, query_id);
    break;

    //control flow
  case T_PROCEDURE_IF:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_IF");
    ret = resolve_procedure_if_stmt(result_plan, node, query_id, stmt);
    break;
  case T_PROCEDURE_DECLARE:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_DECLARE");
    ret = resolve_procedure_declare_stmt(result_plan, node, query_id, stmt);
    break;
  case T_PROCEDURE_ASSGIN:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_ASSGIN");
    ret = resolve_procedure_assign_stmt(result_plan, node, query_id, stmt);
    break;
  case T_PROCEDURE_WHILE:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_WHILE");
    ret = resolve_procedure_while_stmt(result_plan, node, query_id, stmt);
    break;
  case T_PROCEDURE_CASE:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_CASE");
    ret = resolve_procedure_case_stmt(result_plan, node, query_id, stmt);
    break;
  case T_SELECT_INTO: //select and assign
    TBSYS_LOG(DEBUG, "type = T_SELECT_INTO");
    ret = resolve_procedure_select_into_stmt(result_plan, node, query_id);
    break;
  case T_PROCEDURE_LOOP:
    TBSYS_LOG(DEBUG, "type = T_PROCEDURE_LOOP");
    ret = resolve_procedure_loop_stmt(result_plan, node, query_id, stmt);
    break;
    //add by wangdonghui 20160623 :b
    case T_PROCEDURE_EXIT:
      TBSYS_LOG(DEBUG, "type = T_PROCEDURE_EXIT");
      ret = resolve_procedure_exit_stmt(result_plan, node, query_id, stmt);
      break;
    //add :e
    //cursor support
  case T_CURSOR_DECLARE:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_DECLARE");
    ret = resolve_cursor_declare_stmt(result_plan,node, query_id);
    break;
  case T_CURSOR_OPEN:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_OPEN");
    ret = resolve_cursor_open_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_CLOSE:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_CLOSE");
    ret = resolve_cursor_close_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_INTO");
    ret = resolve_cursor_fetch_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_NEXT_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_NEXT_INTO");
    ret = resolve_cursor_fetch_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_PRIOR_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_PRIOR_INTO");
    ret = resolve_cursor_fetch_prior_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_FIRST_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_FIRST_INTO");
    ret = resolve_cursor_fetch_first_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_LAST_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_LAST_INTO");
    ret = resolve_cursor_fetch_last_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_ABS_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_ABS_INTO");
    ret = resolve_cursor_fetch_absolute_into_stmt(result_plan, node, query_id);
    break;
  case T_CURSOR_FETCH_RELATIVE_INTO:
    TBSYS_LOG(DEBUG, "type = T_CURSOR_FETCH_RELATIVE_INTO");
    ret = resolve_cursor_fetch_relative_into_stmt(result_plan, node, query_id);
    break;

  default:
    ret=OB_ERR_PARSE_SQL;
    TBSYS_LOG(ERROR, "could not resovle type[%d] in procedure", node->type_);
    break;
  }
  return ret;
}
//add zt 20151128:e

/**
 * @brief resolve_procedure_execute_stmt
 * parse call procedure statements syntax tree and generate a call procedure logic plan
 * @param result_plan point generated logical plan
 * @param node is root node of call procedure statements syntax tree
 * @param query_id is index of call procedure statements stored in ObVector<ObasicStmt> stms of ObLogicalPlan class data member
 * @return errorcode.
 */
int resolve_procedure_execute_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  OB_ASSERT(result_plan);
  OB_ASSERT(node && node->type_ == T_PROCEDURE_EXEC && node->num_child_ == 3);
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ObProcedureExecuteStmt *stmt = NULL;
  if (OB_SUCCESS != (ret = prepare_resolve_stmt(result_plan, query_id, stmt)))
  {
	  TBSYS_LOG(INFO, "prepare_resolve_stmt have ERROR!");
  }
  else
  {
	  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
	  ObString proc_name;
	  if ((ret = ob_write_string(*name_pool, ObString::make_string(node->children_[0]->str_value_), proc_name)) != OB_SUCCESS)
	  {
	       PARSER_LOG("Can not malloc space for stmt name");
	  }
	  else if((ret=stmt->set_proc_name(proc_name))!=OB_SUCCESS)
	  {
		  TBSYS_LOG(ERROR, "set_proc_name have ERROR!");
	  }
	  else
	  {
		  ParseNode *arguments = node->children_[1];
		  if(arguments!=NULL)
		  {
			  for (int32_t i = 0;i < arguments->num_child_; i++)
			  {
					/*
					ObString name;
					if ((ret = ob_write_string(*name_pool, ObString::make_string(arguments->children_[i]->str_value_), name)) != OB_SUCCESS)
					{
						PARSER_LOG("Resolve variable %s error", arguments->children_[i]->str_value_);
					}
					else if ((ret = stmt->add_variable_name(name)) != OB_SUCCESS)
					{
						PARSER_LOG("Add Using variable failed");
					}*/
					uint64_t expr_id;

                    if ((ret = resolve_independ_expr(result_plan,(ObStmt*)stmt,arguments->children_[i],expr_id,T_VARIABLE_VALUE_LIMIT))!= OB_SUCCESS)
					{
						TBSYS_LOG(ERROR, "resolve_independ_expr  ERROR");
					}
					else
					{
						stmt->add_param_expr(expr_id);
                    }
            }
          }
          //add by wdh 20160716 :b
//          if(ret == OB_SUCCESS && node->children_[2]!=NULL)
//          {
//            OB_ASSERT(node->children_[2]->children_[0]->type_ == T_NO_GROUP);
//            ret = stmt->set_no_group(true);
//            TBSYS_LOG(DEBUG,"www: no_group is %d", true);
//          }
//          else if(ret == OB_SUCCESS && node->children_[2]== NULL)
//          {
//              stmt->set_no_group(false);
//              TBSYS_LOG(DEBUG,"www: no_group is null, no_group is %d", false);
//          }
          //add :e
         //modify by qx 20170317 :b
         if (ret == OB_SUCCESS && node->children_[2]!= NULL)
         {
           for (int32_t i = 0;i < node->children_[2]->num_child_; i++)
           {
             if (node->children_[2]->children_[i]->type_ == T_NO_GROUP)
             {
               ret = stmt->set_no_group(true);
               TBSYS_LOG(ERROR,"www: no_group is %d", true);
             }
             else if (node->children_[2]->children_[i]->type_ == T_LONG_TRANS)
             {
               ret = stmt->set_long_trans(true);
               TBSYS_LOG(ERROR,"www: long_trans is %d", true);
             }
             else
             {
              // shouldn't go here
               TBSYS_LOG(ERROR,"www: find ET !");
             }
           }

         }
         else if (ret == OB_SUCCESS && node->children_[2]== NULL)
         {
           stmt->set_no_group(false);
           TBSYS_LOG(DEBUG,"www: no_group is null, no_group is %d", false);
           stmt->set_long_trans(false);
           TBSYS_LOG(DEBUG,"www: long_trans is null, long_trans is %d", false);
         }
         //add :e
      }
  }
  return ret;
}
//code_coverage_zhujun
//add:e

//add wangjiahao [table lock] 20160616 :b
int resolve_lock_table_stmt(
    ResultPlan* result_plan,
    ParseNode* node,
    uint64_t& query_id)
{
  int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
  ParseNode *lock_table_node = NULL;
  OB_ASSERT(node && node->type_ == T_LOCK_TABLE);
  query_id = OB_INVALID_ID;

  ObLogicalPlan* logical_plan = NULL;
  ObStringBuf* name_pool = static_cast<ObStringBuf*>(result_plan->name_pool_);
  if (result_plan->plan_tree_ == NULL)
  {
    logical_plan = (ObLogicalPlan*)parse_malloc(sizeof(ObLogicalPlan), result_plan->name_pool_);
    if (logical_plan == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not malloc ObLogicalPlan");
    }
    else
    {
      logical_plan = new(logical_plan) ObLogicalPlan(name_pool);
      result_plan->plan_tree_ = logical_plan;
    }
  }
  else
  {
    logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
  }

  if (ret == OB_SUCCESS)
  {
    ObLockTableStmt* lock_table_stmt = (ObLockTableStmt*)parse_malloc(sizeof(ObLockTableStmt), result_plan->name_pool_);
    if (lock_table_stmt == NULL)
    {
      ret = OB_ERR_PARSER_MALLOC_FAILED;
      snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not malloc ObLockTableStmt");
    }
    else
    {
      //ParseNode sys_table_name;
      //sys_table_name.type_ = T_IDENT;

      OB_ASSERT(node->num_child_ == 1);
      lock_table_node = node->children_[0];
      lock_table_stmt = new(lock_table_stmt) ObLockTableStmt(name_pool);
      //sys_table_name.str_value_ = OB_TABLES_SHOW_TABLE_NAME;
      query_id = logical_plan->generate_query_id();
      lock_table_stmt->set_query_id(query_id);

      if (ret == OB_SUCCESS && (ret = logical_plan->add_query(lock_table_stmt)) != OB_SUCCESS)
      {
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
          "Can not add ObLockTableStmt to logical plan");
      }
      if (ret != OB_SUCCESS && lock_table_stmt != NULL)
      {
        lock_table_stmt->~ObLockTableStmt();
      }
    }

    if (OB_SUCCESS == ret)
    {
      OB_ASSERT(lock_table_node);
      ObSchemaChecker *schema_checker = static_cast<ObSchemaChecker*>(result_plan->schema_checker_);
      if (schema_checker == NULL)
      {
        ret = OB_ERR_SCHEMA_UNSET;
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG, "Schema(s) are not set");
      }
      int32_t len = static_cast<int32_t>(strlen(lock_table_node->str_value_));
      ObString table_name(len, len, lock_table_node->str_value_);
      uint64_t lock_table_id = schema_checker->get_table_id(table_name);
      if (lock_table_id == OB_INVALID_ID)
      {
        ret = OB_ERR_TABLE_UNKNOWN;
        snprintf(result_plan->err_stat_.err_msg_, MAX_ERROR_MSG,
            "Unknown table \"%s\"", lock_table_node->str_value_);
      }
      else
      {
        lock_table_stmt->set_lock_table_id(lock_table_id);
      }
    }
  }
  return ret;

}
//add :e
////////////////////////////////////////////////////////////////
/**
 * @brief resolve
 * parse syntax tree and generate logic plan
 * @param result_plan point generated logical plan
 * @param node is root node of syntax tree
 * @return error code.
 */
int resolve(ResultPlan* result_plan, ParseNode* node)
{
	if (!result_plan)
	{
		TBSYS_LOG(ERROR, "null result_plan");
		return OB_ERR_RESOLVE_SQL;
	}
	int& ret = result_plan->err_stat_.err_code_ = OB_SUCCESS;
	if (ret == OB_SUCCESS && result_plan->name_pool_ == NULL)
	{
		ret = OB_ERR_RESOLVE_SQL;
		PARSER_LOG("name_pool_ nust be set");
	}
	if (ret == OB_SUCCESS && result_plan->schema_checker_ == NULL)
	{
		ret = OB_ERR_RESOLVE_SQL;
		PARSER_LOG("schema_checker_ must be set");
	}

	if (OB_LIKELY(OB_SUCCESS == ret))
	{
		bool is_preparable = false;
		switch (node->type_)
		{
		case T_STMT_LIST:
		case T_SELECT:
		case T_DELETE:
		case T_INSERT:
		case T_UPDATE:
		case T_BEGIN:
		case T_COMMIT:
		case T_ROLLBACK:
        case T_PROCEDURE_CREATE:  //add zt 20151119, a hint used when generate and cache procedure plan
			is_preparable = true;
			break;
		default:
			break;
		}
		if (result_plan->is_prepare_ && !is_preparable)
		{
			ret = OB_ERR_RESOLVE_SQL;
			PARSER_LOG("the statement can not be prepared");
		}
	}

	uint64_t query_id = OB_INVALID_ID;
	if (ret == OB_SUCCESS && node != NULL)
	{
		switch (node->type_)
		{
		case T_STMT_LIST:
		{
			ret = resolve_multi_stmt(result_plan, node);
			break;
		}
		case T_SELECT:
		{
			ret = resolve_select_stmt(result_plan, node, query_id);
			break;
		}
		case T_DELETE:
		{
			ret = resolve_delete_stmt(result_plan, node, query_id);
			break;
		}
		case T_INSERT:
		{
			ret = resolve_insert_stmt(result_plan, node, query_id);
			break;
		}
		case T_UPDATE:
		{
			ret = resolve_update_stmt(result_plan, node, query_id);
			break;
		}
		case T_EXPLAIN:
		{
			ret = resolve_explain_stmt(result_plan, node, query_id);
			break;
		}
		case T_CREATE_TABLE:
		{
			ret = resolve_create_table_stmt(result_plan, node, query_id);
			break;
		}
		case T_DROP_TABLE:
		{
			ret = resolve_drop_table_stmt(result_plan, node, query_id);
			break;
		}
		case T_ALTER_TABLE:
		{
			ret = resolve_alter_table_stmt(result_plan, node, query_id);
			break;
		}
        //add hxlong [truncate table] 20170403:b //
        case T_TRUNCATE_TABLE:
        {
            ret = resolve_truncate_table_stmt(result_plan ,node ,query_id);
            break;
        }
        //add:e
		//secondary index
		//longfei [create index]
		case T_CREATE_INDEX:
		{
			ret = resolve_create_index_stmt(result_plan, node, query_id);
			break;
		}
		//longfei [drop index]
        case T_DROP_INDEX:
        {
        ret = resolve_drop_index_stmt(result_plan, node, query_id);
        break;
        }
		case T_SHOW_TABLES:
		// add longfei [show index] 20151019
		case T_SHOW_INDEX:
		// add e
		case T_SHOW_VARIABLES:
		case T_SHOW_COLUMNS:
		case T_SHOW_SCHEMA:
		case T_SHOW_CREATE_TABLE:
		case T_SHOW_TABLE_STATUS:
		case T_SHOW_SERVER_STATUS:
		case T_SHOW_WARNINGS:
		case T_SHOW_GRANTS:
		case T_SHOW_PARAMETERS:
		case T_SHOW_PROCESSLIST :
		{
			ret = resolve_show_stmt(result_plan, node, query_id);
			break;
		}
		case T_CREATE_USER:
		{
			ret = resolve_create_user_stmt(result_plan, node, query_id);
			break;
		}
		case T_DROP_USER:
		{
			ret = resolve_drop_user_stmt(result_plan, node, query_id);
			break;
		}
		case T_SET_PASSWORD:
		{
			ret = resolve_set_password_stmt(result_plan, node, query_id);
			break;
		}
		case T_RENAME_USER:
		{
			ret = resolve_rename_user_stmt(result_plan, node, query_id);
			break;
		}
		case T_LOCK_USER:
		{
			ret = resolve_lock_user_stmt(result_plan, node, query_id);
			break;
		}
		case T_GRANT:
		{
			ret = resolve_grant_stmt(result_plan, node, query_id);
			break;
		}
		case T_REVOKE:
		{
			ret = resolve_revoke_stmt(result_plan, node, query_id);
			break;
		}
		//zhounan unmark:b
      case T_CURSOR_DECLARE:
      {
        ret = resolve_cursor_declare_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_OPEN:
      {
        ret = resolve_cursor_open_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH:
      {
        ret = resolve_cursor_fetch_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_NEXT:
            {
              ret = resolve_cursor_fetch_stmt(result_plan, node, query_id);
              break;
            }
      case T_CURSOR_FETCH_INTO:
      {
        ret = resolve_cursor_fetch_into_stmt(result_plan, node, query_id);
        break;
            }
      case T_CURSOR_FETCH_NEXT_INTO:
            {
              ret = resolve_cursor_fetch_into_stmt(result_plan, node, query_id);
              break;
                  }
      case T_CURSOR_FETCH_PRIOR:
      {
        ret = resolve_cursor_fetch_prior_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_PRIOR_INTO:
      {
        ret = resolve_cursor_fetch_prior_into_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_FIRST:
      {
        ret = resolve_cursor_fetch_first_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_FIRST_INTO:
      {
        ret = resolve_cursor_fetch_first_into_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_LAST:
      {
        ret = resolve_cursor_fetch_last_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_LAST_INTO:
      {
        ret = resolve_cursor_fetch_last_into_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_RELATIVE:
      {
        ret = resolve_cursor_fetch_relative_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_RELATIVE_INTO:
      {
        ret = resolve_cursor_fetch_relative_into_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_FETCH_ABSOLUTE:
      {
        ret = resolve_cursor_fetch_absolute_stmt(result_plan, node, query_id);
         break;
      }
      case T_CURSOR_FETCH_ABS_INTO:
      {
        ret = resolve_cursor_fetch_absolute_into_stmt(result_plan, node, query_id);
         break;
      }
      case T_CURSOR_FETCH_FROMTO:
      {
        ret = resolve_cursor_fetch_fromto_stmt(result_plan, node, query_id);
        break;
      }
      case T_CURSOR_CLOSE:
      {
        ret = resolve_cursor_close_stmt(result_plan, node, query_id);

    	break;
      }
	  //add:e
		case T_PREPARE:
		{
			ret = resolve_prepare_stmt(result_plan, node, query_id);
			break;
		}
		case T_VARIABLE_SET:
		{
			ret = resolve_variable_set_stmt(result_plan, node, query_id);
			break;
		}
		case T_EXECUTE:
		{
			ret = resolve_execute_stmt(result_plan, node, query_id);
			break;
		}
    //add by zhujun:b
    //code_coverage_zhujun
    case T_VAR_ARRAY_VAL:
    {
      ret = resolve_variable_set_array_stmt(result_plan, node, query_id);
      break;
    }
    case T_PROCEDURE_CREATE:
    {
       ret = resolve_procedure_create_stmt(result_plan, node, query_id);
       break;
    }
    case T_PROCEDURE_DROP:
		{
		   ret = resolve_procedure_drop_stmt(result_plan, node, query_id);
		   break;
		}
    case T_PROCEDURE_EXEC:
    {
      ret = resolve_procedure_execute_stmt(result_plan, node, query_id);
      break;
    }
    //code_coverage_zhujun
	  //add:e
		case T_DEALLOCATE:
		{
			ret = resolve_deallocate_stmt(result_plan, node, query_id);
			break;
		}
		case T_BEGIN:
			ret = resolve_start_trans_stmt(result_plan, node, query_id);
			break;
		case T_COMMIT:
			ret = resolve_commit_stmt(result_plan, node, query_id);
			break;
		case T_ROLLBACK:
			ret = resolve_rollback_stmt(result_plan, node, query_id);
			break;
		case T_ALTER_SYSTEM:
			ret = resolve_alter_sys_cnf_stmt(result_plan, node, query_id);
			break;
		case T_KILL:
			ret = resolve_kill_stmt(result_plan, node, query_id);
			break;
		case T_CHANGE_OBI:
			ret = resolve_change_obi(result_plan, node, query_id);
			break;
    //add wangjiahao [table lock] 20160616 :b
    case T_LOCK_TABLE:
      ret = resolve_lock_table_stmt(result_plan, node, query_id);
      break;
    //add :e
		default:
			TBSYS_LOG(ERROR, "unknown top node type=%d", node->type_);
			ret = OB_ERR_UNEXPECTED;
			break;
		};
	}
	if (ret == OB_SUCCESS && result_plan->is_prepare_ != 1
			&& node->type_ != T_STMT_LIST && node->type_ != T_PREPARE)
	{
		ObLogicalPlan* logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
		if (logical_plan != NULL && logical_plan->get_question_mark_size() > 0)
		{
			ret = OB_ERR_PARSE_SQL;
			PARSER_LOG("Uknown column '?'");
		}
	}
	if (ret != OB_SUCCESS && result_plan->plan_tree_ != NULL)
	{
		ObLogicalPlan* logical_plan = static_cast<ObLogicalPlan*>(result_plan->plan_tree_);
		logical_plan->~ObLogicalPlan();
		parse_free(result_plan->plan_tree_);
		result_plan->plan_tree_ = NULL;
	}
	return ret;
}
/**
 * @brief destroy_plan
 * destroy a logic plan free memory
 * @param result_plan point a logical plan that want to be destroyed
 * @return
 */
extern void destroy_plan(ResultPlan* result_plan)
{
	if (result_plan->plan_tree_ == NULL)
		return;

	//delete (static_cast<multi_plan*>(result_plan->plan_tree_));
	parse_free(result_plan->plan_tree_);

	result_plan->plan_tree_ = NULL;
	result_plan->name_pool_ = NULL;
	result_plan->schema_checker_ = NULL;
}
