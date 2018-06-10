/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_physical_plan_cache_manager.h
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

#ifndef OB_MERGESERVER_PHYSICAL_PLAN_CAHCE_MANAGER_H_
#define OB_MERGESERVER_PHYSICAL_PLAN_CAHCE_MANAGER_H_

#include "tbsys.h"
#include "common/ob_string.h"
#include "common/hash/ob_hashmap.h"
#include "sql/ob_sql_result_set.h"
#include "common/ob_name_code_map.h"
#include "ob_merge_server_service.h"

namespace oceanbase
{
  namespace mergeserver
  {
    class ObMergerAsyncRpcStub;  ///<  class declare
    /**
     * @brief The ObProcedureManager class
     * procedure cache manager class
     */
    class ObProcedureManager : public tbsys::CDefaultRunnable
    {
      public:
        typedef hash::ObHashMap<ObString, sql::ObSQLResultSet *> ObProcCache;  ///<  typedef ObProcCache type
        /**
         * @brief run
         * at the start of mergeserver, it will try to fetch the whole name_code_map from rootserver.
         * @param thread unused
         * @param arg  unused
         */
        virtual void run(tbsys::CThread* thread, void * arg);
        /**
         * @brief ObProcedureManager constructor
         */
        ObProcedureManager();
        /**
         * @brief destructor
         */
        virtual ~ObProcedureManager();
        /**
         * @brief init
         * initlize ObProcedureManager object
         * @return
         */
        int init();
        /**
         * @brief set_ms_service
         * set merge server service object
         * @param msservice ObMergeServerService object point
         */
        void set_ms_service(ObMergeServerService *msservice)
        {
            mergeserver_service_ = msservice;
        }
        /**
         * @brief create_procedure
         * create a procedure
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @return  error code
         */
        int create_procedure(const ObString &proc_name, const ObString &proc_source_code);
        /**
         * @brief delete_procedure
         * delete a procedure from name code map
         * @param proc_name procedure name
         * @return error code
         */
        int delete_procedure(const ObString &proc_name);
        /**
         * @brief get_procedure
         * get procedure
         * @param proc_name procedure name
         * @param result_set returned result
         * @return error code
         */
        int get_procedure(const ObString &proc_name, ObSQLResultSet *& result_set);
        /**
         * @brief get_procedure_source
         * get procedure source code
         * @param proc_name procedure name
         * @return procedure source code
         */
        const ObString * get_procedure_source(const ObString &proc_name) { return name_code_map_.get_source_code(proc_name); }
        /**
         * @brief create_procedure_lazy
         * put procedure name and source code to map when function called
         * @param proc_name  procedure name
         * @param proc_source_code procedure source code
         * @return error code
         */
        int create_procedure_lazy(const ObString &proc_name, const ObString &proc_source_code);
        /**
         * @brief get_procedure_lazy
         * try to compile procedure on behalf of session, the plan is indexed by stmt_id in session's cache pool.
         * @param proc_name procedure name
         * @param context sql context
         * @param stmt_id physical id
         * @param no_group no group execution flag
         * @param long_trans long transcation flag
         * @return error code
         */
        int get_procedure_lazy(const ObString &proc_name, ObSqlContext &context, uint64_t &stmt_id, bool no_group, bool long_trans); //modify by qx 20170317 add long transcation
        /**
         * @brief is_consisitent
         * check consistency
         * @param proc_name procedure name
         * @param cache_rs cache result set
         * @param no_group no group execution flag
         * @param long_trans long transcation execution flag
         * @return bool value
         */
        bool is_consisitent(const ObString &proc_name, const ObResultSet &cache_rs, bool no_group ,bool long_trans) const; //modify by qx 20170317 add long transcation

        //      int serialize(char* buf, const int64_t data_len, int64_t& pos) const;
        //      int deserialize(const char* buf, const int64_t data_len, int64_t& pos);
        /**
         * @brief refresh_name_node_map
         * refresh name_node_map
         * @return error code
         */
        /**
         * @brief refresh_name_node_map
         * @return
         */
        int refresh_name_node_map();
        /**
         * @brief reset
         * @return
         */
        int reset(){return name_code_map_.reset();}
        /**
         * @brief get_version
         * @return local version
         */
        int64_t get_version()
        {
            return name_code_map_.get_local_version();
        }
        mutable tbsys::CThreadMutex lock_;  ///<  thread lock

        ObNameCodeMap * get_name_code_map()
        {
            return &name_code_map_;
        }

      private:
        /**
         * @brief compile_procedure
         * compile procedure to physical plan
         * @param proc_name procedure name
         * @return error code
         */
        int compile_procedure(const ObString &proc_name);
        /**
         * @brief compile_procedure_with_context
         * compile procedure with sql context to physical plan
         * @param proc_name procedure name
         * @param context sql context
         * @param stmt_id  physical plan id
         * @param no_group  no group execution flag
         * @return error code
         */
        int compile_procedure_with_context(const ObString &proc_name, ObSqlContext &context, uint64_t &stmt_id, bool no_group = false, bool long_trans = false);
        /**
         * @brief malloc_result_set
         * malloc memery for  plan result set
         * @return  result set
         */
        sql::ObSQLResultSet * malloc_result_set()
        {
          void* ptr = arena_.alloc(sizeof(sql::ObSQLResultSet));
          sql::ObSQLResultSet *ret = NULL;
          ret = new(ptr) sql::ObSQLResultSet();
          return ret;
        }
        /**
         * @brief put_cache_plan
         * put physical plan cache
         * @param proc_name procedure name
         * @param result_set physical plan cache
         * @return error code
         */
        int put_cache_plan(const ObString &proc_name, ObSQLResultSet *result_set);
        /**
         * @brief del_cache_plan
         * delete physical plan cache from map
         * @param proc_name procedure name
         * @return error code
         */
        int del_cache_plan(const ObString &proc_name);

      private:
        /**
         * @brief ObProcedureManager copy constructor disable
         */
        ObProcedureManager(const ObProcedureManager &);
        /**
         * @brief operator =
         * = operator overload disable
         * @return ObProcedureManager object
         */
        const ObProcedureManager & operator = (const ObProcedureManager &);

        ModuleArena arena_;  ///< result memery allocator
        ObStringBuf proc_name_buf_;  ///<  procedure name buffer
        mergeserver::ObMergeServerService * mergeserver_service_;  ///<  merge server service object point
        common::DefaultBlockAllocator block_allocator_;  ///<  block allocator
        ObSQLSessionInfo session_;  ///<  sql session
        ObProcCache name_cache_map_;  ///<  name cache map
        ObNameCodeMap name_code_map_;  ///<
        bool has_init_;  ///<  initlized flag
    };
  }
}

#endif //OB_MERGESERVER_PHYSICAL_PLAN_CAHCE_MANAGER_H_
