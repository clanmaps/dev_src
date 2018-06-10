/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cs_interactive_cell_stream.cpp
 * @brief cs <== communicate ==> cs
 *
 * Created by longfei：provide cell stream for operator ObCsInteractiveAgent
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_04
 */

#include "ob_cs_interactive_cell_stream.h"
#include "ob_read_param_modifier.h"

namespace oceanbase
{
  namespace chunkserver
  {

    ObCsInteractiveCellStream::~ObCsInteractiveCellStream()
    {
      // TODO Auto-generated destructor stub
    }

    ObCsInteractiveCellStream::ObCsInteractiveCellStream(
        ObMergerRpcProxy * rpc_proxy, const common::ObServerType server_type, const int64_t time_out) :
      ObCellStream(rpc_proxy, server_type, time_out)
    {
      finish_ = false;
      reset_inner_stat();
    }

    int ObCsInteractiveCellStream::next_cell()
    {
      int ret = OB_SUCCESS;
      if (NULL == scan_param_)
      {
        TBSYS_LOG(DEBUG, "check scan param is null");
        ret = OB_ITER_END;
      }
      else if (!check_inner_stat())
      {
        TBSYS_LOG(ERROR, "check inner stat failed");
        ret = OB_INNER_STAT_ERROR;
      }
      else
      {
        // do while until get scan data or finished or error occured
        /*if(!cur_result_.is_empty())
         {
           ret = cur_result_.next_cell();
         }*/
        do
        {
          ret = get_next_cell();
          if (OB_ITER_END == ret)
          {
            TBSYS_LOG(DEBUG, "scan cell finish");
            break;
          }
          else if (ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "scan next cell failed:ret[%d]", ret);
            break;
          }
        } while (cur_result_.is_empty() && (OB_SUCCESS == ret));
      }
      return ret;
    }

    int ObCsInteractiveCellStream::scan(const ObScanParam & param)
    {
      int ret = OB_SUCCESS;
      const ObNewRange * range = param.get_range();
      if (NULL == range)
      {
        TBSYS_LOG(WARN, "check scan param failed");
        ret = OB_INPUT_PARAM_ERROR;
      }
      else
      {
        reset_inner_stat();
        scan_param_ = &param;
        ret = scan_row_data();
        if (ret != OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "scan server data failed:ret[%d]", ret);
        }
        else
        {
          /*
            * cs return empty scanner, just return OB_ITER_END
            */
          if (cur_result_.is_empty() && !ObCellStream::first_rpc_)
          {
            // check already finish scan
            ret = check_finish_scan(cur_scan_param_);
            if (OB_SUCCESS != ret)
            {
              TBSYS_LOG(WARN, "check finish scan failed:ret[%d]", ret);
            }
            else if (finish_)
            {
              ret = OB_ITER_END;
            }
          }
        }
      }
      return ret;
    }

    int ObCsInteractiveCellStream::get_next_cell(void)
    {
      int ret = OB_SUCCESS;
      if (finish_)
      {
        ret = OB_ITER_END;
        TBSYS_LOG(DEBUG, "check next already finish");
      }
      else
      {
        last_cell_ = cur_cell_;
        ret = cur_result_.next_cell();
        // need rpc call for new data
        if (OB_ITER_END == ret)
        {
          // scan the new data only by one rpc call
          ret = scan_row_data();
          if (OB_SUCCESS == ret)
          {
            ret = cur_result_.next_cell();
            if ((ret != OB_SUCCESS) && (ret != OB_ITER_END))
            {
              TBSYS_LOG(WARN, "get next cell failed:ret[%d]", ret);
            }
            else if (OB_ITER_END == ret)
            {
              TBSYS_LOG(DEBUG, "finish the scan");
            }
          }
          else if (OB_ITER_END != ret)
          {
            TBSYS_LOG(WARN, "scan server data failed:ret[%d]", ret);
          }
        }
        else if (ret != OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "next cell failed:ret[%d]", ret);
        }
      }
      return ret;
    }

    int ObCsInteractiveCellStream::scan_row_data()
    {
      int ret = OB_SUCCESS;
      // step 1. modify the scan param for next scan rpc call
      if (!ObCellStream::first_rpc_)
      {
        // check already finish scan
        ret = check_finish_scan(cur_scan_param_);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "check finish scan failed:ret[%d]", ret);
        }
        else if (finish_)
        {
          ret = OB_ITER_END;
        }
      }

      // step 2. construct the next scan param
      if (OB_SUCCESS == ret)
      {
        ret = get_next_param(*scan_param_, cur_result_, &cur_scan_param_, range_buffer_);
        if (ret != OB_SUCCESS)
        {
          TBSYS_LOG(WARN, "modify scan param failed:ret[%d]", ret);
        }
      }

      // step 3. scan data according the new scan param
      const int64_t MAX_REP_COUNT = 3;
      if(OB_SUCCESS == ret)
      {
        if(ObCellStream::first_rpc_)
        {
          do
          {
            //先去第一副本上scan
            ret = ObCellStream::rpc_scan_row_data(cur_scan_param_, (chunkserver_)[cur_rep_index_].server_.chunkserver_);
            if(OB_SUCCESS != ret)
            {
              cur_rep_index_ ++;// 只要第一副本失败了,从下次开始就换个CS取数据

            }
          } while(OB_TABLET_HAS_NO_LOCAL_SSTABLE == ret && cur_rep_index_ < MAX_REP_COUNT);
        }
        else
        {
          ret = ObCellStream::rpc_scan_row_data(cur_scan_param_, (chunkserver_)[cur_rep_index_].server_.chunkserver_);
          if (ret != OB_SUCCESS)
          {
            TBSYS_LOG(WARN, "scan row data failed:ret[%d]", ret);
          }
        }
      }
      //if(cur_scan_param_.if_need_fake())
      return ret;
    }

    int ObCsInteractiveCellStream::check_finish_scan(const ObScanParam & param)
    {
      int ret = OB_SUCCESS;
      bool is_fullfill = false;
      if (!finish_)
      {
        int64_t item_count = 0;
        ret = ObCellStream::cur_result_.get_is_req_fullfilled(is_fullfill, item_count);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "get scanner full filled status failed:ret[%d]", ret);
        }
        else if (is_fullfill)
        {
          ObNewRange result_range;
          ret = ObCellStream::cur_result_.get_range(result_range);
          if (OB_SUCCESS != ret)
          {
            TBSYS_LOG(WARN, "get result range failed:ret[%d]", ret);
          }
          else
          {
            finish_ = is_finish_scan(param, result_range);
          }
        }
      }
      return ret;
    }

    int64_t ObCsInteractiveCellStream::get_data_version() const
    {
      return cur_result_.get_data_version();
    }

  } /* namespace chunkserver */
} /* namespace oceanbase */
