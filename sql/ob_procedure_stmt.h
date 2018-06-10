/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_stmt.h
 * @brief the ObProcedureStmt class definition that warp procedure statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "common/hash/ob_hashmap.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {

    enum ParamType
    {
      DEFAULT_TYPE=0,
      IN_TYPE = 1,
      OUT_TYPE = 2,
      INOUT_TYPE = 3
    };  ///<  paramer type
    /**
     * @brief The ObParamDef struct
     * procedure paramer data structure
     */
    struct ObParamDef
    {
      ObString    param_name_;/*参数名称*/
      ObObjType   param_type_;/*参数类型*/
      ObObj       default_value_;/*默认值*/
      bool				is_array;
      ParamType	out_type_;/*输出类型*/
      ObObj       out_var_;
    };

    /**
     * @brief The ObProcedureStmt class
     * procedure statement class definition
     */
    class ObProcedureStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureStmt() :
                ObBasicStmt(T_PROCEDURE)
        {
          cursor_hash_map_.create(hash::cal_next_prime(512));
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureStmt()
        {
        }
        /**
         * @brief set_proc_name
         * set procedure name
         * @param proc_name procedure name
         * @return error code
         */
        int set_proc_name(const ObString &proc_name);
        /**
         * @brief add_proc_param
         * add a procedure paramer
         * @param proc_param  procedure paramer
         * @return error code
         */
        int add_proc_param(const ObParamDef &proc_param);
        /**
         * @brief add_declare_var
         * add a procedure declare variable
         * @param var declare variable
         * @return error code
         */
        int add_declare_var(const ObString &var);

        //add by wdh 20160705 :b
        /**
         * @brief delete_var
         * pop a vaiable
         */
        void delete_var();
        /**
         * @brief set_flag
         * set declare legal flag
         * @param flag bool value
         * @return error code
         */
        int set_flag(const bool &flag);
        /**
         * @brief get_flag
         * get declare legal flag
         * @return flag
         */
        bool get_flag() const;
        //add :e
        /**
         * @brief add_stmt
         * add a procedure statement id
         * @param stmt_id procedure statement id
         * @return error code
         */
        int add_stmt(uint64_t& stmt_id);
        /**
         * @brief get_stmt
         * get procedure block statement id by array index
         * @param index array index
         * @return statement id
         */
        uint64_t get_stmt(int64_t index) const;
        /**
         * @brief get_proc_name
         * get procedure name
         * @return procedure name
         */
        const ObString& get_proc_name() const;
        /**
         * @brief get_declare_var
         * get declare variable by array index
         * @param index array index
         * @return declare variable name
         */
        const ObString& get_declare_var(int64_t index) const;
        /**
         * @brief get_param_size
         * get paramer array size
         * @return paramer array size
         */
        int64_t get_param_size() const;
        /**
         * @brief get_stmt_size
         * get procedure block statement array size
         * @return array size
         */
        int64_t get_stmt_size() const;
        /**
         * @brief get_declare_var_size
         * get declare variable array size
         * @return declare variable array size
         */
        int64_t get_declare_var_size() const;
        /**
         * @brief check_var_exist
         * check varibale name exit
         * @param var vairble name
         * @return bool value
         */
        bool check_var_exist(const ObString &var) const;

        /**
         * @brief get_param
         * get paramer by array index
         * @param index array index
         * @return procedure paramer object
         */
        const ObParamDef &get_param(int64_t index) const;
        /**
         * @brief print
         * print procedure statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);


      private:
        ObString proc_name_;  ///<  proceduere name
        ObArray<uint64_t> proc_block_;  ///<  procedure block statement id array
        ObArray<ObParamDef> params_;   ///<  procedure paramer array
        ObArray<ObString> declare_variable_;   ///<  procedure block inner declare variable name array

        //add by wdh 20160705 :b
        bool is_declare_legal;  ///<  declare legal flag
        //add :e
        hash::ObHashMap<ObString,uint64_t,hash::NoPthreadDefendMode> cursor_hash_map_;  ///<  cursor hash map
    };
  }
}

#endif
