/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_sp_procedure.h
 * @brief procedure instruction and operator physical plan realation class definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */


#ifndef SPPROCEDURE_H
#define SPPROCEDURE_H

#include "ob_no_children_phy_operator.h"
#include "ob_procedure_stmt.h"
#include "ob_procedure_assgin_stmt.h"
#include "ob_procedure_declare_stmt.h"
#include "ob_physical_plan.h"
#include "ob_raw_expr.h"
#include "ob_procedure_static_data_mgr.h"
#include "ob_sql_session_info.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    class SpInstExecStrategy;
    class SpMsInstExecStrategy;
    class SpProcedure;
    class ObProcedureOptimizer;

    enum SpInstType
    {
      SP_E_INST = 0,  ///<  expr instruction
      SP_C_INST,  ///<  if control instruction
      SP_L_INST,  ///<  loop instruction
      SP_CW_INST, ///<  case_when instruction
      SP_B_INST,  ///<  read baseline data
      SP_D_INST,  ///<  maintain delta data
      SP_DE_INST,  ///<  maintain delta data, read into variables
      SP_A_INST,  ///<  analyse inst, read, baseline & delta, aggreation, analyze
      SP_PREGROUP_INST,  ///<  used to fetch static data
      SP_GROUP_INST,  ///<  for a block of instructions
      SP_W_INST,  ///<  while instruction
      SP_EXIT_INST,  ///<  exit instruction
      SP_SQL_INST,  ///<  sql interface for all
      SP_UNKOWN   ///<  unknown instruction type
    };

    enum CallType
    {
      L_LPC = 0,   ///<  locl call
      T_RPC = 1,   ///<  ups call
      S_RPC = 2,   ///<  ms call
      T_AND_S  = 3,  ///<  ups and ms call
    };
    /**
     * @brief merge_call_type
     * merge call type
     * @param a  first call type
     * @param b  second call type
     * @return  call type
     */
    inline CallType merge_call_type(CallType a, CallType b)
    {
      return (CallType)(a | b);
    }

    /**
     * @brief The SpVar struct
     * SpVariables,
     * be careful of the usage of SpVar,
     * remember call clear to totally desconstruct the object,
     * the ~SpVar doesnot free the memory used by idx_value_
     *
     * @remind
     * later I will refactor the SpVar structure. Here we use ObSqlExpression to
     * represent the index value of an array variable. Each time, we need to caculate
     * the expr to get the index value. Later, I will force the index value to a constant
     * such as: int (a raw index) or string (a temp variable name).
     *
     * @remind
     * refactor SpVar finished
     */
    /**
     * @brief The SpVar struct
     * instruct variable
     */
    struct SpVar
    {
      ObString var_name_;  ///< instruct variable name
      ObObj idx_value_;   ///<  instruct variable value for array type

      /**
       * @brief SpVar constructor
       */
      SpVar() { idx_value_.set_null();}

      /**
       * @brief SpVar destructor
       */
      ~SpVar();
      /**
       * @brief is_array
       * judge instruct variable whether is array
       * @return bool value
       */
      bool is_array() const { return !idx_value_.is_null(); }
      /**
       * @brief deserialize
       * deserialize instruct variable
       * @param buf buffer
       * @param data_len  data length
       * @param pos location flag
       * @return error code
       */
      int deserialize(const char *buf, int64_t data_len, int64_t &pos);
      /**
       * @brief serialize
       * serialize instruct variable
       * @param buf buffer
       * @param data_len  data length
       * @param pos location flag
       * @return error code
       */
      int serialize(char *buf, int64_t buf_len, int64_t &pos) const;
      /**
       * @brief to_string
       * @param buf buffer
       * @param buf_len buffer length
       * @return byte number
       */
      int64_t to_string(char *buf, const int64_t buf_len) const;

      //comment, donot forget the set the ownner op of the idx_value_
      /**
       * @brief assign
       * assign SpVar object to self
       * @param other SpVar object
       * @return error code
       */
      int assign(const SpVar &other);
    };

    /**
     * @brief SpVarType
     *    Used to represent the usage of a variable
     *    I'm not sure how many ways the array could be used.
     *    Thus, leave it as a typedef to expand in future
     */
    enum SpVarType
    {
      VM_TMP_VAR,   ///<  temp variable
      VM_FUL_ARY,   ///<  array variable
      VM_DB_TAB     ///<  databas table variable
    };

    /**
     * @brief The SpVarInfo struct
     * Reprenset a variable used in an instruction
     */
    struct SpVarInfo
    {
      /**
       * @brief construction
       */
      SpVarInfo() {}
      /**
       * @brief SpVarInfo construction
       * @param table_id used table id
       */
      SpVarInfo(uint64_t table_id) :
          var_type_(VM_DB_TAB)  { idx_value_.set_int((int64_t)table_id);}
      /**
       * @brief SpVarInfo construction
       * @param name variable name
       */
      SpVarInfo(const ObString &name) :
          var_name_(name), var_type_(VM_TMP_VAR) {}
      /**
       * @brief SpVarInfo construction
       * @param name  variable name
       * @param idx variable index
       */
      SpVarInfo(const ObString &name, const ObObj &idx) :
          var_name_(name), idx_value_(idx), var_type_(VM_FUL_ARY) {}
      /**
       * @brief conflict
       * judge between variable a and  variable b whether is conflict(same)
       * @param a variable a
       * @param b variable b
       * @return judge result bool value
       */
      static bool conflict(const SpVarInfo &a, const SpVarInfo &b);
      /**
       * @brief compare
       * compare other variable whether is same to this
       * @param other other variable
       * @return compare reslut
       */
      bool compare(const SpVarInfo &other) const;
      /**
       * @brief to_string
       * @param buf buffer
       * @param buf_len buffer length
       * @return buffer byte number
       */
      int64_t to_string(char *buf, const int64_t buf_len) const;

      ObString var_name_;   ///<  variable name
      ObObj    idx_value_;  ///<  index value
      SpVarType var_type_;   ///<  instruction variable type
    };

    /**
     * @brief The SpVariableSet struct
     * instruction variable set
     */
    class SpVariableSet
    {
      public:
        const static int VAR_PER_INST = 8;  ///<  every instruction variable max number
        typedef ObSEArray<SpVarInfo, VAR_PER_INST> SpVarArray;  ///<  instruction variable array
        /**
         * @brief SpVariableSet constructor
         */
        SpVariableSet() {}
        /**
         * @brief add_tmp_var
         * add a temp variable
         * @param var_name variable name
         * @return error code
         */
        int add_tmp_var(const ObString &var_name);
        /**
         * @brief add_tmp_var
         * add temp variable set
         * @param var_set variable set
         * @return error code
         */
        int add_tmp_var(const ObIArray<ObString>& var_set);
        /**
         * @brief add_array_var
         * add an array element variable
         * @param arr_name arry name
         * @param idx_value array element index value
         * @return error code
         */
        int add_array_var(const ObString &arr_name, const ObObj &idx_value);
        /**
         * @brief add_var
         * add an instruction variable
         * @param var instruction variable
         * @return error code
         */
        int add_var(const SpVar &var);
        /**
         * @brief add_var_info_set
         * add variable by instruction variable set
         * @param var_set instruction variable set
         * @return error code
         */
        int add_var_info_set(const SpVariableSet &var_set);
        /**
         * @brief add_var_info
         * add a variable information
         * @param var_info variable information
         * @return error code
         */
        int add_var_info(const SpVarInfo &var_info);
        /**
         * @brief add_table_id
         * add table id
         * @param table_id table id
         * @return error code
         */
        int add_table_id(const uint64_t table_id);
        /**
         * @brief count
         * get variable set size
         * @return variable set size
         */
        int64_t count() const { return var_info_set_.count(); }
        /**
         * @brief get_var_info
         * get variable information by set index
         * @param idx set index
         * @return variable information
         */
        const SpVarInfo & get_var_info(int64_t idx) const { return var_info_set_.at(idx); }
        /**
         * @brief exist
         * judge variable whether is exist
         * @param var_name  variable name
         * @return  judgement result bool value
         */
        bool exist(const ObString &var_name) const;
        /**
         * @brief remove
         * remove variable information by set index
         * @param idx set index
         */
        void remove(int64_t idx) { var_info_set_.remove(idx); }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte count
         */
        int64_t to_string(char *buf, int64_t buf_len) const;
        /**
         * @brief conflict
         * judge two sets  whether conflict
         * @param in_set  first set
         * @param out_set second set
         * @return judgement result
         */
        static int conflict(const SpVariableSet &in_set, const SpVariableSet &out_set);
      private:
        SpVarArray var_info_set_;  ///<  instruction variable set
    };

    enum InstDep  ///<  dependence direction between two instructions
    {
      Da_True_Dep = 1,  ///<  data true dependence
      Da_Anti_Dep = 2,  ///<  data anti dependecne
      Da_Out_Dep  = 4,  ///<  data output dependece
      Tr_Itm_Dep  = 8   ///<  transaction item dependence
    };


    /**
     * @brief The SpInst class
     * SpInst should be a simple wrapper, do not own big memory area which
     * should belongs to the physical plan or the result set
     */
    class SpInst
    {
      public:
        /**
         * @brief SpInst constructor
         * @param type instruction type
         */
        SpInst(SpInstType type) : type_(type), proc_(NULL) {}
        /**
         * @brief destructor
         */
        virtual ~SpInst();
        /**
         * @brief get_read_variable_set
         * pure virtual function
         * get read variable set
         * @param read_set return read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const = 0; //bad design ret type as ref ... try to correct later
        /**
         * @brief get_write_variable_set
         * pure virtual function  get write variable set
         * @param write_set return write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const = 0;
        /**
         * @brief get_call_type
         * pure virtual function
         * get call type
         * @return call type
         */
        virtual CallType get_call_type() const  = 0;
        /**
         * @brief check_dep
         * @param inst_in
         * @param inst_out
         * @return
         */
        static int check_dep(SpInst &inst_in, SpInst &inst_out);
        /**
         * @brief set_owner_procedure
         * set procedure own the instruction
         * @param proc procedure
         */
        void set_owner_procedure(SpProcedure *proc) { proc_ = proc;}
        /**
         * @brief get_ownner
         * get procedure
         * @return SpProcedure object pointer
         */
        SpProcedure *get_ownner() const { return proc_; }
        /**
         * @brief get_type
         * get instructin type
         * @return instructin type
         */
        SpInstType get_type() const { return type_; }
        /**
         * @brief is_srpc
         * whether is ms rpc call
         * @return bool value
         */
        bool is_srpc() const { return get_call_type() == S_RPC || get_call_type() == T_AND_S; }
        /**
         * @brief is_trpc
         * whether is ups rpc call
         * @return bool value
         */
        bool is_trpc() const { return get_call_type() == T_RPC; }
        /**
         * @brief get_id
         * get instruction id
         * @return instruction id
         */
        int64_t get_id() const { return id_; }
        /**
         * @brief set_id
         * set instruction id
         * @param id instruction id
         */
        void set_id(int64_t id) { id_ = id; }
        /**
         * @brief in_group_exec
         * whether is instruction group execution
         * @return bool value
         */
        virtual bool in_group_exec() const { return false;}
        /**
         * @brief set_in_group_exec
         * set instruction group execution
         */
        virtual void set_in_group_exec() {}
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                                     ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const {UNUSED(buf); UNUSED(buf_len); return 0;}
        /**
         * @brief assign
         * not support instruction assgin operation
         * @param inst
         * @return
         */
        virtual int assign(const SpInst *inst) { UNUSED(inst); return OB_NOT_SUPPORTED;}
      protected:
        SpInstType type_;  ///<  instruction type
        int64_t id_;   ///<  instruction id
        SpProcedure *proc_;  ///<  the procedure thats owns this instruction
    };

    const static int SP_INST_LIST_SIZE = 16;  ///<  instruction list max size
    typedef ObSEArray<SpInst *, SP_INST_LIST_SIZE> SpInstList;  ///<  typedef instruction list

    /**
     * @brief The SpMultiInsts class
     * consists of multi-instructions, used when define instructions block,
     * such as if-then block, if-else block, loop-body-block
     */
    class SpMultiInsts
    {
    public:
      friend class ObProcedureOptimizer;  ///<  friend class
      /**
       * @brief SpMultiInsts constructor
       */
      SpMultiInsts() : ownner_(NULL) {}
      /**
       * @brief SpMultiInsts constructor
       * @param ownner instruction  own this object
       */
      SpMultiInsts(SpInst *ownner) : ownner_(ownner) {}
      /**
       * @brief destructor
       */
      virtual ~SpMultiInsts();
      /**
       * @brief add_inst
       * add an instruction to list
       * @param inst new instruction
       * @return error code
       */
      int add_inst(SpInst *inst) { return inst_list_.push_back(inst); }
      /**
       * @brief get_inst
       * get instruction by list index
       * @param idx  list index
       * @param inst returned instruction
       * @return error code
       */
      int get_inst(int64_t idx, SpInst *&inst) const;
      /**
       * @brief get_inst
       * get instruction by list index
       * @param idx list index
       * @return  instruction
       */
      SpInst* get_inst(int64_t idx);
      /**
       * @brief inst_count
       * get instruction list count
       * @return instruction list count
       */
      int64_t inst_count() const { return inst_list_.count(); }
      /**
       * @brief get_read_variable_set
       * get read variable set
       * @param read_set read variable set
       */
      void get_read_variable_set(SpVariableSet &read_set) const;
      /**
       * @brief get_write_variable_set
       * get write variable set
       * @param write_set write variable set
       */
      void get_write_variable_set(SpVariableSet &write_set) const;
      /**
       * @brief get_call_type
       * get call type
       * @return  call type
       */
      CallType get_call_type() const;
      /**
       * @brief deserialize_inst
       * deserialize instruction
       * @param buf
       * @param data_len
       * @param pos
       * @param allocator
       * @param operators_store
       * @param op_factory
       * @return error code
       */
      virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                                   ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
      /**
       * @brief serialize_inst
       * serialize instruction
       * @param buf
       * @param buf_len
       * @param pos
       * @return
       */
      virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
      /**
       * @brief optimize
       * pop read baseline data to execution list
       * @param exec_list returned execution list
       * @return error code
       */
      int optimize(SpInstList &exec_list);
      /**
       * @brief set_in_group_exec
       * set group execute
       */
      void set_in_group_exec();
      //add by wdh check whether have exit stmt 20160707
      /**
       * @brief check_exit
       * check whether have exit stmt
       * @return
       */
      bool check_exit();
      /**
       * @brief dfs
       * called by check_exit()
       * @param flag returned flag
       */
      void dfs(bool &flag);
      /**
       * @brief check_tnode_access
       * @return
       */
      int64_t check_tnode_access() const;
      /**
       * @brief clear
       * clear instruction list
       */
      void clear();
      /**
       * @brief to_string
       * @param buf buffer
       * @param buf_len buffer length
       * @return buffer byte number
       */
      int64_t to_string(char *buf, const int64_t buf_len) const;
      /**
       * @brief assign
       * SpMultiInsts object assign operation
       * @param mul_inst SpMultiInsts object
       * @return error code
       */
      int assign(const SpMultiInsts &mul_inst);
    protected:
      SpInstList inst_list_;  ///<  instruction list
      SpInst *ownner_;   ///<  SpMultiInsts object  ownner
    };

    /**
     * @brief The SpExprInst class
     * Represent assign instructions, left_variable = right_values
     */
    class SpExprInst : public SpInst
    {
      public:
        /**
         * @brief SpExprInst  construction
         */
        SpExprInst() : SpInst(SP_E_INST) {}
        /**
         * @brief destructor
         */
        virtual ~SpExprInst();
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_val
         * get right value
         * @return  right value
         */
        ObSqlExpression& get_val() { return right_val_; }
        /**
         * @brief get_var
         * get left variable
         * @return  left variable
         */
        SpVar & get_var() { return left_var_; }
        /**
         * @brief cons_read_var_set
         * get variable set
         * @return variable set
         */
        SpVariableSet & cons_read_var_set() { return rs_; }
        /**
         * @brief get_call_type
         * get call type
         * @return  call type
         */
        CallType get_call_type() const { return L_LPC; }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      private:
        SpVar left_var_;   ///<  left variable
        ObSqlExpression right_val_;  ///<  right value
        SpVariableSet rs_;  ///<  variable set
    };
    /**
     * @brief The SpRdBaseInst class
     * read baseline data instruction
     */
    class SpRdBaseInst :public SpInst
    {
      public:
        /**
         * @brief consturctor
         */
        SpRdBaseInst() : SpInst(SP_B_INST), op_(NULL), table_id_(0), for_group_exec_(false), rw_inst_id_(-1) {}
        /**
         * @brief destructor
         */
        virtual ~SpRdBaseInst();
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief cons_read_var_set
         * get variable set
         * @return variable set
         */
        SpVariableSet & cons_read_var_set() { return rs_; }
        /**
         * @brief set_rdbase_op
         * set read baseline data physical operator
         * @param op  physical operator
         * @param query_id query id
         * @return error code
         */
        int set_rdbase_op(ObPhyOperator *op, int32_t query_id);
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return S_RPC; }
        /**
         * @brief get_rd_op
         * get read baseline data physical operator
         * @return read baseline data physical operator
         */
        ObPhyOperator* get_rd_op() { return op_;}
        /**
         * @brief get_query_id
         * get physical operator query id
         * @return query id
         */
        int32_t get_query_id() const {return query_id_; }
        /**
         * @brief set_tid
         * set table id
         * @param tid table id
         */
        void set_tid(uint64_t tid) { table_id_ = tid; }
        /**
         * @brief set_rw_id
         * set read and write instruction id
         * @param id instruction id
         */
        void set_rw_id(int64_t id) { rw_inst_id_ = id; }
        /**
         * @brief get_rw_id
         * get read and write instruction id
         * @return read and write instruction id
         */
        int64_t get_rw_id() const { return rw_inst_id_; }
        /**
         * @brief is_for_group_exec
         * whether is group execution
         * @return bool value
         */
        bool is_for_group_exec() const { return for_group_exec_; }
        /**
         * @brief set_exec_mode
         * set execute mode
         */
        void set_exec_mode();
        /**
         * @brief get_sdata_id
         * get static data id
         * @return static data id
         */
        int64_t get_sdata_id() const { return sdata_id_; }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      protected:
        ObPhyOperator *op_;  ///< read baseline data physiacl operator
        int32_t query_id_;  ///<  corresponded to the op_

        SpVariableSet rs_;  ///<  the row key variable
        uint64_t table_id_;  ///< table id

        int64_t sdata_id_;   ///<  static data id
        bool for_group_exec_;   ///<  group execution flag

        //Never use in execution phase. Only meaningful in compilation phase.
        int64_t rw_inst_id_;  ///<  read and write instruction id
    };
    /**
     * @brief The SpRwDeltaInst class
     * read and write Delta data instruction
     */
    class SpRwDeltaInst : public SpInst
    {
      public:
        /**
         * @brief SpRwDeltaInst constructor
         * @param type instruction type
         */
        SpRwDeltaInst(SpInstType type = SP_D_INST) : SpInst(type), op_(NULL), ups_exec_op_(NULL), table_id_(0), group_exec_(false) {}
        /**
         * @brief destructor
         */
        virtual ~SpRwDeltaInst();
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief cons_read_var_set
         * get variable set
         * @return variable set
         */
        SpVariableSet & cons_read_var_set() { return rs_; }
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return T_RPC; }
        /**
         * @brief set_rwdelta_op
         * set read and write delta data physiacl operator
         * @param op physiacl operator
         * @return error code
         */
        int set_rwdelta_op(ObPhyOperator *op);
        /**
         * @brief set_ups_exec_op
         * set ups execution physiacl operator
         * @param op physiacl operator
         * @param query_id query id
         * @return error code
         */
        int set_ups_exec_op(ObPhyOperator *op, int32_t query_id);
        /**
         * @brief get_query_id
         * get physical operator query id
         * @return query id
         */
        int32_t get_query_id() const { return query_id_; }
        /**
         * @brief get_rwdelta_op
         * get read and write delta data physiacl operator
         * @return physiacl operator
         */
        ObPhyOperator* get_rwdelta_op() { return op_; }
        /**
         * @brief get_ups_exec_op
         * get ups execution physiacl operator
         * @return physiacl operator
         */
        ObPhyOperator* get_ups_exec_op() { return ups_exec_op_; }
        /**
         * @brief set_tid
         * set table id
         * @param tid table id
         * @return error code
         */
        int set_tid(uint64_t tid) {table_id_ = tid; return OB_SUCCESS;}
        /**
         * @brief in_group_exec
         * get group execution flag
         * @return bool value
         */
        bool in_group_exec() const { return group_exec_; }
        /**
         * @brief set_in_group_exec
         * set group execution flag true
         */
        void set_in_group_exec() { group_exec_ = true; }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      protected:
        ObPhyOperator *op_;  ///<  the main query of the ups execution plan
        ObPhyOperator *ups_exec_op_;  ///<  the ObUpsExecutor wrapper operator
        int32_t query_id_;   ///<  query id
        SpVariableSet rs_;   ///<  variable set
        uint64_t table_id_;  ///<  table id
        bool group_exec_;   ///<  group execution flag
    };
    /**
     * @brief The SpRwDeltaIntoVarInst class
     * read and write delta data into variable instruction
     */
    class SpRwDeltaIntoVarInst : public SpRwDeltaInst
    {
      public:
        /**
         * @brief constructor
         */
        SpRwDeltaIntoVarInst() : SpRwDeltaInst(SP_DE_INST){}
        /**
         * @brief destructor
         */
        virtual ~SpRwDeltaIntoVarInst();
        /**
         * @brief add_assign_var
         * add an assign variable
         * @param var assign variable
         */
        void add_assign_var(const SpVar &var) { var_list_.push_back(var); }
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_var_list
         * get variable list
         * @return variable list
         */
        const ObIArray<SpVar> & get_var_list() const { return var_list_;}

        //for op_ == NULL, it is a range query, otherwise it is a point query
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return (op_ == NULL ? T_AND_S : T_RPC); }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);

      private:
        ObSEArray<SpVar, 16> var_list_;  ///<  variable list
    };
    /**
     * @brief The SpRwCompInst class
     * complex read and write instruction
     */
    class SpRwCompInst : public SpInst
    {
      public:
        /**
         * @brief construtor
         */
        SpRwCompInst() : SpInst(SP_A_INST), op_(NULL), table_id_(0) {}
        /**
         * @brief destructor
         */
        virtual ~SpRwCompInst();
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param returned write_set write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return T_AND_S; }
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet & cons_read_var_set() { return rs_; }
        /**
         * @brief set_rwcomp_op
         * set physical operator
         * @param op physical operator
         * @param query_id instructor query id
         * @return error code
         */
        int set_rwcomp_op(ObPhyOperator *op, int32_t query_id) { op_ = op; query_id_ = query_id; return OB_SUCCESS; }
        /**
         * @brief get_rwcomp_op
         * get physical operator
         * @return physical operator
         */
        ObPhyOperator * get_rwcomp_op() { return op_; }
        /**
         * @brief get_query_id
         * get physical operator query id
         * @return query id
         */
        int32_t get_query_id() const { return query_id_; }
        /**
         * @brief get_var_list
         * get variable list
         * @return variable list
         */
        const ObIArray<SpVar> & get_var_list() const { return var_list_;}
        /**
         * @brief add_assign_var
         * get assign variable
         * @param var returned assign variable
         */
        void add_assign_var(const SpVar &var) { var_list_.push_back(var); }
        /**
         * @brief set_tid
         * @param tid
         * @return error code
         */
        int set_tid(uint64_t tid) {table_id_ = tid; return OB_SUCCESS;}
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      private:
        ObPhyOperator *op_;  ///<  physiacl opeartor that need to execution instruction
        int32_t query_id_;  ///<  instruction query id
        uint64_t table_id_;  ///<  table id

        ObSEArray<SpVar, 16> var_list_;  ///<  variable list

        SpVariableSet rs_;  ///< read veriable set
    };
    /**
     * @brief The SpPlainSQLInst class
     * plain sql instruction
     */
    class SpPlainSQLInst : public SpInst
    {
      public:
        /**
         * @brief constructor
         */
        SpPlainSQLInst() : SpInst(SP_SQL_INST), op_(NULL) {}
        /**
         * @brief destructor
         */
        virtual ~SpPlainSQLInst();
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return T_AND_S; } //never try to optimize this kind of SQL
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet &cons_read_var_set() { return rs_; }
        /**
         * @brief set_main_query
         * set physical operator and query id
         * @param op physical operator
         * @param query_id query id
         */
        void set_main_query(ObPhyOperator *op, int32_t query_id) { op_ = op; query_id_ = query_id; }
        /**
         * @brief get_main_query
         * @return physical operator
         */
        ObPhyOperator * get_main_query() { return op_; }
        /**
         * @brief set_tid
         * @param tid
         * @return error code
         */
        int set_tid(uint64_t tid) {table_id_ = tid; return OB_SUCCESS;}
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
      private:
        ObPhyOperator *op_;  ///<  main query operator
        int32_t query_id_;  ///<  query id
        uint64_t table_id_;  ///<  table id
        SpVariableSet rs_;  ///<  read set
    };
    /**
     * @brief The SpPreGroupInsts class
     * prepare group execution instruction
     */
    class SpPreGroupInsts : public SpInst
    {
      public:
        /**
           * @brief constructor
           */
          SpPreGroupInsts() : SpInst(SP_PREGROUP_INST) {}
          /**
           * @brief destructor
           */
          virtual ~SpPreGroupInsts() {}
          /**
           * @brief get_read_variable_set
           * get read variable set
           * @param read_set returned  read variable set
           */
          virtual void get_read_variable_set(SpVariableSet &read_set) const;
          /**
           * @brief get_write_variable_set
           * get write variable set
           * @param write_set returned write variable set
           */
          virtual void get_write_variable_set(SpVariableSet &write_set) const;
          /**
           * @brief get_call_type
           * get call type
           * @return call type
           */
          CallType get_call_type() const { return S_RPC; }
          /**
           * @brief get_body
           * get instruction lsit
           * @return instruction lsit
           */
          SpMultiInsts *get_body() { return &inst_list_; }
          /**
           * @brief add_inst
           * add an instruction
           * @param inst
           * @return
           */
          int add_inst(SpInst *inst);
          /**
           * @brief get_write_set
           * get write variable set
           * @return
           */
          const SpVariableSet & get_write_set() const { return write_set_; }
          /**
           * @brief assign
           * instruction assign
           * @param inst other instruction
           * @return error code
           */
          int assign(const SpInst *inst) { UNUSED(inst); return OB_SUCCESS; }
          /**
           * @brief to_string
           * @param buf buffer
           * @param buf_len buffer length
           * @return buffer byte number
           */
          virtual int64_t to_string(char *buf, const int64_t buf_len) const;
      private:
          SpMultiInsts inst_list_;   ///<  instruction list
          SpVariableSet write_set_;  ///<  write list
    };

    /**
     * @brief The SpInstBlock class
     * a list of each instruction, which would be sent to ups for further execution
     */
    class SpGroupInsts : public SpInst
    {
      public:
          /**
         * @brief constructor
         */
        SpGroupInsts() : SpInst(SP_GROUP_INST) {}
        /**
         * @brief destructor
         */
        virtual ~SpGroupInsts();
        /**
         * @brief get_name
         * get group execution procedure name
         * @return group execution procedure name
         */
        const ObString & get_name() const { return group_proc_name_; }
        /**
         * @brief set_name
         * set group execution procedure name
         * @param name group execution procedure name
         */
        void set_name(ObString &name) { group_proc_name_ = name; }
        /**
         * @brief set_name
         * set group execution procedure name
         * @param alloc allocator
         * @param name group execution procedure name
         */
        void set_name(ModuleArena &alloc, const ObString &name)
        {
          ob_write_string(alloc, name, group_proc_name_);
        }
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const { return T_RPC; }
        /**
         * @brief add_inst
         * add an insruction
         * @param inst insruction
         * @return error code
         */
        int add_inst(SpInst *inst);
        /**
         * @brief push_back stack
         * instruction push
         * @param inst instruction
         * @return error code
         */
        int push_back(SpInst *inst) { return add_inst(inst); }
        ObIArray<SpInst *> & get_inst_list() { return inst_list_;}
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos,
                             common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store,
                             ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      private:
        SpInstList inst_list_;  ///<  insruction list
        SpVariableSet rs_;  ///<  read set
        SpVariableSet ws_;  ///<  write set
        ObString group_proc_name_;  ///<  group execution procedure name
    };
    class SpIfCtrlInsts;  ///<  class declare
    /**
     * @brief The SpIfBlock class
     * if block instruction
     */
    class SpIfBlock : public SpMultiInsts
    {
      public:
        friend class SpIfCtrlInsts;  ///<  friend class
        /**
         * @brief constructor
         * @param ownner instruction owner
         */
        SpIfBlock(SpInst *ownner) : SpMultiInsts(ownner) {}
        /**
         * @brief destructor
         */
        virtual ~SpIfBlock();
        /**
         * @brief optimize
         * optimize as if block
         * @param exec_list execution list
         * @return error code
         */
        int optimize(SpInstList &exec_list);
    };
    /**
     * @brief The SpIfCtrlInsts class
     * if control instruction
     */
    class SpIfCtrlInsts : public SpInst
    {
      public:
        /**
         * @brief constructor
         */
        SpIfCtrlInsts() : SpInst(SP_C_INST), branch_opened_(-1), then_branch_(this), else_branch_(this) {}
        /**
         * @brief destructor
         */
        virtual ~SpIfCtrlInsts();
        /**
         * @brief add_then_inst
         * add an then instruction
         * @param inst instruction
         * @return error code
         */
        int add_then_inst(SpInst *inst);
        /**
         * @brief add_else_inst
         * add an else instruction
         * @param inst instruction
         * @return error code
         */
        int add_else_inst(SpInst *inst);
        /**
         * @brief get_if_expr
         * get if expression
         * @return returned if expression
         */
        ObSqlExpression & get_if_expr() { return if_expr_; }
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet & cons_read_var_set() { return expr_rs_set_; }
        /**
         * @brief get_then_block
         * get then block instruction
         * @return then block instruction
         */
        SpMultiInsts* get_then_block() { return &then_branch_; }
        /**
         * @brief get_else_block
         * get else block instruction
         * @return else block instruction
         */
        SpMultiInsts* get_else_block() { return &else_branch_; }

        //@deprecated
        /**
         * @brief optimize
         * optimize instruction execution sequence
         * @param exec_list returned execution list
         * @return error code
         */
        int optimize(SpInstList &exec_list);
        /**
         * @brief set_open_flag
         * set open flag
         * @param flag  open flag
         */
        void set_open_flag(int flag) { branch_opened_ = flag; }
        /**
         * @brief get_open_flag
         * get open flag
         * @return open flag
         */
        int get_open_flag() const { return branch_opened_; }
        /**
         * @brief set_in_group_exec
         * set insruction group execution
         */
        void set_in_group_exec();
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos,
                             common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store,
                             ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        int assign(const SpInst *inst);
      private:
        ObSqlExpression if_expr_;  ///<  if expression
        SpVariableSet expr_rs_set_;  ///<  exprssion read variable set
        int branch_opened_;  ///<  -1 for not open, 1 for then branch open, 0 for else branch open
        SpIfBlock then_branch_;  ///<  then block instruction
        SpIfBlock else_branch_;  ///<  else block instruction
    };
    /**
     * @brief The SpLoopInst class
     * loop instruction
     */
    class SpLoopInst : public SpInst
    {
      public:
        /**
         * @brief constructor
         */
        SpLoopInst() : SpInst(SP_L_INST), step_size_(1), loop_body_(this) {}
        /**
         * @brief destructor
         */
        virtual ~SpLoopInst();
        /**
         * @brief get_lowest_expr
         * get lowest expression
         * @return lowest expression
         */
        ObSqlExpression & get_lowest_expr() { return lowest_expr_; }
        /**
         * @brief get_highest_expr
         * get hightest expression
         * @return hightest expression
         */
        ObSqlExpression & get_highest_expr() { return highest_expr_; }

        //add wdh 20160324 :b
        /**
         * @brief set_lowest_expr
         * set lowest expression
         * @param lowest_expr lowest expression
         */
        void set_lowest_expr(ObSqlExpression lowest_expr) { lowest_expr_=lowest_expr; }
        /**
         * @brief set_highest_expr
         * set hightest expression
         * @param highest_expr hightest expression
         */
        void set_highest_expr(ObSqlExpression highest_expr) { highest_expr_=highest_expr; }
        //add :e
        /**
         * @brief get_range_var_set
         * get range of variable set
         * @return range of variable set
         */
        SpVariableSet & get_range_var_set() { return range_var_set_; }
        /**
         * @brief set_step_size
         * set step length
         * @param step step length
         */
        void set_step_size(int64_t step) { step_size_ = step; }
        /**
         * @brief set_loop_var
         * set loop variable
         * @param var loop variable
         */
        void set_loop_var(const SpVar &var) { loop_counter_var_ = var; }
        /**
         * @brief set_reverse
         * set reverse flag
         * @param rev bool value
         */
        void set_reverse(bool rev) { reverse_ = rev; }
        /**
         * @brief get_reverse
         * get reverse flag
         * @return reverse flag
         */
        bool get_reverse() const { return reverse_; }
        /**
         * @brief get_body_block
         * get loop body block instructions
         * @return instructions
         */
        SpMultiInsts* get_body_block() { return &loop_body_; }
        /**
         * @brief get_loop_var
         * get loop variable
         * @return loop variable
         */
        const SpVar & get_loop_var() const { return loop_counter_var_; }
        /**
         * @brief optimize
         * optimize instruction execution
         * @param exec_list returned execution list
         * @return error code
         */
        int optimize(SpInstList &exec_list);
        /**
         * @brief is_simple_loop
         * judge whether is simple loop
         * @return bool value
         */
        bool is_simple_loop() const;
        /**
         * @brief check_dead_loop
         * check whether is dead loop
         * @param begin loop begin count
         * @param end loop end count
         * @param rever reverse flag
         * @return bool value
         */
        static bool check_dead_loop(int64_t begin, int64_t end, bool rever);
        /**
         * @brief set_in_group_exec
         * set insruction group execution
         */
        void set_in_group_exec();
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const;
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos,
                                     ModuleArena &allocator,
                                     ObPhysicalPlan::OperatorStore &operators_store,
                                     ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;

        int serialize_loop_template(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        virtual int assign(const SpInst *inst);
        /**
         * @brief assign_template
         * instruction assign
         * @param old_inst old loop instruction
         * @return error code
         */
        int assign_template(const SpLoopInst *old_inst);

      private:

      private:
        SpVar loop_counter_var_;       ///<  loop counter var
        ObSqlExpression lowest_expr_;  ///<  lowest value
        ObSqlExpression highest_expr_; ///<  highest value

        int64_t step_size_;						 ///<  step size
        SpMultiInsts loop_body_;       ///<  loop body

        SpVariableSet range_var_set_;  ///<  variable range set
        bool reverse_;                 ///<  reverse flag , this variable could be elimated
    };

    //add hjw 20151229:b
    /**
     * @brief The SpWhileInst class
     * while instruction
     */
    class SpWhileInst : public SpInst
    {
       public:
          /**
           * @brief construction
           */
          SpWhileInst():SpInst(SP_W_INST), do_body_(this){}
          /**
           * @brief destruction
           */
          virtual ~SpWhileInst();
          /**
           * @brief get_while_expr
           * get while expression
           * @return  while expression object
           */
          ObSqlExpression& get_while_expr(){return while_expr_;}
          /**
           * @brief get_body_block
           * get while body block instructions
           * @return instructions
           */
          SpMultiInsts* get_body_block() {return &do_body_;}
          /**
           * @brief cons_read_var_set
           * get read vaiable set
           * @return read vaiable set
           */
          SpVariableSet & cons_read_var_set() { return while_expr_var_set_; }
          /**
           * @brief get_read_variable_set
           * get read variable set
           * @param read_set returned  read variable set
           */
          virtual void get_read_variable_set(SpVariableSet &read_set) const;
          /**
           * @brief get_write_variable_set
           * get write variable set
           * @param write_set returned write variable set
           */
          virtual void get_write_variable_set(SpVariableSet &write_set) const;
          /**
           * @brief get_call_type
           * get call type
           * @return call type
           */
          virtual CallType get_call_type() const;
          /**
           * @brief deserialize_inst
           * deserialize instruction
           * @param buf
           * @param data_len
           * @param pos
           * @param allocator
           * @param operators_store
           * @param op_factory
           * @return error code
           */
          virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos,ModuleArena &allocator,ObPhysicalPlan::OperatorStore &operators_store,ObPhyOperatorFactory *op_factory);
          /**
           * @brief serialize_inst
           * serialize instruction
           * @param buf
           * @param buf_len
           * @param pos
           * @return error code
           */
          virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
          /**
           * @brief to_string
           * @param buf buffer
           * @param buf_len buffer length
           * @return buffer byte number
           */
          virtual int64_t to_string(char *buf, const int64_t buf_len) const;
          /**
           * @brief assign
           * instruction assign
           * @param inst other instruction
           * @return error code
           */
          virtual int assign(const SpInst *inst);

       private:
          ObSqlExpression while_expr_;  ///<  while value
          SpVariableSet while_expr_var_set_;  ///<  while expression variable set
          SpMultiInsts do_body_;  ///<  do body instructions
    };
    //add hjw 20151229:e

    //add wdh 20160623 :b
    class SpExitInst : public SpInst
    {
      public:
        /**
         * @brief constructor
         */
        SpExitInst():SpInst(SP_EXIT_INST){}
        /**
         * @brief destructor
         */
        virtual ~SpExitInst();
        /**
         * @brief get_when_expr
         * get when expression
         * @return when expression object
         */
        ObSqlExpression& get_when_expr(){return when_expr_;}
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet & cons_read_var_set() { return when_expr_var_set_; }
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        virtual CallType get_call_type() const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos,ModuleArena &allocator,ObPhysicalPlan::OperatorStore &operators_store,ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief check_when
         * check when expression whether is empty
         * @return bool value
         */
        virtual bool check_when()const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        virtual int assign(const SpInst *inst);

     private:
        ObSqlExpression when_expr_;  ///<  when expression , exit value
        SpVariableSet when_expr_var_set_;  ///<  when expression variable set
    };
    //add :e
    class SpCaseInst;  ///< class declare
    /**
     * @brief The SpWhenBlock class
     * when block instruction
     */
    class SpWhenBlock : public SpMultiInsts
    {
      friend class SpCaseInst;
      public:
        /**
         * @brief constructor
         */
        SpWhenBlock() : SpMultiInsts() {}
        /**
         * @brief constructor
         * @param ownner instruction ownner
         */
        SpWhenBlock(SpInst *ownner) : SpMultiInsts(ownner) {}
        /**
         * @brief destructor
         */
        virtual ~SpWhenBlock();
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet & cons_read_var_set() { return when_expr_var_set_; }
        /**
         * @brief get_when_expr
         * get when expression
         * @return when expression object
         */
        ObSqlExpression& get_when_expr(){return when_expr_;}
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief assign
         * instruction assign
         * @param block when instruction block
         * @return returned when instruction block
         */
        int assign(const SpWhenBlock &block);

      private:
        ObSqlExpression when_expr_;   ///<  when expression
        SpVariableSet when_expr_var_set_;  ///<  when expression variable set
    };
    /**
     * @brief The SpCaseInst class
     * case instruction
     */
    class SpCaseInst : public SpInst
    {
      public:
        /**
         * @brief constructor
         */
        SpCaseInst():SpInst(SP_CW_INST), else_branch_(this) {}
        /**
         * @brief destructor
         */
        virtual ~SpCaseInst();
        /**
         * @brief get_case_expr
         * get case expression
         * @return case expression
         */
        ObSqlExpression& get_case_expr(){return case_expr_;}
        /**
         * @brief get_else_block
         * get else block instruction
         * @return else block instruction
         */
        SpMultiInsts* get_else_block(){return &else_branch_;}
        /**
         * @brief cons_when_list
         * get when block instruction list
         * @return when instruction list
         */
        ObIArray<SpWhenBlock>& cons_when_list() {return when_list_;}
        /**
         * @brief get_when_block
         * get when block instruction by array index
         * @param idx array index
         * @return when block instruction
         */
        SpWhenBlock * get_when_block(int64_t idx) { return & (when_list_.at(idx)); }
        /**
         * @brief get_when_count
         * get when list size
         * @return when list size
         */
        int64_t get_when_count() const { return when_list_.count(); }
        /**
         * @brief set_in_group_exec
         * set insruction group execution
         */
        void set_in_group_exec();
        /**
         * @brief cons_read_var_set
         * get read vaiable set
         * @return read vaiable set
         */
        SpVariableSet & cons_read_var_set() { return case_expr_var_set_; }
        /**
         * @brief get_call_type
         * get call type
         * @return call type
         */
        CallType get_call_type() const;
        /**
         * @brief get_read_variable_set
         * get read variable set
         * @param read_set returned  read variable set
         */
        virtual void get_read_variable_set(SpVariableSet &read_set) const;
        /**
         * @brief get_write_variable_set
         * get write variable set
         * @param write_set returned write variable set
         */
        virtual void get_write_variable_set(SpVariableSet &write_set) const;
        /**
         * @brief deserialize_inst
         * deserialize instruction
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @return error code
         */
        virtual int deserialize_inst(const char *buf, int64_t data_len, int64_t &pos, ModuleArena &allocator, ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory);
        /**
         * @brief serialize_inst
         * serialize instruction
         * @param buf
         * @param buf_len
         * @param pos
         * @return error code
         */
        virtual int serialize_inst(char *buf, int64_t buf_len, int64_t &pos) const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char *buf, const int64_t buf_len) const;
        /**
         * @brief assign
         * instruction assign
         * @param inst other instruction
         * @return error code
         */
        virtual int assign(const SpInst *inst);
      private:
        ObSqlExpression case_expr_;  ///<  case expression
        SpVariableSet case_expr_var_set_;  ///<  case expression variable set
        ObSEArray<SpWhenBlock, 8> when_list_;  ///<  when instruction lsit
        SpMultiInsts else_branch_;  ///<  else instruction
    };


    typedef ObSEArray<int64_t, 8> ObLoopCounter;  ///< represent the instruction location, each loop would create one more counter
    /**
     * @brief The SpInstExecStrategy class
     * instruction execution strategy
     */
    class SpInstExecStrategy
    {
      public:
        /**
         * @brief execute_inst
         * to provide the simple routine ,pure virtual function
         * @param inst instruction
         * @return error code
         */
        virtual int execute_inst(SpInst *inst) = 0;
        /**
         * @brief sdata_mgr_hash
         * static data manager throught the use of hash
         * @param sdata_id static data id
         * @param counter loop counter
         * @return hash value
         */
        static int64_t sdata_mgr_hash(int64_t sdata_id, const ObLoopCounter &counter);
    };
    /**
     * @brief The SpProcedure class
     * SpProcedure stores the basic information of the procedure, the distribution of the instruction operations,
     * the original execution order, the maintenance of the set of variables and the baseline data collection,
     * as well as the use of procedures necessary for the object.
     */
    class SpProcedure : public ObNoChildrenPhyOperator
    {
      public:
        /**
         * @brief SpProcedure constructor
         */
        SpProcedure();
        /**
         * @brief destructor
         */
        virtual ~SpProcedure();
        /**
         * @brief get_type
         * get operator type
         * @return operator type
         */
        virtual ObPhyOperatorType get_type() const
        {
          return PHY_PROCEDURE;
        }
        /**
         * @brief set_proc_name
         * set procedure name
         * @param proc_name procedure name
         * @return error code
         */
        int set_proc_name(const ObString &proc_name);
        /**
         * @brief get_proc_name
         * get procedure name
         * @return procedure name
         */
        const ObString & get_proc_name() const { return proc_name_; }
        /**
         * @brief reset
         * clear operator
         */
        virtual void reset();
        /**
         * @brief reuse
         * clear operator
         */
        virtual void reuse() {}
        /**
         * @brief open
         * open physical operator
         * @return error code
         */
        virtual int open() {return OB_SUCCESS;}
        /**
         * @brief close
         * close physical operator
         * @return error code
         */
        virtual int close();
        /**
         * @brief get_row_desc
         * get row descriptor
         * @param row_desc row descriptor
         * @return  error code
         */
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const {UNUSED(row_desc); return OB_SUCCESS;}
        /**
         * @brief get_next_row
         * get next row
         * @param row ObRow object
         * @return error code
         */
        virtual int get_next_row(const common::ObRow *&row) {UNUSED(row); return OB_ITER_END;}
        /**
         * @brief create_variable_table
         * create veriable table
         * @return error code
         */
        int create_variable_table();
        /**
         * @brief write_variable
         * write vaiable value
         * @param var_name vaiable name
         * @param val vaiable value
         * @return error code
         */
        virtual int write_variable(const ObString &var_name, const ObObj & val);
        /**
         * @brief write_variable
         * write array element value
         * @param array_name array name
         * @param idx_value array index
         * @param val array element value
         * @return error code
         */
        virtual int write_variable(const ObString &array_name, int64_t idx_value, const ObObj &val);
        /**
         * @brief write_variable
         * write instruction variable value
         * @param var instruction variable
         * @param val instruction variable value
         * @return error code
         */
        virtual int write_variable(const SpVar &var, const ObObj &val);
        /**
         * @brief read_variable
         * read vaiable value
         * @param var_name  vaiable name
         * @param val returned vaiable value
         * @return error code
         */
        virtual int read_variable(const ObString &var_name, const ObObj *&val) const ;
        /**
         * @brief read_variable
         * read array element value
         * @param array_name array name
         * @param idx_value array index
         * @param val returned array element value
         * @return error code
         */
        virtual int read_variable(const ObString &array_name, int64_t idx_value, const ObObj *&val) const;
        /**
         * @brief read_variable
         * read instruction variable value
         * @param var instruction variable
         * @param val returned instruction variable value
         * @return error code
         */
        virtual int read_variable(const SpVar &var, const ObObj *&val) const;
        /**
         * @brief read_array_size
         * get array size
         * @param array_name array name
         * @param size returned array size
         * @return error code
         */
        virtual int read_array_size(const ObString &array_name, int64_t &size) const;
        /**
         * @brief read_index_value
         * get object element value
         * @param obj object
         * @param idx_val object index
         * @return error code
         */
        virtual int read_index_value(const ObObj &obj, int64_t &idx_val) const;

        //for static data management
        /**
         * @brief store_static_data
         * store static data
         * @param sdata_id static data id
         * @param hkey the hash value of the iteration number of the corresponding cycle
         * @param p_row_store ObRowStore object pointer
         * @return error code
         */
        int store_static_data(int64_t sdata_id, int64_t hkey, ObRowStore *&p_row_store);
        /**
         * @brief get_static_data_by_id
         * get static data by id
         * @param sdata_id static data id
         * @param p_row_store returned ObRowStore object pointer
         * @return error code
         */
        int get_static_data_by_id(int64_t sdata_id, ObRowStore *&p_row_store);
        /**
         * @brief get_static_data
         * get static data
         * @param idx index
         * @param sdata_id static data id
         * @param hkey the hash value of the iteration number of the corresponding cycle
         * @param p_row_store returned ObRowStore object pointer
         * @return error code
         */
        int get_static_data(int64_t idx, int64_t &sdata_id, int64_t &hkey, const ObRowStore *&p_row_store);
        /**
         * @brief get_static_data_count
         * get static data count
         * @return static data count
         */
        int64_t get_static_data_count() const;
        /**
         * @brief hkey
         * get hkey
         * @param sdata_id static data id
         * @return hkey
         */
        virtual int64_t hkey(int64_t sdata_id) const;

        //remove the instruction that does not owned by itself
        //only used when we build a fake procedure object
        /**
         * @brief clear_inst_list
         * clear instruction list
         */
        void clear_inst_list() { inst_list_.clear(); }
        /**
         * @brief clear_var_tab
         * clear variable table
         */
        void clear_var_tab();
        /**
         * @brief clear_variable
         * remove an instruction variable
         * @param var instruction variable
         */
        void clear_variable(const SpVar &var);
        /**
         * @brief create_inst
         * template function,create an instruction
         * @param mul_inst instruction
         * @return created instruction
         */
        template<class T>
        T * create_inst(SpMultiInsts *mul_inst)
        {
          T * ret = NULL;
          void *ptr = arena_.alloc(sizeof(T));
          if( NULL != ptr )
          {
            ret = new(ptr) T();
            //inst_list_.push_back((SpInst *)ret);
            ((SpInst*)ret)->set_owner_procedure(this);
            if( NULL != mul_inst)
              mul_inst->add_inst(ret);
            else
              inst_list_.push_back((SpInst*)ret);

            ((SpInst*)ret)->set_id(inst_store_.count());
            inst_store_.push_back(ret);
          }
          return ret;
        }

        //factory function, create a new instruction type
        /**
         * @brief create_inst
         * create an instruction
         * @param type instruction type
         * @param mul_inst instruction
         * @return  created instruction
         */
        virtual SpInst* create_inst(SpInstType type, SpMultiInsts *mul_inst);
        /**
         * @brief add_inst
         * add an insruction
         * @param inst insruction
         */
        virtual void add_inst(SpInst *inst)
        {
          inst_list_.push_back(inst);
        }
        /**
         * @brief get_inst_by_id
         * get instruction by id
         * @param inst_id instruction id
         * @param inst returned instruction
         * @return error code
         */
        int get_inst_by_id(int64_t inst_id, SpInst *&inst) { return inst_store_.at(inst_id, inst); }
        /**
         * @brief generate_static_data_id
         * generate static data id
         * @return static data id
         */
        int64_t generate_static_data_id() { return ++static_data_id_gen_; }
        /**
         * @brief debug_status
         * debug instruction status
         * @param inst instruction
         * @return error code
         */
        int debug_status(const SpInst *inst) const;
        /**
         * @brief deserialize_tree
         * deserialize tree
         * @param buf
         * @param data_len
         * @param pos
         * @param allocator
         * @param operators_store
         * @param op_factory
         * @param root root physical operator
         * @return error code
         */
        int deserialize_tree(const char *buf, int64_t data_len, int64_t &pos, common::ModuleArena &allocator,
                             ObPhysicalPlan::OperatorStore &operators_store, ObPhyOperatorFactory *op_factory, ObPhyOperator *&root);
        /**
         * @brief serialize_tree
         * serialize tree
         * @param buf
         * @param buf_len
         * @param pos
         * @param root root physical operator
         * @return error code
         */
        int serialize_tree(char *buf, int64_t buf_len, int64_t &pos, const ObPhyOperator &root) const;
        NEED_SERIALIZE_AND_DESERIALIZE;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        /**
         * @brief get_casted_buf
         * get casted buffer
         * @param varchar_buff char type buffer
         * @param buf ObStringBuf type buffer
         */
        void get_casted_buf(char *&varchar_buff, ObStringBuf *&buf) { varchar_buff = casted_buff_, buf = &name_pool_; }

        const static int64_t FAKE_TABLE_ID = -1;  ///<  fake table id  in fact, not exists
        const static int64_t ROW_VAR_CID = 16;   ///<  row variable column id
        const static int64_t ROW_VAL_CID = 17;   ///<   row variable value column id
      private:
        /**
         * @brief SpProcedure copy constructor disable
         * @param other SpProcedure object
         */
        SpProcedure(const SpProcedure &other);
        /**
         * @brief operator =
         * = operator overload disable
         * @param other SpProcedure object
         * @return SpProcedure object
         */
        SpProcedure& operator=(const SpProcedure &other);

      protected:

        ObString proc_name_;  ///<  procedure name

        SpInstList inst_list_;  ///< instruction list
        SpInstList inst_store_;  ///<  stored instruction list

        //  SpInstExecStrategy *exec_strategy_;
        int64_t static_data_id_gen_;  ///<  generated static data id
        int64_t pc_;  ///<  instruction counter

        ModuleArena arena_;  ///<  maybe we can use the ObTransformer's mem_pool_ to allocate the instruction


        static const int64_t SMALL_BLOCK_SIZE = 4 * 1024LL;  ///< a small block size
        /// typedef VarNameValMapAllocer type
        typedef common::ObPooledAllocator<common::hash::HashMapTypes<common::ObString, common::ObObj>::AllocType, common::ObWrapperAllocator> VarNameValMapAllocer;
        typedef common::hash::ObHashMap<common::ObString,
                common::ObObj,
                common::hash::NoPthreadDefendMode,
                common::hash::hash_func<common::ObString>,
                common::hash::equal_to<common::ObString>,
                VarNameValMapAllocer,
                common::hash::NormalPointer,
                common::ObSmallBlockAllocator<>
                > VarNameValMap;  ///<  typedef VarNameValMap type

        common::ObSmallBlockAllocator<> block_allocator_;  ///<  block allocator
        VarNameValMapAllocer var_name_val_map_allocer_;  ///<  name value map allocator
        VarNameValMap var_name_val_map_;  ///< physiacl plan name value map
        common::ObStringBuf name_pool_;  ///<  physiacl plan name pool
        /**
         * @brief The ObProcedureArray struct
         * procedure array data type
         */
        struct ObProcedureArray
        {
          //        ObString array_name_;
          ObSEArray<ObObj, 8> array_values_;  ///< value array
        };

        ObSEArray<ObProcedureArray, 4> array_table_;  ///<  table array

        ObProcedureStaticDataMgr static_data_mgr_;  ///<  static data maneger
        char casted_buff_[OB_MAX_VARCHAR_LENGTH];  ///<  casted buffer
    };
  }
}

#endif // SPPROCEDURE_H
