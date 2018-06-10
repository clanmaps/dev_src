#ifndef OB_OPTIMIZER_PLAN_H
#define OB_OPTIMIZER_PLAN_H 1

#include "sql/ob_logical_plan.h"

namespace oceanbase
{
  namespace sql 
  {
  
    class ObOptimizerLogical
    {
      public:
        
        ObOptimizerLogical(){}
        ~ObOptimizerLogical(){}
        
        /*
         * @brief decide whether sub query is a simple subquery or not  
         * @return 
         */
        static bool is_simple_subquery(ObSelectStmt *select_stmt);
        
        /*
         * @brief pull up subquery
         * @return 
         */
        static int pull_up_subqueries(ObLogicalPlan *&logical_plan, ObResultSet &result, ResultPlan &result_plan);
        
        /*
         * pull_up_subquery_union
         */ 
        static int pull_up_subquery_union(ObLogicalPlan *&logical_plan, ObSelectStmt *&main_stmt, ResultPlan &result_plan);
        
        /*
         * pull_up_subquery_recursion
         */
        static int pull_up_subquery_recursion(ObLogicalPlan *&logical_plan, ObSelectStmt *&main_stmt, FromItem& from_item, ResultPlan &result_plan);
        
        /*
         * if_can_pull_up
         */
        static int if_can_pull_up(ObLogicalPlan *&logical_plan, ObSelectStmt* subquery_stmt, JoinedTable *&joined_table, uint64_t join_type, bool is_joined, uint64_t table_id);
        
        /*
         * combine_subquery_from_items
         */
        static int combine_subquery_from_items(ObLogicalPlan *&logical_plan, ObSelectStmt *subquery_stmt, ResultPlan &result_plan, bool is_joined);
        
        /*
         * adjust_subquery_join_item
         */
        static int adjust_subquery_join_item(ObLogicalPlan *&logical_plan, JoinedTable *&joined_table, 
            JoinedTable *&sub_joined_table, int64_t sub_joined_position, std::map<uint64_t, uint64_t> alias_table_hashmap, int64_t i);
        
        /*
         * pull_up_from_items
         */ 
        static int pull_up_from_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, FromItem &from_item,
            std::map<uint64_t, uint64_t> &alias_table_hashmap, std::map<uint64_t, uint64_t> &table_id_hashmap, 
            std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id);
        
        /*
         * pull_up_table_items
         */ 
        static int pull_up_table_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
            std::map<uint64_t, uint64_t> &alias_table_hashmap, uint64_t table_id);
        
        /*
         * pull_up_table_items
         */
        static int pull_up_column_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
            std::map<uint64_t, uint64_t> &alias_table_hashmap,
            std::map<uint64_t, uint64_t> &table_id_hashmap, 
            std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id);
        
        /*
         * pull_up_select_items
         */
        static int pull_up_select_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
            std::map<uint64_t, uint64_t> &alias_table_hashmap,
            std::map<uint64_t, uint64_t> &table_id_hashmap, 
            std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id);
        
        /*
         * pull_up_where_items
         */
        static int pull_up_where_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
            std::map<uint64_t, uint64_t> &alias_table_hashmap,
            std::map<uint64_t, uint64_t> &table_id_hashmap, 
            std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id);
        
        /*
         * pull_up_from_join_items
         */ 
        static int pull_up_from_join_items(ObLogicalPlan *&logical_plan, 
            ObSelectStmt *&main_stmt, ObSelectStmt* subquery_stmt, 
            std::map<uint64_t, uint64_t> &alias_table_hashmap,
            std::map<uint64_t, uint64_t> &table_id_hashmap, 
            std::map<uint64_t, uint64_t> &column_id_hashmap, uint64_t table_id,
            JoinedTable *&joined_table);
        
        /*
         * pull up subquery execute
         */ 
        static int pull_up_simple_subquery(ObLogicalPlan *&logical_plan, 
                     ObSelectStmt *&main_stmt, FromItem &from_item, TableItem *&table_item, 
                     JoinedTable *&joined_table, uint64_t join_type, ResultPlan &result_plan);
        
        
        
      private:
        
    
    };

  }
}

#endif

