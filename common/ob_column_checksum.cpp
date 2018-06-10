/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_column_checksum.h
* @brief for column checksum of table
*
* Created by maoxiaoxiao:operations to column checksum, such as add, compare and so on
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_COLUMN_CHECKSUM_CPP
#define OB_COLUMN_CHECKSUM_CPP

#include "ob_column_checksum.h"
#include "tbsys.h"
#include "common/ob_define.h"

using namespace oceanbase::common;

int ObColumnChecksum::tokenize(const char *data, int64_t dlen, char delima, int &token_nr, Token *tokens)
{
  int ret = 0;
  int64_t pos = 0;
  int token = 0;
  const char *ptoken = NULL;

  if (!data || !token_nr || !tokens)
  {
    ret = -1;
    TBSYS_LOG(ERROR, "parameters must be well inited");
    return ret;
  }

  while (pos < dlen)
  {
    ptoken = data + pos;

    for (; data[pos] != delima && data[pos] != '\0' && pos < dlen; pos++);
    tokens[token].token = ptoken;
    tokens[token].len = data + pos - ptoken;
    token++;
    pos++;
  }
  if (token > token_nr)
  {
    TBSYS_LOG(ERROR, "token num mustn't be higher than %d", token_nr);
    ret = -1;
  }
  token_nr = token;
  return ret;
}

int ObColumnChecksum::transform_str_to_int(const char* data, const int64_t &dlen,
                                           uint64_t &value)
{
  int ret = 0;
  int64_t pos = 0;
  value = 0;
  uint64_t pre_value = value;
  if (!data || dlen <= 0)
  {
    ret = -1;
    fprintf(stderr, "input invalid numeric string, NULL or len < 0\n");
    return ret;
  }
  else
  {
    while (pos < dlen)
    {
      if (data[pos] >= '0' && data[pos] <= '9')
      {
        pre_value = value;
        value = value * 10 + static_cast <uint64_t>(data[pos] - '0');
        if (value < pre_value || value > UINT64_MAX)
        {
          TBSYS_LOG(ERROR,
                    "value overflow, legal range[%lu, %lu], input invalid numeric string[%.*s]",
                    (uint64_t) 0, UINT64_MAX, static_cast <int>(dlen), data);
          value = 0;
          ret = -1;
          break;
        }
        pos++;
      }
      else
      {
        value = 0;
        ret = -1;
        TBSYS_LOG(ERROR,
                  "input invalid numeric string[%.*s], invalid character(non numeric)=[%c]",
                  (int) dlen, data, data[pos]);
        break;
      }
    }
  }
  return ret;
}

