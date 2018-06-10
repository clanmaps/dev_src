/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_iterator_adaptor.cpp
* @brief for iteration
*
* modified by maoxiaoxiao:add class to iterate through rows in a row store
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

////===================================================================
 //
 // ob_iterator_adaptor.cpp common / Oceanbase
 //
 // Copyright (C) 2010, 2013 Taobao.com, Inc.
 //
 // Created on 2012-11-21 by Yubai (yubai.lk@taobao.com)
 //
 // -------------------------------------------------------------------
 //
 // Description
 //
 // -------------------------------------------------------------------
 //
 // Change Log
 //
////====================================================================
#include "ob_iterator_adaptor.h"
#include "ob_new_scanner_helper.h"
#include "ob_schema.h"
#include "ob_obj_cast.h"
#include "utility.h"


namespace oceanbase{
  namespace common{
    REGISTER_CREATOR(oceanbase::sql::ObPhyOperatorGFactory, oceanbase::sql::ObPhyOperator, ObRowIterAdaptor, oceanbase::sql::PHY_ROW_ITER_ADAPTOR);
  }
}

namespace oceanbase
{
  namespace common
  {
    ObObjCastHelper::ObObjCastHelper() : need_cast_(false),
                                         col_num_(0)
    {
    }

    ObObjCastHelper::~ObObjCastHelper()
    {
    }

    void ObObjCastHelper::set_need_cast(const bool need_cast)
    {
      need_cast_ = need_cast;
    }

    int ObObjCastHelper::reset(const ObRowDesc &row_desc, const ObSchemaManagerV2 &schema_mgr)
    {
      int ret = OB_SUCCESS;
      need_cast_ = true;
      col_num_ = 0;
      if (OB_ROW_MAX_COLUMNS_COUNT < row_desc.get_column_num())
      {
        TBSYS_LOG(WARN, "row_desc too long %ld", row_desc.get_column_num());
        ret = OB_SIZE_OVERFLOW;
      }
      else
      {
        int64_t i = 0;
        uint64_t tid = OB_INVALID_ID;
        uint64_t cid = OB_INVALID_ID;
        const ObColumnSchemaV2 *col_schema = NULL;
        uint8_t type = (uint8_t)ObMinType;
        while (i < row_desc.get_column_num())
        {
          if (OB_SUCCESS != (ret = row_desc.get_tid_cid(i, tid, cid)))
          {
            TBSYS_LOG(WARN, "get_tid_cid from row_desc fail ret=%d idx=%ld", ret, i);
            break;
          }
          if (OB_ACTION_FLAG_COLUMN_ID == cid)
          {
            type = (uint8_t)ObMinType;
          }
          else if (NULL == (col_schema = schema_mgr.get_column_schema(tid, cid)))
          {
            if (cid < (uint64_t)OB_APP_MIN_COLUMN_ID)
            {
              type = (uint8_t)ObMinType;
            }
            else
            {
              TBSYS_LOG(WARN, "get_column_schema fail tid=%lu cid=%lu", tid, cid);
              ret = OB_ENTRY_NOT_EXIST;
              break;
            }
          }
          else
          {
            type = (uint8_t)col_schema->get_type();
          }
          col_types_[i++].type_ = type;
          //add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
          /*
           *该函数主要做的是把col_schema里面的值存到col_types_数组里面。这里col_schema是从schema_mgr里面获得的用户
           *定义的表的schema。col_types_数组会在后面做类型转化的时候用到。
           *
           *OB原先的实现只是把该列的类型存到col_types_数组里面。我增加的代码原理是先判断该列是否为decimal类型。如果是的话，
           *将该列的precision和scale也存到col_types_数组里面
           */
          int64_t middle_i=i-1;
          if(type==(uint8_t)ObDecimalType)
            {
             col_types_[middle_i].dec_precision_ = static_cast<uint8_t>(col_schema->get_precision()) & META_PREC_MASK;
             col_types_[middle_i].dec_scale_ = static_cast<uint8_t>(col_schema->get_scale()) & META_SCALE_MASK;
             //TBSYS_LOG(ERROR, "test::whx dec_precision = [%d], dec_scale_ = [%d]", col_types_[middle_i].dec_precision_ ,col_types_[middle_i].dec_scale_);
          }
           //add:e
          TBSYS_LOG(DEBUG, "set col type idx=%ld tid=%lu cid=%lu type=%hhu", i - 1, tid, cid, col_types_[i - 1].type_);
        }
        if (OB_SUCCESS == ret)
        {
          col_num_ = i;
        }
      }
      return ret;
    }

