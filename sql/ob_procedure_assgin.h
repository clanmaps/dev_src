/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_procedure_assgin.h
* @brief this class  present a procedure "assgin" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_PROCEDURE_ASSGIN_H
#define OCEANBASE_SQL_OB_PROCEDURE_ASSGIN_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"
#include "ob_procedure_assgin_stmt.h"
namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureAssgin class represent a "assign" operator in procedure
         */
		class ObProcedureAssgin : public ObSingleChildPhyOperator
		{
            public:
                ObProcedureAssgin();
                virtual ~ObProcedureAssgin();
                virtual void reset();
                virtual void reuse();
                virtual int open();
                virtual int close();
                virtual ObPhyOperatorType get_type() const
                {
                    return PHY_PROCEDURE_DECLARE;
                }
                virtual int64_t to_string(char* buf, const int64_t buf_len) const;
                virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
                virtual int get_next_row(const common::ObRow *&row);

                /**
                 * @brief add a "set" operator into assign operator
                 * @param var_val type is ObVariableSetVal
                 * @return int
                 */
                int add_var_val(ObVariableSetVal &var_val);

            private:
                //disallow copy
                ObProcedureAssgin(const ObProcedureAssgin &other);
                ObProcedureAssgin& operator=(const ObProcedureAssgin &other);
                //function members

            private:
                ObArray<ObVariableSetVal> var_val_list_;///< assgin variable list

		};



	}
}

#endif



