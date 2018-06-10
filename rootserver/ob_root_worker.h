/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_root_worker.h
 * @brief root server worker
 *
 * modified by longfei：add rt_create_index() and rt_drop_index()
 * modified by wenghaixing: add icu into root worker
 * modified by maoxiaoxiao:add functions to get column checksum in root server
 *
 * modified by guojinwei,chujiajia,pangtianze,zhangcd:
 * support multiple clusters for HA by adding or modifying
 *   some functions, member variables
 *
 *   1.add the auto_elect_flag to election process.
 *   2.modify the election state.
 *   3.add the majority_count setting in rootserver.
 * modified by wangdonghui:add some process statements and functions for procedure
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         chujiajia  <52151500014@ecnu.cn>
 *         pangtianze <pangtianze@ecnu.com>
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
 *
 *
 ================================================================*/
#ifndef OCEANBASE_ROOTSERVER_ROOT_WORKER_H_
#define OCEANBASE_ROOTSERVER_ROOT_WORKER_H_
#include "common/ob_define.h"
#include "common/ob_base_server.h"
#include "common/thread_buffer.h"
#include "common/ob_client_manager.h"
#include "common/ob_fetch_runnable.h"
#include "common/ob_role_mgr.h"
#include "common/ob_slave_mgr.h"
#include "common/ob_check_runnable.h"
#include "common/ob_packet_queue_thread.h"
#include "common/ob_packet.h"
#include "common/ob_packet_factory.h"
#include "common/ob_timer.h"
#include "common/ob_ms_list.h"
#include "common/ob_config_manager.h"
#include "rootserver/ob_root_server2.h"
#include "rootserver/ob_root_rpc_stub.h"
#include "rootserver/ob_root_sql_proxy.h"
#include "rootserver/ob_root_log_replay.h"
#include "rootserver/ob_root_log_manager.h"
#include "rootserver/ob_root_stat.h"
#include "rootserver/ob_root_fetch_thread.h"
#include "rootserver/ob_root_server_config.h"
#include "rootserver/ob_root_inner_table_task.h"
#include "rootserver/ob_check_rselection.h"
//add wenghaixing [secondary index.static index]20151117
#include "rootserver/ob_index_control_unit.h"
//add e
namespace oceanbase
{
  namespace common
  {
    class ObDataBuffer;
    class ObServer;
    class ObScanner;
    class ObTabletReportInfoList;
    class ObGetParam;
    class ObScanParam;
    class ObRange;
    class ObTimer;
    class MsList;
    class ObGeneralRpcStub;
  }
  using common::ObConfigManager;
  namespace rootserver
  {
    class ObRootWorker :public common::ObBaseServer, public common::ObPacketQueueHandler
    {
      public:
        ObRootWorker(ObConfigManager &config_mgr, ObRootServerConfig &rs_config);
        virtual ~ObRootWorker();

        /**
         * handle packet received from network
         * push packet into queue
         * @param packet   packet to handle
         * @return int     return OB_SUCCESS if packet pushed, else return OB_ERROR
         */
        int handlePacket(common::ObPacket* packet);
        //tbnet::IPacketHandler::HPRetCode handlePacket(onev_request_e *connection, tbnet::Packet *packet);
        int handleBatchPacket(common::ObPacketQueue &packetQueue);
        bool handlePacketQueue(common::ObPacket *packet, void *args);

        int initialize();
        int start_service();
        void wait_for_queue();
        void destroy();

        int create_eio();

        bool start_merge();

        int submit_check_task_process();
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        /**
         * @brief set_auto_elect_flag
         * @param flag the auto_elect_flag value
         * @return OB_SUCCESS if success, not OB_SUCCESS if failed
         */
        int set_auto_elect_flag(bool flag = true);
        int schedule_set_auto_elect_flag_task();
        // add:e
        int submit_delete_tablets_task(const common::ObTabletReportInfoList& delete_list);
        int schedule_after_restart_task(const int64_t delay,bool repeate = false);
        int submit_restart_task();
        //add wenghaixing [secondary index.static_index]20151118
        int submit_job(ObPacket pc);
        //add e
        int set_io_thread_count(int io_thread_num);
        ObRootLogManager* get_log_manager();
        common::ObRoleMgr* get_role_manager();
        common::ThreadSpecificBuffer::Buffer* get_rpc_buffer() const;
        virtual ObRootRpcStub& get_rpc_stub();
        virtual ObGeneralRpcStub& get_general_rpc_stub();

