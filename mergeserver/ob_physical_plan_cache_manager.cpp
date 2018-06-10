/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_physical_plan_cache_manager.cpp
 * @brief procedure physical plan cache management class definition
 *
 * create by wangdonghui
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

#include "ob_physical_plan_cache_manager.h"
#include "common/hash/ob_hashmap.h"
#include "common/hash/ob_hashtable.h"
#include "common/hash/ob_hashutils.h"
#include "common/ob_rpc_stub.h"
#include "ob_merge_server.h"
#include "sql/ob_procedure_optimizer.h"
#define NAME_CACHE_MAP_BUCKET_NUM 100
#define THREAD_SLEEP_UTIME 1000000

#define LAZY_COMPILE

using namespace oceanbase::mergeserver;
using namespace oceanbase::sql;
ObProcedureManager::ObProcedureManager() : has_init_(false)
{
}

ObProcedureManager::~ObProcedureManager()
{
}
int ObProcedureManager::init()
{
  int ret =  OB_SUCCESS;
  ret = name_cache_map_.create(NAME_CACHE_MAP_BUCKET_NUM);
  if(OB_SUCCESS != ret)
  {
      TBSYS_LOG(WARN, "create cache hash map fail:ret[%d]", ret);
  }
  else
  {
      TBSYS_LOG(INFO, "create cache hash map succ");
  }

  if( OB_SUCCESS != (ret = session_.init(block_allocator_)))
  {
      TBSYS_LOG(WARN, "failed to init session");
  }

  ret = name_code_map_.init();
  if(OB_SUCCESS != ret)
  {
      TBSYS_LOG(WARN, "create code hash map fail:ret[%d]", ret);
  }
  else
  {
      has_init_ = true;
      TBSYS_LOG(INFO, "create code hash map succ");
  }

  return ret;
}


//may be need to synchronize between multiple clients
int ObProcedureManager::compile_procedure(const ObString &proc_name)
{
  int ret = OB_SUCCESS;
  const ObString * psource_code = name_code_map_.get_source_code(proc_name);

  if( psource_code == NULL )
  {
    ret = OB_ENTRY_NOT_EXIST;
  }
  else
  {
    const ObString &proc_source_code = *psource_code;
    sql::ObSQLResultSet &rs = *(this->malloc_result_set());
    sql::ObSqlContext context;
    int64_t schema_version = 0;
    if (OB_SUCCESS !=(ret = mergeserver_service_->get_sql_proxy_().init_sql_env_for_cache(context, schema_version, rs, session_)))
    {
      TBSYS_LOG(WARN, "init sql env error.");
    }
    else
    {
      TBSYS_LOG(INFO, "before compile, proc_name: %.*s", proc_name.length(), proc_name.ptr());
      context.session_info_->get_transformer_mem_pool().end_batch_alloc(true);
      context.session_info_->get_transformer_mem_pool().start_batch_alloc();
      context.is_prepare_protocol_ = true;
      if (OB_SUCCESS != (ret = mergeserver_service_->get_sql_proxy_().execute(proc_source_code, rs, context, schema_version)))
      {
        TBSYS_LOG(WARN, "ms execute sql failed. ret = [%d]", ret);
        context.session_info_->get_transformer_mem_pool().end_batch_alloc(true);
      }
      else
      {
        TBSYS_LOG(TRACE, "MS ExecutionPlan: \n%s", to_cstring(*(rs.get_result_set().get_physical_plan())));
        rs.get_result_set().set_cur_schema_version(schema_version);//add by wdh 20160822
        int hash_ret = put_cache_plan(proc_name, &rs);
        if(hash::HASH_INSERT_SUCC != hash_ret)
        {
          if(hash::HASH_EXIST == hash_ret)
          {
            TBSYS_LOG(WARN, "proc physic al plan has existed! proc name: [%s]", proc_name.ptr());
          }
          else
          {
            ret = OB_ERROR;
            TBSYS_LOG(WARN, "gen physical plan and insert into physical plan manager fail! proc name: [%s]",
                      proc_name.ptr());
          }
        }
        else
        {
          TBSYS_LOG(INFO, "proc[%.*s] physical plan insert hashmap succ!", proc_name.length(), proc_name.ptr());
        }
        context.session_info_->get_transformer_mem_pool().end_batch_alloc(false);
      }
      context.is_prepare_protocol_ = false;
      context.session_info_->get_transformer_mem_pool().start_batch_alloc();
    }
  }
  return ret;
}

