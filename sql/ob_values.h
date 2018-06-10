/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_values.h
 * @brief the value physical plan operator
 *
 * mofied by zhutao:add some content for procedure in ObValues class
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 *
 * @date 2016_07_30
 */

/**
 * (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * Version: $Id$
 *
 * ob_values.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */

#ifndef _OB_VALUES_H
#define _OB_VALUES_H 1

#include "sql/ob_single_child_phy_operator.h"
#include "common/ob_row_store.h"
//add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
#include "common/ob_se_array.h"
#include "common/ob_define.h"
//add e
namespace oceanbase
{
  namespace sql
  {
  //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
  class ObValuesKeyInfo{

  public:

      ObValuesKeyInfo();
      ~ObValuesKeyInfo();
      void set_key_info(uint64_t tid,uint64_t cid,uint32_t p,uint32_t s,common::ObObjType type);
      void get_type(common::ObObjType& type);
      void get_key_info(uint32_t& p,uint32_t& s);

      bool is_rowkey(uint64_t tid,uint64_t cid);
  private:
      uint32_t precision_;
      uint32_t scale_;
      uint64_t cid_;
      uint64_t tid_;
      common::ObObjType type_;
  };
  inline void ObValuesKeyInfo::set_key_info(uint64_t tid,uint64_t cid,uint32_t p,uint32_t s,common::ObObjType type){
      tid_=tid;
      cid_=cid;
      type_=type;
      precision_=p;
      scale_=s;
  }
  inline void ObValuesKeyInfo::get_type(common::ObObjType& type){
      type=type_;
  }
  inline void ObValuesKeyInfo::get_key_info(uint32_t& p,uint32_t& s){
      p=precision_;
      s=scale_;
  }
  inline bool ObValuesKeyInfo::is_rowkey(uint64_t tid,uint64_t cid){
      bool ret =false;
      if(tid==tid_&&cid==cid_)ret=true;
      return ret;
  }
  //add e
    class ObValues: public ObSingleChildPhyOperator
    {
      public:
        ObValues();
        virtual ~ObValues();
        virtual void reset();
        virtual void reuse();
        int set_row_desc(const common::ObRowDesc &row_desc);
        int add_values(const common::ObRow &value);
        const common::ObRowStore &get_row_store() {return row_store_;};

        virtual int open();
        virtual int close();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        /**
         * @brief get_row_desc_template
         * get next row descriptor
         * @param row_desc row descriptor
         * @return error code
         */
        int get_row_desc_template(const common::ObRowDesc *&row_desc) const; //add by zt 20160114
        /**
         * @brief get_static_data_id
         * get static data id
         * @return static data id
         */
        int64_t get_static_data_id() const { return static_data_id_; }  //add by zt 2016018
        /**
         * @brief set_static_data_id
         * set static data id
         * @param static_data_id static data id
         */
        void set_static_data_id(int64_t static_data_id) { static_data_id_ = static_data_id; } //add by zt 20160118
        //        bool is_opened() const { return is_open_; } // add zt 20151203
        enum ObPhyOperatorType get_type() const{return PHY_VALUES;}
        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        void set_fix_obvalues();
        void add_rowkey_array(uint64_t tid,uint64_t cid,common::ObObjType type,uint32_t p,uint32_t s);
        bool is_rowkey_column(uint64_t tid,uint64_t cid);
        int get_rowkey_schema(uint64_t tid,uint64_t cid,common::ObObjType& type,uint32_t& p,uint32_t& s);
        typedef common::ObSEArray<ObValuesKeyInfo, common::OB_MAX_ROWKEY_COLUMN_NUMBER> RowkeyInfo;
        //add e
      private:
        // types and constants
        int load_data();
      private:
        // disallow copy
        ObValues(const ObValues &other);
        ObValues& operator=(const ObValues &other);
        // function members
      private:
        // data members
        common::ObRowDesc row_desc_;
        common::ObRow curr_row_;
        common::ObRowStore row_store_;
        //add by zt 20160118:b
        bool is_open_;  ///<  is open
        int64_t static_data_id_;  ///<  static data id
        //add by zt 20160118:e
        //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        bool is_need_fix_obvalues;
        RowkeyInfo obj_array_;
        //add e
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_VALUES_H */