        ObRootServer2& get_root_server();
        ObConfigManager& get_config_mgr();
        ObRootServerConfig& get_config() const;

        int send_obi_role(common::ObiRole obi_role);
        common::ObClientManager* get_client_manager();
        int64_t get_network_timeout();
        common::ObServer get_rs_master();
        common::ThreadSpecificBuffer* get_thread_buffer();
        //add wenghaixing [secondary index.static_index]20151130
        ObIndexControlUnit& get_icu();
        //add e
// add by guojinwei [lease between rs and ups][multi_cluster] 20150820:b
        /**
         * @brief get rs election lease
         * @return the rs election lease
         */
        int64_t get_rs_election_lease();
        // add by guojinwei 20150523:e
        // add by zcd [multi_cluster] 20150405:b
        /**
         * @brief start_master_cluster
         * start as the master cluster rootserver
         * @return OB_SUCCESS if success
         */
        int start_master_cluster();
        /**
         * @brief start_slave_cluster
         * start as the slave cluster rootserver
         * @return OB_SUCCESS if success
         */
        int start_slave_cluster();
        /**
         * @brief set_master_root_server_config_on_slave
         * set the configure information
         * in the master cluster rootserver
         * @param config_string the configure strings
         * @return OB_SUCCESS if success
         */
        int set_master_root_server_config_on_slave(const ObString& config_string);
        /**
         * @brief set_all_slaves_role_to_slave
         * set the obi role of all the slave cluster rootserver to OBI_SLAVE
         * @return OB_SUCCESS if success
         */
        int set_all_slaves_role_to_slave();
        /**
         * @brief get_obi_master_root_server get the current master cluster rootserver address
         * @return the current master cluster rootserver
         */
        ObServer get_obi_master_root_server();
        /**
         * @brief set_obi_master_role
         * set the obi_role to OBI_MASTER of current rootserver
         * @return OB_SUCCESS if success
         */
        int set_obi_master_role();
        /**
         * @brief set_obi_master_first
         * set the current rootserver as Leader.
         * @return OB_SUCCESS if success
         */
        int set_obi_master_first();
        // add:e
        // add by guojinwei [new rs_admin command][multi_cluster] 20150901:b
        // ensure that this cluster is master before using this function
        /**
         * @brief reelect master cluster
         * This function is only called by master cluster.
         * @return OB_SUCCESS if success
         */
        int reelect_master_rootserver();
        // add:e      
      private:
        int start_as_master();
        int start_as_slave();
        template <class Queue>
          int submit_async_task_(const common::PacketCode pcode, Queue &qthread, int32_t task_queue_size,
              const common::ObDataBuffer *data_buffer = NULL,
              const common::ObPacket *packet = NULL);
        template <class Queue>
          int submit_async_task_(const common::PacketCode pcode, Queue &qthread, int32_t task_queue_size,
              const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req,
              const uint32_t channel_id, const int64_t timeout);

