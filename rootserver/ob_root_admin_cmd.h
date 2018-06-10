/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_define.h
 * @brief support multiple clusters for HA by adding or modifying
 *        some functions, member variables
 *        add OB_RS_ADMIN_SET_OBI_MASTER_RS and OB_RS_ADMIN_SET_OBI_MASTER_FIRST
 *        to rs_admin processing.
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 *         zhangcd<zhangcd_ecnu@ecnu.cn>
 * @date 2015_12_30
 */
/*
 * Copyright (C) 2007-2012 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Description here
 *
 * Version: $Id$
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *     - some work details here
 */
#ifndef _OB_ROOT_ADMIN_CMD_H
#define _OB_ROOT_ADMIN_CMD_H 1

namespace oceanbase
{
  namespace rootserver
  {
    static const int OB_RS_ADMIN_CHECKPOINT = 1;
    static const int OB_RS_ADMIN_RELOAD_CONFIG = 2;
    static const int OB_RS_ADMIN_SWITCH_SCHEMA = 3;
    static const int OB_RS_ADMIN_DUMP_ROOT_TABLE = 4;
    static const int OB_RS_ADMIN_DUMP_SERVER_INFO = 5;
    static const int OB_RS_ADMIN_INC_LOG_LEVEL = 6;
    static const int OB_RS_ADMIN_DEC_LOG_LEVEL = 7;
    static const int OB_RS_ADMIN_DUMP_UNUSUAL_TABLETS = 8;
    static const int OB_RS_ADMIN_DUMP_MIGRATE_INFO = 9;
    static const int OB_RS_ADMIN_ENABLE_BALANCE = 11;
    static const int OB_RS_ADMIN_DISABLE_BALANCE = 12;
    static const int OB_RS_ADMIN_ENABLE_REREPLICATION = 13;
    static const int OB_RS_ADMIN_DISABLE_REREPLICATION = 14;
    static const int OB_RS_ADMIN_CLEAN_ERROR_MSG = 15;
    static const int OB_RS_ADMIN_BOOT_STRAP = 16;
    static const int OB_RS_ADMIN_BOOT_RECOVER = 17;
    static const int OB_RS_ADMIN_REFRESH_SCHEMA = 18;
    static const int OB_RS_ADMIN_INIT_CLUSTER = 19;
    static const int OB_RS_ADMIN_CLEAN_ROOT_TABLE = 20;
    static const int OB_RS_ADMIN_CHECK_SCHEMA = 21;
    // add by zcd [multi_cluster] 20150416:b
    /// allocate two numbers for the rs_admin tools
    static const int OB_RS_ADMIN_SET_OBI_MASTER_RS = 22;
    static const int OB_RS_ADMIN_SET_OBI_MASTER_FIRST = 23;
    // add:e
    // add by guojinwei [new rs_admin command]{multi_cluster} 20150901:b
    static const int OB_RS_ADMIN_REELECT = 24;
    // add:e
  } // end namespace rootserver
} // end namespace oceanbase

#endif /* _OB_ROOT_ADMIN_CMD_H */

