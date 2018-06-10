/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor.cpp
* @brief this class  present a cursor physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#include "ob_cursor.h"
#include "common/utility.h"
#include "ob_physical_plan.h"
using namespace oceanbase::sql;
using namespace oceanbase::common;

ObCursor::ObCursor()
  :is_opened_(0),mem_size_limit_(10000000), row_offset_(0),row_count_(0),run_idx_(0),row_num_(0),cursor_reader_(&in_mem_cursor_)
{
}

ObCursor::~ObCursor()
{
}

void ObCursor::reset()
{
  is_opened_ = 0;
  mem_size_limit_ = 0;
  row_count_ = 0;
  row_offset_ = 0;
  run_idx_ = 0;
  row_num_ = 0;
  in_mem_cursor_.reset();
  merge_cursor_.reset();
  // FIXME: why not reset to NULL?
  cursor_reader_ = &in_mem_cursor_;
  ObSingleChildPhyOperator::reset();
}

void ObCursor::reuse()
{
  is_opened_ = 0;
  mem_size_limit_ = 0;
  row_count_ = 0;
  row_offset_ = 0;
  run_idx_ = 0;
  row_num_ = 0;
  in_mem_cursor_.reuse();
  merge_cursor_.reuse();
  // FIXME: why not reset to NULL?
  cursor_reader_ = &in_mem_cursor_;
  ObSingleChildPhyOperator::reuse();
}


void ObCursor::set_mem_size_limit(const int64_t limit)
{
  TBSYS_LOG(INFO, "cursor mem limit=%ld", limit);
  mem_size_limit_ = limit;
}

int ObCursor::set_run_filename(const common::ObString &filename)
{
  TBSYS_LOG(INFO, "cursor run file=%.*s", filename.length(), filename.ptr());
  return merge_cursor_.set_run_filename(filename);
}

int ObCursor::open()
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = ObSingleChildPhyOperator::open()))
  {
    TBSYS_LOG(WARN, "failed to open child_op, err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = do_cursor()))
  {
    TBSYS_LOG(WARN, "failed to store input data, err=%d", ret);
  }
  if(OB_SUCCESS == ret)
  {
	  is_opened_ = 1;
  }
  return ret;
}

int ObCursor::close()
{
  int ret = OB_SUCCESS;
  in_mem_cursor_.reset();
  merge_cursor_.reset();
  cursor_reader_ = &in_mem_cursor_;
  ret = ObSingleChildPhyOperator::close();
  return ret;
}

int ObCursor::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(NULL == child_op_))
  {
    TBSYS_LOG(ERROR, "child op is NULL");
    ret = OB_NOT_INIT;
  }
  else
  {
    ret = child_op_->get_row_desc(row_desc);
  }
  return ret;
}

int ObCursor::do_cursor()
{
  int ret = OB_SUCCESS;
  bool need_merge = false;
  const common::ObRow *input_row = NULL;
  while(OB_SUCCESS == ret
  && OB_SUCCESS == (ret = child_op_->get_next_row(input_row)))
  {
    row_count_++;
    if (OB_SUCCESS != (ret = in_mem_cursor_.add_row(*input_row)))
    {
      TBSYS_LOG(WARN, "failed to add row, err=%d", ret);
    }
    else if (need_dump())
    {
     if (OB_SUCCESS != (ret = merge_cursor_.dump_run(in_mem_cursor_)))
     {
      TBSYS_LOG(WARN, "failed to dump, err=%d", ret);
     }
     else
     {
       TBSYS_LOG(INFO, "need merge cursor");
       if (OB_SUCCESS != (ret = run_array_.push_back(row_count_)))
       {
         TBSYS_LOG(WARN, "failed to push back run array, err=%d", ret);
       }
       else
       {
        row_num_ = row_num_ + row_count_;
        in_mem_cursor_.reset();
        need_merge = true;
        row_count_ = 0;
        cursor_reader_ = &in_mem_cursor_;
       }
      }
     }
   } // end while
    if (OB_ITER_END == ret)
    {
      ret = OB_SUCCESS;
    }
    if (OB_SUCCESS == ret)
    {
     if (need_merge && 0 < in_mem_cursor_.get_row_count())
      {
       if (OB_SUCCESS != (ret = merge_cursor_.dump_run(in_mem_cursor_)))
        {
          TBSYS_LOG(WARN, "failed to dump, err=%d", ret);
        }
       else
        {
          TBSYS_LOG(INFO, "need merge cursor");
          if (OB_SUCCESS != (ret = run_array_.push_back(row_count_)))
          {
            TBSYS_LOG(WARN, "failed to push back run array, err=%d", ret);
          }
          else
          {
           row_num_ = row_num_ + row_count_;
           in_mem_cursor_.reset();
           need_merge = true;
           row_count_ = 0;
           cursor_reader_ = &in_mem_cursor_;
          }
        }
      }
     else
     {
    	 row_num_ = row_num_ + row_count_;
    	 cursor_reader_ = &in_mem_cursor_;
     }
    }
  return ret;
}

