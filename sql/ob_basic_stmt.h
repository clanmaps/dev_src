/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_basic_stmt.h
 * @brief basic statement
 *
 * modified by longfei：add three basic statement: T_CREATE_INDEX, T_DROP_INDEX, T_SHOW_INDEX
 * modified by zhujun：add procedure related StmtType
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author zhujun <51141500091@ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_27
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_basic_stmt.h
 *
 * Authors:
 *   Guibin Du <tianguan.dgb@taobao.com>
 *
 */

#ifndef OCEANBASE_SQL_OB_BASIC_STMT_H_
#define OCEANBASE_SQL_OB_BASIC_STMT_H_

#include "common/ob_define.h"

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObBasicStmt class
     * basic statement class of logical plan has commen data menber and function menber of sub-class
     */
    class ObBasicStmt
    {
    public:
      enum StmtType
      {
	    //zhounan unmark:b
        T_CURSOR_DECLARE,
        T_CURSOR_FETCH,
        T_CURSOR_FETCH_INTO,
        T_CURSOR_FETCH_PRIOR,
        T_CURSOR_FETCH_PRIOR_INTO,
        T_CURSOR_FETCH_FIRST,
        T_CURSOR_FETCH_FIRST_INTO,
        T_CURSOR_FETCH_LAST,
        T_CURSOR_FETCH_LAST_INTO,
        T_CURSOR_FETCH_RELATIVE,
        T_CURSOR_FETCH_RELATIVE_INTO,
        T_CURSOR_FETCH_ABSOLUTE,
        T_CURSOR_FETCH_ABS_INTO,
        T_CURSOR_FETCH_FROMTO,
        T_CURSOR_OPEN,
        T_CURSOR_CLOSE,
		//add:e
        T_NONE,
        T_SELECT,
        T_INSERT,
        T_REPLACE,
        T_DELETE,
        T_UPDATE,
        T_EXPLAIN,
        T_CREATE_TABLE,
        T_DROP_TABLE,
        T_ALTER_TABLE,
      //add wangjiahao [table lock] 20160616 :b
        T_LOCK_TABLE,
      //add :e

      //add hxlong [Truncate Table]: 20170403:b
        T_TRUNCATE_TABLE,
      //add:e

        // show statements
        T_SHOW_TABLES,
        //add longfei
        T_SHOW_INDEX,
        //add:e
        T_SHOW_COLUMNS,
        T_SHOW_VARIABLES,
        T_SHOW_TABLE_STATUS,
        T_SHOW_SCHEMA,
        T_SHOW_CREATE_TABLE,
        T_SHOW_PARAMETERS,
        T_SHOW_SERVER_STATUS,
        T_SHOW_WARNINGS,
        T_SHOW_GRANTS,
        T_SHOW_PROCESSLIST,

        // privileges related
        T_CREATE_USER,
        T_DROP_USER,
        T_SET_PASSWORD,
        T_LOCK_USER,
        T_RENAME_USER,
        T_GRANT,
        T_REVOKE,

        T_PREPARE,
        T_VARIABLE_SET,
        T_EXECUTE,
        T_DEALLOCATE,

        T_START_TRANS,
        T_END_TRANS,

        T_KILL,
        T_ALTER_SYSTEM,
        T_CHANGE_OBI,

        // add longfei [create index] [secondaryindex reconstruct] 20150916:b
        // secondary index related
        T_CREATE_INDEX,
        // add e
        //longfei [drop index]
        T_DROP_INDEX,
        //add by zhujun:b
        //code_coverage_zhujun
        T_PROCEDURE,
        T_PROCEDURE_CREATE,
        T_PROCEDURE_DROP,
        T_PROCEDURE_DECLARE,
        T_PROCEDURE_ASSGIN,
        T_PROCEDURE_WHILE,
        T_PROCEDURE_LOOP,
        T_PROCEDURE_EXIT,//add by wangdonghui
        T_PROCEDURE_CASE,
        T_PROCEDURE_CASEWHEN,
        T_PROCEDURE_IF,
        T_PROCEDURE_ELSEIF,
        T_PROCEDURE_ELSE,
        T_PROCEDURE_EXEC,
        T_PROCEDURE_SELECT_INTO,
        T_VARIABLE_SET_ARRAY_VALUE
        //code_coverage_zhujun
        //add:e
      };

      ObBasicStmt()
        : stmt_type_(T_NONE)
      {
      }
      explicit ObBasicStmt(const StmtType stmt_type)
        : stmt_type_(stmt_type)
      {
      }
      explicit ObBasicStmt(const StmtType stmt_type, uint64_t query_id)
        : stmt_type_(stmt_type), query_id_(query_id)
      {
      }
      virtual ~ObBasicStmt() {}

      void set_stmt_type(const StmtType stmt_type);
      void set_query_id(const uint64_t query_id);
      StmtType get_stmt_type() const;
      uint64_t get_query_id() const;
      bool is_show_stmt() const;

      virtual void print(FILE* fp, int32_t level, int32_t index) = 0;
    protected:
      void print_indentation(FILE* fp, int32_t level) const;

    private:
      StmtType  stmt_type_;  ///<  statement type
      uint64_t  query_id_;  ///<  query id
    };

    inline void ObBasicStmt::set_stmt_type(StmtType stmt_type)
    {
      stmt_type_ = stmt_type;
    }

    inline ObBasicStmt::StmtType ObBasicStmt::get_stmt_type() const
    {
      return stmt_type_;
    }

    inline uint64_t ObBasicStmt::get_query_id() const
    {
      return query_id_;
    }

    inline void ObBasicStmt::set_query_id(const uint64_t query_id)
    {
      query_id_ = query_id;
    }

    inline void ObBasicStmt::print_indentation(FILE* fp, int32_t level) const
    {
      for(int i = 0; i < level; ++i)
        fprintf(fp, "    ");
    }

    inline bool ObBasicStmt::is_show_stmt() const
    {
      return (stmt_type_ >= T_SHOW_TABLES and stmt_type_ <= T_SHOW_SERVER_STATUS);
    }
  }
}

#endif //OCEANBASE_SQL_OB_BASIC_STMT_H_
