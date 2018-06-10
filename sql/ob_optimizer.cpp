#include "sql/ob_optimizer.h"
#include "sql/ob_optimizer_logical.h"
#include "sql/ob_multi_logic_plan.h"

namespace oceanbase 
{
  namespace sql 
  {
    /*
     * @brief enter optimizer
     * @return 
     */
    int ObOptimizer::optimizer(ResultPlan &result_plan, ObResultSet &result)
    {
      int ret = OB_SUCCESS;
      
      if(true)
      {
        standard_optimizer(result_plan, result);
      }
      
      return ret;
    }
    
    /*
     * @brief standard_planner
     * @return 
     */
    int ObOptimizer::standard_optimizer(ResultPlan &result_plan, ObResultSet &result)
    {
      
      int ret = OB_SUCCESS;
      
      // logical optimizer
      ret = logical_optimizer(result_plan, result);
      
      return ret;
    
    }
    
    /*
     * @brief logical planner optimizer
     * @return 
     */
    int ObOptimizer::logical_optimizer(ResultPlan &result_plan, ObResultSet &result)
    {
      
      int ret = OB_SUCCESS;
      
      ObMultiLogicPlan *logical_plans = static_cast<ObMultiLogicPlan*>(result_plan.plan_tree_);
      ObLogicalPlan *logical_plan = NULL;
      for (int32_t i = 0; ret == OB_SUCCESS && i < logical_plans->size(); i++)
      {
        logical_plan = logical_plans->at(i);
        
        // pull up sublink;
        
        // pull up subquery
        ret = ObOptimizerLogical::pull_up_subqueries(logical_plan, result, result_plan);
        
        /* write by lxb for test on 2016/12/25 */
        /*
        FILE* fp = NULL; 
        fp = fopen("/home/lxb/logic_end.txt", "wt");
        logical_plan->print(fp, 0);
        fclose(fp);
        */
        
      }
      
      return ret;
    }

  }
  

}


