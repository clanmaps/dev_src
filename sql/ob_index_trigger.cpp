/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_index_trigger.cpp
* @brief for operations of index trigger
*
* Created by maoxiaoxiao:modify index table with opertions to data table
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#include "ob_index_trigger.h"
#include "common/ob_iterator_adaptor.h"
#include "sql/ob_physical_plan.h"

namespace oceanbase
{
  namespace sql
  {
    REGISTER_PHY_OPERATOR(ObIndexTrigger, PHY_INDEX_TRIGGER);
  }
}

namespace oceanbase
{
  namespace sql
  {
    ObIndexTrigger::ObIndexTrigger()
    {
      cond_flag_ = false;
      delete_flag_for_update_ = true;
      delete_flag_for_replace_ = false;
    }

    ObIndexTrigger::~ObIndexTrigger()
    {

    }

    int ObIndexTrigger::open()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != (ret = ObSingleChildPhyOperator::open()))
      {
        if (!IS_SQL_ERR(ret))
        {
          TBSYS_LOG(WARN, "failed to open child_op, err=%d", ret);
        }
      }
      return ret;
    }

    int ObIndexTrigger::close()
    {
      int ret = OB_SUCCESS;
      if (NULL == child_op_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "child_op is NULL");
      }
      else
      {
        int tmp_ret = OB_SUCCESS;
        if (OB_SUCCESS != (tmp_ret = child_op_->close()))
        {
          TBSYS_LOG(WARN, "close child_op fail ret=%d", tmp_ret);
        }
        //add huangjianwei [secondary index maintain] 20160909:b
        else if (OB_SUCCESS != (tmp_ret = replace_values_.close()))
        {
          TBSYS_LOG(WARN, "fail to close replace_values, err=%d", tmp_ret);
        }
        //add:e
      }
      return ret;
    }

    int ObIndexTrigger::get_next_row(const common::ObRow *&row)
    {
      int ret = OB_SUCCESS;
      if (NULL == child_op_)
      {
        ret = OB_NOT_INIT;
      }
      else
      {
        ObProject* op=static_cast<ObProject*>(child_op_);
        if(OB_SUCCESS != (ret = op->get_next_row(row)))
        {
          if(ret != OB_ITER_END)
            TBSYS_LOG(WARN, "child_op get_next_row fail ret=%d", ret);
        }
      }
      return ret;
    }

    int ObIndexTrigger::get_row_desc(const common::ObRowDesc *&row_desc) const
    {
      int ret = OB_SUCCESS;
      if (NULL == child_op_)
      {
        ret = OB_INVALID_ARGUMENT;
        TBSYS_LOG(WARN, "child_op pointer is NULL");
      }
      else if(OB_SUCCESS != (ret = child_op_->get_row_desc(row_desc)))
      {
        TBSYS_LOG(WARN, "child_op get_next_row fail ret=%d", ret);
      }
      return ret;
    }
    //mod huangjianwei [secondary index maintain] 20160909:b
    //void ObIndexTrigger::set_sql_type(int type)
    void ObIndexTrigger::set_sql_type(const SQLTYPE type)
    //mod:e
    {
      sql_type_ = type;
    }
    //mod huangjianwei [secondary index maintain] 20160909:b
    //void ObIndexTrigger::get_sql_type(int &type)
    void ObIndexTrigger::get_sql_type(SQLTYPE &type)
    //mod:e
    {
      type = sql_type_;
    }

    void ObIndexTrigger::set_data_tid(int64_t table_id)
    {
      data_tid_ = table_id;
    }

    /*void ObIndexTrigger::set_data_row_desc(common::ObRowDesc &data_row_desc)
        {
          data_row_desc_ = data_row_desc;
        }

        void ObIndexTrigger::get_data_row_desc(common::ObRowDesc *&data_row_desc)
        {
          data_row_desc = &data_row_desc_;
        }*/

    void ObIndexTrigger::set_pre_data_row_desc(common::ObRowDesc &data_row_desc)
    {
      pre_data_row_desc_ = data_row_desc;
    }

    void ObIndexTrigger::get_pre_data_row_desc(common::ObRowDesc *&data_row_desc)
    {
      data_row_desc = &pre_data_row_desc_;
    }

    void ObIndexTrigger::set_post_data_row_desc(common::ObRowDesc &data_row_desc)
    {
      post_data_row_desc_ = data_row_desc;
    }

    void ObIndexTrigger::get_post_data_row_desc(common::ObRowDesc *&data_row_desc)
    {
      data_row_desc = &post_data_row_desc_;
    }

    void ObIndexTrigger::set_need_modify_index_num(int64_t num)
    {
      need_modify_index_num_ = num;
    }

    int ObIndexTrigger::cons_data_row_store()
    {
      int ret = OB_SUCCESS;
      const ObRow *row = NULL;
      const ObRowStore::StoredRow *stored_row = NULL;
      if(INSERT == sql_type_)
      {
        while(OB_SUCCESS == (ret = child_op_->get_next_row(row)))
        {
          //mod huangjianwei [index debug] 20180402:b
          //post_data_row_store_.add_row(*row, stored_row);
          if (OB_SUCCESS != (ret = post_data_row_store_.add_row(*row, stored_row)))
          {
            TBSYS_LOG(WARN,"post row_store add row failed, ret=[%d]", ret);
            break;
          }
          //mod:e
        }
        if(OB_ITER_END == ret)
          ret = OB_SUCCESS;
      }
      else if(DELETE == sql_type_)
      {
        ObProject* project_op = dynamic_cast<ObProject*> (child_op_);
        while(OB_SUCCESS == (ret = project_op->get_next_row_with_index(row, &pre_data_row_store_, NULL)))
        {

        }
        if(OB_ITER_END == ret)
          ret = OB_SUCCESS;
      }
      else if(UPDATE == sql_type_)
      {
        ObProject* project_op = dynamic_cast<ObProject*> (child_op_);
        while(OB_SUCCESS == (ret = project_op->get_next_row_with_index(row, &pre_data_row_store_, &post_data_row_store_)))
        {

        }
        if(OB_ITER_END == ret)
          ret = OB_SUCCESS;
      }
      else if(REPLACE == sql_type_)
      {
        bool row_empty_flag = false;
        while(OB_SUCCESS == (ret = child_op_->get_next_row(row)))
        {
          if(OB_SUCCESS != (ret = row->get_is_row_empty(row_empty_flag)))
          {
            TBSYS_LOG(WARN, "fail to get is row empty, err=%d", ret);
          }
          else if(!row_empty_flag)
          {
            delete_flag_for_replace_ = true;
            //mod huangjianwei [index debug] 20180402:b
            //pre_data_row_store_.add_row(*row, stored_row);
            if (OB_SUCCESS != (ret = pre_data_row_store_.add_row(*row, stored_row)))
            {
              TBSYS_LOG(WARN,"pre row_store add row failed, ret=[%d]", ret);
              break;
            }
            //mod:e
          }
        }
        if(OB_ITER_END == ret)
          ret = OB_SUCCESS;
        //add huangjianwei [secondary index maintain] 20160909:b
        if(OB_SUCCESS == ret)
        {
          replace_values_.open();
          while(OB_SUCCESS == (ret = replace_values_.get_next_row(row)))
          {
            //mod huangjianwei [index debug] 20180402:b
            //post_data_row_store_.add_row(*row, stored_row);
            if (OB_SUCCESS != (ret = post_data_row_store_.add_row(*row, stored_row)))
            {
              TBSYS_LOG(WARN,"post row_store add row failed, ret=[%d]", ret);
              break;
            }
            //mod:e
          }
          if(OB_ITER_END == ret)
          ret = OB_SUCCESS;
        }
        //add:e
      }
      return ret;
    }

    void ObIndexTrigger::add_pre_data_row(common::ObRow pre_data_row)
    {
      const ObRowStore::StoredRow *stored_row = NULL;
      pre_data_row_store_.add_row(pre_data_row, stored_row);
    }

    void ObIndexTrigger::add_post_data_row(common::ObRow post_data_row)
    {
      const ObRowStore::StoredRow *stored_row = NULL;
      post_data_row_store_.add_row(post_data_row, stored_row);
    }

    void ObIndexTrigger::get_pre_data_row_store(common::ObRowStore *&pre_data_row_store)
    {
      pre_data_row_store = &pre_data_row_store_;
    }

    void ObIndexTrigger::get_post_data_row_store(common::ObRowStore *&post_data_row_store)
    {
      post_data_row_store = &post_data_row_store_;
    }

    int ObIndexTrigger::add_row_desc_del(int64_t idx, common::ObRowDesc row_desc)
    {
      int ret = OB_SUCCESS;
      if(idx >= OB_MAX_INDEX_NUMS || idx < 0)
      {
        TBSYS_LOG(ERROR,"add row desc_del ,idx is invalid! idx=%ld",idx);
        ret = OB_ERROR;
      }
      else
        index_row_desc_del_[idx] = row_desc;
      return ret;
    }

    int ObIndexTrigger::add_row_desc_ins(int64_t idx, common::ObRowDesc row_desc)
    {
      int ret = OB_SUCCESS;
      if(idx >= OB_MAX_INDEX_NUMS || idx < 0)
      {
        TBSYS_LOG(ERROR,"add row desc_del ,idx is invalid! idx=%ld",idx);
        ret = OB_ERROR;
      }
      else
        index_row_desc_ins_[idx] = row_desc;
      return ret;
    }

    int ObIndexTrigger::get_row_desc_del(int64_t idx, common::ObRowDesc &row_desc)
    {
      int ret = OB_SUCCESS;
      if (idx >= OB_MAX_INDEX_NUMS)
      {
        ret = OB_ERROR;
      }
      else
        row_desc = index_row_desc_del_[idx];
      return ret;
    }

    int ObIndexTrigger::get_row_desc_ins(int64_t idx, common::ObRowDesc &row_desc)
    {
      int ret = OB_SUCCESS;
      if(idx >= OB_MAX_INDEX_NUMS)
      {
        ret = OB_ERROR;
      }
      else
        row_desc = index_row_desc_ins_[idx];
      return ret;
    }

    void ObIndexTrigger::set_cond_flag(bool flag)
    {
      cond_flag_ = flag;
    }

    /*void ObIndexTrigger::set_replace_values_id(uint64_t replace_values_id)
        {
            replace_values_id_ = replace_values_id;
        }*/

    void ObIndexTrigger::reset_iterator()
    {
      if(INSERT == sql_type_)
      {
        ObInsertDBSemFilter *insert_sem = static_cast<ObInsertDBSemFilter*>(child_op_);
        insert_sem->reset_iterator();
      }
      else if(DELETE == sql_type_ || UPDATE == sql_type_)
      {
        if(!cond_flag_)
        {
          ObMultipleGetMerge* fuse_op = static_cast<ObMultipleGetMerge*>(child_op_->get_child(0));
          fuse_op->reset_iterator();
        }
        else
        {
          ObMultipleGetMerge* fuse_op = static_cast<ObMultipleGetMerge*>(child_op_->get_child(0)->get_child(0));
          fuse_op->reset_iterator();
        }
      }
      else if(REPLACE == sql_type_)
      {
        ObMultipleGetMerge* fuse_op = static_cast<ObMultipleGetMerge*>(child_op_);
        fuse_op->reset_iterator();
        //add huangjianwei [secondary index maintain] 20160909:b
        replace_values_.reset_iterator();
        //add:e
      }
    }

    int ObIndexTrigger::handle_trigger(const ObSchemaManagerV2 *schema_mgr, updateserver::ObIUpsTableMgr *host, updateserver::RWSessionCtx &session_ctx)
    {
      int ret = OB_SUCCESS;
      if(schema_mgr == NULL)
      {
        TBSYS_LOG(ERROR,"schema manager is NULL");
        ret = OB_SCHEMA_ERROR;
      }
      else
      {
        if(UPDATE == sql_type_ || REPLACE == sql_type_)
          ret = pre_data_row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
        for(int64_t i = 0; OB_SUCCESS == ret && i < need_modify_index_num_; i++)
        {
          if(OB_SUCCESS != (ret = handle_one_index_table(i, schema_mgr, host, session_ctx)))
          {
            TBSYS_LOG(ERROR,"handle one index table error,ret=%d",ret);
            ret = OB_ERROR;
            break;
          }
        }
      }
      //add huangjianwei [index debug] 20180403:b
      pre_data_row_store_.clear();
      post_data_row_store_.clear();
      //add:e
      return ret;
    }

    int ObIndexTrigger::handle_one_index_table(int64_t index_idx, const ObSchemaManagerV2 *schema_mgr, updateserver::ObIUpsTableMgr *host, updateserver::RWSessionCtx &session_ctx)
    {
      int ret = OB_SUCCESS;
      common::ObRowStore index_row_del;
      common::ObRowStore index_row_ins;
      const common::ObRowStore::StoredRow *stored_row = NULL;
      common::ObRow row_to_store_del;
      common::ObRow row_to_store_ins;
      uint64_t index_tid = OB_INVALID_ID;
      uint64_t index_cid = OB_INVALID_ID;
      if (NULL == child_op_)
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "child_op is NULL");
      }
      else if(NULL == child_op_->get_child(0))
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "second child_op is NULL");
      }
      else if(cond_flag_ && NULL == child_op_->get_child(0)->get_child(0))
      {
        ret = OB_NOT_INIT;
        TBSYS_LOG(ERROR, "second child_op is NULL");
      }
      else
      {
        common::ObRow pre_data_row;
        common::ObRow post_data_row;
        const ObObj *obj = NULL;
        ObObj obj_dml, obj_virtual;
        //modify maoxx [replace bug fix] 20170324
//        if(OB_SUCCESS == ret && (DELETE == sql_type_ || (UPDATE == sql_type_ ) || (REPLACE == sql_type_ )))
        if(OB_SUCCESS == ret && DELETE == sql_type_)
        //modify e
        {
          pre_data_row.set_row_desc(pre_data_row_desc_);
          pre_data_row_store_.reset_iterator();
          while(OB_SUCCESS == (ret = (pre_data_row_store_.get_next_row(pre_data_row))))
          {
            //modify maoxx [replace bug fix] 20170324
//            if((DELETE == sql_type_) || (UPDATE == sql_type_ && delete_flag_for_update_) || (REPLACE == sql_type_ && delete_flag_for_replace_))
            //modify e
            {
              row_to_store_del.set_row_desc(index_row_desc_del_[index_idx]);
              for(int index_cid_idx = 0; index_cid_idx < index_row_desc_del_[index_idx].get_column_num() && OB_SUCCESS == ret; index_cid_idx++)
              {
                if(OB_SUCCESS != (ret = index_row_desc_del_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid)))
                {
                  TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                  break;
                }
                else if(OB_ACTION_FLAG_COLUMN_ID == index_cid)
                {
                  obj_dml.set_int(ObActionFlag::OP_DEL_ROW);
                  if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, obj_dml)))
                  {
                    TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                    break;
                  }
                }
                else if(OB_SUCCESS != (ret = pre_data_row.get_cell(data_tid_, index_cid, obj)))
                {
                  TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                  break;
                }
                else
                {
                  if(NULL == obj)
                  {
                    TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                    ret = OB_INVALID_ARGUMENT;
                    break;
                  }
                  else if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, *obj)))
                  {
                    TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                    break;
                  }
                }
              }
              if(OB_SUCCESS == ret)
              {
                if(OB_SUCCESS != (ret = (index_row_del.add_row(row_to_store_del, stored_row))))
                {
                  TBSYS_LOG(ERROR, "failed to do row_store_del.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_del));
                }
                else
                {
                  row_to_store_del.clear();
                }
              }
            }
          }
          if(OB_ITER_END == ret)
            ret = OB_SUCCESS;
        }

        //add maoxx [insert bug fix] 20170313
        if(OB_SUCCESS == ret && INSERT == sql_type_)
        {
          uint64_t data_tid = OB_INVALID_ID;
          uint64_t data_cid = OB_INVALID_ID;
          post_data_row.set_row_desc(post_data_row_desc_);
          post_data_row_store_.reset_iterator();
          while(OB_SUCCESS == (ret = (post_data_row_store_.get_next_row(post_data_row))))
          {
            row_to_store_ins.set_row_desc(index_row_desc_ins_[index_idx]);
            for(int data_cid_idx = 0; data_cid_idx < post_data_row.get_column_num() && OB_SUCCESS == ret; data_cid_idx++)
            {
              if(OB_SUCCESS != (ret = post_data_row.get_row_desc()->get_tid_cid(data_cid_idx, data_tid, data_cid)))
              {
                TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                break;
              }
              else
              {
                int64_t index_cid_idx = 0;
                for(; index_cid_idx < index_row_desc_ins_[index_idx].get_column_num(); index_cid_idx++)
                {
                  index_row_desc_ins_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid);
                  if(index_cid == data_cid)
                  {
                    break;
                  }
                }
                if(index_cid_idx < index_row_desc_ins_[index_idx].get_column_num())
                {
                  if(OB_SUCCESS != (ret = post_data_row.get_cell(data_tid_, index_cid, obj)))
                  {
                    TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                    break;
                  }
                  else
                  {
                    if(NULL == obj)
                    {
                      TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                      ret = OB_INVALID_ARGUMENT;
                      break;
                    }
                    else if(OB_SUCCESS != (ret = row_to_store_ins.set_cell(index_tid, index_cid, *obj)))
                    {
                      TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                      break;
                    }
                  }
                }
              }
            }//end for
            if(OB_SUCCESS == ret)
            {
              if(OB_SUCCESS != (ret = (index_row_ins.add_row(row_to_store_ins, stored_row))))
              {
                TBSYS_LOG(ERROR, "failed to do row_store.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_ins));
              }
              else
              {
                row_to_store_ins.clear();
              }
            }
          }
          if(OB_ITER_END == ret)
            ret = OB_SUCCESS;
        }
        //add e

        //modify maoxx [update bug fix] 20170313
