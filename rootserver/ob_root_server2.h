/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_server2.h
 * @brief root server
 *
 * modified by longfei：add create_index() and drop_indexs()
 * 
 * modified by wenghaixing: add some function for secondary index construction
 * 1.int get_init_index(const int64_t version, ObArray<uint64_t> *list);  ///< get all int index to list
 * 2.int get_table_from_index(int64_t index_id, uint64_t &table_id);      ///< get original table for index
 * 3.int write_tablet_info_list_to_rt(ObTabletInfoList **tablet_info_list, const int32_t list_size);///< write tablet into roottable
 * 4.bool check_static_index_over(); ///< check if static index build over
 *
 * modified by maoxiaoxiao:add functions to check column checksum, clean column checksum and get column checksum in root server
 *
 * modified by guojinwei,chujiajia and zhangcd:
 *         support multiple clusters for HA by adding or modifying
 *        some functions, member variables
 * modified by wangdonghui:add some functions for procedure
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         chujiajia <52151500014@ecnu.cn>
 *         zhangcd <zhangcd_ecnu@ecnu.cn>
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_26
 */

/*===============================================================
 *   (C) 2007-2010 Taobao Inc.
 *
 *
 *   Version: 0.1 2010-09-26
 *
 *   Authors:
 *          daoan(daoan@taobao.com)
 *          maoqi(maoqi@taobao.com)
 *          xielun.szd(xielun@alipay.com)
 *
 *
 ================================================================*/
#ifndef OCEANBASE_ROOTSERVER_OB_ROOT_SERVER2_H_
#define OCEANBASE_ROOTSERVER_OB_ROOT_SERVER2_H_
#include <tbsys.h>
#include <vector>
#include "common/ob_define.h"
#include "common/ob_server.h"
#include "common/ob_array.h"
#include "common/ob_string.h"
#include "common/ob_scan_param.h"
#include "common/ob_get_param.h"
#include "common/ob_obi_role.h"
#include "common/ob_ups_info.h"
#include "common/ob_schema_table.h"
#include "common/ob_schema_service.h"
#include "common/ob_table_id_name.h"
#include "common/ob_array.h"
#include "common/ob_list.h"
#include "common/ob_bypass_task_info.h"
#include "common/ob_timer.h"
#include "common/ob_tablet_info.h"
#include "common/ob_trigger_event.h"
#include "common/ob_trigger_msg.h"
#include "ob_root_server_state.h"
#include "common/roottable/ob_root_table_service.h"
#include "common/roottable/ob_first_tablet_entry_meta.h"
#include "common/roottable/ob_scan_helper_impl.h"
#include "common/ob_schema_service.h"
#include "common/ob_spin_lock.h"
#include "common/ob_strings.h"
#include "ob_chunk_server_manager.h"
#include "ob_root_table2.h"
#include "ob_root_log_worker.h"
#include "ob_root_async_task_queue.h"
#include "ob_ups_manager.h"
#include "ob_ups_heartbeat_runnable.h"
#include "ob_ups_check_runnable.h"
#include "ob_root_balancer.h"
#include "ob_root_balancer_runnable.h"
#include "ob_root_ddl_operator.h"
#include "ob_daily_merge_checker.h"
#include "ob_heartbeat_checker.h"
#include "ob_root_server_config.h"
#include "ob_root_ms_provider.h"
#include "ob_rs_after_restart_task.h"
#include "ob_schema_service_ms_provider.h"
#include "ob_schema_service_ups_provider.h"
#include "rootserver/ob_root_operation_helper.h"
#include "rootserver/ob_root_timer_task.h"
// add by guojinwei [lease between rs and ups][multi_cluster] 20150908:b
#include "common/ob_election_role_mgr.h"
#include "common/ob_cluster_mgr.h"
// add:e
//add wenghaixing [secondary index.static_index]20151211
#include "common/ob_tablet_histogram_report_info.h"
//add e

//add by wangdonghui 20160229 [physical plan cahce management] :b
#include "common/ob_name_code_map.h"
//add :e
class ObBalanceTest;
class ObBalanceTest_test_n_to_2_Test;
class ObBalanceTest_test_timeout_Test;
class ObBalanceTest_test_rereplication_Test;
class ObBalanceTest_test_n_to_2_with_faulty_dest_cs_Test;
class ObDeleteReplicasTest_delete_in_init_Test;
class ObDeleteReplicasTest_delete_when_rereplication_Test;
class ObDeleteReplicasTest_delete_when_report_Test;
class ObBalanceTest_test_shutdown_servers_Test;
class ObRootServerTest;

