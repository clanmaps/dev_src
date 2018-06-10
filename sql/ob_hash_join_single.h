/**
 * ob_hash_join_single.h
 *
 * Authors:
 *   maoxx
 */

#ifndef OB_HASH_JOIN_SINGLE_H
#define OB_HASH_JOIN_SINGLE_H

#include "ob_join.h"
#include "sql/ob_phy_operator.h"
#include "common/ob_row.h"
#include "common/ob_row_store.h"
#include "common/hash/ob_hashmap.h"
#include "common/ob_rowkey.h"
#include "common/ob_array.h"
#include "common/ob_custom_allocator.h"
#include "ob_filter.h"


namespace oceanbase
{
  namespace sql
  {
    class ObHashJoinSingle: public ObJoin
    {
        //add maoxx [hash join single] 20170614
        typedef std::pair<const ObRowStore::StoredRow*, int8_t> HashTableRowPair;
        typedef common::hash::ObHashTableNode<common::hash::HashMapTypes<uint64_t, HashTableRowPair*>::pair_type> hashnode;
        typedef common::hash::ObHashMap<uint64_t, HashTableRowPair*> HashTableRowMap;
        //add e
      public:
        ObHashJoinSingle();
        ~ObHashJoinSingle();
        virtual int open();
        virtual int close();
        virtual void reset();
        virtual void reuse();
        virtual int set_join_type(const ObJoin::JoinType join_type);
        virtual ObPhyOperatorType get_type() const{return PHY_HASH_JOIN_SINGLE;}
        DECLARE_PHY_OPERATOR_ASSIGN;
        /**
         * 获得下一行的引用
         * @note 在下次调用get_next或者close前，返回的row有效
         * @pre 调用open()
         * @param row [out]
         *
         * @return OB_SUCCESS或OB_ITER_END或错误码
         */
        int get_next_row(const common::ObRow *&row);
        int get_row_desc(const common::ObRowDesc *&row_desc) const;
        int64_t to_string(char* buf, const int64_t buf_len) const;
      private:
        int inner_hash_get_next_row(const common::ObRow *&row);
        int left_hash_outer_get_next_row(const common::ObRow *&row);
        int right_hash_outer_get_next_row(const common::ObRow *&row);
        int full_hash_outer_get_next_row(const common::ObRow *&row);
        int normal_get_next_row(const common::ObRow *&row);
//        int inner_get_next_row(const common::ObRow *&row);
//        int left_outer_get_next_row(const common::ObRow *&row);
//        int right_outer_get_next_row(const common::ObRow *&row);
//        int full_outer_get_next_row(const common::ObRow *&row);
        int compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;
        int left_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;
        int right_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const;
//        int compare_hash_equijoin(const ObRow *&r1, const ObRow& r2, int &cmp,bool left_hash_id_cache_valid,int &last_left_hash_id_);
        //modify maoxx [hash join single] 20170614
//        int get_next_equijoin_left_row(const ObRow *&r1, const ObRow& r2);
        int get_next_equijoin_left_row(const ObRow *&r1, const ObRow& r2, uint64_t& bucket_hash_key, HashTableRowPair*& pair);
        //modify e
        int get_next_leftouterjoin_left_row(const common::ObRow *&row);
        int curr_row_is_qualified(bool &is_qualified);
        int cons_row_desc(const ObRowDesc &rd1, const ObRowDesc &rd2);
        int join_rows(const ObRow& r1, const ObRow& r2);
        int left_join_rows(const ObRow& r1);
        int right_join_rows(const ObRow& r2);
//        int left_rows(const common::ObRow *&row, int &left_hash_id_for_left_outer_join_);
        DISALLOW_COPY_AND_ASSIGN(ObHashJoinSingle);

      private:
        static const int64_t MAX_SINGLE_ROW_SIZE = common::OB_ROW_MAX_COLUMNS_COUNT*(common::OB_MAX_VARCHAR_LENGTH+4);
        typedef int (ObHashJoinSingle::*get_next_row_func_type)(const common::ObRow *&row);
        get_next_row_func_type get_next_row_func_;
        const common::ObRow *last_left_row_;
        const common::ObRow *last_right_row_;
        ObSqlExpression *table_filter_expr_;
        common::ObRow curr_cached_left_row_;
        common::ObRowDesc row_desc_;
        bool left_hash_key_cache_valid_;
        uint64_t last_left_hash_key_;
        bool use_bloomfilter_;
        int64_t left_bucket_pos_for_left_outer_join_;
        bool is_left_iter_end_;
//        int process_sub_query();
//        common::ObRow curr_cached_right_row_;
//        bool last_left_row_has_printed_;
//        bool last_right_row_has_printed_;
//        bool left_cache_is_valid_;
//        bool right_cache_is_valid_;
//        common::ObRowStore left_cache_;
//        common::ObRowStore left_cache_for_right_join_;
//        common::ObRowStore right_cache_;
//        common::ObRow last_join_left_row_;
//        common::ObString last_join_left_row_store_;
//        common::ObRow last_join_right_row_;
//        common::ObString last_join_right_row_store_;
//        bool is_right_iter_end_;
//        ObFilter sub_query_filter_ ;

        //add tianz [SubQuery_for_Instmt] [JHOBv0.1] 20140610:b
        static const int MAX_SUB_QUERY_NUM = 5;
        static const int64_t HASH_BUCKET_NUM = 100000;
        static const int BIG_RESULTSET_THRESHOLD = 50;
        int hashmap_num_ ;
        common::CustomAllocator arena_;
        common::ObArray<common::ObRowkey>  sub_result_;
        common::hash::ObHashMap<common::ObRowkey,common::ObRowkey,common::hash::NoPthreadDefendMode> sub_query_map_[MAX_SUB_QUERY_NUM];
        //add 20140610:e
        common::ObArray<ObObjType> sub_query_map_and_bloomfilter_column_type[MAX_SUB_QUERY_NUM];  //add peiouya [IN_TYPEBUG_FIX] 20151225
        bool is_subquery_result_contain_null[MAX_SUB_QUERY_NUM];  //add peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518

        static const int64_t BLOOMFILTER_ELEMENT_NUM = 1000000;
        common::ObRow curr_row_;

        //modify maoxx [hash join single] 20170614
        /*struct hash_row_store
        {
          common::ObRowStore row_store;
          int32_t id_no_;
          ObArray<int8_t> hash_iterators;
          hash_row_store():id_no_(0) {}
        }hash_table_row_store[HASH_BUCKET_NUM];*/
        hashnode* bucket_node_;
        common::ObRowStore row_store_;
        HashTableRowMap hash_table_;
        //modify e

        //add by steven.h.d  2015.6.4
//        int64_t con_length_;
//        int numi;
        int64_t left_row_count_;
        bool flag_; //add maoxx [hash join single bug fix] 20170908
    };
  } // end namespace sql
} // end namespace oceanbase

#endif // OB_HASH_JOIN_SINGLE_H
