/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_index_trigger.h
* @brief for operations of index trigger
*
* Created by maoxiaoxiao:modify index table with opertions to data table
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_INDEX_TRIGGER_H
#define OB_INDEX_TRIGGER_H

#include "ob_single_child_phy_operator.h"
#include "ob_sql_expression.h"
#include "common/ob_row_store.h"
#include "ob_project.h"
#include "ob_insert_dbsem_filter.h"
#include "updateserver/ob_sessionctx_factory.h"
#include "updateserver/ob_ups_table_mgr.h"
#include "updateserver/ob_ups_utils.h"

namespace oceanbase
{
  namespace sql
  {
    typedef common::ObSEArray<ObSqlExpression, OB_PREALLOCATED_NUM, common::ModulePageAllocator, ObArrayExpressionCallBack<ObSqlExpression> > expr_array;
    typedef common::ObSEArray<ObObj, OB_PREALLOCATED_NUM, common::ModulePageAllocator, ObArrayExpressionCallBack<ObObj> > cast_obj_array;

    /**
     * @brief The ObIndexTrigger class
     * ObIndexTrigger is designed for
     * modifying index table with opertions to data table
     */
    class ObIndexTrigger : public ObSingleChildPhyOperator
    {
    public:

      /**
       * @brief constructor
       */
      ObIndexTrigger();

      /**
       * @brief destructor
       */
      ~ObIndexTrigger();

      /**
       * @brief open
       * @return OB_SUCCESS or other ERROR
       */
      int open();

      /**
       * @brief close
       * @return OB_SUCCESS or other ERROR
       */
      int close();

      /**
       * @brief get_next_row
       * @param row
       * @return OB_SUCCESS or other ERROR
       */
      virtual int get_next_row(const ObRow *&row);

      /**
       * @brief get_row_desc
       * @param row_desc
       * @return OB_SUCCESS or other ERROR
       */
      int get_row_desc(const ObRowDesc *&row_desc) const;

      /**
       * @brief reset
       */
      virtual void reset();

      /**
       * @brief reuse
       */
      virtual void reuse();

      /**
       * @brief to_string
       * @param buf
       * @param buf_len
       * @return pos
       */
      virtual int64_t to_string(char* buf, const int64_t buf_len) const;

      /**
       * @brief get_type
       * @return PHY_INDEX_TRIGGER
       */
      virtual ObPhyOperatorType get_type() const;

      /**
       * @brief set_sql_type
       * @param type sql type to set
       */
      void set_sql_type(const SQLTYPE type);

      /**
       * @brief get_sql_type
       * @param type sql type to get
       */
      void get_sql_type(SQLTYPE& type);

      /**
       * @brief set_data_tid
       * @param table_id
       */
      void set_data_tid(int64_t table_id);

      //void set_data_row_desc(common::ObRowDesc &data_row_desc);
      //void get_data_row_desc(common::ObRowDesc *&data_row_desc);

      /**
       * @brief set_pre_data_row_desc
       * @param data_row_desc pre data row desc to set
       */
      void set_pre_data_row_desc(common::ObRowDesc &data_row_desc);

      /**
       * @brief get_pre_data_row_desc
       * @param data_row_desc pre data row desc to get
       */
      void get_pre_data_row_desc(common::ObRowDesc *&data_row_desc);

      /**
       * @brief set_post_data_row_desc
       * @param data_row_desc post data row desc to set
       */
      void set_post_data_row_desc(common::ObRowDesc &data_row_desc);

      /**
       * @brief get_post_data_row_desc
       * @param data_row_desc post data row desc to get
       */
      void get_post_data_row_desc(common::ObRowDesc *&data_row_desc);

      //void set_index_num(int64_t num);

      /**
       * @brief set_need_modify_index_num
       * set number of index which need to be modified
       * @param num
       */
      void set_need_modify_index_num(int64_t num);

      /**
       * @brief cons_data_row_store
       * store data which is got before and after the operation
       * @return OB_SUCCESS or other ERROR
       */
      int cons_data_row_store();

      /**
       * @brief add_pre_data_row
       * add row to pre_data_row_store_
       * @param pre_data_row
       */
      void add_pre_data_row(common::ObRow pre_data_row);

      /**
       * @brief add_post_data_row
       * add row to post_data_row_store_
       * @param post_data_row
       */
      void add_post_data_row(common::ObRow post_data_row);

      /**
       * @brief get_pre_data_row_store
       * @param pre_data_row_store
       */
      void get_pre_data_row_store(common::ObRowStore *&pre_data_row_store);