//        if(OB_SUCCESS == ret && (INSERT == sql_type_ || UPDATE == sql_type_ || REPLACE == sql_type_))
        if(OB_SUCCESS == ret && UPDATE == sql_type_)
        //modify e
        {
          //mod huangjianwei [secondary index debug]:b
          const ObRowDesc *row_desc_after_cal = NULL;
          ObProject *project_op = dynamic_cast<ObProject*>(get_child(0));
          if(OB_SUCCESS != (ret = project_op->get_row_desc(row_desc_after_cal)))
          {
            TBSYS_LOG(ERROR, "failed in get post data row desc,ret[%d]", ret);
          }
          else
          {
            post_data_row_desc_ = *row_desc_after_cal;
            pre_data_row.set_row_desc(pre_data_row_desc_);
            pre_data_row_store_.reset_iterator();
            post_data_row.set_row_desc(post_data_row_desc_);
            post_data_row_store_.reset_iterator();
          }
          //mod:e

          //modify maoxx [update bug fix] 20170324
          //mod huangjianwei [secondary index debug]:b
          //post_data_row.set_row_desc(post_data_row_desc_);
          //post_data_row_store_.reset_iterator();
          int pre_ret = OB_SUCCESS;
          int post_ret = OB_SUCCESS;
          while(OB_SUCCESS == ret)
          //mod:e
          //while(OB_SUCCESS == (ret = (pre_data_row_store_.get_next_row(post_data_row))))
          {
            pre_ret = pre_data_row_store_.get_next_row(pre_data_row);
            post_ret = post_data_row_store_.get_next_row(post_data_row);
            if(OB_SUCCESS != pre_ret || OB_SUCCESS != post_ret)
            {
              break;
            }
          //modify e

            //add maoxx test
            TBSYS_LOG(ERROR,"test::maoxx pre_data_row=%s", to_cstring(pre_data_row));
            TBSYS_LOG(ERROR,"test::maoxx post_data_row=%s", to_cstring(post_data_row));
            //add e

            if(delete_flag_for_update_)
            {
              row_to_store_del.set_row_desc(index_row_desc_del_[index_idx]);
              for(int index_cid_idx = 0; index_cid_idx < index_row_desc_del_[index_idx].get_column_num() && OB_SUCCESS == ret; index_cid_idx++)
              {
                if(OB_SUCCESS != (ret = index_row_desc_del_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid)))
                {
                  TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                  break;
                }
                else if(OB_ACTION_FLAG_COLUMN_ID == index_cid)
                {
                  obj_dml.set_int(ObActionFlag::OP_DEL_ROW);
                  if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, obj_dml)))
                  {
                    TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                    break;
                  }
                }
                else if(OB_SUCCESS != (ret = pre_data_row.get_cell(data_tid_, index_cid, obj)))
                {
                  TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                  break;
                }
                else
                {
                  if(NULL == obj)
                  {
                    TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                    ret = OB_INVALID_ARGUMENT;
                    break;
                  }
                  else if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, *obj)))
                  {
                    TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                    break;
                  }
                }
              }//end for
              if(OB_SUCCESS == ret)
              {
                if(OB_SUCCESS != (ret = (index_row_del.add_row(row_to_store_del, stored_row))))
                {
                  TBSYS_LOG(ERROR, "failed to do row_store_del.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_del));
                }
                else
                {
                  row_to_store_del.clear();
                }
              }
            }

            row_to_store_ins.set_row_desc(index_row_desc_ins_[index_idx]);
            for(int index_cid_idx = 0; index_cid_idx < index_row_desc_ins_[index_idx].get_column_num() && OB_SUCCESS == ret; index_cid_idx++)
            {
              if(OB_SUCCESS != (ret = index_row_desc_ins_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid)))
              {
                TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                break;
              }
              else if(OB_ACTION_FLAG_COLUMN_ID == index_cid)
              {

              }
              else if(OB_INDEX_VIRTUAL_COLUMN_ID == index_cid)
              {
                obj_virtual.set_null();
                obj = &obj_virtual;
              }
              else
              {
                if(OB_INVALID_INDEX == post_data_row_desc_.get_idx(data_tid_, index_cid))
                {
                  if(OB_SUCCESS != (ret = pre_data_row.get_cell(data_tid_, index_cid, obj)))
                  {
                    TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                    break;
                  }
                }
                else if(OB_SUCCESS != (ret = post_data_row.get_cell(data_tid_, index_cid, obj)))
                {
                  TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                  break;
                }
              }
              if(OB_SUCCESS == ret)
              {
                if(NULL == obj)
                {
                  TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                  ret = OB_INVALID_ARGUMENT;
                  break;
                }
                else if(OB_SUCCESS != (ret = row_to_store_ins.set_cell(index_tid, index_cid, *obj)))
                {
                  TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                  break;
                }
              }
            }//end for
            if(OB_SUCCESS == ret)
            {
              if(OB_SUCCESS != (ret = (index_row_ins.add_row(row_to_store_ins, stored_row))))
              {
                TBSYS_LOG(ERROR, "failed to do row_store.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_ins));
              }
              else
              {
                row_to_store_ins.clear();
              }
            }
          }
          //modify maoxx [update bug fix] 20170324
