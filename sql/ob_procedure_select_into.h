/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_select_into.h
* @brief this class present a procedure "select into" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_SELECT_INTO_H
#define OCEANBASE_SQL_OB_PROCEDURE_SELECT_INTO_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"
namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureSelectInto class
         */
		class ObProcedureSelectInto : public ObSingleChildPhyOperator
		{
		public:
			ObProcedureSelectInto();
			virtual ~ObProcedureSelectInto();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_SELECT_INTO;
			}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);

            /**
             * @brief add variable
             * @param var
             * @return
             */
            int add_variable(ObString &var);

		private:
			//disallow copy
			ObProcedureSelectInto(const ObProcedureSelectInto &other);
			ObProcedureSelectInto& operator=(const ObProcedureSelectInto &other);
			//function members

		private:
            ObArray<ObString> variables_;///< variable list

		};



	}
}

#endif



