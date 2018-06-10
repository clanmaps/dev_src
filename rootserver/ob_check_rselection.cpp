/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_check_rselection.cpp
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
     zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_08_23
 */

#include <time.h>
#include "common/ob_lease_common.h"
#include "rootserver/ob_check_rselection.h"

using namespace oceanbase::common;
using namespace oceanbase::rootserver;

//modify by pangtianze [rs_election] 20160106:b
//modify chujiajia [rs_election][multi_cluster] 20150929:b
//ObCheckRsElection::ObCheckRsElection(ObRootServerConfig &rs_config):rs_election_random_wait_time_(rs_config.rs_election_random_wait_time)
//{
//  // TODO Auto-generated constructor stub
//  is_inited_ = false;
//}
//modify:e
ObCheckRsElection::ObCheckRsElection(ObRootServerConfig &rs_config, ObRootServer2 &root_server)
    :
      election_protection_time_us_(rs_config.rs_election_protection_time_us),
      rs_election_random_wait_time_(rs_config.rs_election_random_wait_time),
      root_server_(root_server)
    {
        is_inited_ = false;
    }
//modify:e

ObCheckRsElection::~ObCheckRsElection()
{
  // TODO Auto-generated destructor stub
}

void ObCheckRsElection::run(tbsys::CThread* thread, void* arg)
{
  UNUSED(thread);
  UNUSED(arg);
  while (!_stop)
  {
    //delete chujiajia [rs_election][multi_cluster] 20150910:b
    // int64_t now = tbsys::CTimeUtil::getTime();
    // if (ob_election_node_.get_role() == OB_LEADER
    // && ((ob_election_node_.get_lease() - now) >= election_protection_time_us)
    //     && (ob_election_node_.get_is_extendlease()))
    // {
    //    if ((ob_election_node_.get_lease() - now) < (election_do_extend_time_us)
    //       && ob_election_node_.get_extendlease_mark() == false)
    //    {
    //      ob_election_node_.set_extendlease_mark(true);
    //      TBSYS_LOG(INFO,
    //                "rs_leader_broadcast:lease-now<%ld,lease-now=:%ld,rs_extendlease()",
    //                election_do_extend_time_us / static_cast<int64_t>(1000000),
    //                (ob_election_node_.get_lease() - now)
    //                / static_cast<int64_t>(1000000));
    //
    //      ob_election_node_.rs_extendlease();
    //    }
    //    if (OB_SUCCESS
    //        != (ob_election_node_.root_server_->grant_ups_lease(false)))
    //    {
    //      TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
    //    }
    //    now = tbsys::CTimeUtil::getTime();
    // }
    // if (ob_election_node_.get_is_send_init_broadcast())
    // {
    //   set_is_run_election(true);
    //   //add chujiajia [rs_election][multi_cluster] 20150909:b
    //   if (!ob_election_node_.get_is_exist_leader())
    //   {
    //     ob_election_node_.root_server_->set_election_role_with_state(
    //         common::ObElectionRoleMgr::DURING_ELECTION);
    //   }
    //   else
    //   {
    //      ob_election_node_.root_server_->set_election_role_with_state(
    //      common::ObElectionRoleMgr::AFTER_ELECTION);
    //   }
    //   // add:e
    //   ob_election_node_.set_is_send_init_broadcast(false);
    //   TBSYS_LOG(INFO, "leader check_rselection_thread is run!");
    // }
    // if (is_run_election_)
    // {
    //   if (ob_election_node_.get_role() == OB_FOLLOWER
    //       && ob_election_node_.get_timeoutmark()
    //       && !ob_election_node_.get_is_exist_leader()
    //       && ob_election_node_.get_leaderinfo().get_ipv4() == 0
    //       && ob_election_node_.get_votefor().get_ipv4() == 0)
    //   {
    //     int t = rand() % 150;
    //     t = (t + 150) * 1000;
    //     if (ob_election_node_.get_is_lower_log())
    //     {
    //       t = t * 2;
    //     }
    //     usleep(t);
    //     TBSYS_LOG(INFO, "usleep(t):%d,start election!", t);
    //   }
    //   if (ob_election_node_.get_votefor().get_ipv4() == 0
    //       && !ob_election_node_.get_is_exist_leader())
    //   {
    //     ob_election_node_.rs_vote();
    //   }
    //
    //   if (ob_election_node_.get_is_exist_leader()
    //       && (ob_election_node_.get_lease() != 0))
    //   {
    //     now = tbsys::CTimeUtil::getTime();
    //     TBSYS_LOG(INFO, "lease=%ld,now=%ld,ObCheckRsElection:lease-now:%ld",
    //               ob_election_node_.get_lease(), now,
    //               (ob_election_node_.get_lease() - now)
    //                / static_cast<int64_t>(1000000));
    //     if (ob_election_node_.get_is_leader())
    //     {
    //       if ((ob_election_node_.get_lease() - now)
    //           < election_protection_time_us)
    //       {
    //         char info1[ELECTION_ARRAY_SIZE] = "NO";
    //         msg_rselection_.type_ = OB_EXPIRE;
    //         ob_election_node_.rt_rpc_stub_->rs_election(
    //             ob_election_node_.get_my_rs(), msg_rselection_, info1,
    //             election_message_time_out_us);
    //         if (strcmp(info1, "YES") == 0)
    //         {
    //           TBSYS_LOG(INFO, "leader expire! reset succ!");
    //           if (OB_SUCCESS
    //               != (ob_election_node_.root_server_->grant_ups_lease(true)))
    //           {
    //             TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
    //           }
    //         }
    //      }
    //      if (OB_SUCCESS
    //          != (ob_election_node_.root_server_->grant_ups_lease(false)))
    //      {
    //        TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
    //      }
    //     }
    //     else
    //     {
    //       if ((ob_election_node_.get_lease() - now) < 0)
    //       {
    //         char info1[ELECTION_ARRAY_SIZE] = "NO";
    //         msg_rselection_.type_ = OB_EXPIRE;
    //         ob_election_node_.rt_rpc_stub_->rs_election(
    //             ob_election_node_.get_my_rs(), msg_rselection_, info1,
    //             election_message_time_out_us);
    //         if (strcmp(info1, "YES") == 0)
    //         {
    //           TBSYS_LOG(INFO, "slave expire! reset succ!");
    //         }
    //       }
    //     }
    //   }
    //   if ((ob_election_node_.get_votefor().get_ipv4() != 0)
    //        && (ob_election_node_.get_is_exist_leader() == false))
    //   {
    //     endtime_ = time(NULL);
    //     if (difftime(endtime_, starttime_) > 1)
    //     {
    //       char info1[ELECTION_ARRAY_SIZE] = "NO";
    //       msg_rselection_.type_ = OB_ELECTIONTIMEOUT;
    //       ob_election_node_.rt_rpc_stub_->rs_election(
    //           ob_election_node_.get_my_rs(), msg_rselection_, info1,
    //           election_message_time_out_us);
    //       if (strcmp(info1, "YES") == 0)
    //       {
    //         TBSYS_LOG(INFO, "election timeout! reset succ!");
    //       }
    //     }
    //   }
    // }
    //delete:e
    //add chujiajia [rs_election][multi_cluster] 20150910:b
    if (ob_election_node_.get_role() == OB_LEADER)
    {
      int64_t now = tbsys::CTimeUtil::getTime();
      /// check if init broadcast finished
      if (ob_election_node_.get_is_send_init_broadcast())
      {
        set_is_run_election(true);
        ob_election_node_.set_is_send_init_broadcast(false);
        TBSYS_LOG(INFO, "leader check_rselection_thread is run!");
      }
      if (is_run_election_)
      {
        /// leader lease will expired 200ms ahead
        if ((ob_election_node_.get_lease() - now) < election_protection_time_us_)
        {
        /// if expired, reset election state
          char info1[ELECTION_ARRAY_SIZE] = "NO";
          msg_rselection_.type_ = OB_EXPIRE;
          ob_election_node_.set_is_pre_leader(true);
          ob_election_node_.get_rt_rpc_stub()->rs_election(
              ob_election_node_.get_my_rs(), msg_rselection_, info1,
              election_message_time_out_us);
          if (strcmp(info1, "YES") == 0)
          {
            TBSYS_LOG(INFO, "leader expire! reset succ!");
            /// update ups's lease
            if (OB_SUCCESS
                != (ob_election_node_.get_root_server()->grant_ups_lease(true)))
            {
              TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
            }
          }
        }
        /// leader extend lease
        else if ((ob_election_node_.get_lease() - now)
            >= election_protection_time_us_
            && ((ob_election_node_.get_lease() - now)
                < (election_do_extend_time_us))
            && (ob_election_node_.get_is_extendlease())
            && ob_election_node_.get_extendlease_mark() == false)
        {
          ob_election_node_.set_extendlease_mark(true);
          TBSYS_LOG(INFO,
              "rs_leader_broadcast:lease-now<%ld,lease-now=:%ld,rs_extendlease()",
              election_do_extend_time_us / static_cast<int64_t>(1000000),
              (ob_election_node_.get_lease() - now)
                  / static_cast<int64_t>(1000000));
          // extend lease to other rs
          ob_election_node_.rs_extendlease();
          if (OB_SUCCESS
              != (ob_election_node_.get_root_server()->grant_ups_lease(false)))
          {
            TBSYS_LOG(WARN, "root_server_->grant_ups_lease() error!");
          }
          now = tbsys::CTimeUtil::getTime();
        }

      }
    }
    else if (ob_election_node_.get_role() == OB_FOLLOWER)
    {
      // modify by zhangcd [rs_election][auto_elect_flag] 20151129:b
      //if (is_run_election_)
      if (is_run_election_)
      // modify:e
      {
        if (!ob_election_node_.get_is_exist_leader()
            && ob_election_node_.get_leaderinfo().get_ipv4() == 0)
        {
          if (ob_election_node_.get_timeoutmark()
              && ob_election_node_.get_votefor().get_ipv4() == 0)
          {
            /// cal random time for waiting before excute election
            //add chujiajia [log synchronization][multi_cluster] 20160603:b
            if(ob_election_node_.get_is_pre_leader())
            {
              usleep(defalut_pre_leader_wait_time_us);
              ob_election_node_.set_is_pre_leader(false);
            }
            //add:e
            //modify chujiajia [rs_election][multi_cluster] 20150929:b
        	TBSYS_LOG(INFO, "rs_election_random_wait_time_:%d ms", ((int)(rs_election_random_wait_time_))/1000);
            int t = rand() % (((int)(rs_election_random_wait_time_))/1000);
            t = t*1000 + (int)(rs_election_random_wait_time_);
            //modify:e
            if (ob_election_node_.get_is_lower_log())
            {
              /// means ups has lower log, so waif for more time before election
              t = t * 2;
            }
            usleep(t);
            TBSYS_LOG(INFO, "usleep(t):%d,start election!", t);
            if (!ob_election_node_.get_is_exist_leader()
                && ob_election_node_.get_leaderinfo().get_ipv4() == 0)
            {
              starttime_ = time(NULL);
              if(ob_election_node_.get_auto_elect_flag())
              {
                /// excute rr vote request phase
                ob_election_node_.rs_vote();
              }
            }
          }
          if (ob_election_node_.get_votefor().get_ipv4() != 0)
          {
            endtime_ = time(NULL);
            /// if election duration time is lager than 1, will reset election state
            if (difftime(endtime_, starttime_) > 1)
            {
              char info1[ELECTION_ARRAY_SIZE] = "NO";
              msg_rselection_.type_ = OB_ELECTIONTIMEOUT;
              ob_election_node_.get_rt_rpc_stub()->rs_election(
                  ob_election_node_.get_my_rs(), msg_rselection_, info1,
                  election_message_time_out_us);
              if (strcmp(info1, "YES") == 0)
              {
                TBSYS_LOG(INFO, "[Follower]election timeout! reset succ!");
              }
            }
          }
        }
        else
        {
          int64_t now = tbsys::CTimeUtil::getTime();
          /// rs lease expired, reset election state
          if ((ob_election_node_.get_lease() - now) < 0)
          {
            char info1[ELECTION_ARRAY_SIZE] = "NO";
            msg_rselection_.type_ = OB_EXPIRE;
            ob_election_node_.get_rt_rpc_stub()->rs_election(
                ob_election_node_.get_my_rs(), msg_rselection_, info1,
                election_message_time_out_us);
            if (strcmp(info1, "YES") == 0)
            {
              TBSYS_LOG(INFO, "slave expire! reset succ!");
            }
          }
        }
      }
    }
    else if(ob_election_node_.get_role() == OB_CANDIDATE)
    {
      // modify by zhangcd [rs_election][auto_elect_flag] 20151129:b
      // if (is_run_election_)
      if (is_run_election_ && ob_election_node_.get_auto_elect_flag())
      // modify:e
      {
        endtime_ = time(NULL);
        if (difftime(endtime_, starttime_) > 1)
        {
          char info1[ELECTION_ARRAY_SIZE] = "NO";
          msg_rselection_.type_ = OB_ELECTIONTIMEOUT;
          ob_election_node_.get_rt_rpc_stub()->rs_election(
          ob_election_node_.get_my_rs(), msg_rselection_, info1,
          election_message_time_out_us);
          if (strcmp(info1, "YES") == 0)
          {
            TBSYS_LOG(INFO, "[Candidate] election timeout! reset succ!");
          }
        }
      }
      // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
      /// if auto_elect_flag is set false，current rs should convert itself from OB_CANDIDATE to OB_FOLLOWER
      else if(!ob_election_node_.get_auto_elect_flag())
      {
        char info1[ELECTION_ARRAY_SIZE] = "NO";
        msg_rselection_.type_ = OB_ELECTIONTIMEOUT;
        ob_election_node_.get_rt_rpc_stub()->rs_election(
        ob_election_node_.get_my_rs(), msg_rselection_, info1,
        election_message_time_out_us);
        if (strcmp(info1, "YES") == 0)
        {
          TBSYS_LOG(INFO, "[Candidate] election timeout! reset succ!");
        }
      }
      // add:e
    }
    // add:e
    //modify chujiajia [rs_election][multi_cluster] 20151025:b
    usleep(static_cast<useconds_t>(rs_election_check_period_));
    //modify:e
  }
}

