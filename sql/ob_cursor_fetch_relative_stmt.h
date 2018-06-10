/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_relative_stmt.h
* @brief this class  present a "cursor fetch relative" logical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_FETCH_RELATIVE_STMT_H
#define OCEANBASE_SQL_FETCH_RELATIVE_STMT_H
#include "ob_basic_stmt.h"
#include "common/ob_string.h"
#include "common/ob_array.h"

namespace oceanbase
{
	namespace sql
	{
      /**
       * @brief The ObFetchRelativeStmt class
       */
	  class ObFetchRelativeStmt : public ObBasicStmt
	  {
		 public:
		 ObFetchRelativeStmt()
			: ObBasicStmt(T_CURSOR_FETCH_RELATIVE)
		      {
		      }
		 virtual ~ObFetchRelativeStmt(){}
         /**
          * @brief set_cursor_name
          * @param name
          */
		 void set_cursor_name(const common::ObString& name);
         /**
          * @brief get_cursor_name
          * @return
          */
		 const common::ObString& get_cursor_name() const;
         /**
          * @brief set_is_next
          * @param is_next
          */
		 void set_is_next(int64_t is_next);
         /**
          * @brief get_is_next
          * @return
          */
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
		 virtual void print(FILE* fp, int32_t level, int32_t index);
		 private:
         common::ObString  cursor_name_;///> cursor name
         common::ObArray<common::ObString> variable_names_;///> variable name list
         int64_t is_next_;///> is next flag
         int64_t  fetch_count_;///> fetch ount
	  };
	  inline void ObFetchRelativeStmt::set_cursor_name(const common::ObString& name)
	  {
	     cursor_name_ = name;
	  }
      inline const common::ObString& ObFetchRelativeStmt::get_cursor_name() const
      {
         return cursor_name_;
      }
      inline void ObFetchRelativeStmt::set_is_next(int64_t is_next)
      {
 	     is_next_ = is_next;
      }
	  inline int64_t ObFetchRelativeStmt::get_is_next()
   	  {
   	     return is_next_;
      }
	  inline void ObFetchRelativeStmt::set_fetch_count(int64_t fetch_count)
      {
         fetch_count_ = fetch_count;
      }
      inline int64_t ObFetchRelativeStmt::get_fetch_count()
      {
         return fetch_count_;
      }
	}
}

#endif //OCEANBASE_SQL_FETCH_STMT_H