int ObColumnChecksum::add(const ObColumnChecksum &col)
{
  int ret = OB_SUCCESS;

  if (0 == strlen(checksum_str))
  {
    strcpy(checksum_str, col.checksum_str);
  }
  else
  {
    int token_nr_src = OB_MAX_COL_CHECKSUM_COLUMN_COUNT;
    Token tokens_src[OB_MAX_COL_CHECKSUM_COLUMN_COUNT];

    int token_nr_add = OB_MAX_COL_CHECKSUM_COLUMN_COUNT;
    Token tokens_add[OB_MAX_COL_CHECKSUM_COLUMN_COUNT];

    if (OB_SUCCESS != (ret = tokenize(checksum_str, strlen(checksum_str), ',', token_nr_src, tokens_src)))
    {
      TBSYS_LOG(ERROR, "failed to parse src column checksum_str[%s]", checksum_str);
      ret = OB_ERROR;
    }
    else if (OB_SUCCESS != (ret = tokenize(col.checksum_str, strlen(col.checksum_str), ',', token_nr_add, tokens_add)))
    {
      TBSYS_LOG(ERROR, "failed to parse add column checksum_str[%s]", col.checksum_str);
      ret = OB_ERROR;
    }
    else if (token_nr_src != token_nr_add)
    {
      TBSYS_LOG(ERROR, "column checksum count is not equal, src column count[%d], add column count[%d], src column checksum_str[%s], add column checksum_str[%s]",
                token_nr_src, token_nr_add, checksum_str, col.checksum_str);
      ret = OB_ERROR;
    }
    else
    {
      /*存储add column checksum_str中所有<cid, checksum>*/
      std::map<uint64_t, uint64_t> col_sum_add;
      for (int idx = 0; idx < token_nr_add && OB_SUCCESS == ret; idx++)
      {
        int attr_nr = 2;
        Token tokens_attr[2];
        if (0 != (ret = tokenize(tokens_add[idx].token, tokens_add[idx].len, ':', attr_nr, tokens_attr)))
        {
          TBSYS_LOG(ERROR, "fialed to separate cid:checksum[%.*s], add column checksum_str[%s]", static_cast<int>(tokens_add[idx].len), tokens_add[idx].token, col.checksum_str);
          ret = OB_ERROR;
          break;
        }
        else if (2 != attr_nr)
        {
          TBSYS_LOG(ERROR, "column checksum must be 2 attributes, in manner of <cid:checksum>, column checksum[%.*s], add column checksum_str[%s]", static_cast<int>(tokens_add[idx].len), tokens_add[idx].token, col.checksum_str);
          ret = OB_ERROR;
          break;
        }
        else
        {
          uint64_t cid = 0;
          uint64_t checksum = 0;
          if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[0].token, tokens_attr[0].len, cid)))
          {
            TBSYS_LOG(ERROR, "fetch column id failed, column id str[%.*s], add column checksum_str[%s]", static_cast<int>(tokens_attr[0].len), tokens_attr[0].token, col.checksum_str);
            ret = OB_ERROR;
            break;
          }
          else if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[1].token, tokens_attr[1].len, checksum)))
          {
            TBSYS_LOG(ERROR, "fetch checksum  failed, checksum str[%.*s], add column checksum_str[%s]", static_cast<int>(tokens_attr[1].len), tokens_attr[1].token, col.checksum_str);
            ret = OB_ERROR;
            break;
          }
          else
          {
            std::map<uint64_t, uint64_t>::iterator iter = col_sum_add.find(cid);
            if (iter != col_sum_add.end())
            {
              TBSYS_LOG(ERROR, "column[cid = %lu] already exists in add column checksum_str[%s]", cid, col.checksum_str);
              ret = OB_ERROR;
              break;
            }
            else
            {
              col_sum_add.insert(std::make_pair(cid, checksum));
            }
          }
        }
      }
      /*存储src column checksum中所有<cid,checksum>*/
      std::vector<std::pair<uint64_t, uint64_t> > col_sum_src;
      for (int idx = 0; idx < token_nr_src && OB_SUCCESS == ret; idx++)
      {
        int attr_nr = 2;
        Token tokens_attr[2];
        if (OB_SUCCESS != (ret = tokenize(tokens_src[idx].token, tokens_src[idx].len, ':', attr_nr, tokens_attr)))
        {
          TBSYS_LOG(ERROR, "fialed to separate cid:checksum[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_src[idx].len), tokens_src[idx].token, checksum_str);
          ret = OB_ERROR;
          break;
        }
        else if (attr_nr != 2)
        {
          TBSYS_LOG(ERROR, "column checksum must be 2 attributes, in manner of <cid:checksum>, column checksum[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_src[idx].len), tokens_src[idx].token, checksum_str);
          ret = OB_ERROR;
          break;
        }
        else
        {
          uint64_t cid = 0;
          uint64_t checksum = 0;
          if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[0].token, tokens_attr[0].len, cid)))
          {
            TBSYS_LOG(ERROR, "fetch column id failed, column id str[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_attr[0].len), tokens_attr[0].token, checksum_str);
            ret = OB_ERROR;
            break;
          }
          else if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[1].token, tokens_attr[1].len, checksum)))
          {
            TBSYS_LOG(ERROR, "fetch checksum  failed, checksum str[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_attr[1].len), tokens_attr[1].token, checksum_str);
            ret = OB_ERROR;
            break;
          }
          else
          {
            //找出cid在col_sum_add中对应的值,并与checksum相加,如果找不到报错
            std::map<uint64_t, uint64_t>::iterator iter = col_sum_add.find(cid);
            if (iter != col_sum_add.end())
            {
              checksum += iter->second;
              col_sum_src.push_back(std::make_pair(cid, checksum));
            }
            else
            {
              TBSYS_LOG(ERROR, "can't find equal column id[cid = %lu], src column checksum_str[%s], add column checksum_str[%s]", cid, checksum_str, col.checksum_str);
              ret = OB_ERROR;
              break;
            }
          }
        }
      }
      if (OB_SUCCESS == ret)
      {
        /*生成新的checksum_str*/
        int pos = 0;
        for (size_t idx = 0; idx < col_sum_src.size(); idx++)
        {
          int length = snprintf(checksum_str + pos, OB_MAX_COL_CHECKSUM_STR_LEN, "%lu:%lu", col_sum_src[idx].first, col_sum_src[idx].second);
          pos += length;
          checksum_str[pos] = ',';
          pos++;
        }
        pos--;
        checksum_str[pos] = '\0';
      }
    }
  }
  return ret;
}

