/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_local_agent.cpp
 * @brief get range data in cs itself
 *
 * Created by longfei：an operator with no children,for scan data on cs itself
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */
#include "ob_index_local_agent.h"
#include "common/ob_scan_param.h"
#include "sql/ob_tablet_scan.h"

namespace oceanbase
{
  namespace chunkserver
  {
    using namespace oceanbase::sql;

    ObIndexLocalAgent::ObIndexLocalAgent()
    {
      local_idx_scan_finish_ = false;
      local_idx_block_end_ = false;
      first_scan_ = false;
      hash_index_ = 0;
      scan_param_ = NULL;
    }

    ObIndexLocalAgent::~ObIndexLocalAgent()
    {
    }

    int ObIndexLocalAgent::open()
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS == (ret = build_sst_scan_param()))
      {
        ret = scan();
      }
      cur_row_.set_row_desc(row_desc_);
      if (OB_ITER_END == ret)
      {
        local_idx_scan_finish_ = true;
        ret = OB_SUCCESS;
      }
      return ret;
    }

    int ObIndexLocalAgent::close()
    {
      int ret = OB_SUCCESS;
      return ret;
    }

    void ObIndexLocalAgent::reset()
    {
      row_desc_.reset();
      sst_scan_.reset();
      sst_scan_param_.reset();
      range_server_hash_ = NULL;
      local_idx_scan_finish_ = false;
      local_idx_block_end_ = false;
      first_scan_ = false;
      hash_index_ = 0;
      scan_param_ = NULL;
    }

    void ObIndexLocalAgent::reuse()
    {

    }

    int ObIndexLocalAgent::scan()
    {
      int ret = OB_SUCCESS;
      ObNewRange fake_range;
      sst_scan_.close();
      sst_scan_.reset();
      if(OB_SUCCESS != (ret = get_next_local_range(fake_range)))
      {
        if (ret != OB_ITER_END)
        {
          TBSYS_LOG(WARN,"get_next_local_range failed.ret[%d]",ret);
        }
      }
      if (OB_SUCCESS == ret)
      {
        ret = sst_scan_.open_scan_context_local_idx(sst_scan_param_, sc_, fake_range);
      }
      if (OB_SUCCESS == ret)
      {
        ret = sst_scan_.init_sstable_scanner_for_local_idx(fake_range);
        //@todo(longfei):ignore the failed fake range at present
//        if (OB_SUCCESS != ret)
//        {
//          if (!query_agent_)
//          {
//            TBSYS_LOG(ERROR, "inner stat error, query agent cannot be null");
//          }
//          else
//          {
//            query_agent_->set_failed_fake_range(fake_range);
//          }
//        }
      }

      if (OB_ITER_END == ret)
      {
        local_idx_block_end_ = true;
      }
      return ret;
    }

    int ObIndexLocalAgent::get_next_local_row(const ObRow *&row)
    {
      int ret = OB_SUCCESS;
      const ObRowkey *row_key = NULL;
      {
        ret = sst_scan_.get_next_row(row_key, row);
        if (OB_ITER_END == ret)
        {
          do
          {
            ret = scan();
            if (OB_ITER_END == ret)
            {
              break;
            }
            else if (OB_SUCCESS == ret)
            {
              ret = sst_scan_.get_next_row(row_key, row);
            }
            else
            {
              break;
            }
            if (OB_ITER_END == ret)
            {
              continue;
            }
          } while (OB_SUCCESS != ret);
        }
      }
      if (OB_ITER_END == ret && local_idx_block_end_)
      {
        local_idx_scan_finish_ = true;
        sst_scan_.close();
      }
      return ret;
    }

    int ObIndexLocalAgent::get_next_local_range(ObNewRange &range)
    {
      int ret = OB_SUCCESS;
      bool find = false;
      if (OB_UNLIKELY(NULL == range_server_hash_))
      {
        TBSYS_LOG(WARN, "should not be here");
        ret = OB_ERR_UNEXPECTED;
      }
      if (OB_SUCCESS == ret)
      {
        HashIterator itr = range_server_hash_->begin();
        ObTabletLocationList list;

        int64_t lp = 0;
        for (; itr != range_server_hash_->end(); ++itr)
        {
          ++lp;
          if (lp <= hash_index_)
          {
            continue;
          }
          else
          {
            list = itr->second;
            if (list[0].server_.chunkserver_.get_ipv4() == self_.get_ipv4()
                && list[0].server_.chunkserver_.get_port() == self_.get_port())
            {
              range = (itr->first);
              hash_index_ = lp;
              find = true;
              break;
            }
          }
        }
      }
      if (OB_SUCCESS == ret && !find)
      {
        ret = OB_ITER_END;
      }
      return ret;
    }

    int64_t ObIndexLocalAgent::to_string(char* buf, const int64_t buf_len) const
    {
      int ret = OB_SUCCESS;
      UNUSED(buf);
      UNUSED(buf_len);
      return ret;
    }

    int ObIndexLocalAgent::get_next_row(const ObRow *&row)
    {
      int ret = OB_SUCCESS;
      const ObRow* tmp_row = &cur_row_;
      if (!local_idx_scan_finish_)
      {
        if (OB_SUCCESS == (ret = get_next_local_row(tmp_row)))
        {
          row = tmp_row;
        }
      }
      return ret;
    }

    int ObIndexLocalAgent::get_row_desc(const ObRowDesc *&row_desc) const
    {
      int ret = OB_SUCCESS;
      row_desc = &row_desc_;
      return ret;
    }

    void ObIndexLocalAgent::set_row_desc(const ObRowDesc &desc)
    {
      row_desc_ = desc;
    }

    int ObIndexLocalAgent::set_scan_param(ObScanParam *scan_param)
    {
      int ret = OB_SUCCESS;
      if (scan_param == NULL)
      {
        TBSYS_LOG(ERROR,"scan_param is null");
        ret = OB_ERR_NULL_POINTER;
      }
      scan_param_ = scan_param;
      return ret;
    }

    int ObIndexLocalAgent::set_range_server_hash(const chunkserver::RangeServerHash *range_server_hash)
    {
      int ret = OB_SUCCESS;
      if (range_server_hash == NULL)
      {
        TBSYS_LOG(ERROR,"range_server_hash is null");
        ret = OB_ERR_NULL_POINTER;
      }
      range_server_hash_ = range_server_hash;
      return ret;
    }

    int ObIndexLocalAgent::build_sst_scan_param()
    {
      int ret = OB_SUCCESS;
      ObTabletScan tmp_table_scan;
      ObSqlScanParam ob_sql_scan_param;
      ObArray <uint64_t> basic_columns;
      if (OB_UNLIKELY(NULL == scan_param_))
      {
        TBSYS_LOG(WARN, "null pointer of scan_param!");
        ret = OB_ERROR;
      }
      if (OB_SUCCESS == ret)
      {
        if (OB_SUCCESS != (ret = ob_sql_scan_param.set_range(*(scan_param_->get_range()))))
        {
          TBSYS_LOG(WARN, "set range failed!,ret[%d]", ret);
        }
        else
        {
          uint64_t tid = OB_INVALID_ID;
          uint64_t cid = OB_INVALID_ID;
          for (int64_t i = 0; i < row_desc_.get_column_num(); i++)
          {
            if (OB_SUCCESS != row_desc_.get_tid_cid(i, tid, cid))
            {
              TBSYS_LOG(WARN, "get tid cid failed! ret [%d]", ret);
              break;
            }
            else if (OB_SUCCESS != (ret = basic_columns.push_back(cid)))
            {
              break;
            }
          }
        }
      }

      if (OB_SUCCESS == ret)
      {
        ret = tmp_table_scan.build_sstable_scan_param_pub(
                basic_columns,
                ob_sql_scan_param,
                sst_scan_param_);
      }
      return ret;
    }

    PHY_OPERATOR_ASSIGN(ObIndexLocalAgent)
    {
      int ret = OB_SUCCESS;
      CAST_TO_INHERITANCE(ObIndexLocalAgent);
      reset();
      row_desc_ = o_ptr->row_desc_;
      return ret;
    }

  }        //end sql
}        //end oceanbase

