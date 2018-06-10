/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_create.h
 * @brief the ObProcedureCreate class is a create procedure PhyOperator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_26
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_CREATE_H
#define OCEANBASE_SQL_OB_PROCEDURE_CREATE_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"

//add by wangdonghui 20160119
#include "sql/ob_sql_context.h"
//add :e

namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
        /**
         * @brief The ObProcedureCreate class
         * ObProcedureCreate designed for physical plan of create
         * it has one child operator that inherit child_op_ from father class
         * operator is ObProcedure type
         */
		class ObProcedureCreate : public ObSingleChildPhyOperator
		{
		public:
            /**
             * @brief constructor
             */
			ObProcedureCreate();
            /**
             * @brief destructor
             */
			virtual ~ObProcedureCreate();
            //add by wangdonghui 20160120 init :b
            /**
             * @brief set_sql_context
             * set sql context
             * @param context
             */
            void set_sql_context(const ObSqlContext &context);
            //add :e
            /**
             * @brief reset
             * clear object content
             */
			virtual void reset();
            /**
             * @brief reuse
             * clear object content
             */
			virtual void reuse();
            /**
             * @brief open
             * important function,open PhyOperator, begin to excute action
             * @return error code
             */
			virtual int open();
            /**
             * @brief close
             * close PhyOperator
             * @return error code
             */
			virtual int close();
            /**
             * @brief get_type
             * get current PhyOperator type
             * @return
             */
			virtual ObPhyOperatorType get_type() const
			{
				return PHY_PROCEDURE_CREATE;
			}
            /**
             * @brief to_string
             * @param buf buffer area
             * @param buf_len  buffer area length
             * @return byte number
             */
			virtual int64_t to_string(char* buf, const int64_t buf_len) const;
            /**
             * @brief get_row_desc
             * get row descriptor
             * @param row_desc  ObRowDesc object pointer
             * @return error code
             */
			virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
            /**
             * @brief get_next_row
             * get next row content
             * @param row ObRow object pointer
             * @return
             */
			virtual int get_next_row(const common::ObRow *&row);
            /**
             * @brief set_proc_name
             * set procedure name
             * @param proc_name procedure name
             * @return error code
             */
			int set_proc_name(ObString &proc_name);/*设置存储过程名*/
            //add by wangdonghui 20160121 :b
            /**
             * @brief set_proc_source_code
             * set procedure source code
             * @param source_code source code
             * @return error code
             */
            int set_proc_source_code(ObString &source_code);
            //add :e

            //delete by wangdonghui 20160128 :b
            //int set_insert_op(ObPhyOperator &insert_op);
            //delete :e

		private:
			//disallow copy
            /**
             * @brief ObProcedureCreate
             * copy constructor,it be disable because be set private function
             * @param other ObProcedureCreate object
             */
			ObProcedureCreate(const ObProcedureCreate &other);
            /**
             * @brief operator =
             * = operator overload
             * same to disable
             * @param other ObProcedureCreate object
             * @return ObProcedureCreate object
             */
			ObProcedureCreate& operator=(const ObProcedureCreate &other);
			//function members

		private:
            ObString proc_name_;  ///< procedure name
            //add by wangdonghui :b
            ObString proc_source_code_; ///< procedure source code
            //add :e

            //delete by wangdonghui we dont need this operation :b
            //ObPhyOperator *insert_op_;
            //delete :e

        //add by wangdonghui 20160119 data members :b
        private:
            bool if_not_exists_;  ///< if exists flag
            ObSqlContext local_context_;  ///< sql context
		};
        //add :e
	}
}

#endif



