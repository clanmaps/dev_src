/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_fromto_stmt.h
* @brief this class  present a "cursor fetch fromto" logical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#ifndef OCEANBASE_SQL_FETCH_FROMTO_STMT_H
#define OCEANBASE_SQL_FETCH_FROMTO_STMT_H
#include "ob_basic_stmt.h"
#include "common/ob_string.h"
#include "common/ob_array.h"

namespace oceanbase
{
	namespace sql
	{
        /**
        * @brief The ObFetchFromtoStmt class
        */
	   class ObFetchFromtoStmt : public ObBasicStmt
	   {
         public:
         ObFetchFromtoStmt()
            : ObBasicStmt(T_CURSOR_FETCH_FROMTO)
              {
              }
         virtual ~ObFetchFromtoStmt(){}
         /**
          * @brief set_cursor_name
          * @param name
          */
         void set_cursor_name(const common::ObString& name);
         /**
          * @brief set_count_from
          * @param fetch_count
          */
         void set_count_f(int64_t fetch_count);
         /**
          * @brief get_count_from
          * @return
          */
         int64_t get_count_f();
         /**
          * @brief set_count_to
          * @param fetch_count
          */
         void set_count_t(int64_t fetch_count);
         /**
          * @brief get_count_to
          * @return
          */
         int64_t get_count_t();
         /**
          * @brief get_cursor_name
          * @return
          */
         const common::ObString& get_cursor_name() const;
         virtual void print(FILE* fp, int32_t level, int32_t index);
         private:
         common::ObString  cursor_name_;///>cursor name
//	     common::ObArray<common::ObString> variable_names_;
         int64_t  count_f_;///> count from
         int64_t  count_t_;///> count to
		};
		inline void ObFetchFromtoStmt::set_cursor_name(const common::ObString& name)
		{
	      cursor_name_ = name;
		}
	    inline const common::ObString& ObFetchFromtoStmt::get_cursor_name() const
		{
		   return cursor_name_;
	    }
	    inline void ObFetchFromtoStmt::set_count_f(int64_t fetch_count)
		{
		   count_f_ = fetch_count;
		}
	    inline int64_t ObFetchFromtoStmt::get_count_f()
		{
		   return count_f_;
	    }
		inline void ObFetchFromtoStmt::set_count_t(int64_t fetch_count)
		{
		   count_t_ = fetch_count;
		}
	    inline int64_t ObFetchFromtoStmt::get_count_t()
		{
		   return count_t_;
		}
	}
}

#endif //OCEANBASE_SQL_FETCH_STMT_H
