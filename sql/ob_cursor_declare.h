/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_declare.h
* @brief this class  present a declare cursor logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_DECLARE_H
#define OCEANBASE_SQL_OB_DECLARE_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"

namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObCursorDeclare class
         */
		class ObCursorDeclare : public ObSingleChildPhyOperator
		{
		public:
			ObCursorDeclare();
			virtual ~ObCursorDeclare();
			virtual void reset();
			virtual void reuse();
            /**
             * @brief store curtsor_name
             * @param cursor_name
             */
			void set_cursor_name(const common::ObString& cursor_name);
            /**
             * @brief get_cursor_name
             * @return
             */
			const common::ObString& get_cursor_name()const;
			//execute the declare statement
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const{ return PHY_CURSOR_DECLARE;}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			// @note always return OB_ITER_END
			virtual int get_next_row(const common::ObRow *&row);
		private:
			//disallow copy
			ObCursorDeclare(const ObCursorDeclare &other);
			ObCursorDeclare& operator=(const ObCursorDeclare &other);
			//function members
			int store_phy_plan_to_session();
		private:
			//data members
            common::ObString cursor_name_;///> cursor name
		};

		inline void ObCursorDeclare::reset()
		{
		    ObSingleChildPhyOperator::reset();
		}

		inline void ObCursorDeclare::reuse()
		{
			ObSingleChildPhyOperator::reuse();
		}

		inline void ObCursorDeclare::set_cursor_name(const common::ObString& cursor_name)
		{
		    cursor_name_ = cursor_name;
		}
	}
}

#endif /* OCEANBASE_SQL_OB_PREPARE_H_ */