inline bool ObCursor::need_dump() const
{
  return mem_size_limit_ <= 0 ? false : (in_mem_cursor_.get_used_mem_size() >= mem_size_limit_);
}

int ObCursor::get_run_file()
{
  int ret = OB_SUCCESS;
  in_mem_cursor_.reset();
  merge_cursor_.set_run_idx(run_idx_);
  cursor_reader_ = &merge_cursor_;
  const common::ObRow *input_row = NULL;
  while(OB_SUCCESS == cursor_reader_->get_next_row(input_row))
  {
   if (OB_SUCCESS != (ret = in_mem_cursor_.add_row(*input_row)))
   {
    TBSYS_LOG(WARN, "failed to add row, err=%d", ret);
   }
  }
  ret = merge_cursor_.end_get_run();
  cursor_reader_ = &in_mem_cursor_;
  row_offset_ = 0;
  return ret;
}

int ObCursor::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if(run_array_.count() > 0)//判断数据是否被刷过磁盘
  {
   if ( 0 < in_mem_cursor_.get_row_count())//判断内存中是否有要取的数据
   {
    ret = cursor_reader_->get_next_row(row);
    if(ret == OB_ITER_END)//需要读取下一个磁盘块
    {
    if(run_idx_ >= (run_array_.count() - 1))
    {
            ret = OB_ITER_END;
    }
    else
    {
     run_idx_++;
     ret = get_run_file();//将磁盘块数据载入内存
     ret = cursor_reader_->get_next_row(row);//读取数据
     row_offset_++;
    }
    }
    else
    {
      row_offset_++;
    }
   }
   else//内存中没有数据，需要从磁盘中读取
   {
      ret = get_run_file();
      TBSYS_LOG(WARN, "get next row enter in merge cursor");
      ret = cursor_reader_->get_next_row(row);
      row_offset_++;
   }
  }
  else
  {
  	 ret = cursor_reader_->get_next_row(row);
  	 if(ret != OB_ITER_END)
  	 row_offset_++;
  }
	  return ret;
}

int ObCursor::get_ab_row(int64_t ab_num_,const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if(ab_num_ > row_num_)
  {
  	ret = OB_ITER_END;
  }
  else
  {
  if(run_array_.count() > 0)
  {
      int64_t idx = (ab_num_*run_array_.count()) /row_num_;
      int64_t comp = 0;
      for(int64_t i=0;i<idx;i++)
      {
        comp = comp + run_array_.at(i);
      }
      if(ab_num_<= comp)
      {
      	while(ab_num_<= (comp - run_array_.at(idx-1)))
      	{
      	   comp = comp - run_array_.at(idx-1);
	   idx--;
      	}
  
      	if(run_idx_ == (idx-1))
      	{
	  row_offset_ = ab_num_ -(comp - run_array_.at(idx-1)) - 1;
	  in_mem_cursor_.set_pos(row_offset_);
	  ret = cursor_reader_->get_next_row(row);
	  row_offset_++;
      	}
      	else
      	{
	  run_idx_ = idx - 1;
	  ret = get_run_file();
	  row_offset_ = ab_num_ -(comp - run_array_.at(idx-1)) - 1;
	  in_mem_cursor_.set_pos(row_offset_);
	  ret = cursor_reader_->get_next_row(row);
	  row_offset_++;
      	}
      }
      else
      {
      	while(ab_num_ >(comp + run_array_.at(idx)))
      	{
	  comp = comp + run_array_.at(idx);
	  idx++;
      	}
      	if(run_idx_ == idx)
      	{
	  if ( 0 >= in_mem_cursor_.get_row_count())
	  {
	  	ret = get_run_file();
	  }
	  row_offset_ = ab_num_ - comp - 1;
	  in_mem_cursor_.set_pos(row_offset_);
	  ret = cursor_reader_->get_next_row(row);
	  row_offset_++;
      	}
      	else
      	{
      	  run_idx_ = idx;
      	  ret = get_run_file();
      	  row_offset_ = ab_num_ - comp - 1;
      	  in_mem_cursor_.set_pos(row_offset_);
      	  ret = cursor_reader_->get_next_row(row);
      	  row_offset_++;
      	}
      }
  }
  else
  {
  	row_offset_ = ab_num_  - 1;
  	in_mem_cursor_.set_pos(row_offset_);
	ret = cursor_reader_->get_next_row(row);
        row_offset_++;
  }
 }
return ret;
}

