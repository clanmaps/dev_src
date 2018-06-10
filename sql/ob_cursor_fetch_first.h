/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_first.h
* @brief this class  present a "cursor fetch first" physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_FETCH_FIRST_H_
#define OCEANBASE_SQL_OB_FETCH_FIRST_H_
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"

namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObFetchFirst class
         */
		class ObFetchFirst: public ObSingleChildPhyOperator
		{
			public:
			ObFetchFirst();
			virtual ~ObFetchFirst();
			virtual void reset();
			virtual void reuse();
			void set_cursor_name(const common::ObString& cursor_name);
			//execute the declare statement
			virtual int open();
			virtual int close();
		    virtual ObPhyOperatorType get_type() const { return PHY_CURSOR_FETCH_FIRST; }
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);
			private:
			//disallow copy
			ObFetchFirst(const ObFetchFirst &other);
			ObFetchFirst& operator=(const ObFetchFirst &other);
			private:
			//data member
			common::ObString cursor_name_;
		};

   	    inline void ObFetchFirst::set_cursor_name(const common::ObString& cursor_name)
	    {
	        cursor_name_ = cursor_name;
	    }
	}
}
#endif/* OCEANBASE_SQL_OB_FETCH_H_ */
