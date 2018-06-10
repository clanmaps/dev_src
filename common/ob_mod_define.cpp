/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_mod_define.cpp
 * @brief define module
 *
 * modified by longfei： add a module for static index handler
 * modified by maoxiaoxiao:add a module to report tablet histogram
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @date 2016_01_21
 */

#include "ob_mod_define.h"
using namespace oceanbase;
using namespace oceanbase::common;

namespace oceanbase
{
  namespace common
  {
    ExpStat g_malloc_size_stat("size");
  }; // end namespace common
}; // end namespace oceanbase

ObModInfo oceanbase::common::OB_MOD_SET[G_MAX_MOD_NUM]; 

oceanbase::common::ObModSet::ObModSet()
{
  init_ob_mod_set();
  mod_num_ = std::min<int32_t>(ObModIds::OB_MOD_END, G_MAX_MOD_NUM);
}

int32_t oceanbase::common::ObModSet::get_mod_id(const char * mod_name)const
{
  int32_t result = 0;
  if (NULL == mod_name)
  {
    result = 0;
  }
  for (int32_t mod_idx = 0; 
      mod_idx < mod_num_ && 0 == result && NULL != mod_name; 
      mod_idx ++)
  {
    if (strcmp(OB_MOD_SET[mod_idx].mod_name_, mod_name) == 0)
    {
      result = mod_idx;
      break;
    }
  }
  return result;
}

const char * oceanbase::common::ObModSet::get_mod_name(const int32_t mod_id) const
{
  const char *result = NULL;
  if (mod_id < 0 || mod_id >= mod_num_)
  {
    result = OB_MOD_SET[0].mod_name_;
  }
  else
  {
    result = OB_MOD_SET[mod_id].mod_name_;
  }
  return result;
}

int32_t oceanbase::common::ObModSet::get_max_mod_num()const
{
  return mod_num_;
}
