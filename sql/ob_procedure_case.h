/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_case.h
* @brief this class  present a procedure "case" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_CASE_H
#define OCEANBASE_SQL_OB_PROCEDURE_CASE_H
#include "ob_multi_children_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"


namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureCase class represent "case" operator in procedure
         */
		class ObProcedureCase : public ObMultiChildrenPhyOperator
		{
		public:
			ObProcedureCase();
			virtual ~ObProcedureCase();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_CASE;
			}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);
			virtual int set_child(int32_t child_idx, ObPhyOperator &child_operator);
			virtual int32_t get_child_num() const;

            /**
             * @brief set case operator's expression
             * @param expr
             * @return
             */
			int set_expr(ObSqlExpression& expr);

            /**
             * @brief set "case" operator whether have else branch
             * @param flag
             * @return
             */
			int set_have_else(bool flag);

            /**
             * @brief set "case" operator's "else" branch physical operator
             * @param else_op
             * @return
             */
			int set_else_op(ObPhyOperator &else_op);

            /**
             * @brief whether "case" operator have "else" branch
             * @return
             */
			bool is_have_else();


		private:
			//disallow copy
			ObProcedureCase(const ObProcedureCase &other);
			ObProcedureCase& operator=(const ObProcedureCase &other);
			//function members

		private:
			//data members
            ObSqlExpression expr_;///< "case"'s expression

            int32_t child_num_;	///< the number of "case when" branch

            bool have_else_; ///< the flag of whether have "else" branch

            ObPhyOperator *else_op_; ///< the physical operator of "else" branch

		};



	}
}

#endif



