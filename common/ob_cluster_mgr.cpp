/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_cluster_mgr.cpp
 * @brief record clusters information
 * This file is designed for recording clusters information
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */
#include "ob_cluster_mgr.h"
#include "utility.h"

namespace oceanbase
{
  namespace common
  {
    ObCluster::ObCluster(): cluster_id_(-1)
    {}

    ObCluster::~ObCluster()
    {}

    DEFINE_SERIALIZE(ObCluster)
    {
      int err = OB_SUCCESS;
      int64_t tmp_pos = pos;
      if (NULL == buf || 0 > buf_len || 0 > pos || pos > buf_len)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if(OB_SUCCESS != (err = rs_.serialize(buf, buf_len, tmp_pos)))
      {
        TBSYS_LOG(ERROR, "fail to serialize ObServer (buf=%p, len=%ld, pos=%ld)=>%d", buf, buf_len, tmp_pos, err);
      }

      else if(OB_SUCCESS != (err = serialization::encode_i64(buf, buf_len, tmp_pos, cluster_id_)))
      {
        TBSYS_LOG(ERROR, "encode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, buf_len, tmp_pos, cluster_id_, err);
      }
      else
      {
        pos = tmp_pos;
      }
      return err;
    }

    DEFINE_DESERIALIZE(ObCluster)
    {
      int err = OB_SUCCESS;
      int64_t tmp_pos = pos;
      if (NULL == buf || 0 > data_len || 0 > pos || pos > data_len)
      {
        err = OB_INVALID_ARGUMENT;
      }
      else if(OB_SUCCESS != (err = rs_.deserialize(buf, data_len, tmp_pos)))
      {
        TBSYS_LOG(ERROR, "fail to deserialize ObServer (buf=%p, len=%ld, pos=%ld)=>%d", buf, data_len, tmp_pos, err);
      }
      else if(OB_SUCCESS != (err = serialization::decode_i64(buf, data_len, tmp_pos, (int64_t*)&cluster_id_)))
      {
        TBSYS_LOG(ERROR, "decode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, data_len, tmp_pos, cluster_id_, err);
      }
      else
      {
        pos = tmp_pos;
      }
      return err;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObCluster)
    {
      int64_t total_size = 0;
      total_size += rs_.get_serialize_size();
      total_size += serialization::encoded_length_i64(cluster_id_);
      return total_size;
    }

    ObClusterMgr::ObClusterMgr()
    {
      clusters_.init(MAX_SERVER_COUNT, data_holder_);
    }

    ObClusterMgr::ObClusterMgr(const ObClusterMgr& other)
    {
      memcpy(data_holder_, other.data_holder_, sizeof(other.data_holder_));
      clusters_ = other.clusters_;
      cluster_num_ = other.cluster_num_;
    }

    ObClusterMgr::~ObClusterMgr()
    {}

    ObClusterMgr::iterator ObClusterMgr::begin()
    {
      return clusters_.get_base_address();
    }

    ObClusterMgr::const_iterator ObClusterMgr::begin() const
    {
      return clusters_.get_base_address();
    }

    ObClusterMgr::iterator ObClusterMgr::end()
    {
      return clusters_.get_base_address() +
        clusters_.get_array_index();
    }

    ObClusterMgr::const_iterator ObClusterMgr::end() const
    {
      return clusters_.get_base_address() +
        clusters_.get_array_index();
    }

    int ObClusterMgr::add_cluster(const ObServer& server, const int64_t& cluster_id)
    {
      int ret = OB_SUCCESS;
      iterator it = find_by_server(server);
      if (it != end())
      {
        it->cluster_id_ = cluster_id;    // new cluster_id
        TBSYS_LOG(INFO, "cluster already exist! server=%s, new cluster_id=%ld", it->rs_.to_cstring(), it->cluster_id_);
      }
      else
      {
        ObCluster tmp_cluster;
        tmp_cluster.rs_ = server;
        tmp_cluster.cluster_id_ = cluster_id;
        ret = clusters_.push_back(tmp_cluster);
        cluster_num_++;
      }
      return ret;
    }

    ObClusterMgr::iterator ObClusterMgr::find_by_ip(const ObServer& server)
    {
      iterator res = end();
      for (int64_t i = 0; i < clusters_.get_array_index(); i++)
      {
        if (clusters_.at(i) == NULL)
        {
          TBSYS_LOG(ERROR, "never reach this, bugs");
        }
        else
        {
          if (!server.compare_by_ip((clusters_.at(i))->rs_) && !(clusters_.at(i))->rs_.compare_by_ip(server))
          {
            res = clusters_.at(i);
            break;
          }
        }
      }
      return res;
    }

    ObClusterMgr::const_iterator ObClusterMgr::find_by_ip(const ObServer& server) const
    {
      const_iterator res = end();
      for (int64_t i = 0; i < clusters_.get_array_index(); i++)
      {
        if (clusters_.at(i) == NULL)
        {
          TBSYS_LOG(ERROR, "never reach this, bugs");
        }
        else
        {
          if (!server.compare_by_ip((clusters_.at(i))->rs_) && !(clusters_.at(i))->rs_.compare_by_ip(server))
          {
            res = clusters_.at(i);
            break;
          }
        }
      }
      return res;
    }

    ObClusterMgr::iterator ObClusterMgr::find_by_server(const ObServer& server)
    {
      iterator res = end();
      for (int64_t i = 0; i < clusters_.get_array_index(); i++)
      {
        if (clusters_.at(i) == NULL)
        {
          TBSYS_LOG(ERROR, "never reach this, bugs");
        }
        else
        {
          if ((clusters_.at(i))->rs_ == server)
          {
            res = clusters_.at(i);
            break;
          }
        }
      }
      return res;
    }

    const ObCluster* ObClusterMgr::get_cluster(const int64_t index) const
    {
      const ObCluster* ret = NULL;
      if (index < cluster_num_)
      {
        ret = data_holder_ + index;
      }
      else
      {
        TBSYS_LOG(ERROR, "never should reach this");
      }
      return ret;
    }

    int64_t ObClusterMgr::get_cluster_num() const
    {
      return cluster_num_;
    }

    DEFINE_SERIALIZE(ObClusterMgr)
    {
      int ret = OB_SUCCESS;
      int64_t tmp_pos = pos;
      int64_t size = cluster_num_;
      if (OB_SUCCESS != (ret = serialization::encode_vi64(buf, buf_len, tmp_pos, size)))
      {
        TBSYS_LOG(ERROR, "encode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, buf_len, tmp_pos, size, ret);
      }
      else
      {
        for (int64_t i = 0; i < size; ++i)
        {
          if (OB_SUCCESS != (ret = data_holder_[i].serialize(buf, buf_len, tmp_pos)))
          {
            break;
          }
        }
      }
      if (OB_SUCCESS == ret)
      {
        pos = tmp_pos;
      }
      return ret;
    }

    DEFINE_DESERIALIZE(ObClusterMgr)
    {
      int ret = OB_ERROR;
      int64_t size = 0;
      if (OB_SUCCESS != (ret = serialization::decode_vi64(buf, data_len, pos, &size)))
      {
        TBSYS_LOG(ERROR, "decode_i64(buf=%p, len=%ld, pos=%ld, i=%ld)=>%d", buf, data_len, pos, cluster_num_, ret);
      }
      else
      {
        cluster_num_ = size;
        for (int64_t i = 0; i < cluster_num_; i++)
        {
          ObCluster cluster;
          if (OB_SUCCESS != (ret = cluster.deserialize(buf, data_len, pos)))
          {
            break;
          }
          clusters_.push_back(cluster);
        }
      }
      return ret;
    }

    DEFINE_GET_SERIALIZE_SIZE(ObClusterMgr)
    {
      int64_t total_size = 0;
      total_size += serialization::encoded_length_vi64(cluster_num_);
      for (int64_t i = 0; i < cluster_num_; ++i)
      {
        total_size += data_holder_[i].get_serialize_size();
      }
      return total_size;
    }

    // add by guojinwei [multi_cluster] 20151203:b
    ObClusterMgr& ObClusterMgr::operator= (const ObClusterMgr& other)
    {
      if (this != &other)
      {
        memcpy(data_holder_, other.data_holder_, sizeof(other.data_holder_));
        clusters_ = other.clusters_;
        cluster_num_ = other.cluster_num_;
      }
      return *this;
    }
    // add:e
  } // end namespace common
} // end namespace oceanbase
