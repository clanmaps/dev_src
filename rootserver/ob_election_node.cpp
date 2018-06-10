/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_election_node.cpp
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

#include <vector>
#include "rootserver/ob_election_node.h"
#include "rootserver/ob_ups_manager.h"

using namespace oceanbase::rootserver;
using namespace oceanbase::common;

int ObElectionNode::init(rootserver::ObRootServer2 &root_server,
                         common::ObClientManager &client_manager,
                         ObRootRpcStub &rt_rpc_stub, common::ObServer& my_rs,
                         common::ObServer& my_ups, ObArray<ObServer>& other_rs)
{
  int ret = OB_SUCCESS;
  char ip_tmp[ELECTION_ARRAY_SIZE];
  int64_t i = 0;
  client_manager_ = &client_manager;
  rt_rpc_stub_ = &rt_rpc_stub;
  root_server_ = &root_server;
  set_my_rs(my_rs);
  set_my_ups(my_ups);
  other_rs_ = other_rs;
  slave_rs_count_ = other_rs_.count();
  for (i = 0; i < slave_rs_count_; i++)
  {
    other_rs_.at(i).ip_to_string(other_rs_ip_[i], ELECTION_ARRAY_SIZE);
    TBSYS_LOG(INFO, "other_rs_ip[%ld]=%s", i, other_rs_ip_[i]);
    is_slave_rs_online_[i] = true;
  }
  role_ = OB_FOLLOWER;
  //add chujiajia [rs_election][multi_cluster] 20150909:b
  root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_FOLLOWER);
  root_server_->set_election_role_with_state(common::ObElectionRoleMgr::INIT);
  TBSYS_LOG(INFO, "role:%d",root_server_->get_election_role().get_role());
  // add:e
  is_lower_log_ = false;
  count_leaderexist_ = 0;
  count_vote_ = 0;
  count_vote_reject_ = 0;
  count_broadcast_ = 0;
  count_extendlease_vote_ = 0;
  count_extendlease_reject_ = 0;
  is_leader_ = false;
  is_exist_leader_ = false;
  is_pre_leader_ = false;
  lease_ = 0;
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // current_term_ = 0;
  //delete:e
  timeout_mark_ = true;
  extendlease_mark_ = false;
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // msg_rselection_.term_ = 0;
  //delete:e
  msg_rselection_.lease_ = 0;
  msg_rselection_.type_ = OB_VOTEREQUEST;
  my_rs_.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  msg_rselection_.addr_.set_ipv4_addr(ip_tmp, my_rs_.get_port());
  is_send_init_broadcast_ = false;
  is_extendlease_ = true;
  is_init_ = true;
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  auto_elect_flag = false;
  // add:e

  return ret;
}

bool ObElectionNode::get_is_init()
{
  return is_init_;
}

bool ObElectionNode::get_is_send_init_broadcast()
{
  return is_send_init_broadcast_;
}

ObServer& ObElectionNode::get_my_ups()
{
  return my_ups_;
}

ObServer& ObElectionNode::get_my_rs()
{
  return my_rs_;
}

ObServer& ObElectionNode::get_votefor()
{
  return votefor_;
}

ObServer& ObElectionNode::get_leaderinfo()
{
  return leaderinfo_;
}

ObMsgRsElection& ObElectionNode::get_msg_rselection()
{
  return msg_rselection_;
}

bool ObElectionNode::get_is_exist_leader()
{
  return is_exist_leader_;
}

//add chujiajia [log synchronization][multi_cluster] 20160603:b
bool ObElectionNode::get_is_pre_leader()
{
  return is_pre_leader_;
}
//add:e

bool ObElectionNode::get_is_lower_log()
{
  return is_lower_log_;
}

bool ObElectionNode::get_timeoutmark()
{
  return timeout_mark_;
}

bool ObElectionNode::get_extendlease_mark()
{
  return extendlease_mark_;
}

bool ObElectionNode::get_is_extendlease()
{
  return is_extendlease_;
}

bool ObElectionNode::get_is_leader()
{
  return is_leader_;
}

int64_t ObElectionNode::get_lease()
{
  return lease_;
}

