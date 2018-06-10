/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ups_phy_operator.cpp
 * @brief set the default sql_env when a sql is executed in an transaction.
 *
 * mofied by zhutao:add function process for procedure physical plan operator
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

/**
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * Authors:
 *   yuanqi <yuanqi.xhf@taobao.com>
 *     - some work details if you want
 */
#include "ob_ups_phy_operator_factory.h"
#include "ob_ups_lock_filter.h"
#include "ob_ups_inc_scan.h"
#include "ob_memtable_modify.h"
#include "ob_memtable_lock.h" // add wangjiahao [table lock] 20160616
#include "ob_ups_procedure.h" //add zt 20151111
#define new_operator(__type__, __allocator__, ...)      \
  ({                                                    \
    __type__ *ret = NULL;                               \
    void* buf = __allocator__.alloc(sizeof(__type__));  \
    if (NULL != buf)                                    \
    {                                                   \
      ret = new(buf) __type__(__VA_ARGS__);             \
    }                                                   \
    ret;                                                \
   })

namespace oceanbase
{
  using namespace sql;
  namespace updateserver
  {
    ObPhyOperator *ObUpsPhyOperatorFactory::get_one(ObPhyOperatorType type, common::ModuleArena &allocator)
    {
      ObPhyOperator *ret = NULL;
      if (NULL == session_ctx_ || NULL == table_mgr_)
      {
        TBSYS_LOG(ERROR, "param not set: session_ctx=%p, table_mgr=%p", session_ctx_, table_mgr_);
      }
      else
      {
        switch(type)
        {
          case PHY_LOCK_FILTER:
            ret = new_operator(ObUpsLockFilter, allocator, *session_ctx_);
            break;
          case PHY_INC_SCAN:
            {
              ObUpsIncScan *ic = tc_rp_alloc(ObUpsIncScan);
              ic->set_session_ctx(session_ctx_);
              ret = ic;
              break;
            }
          case PHY_UPS_MODIFY:
            ret = new_operator(MemTableModify, allocator, *session_ctx_, *table_mgr_);
            break;
          case PHY_UPS_MODIFY_WITH_DML_TYPE:
            ret = new_operator(MemTableModifyWithDmlType, allocator, *session_ctx_, *table_mgr_);
            break;
          // add wangjiahao [table lock] 20160616 :b
          case PHY_UPS_LOCK_TABLE:
            ret = new_operator(MemTableLock, allocator, *session_ctx_, *table_mgr_);
            break;
          //add :e
          //add zt 20151110:b
          case PHY_PROCEDURE:
            ret = new_operator(ObUpsProcedure, allocator, *session_ctx_);
            break;
            //add zt 20151110:e
          default:
            ret = ObPhyOperatorFactory::get_one(type, allocator);
            break;
        }
      }
      return ret;
    }

    void ObUpsPhyOperatorFactory::release_one(sql::ObPhyOperator *opt)
    {
      if (NULL != opt)
      {
        switch (opt->get_type())
        {
          case PHY_INC_SCAN:
            tc_rp_free(dynamic_cast<ObUpsIncScan*>(opt));
            break;
          default:
            ObPhyOperatorFactory::release_one(opt);
            break;
        }
      }
    }

  }; // end namespace updateserver
}; // end namespace oceanbase
