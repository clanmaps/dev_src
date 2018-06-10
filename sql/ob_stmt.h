/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_stmt.h
 * @brief basic structure and functions
 *
 * modified by longfei：add struct ObQueryHint for user use hint in select
 * modified by yu shengjuan : storing such as table_id column_id table_name and other information from sql
 * modified by maoxiaoxiao: add struct ObQueryHint for user to use hint for bloomfilter join or merge join
 * modified by wangdonghui: add hint of no group execution
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author   yu shengjuan <51141500090@ecnu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @date 2016_07_30
 */
#ifndef OCEANBASE_SQL_STMT_H_
#define OCEANBASE_SQL_STMT_H_
#include "common/ob_row_desc.h"
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_vector.h"
#include "common/ob_hint.h"
#include "sql/ob_basic_stmt.h"
#include "parse_node.h"

namespace oceanbase
{ 
  namespace sql
  {
    using namespace common;
    //add longfei [Index Hint]
    struct IndexTableNamePair
    {
      common::ObString src_table_name_;
      common::ObString index_table_name_;
      uint64_t src_table_id_;
      uint64_t index_table_id_;
      IndexTableNamePair()
      {
        src_table_id_ = common::OB_INVALID_ID;
        index_table_id_ = common::OB_INVALID_ID;
        src_table_name_.assign_buffer(src_tb_buf, OB_MAX_TABLE_NAME_LENGTH);
        index_table_name_.assign_buffer(index_tb_buf, OB_MAX_TABLE_NAME_LENGTH);
      }
    private:
      char src_tb_buf[OB_MAX_TABLE_NAME_LENGTH];
      char index_tb_buf[OB_MAX_TABLE_NAME_LENGTH];
    };
    //add e

     // add by yusj[SEMI_JOIN] 20150819
     struct ObSemiTableList
     {
     	common::ObString join_left_table_name_;
     	common::ObString join_left_column_name;
     	common::ObString join_right_table_name_;
     	common::ObString join_right_column_name;
     	uint64_t left_table_id_;
     	uint64_t right_table_id_;
     	uint64_t left_column_id_;
     	uint64_t right_column_id_;
     	ObSemiTableList()
     	{
     		left_table_id_ = common::OB_INVALID_ID;
     		right_table_id_ = common::OB_INVALID_ID;
     		left_column_id_ = common::OB_INVALID_ID;
     		right_column_id_ = common::OB_INVALID_ID;
     		join_left_table_name_.assign_buffer(left_tb_buf, OB_MAX_TABLE_NAME_LENGTH);
     		join_right_table_name_.assign_buffer(right_tb_buf, OB_MAX_TABLE_NAME_LENGTH);
     		join_left_column_name.assign_buffer(left_col_buf, OB_MAX_TABLE_NAME_LENGTH);
     		join_right_column_name.assign_buffer(right_col_buf, OB_MAX_TABLE_NAME_LENGTH);
     	}
     private:
     	char left_tb_buf[OB_MAX_TABLE_NAME_LENGTH];
     	char right_tb_buf[OB_MAX_TABLE_NAME_LENGTH];
      char left_col_buf[OB_MAX_TABLE_NAME_LENGTH];
      char right_col_buf[OB_MAX_TABLE_NAME_LENGTH];
     };
     //add e

     /*add maoxx [bloomfilter_join] 20160406*/
     struct ObJoinOPTypeArray
     {
         ObItemType join_op_type_;
         int32_t index_;
         ObJoinOPTypeArray()
         {
           join_op_type_ = T_INVALID;
           index_ = 0;
         }
     };
     /*add e*/
  }

  namespace common
  {
    template <>
    struct ob_vector_traits<oceanbase::sql::IndexTableNamePair>
    {
      typedef oceanbase::sql::IndexTableNamePair* pointee_type;
      typedef oceanbase::sql::IndexTableNamePair value_type;
      typedef const oceanbase::sql::IndexTableNamePair const_value_type;
      typedef value_type* iterator;
      typedef const value_type* const_iterator;
      typedef int32_t difference_type;
    };

    template <>
    struct ob_vector_traits<oceanbase::sql::ObSemiTableList>
    {
      typedef oceanbase::sql::ObSemiTableList* pointee_type;
      typedef oceanbase::sql::ObSemiTableList value_type;
      typedef const oceanbase::sql::ObSemiTableList const_value_type;
      typedef value_type* iterator;
      typedef const value_type* const_iterator;
      typedef int32_t difference_type;
    };

