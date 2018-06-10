/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_execute_stmt.h
 * @brief the ObProcedureExecuteStmt class definition that warp procedure execute statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_EXECUTE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_EXECUTE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObProcedureExecuteStmt class
     * procedure execute statement class definition
     */
    class ObProcedureExecuteStmt: public ObBasicStmt
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureExecuteStmt() :
            ObBasicStmt(T_PROCEDURE_EXEC)
        {
          proc_stmt_id_=common::OB_INVALID_ID;
        }
        /**
         * @brief destructor
         */
        virtual ~ObProcedureExecuteStmt()
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
         * @brief set_proc_stmt_id
         * set procedure statement id
         * @param proc_stmt_id  procedure statement id
         * @return error code
         */
        int set_proc_stmt_id(uint64_t proc_stmt_id);
        /**
         * @brief get_proc_name
         * get procedure name
         * @return procedure name
         */
        const ObString& get_proc_name() const;/*获取存储过程名*/
        /**
         * @brief get_proc_stmt_id
         * get procedure statement id
         * @return procedure statement id
         */
        uint64_t get_proc_stmt_id() const;/*获取存储过程对应语句id*/
        /**
         * @brief print
         * print procedure execute statement information
         * @param fp
         * @param level
         * @param index
         */
        virtual void print(FILE* fp, int32_t level, int32_t index);

        //    int add_variable_name(const ObString& name);

        //    const ObString& get_variable_name(int64_t index) const;

        //    int64_t get_variable_size() const;
        /**
         * @brief add_param_expr
         * add a paramer expression id
         * @param expr_id paramer expression id
         * @return
         */
        int add_param_expr(uint64_t expr_id);
        /**
         * @brief get_param_expr
         * get paramer expression id by array index
         * @param index array index
         * @return paramer expression id
         */
        uint64_t get_param_expr(int64_t index) const;
        /**
         * @brief get_param_size
         * get paramer array size
         * @return paramer array number
         */
        int64_t get_param_size() const;
        /**
         * @brief set_no_group
         * set no group execution
         * @param no_group  no group flag
         * @return error code
         */
        int set_no_group(bool no_group);//add by wdh 20160718
        /**
         * @brief get_no_group
         * get no group execution flag
         * @return bool value
         */
        bool get_no_group();
        //add by qx 20170317 :b
        /**
         * @brief set_long_trans
         * set long transcation flag
         * @param long_trans
         * @return
         */
        int set_long_trans(bool long_trans);
        /**
         * @brief get_long_trans
         * get long transcation flag
         * @return
         */
        bool get_long_trans();
        //add :e
      private:
        ObString proc_name_;  ///<  procedure name
        //		common::ObArray<common::ObString> variable_names_;
        common::ObArray<uint64_t> param_list_;   ///<  paramer list
        /// need to query data generated from the table when the stored procedure to generate a program corresponding to the stored procedure statement ID
        uint64_t proc_stmt_id_;
        bool no_group_;   ///<  no group execution flag
        //add by qx 20170317 :b
        bool long_trans_;   ///< long transcation flag
        //add :e
    };

  }
}

#endif