void ObElectionNode::set_role(ObElectionRole role)
{
  role_ = role;
}

void ObElectionNode::set_votefor(common::ObServer &ob_tmp)
{
  char ip_tmp[ELECTION_ARRAY_SIZE];
  ob_tmp.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  votefor_.set_ipv4_addr(ip_tmp, ob_tmp.get_port());
}

void ObElectionNode::set_votefor(char ip[ELECTION_ARRAY_SIZE], int port)
{
  votefor_.set_ipv4_addr(ip, port);
}

void ObElectionNode::set_count_vote(int count)
{
  count_vote_ = count;
}

void ObElectionNode::set_count_broadcast(int count)
{
  count_broadcast_ = count;
}

void ObElectionNode::set_count_extendlease_vote(int count)
{
  count_extendlease_vote_ = count;
}


void ObElectionNode::set_is_leader(bool is_leader)
{
  is_leader_ = is_leader;
}

void ObElectionNode::set_is_extendlease(bool is_extendlease)
{
  is_extendlease_ = is_extendlease;
}

void ObElectionNode::set_is_send_init_broadcast(bool is_send_init_broadcast)
{
  is_send_init_broadcast_ = is_send_init_broadcast;
}

void ObElectionNode::set_is_exist_leader(bool is_exist_leader)
{
  is_exist_leader_ = is_exist_leader;
}

//add chujiajia [log synchronization][multi_cluster] 20160603:b
void ObElectionNode::set_is_pre_leader(bool is_pre_leader)
{
  is_pre_leader_ = is_pre_leader;
}
//add:e

void ObElectionNode::set_is_lower_log(bool is_lower_log)
{
  is_lower_log_ = is_lower_log;
}

void ObElectionNode::set_leaderinfo(common::ObServer &ob_tmp)
{
  char ip_tmp[ELECTION_ARRAY_SIZE];
  ob_tmp.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  leaderinfo_.set_ipv4_addr(ip_tmp, ob_tmp.get_port());
}

void ObElectionNode::set_my_rs(common::ObServer &ob_tmp)
{
  char ip_tmp[ELECTION_ARRAY_SIZE];
  ob_tmp.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  my_rs_.set_ipv4_addr(ip_tmp, ob_tmp.get_port());
}

void ObElectionNode::set_my_ups(common::ObServer &ob_tmp)
{
  char ip_tmp[ELECTION_ARRAY_SIZE];
  ob_tmp.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  my_ups_.set_ipv4_addr(ip_tmp, ob_tmp.get_port());
}

void ObElectionNode::set_leaderinfo(char ip[ELECTION_ARRAY_SIZE], int port)
{
  leaderinfo_.set_ipv4_addr(ip, port);
}

void ObElectionNode::set_lease(int64_t lease)
{
  lease_ = lease;
}

//delete chujiajia [rs_election][multi_cluster] 20150902:b
// void ObElectionNode::set_current_term(int64_t x)
// {
//   current_term_ = x;
// }
//delete:e

void ObElectionNode::set_timeoutmark(bool timeout_mark)
{
  timeout_mark_ = timeout_mark;
}

void ObElectionNode::set_extendlease_mark(bool extendlease_mark)
{
  extendlease_mark_ = extendlease_mark;
}

