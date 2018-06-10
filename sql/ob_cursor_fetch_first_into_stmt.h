/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_fetch_first_into_stmt.h
* @brief this class  present a "fetch cursor first into" logical plan in oceanbase
*
* Created by zhujun: support procedure
*
* @version CEDAR 0.2 
* @author zhujun <51141500091@ecnu.edu.cn>
* @date 2014_11_23
*/
#ifndef OCEANBASE_SQL_OB_SURSOR_FETCH_FIRST_INTO_STMT_H_
#define OCEANBASE_SQL_OB_SURSOR_FETCH_FIRST_INTO_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include <map>
using namespace oceanbase::common;

namespace oceanbase {
namespace sql {
/**
 * @brief The ObCursorFetchFirstIntoStmt class
 */
class ObCursorFetchFirstIntoStmt: public ObBasicStmt {
	public:
	ObCursorFetchFirstIntoStmt() :
				ObBasicStmt(T_CURSOR_FETCH_FIRST_INTO) {
			cursor_stmt_id_=common::OB_INVALID_ID;
		}
		virtual ~ObCursorFetchFirstIntoStmt() {
		}
		virtual void print(FILE* fp, int32_t level, int32_t index);

        /**
         * @brief set_cursor_id
         * @param stmt_id
         * @return
         */
		int set_cursor_id(uint64_t stmt_id);
        /**
         * @brief get_cursor_id
         * @return
         */
		uint64_t get_cursor_id();
        /**
         * @brief add_variable
         * @param name
         * @return
         */
		int add_variable(ObString &name);
        /**
         * @brief get_variable
         * @param index
         * @return
         */
		ObString& get_variable(int64_t index);
        /**
         * @brief get_variable_size
         * @return
         */
		int64_t get_variable_size();

	private:
		uint64_t cursor_stmt_id_;
		ObArray<ObString> variable_name_;

	};


}
}

#endif
