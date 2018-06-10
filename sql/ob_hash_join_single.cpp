/**
 * ob_hash_join_single.cpp
 *
 * Authors:
 *   maoxx
 */

#include "ob_hash_join_single.h"
#include "ob_table_rpc_scan.h"
#include "ob_table_mem_scan.h"
#include "ob_postfix_expression.h"
#include "common/utility.h"
#include "common/ob_row_util.h"
#include "sql/ob_physical_plan.h"

using namespace oceanbase::sql;
using namespace oceanbase::common;

ObHashJoinSingle::ObHashJoinSingle()
    :get_next_row_func_(NULL),
     last_left_row_(NULL),
     last_right_row_(NULL),
     table_filter_expr_(NULL)
     ,hashmap_num_(0)
//       right_cache_is_valid_(false),
//       is_right_iter_end_(false)
{
  arena_.set_mod_id(ObModIds::OB_MS_SUB_QUERY);
  left_hash_key_cache_valid_ = false;
  left_bucket_pos_for_left_outer_join_ = -1;
  //mod peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518:b
  for(int i=0;i<MAX_SUB_QUERY_NUM;i++)
  {
    is_subquery_result_contain_null[i] = false;
  }
  //mod peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518:e
  //add maoxx [hash join single] 20170614
  bucket_node_ = NULL;
  //add e
  left_row_count_ = 0;
  //add maoxx [hash join single bug fix] 20170908
  flag_ = false;
  //add e
//  last_left_row_has_printed_ = false;
//  last_right_row_has_printed_ = false;
//  left_cache_is_valid_ = false;
//  is_left_iter_end_ = false;
//  cur_hashmap_num_ = 0;
}

 ObHashJoinSingle::~ObHashJoinSingle()
{
//  char *store_buf = last_join_left_row_store_.ptr();
//  if (NULL != store_buf)
//  {
//    ob_free(store_buf);
//    last_join_left_row_store_.assign_ptr(NULL, 0);
//  }
//  char *right_store_buf = last_join_right_row_store_.ptr();
//  if (NULL != right_store_buf)
//  {
//    ob_free(right_store_buf);
//    last_join_right_row_store_.assign_ptr(NULL, 0);
//  }
   sub_result_.~ObArray();
   for(int i = 0; i < MAX_SUB_QUERY_NUM; i++)
   {
     sub_query_map_[i].destroy();
   }
   arena_.free();
   //add maoxx [hash join single] 20170614
   row_store_.clear();
   for (HashTableRowMap::iterator iter = hash_table_.begin(); iter != hash_table_.end(); iter++)
   {
     delete iter->second;
     iter->second = NULL;
   }
   hash_table_.destroy();
   //add e
   //add 20140610:e
//   if(table_filter_expr_ != NULL)
//   {
//     ObSqlExpression::free (table_filter_expr_);
//     table_filter_expr_ = NULL;
//   }
}

void ObHashJoinSingle::reset()
{
  get_next_row_func_ = NULL;
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  row_desc_.reset();
  left_bucket_pos_for_left_outer_join_ = -1;
  left_hash_key_cache_valid_ = false;
  equal_join_conds_.clear();
  other_join_conds_.clear();
  left_op_ = NULL;
  right_op_ = NULL;
  for(int i = 0; i < MAX_SUB_QUERY_NUM; i++)
  {
    sub_query_map_[i].clear();
  }
  hashmap_num_ = 0;
  //add peiouya [IN_TYPEBUG_FIX] 20151225
  for(int i=0;i<MAX_SUB_QUERY_NUM;i++)
  {
    sub_query_map_and_bloomfilter_column_type[i].clear ();
    is_subquery_result_contain_null[i] = false;  //add peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518
  }
  //add 20151225:e
  //modify maoxx [hash join single] 20170614
  /*for(int i = 0; i < HASH_BUCKET_NUM; i++)
  {
    hash_table_row_store[i].row_store.clear();
  }*/
  bucket_node_ = NULL;
  hash_table_.clear();
  row_store_.clear();
  //modify e
  left_row_count_ = 0;
//  char *store_buf = last_join_left_row_store_.ptr();
//  if (NULL != store_buf)
//  {
//      ob_free(store_buf);
//      last_join_left_row_store_.assign_ptr(NULL, 0);
//  }
//  char *right_store_buf = last_join_right_row_store_.ptr();
//  if (NULL != right_store_buf)
//  {
//    ob_free(right_store_buf);
//    last_join_right_row_store_.assign_ptr(NULL, 0);
//  }
//  last_left_row_has_printed_ = false;
//  last_right_row_has_printed_ = false;
//  left_cache_is_valid_ = false;
//  left_cache_.clear();
//  left_cache_for_right_join_.clear();
//  is_left_iter_end_ = false;
//  right_cache_.clear();
//  right_cache_is_valid_ = false;
//  is_right_iter_end_ = false;
//  arena_.free();
//  if(table_filter_expr_ != NULL)
//  {
//    ObSqlExpression::free (table_filter_expr_);
//    table_filter_expr_ = NULL;
//  }
}

void ObHashJoinSingle::reuse()
{
  get_next_row_func_ = NULL;
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  row_desc_.reset();
  left_bucket_pos_for_left_outer_join_ = -1;
  left_hash_key_cache_valid_ = false;
  equal_join_conds_.clear();
  other_join_conds_.clear();
  left_op_ = NULL;
  right_op_ = NULL;
  for(int i = 0; i < MAX_SUB_QUERY_NUM; i++)
  {
    sub_query_map_[i].clear();
  }
  hashmap_num_ = 0;
  //add peiouya [IN_TYPEBUG_FIX] 20151225
  for(int i=0;i<MAX_SUB_QUERY_NUM;i++)
  {
    sub_query_map_and_bloomfilter_column_type[i].clear ();
    is_subquery_result_contain_null[i] = false;  //add peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518
  }
  //add 20151225:e
  //modify maoxx [hash join single] 20170614
  /*for(int i = 0; i < HASH_BUCKET_NUM; i++)
  {
    hash_table_row_store[i].row_store.reuse();
  }*/
  bucket_node_ = NULL;
  hash_table_.clear();
  row_store_.clear();
  //modify e
  left_row_count_ = 0;
//  char *store_buf = last_join_left_row_store_.ptr();
//  if (NULL != store_buf)
//  {
//    ob_free(store_buf);
//    last_join_left_row_store_.assign_ptr(NULL, 0);
//  }
//  char *right_store_buf = last_join_right_row_store_.ptr();
//  if (NULL != right_store_buf)
//  {
//     ob_free(right_store_buf);
//     last_join_right_row_store_.assign_ptr(NULL, 0);
//  }
//  last_left_row_has_printed_ = false;
//  last_right_row_has_printed_ = false;
//  left_cache_is_valid_ = false;
//  left_cache_.reuse();
//  left_cache_for_right_join_.reuse();
//  is_left_iter_end_ = false;
//  right_cache_.reuse();
//  right_cache_is_valid_ = false;
//  is_right_iter_end_ = false;
//  sub_query_filter_.reuse();
//  cur_hashmap_num_ = 0;
//  arena_.free();
//  if(table_filter_expr_ != NULL)
//  {
//    ObSqlExpression::free (table_filter_expr_);
//    table_filter_expr_ = NULL;
//  }
}

/*
 *设置join类型
 * Bloomfilter join 仅处理inner join与left join
 */
int ObHashJoinSingle::set_join_type(const ObJoin::JoinType join_type)
{
  int ret = OB_SUCCESS;
  ObJoin::set_join_type(join_type);
  switch(join_type)
  {
    case INNER_JOIN:
      //hash part2
      get_next_row_func_ = &ObHashJoinSingle::inner_hash_get_next_row;
      //hash part1
//      get_next_row_func_ = &ObHashJoinSingle::inner_get_next_row;
      use_bloomfilter_ = true;
      break;
    case LEFT_OUTER_JOIN:
      //hash part2
      get_next_row_func_ = &ObHashJoinSingle::left_hash_outer_get_next_row;
      //hash part1
//      get_next_row_func_ = &ObHashJoinSingle::left_outer_get_next_row;
      use_bloomfilter_ = true;
      break;
    case RIGHT_OUTER_JOIN:
      get_next_row_func_ = &ObHashJoinSingle::right_hash_outer_get_next_row;
      use_bloomfilter_ = false;
      break;
    case FULL_OUTER_JOIN:
      get_next_row_func_ = &ObHashJoinSingle::full_hash_outer_get_next_row;
      use_bloomfilter_ = false;
      break;
    default:
      ret = OB_ERR_UNEXPECTED;
      break;
  }
  return ret;
}

/*
 *迭代get_next_row
 *根据join类型 选取相应的get_next_row的方法
 */

int ObHashJoinSingle::get_next_row(const ObRow *&row)
{
  OB_ASSERT(get_next_row_func_);
  return (this->*(this->ObHashJoinSingle::get_next_row_func_))(row);
}

/*
 *获取行描述
 *
 */

int ObHashJoinSingle::get_row_desc(const common::ObRowDesc *&row_desc) const
{
  int ret = OB_SUCCESS;
  if (OB_UNLIKELY(0 >= row_desc_.get_column_num()))
  {
    TBSYS_LOG(ERROR, "not init");
    ret = OB_NOT_INIT;
  }
  else
  {
    row_desc = &row_desc_;
  }
  return ret;
}

int64_t ObHashJoinSingle::to_string(char* buf, const int64_t buf_len) const
{
  int64_t pos = 0;
  databuff_printf(buf, buf_len, pos, "HashJoinSingle ");
  pos += ObJoin::to_string(buf + pos, buf_len - pos);
  return pos;
}

int ObHashJoinSingle::close()
{
  int ret = OB_SUCCESS;
  last_left_row_ = NULL;
  last_right_row_ = NULL;
  row_desc_.reset();
//  char *store_buf = last_join_left_row_store_.ptr();
//  if (NULL != store_buf)
//  {
//    ob_free(store_buf);
//    last_join_left_row_store_.assign_ptr(NULL, 0);
//  }
//  char *right_store_buf = last_join_right_row_store_.ptr();
//  if (NULL != right_store_buf)
//  {
//    ob_free(right_store_buf);
//    last_join_right_row_store_.assign_ptr(NULL, 0);
//  }
//  right_cache_is_valid_ = false;
//  is_right_iter_end_ = false;
//  last_left_row_has_printed_ = false;
//  last_right_row_has_printed_ = false;
//  left_cache_is_valid_ = false;
//  is_left_iter_end_ = false;
//  if(table_filter_expr_ != NULL)
//  {
//    ObSqlExpression::free (table_filter_expr_);
//    table_filter_expr_ = NULL;
//  }
  ret = ObJoin::close();
  return ret;
}