int ObCursor::get_first_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if(run_array_.count() > 0)//如果数据在磁盘中
  {
    if(run_idx_ == 0)
    {
     if ( 0 < in_mem_cursor_.get_row_count())
     {
    	row_offset_ = 0;//定位到第一行
    	in_mem_cursor_.set_pos(row_offset_);
    	ret = cursor_reader_->get_next_row(row);//读取该行数据
    	row_offset_++;
     }

     else
     {
    	 ret = get_next_row(row);
     }
    }
    else
    {
    	run_idx_ = 0;//定位到磁盘第一块数据
    	ret = get_run_file();//将第一块数据载入内存
    	row_offset_ = 0;
    	in_mem_cursor_.set_pos(row_offset_);
     	ret = cursor_reader_->get_next_row(row);//读取该行数据
     	row_offset_++;
    }
  }
else                    //如果数据在内存中
{
	row_offset_ = 0;
	in_mem_cursor_.set_pos(row_offset_);
	ret = cursor_reader_->get_next_row(row);
        row_offset_++;
}
return ret;
}

int ObCursor::get_last_row(const common::ObRow *&row)
{
    int ret = OB_SUCCESS;
    if(run_array_.count() > 0)//如果数据在磁盘中
    {
	if(run_idx_ == (run_array_.count()-1))
	{
	   row_offset_ = run_array_.at(run_idx_) - 1;//定位到最后一行
	   in_mem_cursor_.set_pos(row_offset_);
    	   ret = cursor_reader_->get_next_row(row);//读取该行数据
    	   row_offset_++;
	}
	else
	{
	   run_idx_ = run_array_.count()-1;
	   ret = get_run_file();//将磁盘中最后一块数据载入内存
	   row_offset_ = run_array_.at(run_idx_) - 1;//定位到最后一行
	   in_mem_cursor_.set_pos(row_offset_);
	   ret = cursor_reader_->get_next_row(row);//读取该行数据
	   row_offset_++;
	}
     }
     else//如果数据在内存中
     {
	row_offset_ = row_num_ -1;
	in_mem_cursor_.set_pos(row_offset_);
	ret = cursor_reader_->get_next_row(row);
        row_offset_++;
     }
return ret;
}

int ObCursor::get_prior_row(const common::ObRow *&row)
{
   int ret = OB_SUCCESS;
   if(row_offset_ == 0)        //判断是不是第一行
   {
   	ret = OB_ITER_END;
   }
   else
   {
     if(run_array_.count() > 0)
     {
	if(row_offset_ == 1)
	{
	  if(run_idx_ <= 0)
	  {
	    ret = OB_ITER_END;
	  }
	  else
	  {
	    run_idx_--;
	    ret = get_run_file();//将磁盘中的前一块数据读入内存
	    row_offset_ = run_array_.at(run_idx_) - 1;//定位到该块数据的最后一行
	    in_mem_cursor_.set_pos(row_offset_);
	    ret = cursor_reader_->get_next_row(row);//读取该行数据
	    row_offset_++;
	  }
	}
	else
	{
	  row_offset_ = row_offset_ - 2;//定位到前一行数据
	  in_mem_cursor_.set_pos(row_offset_);
	  ret = cursor_reader_->get_next_row(row);//读取该行数据
     	  row_offset_++;
	}
      }
   else
   {
	if(row_offset_ == 1)
	{
	  ret = OB_ITER_END;
	}
	else
	{
	  row_offset_ = row_offset_ - 2;//定位到前一行数据
	  in_mem_cursor_.set_pos(row_offset_);
	  ret = cursor_reader_->get_next_row(row);//读取该行数据
	  row_offset_++;
	}
   }

  }
return ret;
}

