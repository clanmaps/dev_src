/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_declare_stmt.h
* @brief this class  present a declare cursor logic plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_DECLARE_STMT_H_
#define OCEANBASE_SQL_OB_DECLARE_STMT_H_
#include "ob_basic_stmt.h"
#include "common/ob_string.h"
namespace oceanbase
{
	namespace sql
	{
        /**
         * @brief The ObCursorDeclareStmt class
         */
		class ObCursorDeclareStmt : public ObBasicStmt
		{
		  public:
		  ObCursorDeclareStmt()
		  : ObBasicStmt(T_CURSOR_DECLARE)
		  {
		   declare_query_id_ = common::OB_INVALID_ID;
		  }
		  virtual ~ObCursorDeclareStmt(){}
          /**
               * @brief set_cursor_name
               * @param name
               */
	          void set_cursor_name(const common::ObString& name);
              /**
               * @brief set_declare_query_id
               * @param query_id
               */
	          void set_declare_query_id(const uint64_t query_id);
              /**
               * @brief get_cursor_name
               * @return
               */
	          const common::ObString& get_cursor_name() const;
              /**
               * @brief get_declare_query_id
               * @return
               */
	          uint64_t get_declare_query_id() const;
	          virtual void print(FILE* fp, int32_t level, int32_t index);
		  private:
          common::ObString cursor_name_;///> cursor name
          common::ObString declare_sql_;///> declare cursor sql
          uint64_t declare_query_id_;///> query id
		};
		inline void ObCursorDeclareStmt::set_cursor_name(const common::ObString& name)
		{
			cursor_name_=name;
		}
		inline const common::ObString& ObCursorDeclareStmt::get_cursor_name()const
		{
			return cursor_name_;
		}
		inline uint64_t ObCursorDeclareStmt::get_declare_query_id()const
		{
			return declare_query_id_;
		}
		inline void ObCursorDeclareStmt::set_declare_query_id(const uint64_t query_id_)
		{
			declare_query_id_= query_id_;
		}
	}
}


#endif //OCEANBASE_SQL_OB_DECLARE_STMT_H_
