/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_interactive_agent.cpp
 * @brief for define rpc interface between chunkserver like this :) cs <== rpc ==> cs
 *
 * Created by longfei：  interactive agent is for the global stage of constructing static index,
 *  get the data of the range when those datas in other chunkserver.
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_02
 */
#include "ob_index_interactive_agent.h"

namespace oceanbase
{
  namespace chunkserver
  {
    ObIndexInteractiveAgent::ObIndexInteractiveAgent()
    {
      reset();
    }


    void ObIndexInteractiveAgent::reset()
    {
      interactive_cell_ = NULL;
      scan_param_ = NULL;
      inited_ = false;
      range_server_hash_ = NULL;
      hash_index_ = -1;
      column_count_ = -1;
      failed_fake_range_.reset();
      //      curr_row_.reset();
    }

    void ObIndexInteractiveAgent::reuse()
    {
      //@todo
    }

    void ObIndexInteractiveAgent::set_row_desc(const ObRowDesc &desc)
    {
      row_desc_ = desc;
    }

    int ObIndexInteractiveAgent::start_agent(
        ObScanParam &scan_param,
        ObCsInteractiveCellStream &cs_stream,
        const RangeServerHash *hash)
    {
      int ret = OB_SUCCESS;
      reset();
      interactive_cell_ = &cs_stream;
      scan_param_ = &scan_param;
      range_server_hash_ = hash;//赋值赋的是data_multics_range_hash!
      //ObServer chunkserver;
      if (NULL == hash)
      {
        TBSYS_LOG(WARN, "null pointer of range server hash");
        ret = OB_INNER_STAT_ERROR;
      }
      if (OB_SUCCESS == ret)
      {
        inited_ = true;
        HashIterator iter = range_server_hash_->begin();
        bool iter_end = true;
        ObTabletLocationList list;
        int loop = 0;

//        遍历此table的所有tablet，如果说存在tablet的第一副本不在本cs上的话，
//            就将当前位置用hash_index_记录下来,然后设置interactive_cell的cs(组)为此tablet所在所有cs。
//            跳出遍历

        for (; iter != range_server_hash_->end(); ++iter)
        {
          list = iter->second;
          if (list[0].server_.chunkserver_ == interactive_cell_->get_self())
          {
            //直接跳过本cs上的tablet
            loop++;
            continue;
          }
          else
          {
            scan_param_->set_fake(true);
            scan_param_->set_fake_range(iter->first);
            interactive_cell_->set_chunkserver((iter->second));
            column_count_ = scan_param.get_column_id_size();
            hash_index_ = loop;
            iter_end = false;
            break;
          }
        }
        if (iter_end)
        {
          //在本机上的数据就不需要去和其他CS interactive
          ret = OB_ITER_END;
        }
        if (OB_SUCCESS == ret)
        {
          if (OB_SUCCESS != (ret = interactive_cell_->scan(*scan_param_))
              && OB_ITER_END != ret)
          {
            //TODO:failed range
            //set_failed_fake_range(*(scan_param_->get_fake_range()));
            TBSYS_LOG(WARN, "failed to scan firstly batch data,[%d]", ret);
          }
          else if (OB_SUCCESS == ret || OB_ITER_END == ret)
          {
            ret = OB_SUCCESS;
          }
        }
      }
      return ret;
    }

    void ObIndexInteractiveAgent::stop_agent()
    {
      inited_ = false;
      interactive_cell_ = NULL;
      scan_param_ = NULL;
    }

    int ObIndexInteractiveAgent::open()
    {
      int ret = OB_SUCCESS;
      /*
       * @todo(longfei)现在的代码这儿的操作在write_total_index_v1之前就已经做了。
       * 请将代码转移的这儿来做
       */
      curr_row_.set_row_desc(row_desc_);
      return ret;

    }

    int ObIndexInteractiveAgent::close()
    {
      int ret = OB_SUCCESS;
      //@TODO:complete
      return ret;
    }

    int64_t ObIndexInteractiveAgent::to_string(char* buf, const int64_t buf_len) const
    {
      int ret = OB_SUCCESS;
      UNUSED(buf);
      UNUSED(buf_len);
      return ret;
    }

    int ObIndexInteractiveAgent::get_next_row(const ObRow *&row)
    {
      int ret = OB_SUCCESS;
      if (OB_SUCCESS == (ret = get_next_row(curr_row_)))
      {
        row = &curr_row_;
      }
      return ret;
    }

    void ObIndexInteractiveAgent::set_failed_fake_range(const ObNewRange &range)
    {
      failed_fake_range_.reset();
      failed_fake_range_ = range;
    }

    int ObIndexInteractiveAgent::get_cell(ObCellInfo **cell)
    {
      int ret = OB_SUCCESS;
      if (NULL == interactive_cell_)
      {
        TBSYS_LOG(WARN, "null pointer of ObIndexInteractiveAgent");
        ret = OB_ERROR;
      }
      else
      {
        ret = interactive_cell_->get_cell(cell);
      }
      return ret;
    }