    int ObObjCastHelper::cast_cell(const int64_t idx, ObObj &cell) const
    {
      int ret = OB_SUCCESS;
      if (!need_cast_)
      {
        // do nothing
      }
      else if (col_num_ <= idx
              || 0 > idx)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if ((uint8_t)ObMinType == col_types_[idx].type_)
      {
        // need not cast
      }
      else
      {
        ObString cast_buffer = get_tsi_buffer_();
        ret = obj_cast(cell, (ObObjType)col_types_[idx].type_, cast_buffer);
      }
      return ret;
    }
    //add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
    /*
     *接口函数，用来获得col_types_数组里的precision，scale。idx是数组下标，col_types_数组里面存的是多个列的schema.
     *这2个接口只在ObCellAdaptor::next_cell()里面调用。
     */
    uint32_t ObObjCastHelper::get_precision(const int64_t idx)
     {
            return col_types_[idx].dec_precision_;
     }
    uint32_t ObObjCastHelper::get_scale(const int64_t idx)
     {
            return col_types_[idx].dec_scale_;
     }
    //add:e
    int ObObjCastHelper::cast_rowkey_cell(const int64_t idx, ObObj &cell) const
    {
      int ret = OB_SUCCESS;
      if (!need_cast_)
      {
        // do nothing
      }
      else if (col_num_ <= idx
              || 0 > idx)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else if ((uint8_t)ObMinType == col_types_[idx].type_)
      {
        // need not cast
      }
      else
      {
        ObString cast_buffer = get_tsi_buffer_(idx);
        ret = obj_cast(cell, (ObObjType)col_types_[idx].type_, cast_buffer);
      }
      return ret;
    }

    ObString ObObjCastHelper::get_tsi_buffer_()
    {
      ObString ret;
      static __thread char BUFFER[OB_MAX_VARCHAR_LENGTH];
      ret.assign_ptr(BUFFER, OB_MAX_VARCHAR_LENGTH);
      return ret;
    }

