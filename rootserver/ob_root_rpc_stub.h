/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_rpc_stub.h
 * @brief for rpc among servers
 *
 * modified by Wenghaixing:add some function with rpc for secondary index construction
 * modified by guojinwei:add some remote process control function to the ObRootRpcStub class.
 *                       ObRootRpcStub support multiple clusters for HA by adding or modifying
 *                       some functions, member variables
 * modified by wangdonghui:add update and delete procedure cache functions
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         chujiajia <52151500014@ecnu.cn>
 *         zhangcd <zhangcd_ecnu@ecnu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date  2016_07_30
 */

#ifndef OCEANBASE_ROOT_RPC_STUB_H_
#define OCEANBASE_ROOT_RPC_STUB_H_

#include "common/ob_fetch_runnable.h"
#include "common/ob_common_rpc_stub.h"
#include "common/ob_server.h"
#include "common/ob_schema.h"
#include "common/ob_range.h"
#include "common/ob_tablet_info.h"
#include "common/ob_tablet_info.h"
#include "common/ob_rs_ups_message.h"
#include "common/ob_rs_rs_message.h"
#include "common/ob_data_source_desc.h"
#include "common/ob_list.h"
#include "common/ob_range2.h"
#include "ob_chunk_server_manager.h"
#include "common/ob_new_scanner.h"
#include "sql/ob_sql_scan_param.h"
#include "ob_daily_merge_checker.h"
#include "common/ob_schema_service.h"
#include "common/ob_name_code_map.h"

namespace oceanbase
{
  namespace rootserver
  {
    class ObDataSourceProxyList;

    class ObRootRpcStub : public common::ObCommonRpcStub
    {
      public:
        ObRootRpcStub();
        virtual ~ObRootRpcStub();
        int init(const common::ObClientManager *client_mgr, common::ThreadSpecificBuffer* tsbuffer);
        // synchronous rpc messages
        virtual int slave_register(const common::ObServer& master, const common::ObServer& slave_addr, common::ObFetchParam& fetch_param, const int64_t timeout);
        // add by zcd [multi_cluster] 20150405:b
        virtual int set_slave_obi_role(const ObServer& slave, const common::ObiRole &role, const int64_t timeout);
        virtual int boot_strap(const common::ObServer& server);
        // add:e
        virtual int set_obi_role(const common::ObServer& ups, const common::ObiRole& role, const int64_t timeout_us);
        virtual int switch_schema(const common::ObServer& server, const common::ObSchemaManagerV2& schema_manager, const int64_t timeout_us);

        //add by wangdonghui 20160122 :b
        virtual int update_cache(const common::ObServer& server, const common::ObString & proc_name, const common::ObString & proc_source_code, const int64_t local_version, const int64_t timeout_us);
        virtual int delete_cache(const common::ObServer& server, const common::ObString & proc_name, const int64_t local_version, const int64_t timeout_us);
        virtual int update_whole_cache(const common::ObServer& server, common::ObNameCodeMap* name_code_map, const int64_t timeout_us);

        //add :e
        //add hxlong [truncate table] 20170403:b
        virtual int truncate_table(const common::ObServer& server, const common::ObMutator& mutator, const int64_t timeout_us);
        //add:e
        virtual int migrate_tablet(const common::ObServer& src_cs, const common::ObDataSourceDesc& desc, const int64_t timeout_us);
        virtual int create_tablet(const common::ObServer& cs, const common::ObNewRange& range, const int64_t mem_version, const int64_t timeout_us);
        virtual int delete_tablets(const common::ObServer& cs, const common::ObTabletReportInfoList &tablets, const int64_t timeout_us);
        virtual int get_last_frozen_version(const common::ObServer& ups, const int64_t timeout_us, int64_t &frozen_version);
        virtual int get_obi_role(const common::ObServer& master, const int64_t timeout_us, common::ObiRole &obi_role);
        virtual int get_boot_state(const common::ObServer& master, const int64_t timeout_us, bool &boot_ok);
        virtual int revoke_ups_lease(const common::ObServer &ups, const int64_t lease, const common::ObServer& master, const int64_t timeout_us);
        virtual int import_tablets(const common::ObServer& cs, const uint64_t table_id, const int64_t version, const int64_t timeout_us);
        virtual int get_ups_max_log_seq(const common::ObServer& ups, uint64_t &max_log_seq, const int64_t timeout_us);
        // add by guojinwei [log timestamp][multi_cluster] 20150820:b
        /**
         * @brief get max log timestamp from ups
         * @param[in] ups  the ups server
         * @param[out] max_log_timestamp  max log timestamp from ups
         * @param[in] timeout_us  timeout of the rpc
         * @return OB_SUCCESS if success
         */
        virtual int get_ups_max_log_timestamp(const common::ObServer& ups, int64_t &max_log_timestamp, const int64_t timeout_us);
        // add:e
        virtual int shutdown_cs(const common::ObServer& cs, bool is_restart, const int64_t timeout_us);
        virtual int get_row_checksum(const common::ObServer& server, const int64_t data_version, const uint64_t table_id, ObRowChecksum &row_checksum, int64_t timeout_us);
        // add by zcd [multi_cluster] 20150405:b
        /**
         * @brief set the config info of the specific server
         * @param[in] server address
         * @param[in] configure strings to be set
         * @param[in] timeout_us the max timeout
         * @return OB_SUCCESS if success
         */
        virtual int set_config(const common::ObServer& server, const ObString& config_str, int64_t timeout_us);
        // add:e

