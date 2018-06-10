#include "ob_auto_increment_filter.h"
#include "common/ob_obj_cast.h"

using namespace oceanbase;
using namespace common;
using namespace sql;

ObAutoIncrementFilter::ObAutoIncrementFilter():auto_column_id_(OB_INVALID_ID),
  auto_value_(OB_INVALID_AUTO_INCREMENT_VALUE),is_assigned_(false)
{

}

void ObAutoIncrementFilter::reset()
{
  cur_row_desc_.reset();
  auto_column_id_ = OB_INVALID_ID;
  auto_value_ = OB_INVALID_AUTO_INCREMENT_VALUE;
  is_assigned_ = false;
  ObSingleChildPhyOperator::reset();
}

void ObAutoIncrementFilter::reuse()
{
  cur_row_desc_.reset();
  auto_column_id_ = OB_INVALID_ID;
  auto_value_ = OB_INVALID_AUTO_INCREMENT_VALUE;
  is_assigned_ = false;
  ObSingleChildPhyOperator::reuse();
}

int ObAutoIncrementFilter::open()
{
  int ret = OB_SUCCESS;
  ret = ObSingleChildPhyOperator::open();
  const ObRowDesc *row_desc = NULL;
  uint64_t table_id = OB_INVALID_ID;
  uint64_t column_id = OB_INVALID_ID;

  if (OB_INVALID_ID == auto_column_id_ || OB_INVALID_AUTO_INCREMENT_VALUE == auto_value_)
  {
    TBSYS_LOG(WARN, "ObAutoIncrementFilter is not init, auto_column_id_=%ld, auto_value_=%ld",
              auto_column_id_, auto_value_);
    ret = OB_INVALID_ARGUMENT;
  }
  else if (OB_SUCCESS != (ret = child_op_->get_row_desc(row_desc)))
  {
    TBSYS_LOG(WARN, "fail to get row desc:ret[%d]", ret);
  }
  else
  {
    cur_row_desc_.reset();
    cur_row_desc_.set_rowkey_cell_count(row_desc->get_rowkey_cell_count());
    bool is_insert = false;
    for (int64_t i = 0; OB_SUCCESS == ret && i < row_desc->get_column_num(); i ++)
    {
      if (OB_SUCCESS != (ret = row_desc->get_tid_cid(i, table_id, column_id)))
      {
        TBSYS_LOG(WARN, "fail to get tid cid:ret[%d]", ret);
      }
      else if (auto_column_id_ == column_id)
      {
        is_assigned_ = true;
        break;
      }
      else if (!is_insert && auto_column_id_ < column_id) //insert
      {
        if (OB_INVALID_INDEX != cur_row_desc_.get_idx(table_id, auto_column_id_))
        {
          is_insert = true;
        }
        else if (OB_SUCCESS != (ret = cur_row_desc_.add_column_desc(table_id, auto_column_id_)))
        {
          TBSYS_LOG(WARN, "fail to add column desc, ret=%d", ret);
        }
        else
        {
          is_insert = true;
        }
      }
      if (OB_SUCCESS == ret && OB_SUCCESS != (ret = cur_row_desc_.add_column_desc(table_id, column_id)))
      {
        TBSYS_LOG(WARN, "fail to add column desc:ret[%d]", ret);
      }
    }
  }

  if (OB_SUCCESS == ret)
  {
    cur_row_.set_row_desc(cur_row_desc_);
    TBSYS_LOG(DEBUG, "auto_increment filter cur row desc[%s]", to_cstring(cur_row_desc_));
  }
  return ret;
}