    ObString ObObjCastHelper::get_tsi_buffer_(const int64_t idx)
    {
      ObString ret;
      static __thread char BUFFER[OB_MAX_ROWKEY_COLUMN_NUMBER][OB_MAX_VARCHAR_LENGTH];
      if (OB_MAX_ROWKEY_COLUMN_NUMBER > idx)
      {
        ret.assign_ptr(BUFFER[idx], OB_MAX_VARCHAR_LENGTH);
      }
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    int ObRowkeyCastHelper::cast_rowkey(const ObRowkeyInfo &rki, ObRowkey &rowkey)
    {
      int ret = OB_SUCCESS;
      if (NULL == rowkey.ptr()
          || rki.get_size() != rowkey.get_obj_cnt())
      {
        TBSYS_LOG(WARN, "invalid param rowkey_ptr=%p rki_size=%ld rowkey_obj_cnt=%ld",
                  rowkey.ptr(), rki.get_size(), rowkey.get_obj_cnt());
        ret = OB_INVALID_ARGUMENT;
      }
      TBSYS_LOG(DEBUG, "before cast rowkey %s", to_cstring(rowkey));
      for (int64_t i = 0; OB_SUCCESS == ret && i < rowkey.get_obj_cnt(); i++)
      {
        ObString cast_buffer = get_tsi_buffer_(i);
        const ObRowkeyColumn *rc = rki.get_column(i);
        if (NULL == rc)
        {
          TBSYS_LOG(WARN, "get rowkey column schema fail idx=%ld", i);
          ret = OB_ERR_UNEXPECTED;
        }
        else if (OB_SUCCESS != (ret = obj_cast(const_cast<ObObj&>(rowkey.ptr()[i]), rc->type_, cast_buffer)))
        {
          TBSYS_LOG(WARN, "cast rowkey cell fail idx=%ld %s", i, to_cstring(rowkey.ptr()[i]));
          break;
        }
        else
        {
          TBSYS_LOG(DEBUG, "cast rowkey cell succ idx=%ld %s", i, to_cstring(rowkey.ptr()[i]));
        }
      }
      return ret;
    }

    ObString ObRowkeyCastHelper::get_tsi_buffer_(const int64_t idx)
    {
      ObString ret;
      static __thread char BUFFER[OB_MAX_ROWKEY_COLUMN_NUMBER][OB_MAX_VARCHAR_LENGTH];
      if (OB_MAX_ROWKEY_COLUMN_NUMBER > idx)
      {
        ret.assign_ptr(BUFFER[idx], OB_MAX_VARCHAR_LENGTH);
      }
      return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ObCellAdaptor::ObCellAdaptor() : row_(NULL),
                                     rk_size_(0),
                                     cur_idx_(0),
                                     is_iter_end_(false),
                                     cell_(),
                                     need_nop_cell_(false),
                                     och_()
    {
    }

    ObCellAdaptor::~ObCellAdaptor()
    {
    }

    int ObCellAdaptor::next_cell()
    {
      int ret = OB_SUCCESS;
      const ObObj *value = NULL;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (cur_idx_ >= row_->get_column_num())
      {
        if (!need_nop_cell_)
        {
          ret = OB_ITER_END;
        }
        else
        {
          need_nop_cell_ = false;
          cell_.column_id_ = OB_INVALID_ID;
          cell_.value_.set_ext(ObActionFlag::OP_NOP);
        }
      }
      else if (OB_SUCCESS != (ret = row_->raw_get_cell(cur_idx_, value, cell_.table_id_, cell_.column_id_))
              || NULL == value)
      {
        ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
      }
      else
      {
        TBSYS_LOG(DEBUG, "CELL_ADAPTOR idx=%ld %s", cur_idx_, print_cellinfo(&cell_));
        cell_.value_ = *value;
        //TBSYS_LOG(INFO, "xushilei,test ipdate value=[%s],p=[%d],s=[%d],pp=[%d],ss=[%d],vs=[%d]", to_cstring(ob),ob.get_precision(),ob.get_scale(),dec.get_precision(),dec.get_scale(),dec.get_vscale());
        //test e 30 10 38 0 0
        if (OB_ACTION_FLAG_COLUMN_ID == cell_.column_id_)
        {
          cell_.value_.set_ext(ObActionFlag::OP_DEL_ROW);
        }
        else if (OB_SUCCESS != (ret = och_.cast_cell(cur_idx_, cell_.value_)))
        {
          TBSYS_LOG(WARN, "obj_cast fail ret=%d cur_idx=%ld %s", ret, cur_idx_, to_cstring(cell_.value_));
        }
        if (ObExtendType == cell_.value_.get_type())
        {
          cell_.column_id_ = OB_INVALID_ID;
        }
        //modify xsl ECNU_DECIAML 2017_2
        //add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        /*
         *cell_是做完类型转化后的结果。如果该列的类型是decimal，就把och_里面存的该列的precision和scale也写到cell_里面
         *
         *modify_value是对转化后的值做一次截取。
         */
        if (ObDecimalType == cell_.value_.get_type())
         {
           uint32_t p=och_.get_precision(cur_idx_);
           uint32_t s=och_.get_scale(cur_idx_);
           uint64_t *t1 =NULL;
           //ObString os;

           if(NULL == (t1 = cell_.value_.get_ttint()))
           {
                TBSYS_LOG(ERROR, "failed to do get_decimal() in ObCellAdaptor::next_cell");
           }
           else
           {
               //add xsl DECIMAL
               uint32_t tmp_vs = cell_.value_.get_vscale();
               ObDecimal tmp_dec;
               cell_.value_.get_decimal(tmp_dec);
               TTInt *word = tmp_dec.get_words();
               TTInt whole;
               TTInt BASE(10),pp(tmp_vs);
               BASE.Pow(pp);
               whole = word[0] / BASE;
               if(word->IsSign())
               {
                   whole.ChangeSign();
               }
               std::string str_for_int;
               whole.ToString(str_for_int);
               int len_int = (int)str_for_int.length();
               if( (uint32_t)len_int > (p - s))
               //add e
               //if((p-s) < (cell_.value_.get_precision()- cell_.value_.get_scale()))    //modify xsl ECNU_DECIMAL 2017_1
               {
                   ret=OB_DECIMAL_UNLEGAL_ERROR;
                   TBSYS_LOG(ERROR, "OB_DECIMAL_UNLEGAL_ERROR,p=%d,s=%d,od.get_precision()=%d,od.get_vscale()=%d",p,s,cell_.value_.get_precision(),cell_.value_.get_precision());
               }
               else
               {
                   cell_.value_.set_precision(p);
                   cell_.value_.set_scale(s);
               }
               //}
           }
         }
        //modify e
        if (OB_SUCCESS == ret)
        {
          cur_idx_ += 1;
        }
      }
      is_iter_end_ = (OB_SUCCESS != ret);
      return ret;
    }

    int ObCellAdaptor::get_cell(ObCellInfo** cell)
    {
      return get_cell(cell, NULL);
    }

    int ObCellAdaptor::get_cell(ObCellInfo** cell, bool* is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (NULL == cell)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        *cell = &cell_;
        if (NULL != is_row_changed)
        {
          *is_row_changed = (cur_idx_ <= (rk_size_ + 1));
        }
      }
      return ret;
    }

    int ObCellAdaptor::is_row_finished(bool* is_row_finished)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        if (NULL != is_row_finished)
        {
          if (cur_idx_ == row_->get_column_num()
              && !need_nop_cell_)
          {
            *is_row_finished = true;
          }
          else
          {
            *is_row_finished = false;
          }
        }
      }
      return ret;
    }

    void ObCellAdaptor::set_row(const ObRow *row, const int64_t rk_size)
    {
      row_ = NULL;
      rk_size_ = 0;
      cur_idx_ = 0;
      is_iter_end_ = false;
      cell_.reset();
      need_nop_cell_ = false;
      if (NULL != row
          && 0 < rk_size)
      {
        const ObObj *rk_values = NULL;
        uint64_t table_id = OB_INVALID_ID;
        uint64_t column_id = OB_INVALID_ID;
        if (rk_size <= row->get_column_num()
            && OB_SUCCESS == row->raw_get_cell(0, rk_values, table_id, column_id)
            && NULL != rk_values
            && OB_SUCCESS == cast_rowkey_(*row, rk_size))
        {
          TBSYS_LOG(DEBUG, "set_row succ rk_size=%ld col_num=%ld", rk_size, row->get_column_num());
          row_ = row;
          rk_size_ = rk_size;
          cur_idx_ = rk_size;
          cell_.row_key_.assign(const_cast<ObObj*>(rk_values), rk_size);
          cell_.table_id_ = table_id;
          cell_.column_id_ = column_id;
          need_nop_cell_ = (rk_size == row->get_column_num());
        }
        else
        {
          TBSYS_LOG(WARN, "set_row fail rk_size=%ld col_num=%ld", rk_size, row->get_column_num());
        }
      }
    }

