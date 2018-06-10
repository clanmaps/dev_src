/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_if.h
* @brief this class  present a procedure "if" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_IF_H
#define OCEANBASE_SQL_OB_PROCEDURE_IF_H
#include "ob_multi_children_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"


namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureIf class
         */
		class ObProcedureIf : public ObMultiChildrenPhyOperator
		{
		public:
			ObProcedureIf();
			virtual ~ObProcedureIf();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_IF;
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
             * @brief set "elseif" branch flag
             * @param flag
             * @return
             */
			int set_have_elseif(bool flag);

            /**
             * @brief add operator into "elseif" branch
             * @param elseif_op
             * @return
             */
			int add_elseif_op(ObPhyOperator &elseif_op);

            /**
             * @brief set "else" branch flag
             * @param flag
             * @return
             */
			int set_have_else(bool flag);


            /**
             * @brief set "else" branch operator
             * @param flag
             * @return
             */
			int set_else_op(ObPhyOperator &else_op);
            /**
             * @brief get the flag of "if" statement whether have "elseif" branch
             * @return
             */
			bool is_have_elseif();

            /**
             * @brief get the flag of "if" statement whether have "else" branch
             * @return
             */
			bool is_have_else();


		private:
			//disallow copy
			ObProcedureIf(const ObProcedureIf &other);
			ObProcedureIf& operator=(const ObProcedureIf &other);
			//function members

		private:
			//data members
            ObSqlExpression expr_;///< if expression

            int32_t child_num_;///< then branch statement size

            bool have_elseif_;///< flag of "elseif" branch

            ObArray<ObPhyOperator*> elseif_ops_;///< else if operators

            bool have_else_;///< flag of "else" branch

            ObPhyOperator *else_op_;///< else operator

		};



	}
}

#endif