        virtual int get_split_range(const common::ObServer& ups, const int64_t timeout_us,
             const uint64_t table_id, const int64_t frozen_version, common::ObTabletInfoList &tablets);
        virtual int table_exist_in_cs(const common::ObServer &cs, const int64_t timeout_us,
            const uint64_t table_id, bool &is_exist_in_cs);
        // asynchronous rpc messages
        virtual int heartbeat_to_cs(const common::ObServer& cs,
                                    const int64_t lease_time,
                                    const int64_t frozen_mem_version,
                                    const int64_t schema_version,
                                    const int64_t config_version);

        //add wenghaixing [secondary index.static_index]20151130
        virtual int heartbeat_to_cs_with_index(const ObServer &cs,
                                               const int64_t lease_time,
                                               const int64_t frozen_mem_version,
                                               const int64_t schema_version,
                                               const int64_t config_version,
                                               const IndexBeat &beat);
        //add e

        virtual int heartbeat_to_ms(const common::ObServer& ms,
                                    const int64_t lease_time,
                                    const int64_t frozen_mem_version,
                                    const int64_t schema_version,
                                    const common::ObiRole &role,
                                    const int64_t privilege_version,
                                    const int64_t config_version,
                                    const int64_t procedure_version,
                                    const bool all_ups_state);

        virtual int grant_lease_to_ups(const common::ObServer& ups,
                                       common::ObMsgUpsHeartbeat &msg);

        virtual int request_report_tablet(const common::ObServer& chunkserver);
        virtual int execute_sql(const common::ObServer& ms,
                                const common::ObString sql, int64_t timeout);
        virtual int request_cs_load_bypass_tablet(const common::ObServer& chunkserver,
            const common::ObTableImportInfoList &import_info, const int64_t timeout_us);
        virtual int request_cs_delete_table(const common::ObServer& chunkserver, const uint64_t table_id, const int64_t timeout_us);
        virtual int fetch_ms_list(const common::ObServer& rs, common::ObArray<common::ObServer> &ms_list, const int64_t timeout_us);

        // fetch range table from datasource, uri is used when datasouce need uri info to generate range table(e.g. datasource proxy server)
        virtual int fetch_range_table(const common::ObServer& data_source, const common::ObString& table_name,
            common::ObList<common::ObNewRange*>& range_table , common::ModuleArena& allocator, int64_t timeout);
        virtual int fetch_range_table(const common::ObServer& data_source, const common::ObString& table_name, const common::ObString& uri,
            common::ObList<common::ObNewRange*>& range_table ,common::ModuleArena& allocator, int64_t timeout);
        virtual int fetch_proxy_list(const common::ObServer& ms, const common::ObString& table_name,
            const int64_t cluster_id, ObDataSourceProxyList& proxy_list, int64_t timeout);
        virtual int fetch_slave_cluster_list(const common::ObServer& ms, const common::ObServer& master_rs,
            common::ObServer* slave_cluster_rs, int64_t& rs_count, int64_t timeout);
        //add wenghaixing [secondary index.static_index]20151118
        virtual int get_init_index_from_ups(const ObServer ups, const int64_t timeout, const int64_t data_version, ObArray<uint64_t> *list);
        //add e
        virtual int import(const common::ObServer& rs, const common::ObString& table_name,
            const uint64_t table_id, const common::ObString& uri, const int64_t start_time, const int64_t timeout);
        virtual int kill_import(const common::ObServer& rs, const common::ObString& table_name,
            const uint64_t table_id, const int64_t timeout);
        virtual int get_import_status(const common::ObServer& rs, const common::ObString& table_name,
            const uint64_t table_id, int32_t& status, const int64_t timeout);
        virtual int set_import_status(const common::ObServer& rs, const common::ObString& table_name,
            const uint64_t table_id, const int32_t status, const int64_t timeout);
        virtual int notify_switch_schema(const common::ObServer& rs, const int64_t timeout);
        //add chujiajia [rs_election][multi_cluster] 20150823:b
        /**
         * @brief send rootserver election message 
         * @param[in] root_server  the rs server of the cluster
         * @param[in] msg_rselection  election message
         * @param[out] info[]  response info
         * @param[in] timeout_rs  rpc timeout
         * @return OB_SUCCESS if success
         */
        virtual int rs_election(const common::ObServer &root_server, const ObMsgRsElection &msg_rselection, char info[], const int64_t timeout_rs);
        // add:e
        // add by guojinwei [reelect][multi_cluster] 20151129:b
        /**
         * @brief whether the cluster is ready for election
         * @param[in] rs  the rs server of the cluster
         * @param[in] timeout_us  timeout of the rpc
         * @return OB_SUCCESS if the cluster is ready for election
         */
        virtual int get_cluster_election_ready(const common::ObServer& rs, const int64_t timeout_us);

        /**
         * @brief whether the ups is ready for election
         * @param[in] ups  the ups server of the cluster
         * @param[in] timeout_us  timeout of the rpc
         * @return OB_SUCCESS if the ups is ready for election
         */
        virtual int get_ups_election_ready(const common::ObServer& ups, const int64_t timeout_us);
        // add:e
      private:
        int fill_proxy_list(ObDataSourceProxyList& proxy_list, common::ObNewScanner& scanner);
        int fill_slave_cluster_list(common::ObNewScanner& scanner, const common::ObServer& master_rs,
            common::ObServer* slave_cluster_rs, int64_t& rs_count);
        int get_thread_buffer_(common::ObDataBuffer& data_buffer);
      private:
        static const int32_t DEFAULT_VERSION = 1;
        common::ThreadSpecificBuffer *thread_buffer_;
    };
  } /* rootserver */
} /* oceanbase */

#endif /* end of include guard: OCEANBASE_ROOT_RPC_STUB_H_ */
