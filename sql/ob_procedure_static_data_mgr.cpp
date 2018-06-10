/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_static_data_mgr.cpp
 * @brief the ObProcedureStaticDataMgr class definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_28
 */

#include "ob_procedure_static_data_mgr.h"

namespace oceanbase
{
  namespace sql
  {
    ObProcedureStaticDataMgr::ObProcedureStaticDataMgr()
    {
    }

    int ObProcedureStaticDataMgr::init()
    {
      int ret = OB_SUCCESS;
      if( OB_SUCCESS != (ret = hkey_idx_map_.create(64)))
      {
        TBSYS_LOG(WARN, "fail to create hkey_idx_map");
      }
      return ret;
    }

    int ObProcedureStaticDataMgr::store(int64_t sdata_id, int64_t hkey, ObRowStore *&p_row_store)
    {
      int ret = OB_SUCCESS;
      int64_t idx = -1;
      TBSYS_LOG(TRACE, "store static data[%ld, %ld]", sdata_id, hkey);
      if ( HASH_NOT_EXIST != hkey_idx_map_.get(hkey, idx))
      {
        TBSYS_LOG(ERROR, "static data has been created, sdata_id:%ld, hkey: %ld", sdata_id, hkey);
        ret = OB_ENTRY_EXIST;
      }
      else
      {
        StaticData *item = (StaticData*)static_store_arena_.alloc(sizeof(StaticData));
        item = new(item) StaticData();
        ret = static_store_.push_back(item);
        idx = static_store_.count() - 1;

        item->id = sdata_id;
        item->hkey = hkey;
        p_row_store = &(item->store);

        hkey_idx_map_.set(hkey, idx);
      }
      return ret;
    }

    int ObProcedureStaticDataMgr::get(int64_t idx, int64_t &sdata_id, int64_t &hkey, const ObRowStore *&p_row_store) const
    {
      int ret = (idx < static_store_.count()) ?
            OB_SUCCESS :
            OB_ERROR;

      if( OB_SUCCESS == ret )
      {
        const StaticData *item = static_store_.at(idx);
        sdata_id = item->id;
        hkey = item->hkey;
        p_row_store = &(item->store);
      }
      else
      {
        sdata_id = -1;
        hkey = -1;
        p_row_store = NULL;
      }
      return ret;
    }

    int ObProcedureStaticDataMgr::get(int64_t sdata_id, int64_t hkey, ObRowStore *&p_row_store)
    {
      int ret = OB_SUCCESS;
      int64_t idx = -1;
      TBSYS_LOG(TRACE, "read static data[%ld, %ld]", sdata_id, hkey);
      if( HASH_NOT_EXIST == hkey_idx_map_.get(hkey, idx) )
      {
        TBSYS_LOG(WARN, "static data does not exists, hkey: %ld", hkey);
      }
      else
      {
        StaticData *item = static_store_.at(idx);
        if( item->id == sdata_id )
        {
          p_row_store = &(item->store);
        }
        else
        {
          TBSYS_LOG(WARN, "sdata_id is not consistent, real: %ld, expected: %ld", item->id, sdata_id);
          p_row_store = NULL;
        }
      }
      return ret;
    }

    int64_t ObProcedureStaticDataMgr::get_static_data_count() const
    {
      return static_store_.count();
    }

    int ObProcedureStaticDataMgr::clear()
    {
      for(int64_t i = 0; i < static_store_.count(); ++i)
      {
        static_store_.at(i)->store.clear();
      }
      static_store_.clear();
      static_store_arena_.free();
      hkey_idx_map_.clear();
      return OB_SUCCESS;
    }
  }
}
