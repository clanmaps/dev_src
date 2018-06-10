/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_else.h
* @brief this class  present a procedure "else" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_ELSE_H
#define OCEANBASE_SQL_OB_PROCEDURE_ELSE_H
#include "ob_multi_children_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"


namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureElse class
         */
		class ObProcedureElse : public ObMultiChildrenPhyOperator
		{
		public:
			ObProcedureElse();
			virtual ~ObProcedureElse();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_ELSE;
			}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);
			virtual int set_child(int32_t child_idx, ObPhyOperator &child_operator);
			virtual int32_t get_child_num() const;
		private:
			//disallow copy
			ObProcedureElse(const ObProcedureElse &other);
			ObProcedureElse& operator=(const ObProcedureElse &other);
			//function members

		private:
			//data members
            int32_t child_num_;///>else statements size

		};



	}
}

#endif



