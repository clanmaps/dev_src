/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_execute.h
 * @brief the ObProcedureExecute class definition that wrap procedure execute physical operator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_EXECUTE_H
#define OCEANBASE_SQL_OB_PROCEDURE_EXECUTE_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"
#include "ob_procedure_execute_stmt.h"
#include "build_plan.h"

namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
    /**
     * @brief The ObProcedureExecute class
     * procedure execute physical operator class definition
     */
		class ObProcedureExecute : public ObSingleChildPhyOperator
		{
      public:
        /**
         * @brief constructor
         */
        ObProcedureExecute();
        /**
         * @brief destructor
         */
        virtual ~ObProcedureExecute();
        /**
         * @brief reset
         * clear ObProcedureExecute object
         */
        virtual void reset();
        /**
         * @brief reuse
         * clear ObProcedureExecute object
         */
        virtual void reuse();
        /**
         * @brief open
         * open ObProcedureExecute operator
         * @return error code
         */
        virtual int open();
        /**
         * @brief close
         * close ObProcedureExecute operator
         * @return
         */
        virtual int close();
        /**
         * @brief get_type
         * get ObProcedureExecute operator type
         * @return procedure physical operator type
         */
        virtual ObPhyOperatorType get_type() const
        {
          return PHY_PROCEDURE_EXEC;
        }
        /**
         * @brief to_string
         * @param buf buffer
         * @param buf_len buffer length
         * @return buffer byte number
         */
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        /**
         * @brief get_row_desc
         * get row descriptor
         * @param row_desc row descriptor pointer
         * @return error code
         */
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        /**
         * @brief get_next_row
         * get next row
         * @param row return ObRow object
         * @return error code
         */
        virtual int get_next_row(const common::ObRow *&row);
        /**
         * @brief set_proc_name
         * set procedure name
         * @param proc_name procedure name
         * @return error code
         */
        int set_proc_name(const ObString &proc_name); //proc name
        /**
         * @brief set_stmt_id
         * set cached ObProcedure plan id
         * @param stmt_id cached ObProcedure plan id
         * @return error code
         */
        int set_stmt_id(uint64_t stmt_id);			//the cached ObProcedure plan id
        /**
         * @brief set_no_group
         * set no group execution flag
         * @param no_group no group execution flag
         * @return error code
         */
        int set_no_group(bool no_group);//add by wdh 20160718
        /**
         * @brief get_no_group
         * get no group execution flag
         * @return bool value
         */
        bool get_no_group();
        /**
         * @brief add_param_expr
         * add a paramer expression
         * @param expr  paramer expression
         * @return error code
         */
        int add_param_expr(ObSqlExpression& expr);
        /**
         * @brief get_expr
         * get paramer expression by list index
         * @param idx list index
         * @return paramer expression
         */
        ObSqlExpression & get_expr(int64_t idx) { return param_list_.at(idx); }
        /**
         * @brief get_param_size
         * get paramer list size
         * @return paramer list number
         */
        int64_t get_param_size() const;
        //add by qx 20170317 :b
        /**
         * @brief get_long_trans
         * get long transcation flag
         * @return bool value
         */
        bool get_long_trans();
        /**
         * @brief set_long_trans
         * set long transcation flag
         * @param long_trans flag
         */
        int set_long_trans(bool long_trans);
        //add :e

      private:
        //disallow copy
        /**
         * @brief copy constructor disable
         * @param other ObProcedureExecute object
         */
        ObProcedureExecute(const ObProcedureExecute &other);
        /**
         * @brief operator =
         * = operator overload disable
         * @param other ObProcedureExecute object
         * @return ObProcedureExecute object
         */
        ObProcedureExecute& operator=(const ObProcedureExecute &other);
        //function members

      private:
        //data members
        ObString proc_name_;  ///<  procedure name
        common::ObArray<ObSqlExpression> param_list_;  ///<  paramer list
        uint64_t stmt_id_;  ///< procedure statement physical plan id
        //add by wdh 20160718
        bool no_group_;   ///< no group flag
        //add by qx 20170317  :b
        bool long_trans_;   ///< long transcation flag
        //add :e
		};
	}
}

#endif