int ObColumnChecksum::equal(const ObColumnChecksum &col, bool &is_equal)
{
  int ret = OB_SUCCESS;
  is_equal = false;
  if (!strlen(checksum_str) || !strlen(col.checksum_str))
  {
    if (!strlen(checksum_str) && !strlen(col.checksum_str))
      is_equal = true;
  }

  if(strlen(checksum_str) && strlen(col.checksum_str))
  {
    int token_nr_src = OB_MAX_COL_CHECKSUM_COLUMN_COUNT;
    Token tokens_src[OB_MAX_COL_CHECKSUM_COLUMN_COUNT];
    int token_nr_cmp = OB_MAX_COL_CHECKSUM_COLUMN_COUNT;
    Token tokens_cmp[OB_MAX_COL_CHECKSUM_COLUMN_COUNT];

    if (OB_SUCCESS != (ret = tokenize(checksum_str, strlen(checksum_str), ',', token_nr_src, tokens_src)))
    {
      TBSYS_LOG(ERROR, "failed to parse src column checksum_str[%s]", checksum_str);
      ret = OB_ERROR;
    }
    else if (OB_SUCCESS != (ret = tokenize(col.checksum_str, strlen(col.checksum_str), ',', token_nr_cmp, tokens_cmp)))
    {
      TBSYS_LOG(ERROR, "failed to parse compare column checksum_str[%s]", col.checksum_str);
      ret = OB_ERROR;
    }
    else
    {
      /*存储src column checksum中所有<cid,checksum>*/
      std::map<uint64_t, uint64_t> col_sum_src;
      for (int idx = 0; idx < token_nr_src && OB_SUCCESS == ret; idx++)
      {
        int attr_nr = 2;
        Token tokens_attr[2];
        if (OB_SUCCESS != (ret = tokenize(tokens_src[idx].token, tokens_src[idx].len, ':', attr_nr, tokens_attr)))
        {
          TBSYS_LOG(ERROR, "fialed to separate cid:checksum[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_src[idx].len), tokens_src[idx].token, checksum_str);
          ret = OB_ERROR;
          break;
        }
        else if (2 != attr_nr)
        {
          TBSYS_LOG(ERROR, "column checksum must be 2 attributes, in manner of <cid:checksum>, column checksum[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_src[idx].len), tokens_src[idx].token, checksum_str);
          ret = OB_ERROR;
          break;
        }
        else
        {
          uint64_t cid = 0;
          uint64_t checksum = 0;
          if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[0].token, tokens_attr[0].len, cid)))
          {
            TBSYS_LOG(ERROR, "fetch column id failed, column id str[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_attr[0].len), tokens_attr[0].token, checksum_str);
            ret = OB_ERROR;
            break;
          }
          else if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[1].token, tokens_attr[1].len, checksum)))
          {
            TBSYS_LOG(ERROR, "fetch checksum failed, checksum str[%.*s], src column checksum_str[%s]", static_cast<int>(tokens_attr[1].len), tokens_attr[1].token, checksum_str);
            ret = OB_ERROR;
            break;
          }
          else
          {
            std::map<uint64_t, uint64_t>::iterator iter = col_sum_src.find(cid);
            if (iter != col_sum_src.end())
            {
              TBSYS_LOG(ERROR, "column[cid = %lu] already exists in src column checksum_str[%s]", cid, checksum_str);
              ret = OB_ERROR;
              break;
            }
            else
            {
              col_sum_src.insert(std::make_pair(cid, checksum));
            }
          }
        }
      }
      /*存储compare column checksum_str中所有<cid, checksum>*/
      std::map<uint64_t, uint64_t> col_sum_cmp;
      for (int idx = 0; idx < token_nr_cmp && OB_SUCCESS == ret; idx++)
      {
        int attr_nr = 2;
        Token tokens_attr[2];
        if (OB_SUCCESS != (ret = tokenize(tokens_cmp[idx].token, tokens_cmp[idx].len, ':', attr_nr, tokens_attr)))
        {
          TBSYS_LOG(ERROR, "fialed to separate cid:checksum[%.*s], compare column checksum_str[%s]", static_cast<int>(tokens_cmp[idx].len), tokens_cmp[idx].token, col.checksum_str);
          ret = OB_ERROR;
          break;
        }
        else if (attr_nr != 2)
        {
          TBSYS_LOG(ERROR, "column checksum must be 2 attributes, in manner of <cid:checksum>, column checksum[%.*s], compare column checksum_str[%s]", static_cast<int>(tokens_cmp[idx].len), tokens_cmp[idx].token, col.checksum_str);
          ret = OB_ERROR;
          break;
        }
        else
        {
          uint64_t cid = 0;
          uint64_t checksum = 0;
          if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[0].token, tokens_attr[0].len, cid)))
          {
            TBSYS_LOG(ERROR, "fetch column id failed, column id str[%.*s], compare column checksum_str[%s]", static_cast<int>(tokens_attr[0].len), tokens_attr[0].token, col.checksum_str);
            ret = OB_ERROR;
            break;
          }
          else if (OB_SUCCESS != (ret = transform_str_to_int(tokens_attr[1].token, tokens_attr[1].len, checksum)))
          {
            TBSYS_LOG(ERROR, "fetch checksum failed, checksum str[%.*s], compare column checksum_str[%s]", static_cast<int>(tokens_attr[1].len), tokens_attr[1].token, col.checksum_str);
            ret = OB_ERROR;
            break;
          }
          else
          {
            std::map<uint64_t, uint64_t>::iterator iter = col_sum_cmp.find(cid);
            if (iter != col_sum_cmp.end())
            {
              TBSYS_LOG(ERROR, "column[cid = %lu] already exists in compare column checksum_str[%s]", cid, col.checksum_str);
              ret = OB_ERROR;
              break;
            }
            else
            {
              col_sum_cmp.insert(std::make_pair(cid, checksum));
            }
          }
        }
      }
      /*比较校验和是否相等*/
      is_equal = true;
      if (OB_SUCCESS == ret)
      {
        if (col_sum_cmp.size() <= col_sum_src.size())
        {
          std::map<uint64_t, uint64_t>::iterator iter = col_sum_cmp.begin();
          while (iter != col_sum_cmp.end())
          {
            std::map<uint64_t, uint64_t>::iterator iter_temp = col_sum_src.find(iter->first);
            if (iter_temp != col_sum_src.end())
            {
              if (iter->second != iter_temp->second)
              {
                TBSYS_LOG(ERROR, "equal column id[cid = %lu], but not equal checksum, src column checksum_str[%s], compare column checksum_str[%s]", iter->first, checksum_str, col.checksum_str);
                is_equal = false;
                break;
              }
            }
            else
            {
              TBSYS_LOG(ERROR, "can't find equal column id[cid = %lu], src column checksum_str[%s], compare column checksum_str[%s]", iter->first, checksum_str, col.checksum_str);
              is_equal = false;
              break;
            }
            iter++;
          }
        }
        else
        {
          std::map<uint64_t, uint64_t>::iterator iter = col_sum_src.begin();
          while (iter != col_sum_src.end())
          {
            std::map<uint64_t, uint64_t>::iterator iter_temp = col_sum_cmp.find(iter->first);
            if (iter_temp != col_sum_cmp.end())
            {
              if (iter->second != iter_temp->second)
              {
                TBSYS_LOG(ERROR, "equal column id[cid = %lu], but not equal checksum, src column checksum_str[%s], compare column checksum_str[%s]", iter->first, checksum_str, col.checksum_str);
                is_equal = false;
                break;
              }
            }
            else
            {
              TBSYS_LOG(ERROR, "can't find equal column id[cid = %lu], src column checksum_str[%s], compare column checksum_str[%s]", iter->first, checksum_str, col.checksum_str);
              is_equal = false;
              break;
            }
            iter++;
          }
        }
      }
    }
  }
  //mod e
  return ret;
}

void ObColumnChecksum::deepcopy(const char* col, const int32_t len)
{
  reset();
  if(NULL != col)
  {
    strncpy(checksum_str, col, len);
  }
  if(static_cast<int64_t>(strlen(col)) < OB_MAX_COL_CHECKSUM_STR_LEN - 1)
  {
    checksum_str[strlen(col)] = '\0';
  }
}

void ObColumnChecksum::deepcopy(const char* col)
{
  reset();
  if(NULL != col)
  {
    strncpy(checksum_str, col, strlen(col));
  }
  if(static_cast<int64_t>(strlen(col)) < OB_MAX_COL_CHECKSUM_STR_LEN - 1)
  {
    checksum_str[strlen(col)] = '\0';
  }
}


#endif // OB_COLUMN_CHECKSUM_CPP