    /*add maoxx [bloomfilter_join] 20160406*/
    template <>
      struct ob_vector_traits<oceanbase::sql::ObJoinOPTypeArray>
      {
        typedef oceanbase::sql::ObJoinOPTypeArray* pointee_type;
        typedef oceanbase::sql::ObJoinOPTypeArray value_type;
        typedef const oceanbase::sql::ObJoinOPTypeArray const_value_type;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef int32_t difference_type;
      };
    /*add e*/
  }

  namespace sql
  {
    struct ObQueryHint
    {
      ObQueryHint()
      {
        hotspot_ = false;
        read_consistency_ = common::NO_CONSISTENCY;
        //add by wdh 20160716
        no_group_ = false;  ///<  Default open optimization
      }
      bool has_index_hint() const
      {
        return use_index_array_.size() > 0 ? true : false;
      }
      bool has_semi_join_hint() const
      {
        return use_join_array_.size() > 0 ? true:false;
      }
      bool hotspot_;
      common::ObConsistencyLevel read_consistency_;
      common::ObVector<IndexTableNamePair> use_index_array_; // add by longfei [Index Hint]
      common::ObVector<ObSemiTableList> use_join_array_; // add by yusj [SEMI_JOIN] 20150819
      //add by wdh 20160716
      bool  no_group_;  ///<  no group execution flag
      /*add maoxx [bloomfilter_join] 20160406*/
      common::ObVector<ObJoinOPTypeArray> join_op_type_array_;
      /*add e*/
    };
    
    struct TableItem
    {
      TableItem()
      {
        table_id_ = common::OB_INVALID_ID;
        ref_id_ = common::OB_INVALID_ID;
        has_scan_columns_ = false;
      }
      
      enum TableType
    	{
        BASE_TABLE, //0
        ALIAS_TABLE, //1
        GENERATED_TABLE, //2
    	};

      // if real table id, it is valid for all threads,
      // else if generated id, it is unique just during the thread session
      uint64_t    table_id_;
      common::ObString    table_name_;
      common::ObString    alias_name_;
      TableType   type_;
      // type == BASE_TABLE? ref_id_ is the real Id of the schema
      // type == ALIAS_TABLE? ref_id_ is the real Id of the schema, while table_id_ new generated
      // type == GENERATED_TABLE? ref_id_ is the reference of the sub-query.
      uint64_t    ref_id_;
      bool        has_scan_columns_;
    };

    struct ColumnItem
    {
      uint64_t    column_id_;
      common::ObString    column_name_;
      uint64_t    table_id_;
      uint64_t    query_id_;
      // This attribute is used by resolver, to mark if the column name is unique of all from tables
      bool        is_name_unique_;
      bool        is_group_based_;
      // TBD, we can not calculate resulte type now.
      common::ObObjType     data_type_;
    };
  }

  namespace common
  {
    template <>
      struct ob_vector_traits<oceanbase::sql::TableItem>
      {
        typedef oceanbase::sql::TableItem* pointee_type;
        typedef oceanbase::sql::TableItem value_type;
        typedef const oceanbase::sql::TableItem const_value_type;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef int32_t difference_type;
      };

    template <>
      struct ob_vector_traits<oceanbase::sql::ColumnItem>
      {
        typedef oceanbase::sql::ColumnItem* pointee_type;
        typedef oceanbase::sql::ColumnItem value_type;
        typedef const oceanbase::sql::ColumnItem const_value_type;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef int32_t difference_type;
      };

    template <>
      struct ob_vector_traits<uint64_t>
      {
        typedef uint64_t* pointee_type;
        typedef uint64_t value_type;
        typedef const uint64_t const_value_type;
        typedef value_type* iterator;
        typedef const value_type* const_iterator;
        typedef int32_t difference_type;
      };
  }

  namespace sql
  {
    class ObLogicalPlan;
    class ObStmt : public ObBasicStmt
    {
    public:
      ObStmt(common::ObStringBuf* name_pool, StmtType type);
      virtual ~ObStmt();