//          if(OB_ITER_END == ret)
//            ret = OB_SUCCESS;
          if(OB_ITER_END == pre_ret && OB_ITER_END == post_ret)
          {
            ret = OB_SUCCESS;
          }
          else
          {
            TBSYS_LOG(ERROR, "failed to get next pre row and post row,pre_ret=%d,post_ret=%d", pre_ret, post_ret);
            ret = OB_ERROR;
          }
          //modify e
        }

        //add maoxx [replace bug fix] 20170324
        if(OB_SUCCESS == ret && REPLACE == sql_type_)
        {
          pre_data_row.set_row_desc(pre_data_row_desc_);
          pre_data_row_store_.reset_iterator();
          post_data_row.set_row_desc(post_data_row_desc_);
          post_data_row_store_.reset_iterator();
          bool is_row_empty = false;
          bool pre_break = false;
          while(OB_SUCCESS == ret)
          {
            if(OB_SUCCESS != (ret = pre_data_row_store_.get_next_row(pre_data_row)))
            {
              if(OB_ITER_END == ret)
              {
                ret = OB_SUCCESS;
                pre_break = true;
              }
              else
              {
                TBSYS_LOG(WARN, "get next pre data row failed! ret = %d", ret);
                break;
              }
            }
            if(OB_SUCCESS == ret && !pre_break)
            {
              //add maoxx test
              TBSYS_LOG(ERROR,"test::maoxx pre_data_row=%s", to_cstring(pre_data_row));
              //add e
              if (OB_SUCCESS != (ret = pre_data_row.get_is_row_empty(is_row_empty)))
              {
                TBSYS_LOG(WARN, "fail to get is row empty, err=%d", ret);
              }
              else if(!is_row_empty)
              {
                row_to_store_del.set_row_desc(index_row_desc_del_[index_idx]);
                for(int index_cid_idx = 0; index_cid_idx < index_row_desc_del_[index_idx].get_column_num() && OB_SUCCESS == ret; index_cid_idx++)
                {
                  if(OB_SUCCESS != (ret = index_row_desc_del_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid)))
                  {
                    TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                    break;
                  }
                  else if(OB_ACTION_FLAG_COLUMN_ID == index_cid)
                  {
                    obj_dml.set_int(ObActionFlag::OP_DEL_ROW);
                    if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, obj_dml)))
                    {
                      TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                      break;
                    }
                  }
                  else if(OB_SUCCESS != (ret = pre_data_row.get_cell(data_tid_, index_cid, obj)))
                  {
                    TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                    break;
                  }
                  else
                  {
                    if(NULL == obj)
                    {
                      TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                      ret = OB_INVALID_ARGUMENT;
                      break;
                    }
                    else if(OB_SUCCESS != (ret = row_to_store_del.set_cell(index_tid, index_cid, *obj)))
                    {
                      TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                      break;
                    }
                  }
                }//end for
                if(OB_SUCCESS == ret)
                {
                  if(OB_SUCCESS != (ret = (index_row_del.add_row(row_to_store_del, stored_row))))
                  {
                    TBSYS_LOG(ERROR, "failed to do row_store_del.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_del));
                  }
                  else
                  {
                    //add maoxx test
                    TBSYS_LOG(ERROR,"test::maoxx row_to_store_del=%s", to_cstring(row_to_store_del));
                    //add e
                    row_to_store_del.clear();
                  }
                }
              }
            }
            bool post_break = false;
            bool rowkey_equal = false;
            const ObRowkey* pre_key = NULL;
            const ObRowkey* post_key = NULL;
            while(OB_SUCCESS == ret)
            {
              if(OB_SUCCESS != (ret = post_data_row_store_.get_next_row(post_data_row)))
              {
                if(OB_ITER_END == ret)
                {
                  ret = OB_SUCCESS;
                  post_break = true;
                }
                else
                {
                  TBSYS_LOG(WARN, "get next post data row failed! ret = %d", ret);
                  break;
                }
              }
              if(OB_SUCCESS == ret && !post_break)
              {
                //add maoxx test
                TBSYS_LOG(ERROR,"test::maoxx post_data_row=%s", to_cstring(post_data_row));
                //add e
                if(OB_SUCCESS != (ret = pre_data_row.get_rowkey(pre_key)) || OB_SUCCESS != (ret = post_data_row.get_rowkey(post_key)))
                {
                  TBSYS_LOG(WARN, "get row key failed,ret = %d", ret);
                }
                else if(0 == pre_key->compare(*post_key))
                {
                  post_break = true;
                  rowkey_equal = true;
                }
                row_to_store_ins.set_row_desc(index_row_desc_ins_[index_idx]);
                for(int index_cid_idx = 0; index_cid_idx < index_row_desc_ins_[index_idx].get_column_num() && OB_SUCCESS == ret; index_cid_idx++)
                {
                  if(OB_SUCCESS != (ret = index_row_desc_ins_[index_idx].get_tid_cid(index_cid_idx, index_tid, index_cid)))
                  {
                    TBSYS_LOG(ERROR,"failed in get tid_cid from row desc,ret[%d]", ret);
                    break;
                  }
                  else if(OB_ACTION_FLAG_COLUMN_ID == index_cid)
                  {
                  }
                  else if(OB_INDEX_VIRTUAL_COLUMN_ID == index_cid)
                  {
                    obj_virtual.set_null();
                    obj = &obj_virtual;
                  }
                  else
                  {
                    if((OB_INVALID_INDEX == post_data_row_desc_.get_idx(data_tid_, index_cid)))
                    {
                      if(!rowkey_equal)
                      {
                        obj_virtual.set_null();
                        obj = &obj_virtual;
                      }
                      else if(OB_SUCCESS != (ret = pre_data_row.get_cell(data_tid_, index_cid, obj)))
                      {
                        TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                        break;
                      }
                    }
                    else if(OB_SUCCESS != (ret = post_data_row.get_cell(data_tid_, index_cid, obj)))
                    {
                      TBSYS_LOG(ERROR,"failed in get cell from data_row,ret[%d],tid[%ld],cid[%ld]", ret, data_tid_, index_cid);
                      break;
                    }
                  }
                  if(OB_SUCCESS == ret)
                  {
                    if(NULL == obj)
                    {
                      TBSYS_LOG(ERROR,"obj's pointer can not be NULL!");
                      ret = OB_INVALID_ARGUMENT;
                      break;
                    }
                    else if(OB_SUCCESS != (ret = row_to_store_ins.set_cell(index_tid, index_cid, *obj)))
                    {
                      TBSYS_LOG(ERROR,"set cell failed,ret=%d", ret);
                      break;
                    }
                  }
                }//end for
                if(OB_SUCCESS == ret)
                {
                  if(OB_SUCCESS != (ret = (index_row_ins.add_row(row_to_store_ins, stored_row))))
                  {
                    TBSYS_LOG(ERROR, "failed to do row_store.add_row faile,ret=%d,row=%s", ret, to_cstring(row_to_store_ins));
                  }
                  else
                  {
                    //add maoxx test
                    TBSYS_LOG(ERROR,"test::maoxx row_to_store_ins=%s", to_cstring(row_to_store_ins));
                    //add e
                    row_to_store_ins.clear();
                  }
                }
              }
              if(post_break)
              {
                break;
              }
            }//end while
            if(pre_break)
            {
              break;
            }
          }//end while
        }
      //modify e
      }
      if(NULL != child_op_)
      {
        reset_iterator();
      }
      if(OB_SUCCESS == ret)
      {
        if(DELETE == sql_type_ || (UPDATE == sql_type_ && delete_flag_for_update_) || (REPLACE == sql_type_ && delete_flag_for_replace_))
        {
          ObRowCellIterAdaptor cia;
          ObDmlType dml_type = OB_DML_DELETE;
          cia.set_row_iter(&index_row_del, index_row_desc_del_[index_idx].get_rowkey_cell_count(), schema_mgr, index_row_desc_del_[index_idx]);
          ret = host->apply(session_ctx, cia, dml_type);
        }
        if(INSERT == sql_type_ || UPDATE == sql_type_ || REPLACE == sql_type_)
        {
          ObRowCellIterAdaptor cia;
          ObDmlType dml_type = OB_DML_INSERT;
          cia.set_row_iter(&index_row_ins, index_row_desc_ins_[index_idx].get_rowkey_cell_count(), schema_mgr, index_row_desc_ins_[index_idx]);
          ret = host->apply(session_ctx, cia, dml_type);
        }
      }
      index_row_del.clear();
      index_row_ins.clear();
      return ret;
    }

    void ObIndexTrigger::reset()
    {
      data_tid_ = OB_INVALID_ID;
      need_modify_index_num_ = 0;
      //add huangjianwei [secondary index maintain] 20160909:b
      replace_values_id_ = OB_INVALID_ID;
      replace_values_.reset();
      //add:e
      //data_row_desc_.reset();
      pre_data_row_desc_.reset();
      post_data_row_desc_.reset();
      pre_data_row_store_.clear();
      post_data_row_store_.clear();
      for(int64_t i = 0;i < OB_MAX_INDEX_NUMS; i++)
      {
        index_row_desc_del_[i].reset();
        index_row_desc_ins_[i].reset();
      }
      cond_flag_ = false;
      delete_flag_for_update_ = true;
      delete_flag_for_replace_ = false;
      ObSingleChildPhyOperator::reset();
    }

    void ObIndexTrigger::reuse()
    {
      data_tid_ = OB_INVALID_ID;
      need_modify_index_num_ = 0;
      //add huangjianwei [secondary index maintain] 20160909:b
      replace_values_id_ = OB_INVALID_ID;
      replace_values_.reuse();
      //add:e
      //data_row_desc_.reset();
      pre_data_row_desc_.reset();
      post_data_row_desc_.reset();
      pre_data_row_store_.clear();
      post_data_row_store_.clear();
      for(int64_t i = 0;i < OB_MAX_INDEX_NUMS; i++)
      {
        index_row_desc_del_[i].reset();
        index_row_desc_ins_[i].reset();
      }
      cond_flag_ = false;
      delete_flag_for_update_ = true;
      delete_flag_for_replace_ = false;
      ObSingleChildPhyOperator::reuse();
    }

    int64_t ObIndexTrigger::to_string(char* buf, const int64_t buf_len) const
    {
      int64_t pos = 0;
      if(INSERT == sql_type_)
        databuff_printf(buf, buf_len, pos, "Sql_Type: Insert");
      else if(DELETE == sql_type_)
        databuff_printf(buf, buf_len, pos, "Sql_Type: Delete");
      else if(UPDATE == sql_type_)
        databuff_printf(buf, buf_len, pos, "Sql_Type: Update");
      else if(REPLACE == sql_type_)
        databuff_printf(buf, buf_len, pos, "Sql_Type: Replace");
      databuff_printf(buf, buf_len, pos, "\n");
      databuff_printf(buf, buf_len, pos, "ObIndexTrigger: index_num=[%ld],main_tid=[%lu],", need_modify_index_num_, data_tid_);
      databuff_printf(buf, buf_len, pos, "index delete row desc[");
      int64_t i = 0;
      int64_t cur_pos = 0;
      for (i = 0; i < need_modify_index_num_ - 1; i++)
      {
        cur_pos = index_row_desc_del_[i].to_string(buf + pos, buf_len - pos);
        pos += cur_pos;
        databuff_printf(buf, buf_len, pos, ",");
      }
      cur_pos = index_row_desc_del_[i].to_string(buf + pos, buf_len - pos);
      pos += cur_pos;
      databuff_printf(buf, buf_len, pos, "],index insert row desc[");
      for (i = 0; i < need_modify_index_num_ - 1; i++)
      {
        cur_pos = index_row_desc_ins_[i].to_string(buf + pos, buf_len - pos);
        pos += cur_pos;
        databuff_printf(buf, buf_len, pos, ",");
      }
      cur_pos = index_row_desc_ins_[i].to_string(buf + pos, buf_len - pos);
      pos += cur_pos;

      //add huangjianwei [secondary index maintain] 20160909:b
      databuff_printf(buf, buf_len, pos, "],replace_values_id=[%lu],values=", replace_values_id_);
      pos += replace_values_.to_string(buf + pos, buf_len - pos);
      //add:e

      if (NULL != child_op_)
      {
        cur_pos = child_op_->to_string(buf + pos, buf_len - pos);
        pos += cur_pos;
      }

      return pos;
    }

    ObPhyOperatorType ObIndexTrigger::get_type() const
    {
      return PHY_INDEX_TRIGGER;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObIndexTrigger)
    {
      int64_t size = 0;
      size += serialization::encoded_length_i64(static_cast<int64_t>(sql_type_));
      size += serialization::encoded_length_i64(data_tid_);
      size += serialization::encoded_length_i64(need_modify_index_num_);
      size += static_cast<int64_t>(sizeof(int64_t));
      size += static_cast<int64_t>(sizeof(int64_t));
      size += static_cast<int64_t>(sizeof(int64_t));
      //size += data_row_desc_.get_serialize_size();
      size += pre_data_row_desc_.get_serialize_size();
      size += post_data_row_desc_.get_serialize_size();
      size += pre_data_row_store_.get_serialize_size();
      size += post_data_row_store_.get_serialize_size();
      for(int64_t i = 0; i < need_modify_index_num_; i++)
      {
        size += index_row_desc_del_[i].get_serialize_size();
        size += index_row_desc_ins_[i].get_serialize_size();
      }
      //add huangjianwei [secondary index maintain] 20160909:b
      if(REPLACE== sql_type_)
      {
        size += replace_values_.get_serialize_size();
        //size += static_cast<int64_t>(sizeof(int64_t));
      }
      //add:e
      /*for(int64_t j = 0; j < set_index_column_.count(); j++)
        {
          const ObSqlExpression &expr = set_index_column_.at(j);
          size += expr.get_serialize_size();
        }
          size += static_cast<int64_t>(sizeof(int64_t));
            for(int64_t k = 0; k < set_cast_obj_.count(); k++)
            {
              const ObObj &obj = set_cast_obj_.at(k);
              size += obj.get_serialize_size();
            }*/
      return size;
    }

    DEFINE_SERIALIZE(ObIndexTrigger)
    {
      int ret = OB_SUCCESS;
      if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, (int64_t)sql_type_)))
      {
        TBSYS_LOG(WARN,"failed to encode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, data_tid_)))
      {
        TBSYS_LOG(WARN,"failed to encode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, need_modify_index_num_)))
      {
        TBSYS_LOG(WARN,"failed to encode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, (int64_t)cond_flag_)))
      {
        TBSYS_LOG(WARN,"failed to encode has_other_cond_");
      }
      else if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, (int64_t)delete_flag_for_update_)))
      {
        TBSYS_LOG(WARN,"failed to encode has_other_cond_");
      }
      else if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, (int64_t)delete_flag_for_replace_)))
      {
        TBSYS_LOG(WARN,"failed to encode has_other_cond_");
      }
      /*else if (OB_SUCCESS != (ret = data_row_desc_.serialize(buf, buf_len, pos)))
            {
              TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
            }*/
      else if (OB_SUCCESS != (ret = pre_data_row_desc_.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = post_data_row_desc_.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = pre_data_row_store_.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = post_data_row_store_.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
      }
      else
      {
        for(int64_t i = 0; i < need_modify_index_num_; i++)
        {
          if (OB_SUCCESS == ret && (OB_SUCCESS != (ret = index_row_desc_del_[i].serialize(buf, buf_len, pos))))
          {
            TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
            break;
          }
          if (OB_SUCCESS == ret && (OB_SUCCESS != (ret = index_row_desc_ins_[i].serialize(buf, buf_len, pos))))
          {
            TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
            break;
          }
        }
      }
      //add huangjianwei [secondary index maintain] 20160909:b
      if(REPLACE == sql_type_)
            {
                ObExprValues *replace_values = NULL;
                if (OB_SUCCESS == ret)
                {
                  if (NULL == (replace_values = static_cast<ObExprValues*>(my_phy_plan_->get_phy_query_by_id(replace_values_id_))))
                  {
                    TBSYS_LOG(ERROR, "invalid expr_values, subquery_id=%lu", replace_values_id_);
                  }
                  else if (OB_SUCCESS != (ret = replace_values->serialize(buf, buf_len, pos)))
                  {
                    TBSYS_LOG(WARN, "fail to serialize expr_values. ret=%d", ret);
                  }
                }
            }
      //add:e
           /*if(OB_SUCCESS == ret)
            {
              int64_t set_index_column_count = 0;
              set_index_column_count = set_index_column_.count();
              if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, set_index_column_count)))
              {
                TBSYS_LOG(WARN,"failed to encode ,ret[%d]",ret);
              }
              else
              {
                for(int64_t j = 0; j < set_index_column_count; j++)
                {
                  const ObSqlExpression &expr = set_index_column_.at(j);
                  if (OB_SUCCESS == ret && (OB_SUCCESS != (ret = expr.serialize(buf, buf_len, pos))))
                  {
                    TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
                    break;
                  }
                }
              }
            }
            if(OB_SUCCESS == ret)
            {
              int64_t set_cast_obj_count = 0;
              set_cast_obj_count = set_cast_obj_.count();
              if(OB_SUCCESS != (ret = serialization::encode_i64(buf, buf_len, pos, set_cast_obj_count)))
              {
                TBSYS_LOG(WARN,"failed to encode ,ret[%d]",ret);
              }
              else
              {
                for(int64_t k = 0; k < set_cast_obj_count; k++)
                {
                  const ObObj &obj = set_cast_obj_.at(k);
                  if(OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
                  {
                    TBSYS_LOG(WARN, "serialize fail. ret=%d", ret);
                    break;
                  }
                }
              }
            }*/
      return ret;
    }

    DEFINE_DESERIALIZE(ObIndexTrigger)
    {
      int ret = OB_SUCCESS;
      int64_t sql_type;
      int64_t cond_flag = 0;
      int64_t delete_flag_for_update = 0;
      int64_t delete_flag_for_replace = 0;
      if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &sql_type)))
      {
        TBSYS_LOG(WARN,"failed to decode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &data_tid_)))
      {
        TBSYS_LOG(WARN,"failed to decode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &need_modify_index_num_)))
      {
        TBSYS_LOG(WARN,"failed to decode index_num_,ret[%d]",ret);
      }
      else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &cond_flag)))
      {
        TBSYS_LOG(WARN,"failed to decode has_other_cond_");
      }
      else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &delete_flag_for_update)))
      {
        TBSYS_LOG(WARN,"failed to decode has_other_cond_");
      }
      else if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &delete_flag_for_replace)))
      {
        TBSYS_LOG(WARN,"failed to decode has_other_cond_");
      }
      /*else if (OB_SUCCESS != (ret = data_row_desc_.deserialize(buf, data_len, pos)))
            {
              TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
            }*/
      else if (OB_SUCCESS != (ret = pre_data_row_desc_.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = post_data_row_desc_.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = pre_data_row_store_.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = post_data_row_store_.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
      }
      else
      {
        for(int64_t i = 0; i < need_modify_index_num_; i++)
        {
          if (OB_SUCCESS == ret && (OB_SUCCESS != (ret = index_row_desc_del_[i].deserialize(buf, data_len, pos))))
          {
            TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
            break;
          }
          if (OB_SUCCESS == ret && (OB_SUCCESS != (ret = index_row_desc_ins_[i].deserialize(buf, data_len, pos))))
          {
            TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
            break;
          }
        }
      }
      /*if(OB_SUCCESS == ret)
            {
              int64_t set_index_column_count = 0;
              ObSqlExpression expr;
              if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &set_index_column_count)))
              {
                TBSYS_LOG(WARN,"failed to decode ,ret[%d]",ret);
              }
              else
              {
                for(int64_t j = 0; j < set_index_column_count; j++)
                {
                  if (OB_SUCCESS != (ret = add_set_index_column(expr)))
                  {
                    TBSYS_LOG(DEBUG, "fail to add expr to project ret=%d. buf=%p, data_len=%ld, pos=%ld", ret, buf, data_len, pos);
                    break;
                  }
                  if (OB_SUCCESS != (ret = set_index_column_.at(j).deserialize(buf, data_len, pos)))
                  {
                    TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
                    break;
                  }
                }
              }
            }
            if(OB_SUCCESS == ret)
            {
              int64_t set_cast_obj_count = 0;
              ObObj obj;
              if(OB_SUCCESS != (ret = serialization::decode_i64(buf, data_len, pos, &set_cast_obj_count)))
              {
                TBSYS_LOG(WARN,"failed to decode ,ret[%d]",ret);
              }
              else
              {
                for(int64_t k = 0; k < set_cast_obj_count; k++)
                {
                  if (OB_SUCCESS != (ret = add_set_cast_obj(obj)))
                  {
                      TBSYS_LOG(WARN, "failed to add cast obj to project ret = %d",ret);
                      break;
                  }
                  if(OB_SUCCESS != (ret = set_cast_obj_.at(set_cast_obj_.count()-1).deserialize(buf, data_len, pos)))
                  {
                    TBSYS_LOG(WARN, "deserialize fail. ret=%d", ret);
                    break;
                  }
                }
              }
            }*/
      if(OB_SUCCESS == ret)
      {
        //mod huangjianwei [secondary index maintain] 20160909:b
        //sql_type_ = (int)sql_type;
        sql_type_ = (SQLTYPE)sql_type;
        //mod:e
        cond_flag_ = cond_flag == 0 ? false : true;
        delete_flag_for_update_ = delete_flag_for_update == 0 ? false : true;
        delete_flag_for_replace_ = delete_flag_for_replace == 0 ? false : true;
      }
      //add huangjianwei [secondary index maintain] 20160909:b
      if(REPLACE == sql_type_)
      {
        if (OB_SUCCESS == ret && OB_SUCCESS != (ret = replace_values_.deserialize(buf, data_len, pos)))
          {
             TBSYS_LOG(WARN, "fail to deserialize replace values. ret=%d", ret);
          }
      }
      //add:e
      return ret;
    }

    PHY_OPERATOR_ASSIGN(ObIndexTrigger)
    {
      int ret = OB_SUCCESS;
      CAST_TO_INHERITANCE(ObIndexTrigger);
      reset();
      sql_type_ = o_ptr->sql_type_;
      data_tid_ = o_ptr->data_tid_;
      need_modify_index_num_ = o_ptr->need_modify_index_num_;
      //data_row_desc_= o_ptr->data_row_desc_;
      pre_data_row_desc_= o_ptr->pre_data_row_desc_;
      post_data_row_desc_= o_ptr->post_data_row_desc_;
      for(int64_t i = 0; i < o_ptr->need_modify_index_num_; i++)
      {
        index_row_desc_del_[i] = o_ptr->index_row_desc_del_[i];
        index_row_desc_ins_[i] = o_ptr->index_row_desc_ins_[i];
      }
      //add huangjianwei [secondary index maintain] 20160909:b
      if(REPLACE == sql_type_)
      {
        replace_values_id_ = o_ptr->replace_values_id_;
      }
          /*for(int64_t j = 0; j < o_ptr->set_index_column_.count(); j++)
          {
            if (ret != OB_SUCCESS)
                break;
            if (OB_SUCCESS != (ret = set_index_column_.push_back(o_ptr->set_index_column_.at(j))))
            {
              set_index_column_.at(j).set_owner_op(this);
            }
            else
            {
              break;
            }
          }*/
      return ret;
    }
  }
}