namespace oceanbase
{
  namespace common
  {
    class ObSchemaManagerV2;
    class ObRange;
    class ObTabletInfo;
    class ObTabletLocation;
    class ObScanner;
    class ObCellInfo;
    class ObTabletReportInfoList;
    class ObTableIdNameIterator;
    class ObConfigManager;
    struct TableSchema;
  }
  namespace rootserver
  {
    class ObBootstrap;
    class ObRootTable2;
    class ObRootServerTester;
    class ObRootWorker;
    class ObRootServer2;
    // 参见《OceanBase自举流程》
    class ObBootState
    {
      public:
        enum State
        {
          OB_BOOT_NO_META = 0,
          OB_BOOT_OK = 1,
          OB_BOOT_STRAP = 2,
          OB_BOOT_RECOVER = 3,
        };
      public:
        ObBootState();
        bool is_boot_ok() const;
        void set_boot_ok();
        void set_boot_strap();
        void set_boot_recover();
        bool can_boot_strap() const;
        bool can_boot_recover() const;
        const char* to_cstring() const;
        ObBootState::State get_boot_state() const;
      private:
        mutable common::ObSpinLock lock_;
        State state_;
    };

    class ObRootServer2
    {
      public:
        friend class ObBootstrap;
        static const int64_t DEFAULT_SAFE_CS_NUMBER = 2;
        static const char* ROOT_TABLE_EXT;
        static const char* CHUNKSERVER_LIST_EXT;
        static const char* LOAD_DATA_EXT;
        static const char* SCHEMA_FILE_NAME;
        static const char* TMP_SCHEMA_LOCATION;
      public:
        ObRootServer2(ObRootServerConfig& config);
        virtual ~ObRootServer2();

        bool init(const int64_t now, ObRootWorker* worker);

        //add chujiajia [rs_election][multi_cluster] 20150823:b
        bool is_master_ups_lease_valid();
        bool get_is_have_inited();
        // add:e
        int start_master_rootserver();
        int init_first_meta();
        int init_boot_state();
        void start_threads();
        void stop_threads();
        void start_merge_check();

        // oceanbase bootstrap
        int boot_strap();
        int do_bootstrap(ObBootstrap & bootstrap);
        int boot_recover();
        ObBootState* get_boot();
        ObBootState::State get_boot_state() const;
        int slave_boot_strap();
        int start_notify_switch_schema();
        int notify_switch_schema(bool only_core_tables, bool force_update = false);

        //add by wangdonghui 20160122 :b
        /**
         * @brief notify_update_cache
         * notify all ms update cache of peocedure
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @return error code
         */
        int notify_update_cache(const common::ObString & proc_name, const common::ObString & proc_source_code);
        //add :e

        //add by wangdonghui 20160305 :b
        /**
         * @brief notify_delete_cache
         * notify all ms delete cache of peocedure
         * @param proc_name procedure name
         * @return error code
         */
        int notify_delete_cache(const common::ObString & proc_name);
        //add :e


        //add by qx 20160830 :b
        /**
         * @brief get_UpsManager
         * get ObUpsManager object
         * @return ObUpsManager pointer
         */
        inline ObUpsManager * get_ups_manager() const
        {
          return ups_manager_;
        }
        //add :e


        void set_privilege_version(const int64_t privilege_version);
        // commit update inner table task
        void commit_task(const ObTaskType type, const common::ObRole role, const common::ObServer & server, int32_t sql_port,
                         const char* server_version, const int32_t cluster_role = 0);
        // for monitor info
        // add by guojinwei [obi role switch][multi_cluster] 20150916:b
        /**
         * @brief commit a inner table task with cluster id
         * @param[in] type  task type
         * @param[in] role  server type
         * @param[in] server  server
         * @param[in] sql_port  port
         * @param[in] server_version  the string of server type
         * @param[in] cluster_id  cluster id
         * @param[in] cluster_role  cluster role
         */
        void commit_cluster_task(const ObTaskType type, const common::ObRole role, const common::ObServer & server, int32_t sql_port,
                                 const char* server_version, const int64_t cluster_id, const int32_t cluster_role = 0);
        // add:e
        int64_t get_table_count(void) const;
        void get_tablet_info(int64_t & tablet_count, int64_t & row_count, int64_t & date_size) const;
        int change_table_id(const int64_t table_id, const int64_t new_table_id=0);
        int change_table_id(const ObString& table_name, const uint64_t new_table_id);
        
        // for bypass
        // start_import is called in master cluster by rs_admin, this method will call import on all clusters
        int start_import(const ObString& table_name, const uint64_t table_id, ObString& uri);
        int import(const ObString& table_name, const uint64_t table_id, ObString& uri, const int64_t start_time);
        // start_kill_import is called in master cluster by rs_admin, this method will call kill_import on all clusters
        int start_kill_import(const ObString& table_name, const uint64_t table_id);
        int kill_import(const ObString& table_name, const uint64_t table_id);
        // used by master cluster to check slave cluster's import status
        int get_import_status(const ObString& table_name, const uint64_t table_id, ObLoadDataInfo::ObLoadDataStatus& status);
        int set_import_status(const ObString& table_name, const uint64_t table_id, const ObLoadDataInfo::ObLoadDataStatus status);

