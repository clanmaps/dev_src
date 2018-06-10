/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_phy_operator_type.h
 * @brief all physical operator type
 *
 * modified by longfei：add some operators for secondary index
 * modified by maoxiaoxiao:add physical operator "index trigger","bloomfilter join"
 * modified by Qiushi FAN: insert a new operator type : OB_LEFT_SEMI_JOIN.
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author Qiushi FAN <qsfan@ecnu.cn>
 *
 * @date 2016_07_27
 */

/*
 * (C) 2010-2012 Alibaba Group Holding Limited.
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_phy_operator_type.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *
 */

#ifndef _OB_PHY_OPERATOR_TYPE_H
#define _OB_PHY_OPERATOR_TYPE_H 1

namespace oceanbase
{
  namespace sql
  {
    enum ObPhyOperatorType
    {
      PHY_INVALID               = 0,
      PHY_PROJECT               = 1,
      PHY_LIMIT                 = 2,
      PHY_FILTER                = 3,
      PHY_TABLET_SCAN           = 4,
      PHY_TABLE_RPC_SCAN        = 5,
      PHY_TABLE_MEM_SCAN        = 6,
      PHY_RENAME                = 7,
      PHY_TABLE_RENAME          = 8,
      PHY_SORT                  = 9,
      PHY_MEM_SSTABLE_SCAN      = 10,
      PHY_LOCK_FILTER           = 11,
      PHY_INC_SCAN              = 12,
      PHY_UPS_MODIFY            = 13,
      PHY_INSERT_DB_SEM_FILTER  = 14,
      PHY_MULTIPLE_SCAN_MERGE   = 15,
      PHY_MULTIPLE_GET_MERGE    = 16,
      PHY_VALUES                = 17,
      PHY_EMPTY_ROW_FILTER      = 18,
      PHY_EXPR_VALUES           = 19,
      PHY_ROW_COUNT             = 20,
      PHY_WHEN_FILTER           = 21,
      PHY_CUR_TIME              = 22,
      PHY_UPS_EXECUTOR,         /*20*/
      PHY_TABLET_DIRECT_JOIN,
      PHY_MERGE_JOIN,
      PHY_MERGE_EXCEPT,
      PHY_MERGE_INTERSECT,
      PHY_MERGE_UNION,
      PHY_ALTER_SYS_CNF,
      PHY_ALTER_TABLE,
      PHY_CREATE_TABLE,
      PHY_DEALLOCATE,
      PHY_DROP_TABLE,           /*30*/
      PHY_DUAL_TABLE_SCAN,
      PHY_END_TRANS,
      PHY_PRIV_EXECUTOR,
      PHY_START_TRANS,
      PHY_VARIABLE_SET,
      PHY_TABLET_GET,
      PHY_SSTABLE_GET,
      PHY_SSTABLE_SCAN,
      PHY_UPS_MULTI_GET,
      PHY_UPS_SCAN,             /*40*/
      PHY_RPC_SCAN,
      PHY_DELETE,
      PHY_EXECUTE,
      PHY_EXPLAIN,
      PHY_HASH_GROUP_BY,
      PHY_MERGE_GROUP_BY,
      PHY_INSERT,
      PHY_MERGE_DISTINCT,
      PHY_PREPARE,
      PHY_SCALAR_AGGREGATE,     /*50*/
      PHY_UPDATE,
      PHY_TABLET_GET_FUSE,
      PHY_TABLET_SCAN_FUSE,
      PHY_ROW_ITER_ADAPTOR,
      PHY_INC_GET_ITER,
      PHY_KILL_SESSION,
      PHY_OB_CHANGE_OBI,
      PHY_ADD_PROJECT,
      PHY_UPS_MODIFY_WITH_DML_TYPE,
      //add fanqiushi [semi_join] [0.1] 20150829:b
      PHY_SEMI_LEFT_JOIN,
      //add:e
      /*add maoxx [bloomfilter_join] 20160408*/
      PHY_BLOOMFILTER_JOIN,
      /*add e*/
      //add lbzhong [auto_increment] 20161218:b
      PHY_AUTO_INCREMENT_FILTER,
      //add:e
      PHY_HASH_JOIN_SINGLE,//add maoxx [hash join single] 20170110
      PHY_INDEX_TRIGGER, //add maoxx
      PHY_DROP_INDEX, //add longfei [secondary index drop index]
      PHY_INDEX_LOCAL_AGENT, //add longfei [cons static index] 151202
      PHY_INDEX_INTERACTIVE_AGENT, //add longfei [cons static index] 151204
      PHY_FILL_VALUES, //huangjianwei [update more] fix prepare 180120

      PHY_CURSOR_DECLARE,
      PHY_CURSOR_FETCH,
      PHY_CURSOR_FETCH_INTO,
      PHY_CURSOR_FETCH_PRIOR,
      PHY_CURSOR_FETCH_PRIOR_INTO,
      PHY_CURSOR_FETCH_FIRST,
      PHY_CURSOR_FETCH_FIRST_INTO,
      PHY_CURSOR_FETCH_LAST,
      PHY_CURSOR_FETCH_LAST_INTO,
      PHY_CURSOR_FETCH_RELATIVE,
      PHY_CURSOR_FETCH_RELATIVE_INTO,
      PHY_CURSOR_FETCH_ABSOLUTE,
      PHY_CURSOR_FETCH_ABS_INTO,
      PHY_CURSOR_FETCH_FROMTO,
      PHY_CURSOR_OPEN,
      PHY_CURSOR_CLOSE,
      PHY_CURSOR,
      //add:e
      //add by zhujun:b
      PHY_PROCEDURE,
      PHY_PROCEDURE_CREATE,
      PHY_PROCEDURE_DROP,
      PHY_PROCEDURE_DECLARE,
      PHY_PROCEDURE_ASSGIN,
      PHY_PROCEDURE_EXEC,
      PHY_PROCEDURE_IF,
      PHY_PROCEDURE_CASE,
      PHY_PROCEDURE_CASE_WHEN,
      PHY_PROCEDURE_ELSEIF,
      PHY_PROCEDURE_WHILE,
      PHY_PROCEDURE_ELSE,
      PHY_PROCEDURE_SELECT_INTO,
      PHY_TRUNCATE_TABLE, //add hxlong [Truncate Table]:20170403
      PHY_VARIABLE_SET_ARRAY,
      //add:e
      PHY_UPS_LOCK_TABLE, //add wangjiahao [table lock] 20160616	  
      PHY_END, /* end of phy operator type */


    };

    void ob_print_phy_operator_stat();
    const char* ob_phy_operator_type_str(ObPhyOperatorType type);
    void ob_inc_phy_operator_stat(ObPhyOperatorType type);
    void ob_dec_phy_operator_stat(ObPhyOperatorType type);
  }
}

#define OB_PHY_OP_INC(type) oceanbase::sql::ob_inc_phy_operator_stat((oceanbase::sql::ObPhyOperatorType)type)
#define OB_PHY_OP_DEC(type) oceanbase::sql::ob_dec_phy_operator_stat((oceanbase::sql::ObPhyOperatorType)type)
#endif /* _OB_PHY_OPERATOR_TYPE_H */
