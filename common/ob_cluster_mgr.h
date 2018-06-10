/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cluster_mgr.h
 * @brief record clusters information
 * This file is designed for recording and managing clusters information
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */
#ifndef OCEANBASE_COMMON_OB_CLUSTER_MGR_H_
#define OCEANBASE_COMMON_OB_CLUSTER_MGR_H_

#include "ob_define.h"
#include "ob_server.h"
#include "serialization.h"
#include "ob_array_helper.h"
#include "ob_array.h"

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief ObCluster
     * This structure is designed for recording information of a cluster. (e.g. ip, port, cluster_id)
     */
    struct ObCluster
    {
      ObServer rs_;
      int64_t  cluster_id_;
      ObCluster();
      ~ObCluster();
      NEED_SERIALIZE_AND_DESERIALIZE;
    };

    /**
     * @brief ObClusterMgr
     * This class is designed for recording and managing clusters information.
     */
    class ObClusterMgr
    {
      public:
        enum {
          MAX_SERVER_COUNT = 7,
        };
        typedef ObCluster* iterator;
        typedef const ObCluster* const_iterator;

        /**
         * @brief constructor
         */
        ObClusterMgr();

        /**
         * @brief copy constructor
         */
        ObClusterMgr(const ObClusterMgr& other);

        /**
         * @brief destructor
         */
        ~ObClusterMgr();

        /**
         * @brief add a cluster into ObClusterMgr
         * @param[in] server  the rs ip of the new cluster
         * @param[in] cluster_id  the cluster id of the new cluster
         * @return OB_SUCCESS if success
         */
        int add_cluster(const ObServer& server, const int64_t& cluster_id);

        /**
         * @brief get cluster by rs ip of the cluster
         * @param[in] server  the rs ip of the new cluster
         * @return pointer of the cluster
         */
        iterator find_by_ip(const ObServer& server);

        /**
         * @brief get cluster by rs ip of the cluster
         * @param[in] server  the rs ip of the new cluster
         * @return pointer of the cluster
         */
        const_iterator find_by_ip(const ObServer& server) const;

        /**
         * @brief get cluster by rs server of the cluster
         * @param[in] server  the rs server of the new cluster
         * @return pointer of the cluster
         */
        iterator find_by_server(const ObServer& server);

        /**
         * @brief get cluster by index
         * @param[in] server  the index in ObClusterMgr
         * @return pointer of the cluster
         */
        const ObCluster* get_cluster(const int64_t index) const;

        /**
         * @brief get the number of clusters
         * @return the number of clusters
         */
        int64_t get_cluster_num() const;

        /**
         * @brief the serialization and deserialization of ObClusterMgr
         */
        NEED_SERIALIZE_AND_DESERIALIZE;

        // add by guojinwei [multi_cluster] 20151203:b
        /**
         * @brief the operator overloading
         */
        ObClusterMgr& operator= (const ObClusterMgr& other);
        // add:e

      private:
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;

      private:
        mutable tbsys::CRWLock cluster_mgr_lock_;
        ObCluster data_holder_[MAX_SERVER_COUNT];   ///< the array of cluster
        ObArrayHelper<ObCluster> clusters_;
        int64_t cluster_num_;                       ///< the number of clusters
    };
  } // end namespace common
} // end namespace oceanbase

#endif //OCEANBASE_COMMON_OB_CLUSTER_MGR_H_