        bool is_loading_data() const;

        int write_schema_to_file();
        int trigger_create_table(const uint64_t table_id = 0);
        int trigger_drop_table(const uint64_t table_id);
        int force_create_table(const uint64_t table_id);
        int force_drop_table(const uint64_t table_id);
        int check_schema();

        //add by wdh 20160730 :b
        int trigger_create_procedure();
        //add :e
        /*
         * 从本地读取新schema, 判断兼容性
         */
        //int switch_schema(const int64_t time_stamp, common::ObArray<uint64_t> &deleted_tables);
        /*
         * 切换过程中, update server冻结内存表 或者chunk server 进行merge等耗时操作完成
         * 发送消息调用此函数
         */
        int waiting_job_done(const common::ObServer& server, const int64_t frozen_mem_version);
        /*
         * chunk server register
         * @param out status 0 do not start report 1 start report
         */
        int regist_chunk_server(const common::ObServer& server, const char* server_version, int32_t& status, int64_t timestamp = -1);
        /*
         * merge server register
         * @param out status 0 do not start report 1 start report
         */
        int regist_merge_server(const common::ObServer& server, const int32_t sql_port, const bool is_listen_ms,
            const char* server_version, int64_t timestamp = -1);
        /*
         * chunk server更新自己的磁盘情况信息
         */
        int update_capacity_info(const common::ObServer& server, const int64_t capacity, const int64_t used);
        /*
         * 迁移完成操作
         */
        const common::ObServer& get_self() { return my_addr_; }
       ObRootBalancer* get_balancer() { return balancer_; }

       virtual int migrate_over(const int32_t result, const ObDataSourceDesc& desc,
           const int64_t occupy_size, const uint64_t crc_sum, const uint64_t row_checksum, const int64_t row_count);
        /// if (force_update = true && get_only_core_tables = false) then read new schema from inner table
        int get_schema(const bool froce_update, bool get_only_core_tables, common::ObSchemaManagerV2& out_schema);
        //add by wangdonghui 20160307 :b
        /**
         * @brief get_procedure
         * get name_code_map_
         * @param namecodemap  ObNameCodeMap object
         * @return error
         */
        int get_procedure(common::ObNameCodeMap& namecodemap);
        //add :e
        int64_t get_schema_version() const;
        const ObRootServerConfig& get_config() const;
        ObConfigManager* get_config_mgr();
        int64_t get_privilege_version() const;
        int get_max_tablet_version(int64_t &version) const;
        int64_t get_config_version() const;
        int64_t get_alive_cs_number();
        common::ObSchemaManagerV2* get_ini_schema() const;
        ObRootRpcStub& get_rpc_stub();
        int fetch_mem_version(int64_t &mem_version);
        int create_empty_tablet(common::TableSchema &tschema, common::ObArray<common::ObServer> &cs);
        int get_table_id_name(common::ObTableIdNameIterator *table_id_name, bool& only_core_tables);
        int get_table_schema(const uint64_t table_id, common::TableSchema &table_schema);
        int find_root_table_key(const uint64_t table_id, const common::ObString& table_name, const int32_t max_key_len,
            const common::ObRowkey& key, common::ObScanner& scanner);

        int find_root_table_key(const common::ObGetParam& get_param, common::ObScanner& scanner);
        int find_monitor_table_key(const common::ObGetParam& get_param, common::ObScanner& scanner);
        int find_session_table_key(const common::ObGetParam& get_param, common::ObScanner& scanner);
        int find_statement_table_key(const common::ObGetParam& get_param, common::ObScanner& scanner);
        int find_root_table_range(const common::ObScanParam& scan_param, common::ObScanner& scanner);
        virtual int report_tablets(const common::ObServer& server, const common::ObTabletReportInfoList& tablets, const int64_t time_stamp);
        //add wenghaixing [secondary index.static_index]20151118
        int get_init_index(const int64_t version, ObArray<uint64_t> *list);
        int get_table_from_index(int64_t index_id, uint64_t &table_id);
        int write_tablet_info_list_to_rt(ObTabletInfoList **tablet_info_list, const int32_t list_size);
        bool check_static_index_over();
        //add e
        int receive_hb(const common::ObServer& server, const int32_t sql_port, const bool is_listen_ms, const common::ObRole role);
        common::ObServer get_update_server_info(bool use_inner_port) const;
        int add_range_for_load_data(const common::ObList<common::ObNewRange*> &range);
        int load_data_fail(const uint64_t new_table_id);
        int get_table_id(const ObString table_name, uint64_t& table_id);
        int load_data_done(const ObString table_name, const uint64_t old_table_id);
        int get_master_ups(common::ObServer &ups_addr, bool use_inner_port);
        int table_exist_in_cs(const uint64_t table_id, bool &is_exist);
        int create_tablet(const common::ObTabletInfoList& tablets);
        uint64_t get_table_info(const common::ObString& table_name, int32_t& max_row_key_length) const;
        int get_table_info(const common::ObString& table_name, uint64_t& table_id, int32_t& max_row_key_length);