int ObProcedureManager::compile_procedure_with_context(const ObString &proc_name, ObSqlContext &context, uint64_t &stmt_id, bool no_group, bool long_trans)
{
  int ret = OB_SUCCESS;
  ObSQLResultSet proc_result_set;
  sql::ObProcedure *proc;
  const ObString * psource_code = name_code_map_.get_source_code(proc_name);
  if( psource_code == NULL )
  {
    TBSYS_LOG(WARN, "procedure code does not exist");
    ret = OB_ENTRY_NOT_EXIST;
  }
  else
  {
    const ObString &proc_source_code = *psource_code;

    ObArenaAllocator* allocator = context.session_info_->get_transformer_mem_pool_for_ps();
    context.transformer_allocator_ = allocator;
    proc_result_set.get_result_set().set_ps_transformer_allocator(allocator);
    context.is_prepare_protocol_ = true;
    if( OB_SUCCESS != (ret = mergeserver_service_->get_sql_proxy_().execute(proc_source_code, proc_result_set, context, 0)) )
    {
      TBSYS_LOG(WARN, "prepare the procedure plan fail");
      context.session_info_->get_transformer_mem_pool().end_batch_alloc(true); //fail, we rollback the memory point
    }
    else if ( NULL == (proc = dynamic_cast<ObProcedure*>(proc_result_set.get_result_set().get_physical_plan()->get_main_query())) )
    {
      //failed to optimize procedure
      ret = OB_ERR_UNEXPECTED;
    }
    else if( OB_SUCCESS != (ret = ObProcedureOptimizer::optimize(*proc, no_group)))
    {
      TBSYS_LOG(WARN, "failed to optimize procedure, no_group[%d]", no_group);
    }
    else
    {
      //set cache state
      proc_result_set.get_result_set().set_no_group(no_group);
      //add by qx 20170317 :b
      proc_result_set.get_result_set().set_long_trans(long_trans);
      //add :e
      proc_result_set.get_result_set().set_stmt_hash(name_code_map_.get_hkey(proc_name));
      proc_result_set.get_result_set().set_cur_schema_version(mergeserver_service_->get_merge_server()->get_schema_mgr()->get_latest_version());//add by wdh 20160822
      TBSYS_LOG(DEBUG, "ob_transformer current schema version for [%.*s] is %ld", proc_name.length(), proc_name.ptr(), proc_result_set.get_result_set().get_cur_schema_version());
      if ((ret = context.session_info_->store_plan(proc_name, proc_result_set.get_result_set())) != OB_SUCCESS)
      {
        TBSYS_LOG(WARN, "Store current result failed.");
      }
      else if( !context.session_info_->plan_exists(proc_name, &stmt_id) )
      {
        TBSYS_LOG(WARN, "Cache plan failed, unexpected error");
        ret = OB_ERR_PREPARE_STMT_UNKNOWN;
      }
      else
      {
        TBSYS_LOG(INFO, "compile [%.*s], indexed with [%ld]", proc_name.length(), proc_name.ptr(), stmt_id);
        TBSYS_LOG(INFO, "After Optimize:\n%s", to_cstring(*proc));
      }

    }
    context.is_prepare_protocol_ = false;
  }
  return ret;
}

void ObProcedureManager::run(tbsys::CThread *thread, void *arg)
{
  UNUSED(thread);
  UNUSED(arg);
  bool stop = false;

  int ret = OB_SUCCESS;
  while(!stop)
  {
    if( OB_SUCCESS != (ret = mergeserver_service_->fetch_source(&name_code_map_)) )
    {
      TBSYS_LOG(WARN, "fail to fecth procedure source, retry after 1 seconds");
    }
    else
    {
#ifndef LAZY_COMPILE
      TBSYS_LOG(INFO, "fecth procedure codes[%ld]", name_code_map_.size());
      for(common::ObNameCodeMap::ObNameCodeIterator iter(name_code_map_); OB_SUCCESS == ret && !iter.end(); iter.next())
      {
        const ObString &proc_name = iter.get_proc_name();

        if( OB_SUCCESS != (ret = compile_procedure(proc_name)) )
        {
          TBSYS_LOG(WARN, "fail to compile procedure[ %s ]", to_cstring(proc_name));
          break;
        }
      }
#endif
    }

    if( OB_SUCCESS != ret )
    {
      usleep(THREAD_SLEEP_UTIME);
    }
    else
    {
      stop = true;
      TBSYS_LOG(INFO, "cache procedure codes completed!");
    }
  }
}

