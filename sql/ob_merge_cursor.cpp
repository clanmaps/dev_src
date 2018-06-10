/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_merger_cursor.cpp
* @brief this class  is a cursor helper implement
*
* Created by zhounan: support cursor
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#include "ob_merge_cursor.h"
#include "common/ob_compact_cell_iterator.h"
#include <algorithm>
using namespace oceanbase::sql;
using namespace oceanbase::common;



ObMergeCursor::ObMergeCursor()
  :final_run_(NULL),run_idx_(0),
   dump_run_count_(0),count_(0), row_desc_(NULL)
{
  run_filename_buf_[0] = '\0';
}

ObMergeCursor::~ObMergeCursor()
{
}

void ObMergeCursor::reset()
{
  int ret = OB_SUCCESS;
  if (run_file_.is_opened())
  {
    if (OB_SUCCESS != (ret = run_file_.close()))
    {
      TBSYS_LOG(WARN, "failed to close run file, err=%d", ret);
    }
  }
  if ('\0' != run_filename_buf_[0])
  {
    struct stat stat_buf;
    if (0 == stat(run_filename_buf_, &stat_buf))
    {
      if (0 != unlink(run_filename_buf_))
      {
        TBSYS_LOG(WARN, "failed to remove tmp run file, err=%s", strerror(errno));
      }
    }
  }
  dump_run_count_ = 0;
  run_idx_ = 0;
  row_desc_ = NULL;
}

void ObMergeCursor::reuse()
{
  int ret = OB_SUCCESS;
  if (run_file_.is_opened())
  {
    if (OB_SUCCESS != (ret = run_file_.close()))
    {
      TBSYS_LOG(WARN, "failed to close run file, err=%d", ret);
    }
  }
  struct stat stat_buf;
  if (0 == stat(run_filename_buf_, &stat_buf))
  {
    if (0 != unlink(run_filename_buf_))
    {
      TBSYS_LOG(WARN, "failed to remove tmp run file, err=%s", strerror(errno));
    }
  }
  dump_run_count_ = 0;
  run_idx_ = 0;
  row_desc_ = NULL;
}



int ObMergeCursor::set_run_filename(const common::ObString &filename)
{
  int ret = OB_SUCCESS;
  if (filename.length() >= OB_MAX_FILE_NAME_LENGTH)
  {
    TBSYS_LOG(ERROR, "filename is too long, filename=%.*s", filename.length(), filename.ptr());
    ret = OB_BUF_NOT_ENOUGH;
  }
  else
  {
    snprintf(run_filename_buf_, OB_MAX_FILE_NAME_LENGTH, "%.*s", filename.length(), filename.ptr());
    run_filename_.assign_ptr(run_filename_buf_, filename.length());
  }
  return ret;
}


int ObMergeCursor::dump_run(ObInMemoryCursor &rows)
{
  int ret = OB_SUCCESS;
  if (!run_file_.is_opened())
  {
    if (OB_SUCCESS != (ret = run_file_.open(run_filename_)))
    {
      TBSYS_LOG(WARN, "failed to open run file, err=%d filename=%.*s",
                ret, run_filename_.length(), run_filename_.ptr());
    }
  }
  if (OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = run_file_.begin_append_run(CURSOR_RUN_FILE_BUCKET_ID)))
    {
      TBSYS_LOG(WARN, "failed to begin dump run, err=%d", ret);
    }
    else
    {
      ObString compact_row;
      while (OB_SUCCESS == rows.get_next_compact_row(compact_row))
      {
        if (OB_SUCCESS != (ret = run_file_.append_row(compact_row)))
        {
          TBSYS_LOG(WARN, "failed to append row, err=%d", ret);
          break;
        }
      }
      if (OB_ITER_END == ret)
      {
        ret = OB_SUCCESS;
      }

      if (OB_SUCCESS == ret)
      {
        if (OB_SUCCESS != (ret = run_file_.end_append_run()))
        {
          TBSYS_LOG(WARN, "failed to end dump run, err=%d", ret);
        }
        else
        {
          if (NULL == row_desc_)
          {
            row_desc_ = rows.get_row_desc();
          }
          TBSYS_LOG(INFO, "dump run, row_count=%ld", rows.get_row_count());
        }
      }
    }
  }
  return ret;
}




void ObMergeCursor::set_final_run(ObInMemoryCursor &rows)
{
  final_run_ = &rows;
}


int ObMergeCursor::end_get_run()
{
   int ret = OB_SUCCESS;
   if (OB_SUCCESS != (ret = run_file_.end_read_bucket()))
     {
       TBSYS_LOG(WARN, "failed to end read backet, err=%d", ret);
     }
   
   count_ = 0;

return ret;

}

int ObMergeCursor::get_next_row(common::ObRow &row)
{
    int ret = OB_SUCCESS;
    OB_ASSERT(row_desc_);
    row.set_row_desc(*row_desc_);
    if(count_ == 0)
    {
        if (OB_SUCCESS != (ret = run_file_.begin_read_bucket(CURSOR_RUN_FILE_BUCKET_ID, dump_run_count_)))
        {
            TBSYS_LOG(WARN, "failed to begin to read backet, err=%d", ret);
        }
        if (OB_SUCCESS != (ret = run_file_.get_next_row(run_idx_, row)))
        {
            TBSYS_LOG(WARN, "failed to read next row, err=%d run_idx=%ld", ret, run_idx_);

        }
        count_++;
    }
    else
    {
        if (OB_SUCCESS != (ret = run_file_.get_next_row(run_idx_, row)))
        {
            TBSYS_LOG(WARN, "failed to read next row, err=%d run_idx=%ld", ret, run_idx_);
        }
    }

    return ret;
}

int ObMergeCursor::get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS == (ret = get_next_row(curr_row_)))
  {
    row = &curr_row_;
  }
  return ret;
}
