/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_fromto.h
* @brief this class  present a "cursor fetch fromto" physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#ifndef OCEANBASE_SQL_OB_FETCH_FROMTO_H_
#define OCEANBASE_SQL_OB_FETCH_FROMTO_H_
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"

namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObFetchFromto class
         */
		class ObFetchFromto: public ObSingleChildPhyOperator
		{
			public:
			ObFetchFromto();
			virtual ~ObFetchFromto();
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
            virtual ObPhyOperatorType get_type() const { return PHY_CURSOR_FETCH_FROMTO; }
            virtual int64_t to_string(char* buf, const int64_t buf_len) const;
            virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
            virtual int get_next_row(const common::ObRow *&row);
            /**
             * @brief set_count_from
             * @param is_next
             */
            void set_count_f(int64_t is_next);
            /**
             * @brief get_count_from
             * @return
             */
            int64_t get_count_f();
            /**
             * @brief set_count_to
             * @param count_t
             */
            void set_count_t(int64_t count_t);
            /**
             * @brief get_count_to
             * @return
             */
            int64_t get_count_t();
            /**
             * @brief set_count
             * @param count
             */
            void set_count(int64_t count);
            private:
            //disallow copy
            ObFetchFromto(const ObFetchFromto &other);
            ObFetchFromto& operator=(const ObFetchFromto &other);
            private:
            //data members
            int64_t  count_f_;///> count from
            int64_t  count_t_;///> count to
            int64_t count_;///> count
            common::ObString cursor_name_;///> cursor name
		};

        inline void ObFetchFromto::set_count_f(int64_t is_next)
        {
             count_f_ = is_next;
        }
        inline int64_t ObFetchFromto::get_count_f()
        {
             return count_f_;
        }
        inline void ObFetchFromto::set_count_t(int64_t count_t)
        {
             count_t_ = count_t;
        }
        inline int64_t ObFetchFromto::get_count_t()
        {
             return count_t_;
        }
        inline void ObFetchFromto::set_count(int64_t count)
        {
             count_ = count;
        }
        inline void ObFetchFromto::set_cursor_name(const common::ObString& cursor_name)
        {
             cursor_name_ = cursor_name;
        }
	}
}
#endif/* OCEANBASE_SQL_OB_FETCH_H_ */