        // notice that in_buff can not be const.
        //add chujiajia [rs_election][multi_cluster] 20150823:b
        int rt_rs_election(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add:e
        int rt_get_update_server_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff,
            bool use_inner_port = false);
        int rt_get_merge_delay_interval(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_scan(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_sql_scan(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_after_restart(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_write_schema_to_file(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_change_table_id(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        /**
         * @brief rt_set_auto_elect_flag set the auto_elect_flag, called by handlePacketQueue()
         * @param version
         * @param in_buff input data buffer
         * @param req request used by rpc
         * @param channel_id
         * @param out_buff outcome data buffer
         * @return OB_SUCCESS if success.
         */
        int rt_set_auto_elect_flag(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add:e
        int rt_fetch_schema(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_fetch_schema_version(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_report_tablets(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_waiting_job_done(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_register(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_register_ms(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_migrate_over(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_report_capacity_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_heartbeat(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_heartbeat_ms(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);

        int rt_dump_cs_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_fetch_stats(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_check_tablet_merged(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add wenghaixing [secondary index.static_index]20151118
        int rt_handle_index_job(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_report_index_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        inline ObIndexControlUnit* get_index_control_unit(){return &icu_;}
        //add e
        int rt_split_tablet(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rs_check_root_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_ping(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_slave_quit(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_update_server_report_freeze(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id,            common::ObDataBuffer& out_buff);

        int rt_slave_register(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_renew_lease(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_grant_lease(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_obi_role(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_boot_state(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_set_obi_role_to_slave(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_set_obi_role(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add by guojinwei [obi role switch][multi_cluster] 20150914:b
        int rt_set_slave_cluster_obi_role(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add:e
        int rt_get_last_frozen_version(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_force_cs_to_report(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_admin(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_change_log_level(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_stat(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rs_dump_cs_tablet_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_ups_heartbeat_resp(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_ups_register(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_ups_slave_failure(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_ups(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_set_ups_config(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_change_ups_master(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_cs_list(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_ms_list(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_proxy_list(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_cs_import_tablets(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_shutdown_cs(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_restart_cs(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_cs_delete_tablets(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_delete_tablets(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_create_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_force_create_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_force_drop_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_alter_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_drop_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add hxlong [Truncate Table]:20170403:b
        int rt_truncate_table(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add:e
        //add by wangdonghui 20160121 :b
        /**
         * @brief rt_create_procedure
         * create procedure :deserialize procedure name and source code
         * @param version  rpc version
         * @param in_buff  receive packet buffer
         * @param req  packet request
         * @param channel_id   tbnet need this packet channel_id
         * @param out_buff databuffer
         * @return error code
         */
        int rt_create_procedure(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add :e

        //add by wangdonghui 20160225 [drop procedure] :b
        /**
         * @brief rt_drop_procedure
         * drop procedure :deserialize procedure name
         * @param version  rpc version
         * @param in_buff receive packet buffer
         * @param req  packet request
         * @param channel_id  tbnet need this packet channel_id
         * @param out_buff databuffer
         * @return error code
         */
        int rt_drop_procedure(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add :e

        //add by wangdonghui 20160304 :b
        /**
         * @brief rt_fetch_procedure
         * fetch name_code_map
         * @param version  rpc version
         * @param in_buff receive packet buffer
         * @param req  packet request
         * @param channel_id  tbnet need this packet channel_id
         * @param out_buff  databuffer
         * @return error code
         */
        int rt_fetch_procedure(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add :e

        int rt_execute_sql(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_handle_trigger_event(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        
        //add longfei [create index]
        /**
         * @brief rt_create_index: rs recieve package OB_CREATE_INDEX
         * @param version
         * @param in_buff
         * @param req
         * @param channel_id
         * @param out_buff
         * @return error code
         */
        int rt_create_index(
            const int32_t version,
            common::ObDataBuffer& in_buff,
            onev_request_e* req,
            const uint32_t channel_id,
            common::ObDataBuffer& out_buff);
        /**
         * @brief rt_drop_index: rs recieve package OB_DROP_INDEX
         * @param version
         * @param in_buff
         * @param req
         * @param channel_id
         * @param out_buff
         * @return
         */
        int rt_drop_index(
            const int32_t version,
            common::ObDataBuffer& in_buff,
            onev_request_e* req,
            const uint32_t channel_id,
            common::ObDataBuffer& out_buff);

        //add maoxx
	    /**
         * @brief rt_get_column_checksum
         * get column checksum
         * @param version
         * @param in_buff
         * @param req
         * @param channel_id
         * @param out_buff
         * @return OB_SUCCESS or other ERROR
         */
        int rt_get_column_checksum(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //add e
        int rt_set_config(const int32_t version,
            common::ObDataBuffer& in_buff, onev_request_e* req,
            const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_config(const int32_t version,
            common::ObDataBuffer& in_buff, onev_request_e* req,
            const uint32_t channel_id, common::ObDataBuffer& out_buff);
        //bypass
        int rt_check_task_process(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* conn, const uint32_t channel_id, common::ObDataBuffer& out_buff);

        int rt_prepare_bypass_process(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* conn, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_start_bypass_process(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* conn, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_cs_delete_table_done(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* conn, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rs_cs_load_bypass_sstable_done(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* conn, const uint32_t channel_id, common::ObDataBuffer& out_buff);

        int rt_get_row_checksum(const int32_t version, common::ObDataBuffer& in_buff,
            onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);

        int rt_start_import(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_import(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_start_kill_import(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_kill_import(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_import_status(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_set_import_status(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_notify_switch_schema(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add by zhangcd [majority_count_init] 20151118:b
        /**
         * @brief rt_get_all_clusters_info get all the clusters info from master rootserver
         * @param version
         * @param in_buff
         * @param req
         * @param channel_id
         * @param out_buff
         * @return OB_SUCCESS if success.
         */
        int rt_get_all_clusters_info(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add:e
        // add by guojinwei [reelect][multi_cluster] 20151129:b
        int rt_get_election_ready(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        // add:e
      private:
        int do_stat(int stat_key, char *buf, const int64_t buf_len, int64_t& pos);
        int get_obi_role_from_master();
        int get_boot_state_from_master();
        int do_admin_with_return(int admin_cmd);
        int do_admin_without_return(int admin_cmd);
        int slave_register_(common::ObFetchParam& fetch_param);
        int rt_slave_write_log(const int32_t version, common::ObDataBuffer& in_buffer, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buffer);
        int rt_get_master_ups_config(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_set_master_ups_config(const int32_t version, common::ObDataBuffer& in_buff, onev_request_e* req, const uint32_t channel_id, common::ObDataBuffer& out_buff);
        int rt_get_master_obi_rs(const int32_t version, common::ObDataBuffer &in_buff, onev_request_e *req, const uint32_t channel_id, common::ObDataBuffer &out_buff);
      protected:
        const static int64_t ASYNC_TASK_TIME_INTERVAL = 5000 * 1000;
        ObConfigManager &config_mgr_;
        ObRootServerConfig &config_;
        bool is_registered_;
        ObRootServer2 root_server_;
        common::ObPacketQueueThread read_thread_queue_;
        common::ObPacketQueueThread write_thread_queue_;
        common::ObPacketQueueThread log_thread_queue_;
        common::ThreadSpecificBuffer my_thread_buffer;
        common::ObClientManager client_manager;
        common::ObServer rt_master_;
        common::ObServer self_addr_;
        common::ObRoleMgr role_mgr_;
        common::ObSlaveMgr slave_mgr_;
        common::ObCheckRunnable check_thread_;
        //add chujiajia [rs_election][multi_cluster] 20150823:b
        rootserver::ObCheckRsElection check_rselection_thread_; ///< check election thread, excute election operation
        // add:e
        ObRootFetchThread fetch_thread_;
        ObRootSQLProxy sql_proxy_;
        ObRootRpcStub rt_rpc_stub_;
        common::ObGeneralRpcStub general_rpc_stub_;
        ObRootLogReplay log_replay_thread_;
        ObRootLogManager log_manager_;
        ObRootStatManager stat_manager_;
        int64_t schema_version_;
        MsList ms_list_task_;
        ObRootInnerTableTask inner_table_task_;
        ObRsAfterRestartTask after_restart_task_;
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        /**
         * @brief set_auto_elect_flag_task_ the timer task used for modify the auto_elect_flag
         */
        ObRootSetAutoElectFlagTask set_auto_elect_flag_task_;
        // add:e
        ObTimer timer_;
        //add wenghaixing[secondary index.static index]20151117
        ObIndexControlUnit icu_;
        //add e
    };

    inline ObRootServer2& ObRootWorker::get_root_server()
    {
      return root_server_;
    }
    inline ObConfigManager& ObRootWorker::get_config_mgr()
    {
      return config_mgr_;
    }

    inline ObRootServerConfig& ObRootWorker::get_config() const
    {
      return config_;
    }

    inline ObRootRpcStub& ObRootWorker::get_rpc_stub()
    {
      return rt_rpc_stub_;
    }

    inline ObGeneralRpcStub& ObRootWorker::get_general_rpc_stub()
    {
      return general_rpc_stub_;
    }
    //add wenghaixing [secondary index.static_index]20151130
    inline ObIndexControlUnit& ObRootWorker::get_icu()
    {
      return icu_;
    }

  }
}

#endif