int ObAutoIncrementFilter::get_next_row(const common::ObRow *&row)
{
  int ret = common::OB_SUCCESS;
  ret = child_op_->get_next_row(row);
  if(OB_SUCCESS == ret)
  {
    if (!is_assigned_)
    {
      uint64_t tid = OB_INVALID_ID;
      uint64_t cid = OB_INVALID_ID;
      const ObObj *cell = NULL;
      bool is_insert = false;
      for (int64_t i = 0; OB_SUCCESS == ret && i < row->get_row_desc()->get_column_num(); ++i)
      {
        if (OB_SUCCESS != (ret = row->get_row_desc()->get_tid_cid(i, tid, cid)))
        {
          TBSYS_LOG(WARN, "fail to get_tid_cid, ret=%d", ret);
        }
        else if (!is_insert && auto_column_id_ < cid) //insert
        {
          ObObj tmp_cell;
          tmp_cell.set_type(ObIntType);
          tmp_cell.set_int(++auto_value_);
          if (OB_SUCCESS != (ret = cur_row_.set_cell(tid, auto_column_id_, tmp_cell)))
          {
            TBSYS_LOG(WARN, "fail to set cell, ret=%d", ret);
          }
          is_insert = true;
        }
        if (OB_SUCCESS != (ret = row->get_cell(tid, cid, cell)))
        {
          TBSYS_LOG(WARN, "fail to get_cell, ret=%d", ret);
        }
        else if (OB_SUCCESS != (ret = cur_row_.set_cell(tid, cid, *cell)))//copy
        {
          TBSYS_LOG(WARN, "fail to set cell, ret=%d", ret);
        }
      }
      if(OB_SUCCESS == ret)
      {
        row = &cur_row_;
      }
    }
    else // get assigned id value
    {
      const ObObj *cell = NULL;
      ObObj tmp_value;
      uint64_t tid = OB_INVALID_ID;
      uint64_t cid = OB_INVALID_ID; //UNUSED
      ObString cast_buffer;
      char buffer[OB_MAX_VARCHAR_LENGTH];
      cast_buffer.assign_ptr(buffer, OB_MAX_VARCHAR_LENGTH);
      if (OB_SUCCESS != (ret = row->get_row_desc()->get_tid_cid(0, tid, cid)))
      {
        TBSYS_LOG(WARN, "fail to get_cell, ret=%d", ret);
      }
      else if (OB_SUCCESS != (ret = row->get_cell(tid, auto_column_id_, cell)))
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
          if (tmp_int > auto_value_)
          {
            auto_value_ = tmp_int;
          }
        }
      }
    }
  }
  return ret;
}

int ObAutoIncrementFilter::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (cur_row_desc_.get_column_num() == 0)
  {
    child_op_->get_row_desc(row_desc);
  }
  else
  {
    row_desc = &cur_row_desc_;
  }
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObAutoIncrementFilter, PHY_AUTO_INCREMENT_FILTER);
  }
}

int64_t ObAutoIncrementFilter::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "ObAutoIncrementFilter(auto_column_id=%ld, auto_value=%ld, is_assigned=%d)\n",
                  auto_column_id_, auto_value_, is_assigned_);
  if (NULL != child_op_)
  {
    pos += child_op_->to_string(buf+pos, buf_len-pos);
  }
  return pos;
}

enum ObPhyOperatorType ObAutoIncrementFilter::get_type() const
{
  return PHY_AUTO_INCREMENT_FILTER;
}

void ObAutoIncrementFilter::set_auto_column(const uint64_t auto_column_id, const int64_t auto_value)
{
  auto_column_id_ = auto_column_id;
  auto_value_ = auto_value;
}

int64_t ObAutoIncrementFilter::get_cur_auto_value() const
{
  return auto_value_;
}

PHY_OPERATOR_ASSIGN(ObAutoIncrementFilter)
{
  UNUSED(other);
  return common::OB_SUCCESS;
}

DEFINE_SERIALIZE(ObAutoIncrementFilter)
{
  UNUSED(buf);
  UNUSED(buf_len);
  UNUSED(pos);
  return OB_SUCCESS;
}

DEFINE_DESERIALIZE(ObAutoIncrementFilter)
{
  UNUSED(buf);
  UNUSED(data_len);
  UNUSED(pos);
  return OB_SUCCESS;
}

DEFINE_GET_SERIALIZE_SIZE(ObAutoIncrementFilter)
{
  return 0;
}
