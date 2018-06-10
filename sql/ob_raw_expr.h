/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_raw_expr.h
 * @brief raw expression relation class definition
 *
 * modified by zhutao:add some functions and a class for procedure
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_29
 */

#ifndef OCEANBASE_SQL_RAWEXPR_H_
#define OCEANBASE_SQL_RAWEXPR_H_
#include "common/ob_bit_set.h"
#include "ob_sql_expression.h"
#include "common/ob_define.h"
#include "common/ob_vector.h"
#include "common/ob_string.h"
#include "common/ob_string_buf.h"

// add by lxb on 2017/02/15 for logical optimizer
#include <map>

namespace oceanbase
{
  namespace sql
  {
    // add by lxb on 2017/02/15 for logical optimizer
    class ObSelectStmt;
    
    class ObTransformer;
    class ObLogicalPlan;
    class ObPhysicalPlan;

    class ObRawExpr
    {
    public:
      explicit ObRawExpr(ObItemType expr_type = T_INVALID)
          :type_(expr_type)
      {
        result_type_ = ObMinType;
      }
      virtual ~ObRawExpr() {}
      //virtual void trace(FILE *stream, int indentNum = 0);
      const ObItemType get_expr_type() const { return type_; }
      const common::ObObjType & get_result_type() const { return result_type_; }
      void set_expr_type(ObItemType type) { type_ = type; }
      void set_result_type(const common::ObObjType & type) { result_type_ = type; }

      bool is_const() const;
      bool is_column() const;
      // Format like "C1 = 5"
      bool is_equal_filter() const;
      // Format like "C1 between 5 and 10"
      bool is_range_filter() const;
      // Only format like "T1.c1 = T2.c1", not "T1.c1 - T2.c1 = 0"
      bool is_join_cond() const;
      bool is_aggr_fun() const;
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const = 0;
      virtual void print(FILE* fp, int32_t level) const = 0;
      /**
       * @brief get_raw_var
       * push expressin to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr*> &exprs) const  {UNUSED(exprs);} //add zt for find variables
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type) = 0;
      
    private:
      ObItemType  type_;
      common::ObObjType result_type_;
    };

    class ObConstRawExpr : public ObRawExpr
    {
    public:
      ObConstRawExpr()
      {
      }
      ObConstRawExpr(oceanbase::common::ObObj& val, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), value_(val)
      {
      }
      virtual ~ObConstRawExpr() {}
      const oceanbase::common::ObObj& get_value() const { return value_; }
      void set_value(const oceanbase::common::ObObj& val) { value_ = val; }
      int set_value_and_type(const common::ObObj& val);
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        exprs.push_back(this);
      }
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      oceanbase::common::ObObj value_;
    };

    //add zt 20151125:b
    /**
     * @brief The ObArrayRawExpr class
     * added array expression definition
     */
    class ObArrayRawExpr : public ObRawExpr
    {
      public:
        /**
         * @brief ObArrayRawExpr constructor
         */
        ObArrayRawExpr() : ObRawExpr(T_ARRAY)
        {
        }
        /**
         * @brief set_array_name
         * set array name
         * @param array_name array name
         */
        void set_array_name(const ObString &array_name) { array_name_ = array_name; }
        /**
         * @brief set_idx_value
         * set index value
         * @param obj index value
         */
        void set_idx_value(const ObObj &obj) { idx_value_ = obj; }
        /**
         * @brief get_array_name
         * get array name
         * @return array name
         */
        const ObString& get_array_name() const { return array_name_; }
        /**
         * @brief get_idx_value
         * get index value
         * @return index value
         */
        const ObObj & get_idx_value() const { return idx_value_; }
        /**
         * @brief fill_sql_expression
         * fill sql expression
         * @param inter_expr sql expression
         * @param transformer ObTransformer object pointer
         * @param logical_plan logical plan
         * @param physical_plan physical plan
         * @return error code
         */
        virtual int fill_sql_expression(
            ObSqlExpression& inter_expr,
            ObTransformer *transformer = NULL,
            ObLogicalPlan *logical_plan = NULL,
            ObPhysicalPlan *physical_plan = NULL) const;
        /**
         * @brief print
         * print array expression information
         * @param fp
         * @param level
         */
        void print(FILE* fp, int32_t level) const;
        /**
         * @brief get_raw_var
         * push array expression to stack
         * @param exprs expression stack
         */
        virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
        {
          //remains problem, how to answer this questions
          exprs.push_back(this);
        }
        
