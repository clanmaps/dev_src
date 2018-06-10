/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_first_stmt.h
* @brief this class  present a "cursor fetch first" logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#ifndef OCEANBASE_SQL_FETCH_FIRST_STMT_H
#define OCEANBASE_SQL_FETCH_FIRST_STMT_H
#include "ob_basic_stmt.h"
#include "common/ob_string.h"
#include "common/ob_array.h"

namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObFetchFirstStmt class
         */
		class ObFetchFirstStmt : public ObBasicStmt
		{
		  public:
		  ObFetchFirstStmt()
			: ObBasicStmt(T_CURSOR_FETCH_FIRST)
		      {
		      }
		  virtual ~ObFetchFirstStmt(){}
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
		  virtual void print(FILE* fp, int32_t level, int32_t index);
		  private:
          common::ObString  cursor_name_;///> cursor name
          common::ObArray<common::ObString> variable_names_;///> variables list
		};
		inline void ObFetchFirstStmt::set_cursor_name(const common::ObString& name)
		{
	      cursor_name_ = name;
		}
		inline const common::ObString& ObFetchFirstStmt::get_cursor_name() const
		{
	      return cursor_name_;
		}

	}
}

#endif //OCEANBASE_SQL_FETCH_STMT_H