/**
 * @brief ObProcedureManager::do_execute
 * only update a procedure
 * @param name
 * @param source_code
 * @return
 */
int ObProcedureManager::create_procedure(const ObString  &name, const ObString &source_code)
{
  int ret = OB_SUCCESS;
  //delete_procedure(name); //clear old one
  lock_.lock();
  name_code_map_.put_source_code(name, source_code);

#ifndef LAZY_COMPILE
  if( OB_SUCCESS != (ret = compile_procedure(name)))
  {
    TBSYS_LOG(WARN, "compilation fails during updating procedure");
  }
#endif
  lock_.unlock();
  return ret;
}


int ObProcedureManager::delete_procedure(const ObString &proc_name)
{
  lock_.lock();
  name_code_map_.del_source_code(proc_name);

#ifndef LAZY_COMPILE
  del_cache_plan(proc_name);
#endif
  lock_.unlock();
  return OB_SUCCESS;
}

int ObProcedureManager::get_procedure(const ObString &proc_name, ObSQLResultSet * &result_set)
{
  return name_cache_map_.get(proc_name, result_set);
}

int ObProcedureManager::create_procedure_lazy(const ObString &proc_name, const ObString &proc_source_code)
{
  int ret = OB_SUCCESS;
  if( name_code_map_.exist(proc_name) )
  {
    if( OB_SUCCESS != (ret = delete_procedure(proc_name)))
    {
      TBSYS_LOG(WARN, "fail to delete expired procedure");
    }
  }
  if( OB_SUCCESS == ret )
  {
    ret = name_code_map_.put_source_code(proc_name, proc_source_code);
  }
  return ret;
}

int ObProcedureManager::get_procedure_lazy(const ObString &proc_name, ObSqlContext &context, uint64_t &stmt_id, bool no_group, bool long_trans)
{
  int ret = OB_SUCCESS;

  TBSYS_LOG(TRACE, "compile procedure[%.*s]", proc_name.length(), proc_name.ptr());
  if( !name_code_map_.exist(proc_name))
  {
    ret = OB_ERR_SP_DOES_NOT_EXIST;
    TBSYS_LOG(WARN, "procedure %.*s does not exist", proc_name.length(), proc_name.ptr());
  }
  else if( OB_SUCCESS != (ret = compile_procedure_with_context(proc_name, context, stmt_id, no_group, long_trans)) ) //modify by qx 20170317 add long transcation flag
  {
    TBSYS_LOG(WARN, "failed to compile proc[%.*s]", proc_name.length(), proc_name.ptr());
  }
  return ret;
}

int ObProcedureManager::put_cache_plan(const ObString &proc_name, ObSQLResultSet *result_set)
{
  ObString name;
  ob_write_string(proc_name_buf_, proc_name, name);
  TBSYS_LOG(INFO, "add [%.*s] plan", name.length(), name.ptr());
  return name_cache_map_.set(name, result_set);
}

int ObProcedureManager::del_cache_plan(const ObString &proc_name)
{
  int64_t count = name_cache_map_.size();
  name_cache_map_.erase(proc_name);
  TBSYS_LOG(INFO, "delete [%.*s], prev[ %ld ], now[ %ld ]",
            proc_name.length(), proc_name.ptr(), count, name_cache_map_.size());
  return OB_SUCCESS;
}

bool ObProcedureManager::is_consisitent(const ObString &proc_name, const ObResultSet &cache_rs, bool no_group, bool long_trans) const
{
  bool ret = true;
  ret = name_code_map_.exist(proc_name, cache_rs.get_stmt_hash());

  if( ret )
  {
    ret = (no_group == cache_rs.get_no_group());
  }
  //add by qx 20170317 :e
  else if (ret)
  {
    ret = (long_trans == cache_rs.get_long_trans());
  }
  //add :e
  return ret;
}


int ObProcedureManager::refresh_name_node_map()
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = mergeserver_service_->fetch_source(&name_code_map_)))
  {
    TBSYS_LOG(WARN, "fail to fecth procedure source, retry after 1 seconds");
  }
  return ret;
}