        // add by lxb on 2017/03/21 for logical optimizer
        virtual int optimize_sql_expression(
            ObSelectStmt *&main_stmt,
            common::ObBitSet<> &bit_set,
            std::map<uint64_t, uint64_t> table_id_hashmap,
            std::map<uint64_t, uint64_t> column_id_hashmap,
            uint64_t table_id,
            uint64_t &real_table_id,
            std::map<uint64_t, uint64_t> alias_table_hashmap,
            int type);
        
      private:
        oceanbase::common::ObString array_name_;  ///<  array name
        ObObj idx_value_;  ///< array index value
        //      ObRawExpr *idx_expr_;
    };
    //add zt 20151125:e

    class ObCurTimeExpr : public ObRawExpr
    {
      public:
        explicit ObCurTimeExpr():ObRawExpr(T_CUR_TIME) {}
        virtual ~ObCurTimeExpr() {}
        virtual int fill_sql_expression(
            ObSqlExpression& inter_expr,
            ObTransformer *transformer = NULL,
            ObLogicalPlan *logical_plan = NULL,
            ObPhysicalPlan *physical_plan = NULL) const;
        void print(FILE* fp, int32_t level) const;
        
        // add by lxb on 2017/02/15 for logical optimizer
        virtual int optimize_sql_expression(
            ObSelectStmt *&main_stmt,
            common::ObBitSet<> &bit_set,
            std::map<uint64_t, uint64_t> table_id_hashmap,
            std::map<uint64_t, uint64_t> column_id_hashmap,
            uint64_t table_id,
            uint64_t &real_table_id,
            std::map<uint64_t, uint64_t> alias_table_hashmap,
            int type);
    };

    class ObUnaryRefRawExpr : public ObRawExpr
    {
    public:
      ObUnaryRefRawExpr()
      {
        id_ = OB_INVALID_ID;
      }
      ObUnaryRefRawExpr(uint64_t id, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), id_(id)
      {
      }
      virtual ~ObUnaryRefRawExpr() {}
      uint64_t get_ref_id() const { return id_; }
      void set_ref_id(uint64_t id) { id_ = id; }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;
      int get_name(common::ObString& name) const;
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);

    private:
      uint64_t id_;
    };

    class ObBinaryRefRawExpr : public ObRawExpr
    {
    public:
      ObBinaryRefRawExpr()
      {
        first_id_ = OB_INVALID_ID;
        second_id_ = OB_INVALID_ID;
      }
      ObBinaryRefRawExpr(uint64_t first_id, uint64_t second_id, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), first_id_(first_id), second_id_(second_id)
      {
      }
      virtual ~ObBinaryRefRawExpr() {}
      uint64_t get_first_ref_id() const { return first_id_; }
      uint64_t get_second_ref_id() const { return second_id_; }
      void set_first_ref_id(uint64_t id) { first_id_ = id; }
      void set_second_ref_id(uint64_t id) { second_id_ = id; }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap, 
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);

    private:
      uint64_t first_id_;
      uint64_t second_id_;
    };

