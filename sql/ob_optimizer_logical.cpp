#include <map>
#include "sql/ob_optimizer_logical.h"
#include "sql/ob_select_stmt.h"
#include "sql/ob_explain_stmt.h"
#include "common/hash/ob_hashmap.h"
#include "common/ob_vector.h"
#include "parse_malloc.h"


namespace oceanbase 
{
  namespace sql 
  {
    
    /*
     * @brief decide whether sub query is a simple subquery or not  
     * @return 
     */
    bool ObOptimizerLogical::is_simple_subquery(ObSelectStmt *select_stmt)
    { 
      
      // make sure it's a valid subselect
      if(!(select_stmt->get_stmt_type() == ObSelectStmt::T_SELECT))
        return false;
      
      // can't pull up a query with for update
      if(select_stmt->is_for_update())
        return false;
      
      // can't pull up a query with setops
      if(select_stmt->get_set_op() != ObSelectStmt::NONE)
        return false;
      
      /*
       * can't pull up a subquery involving grouping aggregation, sorting,
       * limiting, for update, distinct 
       */
      if(select_stmt->has_limit() || 
         select_stmt->is_distinct() ||
//         select_stmt->is_set_distinct() ||
         select_stmt->is_show_stmt() ||
         select_stmt->has_limit() ||
         select_stmt->get_group_expr_size()>0 || 
         select_stmt->get_agg_fun_size()>0 ||
         select_stmt->get_having_expr_size()>0 ||
         select_stmt->get_order_item_size()>0)
        return false;
      
      return true;
    }
    
    /*
     * @brief pull up subquery
     * @return 
     */
    int ObOptimizerLogical::pull_up_subqueries(ObLogicalPlan *&logical_plan, ObResultSet &result, ResultPlan &result_plan)
    {
      
      int ret = OB_SUCCESS;
      uint64_t query_id = common::OB_INVALID_ID;
      ObSelectStmt *main_stmt = NULL;
      
      if(logical_plan)
      {
        int32_t stmt_size = logical_plan->get_stmts_count();
        
        // if stmt size > 1, so has subquery
        if (stmt_size > 1) 
        {
          if(query_id == OB_INVALID_ID)
          {
            // explain stmt is different from select stmt
            ObBasicStmt *basic_stmt = logical_plan->get_main_stmt();
            if (basic_stmt->get_stmt_type() == ObBasicStmt::T_EXPLAIN)
            {
              ObExplainStmt *explain_stmt = dynamic_cast<ObExplainStmt*>(basic_stmt);
              main_stmt = dynamic_cast<ObSelectStmt*>(
                    logical_plan->get_query(explain_stmt->get_explain_query_id()));
            }
            else
            {
              main_stmt = dynamic_cast<ObSelectStmt*>(basic_stmt);
            }
          }
          
          if(main_stmt == NULL){
            ret = OB_ERR_ILLEGAL_ID;
            TBSYS_LOG(INFO, "Stmt is not select stmt, so it will not enter the logical optimizer");
          } 
          else if(OB_LIKELY(ret == OB_SUCCESS))
          {
            TBSYS_LOG(INFO, "enter the logical optimizer");
            ret = pull_up_subquery_union(logical_plan, main_stmt, result_plan);
            TBSYS_LOG(WARN, "leave the logical optimizer");
          }
        }
      }
      
      UNUSED(result);
      
      return ret;
    }
    
    /*
     * pull_up_subquery_union
     */ 
    int ObOptimizerLogical::pull_up_subquery_union(ObLogicalPlan *&logical_plan, ObSelectStmt *&main_stmt, ResultPlan &result_plan)
    {
      int ret = OB_SUCCESS;
      
      if (main_stmt->get_set_op() != ObSelectStmt::NONE) 
      {
        uint64_t left_query_id = main_stmt->get_left_query_id();
        uint64_t right_query_id = main_stmt->get_right_query_id();
        
        ObSelectStmt* left_stmt = dynamic_cast<ObSelectStmt*>(logical_plan->get_query(left_query_id));
        ObSelectStmt* right_stmt = dynamic_cast<ObSelectStmt*>(logical_plan->get_query(right_query_id));
        
        int32_t left_from_item_size = left_stmt->get_from_item_size();
        for (int32_t i = 0; i < left_from_item_size; ++i) 
        {
          FromItem& from_item = const_cast<FromItem&>(left_stmt->get_from_item(i));
          pull_up_subquery_recursion(logical_plan, left_stmt, from_item, result_plan);
        }
        
        int32_t right_from_item_size = right_stmt->get_from_item_size();
        for (int32_t i = 0; i < right_from_item_size; ++i) 
        {
          FromItem& from_item = const_cast<FromItem&>(right_stmt->get_from_item(i));
          pull_up_subquery_recursion(logical_plan, right_stmt, from_item, result_plan);
        }
        
        // Old select items is the left stmt's select items.
        common::ObVector<SelectItem>& main_select_items = main_stmt->get_select_items();
        common::ObVector<SelectItem>& left_select_items = left_stmt->get_select_items();
        for (int32_t i = 0; i < main_select_items.size(); ++i) 
        {
          main_select_items.at(i) = left_select_items.at(i);
        }
      } 
      else 
      {
        int32_t from_item_size = main_stmt->get_from_item_size();
        for (int32_t i = 0; i < from_item_size; ++i) 
        {
          FromItem& from_item = const_cast<FromItem&>(main_stmt->get_from_item(i));
          pull_up_subquery_recursion(logical_plan, main_stmt, from_item, result_plan);
        }
      }
      
      return ret;
    }
    