int ObElectionNode::rs_vote()   //candidate send vote_request and handle the responses
{
  int ret = OB_SUCCESS;
  int64_t i = 0;
  const int64_t all_rs_count = slave_rs_count_ + 1;
  char response_info[all_rs_count][ELECTION_ARRAY_SIZE];
  for (i = 0; i < all_rs_count; i++)
  {
    strcpy(response_info[i], "NO");
  }
  int64_t max_log_timestamp = -1;
  if (OB_SUCCESS
      != (ret = rt_rpc_stub_->get_ups_max_log_timestamp(my_ups_,
          max_log_timestamp, election_message_time_out_us)))
  {
    TBSYS_LOG(WARN, "get_ups_max_log_timestamp error, err=%d", ret);
  }
  else
  {
    //modify chujiajia[rs_election][multi_cluster] 20150923:b
    TBSYS_LOG(INFO, "get_ups_max_log_timestamp suc! max_log_timestamp:%ld",
        max_log_timestamp);
    //modify:e
  }
  if (max_log_timestamp >= 0)
  {
    ///change to candidate
    role_ = OB_CANDIDATE;
    //delete chujiajia [rs_election][multi_cluster] 20150902:b
    // current_term_++;
    // msg_rselection_.term_ = current_term_;
    //delete:e
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_CANDIDATE);
    root_server_->set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
    // add:e
    msg_rselection_.type_ = OB_VOTEREQUEST;
    msg_rselection_.max_log_timestamp_ = max_log_timestamp;
    ///send vote request message to self
    if (OB_SUCCESS
        != (ret = rt_rpc_stub_->rs_election(my_rs_, msg_rselection_,
            response_info[0], election_message_time_out_us)))
    {
      TBSYS_LOG(WARN,
          "rs_vote:rt_rpc_stub_->rs_election to my_rs_ error, err=%d", ret);
    }
    if (strcmp(response_info[0], "YES") == 0)
    {
      count_vote_++;
      TBSYS_LOG(INFO,
          "rs_vote response_info[0]:YES,count_vote_++, count_vote_=%ld",
          count_vote_);
    }
    ///send vote request message to other rs
    for (i = 0; i < slave_rs_count_; i++)
    {
      if (OB_SUCCESS
          != (ret = rt_rpc_stub_->rs_election(other_rs_.at(i), msg_rselection_,
              response_info[i + 1], election_message_time_out_us)))
      {
        TBSYS_LOG(WARN,
            "rs_vote:rt_rpc_stub_->rs_election to other_rs_.at(%ld) error, err=%d",
            i + 1, ret);
        is_slave_rs_online_[i] = false;
      }
      else
      {
        is_slave_rs_online_[i] = true;
        if (strcmp(response_info[i + 1], "YES") == 0)
        {
          count_vote_++;
          TBSYS_LOG(INFO,
              "rs_vote:response_info %ld :YES,count_vote_++, count_vote_=%ld",
              i + 1, count_vote_);
        }
        else if (strcmp(response_info[i + 1], "LOWER_LOG") == 0)
        {
          is_lower_log_ = true;
        }
        //delete chujiajia [rs_election][multi_cluster] 20150902:b
        // else if (strcmp(response_info[i + 1], "NO") != 0
        // && (atoi(response_info[i + 1]) >= 0)
        // && (current_term_ < atoi(response_info[i + 1])))
        // {
        // current_term_ = atoi(response_info[i + 1]);
        // TBSYS_LOG(INFO,
        // "rs_vote:change current_term=atoi(response_info %ld),currrent_term_=%ld",
        // i + 1, current_term_);
        // }
        //delete:e
      }
    }
    //modify chujiajia [rs_election][multi_cluster] 20151110:b
    //if (count_vote_ >= ((slave_rs_count_ + 2) / 2))
    ///get mjority granted response or not
    if (count_vote_ >= ((slave_rs_count_ + 1) / 2 + 1))
    //modify:e
    {
      TBSYS_LOG(INFO, "rs_vote:count_vote_:%ld", count_vote_);
      count_vote_ = 0;
      rs_broadcast();
    }
    count_vote_ = 0;
    count_leaderexist_ = 0;
    count_vote_reject_ = 0;
  }
  return ret;
}