    class ObUnaryOpRawExpr : public ObRawExpr
    {
    public:
      ObUnaryOpRawExpr()
      {
        expr_ = NULL;
      }
      ObUnaryOpRawExpr(ObRawExpr *expr, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), expr_(expr)
      {
      }
      virtual ~ObUnaryOpRawExpr() {}
      ObRawExpr* get_op_expr() const { return expr_; }
      void set_op_expr(ObRawExpr *expr) { expr_ = expr; }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      //add zt: 20151104
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        expr_->get_raw_var(exprs);
      }
      //add zt: 20151104
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      ObRawExpr *expr_;
    };

    class ObBinaryOpRawExpr : public ObRawExpr
    {
    public:
      ObBinaryOpRawExpr()
      {
      }
      ObBinaryOpRawExpr(
          ObRawExpr *first_expr, ObRawExpr *second_expr, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), first_expr_(first_expr), second_expr_(second_expr)
      {
      }
      virtual ~ObBinaryOpRawExpr(){}
      ObRawExpr* get_first_op_expr() const { return first_expr_; }
      ObRawExpr* get_second_op_expr() const { return second_expr_; }
      void set_op_exprs(ObRawExpr *first_expr, ObRawExpr *second_expr);
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        first_expr_->get_raw_var(exprs);
        second_expr_->get_raw_var(exprs);
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap, 
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      ObRawExpr *first_expr_;
      ObRawExpr *second_expr_;
    };

    class ObTripleOpRawExpr : public ObRawExpr
    {
    public:
      ObTripleOpRawExpr()
      {
        first_expr_ = NULL;
        second_expr_ = NULL;
        third_expr_ = NULL;
      }
      ObTripleOpRawExpr(
          ObRawExpr *first_expr, ObRawExpr *second_expr,
          ObRawExpr *third_expr, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type),
          first_expr_(first_expr), second_expr_(second_expr),
          third_expr_(third_expr)
      {
      }
      virtual ~ObTripleOpRawExpr(){}
      ObRawExpr* get_first_op_expr() const { return first_expr_; }
      ObRawExpr* get_second_op_expr() const { return second_expr_; }
      ObRawExpr* get_third_op_expr() const { return third_expr_; }
      void set_op_exprs(ObRawExpr *first_expr, ObRawExpr *second_expr, ObRawExpr *third_expr);
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        first_expr_->get_raw_var(exprs);
        second_expr_->get_raw_var(exprs);
        third_expr_->get_raw_var(exprs);
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      ObRawExpr *first_expr_;
      ObRawExpr *second_expr_;
      ObRawExpr *third_expr_;
    };

    class ObMultiOpRawExpr : public ObRawExpr
    {
    public:
      ObMultiOpRawExpr()
      {
      }
      virtual ~ObMultiOpRawExpr(){}
      ObRawExpr* get_op_expr(int32_t index) const
      {
        ObRawExpr* expr = NULL;
        if (index >= 0 && index < exprs_.size())
          expr = exprs_.at(index);
        return expr;
      }
      int add_op_expr(ObRawExpr *expr) { return exprs_.push_back(expr); }
      int32_t get_expr_size() const { return exprs_.size(); }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        for(int32_t i = 0; i < exprs_.size(); ++i)
        {
          exprs_.at(i)->get_raw_var(exprs);
        }
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      oceanbase::common::ObVector<ObRawExpr*> exprs_;
    };

    class ObCaseOpRawExpr : public ObRawExpr
    {
    public:
      ObCaseOpRawExpr()
      {
        arg_expr_ = NULL;
        default_expr_ = NULL;
      }
      virtual ~ObCaseOpRawExpr(){}
      ObRawExpr* get_arg_op_expr() const { return arg_expr_; }
      ObRawExpr* get_default_op_expr() const { return default_expr_; }
      ObRawExpr* get_when_op_expr(int32_t index) const
      {
        ObRawExpr* expr = NULL;
        if (index >= 0 && index < when_exprs_.size())
          expr = when_exprs_[index];
        return expr;
      }
      ObRawExpr* get_then_op_expr(int32_t index) const
      {
        ObRawExpr* expr = NULL;
        if (index >= 0 || index < then_exprs_.size())
          expr = then_exprs_[index];
        return expr;
      }
      void set_arg_op_expr(ObRawExpr *expr) { arg_expr_ = expr; }
      void set_default_op_expr(ObRawExpr *expr) { default_expr_ = expr; }
      int add_when_op_expr(ObRawExpr *expr) { return when_exprs_.push_back(expr); }
      int add_then_op_expr(ObRawExpr *expr) { return then_exprs_.push_back(expr); }
      int32_t get_when_expr_size() const { return when_exprs_.size(); }
      int32_t get_then_expr_size() const { return then_exprs_.size(); }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;


      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        arg_expr_->get_raw_var(exprs);
        for(int32_t i = 0; i < when_exprs_.size(); ++i)
        {
          when_exprs_.at(i)->get_raw_var(exprs);
        }
        for(int32_t i = 0; i < then_exprs_.size(); ++i)
        {
          then_exprs_.at(i)->get_raw_var(exprs);
        }
        default_expr_->get_raw_var(exprs);
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);

    private:
      ObRawExpr *arg_expr_;
      oceanbase::common::ObVector<ObRawExpr*> when_exprs_;
      oceanbase::common::ObVector<ObRawExpr*> then_exprs_;
      ObRawExpr *default_expr_;
    };

    class ObAggFunRawExpr : public ObRawExpr
    {
    public:
      ObAggFunRawExpr()
      {
        param_expr_ = NULL;
        distinct_ = false;
      }
      ObAggFunRawExpr(ObRawExpr *param_expr, bool is_distinct, ObItemType expr_type = T_INVALID)
          : ObRawExpr(expr_type), param_expr_(param_expr), distinct_(is_distinct)
      {
      }
      virtual ~ObAggFunRawExpr() {}
      ObRawExpr* get_param_expr() const { return param_expr_; }
      void set_param_expr(ObRawExpr *expr) { param_expr_ = expr; }
      bool is_param_distinct() const { return distinct_; }
      void set_param_distinct() { distinct_ = true; }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;


      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        param_expr_->get_raw_var(exprs);
      }
      //add zt:e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);

    private:
      // NULL means '*'
      ObRawExpr* param_expr_;
      bool     distinct_;
    };

    class ObSysFunRawExpr : public ObRawExpr
    {
    public:
      ObSysFunRawExpr() {}
      virtual ~ObSysFunRawExpr() {}
      ObRawExpr* get_param_expr(int32_t index) const
      {
        ObRawExpr* expr = NULL;
        if (index >= 0 || index < exprs_.size())
          expr = exprs_[index];
        return expr;
      }
      int add_param_expr(ObRawExpr *expr) { return exprs_.push_back(expr); }
      void set_func_name(const common::ObString& name) { func_name_ = name; }
      const common::ObString& get_func_name() { return func_name_; }
      int32_t get_param_size() const { return exprs_.size(); }
      virtual int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL) const;
      void print(FILE* fp, int32_t level) const;

      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      virtual void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
        for(int32_t i = 0; i < exprs_.size(); ++i)
        {
          exprs_.at(i)->get_raw_var(exprs);
        }
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      virtual int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          common::ObBitSet<> &bit_set,
          std::map<uint64_t, uint64_t> table_id_hashmap,
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);
      
    private:
      common::ObString func_name_;
      common::ObVector<ObRawExpr*> exprs_;
    };

    class ObSqlRawExpr
    {
    public:
      ObSqlRawExpr();
      ObSqlRawExpr(
          uint64_t expr_id,
          uint64_t table_id = oceanbase::common::OB_INVALID_ID,
          uint64_t column_id = oceanbase::common::OB_INVALID_ID,
          ObRawExpr* expr = NULL);
      virtual ~ObSqlRawExpr() {}
      uint64_t get_expr_id() const { return expr_id_; }
      uint64_t get_column_id() const { return column_id_; }
      uint64_t get_table_id() const { return table_id_; }
      void set_expr_id(uint64_t expr_id) { expr_id_ = expr_id; }
      void set_column_id(uint64_t column_id) { column_id_ = column_id; }
      void set_table_id(uint64_t table_id) { table_id_ = table_id; }
      void set_expr(ObRawExpr* expr) { expr_ = expr; }
      void set_tables_set(const common::ObBitSet<> tables_set) { tables_set_ = tables_set; }
      void set_applied(bool is_applied) { is_apply_ = is_applied; }
      void set_contain_aggr(bool contain_aggr) { contain_aggr_ = contain_aggr; }
      void set_contain_alias(bool contain_alias) { contain_alias_ = contain_alias; }
      void set_columnlized(bool is_columnlized) { is_columnlized_ = is_columnlized; }
      bool is_apply() const { return is_apply_; }
      bool is_contain_aggr() const { return contain_aggr_; }
      bool is_contain_alias() const { return contain_alias_; }
      bool is_columnlized() const { return is_columnlized_; }
      ObRawExpr* get_expr() const { return expr_; }
      const common::ObBitSet<>& get_tables_set() const { return tables_set_; }
      common::ObBitSet<>& get_tables_set() { return tables_set_; }
      const common::ObObjType get_result_type() const { return expr_->get_result_type(); }
      int fill_sql_expression(
          ObSqlExpression& inter_expr,
          ObTransformer *transformer = NULL,
          ObLogicalPlan *logical_plan = NULL,
          ObPhysicalPlan *physical_plan = NULL);
      void print(FILE* fp, int32_t level, int32_t index = 0) const;

      //add zt: 20151104 b
      /**
       * @brief get_raw_var
       * push expression to stack
       * @param exprs expression stack
       */
      void get_raw_var(ObIArray<const ObRawExpr *> &exprs) const
      {
          expr_->get_raw_var(exprs);
      }
      //add zt: 20151104 e
      
      // add by lxb on 2017/02/15 for logical optimizer
      int optimize_sql_expression(
          ObSelectStmt *&main_stmt,
          std::map<uint64_t, uint64_t> table_id_hashmap, 
          std::map<uint64_t, uint64_t> column_id_hashmap,
          uint64_t table_id,
          uint64_t &real_table_id,
          std::map<uint64_t, uint64_t> alias_table_hashmap,
          int type);

    private:
      uint64_t  expr_id_;
      uint64_t  table_id_;
      uint64_t  column_id_;
      bool      is_apply_;
      bool      contain_aggr_;
      bool      contain_alias_;
      bool      is_columnlized_;
      common::ObBitSet<>  tables_set_;
      ObRawExpr*  expr_;
    };

  }

}

#endif //OCEANBASE_SQL_RAWEXPR_H_