/*
 * 1、打开前表运算符
 * 2、迭代前表每一行数据，生成Bloomfilter 并维护前表数据
 * 3、构建好的Bloomfilter 作为表达式加入后表filter
 * 4、初始化值
 */

int ObHashJoinSingle::open()
{
  int ret = OB_SUCCESS;
  const ObRowDesc *left_row_desc = NULL;
  const ObRowDesc *right_row_desc = NULL;
//  char *store_buf = NULL;
//  char *right_store_buf = NULL;
  int64_t equal_join_conds_count = equal_join_conds_.count();
  if (equal_join_conds_count <= 0)
  {
    TBSYS_LOG(WARN, "hash join can not work without equijoin conditions");
    ret = OB_NOT_SUPPORTED;
    return ret;
  }
  else if (OB_SUCCESS != (ret = left_op_->open()))
  {
    TBSYS_LOG(WARN, "failed to open child(ren) operator(s), err=%d", ret);
  }
  else if (OB_SUCCESS != (ret = left_op_->get_row_desc(left_row_desc)))
  {
    TBSYS_LOG(WARN, "failed to get child row desc, err=%d", ret);
  }
  if(OB_SUCCESS == ret)
  {
    //delete maoxx [hash join single] 20170614
    /*table_filter_expr_ = ObSqlExpression::alloc();
    if (NULL == table_filter_expr_)
    {
      TBSYS_LOG(WARN, "no memory");
      ret = OB_ALLOCATE_MEMORY_FAILED;
      return ret;
    }
//    hashmap_num_ = 0;
//    bloomfilter_map_[hashmap_num_].create(HASH_BUCKET_NUM);
    common::ObBloomFilterV1 *bloom_filter = NULL;
    table_filter_expr_->set_has_bloomfilter();
    table_filter_expr_->get_bloom_filter(bloom_filter);
    if(OB_SUCCESS != (ret = bloom_filter->init(BLOOMFILTER_ELEMENT_NUM)))
    {
      TBSYS_LOG(WARN, "Problem initialize bloom filter");
      return ret;
    }
    ExprItem dem1,dem2,dem3,dem4,dem5;
    dem1.type_ = T_REF_COLUMN;
    //后缀表达式组建
    for (int64_t i = 0; i < equal_join_conds_count; ++i)
    {
      const ObSqlExpression &expr = equal_join_conds_.at(i);
      ExprItem::SqlCellInfo c1;
      ExprItem::SqlCellInfo c2;
      if (expr.is_equijoin_cond(c1, c2))
      {
        dem1.value_.cell_.tid = c2.tid;
        dem1.value_.cell_.cid = c2.cid;
        //add dragon [invalid_argument] 2016-12-22
        ObTableRpcScan * right_rpc_scan = NULL;
        if(NULL == (right_rpc_scan = dynamic_cast<ObTableRpcScan *>(get_child(1)))){
          //Never Mind, it's OK
          TBSYS_LOG(WARN, "Now, we only support TableRpcScan operator!");
        }
        else
        {
          bool use_index_back = false;
          uint64_t index_tid = OB_INVALID_ID;
          right_rpc_scan->get_is_index_without_storing(use_index_back, index_tid);
          TBSYS_LOG(DEBUG, "%d, %lu", use_index_back, index_tid);
          if(use_index_back)
          {
            if(OB_INVALID_ID == index_tid)
            {
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN, "WTF, use index scan,but index tid is invalid");
            }
            else
              dem1.value_.cell_.tid = index_tid;
          }
          else
          {
            //do nothing
          }
          TBSYS_LOG(DEBUG, "dem1 has been changed from %ld to %ld", c2.tid, dem1.value_.cell_.tid);
        }

        //c1是左表, c2是右表
        TBSYS_LOG(DEBUG, "Dragon says: %ld/%ld c1 is left table while c2 is right table c1[%lu, %lu] c2[%lu, %lu]",
                  i, equal_join_conds_count, c1.tid, c1.cid, c2.tid, c2.cid);
        //add dragon 2016-12-22 e
        table_filter_expr_->add_expr_item(dem1);
      }
    }
    dem2.type_ = T_OP_ROW;
    dem2.data_type_ = ObMinType;
    dem2.value_.int_ = equal_join_conds_count;

    dem3.type_ = T_OP_LEFT_PARAM_END;
    dem3.data_type_ = ObMinType;
    dem3.value_.int_ = 2;

    dem4.type_ = T_REF_QUERY;
    dem4.data_type_ = ObMinType;
    dem4.value_.int_ = 1;

    dem5.type_ = T_OP_IN;
    dem5.data_type_ = ObMinType;
    dem5.value_.int_ = 2;
    table_filter_expr_->add_expr_item(dem2);
    table_filter_expr_->add_expr_item(dem3);
    table_filter_expr_->add_expr_item(dem4);
    table_filter_expr_->add_expr_item(dem5);
    table_filter_expr_->add_expr_item_end();*/
    //delete e

    const ObRow *row = NULL;
    ret = left_op_->get_next_row(row);
    //add maoxx [hash join single] 20170614
    hash_table_.create(HASH_BUCKET_NUM);
    //add e
    //迭代前表数据，构建Bloomfilter
    while (OB_SUCCESS == ret)
    {
      //TBSYS_LOG(ERROR, "load data from left table, row=%s", to_cstring(*row));
      left_row_count_++;
      //delete maoxx [hash join single] 20170614
//      ObObj value[equal_join_conds_count];
      //delete e
      ObRow *curr_row = const_cast<ObRow *>(row);
      uint64_t hash_key = 0;
      for (int64_t i = 0; i < equal_join_conds_count; ++i)
      {
        const ObSqlExpression &expr = equal_join_conds_.at(i);
        ExprItem::SqlCellInfo c1;
        ExprItem::SqlCellInfo c2;
        if (expr.is_equijoin_cond(c1, c2))
        {
          ObObj *temp = NULL;
          if (OB_SUCCESS != (ret = curr_row->get_cell(c1.tid,c1.cid,temp)))
          {
            TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
            break;
          }
          else
          {
            //delete maoxx [hash join single] 20170614
//            value[i] = *temp;
            //delete e
            hash_key = temp->murmurhash64A(hash_key);
          }
        }
      }
      //delete maoxx [hash join single] 20170614
      /*ObRowkey columns;
      columns.assign(value,equal_join_conds_count);
      ObRowkey columns2;
      if(OB_SUCCESS != (ret = columns.deep_copy(columns2,arena_)))
      {
        TBSYS_LOG(WARN, "fail to deep copy column");
        break;
      }
      bloom_filter->insert(columns2);*/
      //delete e
      //modify maoxx [hash join single] 20170614
      /*const ObRowStore::StoredRow *stored_row = NULL;
      bucket_num = bucket_num % HASH_BUCKET_NUM;
      //TBSYS_LOG(ERROR,"DHC bucket_num=%d",(int)bucket_num);
      //sort--part1 维护前表数据
//      left_cache_.add_row(*row, stored_row);
//      stored_row = NULL;
      //hash--part2 维护前表数据
      hash_table_row_store[(int)bucket_num].row_store.add_row(*row, stored_row);
      hash_table_row_store[(int)bucket_num].hash_iterators.push_back(0);*/
      const ObRowStore::StoredRow *stored_row = NULL;
      row_store_.add_row(*row, stored_row);
      HashTableRowPair* pair = new HashTableRowPair(stored_row, 0);
      if(common::hash::HASH_INSERT_SUCC != hash_table_.set_multiple(hash_key, pair))
      {
        TBSYS_LOG(WARN, "fail to insert pair into hash map");
        ret = OB_ERROR;
        break;
      }
      //modify e
      ret = left_op_->get_next_row(row);
    }
	//add wanglei [bloom filter fix] 20160524:b
    if(ret == OB_ITER_END)
      ret = OB_SUCCESS;
	//add wanglei [bloom filter fix] 20160524:e
    //delete maoxx [hash join single bug fix] 20170509
    /*if(use_bloomfilter_ && (left_row_count_ > 0))
    {
      //sort--part1 后表运算符
//      ObSingleChildPhyOperator *sort_query =  dynamic_cast<ObSingleChildPhyOperator *>(get_child(1));
//      ObTableRpcScan * main_query;
//      if(NULL==(main_query= dynamic_cast<ObTableRpcScan *>(sort_query->get_child(0)))){
//        ObTableMemScan * main_query_in;
//        main_query_in= dynamic_cast<ObTableMemScan *>(sort_query->get_child(0));
//        //main_query_in->set_expr(table_filter_expr_);
//      }
//      else
//      {
//        hashmap_num_ ++;
//        main_query->add_filter(table_filter_expr_);
//      }
      //hash--part2 后表运算符
      ObTableRpcScan * main_query = NULL;
//      ObTableMemScan * main_query_in;
      if(NULL != (main_query = dynamic_cast<ObTableRpcScan *>(get_child(1))))
      {
        TBSYS_LOG(ERROR,"DHC Rpc_Scan");
        if(OB_SUCCESS != (ret = main_query->add_filter(table_filter_expr_)))
        {
          ObSqlExpression::free(table_filter_expr_);
          TBSYS_LOG(WARN,"hash join add expression failed!");
        }
      }
//      else
//      {
//        if(NULL != (main_query_in = dynamic_cast<ObTableMemScan *>(get_child(1))))
//        {
//          TBSYS_LOG(ERROR,"DHC MemScan");
//          main_query_in->add_filter(table_filter_expr_);
//        }
//      }
    }*/
    /*int64_t max_size = 0; //测试hash是否分布均匀
    int64_t second_size = 0;
    for(int64_t i = 0; i < HASH_BUCKET_NUM; i++)
    {
        max_size = max_size < hash_table_row_store[(int)i].hash_iterators.count()? hash_table_row_store[(int)i].hash_iterators.count() : max_size;
        second_size = hash_table_row_store[(int)i].hash_iterators.count()< max_size ? (hash_table_row_store[(int)i].hash_iterators.count() > second_size ? hash_table_row_store[(int)i].hash_iterators.count() : second_size) : second_size;
    }
    TBSYS_LOG(ERROR,"max_size=%d second_size=%d",(int)max_size,(int)second_size);*/
    //delete e
  }
  if(OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = right_op_->open()))
    {
      TBSYS_LOG(WARN, "failed to open right_op_ operator(s), err=%d", ret);
    }
    else if (OB_SUCCESS != (ret = right_op_->get_row_desc(right_row_desc)))
    {
      TBSYS_LOG(WARN, "failed to get right_op_ row desc, err=%d", ret);
    }
    else if (OB_SUCCESS != (ret = cons_row_desc(*left_row_desc, *right_row_desc)))
    {
      TBSYS_LOG(WARN, "failed to cons row desc, err=%d", ret);
    }