int ObElectionNode::rs_init_selected_leader_broadcast()  //leader set by administrator send leader_broadcast and handle the responses
{
  int ret = OB_SUCCESS;
  int64_t i = 0;
  count_broadcast_ = 0;
  const int64_t con_slave_rs_count = slave_rs_count_;
  char response_info[con_slave_rs_count][ELECTION_ARRAY_SIZE];
  for (i = 0; i < slave_rs_count_; i++)
  {
    strcpy(response_info[i], "NO");
  }
  msg_rselection_.type_ = OB_BROADCAST;
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // msg_rselection_.term_ = current_term_;
  //delete:e
  int64_t now;
  now = tbsys::CTimeUtil::getTime();
  msg_rselection_.lease_ = now + election_set_leader_lease_length_us;
  count_broadcast_++;
  ///send broadcast message to other rs
  for (i = 0; i < slave_rs_count_; i++)
  {
    if (OB_SUCCESS
        != (ret = rt_rpc_stub_->rs_election(other_rs_.at(i), msg_rselection_,
            response_info[i], election_message_time_out_us)))
    {
      TBSYS_LOG(WARN,
          "rs_init_selected_leader_broadcast:rt_rpc_stub_->rs_election response_info %s error, err=%d",
          response_info[i], ret);
      is_slave_rs_online_[i] = false;
    }
    else
    {
      is_slave_rs_online_[i] = true;
      if (strcmp(response_info[i], "YES") == 0)
      {
        count_broadcast_++;
      }
    }
  }
  if (count_broadcast_ != (slave_rs_count_ + 1))
  {
    TBSYS_LOG(WARN,
        "rs_init_selected_leader_broadcast:not all slave receive selected leader's broadcast!");
  }
  //modify chujiajia [rs_election][multi_cluster] 20151110:b
  //if (count_broadcast_ >= ((slave_rs_count_ + 2) / 2))
  ///get majority granted response or not
  if (count_broadcast_ >= ((slave_rs_count_ + 1) / 2 + 1))
  //modify:e
  {
    TBSYS_LOG(INFO, "rs_init_selected_leader_broadcast:count_broadcast_:%ld",
        count_broadcast_);
    lease_ = msg_rselection_.lease_;
    role_ = OB_LEADER;
    set_leaderinfo(my_rs_);
    set_votefor(my_rs_);
    is_leader_ = true;
    is_exist_leader_ = true;
    is_extendlease_ = true;
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    ///change to leader
    root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_LEADER);
    root_server_->set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    //add:e
    //delete chujiajia [rs_election][multi_cluster] 20150902:b
    // current_term_ = msg_rselection_.term_;
    //delete:e
    TBSYS_LOG(INFO,
        "rs_init_selected_leader_broadcast:leader exist,leader is mine!");
    is_send_init_broadcast_ = true;
    ret = OB_SUCCESS;
    //modify chujiajia [rs_election][multi_cluster] 20150902:b
    while (!is_init_ || !root_server_->get_is_have_inited())
    {
      usleep(1000 * 10);
    }
    //modify:e
    if (OB_SUCCESS != (ret = root_server_->grant_ups_lease(true)))
    {
      TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
    }
  }
  return ret;
}

int ObElectionNode::rs_broadcast()   //candidate who succeed in vote phase send leader_broadcast and handle the responses
{
  int ret = OB_SUCCESS;
  int64_t i = 0;
  count_broadcast_ = 0;
  const int64_t con_slave_rs_count = slave_rs_count_;
  char response_info[con_slave_rs_count][ELECTION_ARRAY_SIZE];
  for (i = 0; i < slave_rs_count_; i++)
  {
    strcpy(response_info[i], "NO");
  }
  msg_rselection_.type_ = OB_BROADCAST;
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  // msg_rselection_.term_ = current_term_;
  //delete:e
  int64_t now;
  now = tbsys::CTimeUtil::getTime();
  msg_rselection_.lease_ = now + election_lease_length_us;
  count_broadcast_++;
  ///send broadcast message to other rs
  for (i = 0; i < slave_rs_count_; i++)
  {      
    if (OB_SUCCESS
        != (ret = rt_rpc_stub_->rs_election(other_rs_.at(i), msg_rselection_,
            response_info[i], election_message_time_out_us)))
    {
      TBSYS_LOG(WARN,
          "rs_broadcast:rt_rpc_stub_->rs_election response_info %s error, err=%d",
          response_info[i], ret);
      is_slave_rs_online_[i] = false;
    }
    else
    {
      is_slave_rs_online_[i] = true;
      if (strcmp(response_info[i], "YES") == 0)
      {
        count_broadcast_++;
      }
    }
  }
  //modify chujiajia [rs_election][multi_cluster] 20151110:b
  //if (count_broadcast_ >= ((slave_rs_count_ + 2) / 2))
  ///get mjority granted response or not
  if (count_broadcast_ >= ((slave_rs_count_ + 1) / 2 + 1))
  //modify:e
  {
    TBSYS_LOG(INFO, "rs_broadcast:count_broadcast_:%ld", count_broadcast_);
    lease_ = msg_rselection_.lease_;
    role_ = OB_LEADER;
    set_leaderinfo(my_rs_);
    set_votefor(my_rs_);
    is_leader_ = true;
    is_exist_leader_ = true;
    is_extendlease_ = true;
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    ///change to leader
    root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_LEADER);
    root_server_->set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    //add:e
    //delete chujiajia [rs_election][multi_cluster] 20150902:b
    // current_term_ = msg_rselection_.term_;
    //delete:e
    TBSYS_LOG(INFO, "rs_broadcast:leader exist,leader is mine!");
    now = tbsys::CTimeUtil::getTime();
    ret = OB_SUCCESS;
    //modify chujiajia [rs_election][multi_cluster] 20150902:b
    while (!is_init_ || !root_server_->get_is_have_inited())
    {
      usleep(1000 * 10);
    }
    //modify:e
    if (OB_SUCCESS != (ret = root_server_->grant_ups_lease(true)))
    {
      TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
    }
  }
  return ret;
}

