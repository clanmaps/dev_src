/**
 * Copyright (C) 2013-2016 DaSE .
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file     ob_merge_join.h
 * @brief    merge_join operator
 * modified by Qiushi FAN: insert some function and member variable of class ObMergeJoin, in order to complete the semijoin feature.
 * @version  CEDAR 0.2 
 * @author   Qiushi FAN <qsfan@ecnu.cn>
 * @date     2015_12_30
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
 * ob_merge_join.h
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *
 */
#ifndef _OB_MERGE_JOIN_H
#define _OB_MERGE_JOIN_H 1

#include "ob_join.h"
#include "common/ob_row.h"
#include "common/ob_array.h"
#include "common/ob_row_store.h"


namespace oceanbase
{
  namespace sql
  {
    // 要求两个输入left_child和right_child在等值join列上排好序
    // 支持所有join类型
    class ObMergeJoin: public ObJoin
    {
      public:
        ObMergeJoin();
        virtual ~ObMergeJoin();
        virtual void reset();
        virtual void reuse();
        virtual int open();
        virtual int close();
        virtual ObPhyOperatorType get_type() const { return PHY_MERGE_JOIN; }
        virtual int set_join_type(const ObJoin::JoinType join_type);
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        //add fanqiushi [semi_join] [0.1] 20150826:b
        /**
        * @brief set is this operator doing semijoin.
        * @param is_semijoin,tid and cid of join column.
        * @param void.
        * @return void.
        */
        void set_is_semi_join(bool is_semi_join,uint64_t tid,uint64_t cid);
        //add:e
        
        DECLARE_PHY_OPERATOR_ASSIGN;
      private:
        int normal_get_next_row(const common::ObRow *&row);
        int inner_get_next_row(const common::ObRow *&row);
        int left_outer_get_next_row(const common::ObRow *&row);
        int right_outer_get_next_row(const common::ObRow *&row);
        int full_outer_get_next_row(const common::ObRow *&row);
        int left_semi_get_next_row(const common::ObRow *&row);
        int right_semi_get_next_row(const common::ObRow *&row);
        int left_anti_semi_get_next_row(const common::ObRow *&row);
        int right_anti_semi_get_next_row(const common::ObRow *&row);
        int compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;
        int left_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;
        int curr_row_is_qualified(bool &is_qualified);
        int cons_row_desc(const ObRowDesc &rd1, const ObRowDesc &rd2);
        int join_rows(const ObRow& r1, const ObRow& r2);
        int left_join_rows(const ObRow& r1);
        int right_join_rows(const ObRow& r2);
        //add fanqiushi [semi_join] [0.1] 20150826:b
        /**
        * @brief change this operator's right child.
        * @param void.
        * @param void.
        * @return Error code.
        */
        int change_right_semi_join_op();
        /**
        * @brief a new open function to do semijoin.
        * @param void.
        * @param void.
        * @return Error code.
        */
        int do_semi_open();
        //add:e
        // disallow copy
        ObMergeJoin(const ObMergeJoin &other);
        ObMergeJoin& operator=(const ObMergeJoin &other);
      private:
        static const int64_t MAX_SINGLE_ROW_SIZE = common::OB_ROW_MAX_COLUMNS_COUNT*(common::OB_MAX_VARCHAR_LENGTH+4);
        // data members
        typedef int (ObMergeJoin::*get_next_row_func_type)(const common::ObRow *&row);
        get_next_row_func_type get_next_row_func_;
        const common::ObRow *last_left_row_;
        const common::ObRow *last_right_row_;
        common::ObRow last_join_left_row_;
        common::ObString last_join_left_row_store_;
        common::ObRowStore right_cache_;
        common::ObRow curr_cached_right_row_;
        common::ObRow curr_row_;
        common::ObRowDesc row_desc_;
        bool right_cache_is_valid_;
        bool is_right_iter_end_;
        //add fanqiushi [semi_join] [0.1] 20150826:b
        bool is_semi_join_;   ///< identifier
        uint64_t right_table_id_;  ///< tid of join column
        uint64_t right_cid_;  ///< cid of join column
        common::ObArray<common::ObObj> filter_set_;  ///< an array of distinct values of left table
        //add:e
    };
    //add fanqiushi [semi_join] [0.1] 20150826:b
    inline void ObMergeJoin::set_is_semi_join(bool is_semi_join,uint64_t tid,uint64_t cid)
    {
      is_semi_join_=is_semi_join;
      right_table_id_=tid;
      right_cid_=cid;
    }
    //add:e
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_MERGE_JOIN_H */