    int ObIndexInteractiveAgent::get_cell(ObCellInfo **cell,
                                          bool *is_row_changed)
    {
      int ret = OB_SUCCESS;
      if (NULL == interactive_cell_)
      {
        TBSYS_LOG(WARN, "null pointer of ObCSScanCellStream");
        ret = OB_ERROR;
      }
      else
      {
        ret = interactive_cell_->get_cell(cell, is_row_changed);
      }
      return ret;
    }

    int ObIndexInteractiveAgent::next_cell()
    {
      int ret = OB_SUCCESS;
      if (!inited_ || NULL == interactive_cell_ || NULL == range_server_hash_)
      {
        TBSYS_LOG(WARN,
                  "run start_agent() first, inited_=%d, interactive_cell_=%p",
                  inited_, interactive_cell_);
        ret = OB_INVALID_ARGUMENT;
      }
      else if (OB_SUCCESS == (ret = interactive_cell_->next_cell()))
      {

      }
      else if (OB_ITER_END == ret)
      {
        do
        {
          if (hash_index_ < range_server_hash_->size())
          {
            HashIterator iter = range_server_hash_->begin();
            int32_t loop = 0;
            ObTabletLocationList list;
            bool iter_end = true;
            //TBSYS_LOG(ERROR,"test::whx OB_ITER_END hash_index_ [%d],size[%ld]", hash_index_,range_server_hash_->size());
            for (; iter != range_server_hash_->end(); iter++)
            {

              list = iter->second;
              if (loop <= hash_index_
                  || list[0].server_.chunkserver_
                  == interactive_cell_->get_self())
              {
                loop++;
                //TBSYS_LOG(ERROR,"test::whx show loop hash_index_[%d]",hash_index_);
                continue;
              }
              else
              {
                scan_param_->set_fake(true);
                scan_param_->set_fake_range(iter->first);
                interactive_cell_->set_chunkserver(iter->second);
                hash_index_ = loop;
                iter_end = false;
                break;
              }
            }
            interactive_cell_->reset_inner_stat();
            //TBSYS_LOG(ERROR,"test::whx ret start_agent next scan,hash_index_ [%d],fake_range(%s),range(%s) ret= [%d]", hash_index_,to_cstring(*(scan_param_->get_fake_range())),to_cstring(*(scan_param_->get_range())), ret);
            if (!iter_end)
            {
              ret = interactive_cell_->scan(*scan_param_);
              if (OB_SUCCESS == ret)
              {
                ret = interactive_cell_->next_cell();
                break;
              }
              else if (OB_ITER_END != ret)
              {
                //add [secondary index bug_fix]
                set_failed_fake_range(*(scan_param_->get_fake_range()));
                //add e
                TBSYS_LOG(WARN, "get next cell failed![%d]", ret);
              }
            }
            else
            {
              break;
            }
          }
          else
          {
            interactive_cell_->reset_inner_stat();
          }
        } while (true);
      }
      return ret;
    }

    int ObIndexInteractiveAgent::get_next_row(ObRow &row)
    {
      int ret = OB_SUCCESS;
      ObCellInfo* cell = NULL;
      //bool is_row_changed = false;
      int64_t column_count = 0;
      do
      {
        if (OB_SUCCESS == (ret = next_cell()))
        {
          ret = get_cell(&cell);
        }
        else if (OB_ITER_END == ret)
        {
          TBSYS_LOG(INFO, "interactiveAgent return OB_ITER_END");
          break;
        }
        else
        {
          TBSYS_LOG(WARN, "get cell failed,ret[%d]", ret);
        }

        if (OB_SUCCESS == ret)
        {
          if (OB_SUCCESS
              == (ret = row.set_cell((cell)->table_id_,
                                     (cell)->column_id_,
                                     (cell)->value_)))
          {
          }
          else
          {
            TBSYS_LOG(WARN, "row set cell failed, tid[%ld], cid[%ld]",
                      (cell)->table_id_, (cell)->column_id_);
            break;
          }
        }
        else
        {
          break;
        }
      } while (++column_count < column_count_);

      if (OB_SUCCESS != ret && OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "get_next_row failed[%d]", ret);
      }
      return ret;
    }

    int ObIndexInteractiveAgent::get_failed_fake_range(ObNewRange &range)
    {
      int ret = OB_SUCCESS;
      /*if(NULL == scan_param_)
      {
        TBSYS_LOG(WARN, "scan param is invalid");
        ret = OB_INVALID_ARGUMENT;
      }
      else if(!scan_param_->if_need_fake() || NULL == scan_param_->get_fake_range())
      {
        TBSYS_LOG(WARN, "scan_param is invalid");
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        range = *(scan_param_->get_fake_range());
      }*/
      if(OB_INVALID_ID == failed_fake_range_.table_id_)
      {
        TBSYS_LOG(WARN, "failed fake range is invalid");
        ret = OB_INNER_STAT_ERROR;
      }
      else
      {
        range = failed_fake_range_;
      }
      return ret;
    }
  }
}

