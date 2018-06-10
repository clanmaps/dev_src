/**
 * optimizer
 */
#ifndef OB_OPTIMIZER_H
#define OB_OPTIMIZER_H 1

#include "sql/ob_result_set.h"
#include "sql/parse_node.h"

namespace oceanbase
{
  namespace sql 
  {
    
  
    class ObOptimizer
    {
      public:
        
        ObOptimizer(){}
        ~ObOptimizer(){}
        
        /*
         * @brief enter optimizer
         * @return 
         */
        static int optimizer(ResultPlan &result_plan, ObResultSet &result);
        
        /*
         * @brief standard_planner
         * @return 
         */
        static int standard_optimizer(ResultPlan &result_plan, ObResultSet &result);
        
        /*
         * @brief logical planner optimizer
         * @return 
         */
        static int logical_optimizer(ResultPlan &result_plan, ObResultSet &result);
      
      private:
        
    
    };

    
  }
}

#endif
