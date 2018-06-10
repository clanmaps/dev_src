/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_create_stmt.h
 * @brief the ObProcedureCreateStmt class definition that warp procedure create statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#ifndef OCEANBASE_SQL_OB_PROCEDURE_CREATE_STMT_H_
#define OCEANBASE_SQL_OB_PROCEDURE_CREATE_STMT_H_
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/ob_array.h"
#include "ob_basic_stmt.h"
#include "parse_node.h"
#include <map>
using namespace oceanbase::common;

namespace oceanbase {
namespace sql {
/**
 * @brief The ObProcedureCreateStmt class
 *
 */
class ObProcedureCreateStmt: public ObBasicStmt {
	public:
    /**
     * @brief constructor
     */
    ObProcedureCreateStmt() :
				ObBasicStmt(T_PROCEDURE_CREATE) {
		proc_id_=common::OB_INVALID_ID;
    //proc_insert_id_=common::OB_INVALID_ID;
		}
    /**
     * @brief destructor
     */
		virtual ~ObProcedureCreateStmt() {
		}
    /**
     * @brief set_proc_name
     * set procedure name
     * @param proc_name procedure name
     * @return error code
     */
		int set_proc_name(ObString &proc_name);
    /**
     * @brief get_proc_name
     * get procedure name
     * @return procedure name
     */
		ObString& get_proc_name();

    //add by wangdonghui :b
    /**
     * @brief set_proc_source_code
     * set procedure source code
     * @param proc_source_code procedure source code
     * @return error code
     */
    int set_proc_source_code(ObString &proc_source_code);
    /**
     * @brief get_proc_source_code
     * get procedure source code
     * @return procedure source code
     */
    ObString& get_proc_source_code();
    //add :e
    /**
     * @brief set_proc_id
     * set procedure id
     * @param stmt_id  create statement id
     * @return error code
     */
		int set_proc_id(uint64_t& stmt_id);
    /**
     * @brief get_proc_id
     * get procedure id
     * @return procedure id
     */
		uint64_t& get_proc_id();


    /**
     * @brief print
     * print create statement info
     * @param fp
     * @param level
     * @param index
     */
		virtual void print(FILE* fp, int32_t level, int32_t index);


	private:
    ObString proc_name_;  ///<  procedure name
    //add by wangdonghui 20160121 :b
    ObString proc_source_code_;  ///<  procedure code
    //add :e
    uint64_t proc_id_;  ///<procedure id
    //uint64_t proc_insert_id_; /*插入到存储过程数据表的语句id*/

	};


}
}

#endif