int ObCursor::get_rela_row(bool is_next,int64_t rela_count,const common::ObRow *&row)
{
   int ret = OB_SUCCESS;
   if(run_array_.count() > 0)//如果数据在磁盘中
   {

	int64_t curr_num = 0;//用于计算当前fetch的行数
	for(int64_t i=0;i<run_idx_;i++)
	{
	  curr_num = curr_num + run_array_.at(i);
	}
	curr_num = curr_num + row_offset_;
	if(is_next == 0)//如果是相对向后fetch
	{
	  if(rela_count >= curr_num)
	  {
	    ret = OB_ITER_END;
	  }
	  else
	  {
	    ret = get_ab_row(curr_num - rela_count,row);//读取该行数据
	  }
	}
	else//如果是相对向前fetch
	{
	  if(rela_count > row_num_ - curr_num)
	  {ret = OB_ITER_END;}
	  else
	  {
	    ret = get_ab_row(curr_num + rela_count,row);//读取该行数据
	  }
	}
}
else//如果数据在内存中
{
	if(is_next == 0)//如果是相对向后fetch
	{
	  if(rela_count >= row_offset_)
	  {
	    ret = OB_ITER_END;
	  }
	  else
	  {
	  	row_offset_ = row_offset_ - rela_count - 1;
		in_mem_cursor_.set_pos(row_offset_);
		ret = cursor_reader_->get_next_row(row);//读取该行数据
	        row_offset_++;
	  }
	}
else//如果是相对向前fetch
{
	if(rela_count > row_num_ - row_offset_)
	{ret = OB_ITER_END;}
	else
	{
	   row_offset_ = row_offset_ + rela_count - 1;
	   in_mem_cursor_.set_pos(row_offset_);
	   ret = cursor_reader_->get_next_row(row);//读取该行数据
	   row_offset_++;
	}
 }
}
return ret;
}

namespace oceanbase{
namespace sql{
REGISTER_PHY_OPERATOR(ObCursor, PHY_CURSOR);
}
}

int64_t ObCursor::to_string(char* buf, const int64_t buf_len) const
{
int64_t pos = 0;
if (NULL != child_op_)
{
int64_t pos2 = child_op_->to_string(buf+pos, buf_len-pos);
pos += pos2;
}
return pos;
}

DEFINE_SERIALIZE(ObCursor)
{
int ret = OB_SUCCESS;

if (OB_SUCCESS == ret)
{
if (OB_SUCCESS != (ret = serialization::encode_vi64(buf, buf_len, pos, mem_size_limit_)))
{
TBSYS_LOG(WARN, ":ret[%d]", ret);
}
}
return ret;
}

DEFINE_DESERIALIZE(ObCursor)
{
int ret = OB_SUCCESS;


if (OB_SUCCESS == ret)
{
if (OB_SUCCESS != (ret = serialization::decode_vi64(buf, data_len, pos, &mem_size_limit_)))
{
TBSYS_LOG(WARN, "fail to decode mem_size_limit_:ret[%d]", ret);
}
}
return ret;
}

DEFINE_GET_SERIALIZE_SIZE(ObCursor)
{
int64_t size = 0;

size += serialization::encoded_length_vi64(mem_size_limit_);
return size;
}

PHY_OPERATOR_ASSIGN(ObCursor)
{
int ret = OB_SUCCESS;
CAST_TO_INHERITANCE(ObCursor);
reset();
//  sort_columns_ = o_ptr->get_sort_columns();
mem_size_limit_ = o_ptr->get_mem_size_limit();
return ret;
}

ObPhyOperatorType ObCursor::get_type() const
{
return PHY_CURSOR;
}



