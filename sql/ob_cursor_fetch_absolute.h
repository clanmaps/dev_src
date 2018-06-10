/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_absolute.h
* @brief this class  present a "cursor fetch absolute" physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_FETCH_ABSOLUTE_H_
#define OCEANBASE_SQL_OB_FETCH_ABSOLUTE_H_
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"


namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObFetchAbsolute class
         */
		class ObFetchAbsolute: public ObSingleChildPhyOperator
		{
			public:
			ObFetchAbsolute();
			virtual ~ObFetchAbsolute();
			virtual void reset();
			virtual void reuse();
            /**
             * @brief set_cursor_name
             * @param cursor_name
             */
			void set_cursor_name(const common::ObString& cursor_name);
			//execute the declare statement
			virtual int open();
			virtual int close();
		    virtual ObPhyOperatorType get_type() const { return PHY_CURSOR_FETCH_ABSOLUTE; }
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
			virtual int get_next_row(const common::ObRow *&row);
			void set_is_next(int64_t is_next);
		    int64_t get_is_next();
            /**
             * @brief set_fetch_count
             * @param fetch_count
             */
	        void set_fetch_count(int64_t fetch_count);
            /**
             * @brief get_fetch_count
             * @return
             */
            int64_t get_fetch_count();
			private:
			//disallow copy
			ObFetchAbsolute(const ObFetchAbsolute &other);
			ObFetchAbsolute& operator=(const ObFetchAbsolute &other);
			//function members
			private:
			//data members
            bool  is_next_;///>next flag
            int64_t  fetch_count_;///> fetch count
            common::ObString cursor_name_;///> cursor name
		};

        inline void ObFetchAbsolute::set_is_next(int64_t is_next)
		{
		    is_next_ = is_next;
		}
	    inline int64_t ObFetchAbsolute::get_is_next()
		{
		    return is_next_;
		}
	    inline void ObFetchAbsolute::set_fetch_count(int64_t fetch_count)
		{
		    fetch_count_ = fetch_count;
		}
	    inline int64_t ObFetchAbsolute::get_fetch_count()
		{
		    return fetch_count_;
		}
	    inline void ObFetchAbsolute::set_cursor_name(const common::ObString& cursor_name)
		{
		    cursor_name_ = cursor_name;
		}

	}
}
#endif/* OCEANBASE_SQL_OB_FETCH_H_ */
