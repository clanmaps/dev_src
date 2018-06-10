/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_rs_rs_message.cpp
 * @brief message among rootservers for exchanging info about election.
 *        add the auto_elect_flag into the serialization process
 *        in case of the communication between all the rootserver.
 * @version CEDAR 0.2 
 * @author
 *   Chu Jiajia  <52151500014@ecnu.cn>
 *   zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_08_23
 */
#include "ob_rs_rs_message.h"
#include <tbsys.h>

using namespace oceanbase::common;

int ObMsgRsElection::serialize(char* buf, const int64_t buf_len,
                               int64_t &pos) const
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = addr_.serialize(buf, buf_len, pos)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  else if (OB_SUCCESS
      != (ret = serialization::encode_bool(buf, buf_len, pos, auto_elect_flag)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  // add:e
  else if (OB_SUCCESS
      != (ret = serialization::encode_vi64(buf, buf_len, pos, lease_)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  else if (OB_SUCCESS
      != (ret = serialization::encode_vi64(buf, buf_len, pos,
          max_log_timestamp_)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // else if (OB_SUCCESS
  // != (ret = serialization::encode_vi64(buf, buf_len, pos, term_)))
  // {
  // TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  // }
  //delete:e
  else if (OB_SUCCESS
      != (ret = serialization::encode_vi64(buf, buf_len, pos, type_)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  else if (OB_SUCCESS
      != (ret = serialization::encode_vstr(buf, buf_len, pos, server_version_)))
  {
    TBSYS_LOG(ERROR, "failed to serialize, err=%d", ret);
  }
  return ret;
}

int ObMsgRsElection::deserialize(const char* buf, const int64_t data_len,
                                 int64_t &pos)
{
  int ret = OB_SUCCESS;
  int64_t server_version_length = 0;
  const char * server_version_temp = NULL;
  if (OB_SUCCESS != (ret = addr_.deserialize(buf, data_len, pos)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  else if (OB_SUCCESS
      != (ret = serialization::decode_bool(buf, data_len, pos, &auto_elect_flag)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  // add:e
  else if (OB_SUCCESS
      != (ret = serialization::decode_vi64(buf, data_len, pos, &lease_)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  else if (OB_SUCCESS
      != (ret = serialization::decode_vi64(buf, data_len, pos,
          &max_log_timestamp_)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // else if (OB_SUCCESS
  // != (ret = serialization::decode_vi64(buf, data_len, pos, &term_)))
  // {
  //   TBSYS_LOG(ERROR, "deserialize error");
  //   ret = OB_INVALID_ARGUMENT;
  // }
  //delete:e
  else if (OB_SUCCESS
      != (ret = serialization::decode_vi64(buf, data_len, pos, &type_)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  else if (NULL
      == (server_version_temp = serialization::decode_vstr(buf, data_len, pos,
          &server_version_length)))
  {
    TBSYS_LOG(ERROR, "deserialize error");
    ret = OB_INVALID_ARGUMENT;
  }
  else
  {
    memcpy(server_version_, server_version_temp, server_version_length);
  }
  return ret;
}
