/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_define.h
 * @brief modify the construction function of class ObBootstrap.
 *
 * @version CEDAR 0.2 
 * @author zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_12_30
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_root_bootstrap.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_ROOT_BOOTSTRAP_H
#define _OB_ROOT_BOOTSTRAP_H 1
#include "common/ob_schema_service.h"
#include "common/ob_array.h"
#include "common/roottable/ob_first_tablet_entry_meta.h"
#include "common/ob_schema_service_impl.h"
#include "common/roottable/ob_scan_helper.h"
#include "ob_root_rpc_stub.h"
#include "ob_ups_manager.h"
#include "ob_root_log_worker.h"

namespace oceanbase
{
  namespace rootserver
  {
    class ObRootServer2;
    typedef common::ObArray<common::ObServer> ObServerArray;
    class ObBootstrap
    {
      public:
        // modify by zcd [multi_cluster] 20150406:b
        /// add the parameter root_worker
        ObBootstrap(ObRootServer2 & root_server, ObRootWorker &root_worker);
        // modify:e
        ~ObBootstrap();
        int init_schema_service(void);
        int bootstrap_core_tables(void);
        int bootstrap_sys_tables(void);
        int bootstrap_ini_tables(void);
        int init_system_table(void);
        int init_meta_file(const ObServerArray & created_cs);
        int init_all_cluster();
        int init_all_sys_config_stat();
        int64_t get_table_count(void) const;
        void set_log_worker(ObRootLogWorker * log_worker);
      private:
        int create_all_core_tables(void);
        int create_sys_table(common::TableSchema & table_schema);
        int create_core_table(const uint64_t table_id, ObServerArray & created_cs);
      private:
        static const int64_t STRING_VALUE_LENGTH = 128;
        int init_users();
        int init_all_sys_param();
        int init_all_sys_stat();
        DISALLOW_COPY_AND_ASSIGN(ObBootstrap);
      private:
        ObRootServer2 & root_server_;
        ObRootLogWorker * log_worker_;
        int64_t core_table_count_;
        int64_t sys_table_count_;
        int64_t ini_table_count_;
        common::ObSchemaServiceImpl schema_service_;
        // add by zcd [multi_cluster] 20150406:b
        ObRootWorker *root_worker_;
        // add:e
    };
  } // end namespace rootserver
} // end namespace oceanbase

#endif /* _OB_ROOT_BOOTSTRAP_H */
