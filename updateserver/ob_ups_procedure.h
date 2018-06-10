/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_procedure.h
 * @brief ups SpUpsInstExecStrategy and ObUpsProcedure class definition
 *
 * create by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

#ifndef OBUPSPROCEDURE_H
#define OBUPSPROCEDURE_H
#include "sql/ob_sp_procedure.h"
#include "ob_session_mgr.h"
#include "ob_ups_procedure_special_executor.h"
using namespace oceanbase::sql;

namespace oceanbase
{
  namespace updateserver
  {
    class SpUpsInstExecStrategy;
    typedef int (*UpsInstHandler)(SpUpsInstExecStrategy *host, SpInst *inst);
    /**
     * @brief The SpUpsInstExecStrategy class
     * instruction execution strategy in ups
     */
    class SpUpsInstExecStrategy
    {
      public:
        /**
         * @brief SpUpsInstExecStrategy constructor
         */
        SpUpsInstExecStrategy();
        /**
         * @brief execute_inst
         * execution instruction
         * @param inst instruction
         * @return error code
         */
        int execute_inst(SpInst *inst)
        {
          return inst_handler[inst->get_type()](this, inst);
        }
        /**
         * @brief execute_block
         * execution group instructions
         * @param inst instructions
         * @return error code
         */
        int execute_block(SpGroupInsts *inst) ;
        /**
         * @brief hkey
         * get hkey
         * @param sdata_id static data
         * @return hkey
         */
        int64_t hkey(int64_t sdata_id) const;