    /*
     * pull_up_subquery_recursion
     */ 
    int ObOptimizerLogical::pull_up_subquery_recursion(ObLogicalPlan *&logical_plan, ObSelectStmt *&main_stmt, FromItem& from_item, ResultPlan &result_plan)
    {
      int ret = OB_SUCCESS;
      
      uint64_t table_id = from_item.table_id_;
      bool is_joined = from_item.is_joined_;
      JoinedTable* joined_table = NULL;
      uint64_t join_type = 10;
      
      if(is_joined) // the connection type of tables is explicit join ( inner join ... ), not , connect . 
      {
        joined_table = main_stmt->get_joined_table(table_id);
        for (int64_t i = 0; i < joined_table->table_ids_.count(); ++i) 
        {
          uint64_t joined_table_id = joined_table->table_ids_.at(i);
          TableItem* table_item = main_stmt->get_table_item_by_id(joined_table_id);
          
          // this table type represent subquery
          if (table_item->type_ == TableItem::GENERATED_TABLE) 
          {
            // this type of sql can not be pulled up : t1 left join (t2 inner join t3)
            if (i > 0 && joined_table->get_join_types().at(i-1) != JoinedTable::T_INNER) 
            {
              join_type = joined_table->get_join_types().at(i-1);
            }
            
            uint64_t ref_id = table_item->ref_id_;
            ObSelectStmt* subquery_stmt = dynamic_cast<ObSelectStmt*>(logical_plan->get_query(ref_id));
            if(subquery_stmt == NULL)
            {
              ret = OB_ERR_ILLEGAL_ID;
              TBSYS_LOG(ERROR, "Get Stmt error");
              
            } 
            else if(OB_LIKELY(ret == OB_SUCCESS))
            {
              // pull up subquery execute
              ret = pull_up_simple_subquery(logical_plan, main_stmt, from_item, table_item, joined_table, join_type, result_plan);
            } 
            else 
            {
              // if it is not sub query, then continue execute.
            }
          }
        }
      }
      else
      {
        TableItem* table_item = main_stmt->get_table_item_by_id(table_id);
        // this table type represent subquery
        if (table_item->type_ == TableItem::GENERATED_TABLE) 
        {
          uint64_t ref_id = table_item->ref_id_;
          ObSelectStmt* subquery_stmt = dynamic_cast<ObSelectStmt*>(logical_plan->get_query(ref_id));
          if(subquery_stmt == NULL)
          {
            ret = OB_ERR_ILLEGAL_ID;
            TBSYS_LOG(ERROR, "Get Stmt error");
            
          } 
          else if(OB_LIKELY(ret == OB_SUCCESS))
          {
            // pull up subquery execute
            ret = pull_up_simple_subquery(logical_plan, main_stmt, from_item, table_item, joined_table, join_type, result_plan);
          } 
          else 
          {
            // if it is not sub query, then continue execute.
          }
        }
      }
      
      return ret;
    }
    
    /*
     * if_can_pull_up
     */
    int ObOptimizerLogical::if_can_pull_up(ObLogicalPlan *&logical_plan, ObSelectStmt* subquery_stmt, JoinedTable *&joined_table, uint64_t join_type, bool is_joined, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      int32_t sub_from_size = subquery_stmt->get_from_item_size();
      for (int32_t i = 0; i < sub_from_size; ++i) 
      {
        FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(i));
        bool sub_is_joined = sub_from_item.is_joined_;
        
        if (is_joined) 
        {
          if (sub_is_joined) 
          {
            JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
            
            // To judge whether the sql can pull up, 10 define before.
            if (join_type != 10) // father join is out join 
            {
              if (join_type == JoinedTable::T_FULL && sub_joined_table->get_table_ids().count()>1) 
              {
                ret = OB_SQL_CAN_NOT_PULL_UP;
              } 
              else 
              {
                for (int64_t j = 0; j < sub_joined_table->get_join_types().count(); ++j)
                {
                  if (sub_joined_table->get_join_types().at(j) == JoinedTable::T_INNER) 
                  {
                    ret = OB_SQL_CAN_NOT_PULL_UP;
                    break;
                  }
                }
              }
              
              if (ret != OB_SUCCESS) 
              {
                TBSYS_LOG(WARN, "SQL can not be pulled up");
                break;
              }
            }
            else 
            {
              if (table_id != joined_table->get_table_ids().at(0)) 
              {
                for (int64_t j = 0; j < sub_joined_table->get_join_types().count(); ++j)
                {
                  if (sub_joined_table->get_join_types().at(j) != JoinedTable::T_INNER) 
                  {
                    ret = OB_SQL_CAN_NOT_PULL_UP;
                    break;
                  }
                }
              }
              
              if (ret != OB_SUCCESS) 
              {
                TBSYS_LOG(WARN, "SQL can not be pulled up");
                break;
              }
            }
          }
          else 
          {
            if (join_type !=10) 
            {
              if (sub_from_size>1) 
              {
                ret = OB_SQL_CAN_NOT_PULL_UP;
                TBSYS_LOG(WARN, "SQL can not be pulled up");
                break;
              }
            }
          }
        } 
      }
      
      if (ret == OB_SUCCESS) 
      {
        int32_t sub_select_item_size = subquery_stmt->get_select_item_size();
        for (int32_t i = 0; i < sub_select_item_size; ++i) 
        {
          SelectItem& sub_select_item = const_cast<SelectItem&>(subquery_stmt->get_select_item(i));
          ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_select_item.expr_id_);
          ObRawExpr* raw_expr = sql_raw_expr->get_expr();
          ObItemType item_type = raw_expr->get_expr_type();
          if (item_type != T_REF_COLUMN) 
          {
            ret = OB_SQL_CAN_NOT_PULL_UP;
            TBSYS_LOG(WARN, "SQL can not be pulled up");
            break;
          }
        }
      }
      
