/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_election_node.h
 * @brief manage election state and info
 *        Add a auto_elect_flag into election process.
 *        Add a set and get function of the member auto_elect_flag
 *
 * @version CEDAR 0.2 
 * @author
 *   Chu Jiajia  <52151500014@ecnu.cn>
 *   Pang Tianze <pangtianze@ecnu.com>
 *   zhangcd <zhangcd_ecnu@ecnu.cn>
 * @date 2015_08_23
 */

#ifndef OB_ELECTION_NODE_H_
#define OB_ELECTION_NODE_H_

#include "common/ob_array.h"
#include "common/ob_server.h"
#include "common/ob_client_manager.h"
#include "common/ob_election_role_mgr.h"
#include "rootserver/ob_root_server2.h"
#include "rootserver/ob_root_rpc_stub.h"

#define MAX_SLAVE_RS_SIZE 1024
#define ELECTION_ARRAY_SIZE 20

const int64_t election_message_time_out_us = 1000000;
const int64_t election_lease_length_us = 5000000;
const int64_t election_do_extend_time_us = 2000000;
//const int64_t election_protection_time_us = 200000;
const int64_t election_set_leader_lease_length_us = 20000000;

namespace oceanbase
{
  namespace rootserver
  {
    enum ObElectionRole
    {
      OB_FOLLOWER = 0,
      OB_CANDIDATE,
      OB_LEADER,
    };
    enum ObMessageType
    {
      OB_VOTEREQUEST = 0,
      OB_BROADCAST,
      OB_EXTENDLEASE,
      OB_EXPIRE,
      OB_ELECTIONTIMEOUT,
      OB_START_ELECTION,
    };
    /**
     * @brief The ObElectionNode class
     * ObElectionNode is designed for managing rootserver state and info about election,
     * by the way, other rootservers addr info is stored in the class.
     */
    class ObElectionNode
    {
      public:
        /**
         * @brief constructor
         */
        ObElectionNode()
        {
          // init();
        }
        /**
         * @brief destructor
         */
        ~ObElectionNode()
        {

        }
        /**
         * @brief get rootserver of local cluster
         * @return rootserver
         */
        common::ObServer& get_my_rs();
        /**
         * @brief get updateserver of local cluster
         * @return updateserver
         */
        common::ObServer& get_my_ups();
        /**
         * @brief get votefor rootserver in last election
         * @return the voterfor rootserver
         */
        common::ObServer& get_votefor();
        /**
         * @brief get leader rootserver
         * @return leader rootserver
         */
        common::ObServer& get_leaderinfo();
        /**
         * @brief get leader exist flag
         * @return leader exist flag, true of false
         */
        bool get_is_exist_leader();
        //add chujiajia [log synchronization][multi_cluster] 20160603:b
        /**
         * @brief get is_pre_leader flag, this flag marks if the node was the pre-leader
         * @return is_pre_leader flag, true of false
         */
        bool get_is_pre_leader();
        //add:e
        /**
         * @brief get timeout mark of election
         * @return timeout mark, true or false
         */
        bool get_timeoutmark();
        /**
         * @brief get extend lease mark of election
         * @return extend lease mark, true or false
         */
        bool get_extendlease_mark();
        /**
         * @brief get election role
         * @return election role, including "OB_FOLLOWER, OB_CANDIDATE, OB_LEADER"
         */
        ObElectionRole get_role();
        /**
         * @brief get election message
         * @return election message
         */
        common::ObMsgRsElection& get_msg_rselection();        
        //int64_t get_current_term();
        std::vector<ObServer>& get_available_slave_rs();
        // add by guojinwei [obi role switch][multi_cluster] 20150914:b
        int get_other_rs(ObArray<ObServer>& other_rs, int64_t& other_rs_num) const;
        // add:e
        /**
         * @brief get_is_leader
         * @return true means the rs is leader.
         */
        bool get_is_leader();
        /**
         * @brief get_is_lower_log
         * @return true means the max ups_log_timestamp is lower than that in other cluster.
         */
        bool get_is_lower_log();
        bool get_is_init();
        /**
         * @brief get is_send_init_broadcast flag
         * @return is_send_init_broadcast, true or false
         */
        bool get_is_send_init_broadcast();
        /**
         * @brief get extend lease flag, the flag allow or ban rootserver extend lease
         * @return true or false
         */
        bool get_is_extendlease();
        /**
         * @brief set is_send_init_broadcast flag
         * @param is_send_init_broadcast, true of false
         */
        void set_is_send_init_broadcast(bool is_send_init_broadcast);
        /**
         * @brief get lease
         * @return lease
         */
        int64_t get_lease();
        /**
         * @brief set election role
         * @param role
         */
        void set_role(ObElectionRole role);
        /**
         * @brief set vote for rootserver
         * @param the server
         */
        void set_votefor(common::ObServer &);
        /**
         * @brief set vote for rootserver
         * @param ip
         * @param port
         */
        void set_votefor(char ip[20], int port);
        /**
         * @brief set vote count
         * @param count
         */
        void set_count_vote(int count);
        /**
         * @brief set broadcast count
         * @param count
         */
        void set_count_broadcast(int count);
        /**
         * @brief set extend lease count
         * @param count
         */
        void set_count_extendlease_vote(int count);
        /**
         * @brief set is_leader flag
         * @param is_leader, true or false
         */
        void set_is_leader(bool is_leader);
        /**
         * @brief set is_lower_log flag
         * @param is_lower_log
         */
        void set_is_lower_log(bool is_lower_log);
        /**
         * @brief set is_exist_leader flag
         * @param is_exist_leader
         */
        void set_is_exist_leader(bool is_exist_leader);
        //add chujiajia [log synchronization][multi_cluster] 20160603:b
        /**
         * @brief set is_pre_leader flag
         * @param is_pre_leader
         */
        void set_is_pre_leader(bool is_pre_leader);
        //add:e
        /**
         * @brief set leader rootserver info
         * @param the leader rootserver
         */
        void set_leaderinfo(common::ObServer &);
        /**
         * @brief set leader rootserver info
         * @param ip
         * @param port
         */
        void set_leaderinfo(char ip[20], int port);
        /**
         * @brief set lease
         * @param lease
         */
        void set_lease(int64_t lease);
        //delete chujiajia [rs_election][multi_cluster] 20150902:b
        //void set_current_term(int64_t x);
        //delete:e
        /**
         * @brief set timeout mark
         * @param timeout_mark, true or false
         */
        void set_timeoutmark(bool timeout_mark);
        /**
         * @brief set extend lease mark
         * @param extendlease_mark, ture or false
         */
        void set_extendlease_mark(bool extendlease_mark);
        /**
         * @brief set is_extendlease flag
         * @param is_extendlease, ture:need extend lease
         */
        void set_is_extendlease(bool is_extendlease);
        /**
         * @brief reset rootserver election state
         * @return
         */
        int timeout_reset();
        /**
         * @brief reset rootserver election state
         * @return
         */
        int expire_reset();
        /**
         * @brief excute vote request phase in election
         * @return
         */
        int rs_vote();
        /**
         * @brief excute leader broadcast when starting by user,
         * could be called once only
         * @return
         */
        int rs_init_selected_leader_broadcast();
        /**
         * @brief excute leader broadcast phase in election
         * @return
         */
        int rs_broadcast();
        /**
         * @brief extend lease to other slave rootservers
         * @return
         */
        int rs_extendlease();
        int init(rootserver::ObRootServer2 &root_server,
                 common::ObClientManager &client_manager,
                 ObRootRpcStub &rt_rpc_stub, common::ObServer& my_rs,
                 common::ObServer& my_ups, ObArray<ObServer>& other_rs);
        /**
         * @brief set rootserver in local cluster
         * @param the rootserver
         */
        void set_my_rs(common::ObServer &ob_tmp);
        /**
         * @brief set updateserver in local cluster
         * @param the updateserver
         */
        void set_my_ups(common::ObServer &ob_tmp);
        /**
         * @brief compare two server,
         * @param info, the other server
         * @return is equal or not
         */
        bool is_equal_to_other_rs(char info[]);
        ObClientManager* get_client_manager();
        ObRootRpcStub* get_rt_rpc_stub();
        ObRootServer2* get_root_server();
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        bool get_auto_elect_flag();
        void set_auto_elect_flag(bool flag);
        // add:e