//modify chujiajia [rs_election][multi_cluster] 20151026:b
//int ObCheckRsElection::init(const int64_t check_period,
//                            rootserver::ObRootServer2 &root_server,
//                            common::ObClientManager &client_manager,
//                            ObRootRpcStub &rt_rpc_stub, common::ObServer& my_rs,
//                            common::ObServer& my_ups,
//                            ObArray<ObServer>& other_rs)
int ObCheckRsElection::init(rootserver::ObRootServer2 &root_server,
                            common::ObClientManager &client_manager,
                            ObRootRpcStub &rt_rpc_stub,
                            common::ObServer& my_rs,
                            common::ObServer& my_ups,
                            ObArray<ObServer>& other_rs)
//modify:e
{
  int ret = OB_SUCCESS;
  ob_election_node_.init(root_server, client_manager, rt_rpc_stub, my_rs,
      my_ups, other_rs);
  //modify chujiajia [rs_election][multi_cluster] 20151026:b
  rs_election_check_period_ = defalut_rs_election_check_period_us;
  //check_period_ = check_period;
  //modify:e
  starttime_ = 0;
  endtime_ = 0;
  is_run_election_ = false;
  is_inited_ = true;
  return ret;
}

bool ObCheckRsElection::get_is_inited()
{
  return is_inited_;
}