        int64_t get_time_stamp_changing() const;
        int64_t get_lease() const;
        int get_server_index(const common::ObServer& server) const;
        int get_cs_info(ObChunkServerManager* out_server_manager) const;
        // get task queue
        ObRootAsyncTaskQueue * get_task_queue(void);
        const ObChunkServerManager &get_server_manager(void) const;
        void clean_daily_merge_tablet_error();
        void set_daily_merge_tablet_error(const char* msg_buff, const int64_t len);
        char* get_daily_merge_error_msg();
        bool is_daily_merge_tablet_error() const;
        void print_alive_server() const;
        bool is_master() const;
        common::ObFirstTabletEntryMeta* get_first_meta();
        void dump_root_table() const;
        //add wenghaixing [secondary index.static index]20151207
        void dump_root_table(const int32_t index) const;
       // int get_rt_tablet_info(const int32_t meta_index, const ObTabletInfo *&tablet_info) const;
       // int get_meta_index(const common::ObTabletInfo &tablet_info, int32_t &meta_index);
        tbsys::CRWLock& get_root_table_lock(){return root_table_rwlock_;}
        ObRootTable2* get_root_table(){return root_table_;}
        tbsys::CThreadMutex& get_root_table_build_lock(){return root_table_build_mutex_;}
        tbsys::CThreadMutex& get_ddl_lock(){return ddl_tool_.get_ddl_lock();}
        int modify_index_stat(const uint64_t index_tid, const IndexStatus stat);
        int modify_index_stat(const ObArray<uint64_t> &index_tid_list, const IndexStatus stat);
        int modify_index_stat_amd();//after merge done
        int modify_init_index();
        int modify_staging_index();
        int check_tablet_version_v2(const uint64_t table_id, const int64_t tablet_version, const int64_t safe_count, bool &is_merged) const;
        int clean_old_checksum(int64_t current_version);
        int check_column_checksum(const int64_t index_table_id);
        int modify_index_process_info(const uint64_t index_tid, const IndexStatus stat);
        int get_rt_tablet_info(const int32_t meta_index, const ObTabletInfo *&tablet_info) const;
        //add e
        bool check_root_table(const common::ObServer &expect_cs) const;
        int dump_cs_tablet_info(const common::ObServer & cs, int64_t &tablet_num) const;
        void dump_unusual_tablets() const;
        int check_tablet_version(const int64_t tablet_version, const int64_t safe_count, bool &is_merged) const;
        int use_new_schema();
        // dump current root table and chunkserver list into file
        int do_check_point(const uint64_t ckpt_id);
        // recover root table and chunkserver list from file
        int recover_from_check_point(const int server_status, const uint64_t ckpt_id);
        int receive_new_frozen_version(int64_t rt_version, const int64_t frozen_version,
            const int64_t last_frozen_time, bool did_replay);
        int report_frozen_memtable(const int64_t frozen_version, const int64_t last_frozen_time,bool did_replay);
        int get_last_frozen_version_from_ups(const bool did_replay);
        // 用于slave启动过程中的同步
        void wait_init_finished();
        const common::ObiRole& get_obi_role() const;
        int request_cs_report_tablet();
        int set_obi_role(const common::ObiRole& role);
        int get_master_ups_config(int32_t &master_master_ups_read_percent,
            int32_t &slave_master_ups_read_percent) const;
        const common::ObUpsList &get_ups_list() const;
        int set_ups_list(const common::ObUpsList &ups_list);
        int do_stat(int stat_key, char *buf, const int64_t buf_len, int64_t& pos);
        int register_ups(const common::ObServer &addr, int32_t inner_port, int64_t log_seq_num, int64_t lease, const char *server_version_);
        int receive_ups_heartbeat_resp(const common::ObServer &addr, ObUpsStatus stat,
            const common::ObiRole &obi_role);
        int ups_slave_failure(const common::ObServer &addr, const common::ObServer &slave_addr);
        int get_ups_list(common::ObUpsList &ups_list);
        int set_ups_config(const common::ObServer &ups, int32_t ms_read_percentage, int32_t cs_read_percentage);
        int set_ups_config(int32_t read_master_master_ups_percentage, int32_t read_slave_master_ups_percentage);
        int change_ups_master(const common::ObServer &ups, bool did_force);
        int serialize_cs_list(char* buf, const int64_t buf_len, int64_t& pos) const;
        int serialize_ms_list(char* buf, const int64_t buf_len, int64_t& pos) const;
        int serialize_proxy_list(char* buf, const int64_t buf_len, int64_t& pos) const;
        int grant_eternal_ups_lease();
        // add by guojinwei [lease between rs and ups][multi_cluster] 20150820:b
        /**
         * @brief grant ups lease
         * @param[in] did_force  whether it is necessary
         * @return OB_SUCCESS if success
         */
        int grant_ups_lease(bool did_force = false);