      int32_t get_table_size() const { return table_items_.size(); }
      int32_t get_column_size() const { return column_items_.size(); }
      int32_t get_condition_size() const { return where_expr_ids_.size(); }
      int32_t get_when_fun_size() const { return when_func_ids_.size(); }
      int32_t get_when_expr_size() const { return when_expr_ids_.size(); }
      int check_table_column(
          ResultPlan& result_plan,
          const common::ObString& column_name, 
          const TableItem& table_item,
          uint64_t& column_id,
          common::ObObjType& column_type);
      int add_table_item(
          ResultPlan& result_plan,
          const common::ObString& table_name,
          const common::ObString& alias_name, 
          uint64_t& table_id,
          const TableItem::TableType type,
          const uint64_t ref_id = common::OB_INVALID_ID);
      int add_column_item(
          ResultPlan& result_plan,
          const oceanbase::common::ObString& column_name,
          const oceanbase::common::ObString* table_name = NULL,
          ColumnItem** col_item = NULL);
      int add_column_item(const ColumnItem& column_item);
      ColumnItem* get_column_item(
        const common::ObString* table_name,
        const common::ObString& column_name);
      ColumnItem* get_column_item_by_id(uint64_t table_id, uint64_t column_id) const ;
      const ColumnItem* get_column_item(int32_t index) const 
      {
        const ColumnItem *column_item = NULL;
        if (0 <= index && index < column_items_.size())
        {
          column_item = &column_items_[index];
        }
        return column_item;
      }
      TableItem* get_table_item_by_id(uint64_t table_id) const;
      TableItem& get_table_item(int32_t index) const 
      {
        OB_ASSERT(0 <= index && index < table_items_.size());
        return table_items_[index];
      }
      
      // add by lxb on 2017/02/12
      common::ObVector<TableItem>& get_table_items()
      {
        return table_items_;
      }
      
      uint64_t get_condition_id(int32_t index) const 
      {
        OB_ASSERT(0 <= index && index < where_expr_ids_.size());
        return where_expr_ids_[index];
      }
      uint64_t get_table_item(
        const common::ObString& table_name,
        TableItem** table_item = NULL) const ;
      int32_t get_table_bit_index(uint64_t table_id) const ;

      // add by lxb on 2017/2/8
      common::ObVector<ColumnItem>& get_column_items()
      {
        return column_items_;
      }
      
      common::ObVector<uint64_t>& get_where_exprs() 
      {
        return where_expr_ids_;
      }

      common::ObVector<uint64_t>& get_when_exprs() 
      {
        return when_expr_ids_;
      }

      common::ObStringBuf* get_name_pool() const 
      {
        return name_pool_;
      }

      ObQueryHint& get_query_hint()
      {
        return query_hint_;
      }
      
      int add_when_func(uint64_t expr_id)
      {
        return when_func_ids_.push_back(expr_id);
      }

      uint64_t get_when_func_id(int32_t index) const
      {
        OB_ASSERT(0 <= index && index < when_func_ids_.size());
        return when_func_ids_[index];
      }

      uint64_t get_when_expr_id(int32_t index) const
      {
        OB_ASSERT(0 <= index && index < when_expr_ids_.size());
        return when_expr_ids_[index];
      }

      void set_when_number(const int64_t when_num)
      {
        when_number_ = when_num;
      }

      int64_t get_when_number() const
      {
        return when_number_;
      }

      virtual void print(FILE* fp, int32_t level, int32_t index = 0);
      
      // add by lxb on 2017/03/21 for logical optimizer
      common::ObRowDesc* get_table_hash()
      {
        return &tables_hash_;
      }

    protected:
      common::ObStringBuf* name_pool_;
      common::ObVector<TableItem>    table_items_;  // from (what)
      common::ObVector<ColumnItem>   column_items_; // select (what)

    private:
      //uint64_t  where_expr_id_;
      common::ObVector<uint64_t>     where_expr_ids_;
      common::ObVector<uint64_t>     when_expr_ids_;
      common::ObVector<uint64_t>     when_func_ids_;
      ObQueryHint                    query_hint_;

      int64_t  when_number_;

      // it is only used to record the table_id--bit_index map
      // although it is a little weird, but it is high-performance than ObHashMap
      common::ObRowDesc tables_hash_;
    };
  }
}

#endif //OCEANBASE_SQL_STMT_H_