bool ObCheckRsElection::get_is_run_election()
{
  return is_run_election_;
}

void ObCheckRsElection::set_is_run_election(bool is_run_electon)
{
  is_run_election_ = is_run_electon;
}

ObElectionNode& ObCheckRsElection::get_ob_election_node()
{
  return ob_election_node_;
}

// modify by zhangcd [muti_clusters] 20151120:b
//ObElectionState ObCheckRsElection::get_election_state()
//{
//  ObElectionState ret;
//  if (!is_run_election_)
//  {
//    ret = INIT;
//  }
//  else if (!ob_election_node_.get_is_exist_leader())
//  {
//    ret = DURING_ELECTION;
//  }
//  else
//  {
//    ret = AFTER_ELECTION;
//  }
//  return ret;
//}
ObElectionRoleMgr::State ObCheckRsElection::get_election_state()
{
  ObElectionRoleMgr::State ret;
  if (!is_run_election_)
  {
    ret = ObElectionRoleMgr::INIT;
  }
  else if (!ob_election_node_.get_is_exist_leader())
  {
    ret = ObElectionRoleMgr::DURING_ELECTION;
  }
  else
  {
    ret = ObElectionRoleMgr::AFTER_ELECTION;
  }
  return ret;
}
// modify:e

ObElectionRole ObCheckRsElection::get_election_role()
{
  return ob_election_node_.get_role();
}

