/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_ms_sql_proxy.cpp
 * @brief set the default sql_env when a sql is executed in an transaction.
 *
 * modified by wangdonghui:add function definition for procedure cache management init sql envrinment
 *
 * @version __DaSE_VERSION
 * @author zhangcd <zhangcd_ecnu@ecnu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Time-stamp: <2013-04-17 15:02:26 fufeng.syd>
 * Version: $Id$
 * Filename: ob_ms_sql_proxy.cpp
 *
 * Authors:
 *   Yudi Shi <fufeng.syd@alipay.com>
 *
 */


#include "tbsys.h"
#include "ob_ms_sql_proxy.h"
#include "sql/ob_sql.h"
#include "sql/ob_sql_context.h"
#include "mergeserver/ob_merge_server_service.h"
#include "common/ob_schema_manager.h"

using namespace oceanbase;
using namespace oceanbase::common;
using namespace oceanbase::common::serialization;
using namespace oceanbase::mergeserver;

ObMsSQLProxy::ObMsSQLProxy()
  :ms_service_(NULL),
  rpc_proxy_(NULL),
  root_rpc_(NULL),
  async_rpc_(NULL),
  schema_mgr_(NULL),
  cache_proxy_(NULL)
{
}

int ObMsSQLProxy::execute(const ObString &sqlstr, ObSQLResultSet &rs,
                          sql::ObSqlContext &context, int64_t schema_version)
{
  int ret = OB_SUCCESS;
  sql::ObResultSet &result = rs.get_result_set();
  UNUSED(schema_version);
  if (OB_SUCCESS !=
      (ret = ObSql::direct_execute(sqlstr, result, context)))
  {
    TBSYS_LOG(WARN, "fail to execute ret=%d query [%.*s]",
        ret, sqlstr.length(), sqlstr.ptr());
  }
  if (NULL != context.schema_manager_)
  {
    schema_mgr_->release_schema(context.schema_manager_);
    context.schema_manager_ = NULL;
  }

  rs.set_sqlstr(sqlstr);
  rs.set_errno(ret);
  return ret;
}

void ObMsSQLProxy::set_env(mergeserver::ObMergerRpcProxy  *rpc_proxy,
                           mergeserver::ObMergerRootRpcProxy *root_rpc,
                           mergeserver::ObMergerAsyncRpcStub   *async_rpc,
                           common::ObMergerSchemaManager *schema_mgr,
                           common::ObTabletLocationCacheProxy *cache_proxy,
                           const mergeserver::ObMergeServerService *ms_service)
{
  rpc_proxy_ = rpc_proxy;
  root_rpc_ = root_rpc;
  async_rpc_ = async_rpc;
  schema_mgr_ = schema_mgr;
  cache_proxy_ = cache_proxy;
  ms_service_ = ms_service;
}