      private:
        common::ObServer my_rs_;
        common::ObServer my_ups_;
        ObArray<ObServer> other_rs_;
        ObElectionRole role_;       ///< election role
        common::ObServer votefor_;  ///< the rs that I vote for
        bool is_lower_log_;         ///< my cluster has lower up_max_log
        bool is_slave_rs_online_[MAX_SLAVE_RS_SIZE];
        char other_rs_ip_[MAX_SLAVE_RS_SIZE][ELECTION_ARRAY_SIZE];
        int64_t slave_rs_count_;
        int64_t count_leaderexist_;
        int64_t count_vote_;        ///< granted vote response count
        int64_t count_vote_reject_;
        int64_t count_broadcast_;   ///< granted broadcast response count
        int64_t count_extendlease_vote_;    ///< granted lease response count
        int64_t count_extendlease_reject_;
        bool is_leader_;                    ///< if I'm leader or not
        bool is_exist_leader_;              ///< leader exist in system or not
        common::ObServer leaderinfo_;
        int64_t lease_;                     ///< the lease beturn rootoserver
        //delete chujiajia [rs_election][multi_cluster] 20150902:b
        //int64_t current_term_;
        //delete:e
        bool timeout_mark_;                 ///< last election is timeout or not
        bool extendlease_mark_;             ///< had extend lease or not
        common::ObMsgRsElection msg_rselection_;
        bool is_init_;
        std::vector<ObServer> available_slave_rs_;
        bool is_send_init_broadcast_;       ///< if init broadcast finished or not
        bool is_extendlease_;               ///< allow extend lease or not
        //add chujiajia [log synchronization][multi_cluster] 20160603:b
        bool is_pre_leader_;                ///< if the RS was former leader or not
        //add:e
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        bool auto_elect_flag;               ///< allow election of not, leader rs will set this variable in all rs
        // add:e
      protected:
        ObClientManager *client_manager_;
        ObRootRpcStub *rt_rpc_stub_;
        ObRootServer2 *root_server_;
    };
  }
}
#endif
