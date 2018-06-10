/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_election_role_mgr.h
 * @brief record rs election information
 * This file is designed for recording and managing rs election role and rs election state
 *
 * @version CEDAR 0.2 
 * @author guojinwei <guojinwei@stu.ecnu.edu.cn>
 * @date 2015_12_30
 */
#ifndef OCEANBASE_COMMON_OB_ELECTION_ROLE_MGR_H_
#define OCEANBASE_COMMON_OB_ELECTION_ROLE_MGR_H_

#include "ob_atomic.h"
#include "serialization.h"
#include <tbsys.h>

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief ObElectionRoleMgr
     * This class is designed for recording and managing rs election role and rs election state
     */
    class ObElectionRoleMgr
    {
      public:
        /**
         * @brief rs election role enum
         */
        enum Role
        {
          OB_FOLLOWER = 0,  ///< follower
          OB_CANDIDATE,     ///< candidate
          OB_LEADER,        ///< leader
        };

        /**
         * @brief election state enum
         */
        enum State
        {
          INIT = 0,         ///< init
          DURING_ELECTION,  ///< during_election
          AFTER_ELECTION,   ///< after_election
        };

      public:
        /**
         * @brief constructor
         */
        ObElectionRoleMgr() : role_(OB_CANDIDATE), state_(INIT)
        {
        }

        /**
         * @brief destructor
         */
        virtual ~ObElectionRoleMgr() { }

        /**
         * @brief get rs election role
         * @return rs election role
         */
        inline Role get_role() const {return role_;}

        /**
         * @brief set rs election role
         * @param[in] role  rs election role
         */
        inline void set_role(const Role role)
        {
          // TBSYS_LOG(INFO, "before set_role=%s state=%s", get_role_str(), get_state_str());
          atomic_exchange(reinterpret_cast<uint32_t*>(&role_), role);
          // TBSYS_LOG(INFO, "after set_role=%s state=%s", get_role_str(), get_state_str());
        }

        /**
         * @brief get rs election state
         * @return rs election state
         */
        inline State get_state() const {return state_;}

        /**
         * @brief set rs election state
         * @param[in] state  rs election state
         */
        inline void set_state(const State state)
        {
          // TBSYS_LOG(INFO, "before set_state=%s role=%s", get_state_str(), get_role_str());
          atomic_exchange(reinterpret_cast<uint32_t*>(&state_), state);
          // TBSYS_LOG(INFO, "after set_state=%s role=%s", get_state_str(), get_role_str());
        }

        /**
         * @brief get string of rs election role
         * @return the pointer of the string
         */
        inline const char* get_role_str() const
        {
          switch (role_)
          {
            case OB_FOLLOWER:   return "OB_FOLLOWER";
            case OB_CANDIDATE:  return "OB_CANDIDATE";
            case OB_LEADER:     return "OB_LEADER";
            default:            return "UNKNOWN";
          }
        }

        /**
         * @brief get string of rs election state
         * @return the pointer of the string
         */
        inline const char* get_state_str() const
        {
          switch (state_)
          {
            case INIT:             return "INIT";
            case DURING_ELECTION:  return "DURING_ELECTION";
            case AFTER_ELECTION:   return "AFTER_ELECTION";
            default:               return "UNKNOWN";
          }
        }

        /**
         * @brief whether it is master according to rs election information
         * @return true if it's master
         *         false if it's not master
         */
        inline bool is_master() const
        {
          return (role_ == ObElectionRoleMgr::OB_LEADER) && (state_ == ObElectionRoleMgr::AFTER_ELECTION);
        }

        /**
         * @brief serialize the instance of ObElectionRoleMgr
         * @return OB_SUCCESS if success
         *         OB_ERROR if not success
         */
        int serialize(char* buf, const int64_t len, int64_t& pos) const
        {
          int err = OB_SUCCESS;
          if (OB_SUCCESS != (err = serialization::encode_i64(buf, len, pos, role_))
              || OB_SUCCESS != (err = serialization::encode_i64(buf, len, pos, state_)))
          {
            TBSYS_LOG(ERROR, "ups_role_mgr.serialize(buf=%p[%ld], pos=%ld)=>%d", buf, len, pos, err);
          }
          return err;
        }

        /**
         * @brief deserialize the instance of ObElectionRoleMgr
         * @return OB_SUCCESS if success
         *         OB_ERROR if not success
         */
        int deserialize(const char* buf, const int64_t len, int64_t& pos)
        {
          int err = OB_SUCCESS;
          if (OB_SUCCESS != (err = serialization::decode_i64(buf, len, pos, (int64_t*)&role_))
              || OB_SUCCESS != (err = serialization::decode_i64(buf, len, pos, (int64_t*)&state_)))
          {
            TBSYS_LOG(ERROR, "ups_role_mgr.deserialize(buf=%p[%ld], pos=%ld)=>%d", buf, len, pos, err);
          }
          return err;
        }


      private:
        Role role_;       ///< rs election role
        State state_;     ///< rs election state
    };
  } // end namespace common
} // end namespace oceanbase

#endif // OCEANBASE_COMMON_OB_ELECTION_ROLE_MGR_H_