int ObMsSQLProxy::init_sql_env(ObSqlContext &context, int64_t &schema_version,
                               ObSQLResultSet &rs, ObSQLSessionInfo &session)
{
  int ret = OB_SUCCESS;
  ObString name = ObString::make_string(OB_READ_CONSISTENCY);
  ObObj type;
  type.set_type(ObIntType);
  ObObj value;
  value.set_int(WEAK);
  ObResultSet &result = rs.get_result_set();
  schema_version = schema_mgr_->get_latest_version();
  context.schema_manager_ = schema_mgr_->get_user_schema(schema_version); // reference count
  if (OB_SUCCESS != (ret = result.init()))
  {
    TBSYS_LOG(ERROR, "init result set error, ret = [%d]", ret);
  }
  else if (NULL == context.schema_manager_)
  {
    TBSYS_LOG(INFO, "table schema not ready, schema_version=%ld", schema_version);
    ret = OB_ERROR;
  }
  else if (OB_SUCCESS != (ret = session.init(block_allocator_)))
  {
    TBSYS_LOG(WARN, "failed to init context, ret=%d", ret);
  }
  else if (OB_SUCCESS != (ret = session.load_system_variable(name, type, value)))
  {
    TBSYS_LOG(ERROR, "load system variable %.*s failed, ret=%d", name.length(), name.ptr(), ret);
  }
  else if (NULL == context.schema_manager_)
  {
    TBSYS_LOG(WARN, "fail to get user schema. schema_version=%ld", schema_version);
    ret = OB_ERROR;
  }
  else
  {
    session.set_version_provider(ms_service_);
    session.set_config_provider(&ms_service_->get_config());

    // add by zcd [multi_cluster] 20150406:b
    /// fix the bug: when we execute sql like 'alter system' through OB_SQL_EXECUTE RPC function,
    /// the function doesn't work, so we should set the value of tx_isolation, ob_tx_timeout and
    /// ob_tx_idle_timeout during the initialization of the session.
    ObObj type_tx_isolation;
    ObString name_str_tx_isolation = ObString::make_string("tx_isolation");
    ObObj value_tx_isolation;
    type_tx_isolation.set_type(ObVarcharType);
    value_tx_isolation.set_varchar(ObString::make_string("READ-COMMITTED"));
    session.load_system_variable(name_str_tx_isolation, type_tx_isolation, value_tx_isolation);

    ObObj type_ob_tx_timeout;
    ObString name_str_ob_tx_timeout = ObString::make_string("ob_tx_timeout");
    ObObj value_ob_tx_timeout;
    type_ob_tx_timeout.set_type(ObIntType);
    value_ob_tx_timeout.set_int(100000000);
    session.load_system_variable(name_str_ob_tx_timeout, type_ob_tx_timeout, value_ob_tx_timeout);

    ObObj type_ob_tx_idle_timeout;
    ObString name_str_ob_tx_idle_timeout = ObString::make_string("ob_tx_idle_timeout");
    ObObj value_ob_tx_idle_timeout;
    type_ob_tx_idle_timeout.set_type(ObIntType);
    value_ob_tx_idle_timeout.set_int(100000000);
    session.load_system_variable(name_str_ob_tx_idle_timeout, type_ob_tx_idle_timeout, value_ob_tx_idle_timeout);
    // add:e

    context.session_info_ = &session;
    context.session_info_->set_current_result_set(&result);

    context.cache_proxy_ = cache_proxy_;    // thread safe singleton
    context.async_rpc_ = async_rpc_;        // thread safe singleton
    context.merger_rpc_proxy_ = rpc_proxy_; // thread safe singleton
    context.rs_rpc_proxy_ = root_rpc_;      // thread safe singleton
    context.merge_service_ = ms_service_;
    context.disable_privilege_check_ = true;
    // add by zcd [multi_cluster] 20150406:b
    context.merger_schema_mgr_ = schema_mgr_;
    // add:e

    // reuse memory pool for parser
    context.session_info_->get_parser_mem_pool().reuse();
    context.session_info_->get_transformer_mem_pool().start_batch_alloc();
  }
  return ret;
}

int ObMsSQLProxy::cleanup_sql_env(ObSqlContext &context, ObSQLResultSet &rs)
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS != (ret = rs.close()))
  {
    TBSYS_LOG(WARN, "close result set error, ret: [%d]", ret);
  }
  if (OB_SUCCESS != (ret = rs.reset()))
  {
    TBSYS_LOG(WARN, "reuse result set error, ret: [%d]", ret);
  }
  OB_ASSERT(context.session_info_);
  context.session_info_->get_parser_mem_pool().reuse();
  context.session_info_->get_transformer_mem_pool().end_batch_alloc(true);
  return ret;
}

//add by zt 20160321 modified by wangdonghui
int ObMsSQLProxy::init_sql_env_for_cache(ObSqlContext &context, int64_t &schema_version,
                                         ObSQLResultSet &rs, ObSQLSessionInfo &session)
{
  int ret = OB_SUCCESS;
  ObString name = ObString::make_string(OB_READ_CONSISTENCY);
  ObObj type;
  type.set_type(ObIntType);
  ObObj value;
  value.set_int(WEAK);
  ObResultSet &result = rs.get_result_set();
  schema_version = schema_mgr_->get_latest_version();
  context.schema_manager_ = schema_mgr_->get_user_schema(schema_version); // reference count
  if (OB_SUCCESS != (ret = result.init()))
  {
    TBSYS_LOG(ERROR, "init result set error, ret = [%d]", ret);
  }
  else if (NULL == context.schema_manager_)
  {
    TBSYS_LOG(INFO, "table schema not ready, schema_version=%ld", schema_version);
    ret = OB_ERROR;
  }
  else if (NULL == context.schema_manager_)
  {
    TBSYS_LOG(WARN, "fail to get user schema. schema_version=%ld", schema_version);
    ret = OB_ERROR;
  }
  else
  {
    session.set_version_provider(ms_service_);
    session.set_config_provider(&ms_service_->get_config());
    context.session_info_ = &session;
    context.session_info_->set_current_result_set(&result);

    context.cache_proxy_ = cache_proxy_;    // thread safe singleton
    context.async_rpc_ = async_rpc_;        // thread safe singleton
    context.merger_rpc_proxy_ = rpc_proxy_; // thread safe singleton
    context.rs_rpc_proxy_ = root_rpc_;      // thread safe singleton
    context.merge_service_ = ms_service_;
    context.disable_privilege_check_ = true;
    // reuse memory pool for parser
    context.session_info_->get_parser_mem_pool().reuse();
    context.session_info_->get_transformer_mem_pool().start_batch_alloc();
  }
  return ret;
}
//end
