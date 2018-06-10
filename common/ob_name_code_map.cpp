/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_name_code_map.h
 * @brief define ObNameCodeMap class for procedure cache management
 *
 * Created by wangdonghui
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_29
 */

#include "ob_name_code_map.h"
#include "common/hash/ob_hashmap.h"
#include "common/hash/ob_hashtable.h"
#include "common/hash/ob_hashutils.h"

#define NAME_CODE_MAP_BUCKET_NUM 100

using namespace oceanbase::common;

ObNameCodeMap::ObNameCodeMap()
{
    local_version=0;
}
ObNameCodeMap::~ObNameCodeMap()
{
}
int ObNameCodeMap::init()
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = name_code_map_.create(NAME_CODE_MAP_BUCKET_NUM)))
  {
      TBSYS_LOG(WARN, "create name code map fail:ret=[%d]", ret);
  }
  else if ( OB_SUCCESS != (ret = name_hash_map_.create(NAME_CODE_MAP_BUCKET_NUM)) )
  {
    TBSYS_LOG(WARN, "create name hash map fail:ret=[%d]", ret);
  }
  else
  {
      is_ready_ = false;
      local_version=0;
      TBSYS_LOG(INFO, "create name code map succ");
  }
  return ret;
}

bool ObNameCodeMap::is_created()
{
  return name_code_map_.created();
}

bool ObNameCodeMap::exist(const ObString &proc_name) const
{
  return name_code_map_.get(proc_name) != NULL;
}

bool ObNameCodeMap::exist(const ObString &proc_name, int64_t hash_code) const
{
  const int64_t *hc = NULL;
  if ( NULL == (hc = name_hash_map_.get(proc_name)) )
  {
    return false;
  }
  else if ( *hc != hash_code )
  {
    return false;
  }
  return true;
}

int64_t ObNameCodeMap::size() const
{
  return name_code_map_.size();
}

int ObNameCodeMap::put_source_code(const ObString &proc_name, const ObString &sour_code)
{
  int ret = OB_SUCCESS;

  ObString name, code;

  arena_.write_string(proc_name, &name);
  arena_.write_string(sour_code, &code);
  int64_t create_ts = tbsys::CTimeUtil::getTime();
  if (hash::HASH_INSERT_SUCC != (ret = name_hash_map_.set(name, create_ts, 0, 0, 1)))
  {
      TBSYS_LOG(DEBUG, "QAQ: name_hash_map_.set ret=%d", ret);
  }
  if (hash::HASH_INSERT_SUCC != (ret = name_code_map_.set(name, code, 0, 0, 1)))
  {
      TBSYS_LOG(DEBUG, "QAQ: name_code_map_.set ret=%d", ret);
  }
  local_version = local_version + (int64_t)1;
  TBSYS_LOG(DEBUG, "QAQ: current version is %ld", local_version);
  return OB_SUCCESS;
}

int64_t ObNameCodeMap::get_hkey(const ObString &proc_name) const
{
  int64_t ret = -1;
  if ( common::hash::HASH_EXIST != name_hash_map_.get(proc_name, ret))
  {
    ret = -1;
  }
  return ret;
}

int ObNameCodeMap::del_source_code(const ObString &proc_name)
{
  int ret = OB_SUCCESS;
  if(hash::HASH_INSERT_SUCC != (ret = name_hash_map_.erase(proc_name)))
  {
      TBSYS_LOG(DEBUG, "QAQ: name_hash_map_.erase ret=%d", ret);
  }
  if(hash::HASH_INSERT_SUCC != (ret = name_code_map_.erase(proc_name)))
  {
      TBSYS_LOG(DEBUG, "QAQ: name_code_map_.erase ret=%d", ret);
  }
  local_version = local_version + (int64_t)1;
  TBSYS_LOG(DEBUG, "QAQ: current version is %ld", local_version);
  return OB_SUCCESS;
}

const ObString * ObNameCodeMap::get_source_code(const ObString &proc_name)
{
  return name_code_map_.get(proc_name);
}

int ObNameCodeMap::serialize(char* buf, const int64_t buf_len, int64_t& pos) const
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(INFO, "the name code map[%p] size is %ld", &name_code_map_, name_code_map_.size());
  if (OB_SUCCESS != (ret = serialization::encode_vi64(buf, buf_len, pos, name_code_map_.size())))
  {
    TBSYS_LOG(WARN, "failed to serialize size, err=%d buf_len=%ld pos=%ld",
              ret, buf_len, pos);
  }
  if (OB_SUCCESS == ret)
  {
    common::hash::ObHashMap<common::ObString, common::ObString >::const_iterator iter = name_code_map_.begin();
    for(;iter != name_code_map_.end(); iter++)
    {
      ObString proc_name = iter->first;
      ObString proc_source_code = iter->second;
      TBSYS_LOG(INFO, "serialize proc name %.*s", proc_name.length(), proc_name.ptr());
      TBSYS_LOG(INFO, "serialize proc source code %.*s", proc_source_code.length(), proc_source_code.ptr());
      if(OB_SUCCESS != (ret = proc_name.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "failed to serialize proc_name, err=%d buf_len=%ld pos=%ld",
                  ret, buf_len, pos);
      }
      else if(OB_SUCCESS != (ret = proc_source_code.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "failed to serialize proc_source_code, err=%d buf_len=%ld pos=%ld",
                  ret, buf_len, pos);
      }
    }
    if(ret == OB_SUCCESS)
    {
        if (OB_SUCCESS != (ret = serialization::encode_vi64(buf, buf_len, pos, local_version)))
        {
          TBSYS_LOG(WARN, "failed to serialize local version, err=%d buf_len=%ld pos=%ld",
                    ret, buf_len, pos);
        }
    }
  }
  return ret;
}
int ObNameCodeMap::deserialize(const char* buf, const int64_t buf_len, int64_t& pos)
{
  int ret = OB_SUCCESS;
  int64_t size = 0;
  ObString proc_source_code;
  ObString proc_name;
  if (OB_SUCCESS != (ret = serialization::decode_vi64(buf, buf_len, pos, &size)))
  {
      TBSYS_LOG(WARN, "failed to decode size, err=%d buf_len=%ld pos=%ld",
                ret, buf_len, pos);
  }
  else
  {
    TBSYS_LOG(INFO, "the name code map size is %ld", size);
    ret = reset();
    for (int64_t i = 0; i < size; i ++)
    {
      //ObString &proc_name = *(this->malloc_string());
      //ObString &proc_source_code = *(this->malloc_string());
      if (OB_SUCCESS != (ret = proc_name.deserialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "failed to deserialize proc_name, err=%d buf_len=%ld pos=%ld",
                  ret, buf_len, pos);
      }
      else if (OB_SUCCESS != (ret = proc_source_code.deserialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "failed to deserialize proc_source_code, err=%d buf_len=%ld pos=%ld",
                  ret, buf_len, pos);
      }
      else
      {
        TBSYS_LOG(INFO, "deserialize proc name %.*s", proc_name.length(), proc_name.ptr());
        TBSYS_LOG(INFO, "deserialize proc source code %.*s", proc_source_code.length(), proc_source_code.ptr());
        put_source_code(proc_name, proc_source_code);
      }
    }
    if(ret == OB_SUCCESS)
    {
        if (OB_SUCCESS != (ret = serialization::decode_vi64(buf, buf_len, pos, &local_version)))
        {
            TBSYS_LOG(WARN, "failed to decode version, err=%d buf_len=%ld pos=%ld",
                      ret, buf_len, pos);
        }
    }
  }
  return ret;
}