//    else if (NULL == (store_buf = static_cast<char*>(ob_malloc(MAX_SINGLE_ROW_SIZE, ObModIds::OB_SQL_MERGE_JOIN))))
//    {
//      TBSYS_LOG(ERROR, "no memory");
//      ret = OB_ALLOCATE_MEMORY_FAILED;
//    }
//    else if (NULL == (right_store_buf = static_cast<char*>(ob_malloc(MAX_SINGLE_ROW_SIZE, ObModIds::OB_SQL_MERGE_JOIN))))
//    {
//      TBSYS_LOG(ERROR, "no memory");
//      ret = OB_ALLOCATE_MEMORY_FAILED;
//    }
    else
    {
      OB_ASSERT(left_row_desc);
      OB_ASSERT(right_row_desc);
      curr_row_.set_row_desc(row_desc_);
      last_left_row_ = NULL;
      last_right_row_ = NULL;
      curr_cached_left_row_.set_row_desc(*left_row_desc);
      left_hash_key_cache_valid_ = false;
      left_bucket_pos_for_left_outer_join_ = -1 ;
//      curr_cached_right_row_.set_row_desc(*right_row_desc);
//      right_cache_is_valid_ = false;
//      is_right_iter_end_ = false;
//      last_left_row_has_printed_ = false;
//      last_right_row_has_printed_ = false;
//      left_cache_is_valid_ = false;
//      is_left_iter_end_ = false;
//      cur_hashmap_num_ = 0;
//      last_join_right_row_store_.assign_buffer(right_store_buf,MAX_SINGLE_ROW_SIZE);
//      last_join_left_row_store_.assign_buffer(store_buf, MAX_SINGLE_ROW_SIZE);

      //add maoxx [hash join single bug fix] 20170509
      //delete maoxx [hash join single] 20170614
      /*if(use_bloomfilter_ && (left_row_count_ > 0))
      {
        //sort--part1 后表运算符
//        ObSingleChildPhyOperator *sort_query =  dynamic_cast<ObSingleChildPhyOperator *>(get_child(1));
//        ObTableRpcScan * main_query;
//        if(NULL==(main_query= dynamic_cast<ObTableRpcScan *>(sort_query->get_child(0)))){
//          ObTableMemScan * main_query_in;
//          main_query_in= dynamic_cast<ObTableMemScan *>(sort_query->get_child(0));
//          //main_query_in->set_expr(table_filter_expr_);
//        }
//        else
//        {
//          hashmap_num_ ++;
//          main_query->add_filter(table_filter_expr_);
//        }
        //hash--part2 后表运算符
        ObTableRpcScan * main_query = NULL;
//        ObTableMemScan * main_query_in;
        if(NULL != (main_query = dynamic_cast<ObTableRpcScan *>(get_child(1))))
        {
          TBSYS_LOG(ERROR,"DHC Rpc_Scan");
          if(OB_SUCCESS != (ret = main_query->add_filter(table_filter_expr_)))
          {
            ObSqlExpression::free(table_filter_expr_);
            TBSYS_LOG(WARN,"hash join add expression failed!");
          }
        }
//        else
//        {
//          if(NULL != (main_query_in = dynamic_cast<ObTableMemScan *>(get_child(1))))
//          {
//            TBSYS_LOG(ERROR,"DHC MemScan");
//            main_query_in->add_filter(table_filter_expr_);
//          }
//        }
      }*/
      //delete e
      /*int64_t max_size = 0; //测试hash是否分布均匀
        int64_t second_size = 0;
        for(int64_t i = 0; i < HASH_BUCKET_NUM; i++)
        {
            max_size = max_size < hash_table_row_store[(int)i].hash_iterators.count()? hash_table_row_store[(int)i].hash_iterators.count() : max_size;
            second_size = hash_table_row_store[(int)i].hash_iterators.count()< max_size ? (hash_table_row_store[(int)i].hash_iterators.count() > second_size ? hash_table_row_store[(int)i].hash_iterators.count() : second_size) : second_size;
        }
        TBSYS_LOG(ERROR,"max_size=%d second_size=%d",(int)max_size,(int)second_size);*/
      //add e
    }
  }
  return ret;
}


