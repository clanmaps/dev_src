/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_drop_index_stmt.h
 * @brief for logical plan of drop index
 *
 * Created by longfei：for resolving logical plan of drop index
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_10_26
 */

#ifndef SQL_OB_DROP_INDEX_STMT_H_
#define SQL_OB_DROP_INDEX_STMT_H_

#include "common/ob_array.h"
#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "sql/ob_basic_stmt.h"
#include "parse_node.h"
#include "ob_drop_table_stmt.h"
namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObDropIndexStmt class
     * ObDropIndexStmt is designed for drop index's logical plan
     */
    class ObDropIndexStmt : public ObDropTableStmt
    {
    public:
      /**
       * @brief ObDropIndexStmt: constructor
       * @param name_pool_
       */
      ObDropIndexStmt(common::ObStringBuf* name_pool_):ObDropTableStmt(name_pool_),drp_all_(false)
      {
        set_stmt_type(T_DROP_INDEX);
      }
      ~ObDropIndexStmt(){}
      /**
       * @brief generate_inner_index_table_name
       * @param idx_name
       * @param ori_tab_name
       * @param inner_idx_name [out]
       * @param len
       * @return succ or fail
       */
      int generate_inner_index_table_name(
          common::ObString& idx_name,
          common::ObString& ori_tab_name,
          char* inner_idx_name,
          int64_t& len);
      /**
       * @brief get original table name
       * @return
       */
      const common::ObString& getOriTabName() const;
      /**
       * @brief set original table name
       * @param oriTabName
       */
      void setOriTabName(const common::ObString& oriTabName);
      /**
       * @brief is drop all index tables?
       * @return
       */
      bool isDrpAll() const;
      /**
       * @brief set drop all index table
       * @param drpAll
       */
      void setDrpAll(bool drpAll);

    private:
      common::ObString ori_tab_name_; ///< original table's name
      bool drp_all_; ///< flag of drop all index
    };
  }
}



#endif /* SQL_OB_DROP_INDEX_STMT_H_ */