void ObCheckRsElection::set_starttime(int64_t current_time)
{
  starttime_ = current_time;
}

void ObCheckRsElection::set_endtime(int64_t current_time)
{
  endtime_ = current_time;
}

// add by zhangcd [rs_election][auto_elect_flag] 20151129:b
bool ObCheckRsElection::get_auto_elect_flag()
{
  return ob_election_node_.get_auto_elect_flag();
}

void ObCheckRsElection::set_auto_elect_flag(bool flag)
{
  TBSYS_LOG(INFO, "set_auto_elect_flag set to %s", flag ? "true" : "false");
  ob_election_node_.set_auto_elect_flag(flag);
}
// add:e
//add by pangtianze [rs_election] 20160106:b
int ObCheckRsElection::handle_vote_request(const common::ObMsgRsElection &msg, char responseinfo[])
{
  int ret = OB_SUCCESS;
  if(!get_auto_elect_flag())
  {
    strcpy(responseinfo, "NO");
  }
  else
  {
    if(!get_is_run_election())
    {
      set_is_run_election(true);
      //add chujiajia [rs_election][multi_cluster] 20150909:b
      /// set election state
      if(!get_ob_election_node().get_is_exist_leader())
      {
        root_server_.set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
      }
      else
      {
        root_server_.set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
      }
      // add:e
      TBSYS_LOG(INFO, "slave check_rselection_thread is run!");
    }
    int64_t max_log_timestamp = -1;
    /// get log_max_timestamp from ups
    TBSYS_LOG(INFO, "rt_rs_election:msg.log_max_timestamp_=%ld", msg.max_log_timestamp_);
    if (OB_SUCCESS != (ret = root_server_.get_rpc_stub().get_ups_max_log_timestamp(
                       get_ob_election_node().get_my_ups(),
                       max_log_timestamp, election_message_time_out_us)))
    {
      TBSYS_LOG(WARN, "rt_rs_election:rt_rpc_stub_->get_ups_max_log_timestamp error, err=%d",ret);
    }
    TBSYS_LOG(INFO, "rt_rs_election:get_ups_max_log_timestamp=%ld", max_log_timestamp);
    /// max_log_timestamp>=0 means ups is normal
    if(((int64_t)(max_log_timestamp)) >= 0)
    {
      if ((msg.max_log_timestamp_ >= max_log_timestamp)
           && (get_ob_election_node().get_votefor().get_ipv4() == 0))
      {
        char ip_tmp[ELECTION_ARRAY_SIZE];
        msg.addr_.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
        get_ob_election_node().set_votefor(ip_tmp,msg.addr_.get_port());
        set_starttime(time(NULL));
        strcpy(responseinfo, "YES");
      }
      else if(msg.max_log_timestamp_ <max_log_timestamp)
      {
        TBSYS_LOG(INFO, "rt_rs_election:msg.max_log_timestamp_<max_log_timestamp, LOWER_LOG");
        strcpy(responseinfo, "LOWER_LOG");
      }
    }
  }
  return ret;
}

