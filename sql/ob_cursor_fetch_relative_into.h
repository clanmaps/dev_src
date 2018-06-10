/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_relative_into.h
* @brief this class  present a "cursor fetch relative into" physical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <513401199201286431@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_CURSOR_FETCH_RELATIVE_INTO_H
#define OCEANBASE_SQL_OB_CURSOR_FETCH_RELATIVE_INTO_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"
namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObCursorFetchRelativeInto class
         */
		class ObCursorFetchRelativeInto : public ObSingleChildPhyOperator
		{
		public:
			ObCursorFetchRelativeInto();
			virtual ~ObCursorFetchRelativeInto();
			virtual void reset();
			virtual void reuse();
			virtual int open();
			virtual int close();
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_CURSOR_FETCH_RELATIVE_INTO;
			}
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);

			int add_variable(ObString &var);/*添加一个变量*/

		private:
			//disallow copy
			ObCursorFetchRelativeInto(const ObCursorFetchRelativeInto &other);
			ObCursorFetchRelativeInto& operator=(const ObCursorFetchRelativeInto &other);
			//function members

		private:
			ObArray<ObString> variables_;/*fetch first into定义变量列表*/

		};



	}
}

#endif



