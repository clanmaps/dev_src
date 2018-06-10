/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file  ob_index_service.h
 * @brief ObIndexService provide interface of inner system table, with scan / update
 *        generally use it to modify index info
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */

#ifndef OB_INDEX_SERVICE_H
#define OB_INDEX_SERVICE_H
#include "common/ob_define.h"
#include "common/ob_packet.h"
#include "common/roottable/ob_scan_helper.h"
#include "common/nb_accessor/ob_nb_accessor.h"
#include "common/nb_accessor/nb_query_res.h"

/// define update code of mutator

#define UPDATE_INT(table_name, rowkey, column_name, value) \
if(OB_SUCCESS == ret) \
{ \
  ObObj int_value; \
  int_value.set_int(value); \
  ret = mutator->update(table_name, rowkey, OB_STR(column_name), int_value); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "update value to mutator fail:column_name[%s], ret[%d]", column_name, ret); \
  } \
}

#define OB_STR(str) \
  ObString(0, static_cast<int32_t>(strlen(str)), const_cast<char *>(str))

using namespace oceanbase::common;
using namespace oceanbase::common::nb_accessor;
namespace oceanbase
{

  namespace rootserver
  {
    class ObRootWorker;
    /**
     * @brief The ObIndexService class
     * ObIndexService is designed for
     * scan inner system table
     * modify inner system table
     */
    class ObIndexService
    {
      public:
        /**
         * @brief constructor
         */
        ObIndexService();

        /**
         * @brief destructor
         */
        ~ObIndexService();

        /**
         * @brief init inner system param.
         * @param worker a link field of rootwoker
         * @return err code if success or not.
         */
        void init(ObRootWorker* worker);

        /**
         * @brief set inner env for scan client.
         * @param client_proxy a link of ScanHelper
         * @return err code if success or not.
         */
        int  set_env(ObScanHelper* client_proxy);

        /**
         * @brief set index id.
         * @param idx intex tid
         */
        void set_index_tid(uint64_t idx){index_tid_ = idx;}

      public:
        /**
         * @brief fetch index stat by table id, cluster id from inner system table.
         * @param table_id  index table id.
         * @param cluster_id cluster id
         * @param stat output param status of index
         * @return err code if success or not.
         */
        int fetch_index_stat(const uint64_t table_id, const int64_t cluster_id, int64_t &stat);

        /**
         * @brief submit a job packet to rootserver.
         * @param pc PacketCode to be send
         * @return err code if success or not.
         */
        int submit_job(const common::PacketCode pc);

        /**
         * @brief modify index process info in inner system table.
         * @param index_tid index id.
         * @param stat index status to update.
         * @return if success or not.
         */
        int modify_index_process_info(const uint64_t index_tid, const IndexStatus stat);

        /**
         * @brief modify index stat by ddl in inner table.
         * @param index_table_name index name.
         * @param index_table_id index id.
         * @param stat index status to update
         * @return if success or not.
         */
        int modify_index_stat_ddl(ObString index_table_name, uint64_t index_table_id, int stat);

        /**
         * @brief create a mutator to update info in inner system table.
         * @param index_table_name index name
         * @param status index status to update
         * @param mutator output param, a link of ObMutator
         * @return err code if success or not.
         */
        int create_index_status_mutator(ObString index_table_name, int status, ObMutator* mutator);

        /**
         * @brief get index status from inner system table by index id and cluster_count.
         *        if all cluster has finished, return NOT_AVAILABLE, else return INIT
         * @param table_id index id.
         * @param cluster_count number of cluster
         * @param stat output param, index stat
         * @return err code if success or not.
         */
        int get_index_stat(const uint64_t table_id, const int64_t cluster_count, IndexStatus &stat);
      private:

        uint64_t        index_tid_;                     ///< index id
        ObRootWorker    *root_worker_;                  ///< a link field of root worker
        ObScanHelper    *client_proxy_;                 ///< scan helper, to access inner table, a link field
        nb_accessor::ObNbAccessor nb_accessor_;         ///< ObNbAccessor, is used to modify inner system table
        tbsys::CThreadMutex mutex_;                     ///< mutex lock

    };

  }   //rootserver
}     //oceanbase
#endif // OB_INDEX_SERVICE_H

