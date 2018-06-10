/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_check_rselection.h
 * @brief manage election procedure
 * Check and extend rootserver lease, and handle election procedure.
 *           Add a auto_elect_flag into election process.
 *           When the flag is true, if the leader disappeared,
 *           the followers would timeout and not tranform to candidate.
 *           the candiate would tranform to follower.
 * @version CEDAR 0.2 
 * @author
 *   Chu Jiajia  <52151500014@ecnu.cn>
 *   Pang Tianze <pangtianze@ecnu.com>
 *   zhangcd <zhangcd_ecnu@ecnu.cn>
 * @date 2015_08_23
 */

#ifndef OB_CHECK_RSELECTION_H_
#define OB_CHECK_RSELECTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "tbsys.h"
#include "common/ob_election_role_mgr.h"
#include "rootserver/ob_election_node.h"

//add chujiajia [rs_election][multi_cluster] 20151026:b
const int64_t defalut_rs_election_check_period_us = 20 * 1000;
const int64_t defalut_pre_leader_wait_time_us = 10 * 1000 * 1000;
//add:e

namespace oceanbase
{
using namespace common;
namespace rootserver
{
// del by zhangcd [multi_clusters] 20151120:b
//enum ObElectionState {
//	INIT = 0, DURING_ELECTION, AFTER_ELECTION,
//};
// del:e
class ObCheckRsElection: public tbsys::CDefaultRunnable
{
public:
  //mod by pangtianze [rs_election] 20160106
  ObCheckRsElection(ObRootServerConfig &rs_config, ObRootServer2 &root_server);
  //ObCheckRsElection(ObRootServerConfig &rs_config);
  //mod:e
	virtual ~ObCheckRsElection();
	virtual void run(tbsys::CThread* thread, void* arg);
    //modify chujiajia [rs_election][multi_cluster] 20151026:b
  //int init(const int64_t check_period, rootserver::ObRootServer2 &root_server,
  //		   common::ObClientManager &client_manager,
  //		   rootserver::ObRootRpcStub &rt_rpc_stub, common::ObServer& my_rs,
  //		   common::ObServer& my_ups, ObArray<ObServer>& other_rs);
	int init(rootserver::ObRootServer2 &root_server,
			 common::ObClientManager &client_manager,
			 rootserver::ObRootRpcStub &rt_rpc_stub,
			 common::ObServer& my_rs,
			 common::ObServer& my_ups,
			 ObArray<ObServer>& other_rs);
	//modify:e
	rootserver::ObElectionNode& get_ob_election_node();
  // modify by zhangcd [multi_clusters] 20151120:b
  // ObElectionState get_election_state();
  /**
   * @brief get election state, including "INIT, DURING_ELECTION, AFTER_ELECTION"
   * @return election state
   */
  ObElectionRoleMgr::State get_election_state();
  // modify:e
  /**
   * @brief get election role, including "OB_FOLLOWER, OB_CANCELED, OB_LEADER"
   * @return election role
   */
  ObElectionRole get_election_role();
  /**
   * @brief get inited flag of check election thread
   * @return is_inited
   */
  bool get_is_inited();
  /**
   * @brief get run election flag representing election is running or not
   * @return is_run_election
   */
  bool get_is_run_election();
  /**
   * @brief set run election flag
   * @param is_run_electon, true or false
   */
  void set_is_run_election(bool is_run_electon);
  /**
   * @brief set election start time
   * @param current_time
   */
  void set_starttime(int64_t current_time);
  /**
   * @brief set election end time
   * @param current_time
   */
	void set_endtime(int64_t current_time);
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  bool get_auto_elect_flag();
  void set_auto_elect_flag(bool flag);
  // add:e
  //add by pangtianze [rs_election] 20160106:b
  /**
   * @brief handle received rs_extend_lease message
   * @param msg
   * @param responseinfo, response info to source rs
   * @return true means successed
   */
  int handle_rs_extend_lease(const common::ObMsgRsElection &msg, char responseinfo[]);
  /**
   * @brief handle received vote_request message
   * @param msg
   * @param responseinfo, response info to source rs
   * @return true means successed
   */
  int handle_vote_request(const common::ObMsgRsElection &msg, char responseinfo[]);
  /**
   * @brief handle received broadcast message
   * @param msg
   * @param responseinfo, response info to source rs
   * @return true means successed
   */
  int handle_broadcast(const common::ObMsgRsElection &msg, char responseinfo[]);
  /**
   * @brief handle received expire message
   * @param msg
   * @param responseinfo, response info to source rs
   * @return true means successed
   */
  int handle_expire(char responseinfo[]);
  /**
   * @brief handle received electiontimeout message
   * @param msg
   * @param responseinfo, response info to source rs
   * @return true means successed
   */
  int handle_electiontimeout(char responseinfo[]);
  //add:e
protected:
  //modify chujiajia [rs_election][multi_cluster] 20151026:b
  //int64_t check_period_;
  int64_t election_protection_time_us_;    ///< the ahead of time before leader lease expiration
  int64_t rs_election_check_period_;       ///< check election state interval
  //modify:e
  rootserver::ObElectionNode ob_election_node_;
  common::ObMsgRsElection msg_rselection_; ///< election message
  time_t starttime_;                       ///< election start time
  time_t endtime_;                         ///< election start time
  bool is_run_election_;                   ///< mark the election_thread is running or not
  bool is_inited_;
  //add chujiajia [rs_election][multi_cluster] 20150929:b
  const int64_t &rs_election_random_wait_time_; ///< the min interval before start vote
  // add:e
  //add by pangtianze [rs_election] 20160106:b
  ObRootServer2 &root_server_;
  //add:e
};
} // end namespace common
} // end namespace oceanbase//add chujiajia [rs_election][multi_cluster] 20150929:b
#endif /* OB_CHECK_RSELECTION_H_ */