        /**
         * @brief get election role manager
         * @return the reference of election_role_
         */
        const common::ObElectionRoleMgr &get_election_role() const;

        /**
         * @brief update election role
         * @param[in] role  election role
         * @return OB_SUCCESS if success
         */
        int set_election_role_with_role(const common::ObElectionRoleMgr::Role &role);

        /**
         * @brief update election state
         * @param[in] state  election state
         * @return OB_SUCCESS if success
         */
        int set_election_role_with_state(const common::ObElectionRoleMgr::State &state);
        // add:e
        // add by guojinwei [obi role switch][multi_cluster] 20150915:b
        /**
         * @brief update cluster role of slave clusters in inner table
         * @return OB_SUCCESS if success
         */
        int set_slave_cluster_obi_role();
        // add:e
        // add by guojinwei [reelect][multi_cluster] 20151129:b
        /**
         * @brief get cluster manager
         * @param[out] cluster_mgr  the pointer reference of ObClusterMgr
         * @return OB_SUCCESS if success
         */
        int get_cluster_mgr(ObClusterMgr *&cluster_mgr);

        /**
         * @brief whether the clusters are ready for election
         * This function is only called by master cluster.
         * @return OB_SUCCESS if the clusters are ready for election
         */
        int is_clusters_ready_for_election();
        // add:e
        int cs_import_tablets(const uint64_t table_id, const int64_t tablet_version);
        /// force refresh the new schmea manager through inner table scan
        int refresh_new_schema(int64_t & table_count);
        int switch_ini_schema();
        int renew_user_schema(int64_t & table_count);

        //add by wangdonghui 20160307 :b
        /**
         * @brief renew_procedure_info
         * refresh procedure
         * @return error code
         */
        int renew_procedure_info();
        /**
         * @brief refresh_new_procedure
         * force refresh physical plan manager
         * @return error code
         */
        int refresh_new_procedure();
        //add :e
        int renew_core_schema(void);
        void dump_schema_manager();
        void dump_migrate_info() const; // for monitor
        int shutdown_cs(const common::ObArray<common::ObServer> &servers, enum ShutdownOperation op);
        void restart_all_cs();
        void cancel_restart_all_cs();
        int cancel_shutdown_cs(const common::ObArray<common::ObServer> &servers, enum ShutdownOperation op);
        void reset_hb_time();
        int remove_replica(const bool did_replay, const common::ObTabletReportInfo &replica);
        int delete_tables(const bool did_replay, const common::ObArray<uint64_t> &deleted_tables);
        int delete_replicas(const bool did_replay, const common::ObServer & cs, const common::ObTabletReportInfoList & replicas);
        int create_table(const bool if_not_exists, const common::TableSchema &tschema);
        int alter_table(common::AlterTableSchema &tschema);
        int drop_tables(const bool if_exists, const common::ObStrings &tables);
        //add hxlong [Truncate Table]:20170318:b
        int truncate_tables(const bool if_exists, const common::ObStrings &tables, const common::ObString &user, const common::ObString & comment);
        int truncate_one_table( const common::ObMutator & mutator);
        //add:e
        //add by wangdonghui 20160121 :b
        /**
         * @brief create_procedure
         * create procedure throuth ddl tool
         * @param if_exists is a flag
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @return error code
         */
        int create_procedure(const bool if_exists, const common::ObString proc_name, const common::ObString proc_source_code);
        //add :e

        //add by wangdonghui 20160225 [drop procedure] :b
        /**
         * @brief drop_procedure
         * drop procedure throuth ddl tool
         * @param if_exists is a flag
         * @param proc_name procedure name
         * @return error code
         */
        int drop_procedure(bool if_exists, const common::ObString &proc_name);
        //add :e

