/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_drop.h
 * @brief the ObProcedureDrop class definition that warp procedure drop PhyOperator
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_DROP_H
#define OCEANBASE_SQL_OB_PROCEDURE_DROP_H
#include "sql/ob_single_child_phy_operator.h"
#include "ob_sql_session_info.h"
#include "common/dlist.h"

//add by wangdonghui 20160225 [drop procedure]:b
#include "sql/ob_sql_context.h"
//add :e

namespace oceanbase
{
	namespace sql
	{
		class ObPhysicalPlan;
    /**
     * @brief The ObProcedureDrop class
     * define PhyOperator of drop procedure
     */
		class ObProcedureDrop : public ObSingleChildPhyOperator
		{
      public:
        /**
         * @brief constructor
         */
        ObProcedureDrop();
        /**
         * @brief destructor
         */
        virtual ~ObProcedureDrop();
        //add by wangdonghui 20160225 [drop procedure init] :b
        /**
         * @brief set_rpc_stub
         * set rpc procxy
         * @param rpc ms rpc procxy
         */
        void set_rpc_stub(mergeserver::ObMergerRootRpcProxy* rpc);
        //add :e
        /**
         * @brief reset
         * clear physical operator
         */
        virtual void reset();
        /**
         * @brief reuse
         * clear physical operator
         */
        virtual void reuse();
        /**
         * @brief open
         * important function open physical operator
         * @return error code
         */
        virtual int open();
        /**
         * @brief close
         * close physical operator
         * @return error code
         */
        virtual int close();
        /**
         * @brief get_type
         * get physical operator type
         * @return physical operator type
         */
        virtual ObPhyOperatorType get_type() const
        {
          return PHY_PROCEDURE_DROP;
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
         * get row describor
         * @param row_desc row describor
         * @return error code
         */
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        /**
         * @brief get_next_row
         * get next row
         * @param row ObRow object pointer
         * @return error code
         */
        virtual int get_next_row(const common::ObRow *&row);
        /**
         * @brief set_proc_name
         * set procedure name
         * @param proc_name procedure name
         * @return
         */
        int set_proc_name(ObString &proc_name);
        /**
         * @brief set_if_exists
         * set if exists flag
         * @param flag bool value
         */
        void set_if_exists(bool flag);
        /**
         * @brief if_exists
         * get if exists flag
         * @return if exists flag
         */
        bool if_exists();
      private:
        //disallow copy
        /**
         * @brief copy constructor  disable
         * @param other ObProcedureDrop object
         */
        ObProcedureDrop(const ObProcedureDrop &other);
        /**
         * @brief operator =
         * = operator overload disable
         * @param other ObProcedureDrop object
         * @return ObProcedureDrop object
         */
        ObProcedureDrop& operator=(const ObProcedureDrop &other);
        //function members

      private:
        ObString proc_name_;  ///<  procedure name
        bool  if_exists_;  ///<   if exists flag
        //add by wangdonghui 20160225 [drop procedure] :b
        mergeserver::ObMergerRootRpcProxy* rpc_;  ///<  ms rpc proxy
        //add :e

		};

    //add by wangdonghui [drop procedure] 20160225 :b
    inline void ObProcedureDrop::set_rpc_stub(mergeserver::ObMergerRootRpcProxy* rpc)
    {
      rpc_ = rpc;
    }
    //add :e
	}
}

#endif