    int ObCellAdaptor::cast_rowkey_(const ObRow &row, const int64_t rk_size)
    {
      int ret = OB_SUCCESS;
      for (int64_t i = 0; i < rk_size; i++)
      {
        const ObObj *value = NULL;
        uint64_t table_id = OB_INVALID_ID;
        uint64_t column_id = OB_INVALID_ID;
        if (OB_SUCCESS != (ret = row.raw_get_cell(i, value, table_id, column_id))
            || NULL == value)
        {
          TBSYS_LOG(WARN, "get_cell from row fail ret=%d idx=%ld", ret, i);
          ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
          break;
        }
        ObObj &casted_value = const_cast<ObObj&>(*value);
        if (OB_SUCCESS != (ret = och_.cast_rowkey_cell(i, casted_value)))
        {
          TBSYS_LOG(WARN, "obj_cast fail ret=%d idx=%ld %s", ret, i, to_cstring(casted_value));
          break;
        }
      }
      return ret;
    }

    void ObCellAdaptor::reset()
    {
      row_ = NULL;
      rk_size_ = 0;
      cur_idx_ = 0;
      is_iter_end_ = false;
      cell_.reset();
      need_nop_cell_ = false;
    }

    ObObjCastHelper &ObCellAdaptor::get_och()
    {
      return och_;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ObCellIterAdaptor::ObCellIterAdaptor() : row_iter_(NULL),
                                             rk_size_(0),
                                             single_row_iter_(),
                                             is_iter_end_(false),
                                             set_row_iter_ret_(OB_SUCCESS)
                                             //add lbzhong [auto_increment] 20161127:b
                                             , updated_auto_value_(OB_INVALID_AUTO_INCREMENT_VALUE)
                                             //add:e
    {
    }

    ObCellIterAdaptor::~ObCellIterAdaptor()
    {
    }

    int ObCellIterAdaptor::next_cell()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != set_row_iter_ret_)
      {
        ret = set_row_iter_ret_;
      }
      else if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = single_row_iter_.next_cell();
        if (OB_ITER_END == ret)
        {
          const ObRow *row = NULL;
          if (OB_SUCCESS != (ret = row_iter_->get_next_row(row))
              || NULL == row)
          {
            ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
          }
          else
          {
            single_row_iter_.set_row(row, rk_size_);
            ret = single_row_iter_.next_cell();
          }
        }
      }
      is_iter_end_ = (OB_SUCCESS != ret);
      return ret;
    }

    int ObCellIterAdaptor::get_cell(ObCellInfo** cell)
    {
      return get_cell(cell, NULL);
    }

    int ObCellIterAdaptor::get_cell(ObCellInfo** cell, bool* is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (NULL == cell)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        ret = single_row_iter_.get_cell(cell, is_row_changed);
      }
      return ret;
    }

    int ObCellIterAdaptor::is_row_finished(bool* is_row_finished)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = single_row_iter_.is_row_finished(is_row_finished);
      }
      return ret;
    }

    void ObCellIterAdaptor::set_row_iter(sql::ObPhyOperator *row_iter, const int64_t rk_size, const ObSchemaManagerV2 *schema_mgr
                                         //add lbzhong [auto_increment] 20161127:b
                                         , const bool is_update_auto_value /* = false */
                                         //add:e
                                         )
    {
      row_iter_ = NULL;
      rk_size_ = 0;
      single_row_iter_.reset();
      is_iter_end_ = false;
      if (NULL != row_iter
          && 0 < rk_size)
      {
        int tmp_ret = OB_SUCCESS;
        if (NULL != schema_mgr)
        {
          const ObRowDesc *row_desc = NULL;
          if (OB_SUCCESS != (tmp_ret = row_iter->get_row_desc(row_desc))
              || NULL == row_desc)
          {
            TBSYS_LOG(WARN, "get_row_desc fail ret=%d phy_op=%p phy_type=%d", tmp_ret, row_iter, row_iter->get_type());
            tmp_ret = (OB_SUCCESS == tmp_ret) ? OB_ERROR : tmp_ret;
          }
          else
          {
            tmp_ret = single_row_iter_.get_och().reset(*row_desc, *schema_mgr);
          }
        }
        else
        {
          single_row_iter_.get_och().set_need_cast(false);
        }
        const ObRow *row = NULL;
        if (OB_SUCCESS == tmp_ret)
        {
          tmp_ret = row_iter->get_next_row(row);
          TBSYS_LOG(DEBUG, "ITER_ADAPTOR ret=%d op=%p type=%d %s",
                    tmp_ret, row_iter, row_iter->get_type(), (NULL == row) ? "nil" : to_cstring(*row));
          if ((OB_SUCCESS == tmp_ret && NULL != row)
              || OB_ITER_END == tmp_ret)
          {
            row_iter_ = row_iter;
            rk_size_ = rk_size;
            if (OB_ITER_END == tmp_ret)
            {
              is_iter_end_ = true;
            }
            else
            {
              //add lbzhong [auto_increment] 20161124:b
              if (is_update_auto_value)
              {
                tmp_ret = set_updated_auto_value(row);
              }
              //add:e
              single_row_iter_.set_row(row, rk_size);
            }
          }
        }
        set_row_iter_ret_ = tmp_ret;
      }
    }

    void ObCellIterAdaptor::reset()
    {
      row_iter_ = NULL;
      rk_size_ = 0;
      single_row_iter_.reset();
      is_iter_end_ = false;
      //add lbzhong [auto_increment] 20161127:b
      updated_auto_value_ = OB_INVALID_AUTO_INCREMENT_VALUE;
      //add:e
    }

    //add lbzhong [auto_increment] 20161127:b
    int ObCellIterAdaptor::set_updated_auto_value(const ObRow *&row)
    {
      int ret = OB_SUCCESS;
      const ObObj *value = NULL;
      uint64_t table_id = OB_INVALID_ID;
      uint64_t column_id = OB_INVALID_ID;
      if (OB_SUCCESS != (ret = row->raw_get_cell(2, value, table_id, column_id))
          || NULL == value)
      {
        TBSYS_LOG(WARN, "fail to get updated auto value, ret=%d", ret);
      }
      else
      {
        value->get_int(updated_auto_value_);
      }
      return ret;
    }

    int64_t ObCellIterAdaptor::get_updated_auto_value()
    {
      return updated_auto_value_;
    }

    //add:e

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ObRowIterAdaptor::ObRowIterAdaptor() : mod_(ObModIds::OB_ROW_ITER_ADAPTOR),
                                           allocator_(ALLOCATOR_PAGE_SIZE, mod_),
                                           cell_iter_(NULL),
                                           cur_row_(),
                                           is_ups_row_(true)
    {
    }

    ObRowIterAdaptor::ObRowIterAdaptor(bool is_ups_row) : mod_(ObModIds::OB_ROW_ITER_ADAPTOR),
                                           allocator_(ALLOCATOR_PAGE_SIZE, mod_),
                                           cell_iter_(NULL),
                                           cur_row_(),
                                           is_ups_row_(is_ups_row)
    {
    }

    ObRowIterAdaptor::~ObRowIterAdaptor()
    {
    }

    int ObRowIterAdaptor::set_child(int32_t child_idx, ObPhyOperator &child_operator)
    {
      UNUSED(child_idx);
      UNUSED(child_operator);
      return OB_NOT_SUPPORTED;
    }

    int ObRowIterAdaptor::open()
    {
      int ret = OB_SUCCESS;
      // do nothing
      return ret;
    }

    int ObRowIterAdaptor::close()
    {
      int ret = OB_SUCCESS;
      // do nothing
      return ret;
    }

    int ObRowIterAdaptor::get_next_row(const ObRow *&row)
    {
      const ObRowkey *rowkey = NULL;
      return get_next_row(rowkey, row);
    }

    int ObRowIterAdaptor::get_next_row(const ObRowkey *&rowkey, const ObRow *&row)
    {
      int ret = OB_SUCCESS;
      if (NULL == cell_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else
      {
        int64_t cell_counter = 0;
        cur_row_.reset(false, is_ups_row_ ? ObRow::DEFAULT_NOP : ObRow::DEFAULT_NULL);
        allocator_.reuse();
        bool is_row_finished = false;
        while (OB_SUCCESS == (ret = cell_iter_->next_cell()))
        {
          ObCellInfo *ci = NULL;
          if (OB_SUCCESS != (ret = cell_iter_->get_cell(&ci))
              || NULL == ci
              || OB_SUCCESS != (ret = cell_iter_->is_row_finished(&is_row_finished)))
          {
            ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
            break;
          }
          if (deep_copy_
              && OB_SUCCESS != (ret = ob_write_obj(allocator_, ci->value_, ci->value_)))
          {
            TBSYS_LOG(WARN, "deep copy cell fail ret=%d allocator_total=%ld allocator_used=%ld",
                      ret, allocator_.total(), allocator_.used());
            break;
          }
          if (OB_SUCCESS != (ret = ObNewScannerHelper::add_cell(cur_row_, *ci, is_ups_row_)))
          {
            TBSYS_LOG(WARN, "add cell to cur_row fail ret=%d cell=[%s] row=[%s]",
                      ret, print_cellinfo(ci), to_cstring(cur_row_));
            break;
          }
          cell_counter++;
          if (is_row_finished)
          {
            rowkey = &(ci->row_key_);
            break;
          }
        }
        if (!is_row_finished
            && 0 != cell_counter)
        {
          TBSYS_LOG(ERROR, "unexpected row iter end, but irf is false, ret=%d cell_counter=%ld %s", ret, cell_counter, to_cstring(cur_row_));
          ret = OB_ERR_UNEXPECTED;
        }
        ret = (0 == cell_counter) ? OB_ITER_END : ret;
        rowkey = (OB_SUCCESS == ret) ? rowkey : NULL;
        row = (OB_SUCCESS == ret) ? &cur_row_ : NULL;
      }
      return ret;
    }

    int ObRowIterAdaptor::get_row_desc(const ObRowDesc *&row_desc) const
    {
      int ret = OB_SUCCESS;
      if (NULL == (row_desc = cur_row_.get_row_desc()))
      {
        ret = OB_NOT_INIT;
      }
      return ret;
    }

    int64_t ObRowIterAdaptor::to_string(char* buf, const int64_t buf_len) const
    {
      int64_t pos = 0;
      if (NULL != buf
          && 0 < buf_len)
      {
        int64_t plen = snprintf(buf, buf_len, "iter=%p allocator_total=%ld allocator_used=%ld deep_copy=%s",
                                cell_iter_, allocator_.total(), allocator_.used(), STR_BOOL(deep_copy_));
        pos = std::min(plen, buf_len);
        pos += cur_row_.to_string(buf + pos, buf_len - pos);
      }
      return pos;
    }

    void ObRowIterAdaptor::set_cell_iter(ObIterator *cell_iter, const ObRowDesc &row_desc, const bool deep_copy)
    {
      cell_iter_ = cell_iter;
      cur_row_.set_row_desc(row_desc);
      deep_copy_ = deep_copy;
      allocator_.reuse();
    }

    void ObRowIterAdaptor::reset()
    {
      cell_iter_ = NULL;
      cur_row_.reset(false, is_ups_row_ ? ObRow::DEFAULT_NOP : ObRow::DEFAULT_NULL);
      deep_copy_ = false;
      allocator_.free();
    }

    void ObRowIterAdaptor::reuse()
    {
      cell_iter_ = NULL;
      cur_row_.reset(false, is_ups_row_ ? ObRow::DEFAULT_NOP : ObRow::DEFAULT_NULL);
      deep_copy_ = false;
      allocator_.reuse();
    }

    //add maoxx
    ObRowCellIterAdaptor::ObRowCellIterAdaptor() : row_iter_(NULL),
                                                   row_desc_(),
                                                   index_row_tmp_(),
                                                   rk_size_(0),
                                                   single_row_iter_(),
                                                   is_iter_end_(false),
                                                   set_row_iter_ret_(OB_SUCCESS)
                                                   //add lbzhong [auto_increment] 20161217:b
                                                   , auto_column_id_(OB_INVALID_ID), max_auto_value_(OB_INVALID_AUTO_INCREMENT_VALUE)
                                                   //add:e
    {
    }

    ObRowCellIterAdaptor::~ObRowCellIterAdaptor()
    {
    }

    int ObRowCellIterAdaptor::next_cell()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != set_row_iter_ret_)
      {
        ret = set_row_iter_ret_;
      }
      else if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {

        ret = single_row_iter_.next_cell();
        if (OB_ITER_END == ret)
        {
          index_row_tmp_.set_row_desc(row_desc_);
          if (OB_SUCCESS != (ret = row_iter_->get_next_row(index_row_tmp_)))
          {
            ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
          }
          //add lbzhong [auto_increment] 20161217:b
          else if (OB_INVALID_ID != auto_column_id_ && OB_SUCCESS != (ret = update_max_auto_value()))
          {
            //do nothing
          }
          //add:e
          else
          {
            single_row_iter_.set_row(&index_row_tmp_, rk_size_);
            ret = single_row_iter_.next_cell();
          }
        }
      }
      is_iter_end_ = (OB_SUCCESS != ret);
      return ret;
    }

    int ObRowCellIterAdaptor::get_cell(ObCellInfo** cell)
    {
      return get_cell(cell, NULL);
    }

    int ObRowCellIterAdaptor::get_cell(ObCellInfo** cell, bool* is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (NULL == cell)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        ret = single_row_iter_.get_cell(cell, is_row_changed);
      }
      return ret;
    }

    int ObRowCellIterAdaptor::is_row_finished(bool* is_row_finished)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_iter_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = single_row_iter_.is_row_finished(is_row_finished);
      }
      return ret;
    }

    void ObRowCellIterAdaptor::set_row_iter(ObRowStore *row_iter, const int64_t rk_size, const ObSchemaManagerV2 *schema_mgr, ObRowDesc row_desc)
    {
        row_iter_ = NULL;
        rk_size_ = 0;
        single_row_iter_.reset();
        row_desc_.reset();
        is_iter_end_ = false;
        if (NULL != row_iter
            && 0 < rk_size )
        {
          int tmp_ret = OB_SUCCESS;
          if (NULL != schema_mgr)
          {
              tmp_ret = single_row_iter_.get_och().reset(row_desc, *schema_mgr);
          }
          else
          {
            single_row_iter_.get_och().set_need_cast(false);
          }
          if (OB_SUCCESS == tmp_ret)
          {
            row_desc_=row_desc;
            index_row_tmp_.set_row_desc(row_desc_);
            tmp_ret = row_iter->get_next_row(index_row_tmp_);
            TBSYS_LOG(DEBUG, "INDEX_ITER_ADAPTOR ret=%d row_tmp2= %s", tmp_ret, to_cstring(index_row_tmp_));
            if (OB_SUCCESS == tmp_ret || OB_ITER_END == tmp_ret)
            {
              row_iter_ = row_iter;
              rk_size_ = rk_size;
              if (OB_ITER_END == tmp_ret)
              {
                is_iter_end_ = true;
              }
              //add lbzhong [auto_increment] 20161217:b
              else if (OB_INVALID_ID != auto_column_id_ && OB_SUCCESS != (tmp_ret = update_max_auto_value()))
              {
                //do nothing
              }
              //add:e
              else
              {
                single_row_iter_.set_row(&index_row_tmp_, rk_size);
              }
            }
          }
          set_row_iter_ret_ = tmp_ret;
        }
    }

    void ObRowCellIterAdaptor::reset()
    {
      row_iter_ = NULL;
      row_desc_.reset();
      rk_size_ = 0;
      single_row_iter_.reset();
      is_iter_end_ = false;
      //add lbzhong [auto_increment] 20161217:b
      auto_column_id_ = OB_INVALID_ID;
      max_auto_value_ = OB_INVALID_AUTO_INCREMENT_VALUE;
      //add:e
    }
    //add e

    //add lbzhong [auto_increment] 20161127:b
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    void ObRowCellIterAdaptor::set_auto_column_id(const uint64_t auto_column_id)
    {
      auto_column_id_ = auto_column_id;
    }

    int64_t ObRowCellIterAdaptor::get_max_auto_value() const
    {
      return max_auto_value_;
    }

    int ObRowCellIterAdaptor::update_max_auto_value()
    {
      int ret = OB_SUCCESS;
      const ObObj *cell = NULL;
      ObObj tmp_value;
      uint64_t tid = OB_INVALID_ID;
      uint64_t cid = OB_INVALID_ID; //UNUSED
      ObString cast_buffer;
      char buffer[OB_MAX_VARCHAR_LENGTH];
      cast_buffer.assign_ptr(buffer, OB_MAX_VARCHAR_LENGTH);
      if (OB_SUCCESS != (ret = row_desc_.get_tid_cid(0, tid, cid)))
      {
        TBSYS_LOG(WARN, "fail to get_cell, ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = index_row_tmp_.get_cell(tid, auto_column_id_, cell)))
      {
        TBSYS_LOG(WARN, "fail to get_cell, ret=%d", ret);
      }
      else
      {
        tmp_value = *cell;
        if (OB_SUCCESS != (ret = obj_cast(tmp_value, ObIntType, cast_buffer)))
        {
          TBSYS_LOG(WARN, "fail to obj_cast, ret=%d", ret);
        }
        else
        {
          int64_t tmp_int = 0;
          tmp_value.get_int(tmp_int);
          if (tmp_int > max_auto_value_)
          {
            max_auto_value_ = tmp_int;
          }
        }
      }
      return ret;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ObAutoIncrementCellAdaptor::ObAutoIncrementCellAdaptor() : row_(NULL),
                                     rk_size_(0),
                                     cur_idx_(0),
                                     is_iter_end_(false),
                                     cell_(),
                                     need_nop_cell_(false)
    {
    }

    ObAutoIncrementCellAdaptor::~ObAutoIncrementCellAdaptor()
    {
    }

    int ObAutoIncrementCellAdaptor::next_cell()
    {
      int ret = OB_SUCCESS;
      const ObObj *value = NULL;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (cur_idx_ >= row_->get_column_num())
      {
        if (!need_nop_cell_)
        {
          ret = OB_ITER_END;
        }
        else
        {
          need_nop_cell_ = false;
          cell_.column_id_ = OB_INVALID_ID;
          cell_.value_.set_ext(ObActionFlag::OP_NOP);
        }
      }
      else if (OB_SUCCESS != (ret = row_->raw_get_cell(cur_idx_, value, cell_.table_id_, cell_.column_id_))
              || NULL == value)
      {
        ret = (OB_SUCCESS == ret) ? OB_ERROR : ret;
      }
      else
      {
        TBSYS_LOG(DEBUG, "CELL_ADAPTOR idx=%ld %s", cur_idx_, print_cellinfo(&cell_));
        cell_.value_ = *value;
        if (OB_ACTION_FLAG_COLUMN_ID == cell_.column_id_)
        {
          cell_.value_.set_ext(ObActionFlag::OP_DEL_ROW);
        }
        if (ObExtendType == cell_.value_.get_type())
        {
          cell_.column_id_ = OB_INVALID_ID;
        }
        if (OB_SUCCESS == ret)
        {
          cur_idx_ += 1;
        }
      }
      is_iter_end_ = (OB_SUCCESS != ret);
      return ret;
    }

    int ObAutoIncrementCellAdaptor::get_cell(ObCellInfo** cell)
    {
      return get_cell(cell, NULL);
    }

    int ObAutoIncrementCellAdaptor::get_cell(ObCellInfo** cell, bool* is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (NULL == cell)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        *cell = &cell_;
        if (NULL != is_row_changed)
        {
          *is_row_changed = (cur_idx_ <= (rk_size_ + 1));
        }
      }
      return ret;
    }

    int ObAutoIncrementCellAdaptor::is_row_finished(bool* is_row_finished)
    {
      int ret = OB_SUCCESS;
      if (NULL == row_)
      {
        ret = OB_NOT_INIT;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        if (NULL != is_row_finished)
        {
          if (cur_idx_ == row_->get_column_num()
              && !need_nop_cell_)
          {
            *is_row_finished = true;
          }
          else
          {
            *is_row_finished = false;
          }
        }
      }
      return ret;
    }

    void ObAutoIncrementCellAdaptor::set_row(const ObRow *row, const int64_t rk_size)
    {
      row_ = NULL;
      rk_size_ = 0;
      cur_idx_ = 0;
      is_iter_end_ = false;
      cell_.reset();
      need_nop_cell_ = false;
      if (NULL != row
          && 0 < rk_size)
      {
        const ObObj *rk_values = NULL;
        uint64_t table_id = OB_INVALID_ID;
        uint64_t column_id = OB_INVALID_ID;
        if (rk_size <= row->get_column_num()
            && OB_SUCCESS == row->raw_get_cell(0, rk_values, table_id, column_id)
            && NULL != rk_values)
        {
          TBSYS_LOG(DEBUG, "set_row succ rk_size=%ld col_num=%ld", rk_size, row->get_column_num());
          row_ = row;
          rk_size_ = rk_size;
          cur_idx_ = rk_size;
          cell_.row_key_.assign(const_cast<ObObj*>(rk_values), rk_size);
          cell_.table_id_ = table_id;
          cell_.column_id_ = column_id;
          need_nop_cell_ = (rk_size == row->get_column_num());
        }
        else
        {
          TBSYS_LOG(WARN, "set_row fail rk_size=%ld col_num=%ld", rk_size, row->get_column_num());
        }
      }
    }

    void ObAutoIncrementCellAdaptor::reset()
    {
      row_ = NULL;
      rk_size_ = 0;
      cur_idx_ = 0;
      is_iter_end_ = false;
      cell_.reset();
      need_nop_cell_ = false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////

    ObAutoIncrementCellIterAdaptor::ObAutoIncrementCellIterAdaptor() : single_row_iter_(),
                                             is_iter_end_(false),
                                             set_row_iter_ret_(OB_SUCCESS)
    {
    }

    ObAutoIncrementCellIterAdaptor::~ObAutoIncrementCellIterAdaptor()
    {
    }

    int ObAutoIncrementCellIterAdaptor::next_cell()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS != set_row_iter_ret_)
      {
        ret = set_row_iter_ret_;
      }
      else if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = single_row_iter_.next_cell();
      }
      is_iter_end_ = (OB_SUCCESS != ret);
      return ret;
    }

    int ObAutoIncrementCellIterAdaptor::get_cell(ObCellInfo** cell)
    {
      return get_cell(cell, NULL);
    }

    int ObAutoIncrementCellIterAdaptor::get_cell(ObCellInfo** cell, bool* is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else if (NULL == cell)
      {
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        ret = single_row_iter_.get_cell(cell, is_row_changed);
      }
      return ret;
    }

    int ObAutoIncrementCellIterAdaptor::is_row_finished(bool* is_row_finished)
    {
      int ret = OB_SUCCESS;
      if (is_iter_end_)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = single_row_iter_.is_row_finished(is_row_finished);
      }
      return ret;
    }

    void ObAutoIncrementCellIterAdaptor::set_row_iter(const uint64_t table_id, const uint64_t column_id, const int64_t auto_value)
    {
      single_row_iter_.reset();
      is_iter_end_ = false;
      int tmp_ret = OB_SUCCESS;
      row_desc_.set_rowkey_cell_count(2);
      for (int32_t i = 0; OB_SUCCESS == tmp_ret && i < 3; i++)
      {
        if (OB_SUCCESS != (tmp_ret = row_desc_.add_column_desc(OB_ALL_AUTO_INCREMENT_TID, OB_APP_MIN_COLUMN_ID + i)))
        {
          TBSYS_LOG(WARN, "fail to add column desc, ret=%d", tmp_ret);
        }
      }
      if (OB_SUCCESS == tmp_ret)
      {
        row_.set_row_desc(row_desc_);
        ObObj cell;
        cell.set_int(table_id);
        row_.set_cell(OB_ALL_AUTO_INCREMENT_TID, OB_APP_MIN_COLUMN_ID, cell);
        cell.set_int(column_id);
        row_.set_cell(OB_ALL_AUTO_INCREMENT_TID, OB_APP_MIN_COLUMN_ID + 1, cell);
        cell.set_int(auto_value);
        row_.set_cell(OB_ALL_AUTO_INCREMENT_TID, OB_APP_MIN_COLUMN_ID + 2, cell);

        single_row_iter_.set_row(&row_, 2);
      }
      set_row_iter_ret_ = tmp_ret;
    }

    void ObAutoIncrementCellIterAdaptor::reset()
    {
      single_row_iter_.reset();
      is_iter_end_ = false;
    }
    //add:e
  }
}
