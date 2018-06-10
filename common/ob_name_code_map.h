/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_name_code_map.h
 * @brief define ObNameCodeMap class for procedure cache management
 *
 * Created by wangdonghui
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_29
 */

#ifndef OB_COMMON_NAME_CODE_MAP_H_
#define OB_COMMON_NAME_CODE_MAP_H_

#include "common/ob_string.h"
#include "common/ob_string_buf.h"
#include "common/hash/ob_hashmap.h"

namespace oceanbase
{
  namespace common
  {
    /**
     * @brief The ObNameCodeMap class
     * the class designed for cache management
     */
    class ObNameCodeMap
    {
      public:
        typedef hash::ObHashMap<ObString, int64_t> ObProcHashCache;  ///<  typedef ObProcHashCache type
        /**
         * @brief The ObNameCodeIterator class
         * name code map iterator
         */
        class ObNameCodeIterator
        {
          public:
            /**
             * @brief ObNameCodeIterator constructor
             * @param name_code_map ObNameCodeMap object
             */
            ObNameCodeIterator(const ObNameCodeMap &name_code_map) : code_map_(name_code_map),
              iter_(name_code_map.name_code_map_.begin())
            {
            }
            /**
             * @brief next
             * next name code map
             * @return error code
             */
            int next()
            {
              iter_++;
              return OB_SUCCESS;
            }
            /**
             * @brief get_proc_name
             * get procedure name
             * @return procedure name
             */
            const ObString & get_proc_name() const
            {
              return iter_->first;
            }
            /**
             * @brief get_sour_code
             * get procedure source code
             * @return procedure source code
             */
            const ObString & get_sour_code() const
            {
              return iter_->second;
            }
            /**
             * @brief end
             * judge iterator arrived code map end
             * @return bool value
             */
            bool end()
            {
              return iter_ == code_map_.name_code_map_.end();
            }

          private:
            const ObNameCodeMap &code_map_;  ///< name code map
            hash::ObHashMap<ObString, ObString>::const_iterator iter_;  ///<  name code map iterator
        };

        friend class ObNameCodeIterator;
        /**
         * @brief ObNameCodeMap constructor
         */
        ObNameCodeMap();
        /**
         * @brief destructor
         */
        virtual ~ObNameCodeMap();
        /**
         * @brief init
         * Initialization name code map
         * @return error code
         */
        int init();
        /**
         * @brief serialize
         * serialize object
         * @param buf
         * @param data_len
         * @param pos
         * @return error code
         */
        int serialize(char* buf, const int64_t data_len, int64_t& pos) const;
        /**
         * @brief deserialize
         * deserialize object
         * @param buf
         * @param data_len
         * @param pos
         * @return
         */
        int deserialize(const char* buf, const int64_t data_len, int64_t& pos);
        /**
         * @brief put_source_code
         * insert a stored procedure source
         * @param proc_name procedure name
         * @param sour_code procedure source
         * @return error code
         */
        int put_source_code(const ObString &proc_name, const ObString &sour_code);
        /**
         * @brief del_source_code
         * delete a stored procedure source
         * @param proc_name procedure name
         * @return error code
         */
        int del_source_code(const ObString &proc_name);
        /**
         * @brief get_source_code
         * get procedure source by procedure name
         * @param proc_name procedure name
         * @return procedure source
         */
        const ObString * get_source_code(const ObString &proc_name);
        /**
         * @brief get_hkey
         * get hkey
         * @param proc_name procedure name
         * @return hkey
         */
        int64_t get_hkey(const ObString &proc_name) const;
        /**
         * @brief is_created
         * check whether name_code_map_ already create
         * @return bool value
         */
        bool is_created();
        /**
         * @brief exist
         * check whether procedure exist
         * @param proc_name procedure name
         * @return bool value
         */
        bool exist(const ObString &proc_name) const;
        /**
         * @brief exist
         * check whether procedure exist
         * @param proc_name procedure name
         * @param hash_code hasn code
         * @return  bool value
         */
        bool exist(const ObString &proc_name, int64_t hash_code) const;
        /**
         * @brief size
         * get name code map size
         * @return map size
         */
        int64_t size() const;
        /**
         * @brief get_state
         * get state flag
         * @return bool value
         */
        int get_state(){
            return is_ready_;
        }
        /**
         * @brief set_state
         * set state flag
         * @param sta bool value
         */
        void set_state(bool sta){
            is_ready_ = sta;
        }
        /**
         * @brief reset
         * reset name_code_map_
         */
        int reset(){
          local_version = 0;
          common::hash::ObHashMap<common::ObString, common::ObString >::const_iterator iter = name_code_map_.begin();
          for(;iter != name_code_map_.end(); iter++)
          {
              ObString name = iter->first;
              TBSYS_LOG(WARN, "reseting name code map iter->first=%s", name.ptr());
              name_code_map_.erase(name);
          }
          common::hash::ObHashMap<ObString, int64_t >::const_iterator iter1 = name_hash_map_.begin();
          for(; iter1 != name_hash_map_.end(); iter1++)
          {
              ObString name = iter1->first;
              TBSYS_LOG(WARN, "reseting name hash map iter->first=%s", name.ptr());
              name_hash_map_.erase(name);
          }
          return OB_SUCCESS;
        }
        /**
         * @brief get_local_version
         * @return local_version
         */
        int64_t get_local_version(){
            return local_version;
        }
        /**
         * @brief set_local_version
         * @param version
         */
        void set_local_version(int version){
            local_version = version;
        }
        /**
         * @brief get_name_code_map
         * @return
         */
        hash::ObHashMap<ObString, ObString> * get_name_code_map()
        {
            return &name_code_map_;
        }

    private:
      /**
       * @brief operator =
       * = operator overload disable
       * @return ObNameCodeMap object
       */
      ObNameCodeMap & operator = (const ObNameCodeMap &);
      hash::ObHashMap<ObString, ObString> name_code_map_;  ///<  name code map
      ObProcHashCache name_hash_map_;  ///< name hash map
      common::ObStringBuf arena_;  ///<  buffer allotter
      bool is_ready_;  ///<  ready flag
      int64_t local_version; ///<  local name code version
    };
  }
}
#endif  //OB_COMMON_NAME_CODE_MAP_H_