int ObElectionNode::rs_extendlease()    //leader send extendlease_request to all RS and handle the responses
{
  int ret = OB_SUCCESS;
  int64_t i = 0;
  const int64_t con_slave_rs_count = slave_rs_count_;
  char response_info[con_slave_rs_count][ELECTION_ARRAY_SIZE];
  for (i = 0; i < slave_rs_count_; i++)
  {
    strcpy(response_info[i], "NO");
  }
  msg_rselection_.type_ = OB_EXTENDLEASE;
  //delete chujiajia [rs_election][multi_cluster] 20150902:b
  //msg_rselection_.term_ = current_term_;
  //delete:e
  msg_rselection_.lease_ = lease_ + election_lease_length_us;
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  msg_rselection_.auto_elect_flag = auto_elect_flag;
  // add:e
  ///verify the ups is valid before extend lease
  if (!root_server_->is_master_ups_lease_valid())
  {
    TBSYS_LOG(WARN, "rs_extendlease:master_ups lease is expire!");
  }
  else
  {
    TBSYS_LOG(INFO, "rs_extendlease:master_ups lease is not expire!");
    count_extendlease_vote_++;
    ///send extend lease message to other rs
    for (i = 0; i < slave_rs_count_; i++)
    {
      if (OB_SUCCESS
          != (ret = rt_rpc_stub_->rs_election(other_rs_.at(i), msg_rselection_,
              response_info[i], election_message_time_out_us)))
      {
        TBSYS_LOG(WARN,
            "rs_extendlease: rt_rpc_stub_->rs_election error, err=%d", ret);
      }
      if (strcmp(response_info[i], "YES") == 0)
      {
        count_extendlease_vote_++;
      }
    }
    //modify chujiajia [rs_election][multi_cluster] 20151110:b
    //if (count_extendlease_vote_ >= ((slave_rs_count_ + 2) / 2))
    if (count_extendlease_vote_ >= ((slave_rs_count_ + 1) / 2 + 1))
    //modify:e
    {
      /// update self lease
      lease_ = msg_rselection_.lease_;
      TBSYS_LOG(INFO, "rs_extendlease:count_extendlease_vote:%ld",
          count_extendlease_vote_);
      ret = OB_SUCCESS;
      if (OB_SUCCESS != (ret = root_server_->grant_ups_lease(true)))
      {
        TBSYS_LOG(WARN, "root_server_->grant_ups_lease(true) error!");
      }
    }
    extendlease_mark_ = false;
    // set_is_extendlease(false);
    // TBSYS_LOG(INFO,"set_is_extendlease(false)");
    count_extendlease_vote_ = 0;
    count_extendlease_reject_ = 0;
  }
  return ret;
}

//delete chujiajia [rs_election][multi_cluster] 20150902:b
// int64_t ObElectionNode::get_current_term()
// {
// return current_term_;
// }
//delete:e