        //add by wangdonghui 20160304 :b
        /**
         * @brief get_name_code_map
         * get name_code_map_
         * @return  ObNameCodeMap pointer
         */
        common::ObNameCodeMap* get_name_code_map();
        //add :e
        int64_t get_last_frozen_version() const;
        // add by zcd [multi_cluster] 20150416:b
        // 返回slave_array_的内容
        std::vector<common::ObServer> get_slave_root_cluster_ip();
        // 从slave_array_中移除当前rs的地址
        int remove_current_rs_from_slaves_array();
        int parse_string_to_ips(const char *str, std::vector<ObServer>& slave_array);
        // add:e
        // longfei [create index]
        //secodary index service
        /**
         * @brief create_index: sql api
         * @param if_not_exists
         * @param tschema
         * @return error code
         */
        int create_index(const bool if_not_exists, const common::TableSchema &tschema);
        /**
         * @brief drop_indexs: sql api
         * @param if_exists
         * @param indexs
         * @return error code
         */
        int drop_indexs(const bool if_exists, const common::ObStrings &indexs);
        /**
         * @brief drop_one_index: drop index from inner and syn schema
         * @param if_exists
         * @param table_name
         * @param refresh
         * @return
         */
        int drop_one_index(const bool if_exists, const ObString &table_name, bool &refresh);

        //add maoxx
        /**
         * @brief check_column_checksum
         * check column checksum
         * @param index_table_id
         * @param column_checksum_flag
         * @return OB_SUCCESS or other ERROR
         */
        int check_column_checksum(const int64_t index_table_id, bool &column_checksum_flag);

        /**
         * @brief clean_column_checksum
         * clean column checksum
         * @param current_version
         * @return OB_SUCCESS or other ERROR
         */
        int clean_column_checksum(int64_t current_version);

        /**
         * @brief get_column_checksum
         * get column checksum
         * @param range
         * @param required_version
         * @param column_checksum
         * @return OB_SUCCESS or other ERROR
         */
        int get_column_checksum(const ObNewRange range, const int64_t required_version, ObString& column_checksum);
        //add e

        //add longfei [cons static index] 151216:b
        /**
         * @brief get_local_schema
         * @return schema_manager_for_cache_
         */
        inline ObSchemaManagerV2* get_local_schema() const
        {
          return schema_manager_for_cache_;;
        }
        //add e

        //for bypass process begin
        ObRootOperationHelper* get_bypass_operation();
        bool is_bypass_process();
        int set_bypass_flag(const bool flag);
        int get_new_table_id(uint64_t &max_table_id, ObBypassTaskInfo &table_name_id);
        int clean_root_table();
        int slave_clean_root_table();
        void set_bypass_version(const int64_t version);
        int unlock_frozen_version();
        int lock_frozen_version();
        int prepare_bypass_process(common::ObBypassTaskInfo &table_name_id);
        virtual int start_bypass_process(common::ObBypassTaskInfo &table_name_id);
        int check_bypass_process();
        const char* get_bypass_state()const;
        int cs_delete_table_done(const common::ObServer &cs,
            const uint64_t table_id, const bool is_succ);
        int delete_table_for_bypass();
        int cs_load_sstable_done(const common::ObServer &cs,
            const common::ObTableImportInfoList &table_list, const bool is_load_succ);
        virtual int bypass_meta_data_finished(const OperationType type, ObRootTable2 *root_table,
            ObTabletInfoManager *tablet_manager, common::ObSchemaManagerV2 *schema_mgr);
        int use_new_root_table(ObRootTable2 *root_table, ObTabletInfoManager *tablet_manager);
        int switch_bypass_schema(common::ObSchemaManagerV2 *schema_mgr, common::ObArray<uint64_t> &delete_tables);
        int64_t get_frozen_version_for_cs_heartbeat() const;
        //for bypass process end
        /// check the table exist according the local schema manager
        int check_table_exist(const common::ObString & table_name, bool & exist);

        //add by wangdonghui 20160122 :b
        /**
         * @brief check_procedure_exist
         * check procedure exist or not
         * @param proc_name
         * @param exist
         * @return
         */
        int check_procedure_exist(const common::ObString & proc_name, bool & exist);
        //add :e
        int delete_dropped_tables(int64_t & table_count);
        void after_switch_to_master();
        int after_restart();
        int make_checkpointing();
        ObRootMsProvider & get_ms_provider() { return ms_provider_; }

        //table_id is OB_INVALID_ID , get all table's row_checksum
        int get_row_checksum(const int64_t tablet_version, const uint64_t table_id, ObRowChecksum &row_checksum);