      private:
        /**
         * @brief execute_expr
         * execute expression instruction
         * @param inst instruction
         * @return error code
         */
        int execute_expr(SpExprInst *inst) ;
        /**
         * @brief execute_rd_base
         * exexute read baseline data instruction
         * in fact ,it prohibited
         * @param inst instruction
         * @return error code
         */
        int execute_rd_base(SpRdBaseInst *inst)  { UNUSED(inst); return OB_ERROR; }
        /**
         * @brief execute_rw_delta
         * executie read read and weite delta data instruction
         * @param inst instruction
         * @return error code
         */
        int execute_rw_delta(SpRwDeltaInst *inst) ;
        /**
         * @brief execute_rw_delta_into_var
         * executie read read and weite delta data into variable instruction
         * @param inst instruction
         * @return error code
         */
        int execute_rw_delta_into_var(SpRwDeltaIntoVarInst *inst) ;
        /**
         * @brief execute_rw_comp
         * prohibited
         * @param inst
         * @return
         */
        int execute_rw_comp(SpRwCompInst *inst) { UNUSED(inst); return OB_ERROR; }
        /**
         * @brief execute_if_ctrl
         * executie if contriol instruction
         * @param inst instruction
         * @return error code
         */
        int execute_if_ctrl(SpIfCtrlInsts *inst);
        //      int execute_loop(SpLoopInst *inst) { UNUSED(inst); return OB_ERROR; }
        /**
         * @brief execute_casewhen
         * executie case when instruction
         * @param inst instruction
         * @return error code
         */
        int execute_casewhen(SpCaseInst *inst);
        /**
         * @brief execute_multi_inst
         * executie multi instruction
         * @param mul_inst multi instruction
         * @return
         */
        int execute_multi_inst(SpMultiInsts *mul_inst);
        /**
         * @brief execute_while
         * executie while instruction
         * if we try to support while on the ups, make sure no dead loop is involved
         * @param inst instruction
         * @return error code
         */
        int execute_while(SpWhileInst *inst);
        /**
         * @brief execute_loop
         * executie while instruction
         * if we try to support loop-exit on the ups, make sure no dead loop is involved
         * @param inst instruction
         * @return error code
         */
        int execute_loop(SpLoopInst *inst);
      private:
        /**
         * @brief pexecute_expr
         * static function appoint a SpUpsInstExecStrategy object to execute expression instruction
         * @param host SpUpsInstExecStrategy object pointer
         * @param inst expression instruction
         * @return error code
         */
        static int pexecute_expr(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_rw_delta
         * static function appoint a SpUpsInstExecStrategy object to execute read read and weite delta data instruction
         * @param host SpUpsInstExecStrategy object pointer
         * @param inst read read and weite delta data instruction
         * @return error code
         */
        static int pexecute_rw_delta(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_rw_delta_into_var
         * static function appoint a SpUpsInstExecStrategy object to execute read read and weite delta data into variable instruction
         * @param host SpUpsInstExecStrategy object pointer
         * @param inst read read and weite delta data into variable instruction
         * @return error code
         */
        static int pexecute_rw_delta_into_var(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_if_ctrl
         * static function appoint a SpUpsInstExecStrategy object to execute read if contriol instruction
         * @param host SpUpsInstExecStrategy object pointer
         * @param inst if contriol instruction
         * @return error code
         */
        static int pexecute_if_ctrl(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_loop
         * static function appoint a SpUpsInstExecStrategy object to execute loop instruction
         * @param host SpUpsInstExecStrategy object pointer
         * @param inst loop instruction
         * @return error code
         */
        static int pexecute_loop(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_block
         * static function appoint a SpUpsInstExecStrategy object to execute group instruction
         * @param host host SpUpsInstExecStrategy object pointer
         * @param inst group instruction
         * @return error code
         */
        static int pexecute_block(SpUpsInstExecStrategy *host, SpInst *inst);
        /**
         * @brief pexecute_casewhen
         * static function appoint a SpUpsInstExecStrategy object to execute case when instruction
         * @param host host SpUpsInstExecStrategy object pointer
         * @param inst case when instruction
         * @return error code
         */
        static int pexecute_casewhen(SpUpsInstExecStrategy *host, SpInst *inst);
      private:
        UpsInstHandler inst_handler[SP_UNKOWN];   ///<  instruction handler
        ObLoopCounter loop_counter_;  ///<  loop counter
        ObRow curr_row_;  ///<  current row
    };
    /**
     * @brief The ObUpsProcedure class
     * ups procedure physical operator
     */
    class ObUpsProcedure : public sql::SpProcedure
    {
      public:
        /**
         * @brief ObUpsProcedure constructor
         * @param session_ctx session context
         */
        ObUpsProcedure(BaseSessionCtx& session_ctx);
        /**
         * @brief destructor
         */
        virtual ~ObUpsProcedure();
        /**
         * @brief reset
         * clear object
         */
        virtual void reset();
        /**
         * @brief reuse
         * clear object
         */
        virtual void reuse();
        /**
         * @brief open
         * open physical plan operator
         * @return error code
         */
        virtual int open();
        /**
         * @brief close
         * close physical plan operator
         * @return
         */
        virtual int close();
        /**
         * @brief hkey
         * get hkey
         * @param sdata_id static data id
         * @return hkey
         */
        virtual int64_t hkey(int64_t sdata_id) const;
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte size
         */
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        /**
         * @brief get_session_ctx
         * get session context
         * @return session context
         */
        BaseSessionCtx * get_session_ctx() { return session_ctx_; }
        /**
         * @brief get_next_row
         * get next row
         * @param row returned ObRow object pointer
         * @return error code
         */
        virtual int get_next_row(const common::ObRow *&row);
        /**
         * @brief get_row_desc
         * get row descriptor
         * @param row_desc returned  row descriptor
         * @return error code
         */
        virtual int get_row_desc(const ObRowDesc *&row_desc) const;

      private:
        //disallow copy
        static const int64_t SMALL_BLOCK_SIZE = 4 * 1024LL;  ///<  small block size
        ObUpsProcedure(const ObUpsProcedure &other);
        /**
         * @brief operator =
         * = operator overload disable
         * @param other  ObUpsProcedure object
         * @return  ObUpsProcedure object
         */
        ObUpsProcedure& operator=(const ObUpsProcedure &other);
        /**
         * @brief make_fake_desc
         * generate row descroptor
         * @return  error code
         */
        int make_fake_desc();
      private:

        SpUpsInstExecStrategy strategy_;  ///<  instruction execution strategy

        VarNameValMap::const_iterator var_iter_;  ///<  variable name value map iterator
        ObRowDesc fake_desc_;  ///< temp row descriptor
        ObRow var_row_;   ///<  ObRow object
        BaseSessionCtx *session_ctx_;  ///<  session context
    };

  }
}
#endif // OBUPSPROCEDURE_H