//判断等值连接条件是否满足
int ObHashJoinSingle::compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const
{
  int ret = OB_SUCCESS;
  cmp = 0;
  const ObObj *res1 = NULL;
  const ObObj *res2 = NULL;
  ObExprObj obj1;
  ObExprObj obj2;
  for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
  {
    const ObSqlExpression &expr = equal_join_conds_.at(i);
    ExprItem::SqlCellInfo c1;
    ExprItem::SqlCellInfo c2;
    if (expr.is_equijoin_cond(c1, c2))
    {
      if (OB_SUCCESS != (ret = r1.get_cell(c1.tid, c1.cid, res1)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, res2)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
        break;
      }
      else
      {
        obj1.assign(*res1);
        obj2.assign(*res2);
        if (OB_SUCCESS != obj1.compare(obj2, cmp))
        {
          if (obj1.is_null())
          {
            cmp = -10;
          }
          else
          {
            cmp = 10;
          }
          break;
        }
        else if (0 != cmp)
        {
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "invalid equijoin condition");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
  }
  return ret;
}

/*int ObHashJoinSingle::left_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const
{
  int ret = OB_SUCCESS;
  cmp = 0;
  const ObObj *res1 = NULL;
  const ObObj *res2 = NULL;
  ObExprObj obj1;
  ObExprObj obj2;
  for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
  {
    const ObSqlExpression &expr = equal_join_conds_.at(i);
    ExprItem::SqlCellInfo c1;
    ExprItem::SqlCellInfo c2;
    if (expr.is_equijoin_cond(c1, c2))
    {
      if (OB_SUCCESS != (ret = r1.get_cell(c1.tid, c1.cid, res1)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else if (OB_SUCCESS != (ret = r2.get_cell(c1.tid, c1.cid, res2)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else
      {
        obj1.assign(*res1);
        obj2.assign(*res2);
        if (OB_SUCCESS != obj1.compare(obj2, cmp))
        {
          cmp = -10;
          break;
        }
        else if (0 != cmp)
        {
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "invalid equijoin condition");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
  }
  return ret;
}

int ObHashJoinSingle::right_row_compare_equijoin_cond(const ObRow& r1, const ObRow& r2, int &cmp) const
{
  //TBSYS_LOG(ERROR, "dhc ObHashJoinSingle::right_row_compare_equijoin_cond()");
  int ret = OB_SUCCESS;
  cmp = 0;
  const ObObj *res1 = NULL;
  const ObObj *res2 = NULL;
  ObExprObj obj1;
  ObExprObj obj2;
  for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
  {
    const ObSqlExpression &expr = equal_join_conds_.at(i);
    ExprItem::SqlCellInfo c1;
    ExprItem::SqlCellInfo c2;
    if (expr.is_equijoin_cond(c1, c2))
    {
      if (OB_SUCCESS != (ret = r1.get_cell(c2.tid, c2.cid, res1)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, res2)))
      {
        TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
        break;
      }
      else
      {
        obj1.assign(*res1);
        obj2.assign(*res2);
        if (OB_SUCCESS != obj1.compare(obj2, cmp))
        {
          cmp = -10;
          break;
        }
        else if (0 != cmp)
        {
          break;
        }
      }
    }
    else
    {
      TBSYS_LOG(ERROR, "invalid equijoin condition");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
  }
  return ret;
}*/

int ObHashJoinSingle::curr_row_is_qualified(bool &is_qualified)
{
  int ret = OB_SUCCESS;
  is_qualified = true;
  const ObObj *res = NULL;
  // int hash_mape_index = 0;//add tianz [SubQuery_for_Instmt] [JHOBv0.1] 20140610
  for (int64_t i = 0; i < other_join_conds_.count(); ++i)
  {
    ObSqlExpression &expr = other_join_conds_.at(i);
    //mod peiouya [IN_TYPEBUG_FIX] 20151225:b
    //add tianz [SubQuery_for_Instmt] [JHOBv0.1] 20140610:b
    /*
    bool use_hash_map = false;
    bool is_hashmap_contain_null = false;  //add peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518
    common::hash::ObHashMap<common::ObRowkey,common::ObRowkey,common::hash::NoPthreadDefendMode>* p = NULL;
    common::ObArray<ObObjType> * p_data_type_desc = NULL;
    if(hashmap_num_>0 && expr.get_sub_query_num()>0)
    {
      p =&(sub_query_map_[hash_mape_index]);
      is_hashmap_contain_null = is_subquery_result_contain_null[hash_mape_index];  //add peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518
      p_data_type_desc=  &(sub_query_map_and_bloomfilter_column_type[hash_mape_index]);
      use_hash_map = true;
      hash_mape_index = hash_mape_index + (int)expr.get_sub_query_num();
    }
    //add 20140610:e
    */
    
    //mod peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518:b
    ////mod tianz [SubQuery_for_Instmt] [JHOBv0.1] 20140610:b
    if (OB_SUCCESS != (ret = expr.calc(curr_row_, res)))
    ////if (OB_SUCCESS != (ret = expr.calc(curr_row_, res, p, use_hash_map)))
    ////mod 20140610:e
    //if (OB_SUCCESS != (ret = expr.calc(curr_row_, res, p, p_data_type_desc, use_hash_map)))
    // if (OB_SUCCESS != (ret = expr.calc(curr_row_, res, p, is_hashmap_contain_null,p_data_type_desc, use_hash_map)))
    //mod 20151225:e
    //mod peiouya [IN_AND NOT_IN_WITH_NULL_BUG_FIX] 20160518:e
    {
      TBSYS_LOG(WARN, "failed to calc expr, err=%d", ret);
    }
    else if (!res->is_true())
    {
      is_qualified = false;
      break;
    }
  }
  return ret;
}

int ObHashJoinSingle::cons_row_desc(const ObRowDesc &rd1, const ObRowDesc &rd2)
{
  int ret = OB_SUCCESS;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  for (int64_t i = 0; i < rd1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = rd1.get_tid_cid(i, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch");
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = row_desc_.add_column_desc(tid, cid)))
    {
      TBSYS_LOG(WARN, "failed to add column desc, err=%d", ret);
      break;
    }
  }
  for (int64_t i = 0; OB_SUCCESS == ret && i < rd2.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = rd2.get_tid_cid(i, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch");
      ret = OB_ERR_UNEXPECTED;
    }
    else if (OB_SUCCESS != (ret = row_desc_.add_column_desc(tid, cid)))
    {
      TBSYS_LOG(WARN, "failed to add column desc, err=%d", ret);
    }
  }
  return ret;
}

int ObHashJoinSingle::join_rows(const ObRow& r1, const ObRow& r2)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t i = 0;
  for (; i < r1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = r1.raw_get_cell(i, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  }
  for (int64_t j = 0; OB_SUCCESS == ret && j < r2.get_column_num(); ++j)
  {
    if (OB_SUCCESS != (ret = r2.raw_get_cell(j, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i+j, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  }
  return ret;
}

int ObHashJoinSingle::left_join_rows(const ObRow& r1)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t i = 0;
  for (; i < r1.get_column_num(); ++i)
  {
    if (OB_SUCCESS != (ret = r1.raw_get_cell(i, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
      break;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  }
  int64_t right_row_column_num = row_desc_.get_column_num() - r1.get_column_num();
  ObObj null_cell;
  null_cell.set_null();
  for (int64_t j = 0; OB_SUCCESS == ret && j < right_row_column_num; ++j)
  {
    if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i+j, null_cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  }
  return ret;
}

int ObHashJoinSingle::right_join_rows(const ObRow& r2)
{
  int ret = OB_SUCCESS;
  const ObObj *cell = NULL;
  uint64_t tid = OB_INVALID_ID;
  uint64_t cid = OB_INVALID_ID;
  int64_t left_row_column_num = row_desc_.get_column_num() - r2.get_column_num();
  ObObj null_cell;
  null_cell.set_null();
  for (int64_t i = 0; i < left_row_column_num; ++i)
  {
    if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(i, null_cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d i=%ld", ret, i);
      break;
    }
  }
  for (int64_t j = 0; OB_SUCCESS == ret && j < r2.get_column_num(); ++j)
  {
    if (OB_SUCCESS != (ret = r2.raw_get_cell(j, cell, tid, cid)))
    {
      TBSYS_LOG(ERROR, "unexpected branch, err=%d", ret);
      ret = OB_ERR_UNEXPECTED;
    }
    else if (OB_SUCCESS != (ret = curr_row_.raw_set_cell(left_row_column_num+j, *cell)))
    {
      TBSYS_LOG(WARN, "failed to set cell, err=%d j=%ld", ret, j);
    }
  }
  return ret;
}

// INNER_JOIN
//int ObHashJoinSingle::inner_get_next_row(const common::ObRow *&row)
//{
//  int ret = OB_SUCCESS;
//  //const ObRow *left_row = NULL;
//  const ObRow *right_row = NULL;
//  //get_next_right_row(right_row);
//  // fetch the next left row
//  if (NULL != last_left_row_)
//  {
//    curr_cached_left_row_ = *last_left_row_;
//    last_left_row_ = NULL;
//  }
//  else
//  {
//    ret = left_cache_.get_next_row(curr_cached_left_row_);
//  }
//  while(OB_SUCCESS == ret)
//  {
//    if (right_cache_is_valid_)
//    {
//      OB_ASSERT(!right_cache_.is_empty());
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = left_row_compare_equijoin_cond(curr_cached_left_row_, last_join_left_row_, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        // fetch the next right row from right_cache
//        if (OB_SUCCESS != (ret = right_cache_.get_next_row(curr_cached_right_row_)))
//        {
//          if (OB_UNLIKELY(OB_ITER_END != ret))
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right_cache, err=%d", ret);
//          }
//          else
//          {
//            right_cache_.reset_iterator(); // continue
//            // fetch the next left row
//            ret = left_cache_.get_next_row(curr_cached_left_row_);
//          }
//        }
//        else
//        {
//          bool is_qualified = false;
//          if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, curr_cached_right_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//          {
//            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//          }
//          else if (is_qualified)
//          {
//            // output
//            row = &curr_row_;
//            last_left_row_ = &curr_cached_left_row_;
//            break;
//          }
//          else
//          {
//            // continue with the next cached right row
//            OB_ASSERT(NULL == last_left_row_);
//          }
//        }
//      }
//      else
//      {
//        // left_row > last_join_left_row_ on euqijoin conditions
//        right_cache_is_valid_ = false;
//        right_cache_.clear();
//      }
//    }
//    else
//    {
//      //  TBSYS_LOG(ERROR,"DHC etch the next right row number=[%d]",number++);
//      // fetch the next right row
//      if (OB_UNLIKELY(is_right_iter_end_))
//      {
//        ret = OB_ITER_END;
//        break;
//      }
//      else if (NULL != last_right_row_)
//      {
//        right_row = last_right_row_;
//        last_right_row_ = NULL;
//      }
//      else
//      {
//        ret = right_op_->get_next_row(right_row);;
//        //TBSYS_LOG(ERROR,"DHC etch the next right row ret=[%d]",ret);
//        if (OB_SUCCESS != ret)
//        {
//          if (OB_ITER_END == ret)
//          {
//            TBSYS_LOG(DEBUG, "end of right child op");
//            is_right_iter_end_ = true;
//            if (!right_cache_.is_empty())
//            {
//              // no more right rows and the right cache is not empty, we SHOULD look at the next left row
//              right_cache_is_valid_ = true;
//              OB_ASSERT(NULL == last_right_row_);
//              OB_ASSERT(NULL == last_left_row_);
//              ret = left_cache_.get_next_row(curr_cached_left_row_);
//            }
//            continue;
//          }
//          else
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
//            break;
//          }
//        }
//      }
//      OB_ASSERT(right_row);
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, *right_row, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        if (right_cache_.is_empty())
//        {
//          // store the joined left row
//          last_join_left_row_store_.assign_buffer(last_join_left_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
//          if (OB_SUCCESS != (ret = ObRowUtil::convert(curr_cached_left_row_, last_join_left_row_store_, last_join_left_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to store left row, err=%d", ret);
//            break;
//          }
//        }
//        bool is_qualified = false;
//        const ObRowStore::StoredRow *stored_row = NULL;
//        if (OB_SUCCESS != (ret = right_cache_.add_row(*right_row, stored_row)))
//        {
//          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, *right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_left_row_ = &curr_cached_left_row_;
//          OB_ASSERT(NULL == last_right_row_);
//          break;
//        }
//        else
//        {
//          // continue with the next right row
//          //OB_ASSERT(NULL != left_row);
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//        }
//      } // end 0 == cmp
//      else if (cmp < 0)
//      {
//        if (!right_cache_.is_empty())
//        {
//          right_cache_is_valid_ = true;
//        }
//        last_right_row_ = right_row;
//        OB_ASSERT(NULL == last_left_row_);
//        ret = left_cache_.get_next_row(curr_cached_left_row_);
//      }
//      else
//      {
//        //OB_ASSERT(NULL != left_row);
//        OB_ASSERT(NULL == last_left_row_);
//        OB_ASSERT(NULL == last_right_row_);
//      }
//    }
//  } // end while
//  return ret;
//}

//int ObHashJoinSingle::left_outer_get_next_row(const common::ObRow *&row)
//{
//  //TBSYS_LOG(ERROR, "dhc ObHashJoinSingle::left_outer_get_next_row() [%d]",(int)left_cache_.is_empty());
//  int ret = OB_SUCCESS;
//  const ObRow *right_row = NULL;
//  // fetch the next left row
//  if (NULL != last_left_row_)
//  {
//    curr_cached_left_row_ = *last_left_row_;
//    last_left_row_ = NULL;
//    last_left_row_has_printed_ = true;
//  }
//  else
//  {
//    ret = left_cache_.get_next_row(curr_cached_left_row_);
//    last_left_row_has_printed_ = false;
//  }
//  while(OB_SUCCESS == ret)
//  {
//    if (right_cache_is_valid_)
//    {
//      OB_ASSERT(!right_cache_.is_empty());
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = left_row_compare_equijoin_cond(curr_cached_left_row_, last_join_left_row_, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        // fetch the next right row from right_cache
//        if (OB_SUCCESS != (ret = right_cache_.get_next_row(curr_cached_right_row_)))
//        {
//          if (OB_UNLIKELY(OB_ITER_END != ret))
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right_cache, err=%d", ret);
//          }
//          else
//          {
//            right_cache_.reset_iterator(); // continue
//            if (!last_left_row_has_printed_)
//            {
//                if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//                {
//                    TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//                }
//                else
//                {
//                    // output
//                    row = &curr_row_;
//                    last_left_row_ = NULL;
//                    break;
//                }
//            }
//            else
//            {
//                ret = left_cache_.get_next_row(curr_cached_left_row_);
//                last_left_row_has_printed_ = false;
//            }
//            //add 20140610:e
//          }
//        }
//        else
//        {
//          bool is_qualified = false;
//          if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, curr_cached_right_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//          {
//            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//          }
//          else if (is_qualified)
//          {
//            // output
//            row = &curr_row_;
//            last_left_row_ = &curr_cached_left_row_;
//            break;
//          }
//          else
//          {
//            // continue with the next cached right row
//            OB_ASSERT(NULL == last_left_row_);
//          }
//        }
//      }
//      else
//      {
//        // left_row > last_join_left_row_ on euqijoin conditions
//        right_cache_is_valid_ = false;
//        right_cache_.clear();
//      }
//    }
//    else
//    {
//      // fetch the next right row
//      if (OB_UNLIKELY(is_right_iter_end_))
//      {
//        // no more right rows, but there are left rows left
//        //del dyr [using_AND_in_ONstmt] [JHOBv0.1] 20140610:b
//        //bool is_qualified = false;
//        //del 20140610:e
//        if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else
//        {
//          // output
//          row = &curr_row_;
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//          break;
//        }
//      }
//      else if (NULL != last_right_row_)
//      {
//        right_row = last_right_row_;
//        last_right_row_ = NULL;
//      }
//      else
//      {
//        ret = right_op_->get_next_row(right_row);;
//        if (OB_SUCCESS != ret)
//        {
//          if (OB_ITER_END == ret)
//          {
//            //TBSYS_LOG(INFO, "end of right child op");
//            is_right_iter_end_ = true;
//            if (!right_cache_.is_empty())
//            {
//              // no more right rows and the right cache is not empty, we SHOULD look at the next left row
//              right_cache_is_valid_ = true;
//              OB_ASSERT(NULL == last_right_row_);
//              OB_ASSERT(NULL == last_left_row_);
//              if (!last_left_row_has_printed_)
//              {
//                  if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//                  {
//                      TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//                  }
//                  else
//                  {
//                      // output
//                      row = &curr_row_;
//                      break;
//                  }
//              }
//              else
//              {
//                  ret = left_cache_.get_next_row(curr_cached_left_row_);
//                  last_left_row_has_printed_ = false;
//              }
//            }
//            else
//            {
//              ret = OB_SUCCESS;
//            }
//            continue;
//          }
//          else
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
//            break;
//          }
//        }
//      }
//      OB_ASSERT(right_row);
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, *right_row, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        if (right_cache_.is_empty())
//        {
//          // store the joined left row
//          last_join_left_row_store_.assign_buffer(last_join_left_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
//          if (OB_SUCCESS != (ret = ObRowUtil::convert(curr_cached_left_row_, last_join_left_row_store_, last_join_left_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to store left row, err=%d", ret);
//            break;
//          }
//        }
//        bool is_qualified = false;
//        const ObRowStore::StoredRow *stored_row = NULL;
//        if (OB_SUCCESS != (ret = right_cache_.add_row(*right_row, stored_row)))
//        {
//          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, *right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_left_row_ = &curr_cached_left_row_;
//          OB_ASSERT(NULL == last_right_row_);
//          break;
//        }
//        else
//        {
//          // continue with the next right row
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//        }

//      } // end 0 == cmp
//      else if (cmp < 0)
//      {
//        // left_row < right_row on equijoin conditions
//        if (!right_cache_.is_empty())
//        {
//          right_cache_is_valid_ = true;
//          OB_ASSERT(NULL == last_left_row_);
//          last_right_row_ = right_row;
//          //del dyr [using_AND_in_ONstmt] [JHOBv0.1] 20140610:b
//          //ret = left_cache_.get_next_row(curr_cached_left_row_);
//          //del 20140610:e
//          //add dyr [using_AND_in_ONstmt] [JHOBv0.1] 20140610:b
//          if (!last_left_row_has_printed_)
//          {
//              if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//              {
//                  TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//              }
//              else
//              {
//                  // output
//                  row = &curr_row_;
//                  break;
//              }
//          }
//          else
//          {
//              ret = left_cache_.get_next_row(curr_cached_left_row_);
//              last_left_row_has_printed_ = false;
//          }
//        }
//        else
//        {
//          //del dyr [using_AND_in_ONstmt] [JHOBv0.1] 20140610:b
//          //bool is_qualified = false;
//          //del 20140610:e
//          if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else
//          {
//              // output
//              row = &curr_row_;
//              OB_ASSERT(NULL == last_left_row_);
//              last_right_row_ = right_row;
//              break;
//          }
//        }
//      }
//      else
//      {
//        // left_row > right_row on euqijoin conditions
//        // continue with the next right row
//        OB_ASSERT(NULL == last_left_row_);
//        OB_ASSERT(NULL == last_right_row_);
//      }
//    }
//  } // end while
//  return ret;
//}

//int ObHashJoinSingle::right_outer_get_next_row(const common::ObRow *&row)
//{
//  int ret = OB_SUCCESS;
//  const ObRow *right_row  = NULL;
//  // fetch the next right row
//  if (NULL != last_right_row_)
//  {
//    right_row = last_right_row_;
//    last_right_row_ = NULL;
//    last_right_row_has_printed_ = true;
//  }
//  else
//  {
//    ret = right_op_->get_next_row(right_row);;
//    last_right_row_has_printed_ = false;
//  }
//  while(OB_SUCCESS == ret)
//  {
//    if (left_cache_is_valid_)
//    {
//      OB_ASSERT(!left_cache_for_right_join_.is_empty());
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = right_row_compare_equijoin_cond(*right_row, last_join_right_row_, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        // fetch the next left row from left_cache
//        if (OB_SUCCESS != (ret = left_cache_for_right_join_.get_next_row(curr_cached_left_row_)))
//        {
//          if (OB_UNLIKELY(OB_ITER_END != ret))
//          {
//            TBSYS_LOG(WARN, "failed to get next row from left_cache, err=%d", ret);
//          }
//          else
//          {
//            left_cache_for_right_join_.reset_iterator(); // continue
//            if (!last_right_row_has_printed_)
//            {
//                if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//                {
//                    TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//                }
//                else
//                {
//                    // output
//                    row = &curr_row_;
//                    last_right_row_ = NULL;
//                    break;
//                }
//            }
//            else
//            {
//                ret = right_op_->get_next_row(right_row);;
//                last_right_row_has_printed_ = false;
//            }
//          }
//        }
//        else
//        {
//          bool is_qualified = false;
//          if (OB_SUCCESS != (ret = join_rows( curr_cached_left_row_,*right_row)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//          {
//            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//          }
//          else if (is_qualified)
//          {
//            // output
//            row = &curr_row_;
//            last_right_row_ = right_row;
//            break;
//          }
//          else
//          {
//            // continue with the next cached left row
//            OB_ASSERT(NULL == last_right_row_);
//            OB_ASSERT(NULL != right_row);
//          }
//        }
//      }
//      else
//      {
//        // right_row > last_join_right_row_ on euqijoin conditions
//        left_cache_is_valid_ = false;
//        left_cache_for_right_join_.clear();
//      }
//    }
//    else
//    {
//      // fetch the next left row
//      if (OB_UNLIKELY(is_left_iter_end_))
//      {
//        // no more left rows, but there are right rows left
//        OB_ASSERT(right_row);
//        if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else
//        {
//          // output
//          row = &curr_row_;
//          OB_ASSERT(NULL == last_right_row_);
//          OB_ASSERT(NULL == last_left_row_);
//          break;
//        }
//      }
//      else if (NULL != last_left_row_)
//      {
//        curr_cached_left_row_ = *last_left_row_;
//        last_left_row_ = NULL;
//      }
//      else
//      {
//        ret = left_cache_.get_next_row(curr_cached_left_row_);
//        if (OB_SUCCESS != ret)
//        {
//          if (OB_ITER_END == ret)
//          {
//            TBSYS_LOG(INFO, "end of left child op");
//            is_left_iter_end_ = true;
//            if (!left_cache_for_right_join_.is_empty())
//            {
//              // no more left rows and the left cache is not empty, we SHOULD look at the next right row
//              left_cache_is_valid_ = true;
//              OB_ASSERT(NULL == last_left_row_);
//              OB_ASSERT(NULL == last_right_row_);
//              if (!last_right_row_has_printed_)
//              {
//                  if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//                  {
//                      TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//                  }
//                  else
//                  {
//                      // output
//                      row = &curr_row_;
//                      break;
//                  }
//              }
//              else
//              {
//                  ret = right_op_->get_next_row(right_row);;
//                  last_right_row_has_printed_ = false;
//              }
//            }
//            else
//            {
//              ret = OB_SUCCESS;
//            }
//            continue;
//          }
//          else
//          {
//            TBSYS_LOG(WARN, "failed to get next row from left child, err=%d", ret);
//            break;
//          }
//        }
//      }
//      OB_ASSERT(right_row);
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, *right_row, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        if (left_cache_for_right_join_.is_empty())
//        {
//          // store the joined right row
//          last_join_right_row_store_.assign_buffer(last_join_right_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
//          if (OB_SUCCESS != (ret = ObRowUtil::convert(*right_row, last_join_right_row_store_, last_join_right_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to store right row, err=%d", ret);
//            break;
//          }
//        }
//        bool is_qualified = false;
//        const ObRowStore::StoredRow *stored_row = NULL;
//        if (OB_SUCCESS != (ret = left_cache_for_right_join_.add_row(curr_cached_left_row_, stored_row)))
//        {
//          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_,*right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_right_row_ = right_row;
//          OB_ASSERT(NULL == last_left_row_);
//          break;
//        }
//        else
//        {
//          // continue with the next left row
//          OB_ASSERT(NULL != right_row);
//          OB_ASSERT(NULL == last_right_row_);
//          OB_ASSERT(NULL == last_left_row_);
//        }

//      } // end 0 == cmp
//      else if (cmp > 0)
//      {
//        // right_row < curr_cached_left_row_ on equijoin conditions
//        if (!left_cache_for_right_join_.is_empty())
//        {
//          left_cache_is_valid_ = true;
//          OB_ASSERT(NULL == last_right_row_);
//          last_left_row_ = &curr_cached_left_row_;
//          if (!last_right_row_has_printed_)
//          {
//              if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//              {
//                  TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//              }
//              else
//              {
//                  // output
//                  row = &curr_row_;
//                  break;
//              }
//          }
//          else
//          {
//              ret = right_op_->get_next_row(right_row);;
//              last_right_row_has_printed_ = false;
//          }
//        }
//        else
//        {
//          if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else
//          {
//              // output
//              row = &curr_row_;
//              OB_ASSERT(NULL == last_right_row_);
//              last_left_row_ = &curr_cached_left_row_;
//              break;
//          }
//        }
//      }
//      else
//      {
//        // right_row > left_row on euqijoin conditions
//        // continue with the next left row
//        OB_ASSERT(NULL != right_row);
//        OB_ASSERT(NULL == last_right_row_);
//        OB_ASSERT(NULL == last_left_row_);
//      }
//    }
//  } // end while
//  return ret;
//}

//int ObHashJoinSingle::full_outer_get_next_row(const common::ObRow *&row)
//{
//  int ret = OB_SUCCESS;
//  const ObRow *left_row = NULL;
//  const ObRow *right_row = NULL;
//  // fetch the next left row
//  if (NULL != last_left_row_)
//  {
//    curr_cached_left_row_ = *last_left_row_;
//    last_left_row_ = NULL;
//  }
//  else
//  {
//    ret = left_cache_.get_next_row(curr_cached_left_row_);
//  }

//  while(OB_SUCCESS == ret)
//  {
//    if (right_cache_is_valid_)
//    {
//      OB_ASSERT(!right_cache_.is_empty());
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = left_row_compare_equijoin_cond(curr_cached_left_row_, last_join_left_row_, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        // fetch the next right row from right_cache
//        if (OB_SUCCESS != (ret = right_cache_.get_next_row(curr_cached_right_row_)))
//        {
//          if (OB_UNLIKELY(OB_ITER_END != ret))
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right_cache, err=%d", ret);
//          }
//          else
//          {
//            right_cache_.reset_iterator(); // continue
//            // fetch the next left row
//            ret = left_cache_.get_next_row(curr_cached_left_row_);
//          }
//        }
//        else
//        {
//          bool is_qualified = false;
//          if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, curr_cached_right_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//          {
//            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//          }
//          else if (is_qualified)
//          {
//            // output
//            row = &curr_row_;
//            last_left_row_ = &curr_cached_left_row_;
//            break;
//          }
//          else
//          {
//            // continue with the next cached right row
//            OB_ASSERT(NULL == last_left_row_);
//            //OB_ASSERT(NULL != left_row);
//          }
//        }
//      }
//      else
//      {
//        // left_row > last_join_left_row_ on euqijoin conditions
//        right_cache_is_valid_ = false;
//        right_cache_.clear();
//      }
//    }
//    else
//    {
//      // fetch the next right row
//      if (OB_UNLIKELY(is_right_iter_end_))
//      {
//        // no more right rows, but there are left rows left
//        OB_ASSERT(left_row);
//        bool is_qualified = false;
//        if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//          break;
//        }
//        else
//        {
//          // continue with the next left row
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//          ret = left_cache_.get_next_row(curr_cached_left_row_);
//          continue;
//        }
//      }
//      else if (NULL != last_right_row_)
//      {
//        right_row = last_right_row_;
//        last_right_row_ = NULL;
//      }
//      else
//      {
//        ret = right_op_->get_next_row(right_row);;
//        if (OB_SUCCESS != ret)
//        {
//          if (OB_ITER_END == ret)
//          {
//            TBSYS_LOG(INFO, "end of right child op");
//            is_right_iter_end_ = true;
//            if (!right_cache_.is_empty())
//            {
//              // no more right rows and the right cache is not empty, we SHOULD look at the next left row
//              right_cache_is_valid_ = true;
//              OB_ASSERT(NULL == last_right_row_);
//              OB_ASSERT(NULL == last_left_row_);
//              ret = left_cache_.get_next_row(curr_cached_left_row_);
//            }
//            else
//            {
//              ret = OB_SUCCESS;
//            }
//            continue;
//          }
//          else
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
//            break;
//          }
//        }
//      }
//      OB_ASSERT(right_row);
//      int cmp = 0;
//      if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, *right_row, cmp)))
//      {
//        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
//        break;
//      }
//      if (0 == cmp)
//      {
//        if (right_cache_.is_empty())
//        {
//          // store the joined left row
//          last_join_left_row_store_.assign_buffer(last_join_left_row_store_.ptr(), MAX_SINGLE_ROW_SIZE);
//          if (OB_SUCCESS != (ret = ObRowUtil::convert(curr_cached_left_row_, last_join_left_row_store_, last_join_left_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to store left row, err=%d", ret);
//            break;
//          }
//        }
//        bool is_qualified = false;
//        const ObRowStore::StoredRow *stored_row = NULL;
//        if (OB_SUCCESS != (ret = right_cache_.add_row(*right_row, stored_row)))
//        {
//          TBSYS_LOG(WARN, "failed to store the row, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = join_rows(curr_cached_left_row_, *right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_left_row_ = &curr_cached_left_row_;
//          OB_ASSERT(NULL == last_right_row_);
//          break;
//        }
//        else
//        {
//          // continue with the next right row
//          OB_ASSERT(NULL == last_left_row_);
//          OB_ASSERT(NULL == last_right_row_);
//        }
//      } // end 0 == cmp
//      else if (cmp < 0)
//      {
//        // left_row < right_row on equijoin conditions
//        if (!right_cache_.is_empty())
//        {
//          right_cache_is_valid_ = true;
//          OB_ASSERT(NULL == last_left_row_);
//          last_right_row_ = right_row;
//          ret = left_cache_.get_next_row(curr_cached_left_row_);
//        }
//        else
//        {
//          bool is_qualified = false;
//          if (OB_SUCCESS != (ret = left_join_rows(curr_cached_left_row_)))
//          {
//            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//          }
//          else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//          {
//            TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//          }
//          else if (is_qualified)
//          {
//            // output
//            row = &curr_row_;
//            OB_ASSERT(NULL == last_left_row_);
//            last_right_row_ = right_row;
//            break;
//          }
//          else
//          {
//            // continue with the next left row
//            OB_ASSERT(NULL == last_left_row_);
//            ret = left_cache_.get_next_row(curr_cached_left_row_);
//            last_right_row_ = right_row;
//          }
//        }
//      }
//      else
//      {
//        // left_row > right_row on euqijoin conditions
//        OB_ASSERT(NULL != right_row);
//        OB_ASSERT(NULL == last_left_row_);
//        OB_ASSERT(NULL == last_right_row_);
//        bool is_qualified = false;
//        if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_left_row_ = &curr_cached_left_row_;
//          break;
//        }
//        else
//        {
//          // continue with the next right row
//        }
//      }
//    }
//  } // end while
//  if (OB_ITER_END == ret && !is_right_iter_end_)
//  {
//    OB_ASSERT(NULL == last_left_row_);
//    // left row finished but we still have right rows left
//    do
//    {
//      if (NULL != last_right_row_)
//      {
//        right_row = last_right_row_;
//        last_right_row_ = NULL;
//      }
//      else
//      {
//        ret = right_op_->get_next_row(right_row);;
//        if (OB_SUCCESS != ret)
//        {
//          if (OB_ITER_END == ret)
//          {
//            TBSYS_LOG(INFO, "end of right child op");
//            break;
//          }
//          else
//          {
//            TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
//            break;
//          }
//        }
//      }
//      OB_ASSERT(right_row);
//      OB_ASSERT(NULL == last_right_row_);
//      bool is_qualified = false;
//      if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
//      {
//        TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//      }
//      else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//      {
//        TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//      }
//      else if (is_qualified)
//      {
//        // output
//        row = &curr_row_;
//        break;
//      }
//      else
//      {
//        // continue with the next right row
//      }
//    }
//    while (OB_SUCCESS == ret);
//  }
//  return ret;
//}

//int ObHashJoinSingle::left_rows(const common::ObRow *&row, int &left_hash_id_for_left_outer_join_)
//{
//  //TBSYS_LOG(ERROR,"DHC last_hash_i=%d",left_hash_id_for_left_outer_join_);
//  int ret = OB_SUCCESS;
//  for(; left_hash_id_for_left_outer_join_ < HASH_BUCKET_NUM; )
//  {
//    if (OB_SUCCESS != (ret = hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.get_next_row(curr_cached_left_row_)))
//    {
//      if (OB_UNLIKELY(OB_ITER_END != ret))
//      {
//        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
//      }
//      else
//      {
//        left_hash_id_for_left_outer_join_++;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ = 0;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.reset_iterator();
//        continue;
//      }
//    }
//    else
//    {
//      hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_++;
//      //TBSYS_LOG(ERROR,"left_rows i=%d %s %d",left_hash_id_for_left_outer_join_,to_cstring(curr_cached_left_row_),curr_cached_left_row_.get_raw_row().get_reserved2());
//      if(1==hash_table_row_store[(int)left_hash_id_for_left_outer_join_].hash_iterators.at(hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_-1))
//      {
//        continue;
//      }
//      row = &curr_cached_left_row_;
//      break;
//    }
//  }
//  return ret;
//}

//modify maoxx [hash join single] 20170614
/*int ObHashJoinSingle::get_next_leftouterjoin_left_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  for(; left_hash_id_for_left_outer_join_ < HASH_BUCKET_NUM; )
  {
    if (OB_SUCCESS != (ret = hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.get_next_row(curr_cached_left_row_)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
      }
      else
      {
        left_hash_id_for_left_outer_join_++;
        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ = 0;
        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.reset_iterator();
        continue;
      }
    }
    else
    {
      hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_++;
      if(1 == hash_table_row_store[(int)left_hash_id_for_left_outer_join_].hash_iterators.at(hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ - 1))
      {
        continue;
      }
      row = &curr_cached_left_row_;
      break;
    }
  }
  if(left_hash_id_for_left_outer_join_ >= HASH_BUCKET_NUM)
  {
    ret = OB_ITER_END;
  }
  return ret;
}*/
int ObHashJoinSingle::get_next_leftouterjoin_left_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  int64_t hash_table_bucket_num = hash::cal_next_prime(HASH_BUCKET_NUM);
  HashTableRowPair* pair = NULL;
  for(; left_bucket_pos_for_left_outer_join_ < hash_table_bucket_num; )
  {
    if(common::hash::HASH_EXIST != (ret = hash_table_.get_all(left_bucket_pos_for_left_outer_join_, bucket_node_, 0, pair)))
    {
      if (common::hash::HASH_NOT_EXIST != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
        ret = OB_ERROR;
      }
      else
      {
        left_bucket_pos_for_left_outer_join_++;
        bucket_node_ = NULL;
        ret = OB_ITER_END;
        continue;
      }
    }
    else
    {
      if(1 == pair->second)
      {
        continue;
      }
      if(OB_SUCCESS != (ret = ObRowUtil::convert(pair->first->get_compact_row(), curr_cached_left_row_)))
      {
        TBSYS_LOG(WARN, "fail to convert compact row to ObRow:ret[%d]", ret);
      }
      else
      {
        row = &curr_cached_left_row_;
      }
      break;
    }
  }
  if(left_bucket_pos_for_left_outer_join_ >= hash_table_bucket_num)
  {
    ret = OB_ITER_END;
  }
  return ret;
}
//modify e

//int ObHashJoinSingle::compare_hash_equijoin(const ObRow *&r1, const ObRow& r2, int &cmp, bool left_hash_id_cache_valid_, int &last_left_hash_id_)
//{
//  //TBSYS_LOG(ERROR, "dhc ObHashJoinSingle::compare_hash_equijoin()");
//  int ret = OB_SUCCESS;
//  uint64_t bucket_num = 0;
//  const ObObj *res1 = NULL;
//  const ObObj *res2 = NULL;
//  ObExprObj obj1;
//  ObExprObj obj2;
//  if(!left_hash_id_cache_valid_)
//  {
//    for (int64_t i = 0; i < con_length_; ++i)
//    {
//      const ObSqlExpression &expr = equal_join_conds_.at(i);
//      ExprItem::SqlCellInfo c1;
//      ExprItem::SqlCellInfo c2;
//      const ObObj *temp;
//      if (expr.is_equijoin_cond(c1, c2))
//      {
//        if (OB_SUCCESS != (ret = r2.get_cell(c2.tid,c2.cid,temp)))
//        {
//          TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
//          break;
//        }
//        else
//        {
//          bucket_num = temp->murmurhash64A(bucket_num);
//        }
//      }
//    }
//    bucket_num = bucket_num % HASH_BUCKET_NUM;
//    last_left_hash_id_ = (int)bucket_num;
//    hash_table_row_store[(int)bucket_num].row_store.reset_iterator();
//    hash_table_row_store[(int)bucket_num].id_no_=0;
//  }
//  else
//  {
//    bucket_num = last_left_hash_id_;
//  }
//  while(OB_SUCCESS == ret)
//  {
//    if (OB_SUCCESS != (ret = hash_table_row_store[(int)bucket_num].row_store.get_next_row(curr_cached_left_row_)))
//    {
//      if (OB_UNLIKELY(OB_ITER_END != ret))
//      {
//        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
//      }
//      left_hash_id_cache_valid_ = false;
//      break;
//    }
//    else
//    {
//      hash_table_row_store[(int)bucket_num].id_no_++;
//      for (int64_t ii = 0; ii < con_length_; ++ii)
//      {
//        const ObSqlExpression &expr = equal_join_conds_.at(ii);
//        ExprItem::SqlCellInfo c1;
//        ExprItem::SqlCellInfo c2;
//        if (expr.is_equijoin_cond(c1, c2))
//        {
//          if (OB_SUCCESS != (ret = curr_cached_left_row_.get_cell(c1.tid,c1.cid,res1)))
//          {
//            TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c1.tid, c1.cid);
//            break;
//          }
//          else if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, res2)))
//          {
//            TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
//            break;
//          }
//          else
//          {
//            obj1.assign(*res1);
//            obj2.assign(*res2);
//            if (OB_SUCCESS != obj1.compare(obj2, cmp))
//            {
//              if (obj1.is_null())
//              {
//                cmp = -10;
//              }
//              else
//              {
//                cmp = 10;
//              }
//              break;
//            }
//            else if (0 != cmp)
//            {
//              break;
//            }
//          }
//        }
//      }
//      if(cmp == 0)
//      {
//        r1 = &curr_cached_left_row_;
//        break;
//      }
//    }
//  }
//  return ret;
//}

//modify maoxx [hash join single] 20170614
/*int ObHashJoinSingle::get_next_equijoin_left_row(const ObRow *&r1, const ObRow& r2)
{
  int ret = OB_SUCCESS;
  uint64_t bucket_num = 0;
  if(!left_hash_id_cache_valid_)
  {
    for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
    {
      const ObSqlExpression &expr = equal_join_conds_.at(i);
      ExprItem::SqlCellInfo c1;
      ExprItem::SqlCellInfo c2;
      const ObObj *temp = NULL;
      if (expr.is_equijoin_cond(c1, c2))
      {
        if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, temp)))
        {
          TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
          break;
        }
        else
        {
          bucket_num = temp->murmurhash64A(bucket_num);
        }
      }
    }
    bucket_num = bucket_num % HASH_BUCKET_NUM;
    last_left_hash_id_ = (int)bucket_num;
    hash_table_row_store[(int)bucket_num].row_store.reset_iterator();
    hash_table_row_store[(int)bucket_num].id_no_ = 0;
  }
  else
  {
    bucket_num = last_left_hash_id_;
  }
  while(OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = hash_table_row_store[(int)bucket_num].row_store.get_next_row(curr_cached_left_row_)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
      }
      left_hash_id_cache_valid_ = false;
      break;
    }
    else
    {
      hash_table_row_store[(int)bucket_num].id_no_++;
      int cmp = 0;
      if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, r2, cmp)))
      {
        TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
        break;
      }
      if(cmp == 0)
      {
        r1 = &curr_cached_left_row_;
        break;
      }
    }
  }
  return ret;
}*/
int ObHashJoinSingle::get_next_equijoin_left_row(const ObRow *&r1, const ObRow& r2, uint64_t& bucket_hash_key, HashTableRowPair*& pair)
{
  int ret = OB_SUCCESS;
  uint64_t hash_key = 0;
  if(!left_hash_key_cache_valid_)
  {
    for (int64_t i = 0; i < equal_join_conds_.count(); ++i)
    {
      const ObSqlExpression &expr = equal_join_conds_.at(i);
      ExprItem::SqlCellInfo c1;
      ExprItem::SqlCellInfo c2;
      const ObObj *temp = NULL;
      if (expr.is_equijoin_cond(c1, c2))
      {
        if (OB_SUCCESS != (ret = r2.get_cell(c2.tid, c2.cid, temp)))
        {
          TBSYS_LOG(ERROR, "failed to get cell, err=%d tid=%lu cid=%lu", ret, c2.tid, c2.cid);
          break;
        }
        else
        {
          hash_key = temp->murmurhash64A(hash_key);
        }
      }
    }
    last_left_hash_key_ = hash_key;
  }
  else
  {
    hash_key = last_left_hash_key_;
  }
  bucket_hash_key = hash_key;
  while(OB_SUCCESS == ret)
  {
    if(common::hash::HASH_EXIST != (ret = hash_table_.get_multiple(bucket_node_, hash_key, pair)))
    {
      if (common::hash::HASH_NOT_EXIST != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
        ret = OB_ERROR;
      }
      else
      {
        ret = OB_ITER_END;
      }
      left_hash_key_cache_valid_ = false;
      break;
    }
    else
    {
      if(OB_SUCCESS != (ret = ObRowUtil::convert(pair->first->get_compact_row(), curr_cached_left_row_)))
      {
        TBSYS_LOG(WARN, "fail to convert compact row to ObRow:ret[%d]", ret);
      }
      else
      {
        int cmp = 0;
        if (OB_SUCCESS != (ret = compare_equijoin_cond(curr_cached_left_row_, r2, cmp)))
        {
          TBSYS_LOG(WARN, "failed to compare, err=%d", ret);
          break;
        }
        if(cmp == 0)
        {
          r1 = &curr_cached_left_row_;
          break;
        }
      }
    }
  }
  return ret;
}
//modify e

//int ObHashJoinSingle::inner_hash_get_next_row(const common::ObRow *&row)
//{
//  int ret = OB_SUCCESS;
//  const ObRow *right_row = NULL;
//  const ObRow *left_row = NULL;

//  while(OB_SUCCESS == ret)
//  {
//    if (left_hash_id_cache_valid_)
//    {
//      int cmp = -10;
//      if (OB_SUCCESS != (ret = compare_hash_equijoin(left_row, *last_right_row_, cmp, left_hash_id_cache_valid_, last_left_hash_id_)))
//      {
//        if(OB_ITER_END == ret)
//        {
//          cmp = -10;
//          ret = OB_SUCCESS;
//        }
//      }
//      if (0 == cmp)
//      {
//        bool is_qualified = false;
//        if (OB_SUCCESS != (ret = join_rows(*left_row, *last_right_row_)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_-1) = 1;
//          //TBSYS_LOG(ERROR,"curr_hash_row %s",to_cstring(*left_row));
//          break;
//        }
//      }
//      else
//      {
//        left_hash_id_cache_valid_ = false;
//        ret = OB_SUCCESS;
//      }
//    }
//    else
//    {
//      // fetch the next right row
//      ret = right_op_->get_next_row(right_row);
//      if (OB_SUCCESS != ret)
//      {
//        if (OB_ITER_END == ret)
//        {
//          TBSYS_LOG(DEBUG, "end of right child op");
////          is_right_iter_end_ = true;
//          break;
//        }
//        else
//        {
//          TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
//          break;
//        }
//      }
//      OB_ASSERT(right_row);
//      int cmp = -10;
//      if (OB_SUCCESS != (ret = compare_hash_equijoin(left_row, *right_row, cmp, left_hash_id_cache_valid_, last_left_hash_id_)))
//      {
//        if(OB_ITER_END == ret)
//        {
//          cmp = -10;
//          ret = OB_SUCCESS;
//        }
//      }
//      if (0 == cmp)
//      {
//        left_hash_id_cache_valid_ = true;
//        bool is_qualified = false;
//        if (OB_SUCCESS != (ret = join_rows(*left_row, *right_row)))
//        {
//          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//        }
//        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
//        {
//          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
//        }
//        else if (is_qualified)
//        {
//          // output
//          row = &curr_row_;
//          last_right_row_ = right_row;
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_-1) = 1;
//          break;
//        }
//      }
//    }
//  } // end while

//  return ret;
//}

int ObHashJoinSingle::inner_hash_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  const ObRow *right_row = NULL;
  const ObRow *left_row = NULL;

  //add maoxx [hash join single] 20170614
  HashTableRowPair* pair = NULL;
  uint64_t bucket_hash_key = 0;
  //add e

  while(OB_SUCCESS == ret)
  {
    if (left_hash_key_cache_valid_)
    {
      //modify maoxx [hash join single] 20170614
//      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *last_right_row_)))
      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *last_right_row_, bucket_hash_key, pair)))
      //modify e
      {
        if(OB_ITER_END == ret)
        {
          //add maoxx [hash join single] 20170614
          bucket_node_ = NULL;
          //add e
          left_hash_key_cache_valid_ = false;
          ret = OB_SUCCESS;
        }
      }
      else
      {
        bool is_qualified = false;
        if (OB_SUCCESS != (ret = join_rows(*left_row, *last_right_row_)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          //modify maoxx [hash join single] 20170614
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_ - 1) = 1;
          HashTableRowPair* new_pair = new HashTableRowPair(pair->first, 1);
          delete pair;
          pair = NULL;
          if(common::hash::HASH_MODIFY_SUCC != hash_table_.modify_multiple(bucket_node_, bucket_hash_key, new_pair))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
            ret = OB_ERROR;
          }
          //modify e
          break;
        }
      }
    }
    else
    {
      // fetch the next right row
      if(left_row_count_ == 0)
      {
        ret = OB_ITER_END;
      }
      else
      {
        ret = right_op_->get_next_row(right_row);
      }
      if (OB_SUCCESS != ret)
      {
        if (OB_ITER_END == ret)
        {
          TBSYS_LOG(DEBUG, "end of right child op");
          break;
        }
        else
        {
          TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
          break;
        }
      }
      OB_ASSERT(right_row);
      //modify maoxx [hash join single] 20170614
//      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *right_row)))
      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *right_row, bucket_hash_key, pair)))
      //modify e
      {
        if(OB_ITER_END == ret)
        {
          //add maoxx [hash join single] 20170614
          bucket_node_ = NULL;
          //add e
          left_hash_key_cache_valid_ = false;
          ret = OB_SUCCESS;
        }
      }
      else
      {
        //modify maoxx [hash join single bug fix] 20170908
        //modify maoxx  [hash join single bug fix] 20170428
        left_hash_key_cache_valid_ = true;
        last_right_row_ = right_row;
        //modify e
        //modify e
        bool is_qualified = false;
        if (OB_SUCCESS != (ret = join_rows(*left_row, *right_row)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          //delete maoxx [hash join single bug fix] 20170908
          //add maoxx  [hash join single bug fix] 20170428
//          left_hash_key_cache_valid_ = true;
          //add e
//          last_right_row_ = right_row;
          //delete e
          //modify maoxx [hash join single] 20170614
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_ - 1) = 1;
          HashTableRowPair* new_pair = new HashTableRowPair(pair->first, 1);
          delete pair;
          pair = NULL;
          if(common::hash::HASH_MODIFY_SUCC != hash_table_.modify_multiple(bucket_node_, bucket_hash_key, new_pair))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
            ret = OB_ERROR;
          }
          //modify e
          break;
        }
      }
    }
  } // end while
  return ret;
}