int ObElectionNode::timeout_reset()
{
  int ret = OB_SUCCESS;
  role_ = OB_FOLLOWER;
  votefor_.reset();
  count_leaderexist_ = 0;
  count_vote_ = 0;
  count_broadcast_ = 0;
  count_extendlease_vote_ = 0;
  count_vote_reject_ = 0;
  count_extendlease_reject_ = 0;
  is_leader_ = false;
  is_exist_leader_ = false;
  leaderinfo_.reset();
  lease_ = 0;
  extendlease_mark_ = false;
  timeout_mark_ = true;
  //add chujiajia [rs_election][multi_cluster] 20150909:b
  ///change to follower
  root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_FOLLOWER);
  root_server_->set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
  // add:e
  return ret;
}

int ObElectionNode::expire_reset()
{
  int ret = OB_SUCCESS;
  role_ = OB_FOLLOWER;
  votefor_.reset();
  count_leaderexist_ = 0;
  count_vote_ = 0;
  count_broadcast_ = 0;
  count_extendlease_vote_ = 0;
  count_vote_reject_ = 0;
  count_extendlease_reject_ = 0;
  is_leader_ = false;
  is_exist_leader_ = false;
  leaderinfo_.reset();
  lease_ = 0;
  extendlease_mark_ = false;
  //modify chujiajia [rs_election][multi_cluster] 20150912:b
  timeout_mark_ = true;
  //modify:e
  //add chujiajia [rs_election][multi_cluster] 20150909:b
  ///change to follower
  root_server_->set_election_role_with_role(common::ObElectionRoleMgr::OB_FOLLOWER);
  root_server_->set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
  //add:e
  return ret;
}

ObElectionRole ObElectionNode::get_role()
{
  return role_;
}

ObClientManager* ObElectionNode::get_client_manager()
{
  return client_manager_;
}

ObRootRpcStub* ObElectionNode::get_rt_rpc_stub()
{
  return rt_rpc_stub_;
}

ObRootServer2* ObElectionNode::get_root_server()
{
  return root_server_;
}

// add by zcd [multi_cluster] 20150405:b
std::vector<ObServer>& ObElectionNode::get_available_slave_rs()
{
  available_slave_rs_.clear();
  for (unsigned int i = 0; i < slave_rs_count_; i++)
  {
    if (is_slave_rs_online_[i] == true)
    {
      TBSYS_LOG(INFO, "available_slave_rs_.push_back(otheraddr1_)");
      available_slave_rs_.push_back(other_rs_.at(i));
    }
  }
  TBSYS_LOG(INFO, "available_slave_rs_ count=%ld", available_slave_rs_.size());

  for (unsigned int i = 0; i < available_slave_rs_.size(); i++)
  {
    TBSYS_LOG(INFO, "ObElectionNode:: available_slave_rs_tmp.at(i):%s",
        to_cstring(available_slave_rs_.at(i)));
  }
  TBSYS_LOG(INFO, "get_available_slave_rs end");
  return available_slave_rs_;
}
// add:e

bool ObElectionNode::is_equal_to_other_rs(char info[])
{
  bool ret = false;
  int64_t i;
  for (i = 0; i < slave_rs_count_; i++)
  {
    if (strcmp(info, other_rs_ip_[i]) == 0)
    {
      ret = true;
    }
  }
  return ret;
}

// add by guojinwei [obi role switch][multi_cluster] 20150914:b
int ObElectionNode::get_other_rs(ObArray<ObServer>& other_rs, int64_t& other_rs_num) const
{
  int ret = OB_SUCCESS;
  other_rs = other_rs_;
  other_rs_num = slave_rs_count_;
  return ret;
}
// add:e

// add by zhangcd [rs_election][auto_elect_flag] 20151129:b
bool ObElectionNode::get_auto_elect_flag()
{
  return auto_elect_flag;
}

void ObElectionNode::set_auto_elect_flag(bool flag)
{
  TBSYS_LOG(INFO, "set_auto_elect_flag set to %s", flag ? "true" : "false");
  auto_elect_flag = flag;
}
// add:e
