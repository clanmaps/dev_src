/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_bloomfilter_join.h
* @brief for operations of bloomfilter join
*
* Created by maoxiaoxiao:do bloomfilter join
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_07_27
*/

#ifndef OB_BLOOMFILTER_JOIN_H
#define OB_BLOOMFILTER_JOIN_H

#include "ob_join.h"
#include "common/bloom_filter.h"
#include "common/ob_row_store.h"
#include "ob_phy_operator.h"
#include "ob_table_rpc_scan.h"
#include "ob_table_mem_scan.h"
#include "common/ob_row_util.h"

namespace oceanbase
{
  namespace sql
  {
    // 要求对输入right_child在等值join列上使用bloomfilter过滤数据
    // 支持inner join, left join，不支持outer join, right join
    /**
     * @brief The ObBloomFilterJoin class
     * ObBloomFilterJoin is designed for
     * doing bloomfilter join
     */
    class ObBloomFilterJoin: public ObJoin
    {
    public:
      /**
       * @brief constructor
       */
      ObBloomFilterJoin();

      /**
       * @brief destructor
       */
      virtual ~ObBloomFilterJoin();

      /**
       * @brief reset
       */
      virtual void reset();

      /**
       * @brief reuse
       */
      virtual void reuse();

      /**
       * @brief open
       * @return OB_SUCCESS or other ERROR
       */
      virtual int open();

      /**
       * @brief close
       * @return OB_SUCCESS or other ERROR
       */
      virtual int close();

      /**
       * @brief get_type
       * @return PHY_BLOOMFILTER_JOIN
       */
      virtual ObPhyOperatorType get_type() const { return PHY_BLOOMFILTER_JOIN; }

      /**
       * @brief set_join_type
       * @param join_type
       * @return OB_SUCCESS or other ERROR
       */
      virtual int set_join_type(const ObJoin::JoinType join_type);

      /**
       * @brief get_next_row
       * @param row
       * @return OB_SUCCESS or other ERROR
       */
      virtual int get_next_row(const common::ObRow *&row);

      /**
       * @brief get_row_desc
       * @param row_desc
       * @return OB_SUCCESS or other ERROR
       */
      virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;

      /**
       * @brief to_string
       * @param buf
       * @param buf_len
       * @return pos
       */
      virtual int64_t to_string(char* buf, const int64_t buf_len) const;

      DECLARE_PHY_OPERATOR_ASSIGN;
    private:
      //int normal_get_next_row(const common::ObRow *&row);

      /**
       * @brief inner_get_next_row
       * get next row for inner join
       * @param row
       * @return OB_SUCCESS or other ERROR
       */
      int inner_get_next_row(const common::ObRow *&row);

      /**
       * @brief left_outer_get_next_row
       * get next row for left outer join
       * @param row
       * @return OB_SUCCESS or other ERROR
       */
      int left_outer_get_next_row(const common::ObRow *&row);

      //int right_outer_get_next_row(const common::ObRow *&row);
      //int full_outer_get_next_row(const common::ObRow *&row);
      //int left_semi_get_next_row(const common::ObRow *&row);
      //int right_semi_get_next_row(const common::ObRow *&row);
      //int left_anti_semi_get_next_row(const common::ObRow *&row);
      //int right_anti_semi_get_next_row(const common::ObRow *&row);

      /**
       * @brief compare_equijoin_cond
       * @param r1
       * @param r2
       * @param cmp
       * @return OB_SUCCESS or other ERROR
       */
      int compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;

      /**
       * @brief left_row_compare_equijoin_cond
       * @param r1
       * @param r2
       * @param cmp
       * @return OB_SUCCESS or other ERROR
       */
      int left_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;

      /**
       * @brief curr_row_is_qualified
       * @param is_qualified
       * @return OB_SUCCESS or other ERROR
       */
      int curr_row_is_qualified(bool &is_qualified);

      /**
       * @brief cons_row_desc
       * @param rd1
       * @param rd2
       * @return OB_SUCCESS or other ERROR
       */
      int cons_row_desc(const ObRowDesc &rd1, const ObRowDesc &rd2);

      /**
       * @brief join_rows
       * @param r1
       * @param r2
       * @return OB_SUCCESS or other ERROR
       */
      int join_rows(const ObRow& r1, const ObRow& r2);

      /**
       * @brief left_join_rows
       * @param r1
       * @return OB_SUCCESS or other ERROR
       */
      int left_join_rows(const ObRow& r1);

      /**
       * @brief right_join_rows
       * @param r2
       * @return OB_SUCCESS or other ERROR
       */
      int right_join_rows(const ObRow& r2);

      // disallow copy
      ObBloomFilterJoin(const ObBloomFilterJoin &other);
      ObBloomFilterJoin& operator=(const ObBloomFilterJoin &other);

    private:
      static const int64_t MAX_SINGLE_ROW_SIZE = common::OB_ROW_MAX_COLUMNS_COUNT*(common::OB_MAX_VARCHAR_LENGTH+4);
      // data members
      typedef int (ObBloomFilterJoin::*get_next_row_func_type)(const common::ObRow *&row);
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

      common::ObRow left_row_;
      common::ObRowStore left_row_store_;
      common::ObBloomFilterV1 *bloom_filter_;   ///<bloom filter of left table for join
      bool use_bloom_filter_;   ///<true if need to do bloomfilter join
      ObSqlExpression *table_filter_expr_;   ///<filter for bloomfilter join
      /*add maoxx [bloomfilter_join] 20160722*/
      static const int64_t BASIC_SYMBOL_COUNT = 64;
      /*add e*/
      static const int64_t BLOOMFILTER_ELEMENT_NUM = 1000000;
    };
  } // end namespace sql
} // end namespace oceanbase

#endif // OB_BLOOMFILTER_JOIN_H