      return ret;
    }
    
    /*
     * combine_subquery_from_items
     */
    int ObOptimizerLogical::combine_subquery_from_items(ObLogicalPlan *&logical_plan, 
          ObSelectStmt* subquery_stmt, ResultPlan &result_plan, bool is_joined)
    {
      int ret = OB_SUCCESS;
      
      int32_t sub_from_size = subquery_stmt->get_from_item_size();
      
      // if is_joined is true, the sub query' from item should combine to join item.
      if (is_joined && sub_from_size>1) 
      {
        common::ObVector<uint64_t> join_exprs;
        
        for (int32_t i = 0; i < sub_from_size; ++i) 
        {
          FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(i));
          bool sub_is_joined = sub_from_item.is_joined_;
          
          if (sub_is_joined) 
          {
            JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
            common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
            for (int64_t j = 0; j < sub_joined_table_ids.count(); ++j) 
            {
              common::ObBitSet<> table_bitset;
              int32_t bit_index = subquery_stmt->get_table_bit_index(sub_joined_table_ids.at(j));
              table_bitset.add_member(bit_index);
              
              for (int32_t k = 0; k < subquery_stmt->get_condition_size(); ++k)  // 过滤非连接条件
              {
                uint64_t sub_where_exprs_id = subquery_stmt->get_condition_id(k);
                ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_where_exprs_id);
                common::ObBitSet<>& bit_set = sql_raw_expr->get_tables_set();
                if (table_bitset.is_superset(bit_set)) 
                {
                  sql_raw_expr->set_applied(true);
                }
              }
            }
          } 
          else 
          {
            common::ObBitSet<> table_bitset;
            int32_t bit_index = subquery_stmt->get_table_bit_index(sub_from_item.table_id_);
            table_bitset.add_member(bit_index);
            
            for (int32_t j = 0; j < subquery_stmt->get_condition_size(); ++j) 
            {
              uint64_t sub_where_exprs_id = subquery_stmt->get_condition_id(j);
              ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_where_exprs_id);
              common::ObBitSet<>& bit_set = sql_raw_expr->get_tables_set();
              if (table_bitset.is_superset(bit_set)) 
              {
                sql_raw_expr->set_applied(true);
              }
            }
          }
        }
        
        common::ObVector<uint64_t>& sub_where_exprs = subquery_stmt->get_where_exprs();
        int32_t sub_condition_size = subquery_stmt->get_condition_size();
        for (int32_t i = sub_condition_size-1; i >= 0; --i) 
        {
          uint64_t sub_where_exprs_id = subquery_stmt->get_condition_id(i);
          ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_where_exprs_id);
          if (!(sql_raw_expr->is_apply())) 
          {
            join_exprs.push_back(sub_where_exprs_id);
            sub_where_exprs.remove(i);
          }
        }
        
        // sub query's from table transfer to join item
        JoinedTable* new_joined_table = (JoinedTable*)parse_malloc(sizeof(JoinedTable), result_plan.name_pool_);
        new_joined_table = new(new_joined_table) JoinedTable;
        
        FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(0));
        bool sub_is_joined = sub_from_item.is_joined_;
        if (sub_is_joined) 
        {
          JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
          common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
          common::ObArray<uint64_t>& sub_expr_ids = sub_joined_table->get_expr_ids();
          for (int64_t i = 0; i < sub_joined_table_ids.count(); ++i) 
          {
            new_joined_table->add_table_id(sub_joined_table_ids.at(i));
            if (i>0) 
            {
              uint64_t sub_join_type = sub_joined_table->get_join_types().at(i-1);
              switch (sub_join_type) 
              {
              case JoinedTable::T_FULL:
                new_joined_table->add_join_type(JoinedTable::T_FULL);
                break;
              case JoinedTable::T_INNER:
                new_joined_table->add_join_type(JoinedTable::T_INNER);
                break;
              case JoinedTable::T_LEFT:
                new_joined_table->add_join_type(JoinedTable::T_LEFT);
                break;
              case JoinedTable::T_RIGHT:
                new_joined_table->add_join_type(JoinedTable::T_RIGHT);
                break;
              default:
                break;
              }
              new_joined_table->add_expr_id(sub_expr_ids.at(i-1));
              
              sub_expr_ids.at(i-1) = 0;
            }
          }
        }
        else 
        {
          new_joined_table->add_table_id(sub_from_item.table_id_);
        }
        
        for (int64_t i = 0; i < new_joined_table->get_table_ids().count(); ++i) 
        {
          uint64_t new_table_id = new_joined_table->get_table_ids().at(i);
          
          common::ObBitSet<> table_bitset;
          int32_t bit_index = subquery_stmt->get_table_bit_index(new_table_id);
          table_bitset.add_member(bit_index);
          
          int32_t join_exprs_size = join_exprs.size();
          for (int32_t j = join_exprs_size-1; j >= 0; --j) 
          {
            ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(join_exprs.at(j));
            common::ObBitSet<>& bit_set = sql_raw_expr->get_tables_set();
            if (table_bitset.is_subset(bit_set)) 
            {
              ObRawExpr* raw_expr = sql_raw_expr->get_expr();
              
              ObBinaryOpRawExpr* binary_op_raw_expr = dynamic_cast<ObBinaryOpRawExpr*>(raw_expr);
              ObBinaryRefRawExpr* left_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_first_op_expr());
              ObBinaryRefRawExpr* right_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_second_op_expr());
              
              uint64_t left_first_ref_id = left_expr->get_first_ref_id(); // table id
              uint64_t right_first_ref_id = right_expr->get_first_ref_id();
              
              if (left_first_ref_id == new_table_id) 
              {
                new_joined_table->add_table_id(right_first_ref_id);
              }
              else if (right_first_ref_id == new_table_id) 
              {
                new_joined_table->add_table_id(left_first_ref_id);
              }
              new_joined_table->add_join_type(JoinedTable::T_INNER);
              new_joined_table->add_expr_id(join_exprs.at(j)); 
              
              join_exprs.remove(j);
            }
          }
          
          for (int32_t j = 0; j < sub_from_size; ++j) 
          {
            FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(j));
            bool sub_is_joined = sub_from_item.is_joined_;
            if (sub_is_joined) 
            {
              JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
              common::ObArray<uint64_t>& sub_expr_ids = sub_joined_table->get_expr_ids();
              for (int64_t k = 0; k < sub_expr_ids.count(); ++k) 
              {
                if (sub_expr_ids.at(k) > 0) 
                {
                  ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_expr_ids.at(k));
                  common::ObBitSet<>& bit_set = sql_raw_expr->get_tables_set();
                  if (table_bitset.is_subset(bit_set)) 
                  {
                    ObRawExpr* raw_expr = sql_raw_expr->get_expr();
                    
                    ObBinaryOpRawExpr* binary_op_raw_expr = dynamic_cast<ObBinaryOpRawExpr*>(raw_expr);
                    ObBinaryRefRawExpr* left_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_first_op_expr());
                    ObBinaryRefRawExpr* right_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_second_op_expr());
                    
                    uint64_t left_first_ref_id = left_expr->get_first_ref_id(); // table id
                    uint64_t right_first_ref_id = right_expr->get_first_ref_id();
                    
                    if (left_first_ref_id == new_table_id) 
                    {
                      new_joined_table->add_table_id(right_first_ref_id);
                    }
                    else if (right_first_ref_id == new_table_id) 
                    {
                      new_joined_table->add_table_id(left_first_ref_id);
                    }
                    
                    uint64_t sub_join_type = sub_joined_table->get_join_types().at(k);
                    switch (sub_join_type) 
                    {
                    case JoinedTable::T_FULL:
                      new_joined_table->add_join_type(JoinedTable::T_FULL);
                      break;
                    case JoinedTable::T_INNER:
                      new_joined_table->add_join_type(JoinedTable::T_INNER);
                      break;
                    case JoinedTable::T_LEFT:
                      new_joined_table->add_join_type(JoinedTable::T_LEFT);
                      break;
                    case JoinedTable::T_RIGHT:
                      new_joined_table->add_join_type(JoinedTable::T_RIGHT);
                      break;
                    default:
                      break;
                    }
                    new_joined_table->add_expr_id(sub_expr_ids.at(k)); 
                    
                    sub_expr_ids.at(k) = 0;
                  }
                }
              }
            }
          }
        }
        
        uint64_t new_joined_tid = subquery_stmt->generate_joined_tid();
        new_joined_table->set_joined_tid(new_joined_tid);
        common::ObVector<FromItem>& sub_from_items = subquery_stmt->get_from_items();
        sub_from_items.clear();
        subquery_stmt->add_from_item(new_joined_tid, true);
        subquery_stmt->add_joined_table(new_joined_table);
      }
      
      return ret;
    }
    
    /*
     * adjust_subquery_join_item
     */ 
    int ObOptimizerLogical::adjust_subquery_join_item(ObLogicalPlan *&logical_plan, JoinedTable *&joined_table, 
        JoinedTable *&sub_joined_table, int64_t sub_joined_position, std::map<uint64_t, uint64_t> alias_table_hashmap, int64_t i)
    {
      int ret = OB_SUCCESS;
      
      common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
      common::ObArray<uint64_t>& sub_joined_expr_ids = sub_joined_table->get_expr_ids();
      common::ObArray<uint64_t>& sub_joined_types = sub_joined_table->get_join_types();
      
      common::ObArray<uint64_t>& table_ids = joined_table->get_table_ids();
      common::ObArray<uint64_t>& expr_ids = joined_table->get_expr_ids();
      common::ObArray<uint64_t>& joined_types = joined_table->get_join_types();
      int64_t sub_position = sub_joined_position;
      int64_t position = i+1;
      
      int64_t next_joined_table_position = sub_joined_position;
      int64_t pre_joined_table_position = -1;
      
      // when sub query pull up, the order of main table should keep the same.
      // such as: '(t1 inner join t2) left join t3' can not become 't1 left join t3 inner join t2'.
      // First, start from this position, move one site backward.
      while (sub_position>0) 
      {
        if (next_joined_table_position == 0) 
        {
          break;
        }
        
        int64_t total = table_ids.count();
        int64_t expr_total = expr_ids.count();
        int64_t type_total = joined_types.count();
        
        // find the joined table of the k index in sub_joined_table_ids
        uint64_t next_expr_id = 0;
        uint64_t next_joined_table_id = 0;
        if (next_joined_table_position<=1) 
        {
          next_expr_id = sub_joined_expr_ids.at(0);
        }
        else 
        {
          next_expr_id = sub_joined_expr_ids.at(next_joined_table_position-1);
        }
        ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(next_expr_id));
        ObRawExpr* raw_expr = sql_raw_expr->get_expr();
        
        ObItemType item_type = raw_expr->get_expr_type();
        if(item_type == T_OP_EQ)
        {
          ObBinaryOpRawExpr* binary_op_raw_expr = dynamic_cast<ObBinaryOpRawExpr*>(raw_expr);
          ObBinaryRefRawExpr* left_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_first_op_expr());
          ObBinaryRefRawExpr* right_expr = dynamic_cast<ObBinaryRefRawExpr*>(binary_op_raw_expr->get_second_op_expr());
          
          if (left_expr->get_first_ref_id() != sub_joined_table_ids.at(next_joined_table_position))
          {
            next_joined_table_id = left_expr->get_first_ref_id();
          } 
          else if (right_expr->get_first_ref_id() != sub_joined_table_ids.at(next_joined_table_position))
          {
            next_joined_table_id = right_expr->get_first_ref_id();
          }
          
          pre_joined_table_position = next_joined_table_position;
          
          for (int64_t k = 0; k < sub_joined_position; ++k) 
          {
            if (sub_joined_table_ids.at(k) == next_joined_table_id) 
            {
              next_joined_table_position = k; // 为0的话，需再次遍历子查询的join item
              sub_joined_table_ids.at(next_joined_table_position) = 0; // remark the table which has pulled up.
              break;
            }
          }
        }
        
        uint64_t next_joined_type = 100; 
        if (pre_joined_table_position <= 1) 
        {
          next_joined_type = sub_joined_types.at(0);
        }
        else 
        {
          next_joined_type = sub_joined_types.at(pre_joined_table_position-1);
        }
        
        if (next_joined_type == JoinedTable::T_LEFT) 
        {
//                        next_joined_type = JoinedTable::T_RIGHT;
          next_joined_type = JoinedTable::T_LEFT; // because the join order depends on the where condition.
        }
        else if (next_joined_type == JoinedTable::T_RIGHT) 
        {
//                        next_joined_type = JoinedTable::T_LEFT;
          next_joined_type = JoinedTable::T_RIGHT;
        }
        
        // Table id and expr_id insert into the main joined item.
        if (position == total) 
        {
          table_ids.push_back(alias_table_hashmap[next_joined_table_id]);
          expr_ids.push_back(next_expr_id);
          joined_types.push_back(next_joined_type);
        }
        else 
        {
          // move main join item.
          while (position<total) 
          {
            if (total == table_ids.count()) 
            {
              table_ids.push_back(table_ids.at(total-1)); // this function can extend buf.
              expr_ids.push_back(expr_ids.at(expr_total-1));
              joined_types.push_back(joined_types.at(type_total-1));
            } 
            else 
            {
              table_ids.at(total) = table_ids.at(total-1);
              expr_ids.at(expr_total) = expr_ids.at(expr_total-1);
              joined_types.at(type_total) = joined_types.at(type_total-1);
            }
            --total;
            --expr_total;
            --type_total;
          }
          
          table_ids.at(position) = alias_table_hashmap[next_joined_table_id];
          expr_ids.at(position-1) = next_expr_id;
          joined_types.at(position-1) = next_joined_type;
        }
        
        ++position;
        --sub_position;
      }
      
      // Second, add table id and expr id which start from sub_joined_position+1 forward.
      sub_position = 0;
      while (sub_position<sub_joined_table_ids.count()-1) 
      {
        int64_t total = table_ids.count();
        int64_t expr_total = expr_ids.count();
        int64_t type_total = joined_types.count();
        
        if (sub_joined_table_ids.at(sub_position+1) > 0) 
        {
          // Table id and expr_id insert into the main joined item.
          if (position == total) 
          {
            table_ids.push_back(alias_table_hashmap[sub_joined_table_ids.at(sub_position+1)]);
            expr_ids.push_back(sub_joined_expr_ids.at(sub_position));
            joined_types.push_back(sub_joined_types.at(sub_position));
          }
          else 
          {
            // move main join item.
            while (position<total) 
            {
              if (total == table_ids.count()) 
              {
                table_ids.push_back(table_ids.at(total-1)); // this function can extend buf.
                expr_ids.push_back(expr_ids.at(expr_total-1));
                joined_types.push_back(joined_types.at(type_total-1));
              } 
              else 
              {
                table_ids.at(total) = table_ids.at(total-1);
                expr_ids.at(expr_total) = expr_ids.at(expr_total-1);
                joined_types.at(type_total) = joined_types.at(type_total-1);
              }
              --total;
              --expr_total;
              --type_total;
            }
            
            table_ids.at(position) = alias_table_hashmap[sub_joined_table_ids.at(sub_position+1)];
            expr_ids.at(position-1) = sub_joined_expr_ids.at(sub_position);
            joined_types.at(position-1) = sub_joined_types.at(sub_position);
          }
          
          ++position;
        }
        ++sub_position;
      }
      
      return ret;
    }
    
    /*
     * pull_up_from_items
     */ 
    int ObOptimizerLogical::pull_up_from_items(ObLogicalPlan *&logical_plan, 
        ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, FromItem &from_item,
        std::map<uint64_t, uint64_t> &alias_table_hashmap, std::map<uint64_t, uint64_t> &table_id_hashmap, 
        std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      bool is_joined = from_item.is_joined_;
      common::ObVector<FromItem>& from_items = main_stmt->get_from_items();
      
      // start pull up from item
      int32_t sub_from_size = subquery_stmt->get_from_item_size();
      for (int32_t i = 0; i < sub_from_size; ++i) 
      {
        FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(i));
        bool sub_is_joined = sub_from_item.is_joined_;
        
        if (is_joined) 
        {
          if (sub_is_joined) 
          {
            JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
            common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
            common::ObRowDesc* table_hash = main_stmt->get_table_hash();
            
            // Add column desc first, this step has not pull up from item.
            for (int64_t j = 0; j < sub_joined_table_ids.count(); ++j)
            {
              // If the table exist in main stmt, it should change to alias table.
              int64_t idx = table_hash->get_idx(sub_joined_table_ids.at(j), OB_INVALID_ID);
              if (idx > 0) 
              {
                uint64_t alias_table_id = logical_plan->generate_table_id();
                alias_table_hashmap[sub_joined_table_ids.at(j)] = alias_table_id;
              } 
              else 
              {
                alias_table_hashmap[sub_joined_table_ids.at(j)] = sub_joined_table_ids.at(j);
              }
              
              table_hash->add_column_desc(alias_table_hashmap[sub_joined_table_ids.at(j)], OB_INVALID_ID);
            }
          }
          else 
          {
            common::ObRowDesc* table_hash = main_stmt->get_table_hash();
            
            // If the table exist in main stmt, it should change to alias table.
            int64_t idx = table_hash->get_idx(sub_from_item.table_id_, OB_INVALID_ID);
            if (idx > 0) 
            {
              uint64_t alias_table_id = logical_plan->generate_table_id();
              alias_table_hashmap[sub_from_item.table_id_] = alias_table_id;
            } 
            else 
            {
              alias_table_hashmap[sub_from_item.table_id_] = sub_from_item.table_id_;
            }
            
            // it lead to decrease cells_desc_count_, so add_column_desc() is wrong.
            // table_hash->remove_column_desc(from_item.table_id_, OB_INVALID_ID); 
            table_hash->add_column_desc(alias_table_hashmap[sub_from_item.table_id_], OB_INVALID_ID);
          }
        } 
        else 
        {
          if (sub_is_joined) 
          {
            JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
            common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
            common::ObArray<uint64_t>& sub_joined_expr_ids = sub_joined_table->get_expr_ids();
            common::ObRowDesc* table_hash = main_stmt->get_table_hash();
            
            for (int64_t j = 0; j < sub_joined_table_ids.count(); ++j)
            {
              // If the table exist in main stmt, it should change to alias table.
              int64_t idx = table_hash->get_idx(sub_joined_table_ids.at(j), OB_INVALID_ID);
              if (idx > 0) 
              {
                uint64_t alias_table_id = logical_plan->generate_table_id();
                alias_table_hashmap[sub_joined_table_ids.at(j)] = alias_table_id;
                
                sub_joined_table_ids.at(j) = alias_table_id;
              } 
              else 
              {
                alias_table_hashmap[sub_joined_table_ids.at(j)] = sub_joined_table_ids.at(j);
              }
              
              table_hash->add_column_desc(sub_joined_table_ids.at(j), OB_INVALID_ID);
            }
            
            for (int64_t k = 0; k < sub_joined_expr_ids.count(); ++k) 
            {
              uint64_t expr_id = sub_joined_expr_ids.at(k);
              ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(expr_id));
              
              uint64_t real_table_id = 0;
              sql_raw_expr->optimize_sql_expression(main_stmt, 
                  table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 2); // 对join条件中表替换成取别名的表
            }
            
            uint64_t joined_tid = main_stmt->generate_joined_tid();
            
            if(i == 0) // 子查询中第一个item直接替换父查询中对应的fromitem，后续的push到最后。
            {
              from_item.table_id_ = joined_tid;
              from_item.is_joined_ = sub_from_item.is_joined_;
            }
            else 
            {
              FromItem new_from_item;
              new_from_item.table_id_ = joined_tid;
              new_from_item.is_joined_ = sub_from_item.is_joined_;
              from_items.push_back(new_from_item);
            }
            
            sub_joined_table->set_joined_tid(joined_tid);
            main_stmt->add_joined_table(sub_joined_table);
          }
          else 
          {
            common::ObRowDesc* table_hash = main_stmt->get_table_hash();
            
            // If the table exist in main stmt, it should change to alias table.
            int64_t idx = table_hash->get_idx(sub_from_item.table_id_, OB_INVALID_ID);
            if (idx > 0) 
            {
              uint64_t alias_table_id = logical_plan->generate_table_id();
              alias_table_hashmap[sub_from_item.table_id_] = alias_table_id;
            } 
            else 
            {
              alias_table_hashmap[sub_from_item.table_id_] = sub_from_item.table_id_;
            }
            
            // it lead to decrease cells_desc_count_, so add_column_desc() is wrong.
            // table_hash->remove_column_desc(from_item.table_id_, OB_INVALID_ID); 
            table_hash->add_column_desc(alias_table_hashmap[sub_from_item.table_id_], OB_INVALID_ID);
            
            if(i == 0)
            {
              from_item.table_id_ = alias_table_hashmap[sub_from_item.table_id_];
              from_item.is_joined_ = sub_from_item.is_joined_;
            }
            else 
            {
              FromItem new_from_item;
              new_from_item.table_id_ = alias_table_hashmap[sub_from_item.table_id_];
              new_from_item.is_joined_ = sub_from_item.is_joined_;
              from_items.push_back(new_from_item);
            }
          }
        }
      }
      
      return ret;
    }
    
    /*
     * pull_up_table_items
     */ 
    int ObOptimizerLogical::pull_up_table_items(ObLogicalPlan *&logical_plan, 
        ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
        std::map<uint64_t, uint64_t> &alias_table_hashmap, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      common::ObVector<TableItem>& table_items = main_stmt->get_table_items();
      int32_t table_item_size = main_stmt->get_table_size();
      for (int32_t i = 0; i < table_item_size; ++i) 
      {
        if (main_stmt->get_table_item(i).table_id_ == table_id) 
        {
          table_items.remove(i);
          break;
        }
      }
      
      int32_t sub_table_item_size = subquery_stmt->get_table_size();
      for (int32_t i = 0; i < sub_table_item_size; ++i) 
      {
        TableItem& sub_table_item = subquery_stmt->get_table_item(i);
        uint64_t sub_table_id = sub_table_item.table_id_;
        
        if (alias_table_hashmap[sub_table_id] == sub_table_id) 
        {
          table_items.push_back(sub_table_item);
        } 
        else // base table transform to alias table 
        {
          ObString new_alias_name;
          ob_write_string(*logical_plan->get_name_pool(), 
                          ObString::link_string("alias", logical_plan->generate_alias_table_id()), 
                          new_alias_name);
          
          sub_table_item.table_id_ = alias_table_hashmap[sub_table_id];
          sub_table_item.alias_name_ = new_alias_name;
          sub_table_item.type_ = TableItem::ALIAS_TABLE;
          table_items.push_back(sub_table_item);
        }
      }
      
      return ret;
    }
    
    /*
     * pull_up_column_items
     */ 
    int ObOptimizerLogical::pull_up_column_items(ObLogicalPlan *&logical_plan, 
                ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
                std::map<uint64_t, uint64_t> &alias_table_hashmap, 
                std::map<uint64_t, uint64_t> &table_id_hashmap, 
                std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      int32_t sub_column_size = subquery_stmt->get_column_size();
      
      common::ObVector<ColumnItem>& main_column_items = main_stmt->get_column_items();
      int32_t column_size = main_stmt->get_column_size();
      for (int32_t i = column_size-1; i >= 0; --i) // as the function remove below, i should decline, and it can avoid NULL exception .
      {
        ColumnItem* column_item = const_cast<ColumnItem*>(main_stmt->get_column_item(i));
        common::ObString column_name = column_item->column_name_;
        if(column_item->table_id_ == table_id)
        {
          int32_t sub_select_size = subquery_stmt->get_select_item_size();
          for (int32_t j = 0; j < sub_select_size; ++j) 
          {
            SelectItem& sub_select_item = const_cast<SelectItem&>(subquery_stmt->get_select_item(j));
            uint64_t sub_expr_id = sub_select_item.expr_id_;
            common::ObString sub_alias_name = sub_select_item.alias_name_;
            
            ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_expr_id);
            ObRawExpr* raw_expr = sql_raw_expr->get_expr();
            ObItemType item_type = raw_expr->get_expr_type();
            if(item_type == T_REF_COLUMN)
            {
              ObBinaryRefRawExpr* binary_ref_raw_expr = dynamic_cast<ObBinaryRefRawExpr*>(raw_expr);
              uint64_t first_ref_id = binary_ref_raw_expr->get_first_ref_id();
              uint64_t second_ref_id = binary_ref_raw_expr->get_second_ref_id();
              if(column_name.compare(sub_alias_name) == 0) // the column names in sub query can not same. 
              {
                column_id_hashmap[column_item->column_id_] = second_ref_id; // join condition column id
                table_id_hashmap[column_item->column_id_] = alias_table_hashmap[first_ref_id]; // join condition table id
                break;
              }
            }
          }
          
          main_column_items.remove(i);
        }
      }
      
      for (int32_t i = 0; i < sub_column_size; ++i)
      {
        ColumnItem* sub_column_item = const_cast<ColumnItem*>(subquery_stmt->get_column_item(i));
        sub_column_item->query_id_ = main_stmt->get_query_id();
        sub_column_item->table_id_ = alias_table_hashmap[sub_column_item->table_id_];
        
        main_column_items.push_back(*sub_column_item);
        // if column id has scan, table item's has_scan_columns_ should set true.
        main_stmt->get_table_item_by_id(sub_column_item->table_id_)->has_scan_columns_ = true;
      }
      
      return ret;
    }
    
    /*
     * pull_up_where_items
     */ 
    int ObOptimizerLogical::pull_up_where_items(ObLogicalPlan *&logical_plan, 
                ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
                std::map<uint64_t, uint64_t> &alias_table_hashmap,
                std::map<uint64_t, uint64_t> &table_id_hashmap, 
                std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      int32_t sub_condition_size = subquery_stmt->get_condition_size();
      common::ObVector<uint64_t>& main_where_exprs = main_stmt->get_where_exprs();
      for (int32_t j = 0; j < sub_condition_size; ++j) 
      {
        uint64_t sub_where_exprs_id = subquery_stmt->get_condition_id(j);
        main_where_exprs.push_back(sub_where_exprs_id);
        
        ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(sub_where_exprs_id);
        uint64_t real_table_id = 0;
        sql_raw_expr->optimize_sql_expression(main_stmt, 
            table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 2);
      }
      
      // main condition
      int32_t condition_size = main_stmt->get_condition_size();
      for (int32_t i = 0; i < condition_size; ++i) 
      {
        uint64_t real_table_id = 0;
        uint64_t expr_id = main_stmt->get_condition_id(i);
        ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(expr_id));
        sql_raw_expr->optimize_sql_expression(main_stmt, 
            table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 1);
      }
      
      return ret;
    }
    
    /*
     * pull_up_column_items
     */ 
    int ObOptimizerLogical::pull_up_select_items(ObLogicalPlan *&logical_plan, 
                ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
                std::map<uint64_t, uint64_t> &alias_table_hashmap,
                std::map<uint64_t, uint64_t> &table_id_hashmap, 
                std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id)
    {
      int ret = OB_SUCCESS;
      
      int32_t select_size = main_stmt->get_select_item_size();
      int32_t sub_select_size = subquery_stmt->get_select_item_size();
      for (int32_t i = 0; i < sub_select_size; ++i) 
      {
        SelectItem& sub_select_item = const_cast<SelectItem&>(subquery_stmt->get_select_item(i));
        uint64_t sub_expr_id = sub_select_item.expr_id_;
        common::ObString sub_alias_name = sub_select_item.alias_name_;
        bool exist = false; // if main stmt has sub stmt's select item(column), if has not, then delete.
        
        for (int32_t j = 0; j < select_size; ++j) 
        {
          SelectItem& select_item = const_cast<SelectItem&>(main_stmt->get_select_item(j));
          uint64_t expr_id = select_item.expr_id_;
          common::ObString alias_name = select_item.alias_name_;
          
          ObSqlRawExpr* sql_raw_expr = logical_plan->get_expr(expr_id);
          ObRawExpr* raw_expr = sql_raw_expr->get_expr();
          ObItemType item_type = raw_expr->get_expr_type();
          
          if(item_type == T_REF_COLUMN)
          {
            ObBinaryRefRawExpr* binary_ref_raw_expr = dynamic_cast<ObBinaryRefRawExpr*>(raw_expr);
            uint64_t first_ref_id = binary_ref_raw_expr->get_first_ref_id();
            
            if(first_ref_id == table_id && (alias_name.compare(sub_alias_name)) == 0)
            {
              exist = true;
              select_item.expr_id_ = sub_expr_id;
              
              // sub table id to alias table id
              ObSqlRawExpr* sub_sql_raw_expr = logical_plan->get_expr(sub_expr_id);
              uint64_t real_table_id = 0;
              sub_sql_raw_expr->optimize_sql_expression(main_stmt, 
                  table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 2);
              
              // delete this Expr 
              logical_plan->delete_expr_by_id(expr_id);
              
              break;
            }
          }
        }
        
        if(!exist)
        {
          logical_plan->delete_expr_by_id(sub_expr_id);
        }
      }
      
      return ret;
    }
    
    /*
     * pull_up_from_join_items
     */ 
    int ObOptimizerLogical::pull_up_from_join_items(ObLogicalPlan *&logical_plan, 
                ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
                std::map<uint64_t, uint64_t> &alias_table_hashmap,
                std::map<uint64_t, uint64_t> &table_id_hashmap, 
                std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id, JoinedTable *&joined_table)
    {
      int ret = OB_SUCCESS;
      
      common::ObVector<FromItem>& from_items = main_stmt->get_from_items();
      common::ObArray<uint64_t>& table_ids = joined_table->get_table_ids();
      for (int64_t i = 0; i < table_ids.count(); ++i) 
      {
        if(table_ids.at(i) == table_id)
        {
          uint64_t expr_id = 0;
          uint64_t real_table_id = 0; // the real table in join expr, when the subquery has multi table.
          uint64_t sub_real_table_id = 0; // for the table which exist int father query.
          if (i == 0) 
          {
            expr_id = joined_table->get_expr_ids().at(0);
          } 
          else 
          {
            expr_id = joined_table->get_expr_ids().at(i-1);
          }
          ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(expr_id));
          sql_raw_expr->optimize_sql_expression(main_stmt, 
              table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 1);
          
          // change table id in main joined item.
          table_ids.at(i) = real_table_id;
          
          // All the tables in subquery should be added to the main stmt's from items. 
          int32_t sub_from_size = subquery_stmt->get_from_item_size();
          for (int32_t j = 0; j < sub_from_size; ++j) 
          {
            FromItem& sub_from_item = const_cast<FromItem&>(subquery_stmt->get_from_item(j));
            bool sub_is_joined = sub_from_item.is_joined_;
            
            if (sub_is_joined) 
            {
              JoinedTable* sub_joined_table = subquery_stmt->get_joined_table(sub_from_item.table_id_);
              common::ObArray<uint64_t>& sub_joined_table_ids = sub_joined_table->get_table_ids();
              common::ObArray<uint64_t>& sub_joined_expr_ids = sub_joined_table->get_expr_ids();
              
              // Get the postion of real joined table in subquery's joined_table_ids.
              int64_t sub_joined_position = -1;
              for (int64_t k = sub_joined_table_ids.count()-1; k >=0 ; --k) 
              {
                if (real_table_id == alias_table_hashmap[sub_joined_table_ids.at(k)]) 
                {
                  sub_real_table_id = sub_joined_table_ids.at(k);
                  sub_joined_position = k;
                  sub_joined_table_ids.at(sub_joined_position) = 0; // remark the table which has pulled up.
                  break;
                }
              }
              
              if (sub_joined_position >= 0) 
              {
                adjust_subquery_join_item(logical_plan, joined_table, sub_joined_table, sub_joined_position, alias_table_hashmap, i);
              }
              else 
              {
                uint64_t joined_tid = main_stmt->generate_joined_tid();
                
                FromItem new_from_item;
                new_from_item.table_id_ = joined_tid;
                new_from_item.is_joined_ = sub_from_item.is_joined_;
                from_items.push_back(new_from_item);
                
                for (int64_t k = 0; k < sub_joined_table_ids.count(); ++k) 
                {
                  uint64_t _sub_table_id = sub_joined_table_ids.at(k);
                  sub_joined_table_ids.at(k) = alias_table_hashmap[_sub_table_id];
                }
                
                sub_joined_table->set_joined_tid(joined_tid);
                main_stmt->add_joined_table(sub_joined_table);
              }
              
              // In sub query, the table which is exist in father query should change to alias table.
              for (int64_t k = 0; k < sub_joined_expr_ids.count(); ++k) 
              {
                uint64_t expr_id = sub_joined_expr_ids.at(k);
                
                ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(expr_id));
                sql_raw_expr->optimize_sql_expression(main_stmt, 
                    table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 2);
              }
            }
            else 
            {
              if (real_table_id == alias_table_hashmap[sub_from_item.table_id_]) 
              {
                continue;
              }
              else 
              {
                FromItem new_from_item;
                new_from_item.table_id_ = alias_table_hashmap[sub_from_item.table_id_];
                new_from_item.is_joined_ = sub_from_item.is_joined_;
                from_items.push_back(new_from_item);
              }
            }
          }
          break;
        }
      }
      
      common::ObArray<uint64_t>& expr_ids = joined_table->get_expr_ids();
      for (int64_t i = 0; i < expr_ids.count(); ++i) 
      {
        uint64_t real_table_id = 0;
        uint64_t expr_id = expr_ids.at(i);
        ObSqlRawExpr* sql_raw_expr = const_cast<ObSqlRawExpr*>(logical_plan->get_expr(expr_id));
        sql_raw_expr->optimize_sql_expression(main_stmt, 
            table_id_hashmap, column_id_hashmap, table_id, real_table_id, alias_table_hashmap, 1);
      }
      
      return ret;
    }
    
    /*
     * pull up subquery execute
     */ 
    int ObOptimizerLogical::pull_up_simple_subquery(ObLogicalPlan *&logical_plan, 
          ObSelectStmt *&main_stmt, FromItem &from_item, TableItem *&table_item, 
          JoinedTable *&joined_table, uint64_t join_type, ResultPlan &result_plan)
    {
      int ret = OB_SUCCESS;
      
      uint64_t table_id = table_item->table_id_; // if subquery, from_item's table_id equal table_item's table_id
      bool is_joined = from_item.is_joined_;
      uint64_t ref_id = table_item->ref_id_;
      ObSelectStmt* subquery_stmt = dynamic_cast<ObSelectStmt*>(logical_plan->get_query(ref_id));
      
      
      /** pull up subquery recursion **/
      ret = pull_up_subquery_union(logical_plan, subquery_stmt, result_plan);
      
      /** 
       * 1. sub_table_id replace table_id, change column_desc. 
       *    tables_hash_(ObRowDesc) must be replace. 
      **/
      std::map<uint64_t, uint64_t> column_id_hashmap; // for main stmt's condition
      std::map<uint64_t, uint64_t> table_id_hashmap; // for main stmt's condition 
      std::map<uint64_t, uint64_t> alias_table_hashmap; // base table transform to alias table
      
      if (ret==OB_SUCCESS && is_simple_subquery(subquery_stmt)) 
      {
      }
      else 
      {
        ret = OB_SQL_CAN_NOT_PULL_UP;
      }
      
      // decide whether the sub query can pull up or not
      if (ret == OB_SUCCESS) 
      {
        ret = if_can_pull_up(logical_plan, subquery_stmt, joined_table, join_type, is_joined, table_id);
      }
      
      if (ret == OB_SUCCESS) 
      {
        ret = combine_subquery_from_items(logical_plan, subquery_stmt, result_plan, is_joined);
      }
      
      if (ret == OB_SUCCESS) 
      {
        ret = pull_up_from_items(logical_plan, main_stmt, subquery_stmt, from_item, 
                                 alias_table_hashmap, table_id_hashmap, column_id_hashmap, table_id);
      }
      
      if (ret == OB_SUCCESS) 
      {
        /** 2. sub_table_item replace table_item **/
        ret = pull_up_table_items(logical_plan, main_stmt, subquery_stmt, alias_table_hashmap, table_id);
        
        /** 3. sub_table_id replace column_item.TableRef **/
        if (ret == OB_SUCCESS) 
        {
          ret = pull_up_column_items(logical_plan, main_stmt, subquery_stmt, alias_table_hashmap, table_id_hashmap, column_id_hashmap, table_id);
        }
        
        /** 4. sub_select_item.sub_expr_id replace select_item.expr_id **/
        if (ret == OB_SUCCESS) 
        {
          ret = pull_up_select_items(logical_plan, main_stmt, subquery_stmt, alias_table_hashmap, table_id_hashmap, column_id_hashmap, table_id);
        } 
        
        /** 
         * 5. main conditions add the conditions of sub query,
         *    sub_table_id replace ObRawExpr's table_id. 
         **/
        if (ret == OB_SUCCESS) 
        {
          ret = pull_up_where_items(logical_plan, main_stmt, subquery_stmt, alias_table_hashmap, table_id_hashmap, column_id_hashmap, table_id);
        } 
        
        // consider the sub_table_id of join condition
        // (if has the same column name, it choose the first one, and the sequence of the column is same)
        // join condition should repalce table_id
        if (ret == OB_SUCCESS) 
        {
          if(is_joined)
          {
            ret = pull_up_from_join_items(logical_plan, main_stmt, subquery_stmt, alias_table_hashmap, table_id_hashmap, column_id_hashmap, table_id, joined_table);
          }
        } 
        
        /** 6.delete sub query stmt **/
        if (ret == OB_SUCCESS) 
        {
          logical_plan->delete_stmt_by_query_id(ref_id);
        } 
      }
      
      if (ret == OB_SUCCESS) 
      {
      }
      else 
      {
        TBSYS_LOG(WARN, "pull up sub query fail");
      }
      
      return ret;
    }
  }
}


