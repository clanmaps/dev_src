/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file base_main.h
 * @brief base main for all servers
 *   support multiple clusters for HA by adding or modifying
 *   some functions, member variables
 *   modify the command line parameters of rootserver,
 *   add the -s/--all_root_servers to command line.
 *
 * @version CEDAR 0.2 
 * @author chujiajia <52151500014@ecnu.cn>
 *         zhangcd <zhangcd_ecnu@ecnu.cn>
 * @date 2015_12_30
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
#ifndef OCEANBASE_ROOTSERVER_BASE_MAIN_H_
#define OCEANBASE_ROOTSERVER_BASE_MAIN_H_
#include <tbsys.h>

#include "ob_define.h"
namespace oceanbase
{
  namespace common
  {
    extern bool PACKET_RECORDER_FLAG;
    class BaseMain
    {
      public:
        //static BaseMain* instance() {
        //  return instance_;
        //}
        virtual ~BaseMain();
        virtual int start(const int argc, char *argv[]);
        virtual void destroy();

        /* restart server if and only if the *restart server flag* is
         * set. @func destroy should been executed before. */
        static  bool restart_server(int argc, char *argv[]);
        /* set *restart_server* flag, after that invoke restart_server
         * funtion to restart. otherwise restart_server function will
         * do nothing. */
        static  void set_restart_flag(bool restart = true);
      protected:
        BaseMain(const bool daemon = true);
        virtual void do_signal(const int sig);
        void add_signal_catched(const int sig);
        static BaseMain* instance_;
        virtual void print_usage(const char *prog_name);
        virtual void print_version();
        virtual void parse_cmd_line(const int argc, char *const argv[]);
        virtual int do_work() = 0;
        char config_[OB_MAX_FILE_NAME_LENGTH];
        char proxy_config_file_[OB_MAX_FILE_NAME_LENGTH];
        char cmd_data_dir_[OB_MAX_FILE_NAME_LENGTH];
        char cmd_prefix_dir_[OB_MAX_FILE_NAME_LENGTH];
        char cmd_rs_ip_[OB_IP_STR_BUFF];
        char cmd_master_rs_ip_[OB_IP_STR_BUFF];
        char cmd_datadir_[OB_MAX_FILE_NAME_LENGTH];
        char cmd_appname_[OB_MAX_APP_NAME_LENGTH];
        char cmd_devname_[OB_MAX_APP_NAME_LENGTH];
        char cmd_extra_config_[OB_MAX_EXTRA_CONFIG_LENGTH];

        // add by zcd [multi_cluster] 20150416:b
        // 保存当前-s选项的内容
        char cmd_rs_cluster_ips_[1024];
        // add:e

        int32_t cmd_cluster_id_;
        int32_t cmd_rs_port_;
        int32_t cmd_master_rs_port_;
        int32_t cmd_port_;
        int32_t cmd_inner_port_;
        int32_t cmd_obmysql_port_;
        //add chujiajia [rs_election][multi_cluster] 20150929:b
        int64_t cmd_rs_election_random_wait_time_;
        // add:e
        //add hushuang [scalable commit]20160630
        int64_t cmd_commit_group_size_;
        //add e
        char cmd_ms_type_[OB_MAX_MS_TYPE_LENGTH];
        const char *pid_dir_;
        const char *log_dir_;
        const char *server_name_;
      private:
        bool use_daemon_;
        static void sign_handler(const int sig);
        static bool restart_;
    };

  }
}
#endif
