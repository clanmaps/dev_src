/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_ups_modify.h
* @brief for modifying data in update server
*
* modified by maoxiaoxiao:modify functions to update data in index table
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_ups_modify.h
 *
 * Authors:
 *   Li Kai <yubai.lk@alipay.com>
 *
 */
#ifndef  OCEANBASE_UPDATESERVER_MEMTABLE_MODIFY_H_
#define  OCEANBASE_UPDATESERVER_MEMTABLE_MODIFY_H_

#include "sql/ob_ups_modify.h"
#include "common/ob_iterator.h"
#include "common/ob_iterator_adaptor.h"
#include "ob_sessionctx_factory.h"
#include "ob_ups_table_mgr.h"
#include "ob_ups_utils.h"
//add maoxx
#include "sql/ob_index_trigger.h"
//add e
//add lbzhong [auto_increment] 20161127:b
#include "ob_update_server_main.h"
#include "sql/ob_auto_increment_filter.h"
//add:e

namespace oceanbase
{
  namespace updateserver
  {
    template <class T>
    class MemTableModifyTmpl : public T, public RowkeyInfoCache
    {
      public:
        MemTableModifyTmpl(RWSessionCtx &session, ObIUpsTableMgr &host);
        ~MemTableModifyTmpl();
      public:
        int open();
        int close();
        int get_next_row(const common::ObRow *&row);
        int get_row_desc(const common::ObRowDesc *&row_desc) const;
        int get_affected_rows(int64_t& row_count);
        int64_t to_string(char* buf, const int64_t buf_len) const;
        //add lbzhong [auto_increment] 20161127:b
        int check_auto_increment(const int64_t table_id, uint64_t& auto_column_id,
                             int64_t& auto_value, const CommonSchemaManager *&sm,
                                 ObPhyOperator * child_op,
                                 ObAutoIncrementFilter *& auto_increment_op);
        int get_auto_increment_value(const int64_t table_id, const uint64_t column_id, int64_t &auto_value);
        int update_auto_increment_value(const int64_t current_value, const uint64_t table_id,
                                        const uint64_t auto_column_id, const int64_t old_value);
        int set_is_update_auto_value();
        int64_t get_updated_auto_value() const;
        //add:e
      private:
        RWSessionCtx &session_;
        ObIUpsTableMgr &host_;
        //add lbzhong [auto_increment] 20161218:b
        bool is_update_auto_value_;
        int64_t update_auto_value_;
        //add:e
    };

    template <class T>
    MemTableModifyTmpl<T>::MemTableModifyTmpl(RWSessionCtx &session, ObIUpsTableMgr &host): session_(session),
                                                                                 host_(host)
                                                                                 //add lbzhong [auto_increment] 20161218:b
                                                                                 , is_update_auto_value_(false), update_auto_value_(OB_INVALID_AUTO_INCREMENT_VALUE)
                                                                                 //add:e
    {
    }

    template <class T>
    MemTableModifyTmpl<T>::~MemTableModifyTmpl()
    {
    }