//int ObHashJoinSingle::left_hash_outer_get_next_row(const common::ObRow *&row)
//{
//  int ret = OB_SUCCESS;
//  const ObRow *left_row = NULL;
//  const ObRow *inner_hash_row = NULL;
//  if(left_hash_id_for_left_outer_join_ == -1)
//  {
//    if (OB_SUCCESS != (ret = inner_hash_get_next_row(inner_hash_row)))
//    {
//      if (OB_UNLIKELY(OB_ITER_END != ret))
//      {
//        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
//      }
//      else
//      {
//        left_hash_id_for_left_outer_join_ = 0;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ = 0;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.reset_iterator();
//      }
//    }
//    else
//    {
//      row = inner_hash_row;
//    }
//  }
//  if(left_hash_id_for_left_outer_join_ != -1)
//  {
//    if (OB_SUCCESS != (ret = left_rows(left_row, left_hash_id_for_left_outer_join_)))
//    {
//      if (OB_UNLIKELY(OB_ITER_END != ret))
//      {
//        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
//      }
//    }
//    else
//    {
//      //TBSYS_LOG(ERROR,"DHC last_hash_i=%d",left_hash_id_for_left_outer_join_);
//      if (OB_SUCCESS != (ret = left_join_rows(*left_row)))
//      {
//        TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
//      }
//      else
//      {
//         //TBSYS_LOG(ERROR,"dhc right_row=%s  row=%s",to_cstring(*left_row),to_cstring(curr_row_));
//         row = &curr_row_;
//      }
//    }
//  }
//  return ret;
//}