int ObCheckRsElection::handle_rs_extend_lease(const common::ObMsgRsElection &msg, char responseinfo[])
{
  int ret = OB_SUCCESS;
  char ip_tmp[ELECTION_ARRAY_SIZE];
  char ip_receive[ELECTION_ARRAY_SIZE];
  get_ob_election_node().get_leaderinfo().ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
  msg.addr_.ip_to_string(ip_receive, ELECTION_ARRAY_SIZE);
  // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
  set_auto_elect_flag(msg.auto_elect_flag);
  // add:e
  if(!get_is_run_election())
  {
    set_is_run_election(true);
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    if(!get_ob_election_node().get_is_exist_leader())
    {
      root_server_.set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
    }
    else
    {
      root_server_.set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    }
    // add:e
    TBSYS_LOG(INFO, "slave check_rselection_thread is run!");
  }
  //add chujiajia [rs_election][multi_cluster] 20150923:b
  int64_t max_log_timestamp = -1;
  /// get ups_max_log from ups
  if (OB_SUCCESS != (ret = root_server_.get_rpc_stub().get_ups_max_log_timestamp(
                    get_ob_election_node().get_my_ups(),
                    max_log_timestamp, election_message_time_out_us)))
  {
    TBSYS_LOG(WARN, "rt_rs_election:rt_rpc_stub_->get_ups_max_log_timestamp error, err=%d",ret);
  }
  TBSYS_LOG(INFO, "rt_rs_election:get_ups_max_log_timestamp=%ld", max_log_timestamp);
  if (get_ob_election_node().get_leaderinfo().get_ipv4() == 0)
  {
    get_ob_election_node().set_leaderinfo(ip_receive, msg.addr_.get_port());
    get_ob_election_node().set_is_exist_leader(true);
    get_ob_election_node().set_role(OB_FOLLOWER);
    root_server_.set_election_role_with_role(common::ObElectionRoleMgr::OB_FOLLOWER);
    root_server_.set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    get_ob_election_node().set_is_leader(false);
    ///update lease
    get_ob_election_node().set_lease(msg.lease_);
    get_ob_election_node().set_is_lower_log(false);
    TBSYS_LOG(INFO,"Receive extend_lease request! leader is %s,lease=%ld",
                   ip_receive, get_ob_election_node().get_lease());
  }
  else if (strcmp(ip_tmp, ip_receive) == 0)
  {
    get_ob_election_node().set_lease(msg.lease_);
    TBSYS_LOG(INFO,"Receive extend_lease request! leader is %s,lease=%ld",
                    ip_tmp, get_ob_election_node().get_lease());
    // reply YES iff (1) ip of leader does not change and (2) ups is alive
    if (((int64_t)(max_log_timestamp)) >= 0)
    {
      strcpy(responseinfo, "YES");
    }
  }
  return ret;
}