    template <class T>
    int MemTableModifyTmpl<T>::open()
    {
      int ret = OB_SUCCESS;
      const ObRowDesc *row_desc = NULL;
      uint64_t table_id = OB_INVALID_ID;
      uint64_t column_id = OB_INVALID_ID;
      const ObRowkeyInfo *rki = NULL;
//add wangjiahao [table lock] 20160616 :b
      if (this->get_dml_type() == OB_DML_REPLACE)
      {
        T::child_op_->get_row_desc(row_desc);
        row_desc->get_tid_cid(0, table_id, column_id);
      }
      else
      {
        table_id = this->get_table_id();
      }
      //TBSYS_LOG(INFO, "##TEST_PRINT## what's' the fuck table_id=%lu", table_id);
      SessionTableLockInfo* stblk_info = NULL;
      if (table_id > 3000 && table_id <= 5048)
      {

        if (NULL == (stblk_info = session_.get_tblock_info()))
        {
          TBSYS_LOG(ERROR, "SessionTableLockInfo is NULL. table_id=%lu err=%d", table_id, ret);
        }
        else
        {
          TableLockMgr& global_tblk_mgr = host_.get_table_lock_mgr();
          uint32_t uid = session_.get_session_descriptor();
          if (OB_SUCCESS != (ret = stblk_info->lock_table(global_tblk_mgr, uid, table_id, IX_LOCK)))
          {
            TBSYS_LOG(WARN, "Lock table failed table_id=%ld err=%d", table_id, ret);
          }
        }

      }
//add :e
      //add lbzhong [auto_increment] 20161128:b
      uint64_t auto_column_id = OB_INVALID_ID;
      int64_t auto_value = OB_INVALID_AUTO_INCREMENT_VALUE;
      int64_t cur_auto_value = OB_INVALID_AUTO_INCREMENT_VALUE;
      ObAutoIncrementFilter *auto_increment_op = NULL;
      const CommonSchemaManager *sm = NULL;
      //add wsx[fix bug:ups coredump]
      UpsSchemaMgrGuard sm_guard;
      //e
      if (OB_SUCCESS == ret)
      {
      //del wsx
      //  UpsSchemaMgrGuard sm_guard;
      //e
        if (NULL == (sm = host_.get_schema_mgr().get_schema_mgr(sm_guard)))
        {
          TBSYS_LOG(WARN, "get_schema_mgr fail");
          ret = OB_SCHEMA_ERROR;
        }
        else if ((OB_DML_REPLACE == T::get_dml_type() || OB_DML_INSERT == T::get_dml_type()) &&
                 OB_SUCCESS != (ret = check_auto_increment(table_id, auto_column_id,
                                                           auto_value, sm, T::child_op_, auto_increment_op)))
        {
          if (OB_ERR_AUTO_VALUE_NOT_SERVE != ret)
          {
            TBSYS_LOG(WARN, "check_auto_increment fail, ret=%d", ret);
          }
        }
      }
      //add:e
      if (OB_SUCCESS != ret)
      {}
      else if (NULL == T::child_op_)
      {
        ret = OB_NOT_INIT;
      }
      else if (OB_SUCCESS != (ret = T::child_op_->open()))
      {
        if (!IS_SQL_ERR(ret))
        {
          TBSYS_LOG(WARN, "child operator open fail ret=%d", ret);
        }
      }
      else if (OB_SUCCESS != (ret = T::child_op_->get_row_desc(row_desc))
              || NULL == row_desc)
      {
        if (OB_ITER_END != ret)
        {
          TBSYS_LOG(WARN, "get_row_desc from child_op=%p type=%d fail ret=%d", T::child_op_, T::child_op_->get_type(), ret);
          ret = (OB_SUCCESS != ret) ? OB_ERROR : ret;
        }
        else
        {
          ret = OB_SUCCESS;
        }
      }
      else if (OB_SUCCESS != (ret = row_desc->get_tid_cid(0, table_id, column_id))
              || OB_INVALID_ID == table_id)
      {
        TBSYS_LOG(WARN, "get_tid_cid from row_desc fail, child_op=%p type=%d %s ret=%d",
                  T::child_op_, T::child_op_->get_type(), to_cstring(*row_desc), ret);
        ret = (OB_SUCCESS != ret) ? OB_ERROR : ret;
      }
      else if (NULL == (rki = get_rowkey_info(host_, table_id)))
      {
        TBSYS_LOG(WARN, "get_rowkey_info fail table_id=%lu", table_id);
        ret = OB_SCHEMA_ERROR;
      }
      else
      {
        //del lbzhong [auto_increment] 20161218:b
        /*
        UpsSchemaMgrGuard sm_guard;
        const CommonSchemaManager *sm = NULL;
        if (NULL == (sm = host_.get_schema_mgr().get_schema_mgr(sm_guard)))
        {
          TBSYS_LOG(WARN, "get_schema_mgr fail");
          ret = OB_SCHEMA_ERROR;
        }

        else
        {
        */
        //del:e
          //add maoxx
          if(T::child_op_->get_type() == sql::PHY_INDEX_TRIGGER)
          {
            ObIUpsTableMgr* host = &host_;
            sql::ObIndexTrigger *tmp_index_trigger = NULL;
            tmp_index_trigger = static_cast<sql::ObIndexTrigger*>(T::child_op_);
            if(OB_SUCCESS == (ret = (tmp_index_trigger->cons_data_row_store())))
            {
              //mod huangjianwei [secondary index maintain] 20160909:b
              //int sql_type;
              SQLTYPE sql_type;
              //mod:e
              ObRowDesc *row_desc = NULL;
              ObRowStore *pre_row_store = NULL;
              ObRowStore *post_row_store = NULL;
              ObRowCellIterAdaptor cia;
              //add lbzhong [auto_increment] 20161217:b
              if (OB_INVALID_ID != auto_column_id)
              {
                cia.set_auto_column_id(auto_column_id);
              }
              //add:e
              tmp_index_trigger->get_sql_type(sql_type);
              if(DELETE == sql_type)
              {
                tmp_index_trigger->get_pre_data_row_desc(row_desc);
                if (OB_SUCCESS != (ret = row_desc->add_column_desc(table_id, OB_ACTION_FLAG_COLUMN_ID)))
                {
                  TBSYS_LOG(ERROR, "Failed to add column desc, ret=%d", ret);
                }
                else
                {
                  tmp_index_trigger->get_pre_data_row_store(pre_row_store);
                  cia.set_row_iter(pre_row_store, row_desc->get_rowkey_cell_count(), sm, *row_desc);
                }
              }
              //add huangjianwei [secondary index debug] 20170314:b
              else if(UPDATE == sql_type)
              {
                const ObRowDesc *update_row_desc = NULL;
                sql::ObProject *project = NULL;
                project = static_cast<sql::ObProject*>(tmp_index_trigger->get_child(0));
                project->get_row_desc(update_row_desc);
                tmp_index_trigger->get_post_data_row_store(post_row_store);
                cia.set_row_iter(post_row_store, rki->get_size(), sm, *update_row_desc);
               }
              //add:e
              //mod huangjianwei [secondary index debug] 20170314:b
              //else if(INSERT == sql_type || UPDATE == sql_type || REPLACE == sql_type)
              else if(INSERT == sql_type|| REPLACE == sql_type)
              //mod:e
              {
                tmp_index_trigger->get_post_data_row_desc(row_desc);
                tmp_index_trigger->get_post_data_row_store(post_row_store);
                cia.set_row_iter(post_row_store, row_desc->get_rowkey_cell_count(), sm, *row_desc);
              }
              ret = host->apply(session_, cia, T::get_dml_type());
              session_.inc_dml_count(T::get_dml_type());

              if(OB_SUCCESS == ret && OB_SUCCESS != (ret = tmp_index_trigger->handle_trigger(sm, host, session_)))
              {
                TBSYS_LOG(ERROR, "modify index table fail");
              }
              /*else
                    {
                      index_num = tmp_index_tri->get_index_num();
                    }*/
              if (OB_INVALID_ID != auto_column_id)
              {
                cur_auto_value = cia.get_max_auto_value();
              }
            }
            else
              TBSYS_LOG(WARN, "index_trigger get_next_data_row fail ret=%d", ret);
          }
          else
          {
            ObCellIterAdaptor cia;
            cia.set_row_iter(T::child_op_, rki->get_size(), sm
                             //add lbzhong [auto_increment] 20161127:b
                             , is_update_auto_value_
                             //add:e
                             );
            ret = host_.apply(session_, cia, T::get_dml_type());
            session_.inc_dml_count(T::get_dml_type());

            //add lbzhong [auto_increment] 20161207:b
            if (OB_INVALID_ID != auto_column_id)
            {
              if (auto_increment_op == NULL)
              {
                TBSYS_LOG(WARN, "auto_increment_op is NULL");
                ret = OB_ERR_UNEXPECTED;
              }
              else
              {
                cur_auto_value = auto_increment_op->get_cur_auto_value();
              }
            }
            else if (is_update_auto_value_)
            {
              update_auto_value_ = cia.get_updated_auto_value();
            }
            //add:e
          }
          //add e
        //} //del lbzhong [auto_increment] 20161218:b
      }
      //add lbzhong [auto_increment] 20161218:b
      if (OB_SUCCESS == ret)
      {
        if (OB_SUCCESS == ret && OB_INVALID_ID != auto_column_id
            && this->get_auto_value() == OB_INVALID_AUTO_INCREMENT_VALUE)
        {
          ret = update_auto_increment_value(cur_auto_value, table_id, auto_column_id, auto_value);
        }
      }
      //add:e
      if (OB_SUCCESS != ret)
      {
        if (NULL != T::child_op_)
        {
          T::child_op_->close();
        }
      }
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>::close()
    {
      int ret = OB_SUCCESS;
      if (NULL == T::child_op_)
      {
        ret = OB_NOT_INIT;
      }
      else
      {
        int tmp_ret = OB_SUCCESS;
        if (OB_SUCCESS != (tmp_ret = T::child_op_->close()))
        {
          TBSYS_LOG(WARN, "child operator close fail ret=%d", tmp_ret);
        }
      }
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>::get_affected_rows(int64_t& row_count)
    {
      int ret = OB_SUCCESS;
      row_count = session_.get_ups_result().get_affected_rows();
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>::get_next_row(const common::ObRow *&row)
    {
      UNUSED(row);
      return OB_ITER_END;
    }

    template <class T>
    int MemTableModifyTmpl<T>::get_row_desc(const common::ObRowDesc *&row_desc) const
    {
      UNUSED(row_desc);
      return OB_ITER_END;
    }

    template <class T>
    int64_t MemTableModifyTmpl<T>::to_string(char* buf, const int64_t buf_len) const
    {
      int64_t pos = 0;
      databuff_printf(buf, buf_len, pos, "MemTableModify(op_type=%d dml_type=%s session=%p[%d:%ld])\n",
                      T::get_type(),
                      str_dml_type(T::get_dml_type()),
                      &session_,
                      session_.get_session_descriptor(),
                      session_.get_session_start_time());
      if (NULL != T::child_op_)
      {
        pos += T::child_op_->to_string(buf+pos, buf_len-pos);
      }
      return pos;
    }

    typedef MemTableModifyTmpl<sql::ObUpsModify> MemTableModify;
    typedef MemTableModifyTmpl<sql::ObUpsModifyWithDmlType> MemTableModifyWithDmlType;

    //add lbzhong [auto_increment] 20161127:b
    template <class T>
    int MemTableModifyTmpl<T>::check_auto_increment(const int64_t table_id, uint64_t& auto_column_id,
                                                    int64_t& auto_value, const CommonSchemaManager *&sm,
                                                    ObPhyOperator * child_op,
                                                    ObAutoIncrementFilter *& auto_increment_op)
    {
      int ret = OB_SUCCESS;
      if (NULL != sm)
      {
        ObColumnSchemaV2* columns = NULL;
        int32_t column_size = 0;
        columns = const_cast<ObColumnSchemaV2*>(sm->get_table_schema(table_id, column_size));
        if (columns != NULL)
        {
          for (int32_t i = 0; i < column_size; i++)
          {
            if (columns[i].is_auto_increment())
            {
              auto_column_id = columns[i].get_id();
              break;
            }
          }
        }
        if (OB_INVALID_ID != auto_column_id)
        {
          if ((auto_value = this->get_auto_value()) > OB_INVALID_AUTO_INCREMENT_VALUE)
          {
            //do nothing
          }
          else if (OB_SUCCESS != (ret = get_auto_increment_value(table_id, auto_column_id, auto_value)))
          {
            //do nothing
          }
          ObPhyOperator *tmp_op = child_op;
          while (NULL != tmp_op)
          {
            if(PHY_AUTO_INCREMENT_FILTER == tmp_op->get_type())
            {
              auto_increment_op = static_cast<ObAutoIncrementFilter*>(tmp_op);
              break;
            }
            tmp_op = tmp_op->get_child(0);
          }
          if (auto_increment_op != NULL)
          {
            auto_increment_op->set_auto_column(auto_column_id, auto_value);
          }
        }
      }
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>::get_auto_increment_value(const int64_t table_id, const uint64_t column_id, int64_t &auto_value)
    {
      int ret = OB_SUCCESS;
      ObRowDesc row_desc;
      row_desc.set_rowkey_cell_count(2);

      ObCellNewScanner new_scanner;
      ObGetParam get_param;
      ObCellInfo cell_info;
      ObObj rowkey_objs[2];
      rowkey_objs[0].set_int(table_id);
      rowkey_objs[1].set_int(column_id);
      cell_info.row_key_.assign(rowkey_objs, 2);
      cell_info.table_id_ = OB_ALL_AUTO_INCREMENT_TID;
      cell_info.column_id_ = OB_APP_MIN_COLUMN_ID + 2;
      if (OB_SUCCESS != (ret = get_param.add_cell(cell_info)))
      {
        TBSYS_LOG(WARN, "fail to add cell to get param:ret[%d]", ret);
      }
      else if (OB_SUCCESS != (ret = row_desc.add_column_desc(OB_ALL_AUTO_INCREMENT_TID, OB_APP_MIN_COLUMN_ID + 2)))
      {
        TBSYS_LOG(WARN, "fail to add column to row_desc:ret[%d]", ret);
      }
      if (OB_SUCCESS == ret)
      {
        TableMgr* table_mgr = ObUpdateServerMain::get_instance()->get_update_server().get_table_mgr().get_table_mgr();
        ObVersionRange version_range;
        version_range.border_flag_.set_inclusive_start();
        version_range.border_flag_.set_max_value();
        version_range.start_version_ = table_mgr->get_cur_major_version();
        get_param.set_version_range(version_range);

        new_scanner.set_row_desc(row_desc);
        if (OB_SUCCESS != (ret = ObUpdateServerMain::get_instance()->get_update_server().get_table_mgr().new_get(session_,
                                  get_param, new_scanner, tbsys::CTimeUtil::getTime(), 1000, sql::LF_WRITE)))
        {
          TBSYS_LOG(WARN, "fail to get max_value:ret[%d]", ret);
        }
      }
      if (OB_SUCCESS == ret)
      {
        ObRow row;
        row.set_row_desc(row_desc);
        const ObRowkey *rowkey = NULL;
        while (OB_SUCCESS == ret)
        {
          ret = new_scanner.get_next_row(rowkey, row);;
          if (OB_ITER_END == ret)
          {
            ret = OB_SUCCESS;
            break;
          }
          else
          {
            const ObObj *cell = NULL;
            if (OB_SUCCESS != (ret = row.get_cell(OB_ALL_AUTO_INCREMENT_TID, 18, cell)))
            {
              TBSYS_LOG(WARN, "fail to get cell:ret[%d]", ret);
            }
            else
            {
              auto_value = OB_INVALID_AUTO_INCREMENT_VALUE;
              cell->get_int(auto_value);
              if (OB_INVALID_AUTO_INCREMENT_VALUE == auto_value)
              {
                ret = OB_ERR_AUTO_VALUE_NOT_SERVE;
              }
            }
          }
        }
      }
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>::update_auto_increment_value(const int64_t current_value, const uint64_t table_id,
                                                           const uint64_t auto_column_id, const int64_t old_value)
    {
      int ret = OB_SUCCESS;
      if (current_value > old_value)
      {
        ObAutoIncrementCellIterAdaptor aicia;
        aicia.set_row_iter(table_id, auto_column_id, current_value);
        ret = host_.apply(session_, aicia, OB_DML_UPDATE);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "fail to update auto_increment, ret=%d", ret);
        }
      }
      return ret;
    }

    template <class T>
    int MemTableModifyTmpl<T>:: set_is_update_auto_value()
    {
      is_update_auto_value_ = true;
      return OB_SUCCESS;
    }

    template <class T>
    int64_t MemTableModifyTmpl<T>:: get_updated_auto_value() const
    {
      return update_auto_value_;
    }
    //add:e
  } // end namespace updateserver
} // end namespace oceanbase

#endif /* OCEANBASE_UPDATESERVER_MEMTABLE_MODIFY_H_ */