        friend class ObRootServerTester;
        friend class ObRootLogWorker;
        friend class ObDailyMergeChecker;
        friend class ObHeartbeatChecker;
        friend class ::ObBalanceTest;
        friend class ::ObBalanceTest_test_n_to_2_Test;
        friend class ::ObBalanceTest_test_timeout_Test;
        friend class ::ObBalanceTest_test_rereplication_Test;
        friend class ::ObBalanceTest_test_n_to_2_with_faulty_dest_cs_Test;
        friend class ::ObDeleteReplicasTest_delete_in_init_Test;
        friend class ::ObDeleteReplicasTest_delete_when_rereplication_Test;
        friend class ::ObDeleteReplicasTest_delete_when_report_Test;
        friend class ::ObBalanceTest_test_shutdown_servers_Test;
        friend class ::ObRootServerTest;
        friend class ObRootReloadConfig;
		//add wsx:seidx bug
		int write_tablet_info_list_to_rt_v2(ObTabletInfoList **tablet_info_list, int32_t server_index[], const int32_t list_size);
        //add e
      private:
        bool async_task_queue_empty()
        {
          return seq_task_queue_.size() == 0;
        }
        int after_boot_strap(ObBootstrap & bootstrap);
        /*
         * 收到汇报消息后调用
         */
        int got_reported(const common::ObTabletReportInfoList& tablets, const int server_index,
            const int64_t frozen_mem_version, const bool for_bypass = false, const bool is_replay_log = false);
        /*
         * 旁路导入时，创建新range的时候使用
         */
        int add_range_to_root_table(const ObTabletReportInfoList &tablets, const bool is_replay_log = false);
        /*
         * 处理汇报消息, 直接写到当前的root table中
         * 如果发现汇报消息中有对当前root table的tablet的分裂或者合并
         * 要调用采用写拷贝机制的处理函数
         */
        int got_reported_for_query_table(const common::ObTabletReportInfoList& tablets,
            const int32_t server_index, const int64_t frozen_mem_version, const bool for_bypass = false);
        /*
         * 写拷贝机制的,处理汇报消息
         */
        int got_reported_with_copy(const common::ObTabletReportInfoList& tablets,
            const int32_t server_index, const int64_t have_done_index, const bool for_bypass = false);

        int create_new_table(const bool did_replay, const common::ObTabletInfo& tablet,
            const common::ObArray<int32_t> &chunkservers, const int64_t mem_version);
        int slave_batch_create_new_table(const common::ObTabletInfoList& tablets,
            int32_t** t_server_index, int32_t* replicas_num, const int64_t mem_version);
        void get_available_servers_for_new_table(int* server_index, int32_t expected_num, int32_t &results_num);
        int get_deleted_tables(const common::ObSchemaManagerV2 &old_schema,
            const common::ObSchemaManagerV2 &new_schema, common::ObArray<uint64_t> &deleted_tables);
        int make_out_cell_all_server(ObCellInfo& out_cell, ObScanner& scanner,
            const int32_t max_row_count) const;


        /*
         * 生成查询的输出cell
         */
        int make_out_cell(common::ObCellInfo& out_cell, ObRootTable2::const_iterator start,
            ObRootTable2::const_iterator end, common::ObScanner& scanner, const int32_t max_row_count,
            const int32_t max_key_len) const;