      /**
       * @brief get_post_data_row_store
       * @param post_data_row_store
       */
      void get_post_data_row_store(common::ObRowStore *&post_data_row_store);

      /**
       * @brief add_row_desc_del
       * add row description to index_row_desc_del_[]
       * @param idx
       * @param row_desc row description to add
       * @return OB_SUCCESS or other ERROR
       */
      int add_row_desc_del(int64_t idx, common::ObRowDesc row_desc);

      /**
       * @brief add_row_desc_ins
       * add row description to index_row_desc_ins_[]
       * @param idx
       * @param row_desc row description to add
       * @return OB_SUCCESS or other ERROR
       */
      int add_row_desc_ins(int64_t idx, common::ObRowDesc row_desc);

      /**
       * @brief get_row_desc_del
       * get row description from index_row_desc_del_[] with the given index
       * @param idx
       * @param row_desc row description to get
       * @return OB_SUCCESS or other ERROR
       */
      int get_row_desc_del(int64_t idx, common::ObRowDesc &row_desc);

      /**
       * @brief get_row_desc_ins
       * get row description from index_row_desc_ins_[] with the given index
       * @param idx
       * @param row_desc row description to get
       * @return OB_SUCCESS or other ERROR
       */
      int get_row_desc_ins(int64_t idx, common::ObRowDesc &row_desc);

      /**
       * @brief set_cond_flag
       * @param flag
       */
      void set_cond_flag(bool flag);

      //for update
      //int add_set_index_column(const ObSqlExpression& expr);
      //int add_set_cast_obj(const ObObj &obj);
      //void set_replace_values_id(uint64_t replace_values_id);

      /**
       * @brief reset_iterator
       */
      void reset_iterator();

      /**
       * @brief handle_trigger
       * handle all index tables which need to be modified
       * @param schema_mgr
       * @param host
       * @param session_ctx
       * @return OB_SUCCESS or other ERROR
       */
      int handle_trigger(const ObSchemaManagerV2 *schema_mgr, updateserver::ObIUpsTableMgr *host, updateserver::RWSessionCtx &session_ctx);

      /**
       * @brief handle_one_index_table
       * handle one index table which need to be modified
       * @param index_idx
       * @param schema_mgr
       * @param host
       * @param session_ctx
       * @return OB_SUCCESS or other ERROR
       */
      int handle_one_index_table(int64_t index_idx, const ObSchemaManagerV2 *schema_mgr, updateserver::ObIUpsTableMgr *host, updateserver::RWSessionCtx &session_ctx);

      //add huangjianwei [secondary index maintain] 20160909:b
       void set_input_values(uint64_t subquery) { replace_values_id_ = subquery; }
       //add:e
      DECLARE_PHY_OPERATOR_ASSIGN;
      NEED_SERIALIZE_AND_DESERIALIZE;

    private:
      //mod huangjianwei [secondary index maintain] 20160909:b
      //int sql_type_; ///<type of sql query statement, including 0 for insert, 1 for delete, 2 for update, 3 for replace
      SQLTYPE sql_type_;
      //mod:e
      int64_t data_tid_; ///<data table id

      //common::ObRow data_row_;
      //common::ObRowDesc data_row_desc_;

      common::ObRowDesc pre_data_row_desc_; ///<row description for data which is get before the operation
      common::ObRowDesc post_data_row_desc_; ///<row description for data which is get after the operation

      //int64_t index_num;

      int64_t need_modify_index_num_; ///<number of index tables which need to be modified
      common::ObRowStore pre_data_row_store_; ///<row store for data which is get before the operation
      common::ObRowStore post_data_row_store_; ///<row store for data which is get after the operation
      common::ObRowDesc index_row_desc_del_[OB_MAX_INDEX_NUMS]; ///<row description for data which need to be deleted
      common::ObRowDesc index_row_desc_ins_[OB_MAX_INDEX_NUMS]; ///<row description for data which need to be inserted
      bool cond_flag_; ///<true if the sql query statement has filter condition

      //for update
      //expr_array set_index_column_;
      //cast_obj_array set_cast_obj_;
      //ModuleArena arena_;

      bool delete_flag_for_update_; ///<true if the update sql query statement need delete rows from index table
      bool delete_flag_for_replace_; ///<true if the replace sql query statement need delete rows from index table

      //add huangjianwei [secondary index maintain] 20160909:b
      uint64_t replace_values_id_;
      ObExprValues replace_values_;
      //add:e
    };
  }
}
#endif // OB_INDEX_TRIGGER_H

