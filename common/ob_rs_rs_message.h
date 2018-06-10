/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_rs_rs_message.h
 * @brief message among rootservers for exchanging info about election.
 *        add the auto_elect_flag into the serialization process
 *        in case of the communication between all the rootserver.
 * @version CEDAR 0.2 
 * @author
 *   Chu Jiajia  <52151500014@ecnu.cn>
 *   zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_08_23
 */
#ifndef _OB_RS_RS_MESSAGE_H
#define _OB_RS_RS_MESSAGE_H

#include "ob_server.h"
#include "data_buffer.h"
#include "ob_result.h"
#include "ob_obi_role.h"

namespace oceanbase
{
  namespace common
  {
    struct ObMsgRsElection
    {
        static const int MY_VERSION = 1;
        ObServer addr_;
        int64_t lease_;
        int64_t max_log_timestamp_;
        //delete chujiajia [rs_election][multi_cluster] 20150902:b
        //int64_t term_;
        //delete:e
        int64_t type_;
        char server_version_[OB_SERVER_VERSION_LENGTH];
        // add by zhangcd [rs_election][auto_elect_flag] 20151129:b
        bool auto_elect_flag;
        // add:e
        int serialize(char* buf, const int64_t buf_len, int64_t& pos) const;
        int deserialize(const char* buf, const int64_t data_len, int64_t& pos);
    };
  } // end namespace common
} // end namespace oceanbase

#endif /* _OB_RS_RS_MESSAGE_H */
