/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_absolute_stmt.h
* @brief this class  present a "cursor fetch absolute" logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_FETCH_ABSOLUTE_STMT_H
#define OCEANBASE_SQL_FETCH_ABSOLUTE_STMT_H
#include "ob_basic_stmt.h"
#include "common/ob_string.h"
#include "common/ob_array.h"

namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObFetchAbsoluteStmt class
         */
		class ObFetchAbsoluteStmt : public ObBasicStmt
		{
	      public:
		  ObFetchAbsoluteStmt()
			: ObBasicStmt(T_CURSOR_FETCH_ABSOLUTE)
		      {
		      }
		  virtual ~ObFetchAbsoluteStmt(){}
		  void set_cursor_name(const common::ObString& name);
		  const common::ObString& get_cursor_name() const;
		  void set_is_next(int64_t is_next);
		  int64_t get_is_next();
		  void set_fetch_count(int64_t fetch_count);
		  int64_t get_fetch_count();
		  virtual void print(FILE* fp, int32_t level, int32_t index);
          private:
	      common::ObString  cursor_name_;
	      common::ObArray<common::ObString> variable_names_;
	      int64_t is_next_;
	      int64_t  fetch_count_;
		};

		inline void ObFetchAbsoluteStmt::set_cursor_name(const common::ObString& name)
		{
	      cursor_name_ = name;
		}
        inline const common::ObString& ObFetchAbsoluteStmt::get_cursor_name() const
		{
	      return cursor_name_;
        }
        inline void ObFetchAbsoluteStmt::set_is_next(int64_t is_next)
   		{
   		  is_next_ = is_next;
   		}
        inline int64_t ObFetchAbsoluteStmt::get_is_next()
   		{
   		  return is_next_;
   		}
  	    inline void ObFetchAbsoluteStmt::set_fetch_count(int64_t fetch_count)
        {
    	   fetch_count_ = fetch_count;
        }
        inline int64_t ObFetchAbsoluteStmt::get_fetch_count()
        {
    	   return fetch_count_;
        }
	}
}

#endif //OCEANBASE_SQL_FETCH_STMT_H