        // stat related functions
        void do_stat_start_time(char *buf, const int64_t buf_len, int64_t& pos);
        void do_stat_local_time(char *buf, const int64_t buf_len, int64_t& pos);
        void do_stat_common(char *buf, const int64_t buf_len, int64_t& pos);
        void do_stat_schema_version(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_mem(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_table_num(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_tablet_num(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_cs(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_ms(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_ups(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_all_server(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_frozen_time(char* buf, const int64_t buf_len, int64_t &pos);
        int64_t get_stat_value(const int32_t index);
        void do_stat_cs_num(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_ms_num(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_merge(char* buf, const int64_t buf_len, int64_t &pos);
        void do_stat_unusual_tablets_num(char* buf, const int64_t buf_len, int64_t &pos);

        void switch_root_table(ObRootTable2 *rt, ObTabletInfoManager *ti);
        int switch_schema_manager(const common::ObSchemaManagerV2 & schema_manager);
        /*
         * 在一个tabelt的各份拷贝中, 寻找合适的备份替换掉
         */
        int write_new_info_to_root_table(
            const common::ObTabletInfo& tablet_info, const int64_t tablet_version, const int32_t server_index,
            ObRootTable2::const_iterator& first, ObRootTable2::const_iterator& last, ObRootTable2 *p_root_table);
        bool check_all_tablet_safe_merged(void) const;
        int create_root_table_for_build();
        DISALLOW_COPY_AND_ASSIGN(ObRootServer2);
        int get_rowkey_info(const uint64_t table_id, common::ObRowkeyInfo &info) const;
        int select_cs(const int64_t select_num, common::ObArray<std::pair<common::ObServer, int32_t> > &chunkservers);
      private:
        int try_create_new_tables(int64_t fetch_version);
        int try_create_new_table(int64_t frozen_version, const uint64_t table_id);
        int check_tablets_legality(const common::ObTabletInfoList &tablets);
        int split_table_range(const int64_t frozen_version, const uint64_t table_id,
            common::ObTabletInfoList &tablets);
        int create_table_tablets(const uint64_t table_id, const common::ObTabletInfoList & list);
        int create_tablet_with_range(const int64_t frozen_version,
            const common::ObTabletInfoList& tablets);
        int create_empty_tablet_with_range(const int64_t frozen_version,
            ObRootTable2 *root_table, const common::ObTabletInfo &tablet,
            int32_t& created_count, int* t_server_index);

        /// for create and delete table xielun.szd
        int drop_one_table(const bool if_exists, const common::ObString & table_name, bool & refresh);
        /// force sync schema to all servers include ms\cs\master ups
        int force_sync_schema_all_servers(const common::ObSchemaManagerV2 &schema);

        //add by wangdonghui 20160123 :b

        int sync_proc_all_ms(void);
        /**
         * @brief force_sync_cahce_all_servers
         * sync cache to ms
         * @param proc_name procedure name
         * @param proc_source_code procedure source code
         * @return error code
         */
        int force_sync_cahce_all_servers(const common::ObString proc_name, const common::ObString proc_source_code);
        /**
         * @brief force_delete_cahce_all_servers
         * delete cache to ms
         * @param proc_name procedure name
         * @return error code
         */
        int force_delete_cahce_all_servers(const common::ObString proc_name);
        //add :e
        int force_heartbeat_all_servers(void);
        int get_ms(common::ObServer& ms_server);


      private:
        static const int MIN_BALANCE_TOLERANCE = 1;

        common::ObClientHelper client_helper_;
        ObRootServerConfig &config_;
        ObRootWorker* worker_; //who does the net job
        ObRootLogWorker* log_worker_;

        // cs & ms manager
        ObChunkServerManager server_manager_;
        mutable tbsys::CRWLock server_manager_rwlock_;

        mutable tbsys::CThreadMutex root_table_build_mutex_; //any time only one thread can modify root_table
        ObRootTable2* root_table_;
        ObTabletInfoManager* tablet_manager_;
        mutable tbsys::CRWLock root_table_rwlock_; //every query root table should rlock this
        common::ObTabletReportInfoList delete_list_;
        bool have_inited_;
        bool first_cs_had_registed_;
        volatile bool receive_stop_;

        mutable tbsys::CThreadMutex frozen_version_mutex_;
        int64_t last_frozen_mem_version_;
        int64_t last_frozen_time_;
        int64_t next_select_cs_index_;
        int64_t time_stamp_changing_;

        common::ObiRole obi_role_;        // my role as oceanbase instance
        common::ObServer my_addr_;

        time_t start_time_;
        // ups related
        ObUpsManager *ups_manager_;
        ObUpsHeartbeatRunnable *ups_heartbeat_thread_;
        ObUpsCheckRunnable *ups_check_thread_;
        // balance related
        ObRootBalancer *balancer_;
        ObRootBalancerRunnable *balancer_thread_;
        ObRestartServer *restart_server_;
        // schema service
        int64_t schema_timestamp_;
        int64_t privilege_timestamp_;
        common::ObSchemaService *schema_service_;
        common::ObScanHelperImpl *schema_service_scan_helper_;
        ObSchemaServiceMsProvider *schema_service_ms_provider_;
        ObSchemaServiceUpsProvider *schema_service_ups_provider_;
        // new root table service
        mutable tbsys::CThreadMutex rt_service_wmutex_;
        common::ObFirstTabletEntryMeta *first_meta_;
        common::ObRootTableService *rt_service_;
        // sequence async task queue
        ObRootAsyncTaskQueue seq_task_queue_;
        ObDailyMergeChecker merge_checker_;
        ObHeartbeatChecker heart_beat_checker_;
        // trigger tools
        ObRootMsProvider ms_provider_;
        common::ObTriggerEvent root_trigger_;
        // ddl operator
        tbsys::CThreadMutex mutex_lock_;
        ObRootDDLOperator ddl_tool_;
        ObBootState boot_state_;
        //to load local schema.ini file, only use the first_time
        common::ObSchemaManagerV2* local_schema_manager_;
        //used for cache
        common::ObSchemaManagerV2 * schema_manager_for_cache_;
        mutable tbsys::CRWLock schema_manager_rwlock_;

        ObRootServerState state_;  //RS state
        int64_t bypass_process_frozen_men_version_; //旁路导入状态下，可以广播的frozen_version
        ObRootOperationHelper operation_helper_;
        ObRootOperationDuty operation_duty_;
        common::ObTimer timer_;

        // add by zcd [multi_cluster] 20150416:b
        std::vector<common::ObServer> slave_array_;
        // add:e

        // add by guojinwei [lease between rs and ups][multi_cluster] 20150908:b
        common::ObElectionRoleMgr election_role_;    ///< the information of rs election
        common::ObClusterMgr cluster_mgr_;           ///< the information of clusters
        // add:e
		
        //add by wangdonghui [procedure physical plan cache management] 20160229:b
        common::ObNameCodeMap *name_code_map_;  ///< rootserver store name code map
        //add :e		
    };
  }
}

#endif
