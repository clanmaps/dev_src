/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_sql_proxy.h
 * @brief modify the contruct function of ObRootSQLProxy
 * @version CEDAR 0.2 
 * @author zhangcd <zhangcd_ecnu@ecnu.cn>
 * @date 2015_12_30
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
  *   zhidong <xielun.szd@taobao.com>
  *     - some work details if you want
  */

#ifndef OB_ROOT_SQL_PROXY_H_
#define OB_ROOT_SQL_PROXY_H_

#include "ob_root_ms_provider.h"

namespace oceanbase
{
  namespace common
  {
    class ObString;
  }
  namespace rootserver
  {
    class ObRootRpcStub;
    class ObChunkServerManager;
    // thread safe sql proxy
    class ObRootSQLProxy
    {
    public:
      // modify by zcd [multi_cluster] 20150405:b
      /**
       * @brief the constructed function of class ObRootSQLProxy
       * @param[in] server_manager
       * @param[in] config
       * @param[in] rpc_stub
       * @param[in] root_worker
       */
      ObRootSQLProxy(ObChunkServerManager & server_manager, ObRootServerConfig &config, ObRootRpcStub & rpc_stub, ObRootWorker &root_worker);
      // modify:e
      virtual ~ObRootSQLProxy();
    public:
      // exectue sql query
      int query(const int64_t retry_times, const int64_t timeout, const common::ObString & sql);
      int query(const bool query_master_cluster, const int64_t retry_times, const int64_t timeout, const common::ObString & sql);
      friend class ObRootInnerTableTask;
    private:
      ObRootMsProvider ms_provider_;
      ObRootRpcStub & rpc_stub_;
    };
  }
}

#endif //OB_ROOT_SQL_PROXY_H_