int ObCheckRsElection::handle_broadcast(const common::ObMsgRsElection &msg, char responseinfo[])
{
  int ret = OB_SUCCESS;
  if(!get_is_run_election())
  {
    set_is_run_election(true);
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    if(!get_ob_election_node().get_is_exist_leader())
    {
      root_server_.set_election_role_with_state(common::ObElectionRoleMgr::DURING_ELECTION);
    }
    else
    {
      root_server_.set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    }
    // add:e
    TBSYS_LOG(INFO, "slave check_rselection_thread is run!");
  }
  if (get_ob_election_node().get_leaderinfo().get_ipv4() == 0)
  {
    char ip_tmp[ELECTION_ARRAY_SIZE];
    msg.addr_.ip_to_string(ip_tmp, ELECTION_ARRAY_SIZE);
    get_ob_election_node().set_leaderinfo(ip_tmp,msg.addr_.get_port());
    get_ob_election_node().set_role(OB_FOLLOWER);
    get_ob_election_node().set_is_leader(false);
    get_ob_election_node().set_is_exist_leader(true);
    //add chujiajia [rs_election][multi_cluster] 20150909:b
    root_server_.set_election_role_with_role(common::ObElectionRoleMgr::OB_FOLLOWER);
    root_server_.set_election_role_with_state(common::ObElectionRoleMgr::AFTER_ELECTION);
    // add:e
    get_ob_election_node().set_is_lower_log(false);
    get_ob_election_node().set_lease(msg.lease_);
    //delete chujiajia [rs_election][multi_cluster] 20150902:b
    // check_rselection_thread_.get_ob_election_node().set_current_term(int(msg.term_));
    //delete:e
    strcpy(responseinfo, "YES");
    //delete chujiajia [rs_election][multi_cluster] 20150902:b
    // TBSYS_LOG(INFO, "Receive broadcast,leader is %s,lease=%ld,term=%ld",
    //                 ip_tmp,
    //                 check_rselection_thread_.get_ob_election_node().get_lease(),
    //                 check_rselection_thread_.get_ob_election_node().get_current_term());
    //delete:e
    TBSYS_LOG(INFO, "Receive broadcast,leader is %s,lease=%ld",
                    ip_tmp,
                    get_ob_election_node().get_lease());
  }
  return ret;
}

int ObCheckRsElection::handle_expire(char responseinfo[])
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS == (ret = get_ob_election_node().expire_reset()))
  {
    strcpy(responseinfo, "YES");
    TBSYS_LOG(INFO, "expire_reset()==OB_SUCCESS");
  }
  return ret;
}

int ObCheckRsElection::handle_electiontimeout(char responseinfo[])
{
  int ret = OB_SUCCESS;
  if (OB_SUCCESS == (ret = get_ob_election_node().timeout_reset()))
  {
    strcpy(responseinfo, "YES");
    TBSYS_LOG(INFO, "timeout_reset()==OB_SUCCESS");
  }
  return ret;
}
//add:e