int ObHashJoinSingle::left_hash_outer_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  const ObRow *left_row = NULL;
  const ObRow *inner_hash_row = NULL;

  if(left_bucket_pos_for_left_outer_join_ == -1)
  {
    if (OB_SUCCESS != (ret = inner_hash_get_next_row(inner_hash_row)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
      }
      else
      {
        left_bucket_pos_for_left_outer_join_ = 0;
        //delete maoxx [hash join single] 20170614
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ = 0;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.reset_iterator();
        //delete e
      }
    }
    else
    {
      row = inner_hash_row;
    }
  }
  if(left_bucket_pos_for_left_outer_join_ != -1)
  {
    if (OB_SUCCESS != (ret = get_next_leftouterjoin_left_row(left_row)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash table, err=%d", ret);
      }
    }
    else
    {
      if (OB_SUCCESS != (ret = left_join_rows(*left_row)))
      {
        TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
      }
      else
      {
         row = &curr_row_;
      }
    }
  }
  return ret;
}

int ObHashJoinSingle::right_hash_outer_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  const ObRow *right_row = NULL;
  const ObRow *left_row = NULL;

  //add maoxx [hash join single] 20170614
  HashTableRowPair* pair = NULL;
  uint64_t bucket_hash_key = 0;
  //add e

  while(OB_SUCCESS == ret)
  {
    if (left_hash_key_cache_valid_)
    {
      //modify maoxx [hash join single] 20170614
//      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *last_right_row_)))
      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *last_right_row_, bucket_hash_key, pair)))
      //modify e
      {
        if(OB_ITER_END == ret)
        {
          //add maoxx [hash join single] 20170614
          bucket_node_ = NULL;
          //add e
          left_hash_key_cache_valid_ = false;
          //modify maoxx [hash join single bug fix] 20170908
//          ret = OB_SUCCESS;
          if(flag_)
          {
            ret = OB_SUCCESS;
          }
          else if (OB_SUCCESS != (ret = right_join_rows(*last_right_row_)))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
          }
          else
          {
            // output
            row = &curr_row_;
            last_right_row_ = NULL;
            break;
          }
          //modify e
        }
      }
      else
      {
        bool is_qualified = false;
        if (OB_SUCCESS != (ret = join_rows(*left_row, *last_right_row_)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          //add maoxx [hash join single bug fix] 20170908
          flag_ = true;
          //add e
          //modify maoxx [hash join single] 20170614
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_ - 1) = 1;
          HashTableRowPair* new_pair = new HashTableRowPair(pair->first, 1);
          delete pair;
          pair = NULL;
          if(common::hash::HASH_MODIFY_SUCC != hash_table_.modify_multiple(bucket_node_, bucket_hash_key, new_pair))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
            ret = OB_ERROR;
          }
          //modify e
          break;
        }
      }
    }
    else
    {
      // fetch the next right row
      ret = right_op_->get_next_row(right_row);
      if (OB_SUCCESS != ret)
      {
        if (OB_ITER_END == ret)
        {
          TBSYS_LOG(DEBUG, "end of right child op");
          break;
        }
        else
        {
          TBSYS_LOG(WARN, "failed to get next row from right child, err=%d", ret);
          break;
        }
      }
      OB_ASSERT(right_row);
      //add maoxx [hash join single bug fix] 20170908
      flag_ = false;
      //add e
      //modify maoxx [hash join single] 20170614
//      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *right_row)))
      if (OB_SUCCESS != (ret = get_next_equijoin_left_row(left_row, *right_row, bucket_hash_key, pair)))
      //modify e
      {
        if(OB_ITER_END == ret)
        {
          //add maoxx [hash join single] 20170614
          bucket_node_ = NULL;
          //add e
          left_hash_key_cache_valid_ = false;
          if (OB_SUCCESS != (ret = right_join_rows(*right_row)))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
          }
          else
          {
            // output
            row = &curr_row_;
            last_right_row_ = NULL;
            break;
          }
        }
      }
      else
      {
        //modify maoxx [hash join single bug fix] 20170908
        //modify maoxx  [hash join single bug fix] 20170428
        left_hash_key_cache_valid_ = true;
        last_right_row_ = right_row;
        //modify e
        //modify e
        bool is_qualified = false;
        if (OB_SUCCESS != (ret = join_rows(*left_row, *right_row)))
        {
          TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = curr_row_is_qualified(is_qualified)))
        {
          TBSYS_LOG(WARN, "failed to test qualification, err=%d", ret);
        }
        else if (is_qualified)
        {
          // output
          row = &curr_row_;
          //add maoxx [hash join single bug fix] 20170908
          flag_ = true;
          //add e
          //delete maoxx [hash join single bug fix] 20170908
          //add maoxx  [hash join single bug fix] 20170428
//          left_hash_key_cache_valid_ = true;
          //add e
//          last_right_row_ = right_row;
          //delete e
          //modify maoxx [hash join single] 20170614
//          hash_table_row_store[(int)last_left_hash_id_].hash_iterators.at(hash_table_row_store[(int)last_left_hash_id_].id_no_ - 1) = 1;
          HashTableRowPair* new_pair = new HashTableRowPair(pair->first, 1);
          delete pair;
          pair = NULL;
          if(common::hash::HASH_MODIFY_SUCC != hash_table_.modify_multiple(bucket_node_, bucket_hash_key, new_pair))
          {
            TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
            ret = OB_ERROR;
          }
          //modify e
          break;
        }
      }
    }
  } // end while
  return ret;
}

