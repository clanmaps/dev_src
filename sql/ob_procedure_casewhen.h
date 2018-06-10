/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_casewhen.h
* @brief this class  present a procedure "casewhen" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_CASEWHEN_H
#define OCEANBASE_SQL_OB_PROCEDURE_CASEWHEN_H
#include "ob_multi_children_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"


namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureCaseWhen class
         */
		class ObProcedureCaseWhen : public ObMultiChildrenPhyOperator
		{
		public:
			ObProcedureCaseWhen();
			virtual ~ObProcedureCaseWhen();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_CASE_WHEN;
			}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);
			virtual int set_child(int32_t child_idx, ObPhyOperator &child_operator);
			virtual int32_t get_child_num() const;

            /**
             * @brief set expression
             * @param expr
             * @return
             */
			int set_expr(ObSqlExpression& expr);
            /**
             * @brief set compare expression
             * @param expr
             * @return
             */
			int set_compare_expr(ObSqlExpression& expr);
            /**
             * @brief set case value expression
             * @param expr
             * @return
             */
			int set_case_expr(ObSqlExpression& expr);
		private:
			//disallow copy
			ObProcedureCaseWhen(const ObProcedureCaseWhen &other);
			ObProcedureCaseWhen& operator=(const ObProcedureCaseWhen &other);
			//function members

		private:
			//data members
            ObSqlExpression compare_expr_;///> the compare expression
            ObSqlExpression expr_;  ///> the "casewhen" statement's expression
            ObSqlExpression case_expr_;///> the case expression
			int32_t child_num_;

		};



	}
}

#endif