int ObHashJoinSingle::full_hash_outer_get_next_row(const common::ObRow *&row)
{
  int ret = OB_SUCCESS;
  const ObRow *left_row = NULL;
  const ObRow *right_hash_row = NULL;
  if(left_bucket_pos_for_left_outer_join_ == -1)
  {
    if (OB_SUCCESS != (ret = right_hash_outer_get_next_row(right_hash_row)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash_table, err=%d", ret);
      }
      else
      {
        left_bucket_pos_for_left_outer_join_ = 0;
        //delete maoxx [hash join single] 20170614
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].id_no_ = 0;
//        hash_table_row_store[(int)left_hash_id_for_left_outer_join_].row_store.reset_iterator();
        //delete e
      }
    }
    else
    {
      row = right_hash_row;
    }
  }
  if(left_bucket_pos_for_left_outer_join_ != -1)
  {
    if (OB_SUCCESS != (ret = get_next_leftouterjoin_left_row(left_row)))
    {
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get next row from hash table, err=%d", ret);
      }
    }
    else
    {
      if (OB_SUCCESS != (ret = left_join_rows(*left_row)))
      {
        TBSYS_LOG(WARN, "failed to join rows, err=%d", ret);
      }
      else
      {
         row = &curr_row_;
      }
    }
  }
  return ret;
}

namespace oceanbase{
  namespace sql{
    REGISTER_PHY_OPERATOR(ObHashJoinSingle, PHY_HASH_JOIN_SINGLE);
  }
}

PHY_OPERATOR_ASSIGN(ObHashJoinSingle)
{
  int ret = OB_SUCCESS;
  UNUSED(other);
  reset();
  return ret;
}
