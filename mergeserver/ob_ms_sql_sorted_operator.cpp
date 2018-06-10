/**
 * (C) 2010-2011 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * ob_merger_sorted_operator.cpp for
 *
 * Authors:
 *   wushi <wushi.ly@taobao.com>
 *
 */
#include "ob_ms_sql_sorted_operator.h"
#include <algorithm>
#include "sql/ob_sql_scan_param.h"
#include "common/ob_new_scanner.h"
#include "common/ob_range2.h"
using namespace oceanbase;
using namespace oceanbase::common;
using namespace oceanbase::sql;
using namespace oceanbase::mergeserver;
using namespace std;

void oceanbase::mergeserver::ObMsSqlSortedOperator::sharding_result_t::init(ObNewScanner & sharding_res, const ObNewRange & query_range,
  const ObSqlScanParam &param, ObRowkey & last_process_rowkey, const int64_t fullfilled_item_num)
{
  sharding_res_ = &sharding_res;
  sharding_query_range_ = &query_range;
  param_ = &param;
  last_row_key_ = last_process_rowkey;
  fullfilled_item_num_ = fullfilled_item_num;
}

bool oceanbase::mergeserver::ObMsSqlSortedOperator::sharding_result_t::operator <(const sharding_result_t &other) const
{
  bool res = false;
  res = (sharding_query_range_->compare_with_startkey2(*other.sharding_query_range_) < 0);
  return res;
}

oceanbase::mergeserver::ObMsSqlSortedOperator::ObMsSqlSortedOperator()
{
  reset();
}


oceanbase::mergeserver::ObMsSqlSortedOperator::~ObMsSqlSortedOperator()
{
}

void oceanbase::mergeserver::ObMsSqlSortedOperator::reset()
{
  cur_sharding_result_idx_ = 0;
  scan_param_ = NULL;
  seamless_result_count_  = 0;
  total_mem_size_used_ = 0;
  sharding_result_arr_.clear();
  //add jinty [sort in cs bugfix] 20180324:b
  seamless_fullfilled_result_count_ = 0;
  cur_fullfilled_result_idx_ = 0;
  fullfilled_result_arr_.clear();
  //add:e
}

int oceanbase::mergeserver::ObMsSqlSortedOperator::set_param(const ObSqlScanParam & scan_param)
{
  int err = OB_SUCCESS;
  reset();
  scan_param_ = &scan_param;
  scan_range_ = *scan_param_->get_range();
  return err;
}

// note: keep in mind that "readed" scanners' rowkey buffer were freed already
void oceanbase::mergeserver::ObMsSqlSortedOperator::sort(bool &is_finish, ObNewScanner * last_sharding_res)
{
  /*
   * 1. sort unread scanners
   * 2. check if first sorted unread scanner's start key equals to last seamed end key
   * 3. if seamed, check if finish
   * 4. update sort params
   *
   * sharding_result_count_: total scanner received
   * cur_sharding_result_idx_: current readable scanner index
   * seamless_result_count_: seamed scanners
   *
   */
  int64_t seamless_result_idx = 0;
  // sort 'clean' scanners
  sharding_result_t *first = &sharding_result_arr_.at(0);
  sharding_result_t *start = &sharding_result_arr_.at(cur_sharding_result_idx_);
  int64_t count = sharding_result_arr_.count();
  std::sort(start, first+count);
  if (seamless_result_count_ <= 0)
  {
    // no seamless result
    if ((sharding_result_arr_[0].sharding_query_range_->start_key_  == scan_range_.start_key_)
        || (sharding_result_arr_[0].sharding_query_range_->start_key_.is_min_row()))
    {
      seamless_result_count_ = 1;
    }
  }

  if (seamless_result_count_ > 0)
  {
    // at lease one seamless result
    for (seamless_result_idx = seamless_result_count_;
         seamless_result_idx < sharding_result_arr_.count();
        seamless_result_idx++)
    {
      if (sharding_result_arr_[seamless_result_idx - 1].last_row_key_
          == sharding_result_arr_[seamless_result_idx].sharding_query_range_->start_key_)
      {
        seamless_result_count_ = seamless_result_idx + 1;
        TBSYS_LOG(DEBUG, "prev last_row_key=%s, cur star_key_=%s, seamless=%ld",
            to_cstring(sharding_result_arr_[seamless_result_idx - 1].last_row_key_),
            to_cstring(sharding_result_arr_[seamless_result_idx].sharding_query_range_->start_key_),
            seamless_result_count_
            );
      }
      else
      {
        break;
      }
    }
  }

  if (seamless_result_count_ > 0) // implicates that startkey matched already
  {
    OB_ASSERT(NULL != last_sharding_res);
    TBSYS_LOG(DEBUG, "last seamless=%s", to_cstring(sharding_result_arr_[seamless_result_count_-1].last_row_key_));
    if (sharding_result_arr_[seamless_result_count_-1].last_row_key_ >= scan_range_.end_key_ ||
        sharding_result_arr_[seamless_result_count_-1].last_row_key_.is_max_row())
    {
      /* check last seemless result.
       * Finish the whole scan if last scanner fullfilled and its end_key_ equals to scan_range's end_key_
       */
      TBSYS_LOG(DEBUG, "biubiu. seamless_result_count_=%ld, start_key=%s, end_key=%s",
          seamless_result_count_,
          to_cstring(sharding_result_arr_[seamless_result_count_-1].sharding_query_range_->start_key_),
          to_cstring(sharding_result_arr_[seamless_result_count_-1].sharding_query_range_->end_key_));
      is_finish = true;
    }
    else
    {
      is_finish = false;
    }
  }
}

int oceanbase::mergeserver::ObMsSqlSortedOperator::sort_fullfilled_result(bool &is_finish, ObNewScanner *last_sharding_res/*=NULL*/)
{
  /*
   * 1. sort unread scanners
   * 2. check if first sorted unread scanner's start key equals to last seamed end key
   * 3. if seamed, check if finish
   * 4. update sort params
   *
   * sharding_result_count_: total scanner received
   * cur_sharding_result_idx_: current readable scanner index
   * seamless_result_count_: seamed scanners
   *
   * cur_fullfilled_result_idx_:current fullfilled tablet scan range Index
   * seamless_fullfilled_result_count_:seamed fullfilled tablet scan range
   */
  int ret = OB_SUCCESS;
  int64_t seamless_result_idx = 0;
  // sort 'clean' scanners
  sharding_result_t *first = &fullfilled_result_arr_.at(0);
  sharding_result_t *start = &fullfilled_result_arr_.at(cur_fullfilled_result_idx_);
  int64_t count = fullfilled_result_arr_.count();
  std::sort(start, first+count);//first+count is end ptr

  if (seamless_fullfilled_result_count_ <= 0)
  {
    // no seamless result
    if ( //0 == fullfilled_result_arr_[0].sharding_query_range_->compare_with_startkey2(scan_range_.start_key_)
        (fullfilled_result_arr_[0].sharding_query_range_->start_key_  == scan_range_.start_key_)
           || (fullfilled_result_arr_[0].sharding_query_range_->start_key_.is_min_row()))
    {
      TBSYS_LOG(DEBUG,"first scan range has reached ms,total request scan range[%s],fullfilled_result_scan_range[%s]",
                       to_cstring(scan_range_),to_cstring(*(fullfilled_result_arr_[0].sharding_query_range_)));
      seamless_fullfilled_result_count_ = 1;
    }
  }

  if (seamless_fullfilled_result_count_ > 0)
  {
    // at lease one seamless result
    for (seamless_result_idx = seamless_fullfilled_result_count_;
         seamless_result_idx < fullfilled_result_arr_.count();
         seamless_result_idx++)
    {
      if (fullfilled_result_arr_[seamless_result_idx - 1].last_row_key_
            == fullfilled_result_arr_[seamless_result_idx].sharding_query_range_->start_key_)
      {
        seamless_fullfilled_result_count_ = seamless_result_idx + 1;
        TBSYS_LOG(DEBUG, "prev last_row_key=%s, cur star_key_=%s, seamless=%ld",
                        to_cstring(fullfilled_result_arr_[seamless_result_idx - 1].last_row_key_),
                        to_cstring(fullfilled_result_arr_[seamless_result_idx].sharding_query_range_->start_key_),
                        seamless_fullfilled_result_count_);
      }
      else
      {
        break;
      }
    }
  }

  if (seamless_fullfilled_result_count_ > 0) // implicates that startkey matched already
  {
    OB_ASSERT(NULL != last_sharding_res);
    TBSYS_LOG(DEBUG, "last seamless last rowkey=%s,sharding_query_range_[%s],seamless_fullfilled_result_count_[%ld]",
                      to_cstring(fullfilled_result_arr_[seamless_fullfilled_result_count_-1].last_row_key_),
                      to_cstring(*fullfilled_result_arr_[seamless_fullfilled_result_count_-1].sharding_query_range_),
                      seamless_fullfilled_result_count_);

    //统计可以返回给上层算子的scanner数,即统计readable的scanner数目
    //比如,若当前第1、2个tablet的scanner已经全部返回(即为接收到了对应tablet_range的两个fullfilled为true的scannner),
    //则将sharding_result_arr进行排序,比较range,统计可以返回给上层算子的scanner数-->seamless_result_count_
    //sort total scanner to make sure return scanners belongs to this seamed fullfilled tablet scan range
    sharding_result_t *first_scanner = &sharding_result_arr_.at(0);
    sharding_result_t *start_scanner = &sharding_result_arr_.at(cur_sharding_result_idx_);
    int64_t total_scanner_count = sharding_result_arr_.count();
    std::sort(start_scanner, first_scanner+total_scanner_count);

    int64_t total_seamless_count = 0;
    for(int64_t tablet_count = cur_fullfilled_result_idx_ ;
                tablet_count < seamless_fullfilled_result_count_ ; tablet_count++)
    {
      TBSYS_LOG(DEBUG,"seamless tablet_range[%s],cur_fullfilled_result_idx[%ld]",
                      to_cstring(*(fullfilled_result_arr_[tablet_count].sharding_query_range_)),cur_fullfilled_result_idx_);
      while(total_seamless_count < total_scanner_count)
      {
        if(sharding_result_arr_[total_seamless_count].sharding_query_range_->start_key_
             >= fullfilled_result_arr_[tablet_count].sharding_query_range_->end_key_)
        {
          break;
        }
        else
        {
          total_seamless_count++;
        }
      }
      cur_fullfilled_result_idx_++;
    }
    TBSYS_LOG(DEBUG,"seamless_count_total[%ld],sharding_arr_count[%ld]",total_seamless_count,sharding_result_arr_.count());
    seamless_result_count_ = total_seamless_count;

    //if (fullfilled_result_arr_[seamless_fullfilled_result_count_-1].last_row_key_ >= scan_range_.end_key_ ||
    //    fullfilled_result_arr_[seamless_fullfilled_result_count_-1].last_row_key_.is_max_row())
    if(fullfilled_result_arr_[seamless_fullfilled_result_count_-1].sharding_query_range_->end_key_ >= scan_range_.end_key_
        || fullfilled_result_arr_[seamless_fullfilled_result_count_-1].sharding_query_range_->end_key_.is_max_row())
    {
      /* check last seemless result.
       * Finish the whole scan if last scanner fullfilled and its end_key_ equals to scan_range's end_key_
       */
      TBSYS_LOG(DEBUG, "biubiu. seamless_result_count_=%ld, start_key=%s, end_key=%s",
          seamless_fullfilled_result_count_,
          to_cstring(fullfilled_result_arr_[seamless_fullfilled_result_count_-1].sharding_query_range_->start_key_),
          to_cstring(fullfilled_result_arr_[seamless_fullfilled_result_count_-1].sharding_query_range_->end_key_));
      is_finish = true;

      if(total_seamless_count < sharding_result_arr_.count())
      {
        ret = OB_ERR_UNEXPECTED;
        TBSYS_LOG(ERROR,"can not happen,not all scanner has been processed,"\
                        "total_seamless_count[%ld] is less than total_scanner_count[%ld],ret=%d",
                        total_seamless_count,sharding_result_arr_.count(),ret);
      }
    }
    else
    {
      is_finish = false;
    }
  }
  return ret;
}


int oceanbase::mergeserver::ObMsSqlSortedOperator::add_sharding_result(ObNewScanner & sharding_res, const ObNewRange &query_range,
  bool &is_finish, ObStringBuf &rowkey_buffer)
{
  bool is_fullfilled = false;
  int64_t fullfilled_item_num = 0;
  ObRowkey last_row_key;
  ObRowkey stored_rowkey;
  ObNewRange cs_tablet_range;
  int err = OB_SUCCESS;

  if ((OB_SUCCESS == err) && (NULL == scan_param_))
  {
    TBSYS_LOG(WARN,"operator was not initialized yet [scan_param_:%p]", scan_param_);
    err = OB_INVALID_ARGUMENT;
  }
  else if (OB_SUCCESS != (err = sharding_res.get_is_req_fullfilled(is_fullfilled,fullfilled_item_num)))
  {
    TBSYS_LOG(WARN,"fail to get fullfilled info from sharding result [err:%d]", err);
  }

  //add jinty [sort in cs bugfix] 20180324:b//TODO::is necessary?
  bool is_sort_in_cs = false;
  if(OB_SUCCESS == err)
  {
    if((scan_param_->has_group()) && (scan_param_->has_scalar_agg()))
    {
      err = OB_INVALID_ARGUMENT;
      TBSYS_LOG(WARN,"Group operator and scalar aggregate operator"\
                     "can not appear in TabletScan at the same time.err=%d",err);
    }
    else if((scan_param_->has_group()) && !(scan_param_->has_group_columns_sort()))
    {
      err = OB_INVALID_ARGUMENT;
      TBSYS_LOG(WARN,"Physical plan error, group need a sort operator. err=%d",err);
    }
    is_sort_in_cs = (scan_param_->has_group());// || scan_param->has_lisagg();
  }
  //add:e

  // prepare last row key
  if (OB_SUCCESS == err)
  {
    if (true == is_fullfilled)
    {
      // last_row_key has to be modified
      if (OB_SUCCESS != (err = sharding_res.get_range(cs_tablet_range)))
      {
        TBSYS_LOG(WARN,"fail to get tablet range from sharding result [err:%d]", err);
      }
      else
      {

        last_row_key = (cs_tablet_range.end_key_ < query_range.end_key_) ? cs_tablet_range.end_key_ : query_range.end_key_;
        if (last_row_key.is_max_row())
        {
          TBSYS_LOG(DEBUG, "got max");
        }
        else
        {
          TBSYS_LOG(DEBUG, "got normal: last=%s. endkey=%s", to_cstring(last_row_key), to_cstring(cs_tablet_range.end_key_));
        }
      }
    }
    else if (OB_SUCCESS != (err = sharding_res.get_last_row_key(last_row_key)))
    {
      if (OB_ENTRY_NOT_EXIST == err)
      {
        TBSYS_LOG(WARN, "last row key not exist");
        err = OB_SUCCESS;
      }
      else
      {
        TBSYS_LOG(WARN,"fail to get last rowkey from sharding result [err:%d]", err);
      }
    }
  }

  if (OB_SUCCESS == err)
  {
    if (OB_SUCCESS != (err = rowkey_buffer.write_string(last_row_key, &stored_rowkey)))
    {
      TBSYS_LOG(WARN, "fail to store rowkey to buffer. %s", to_cstring(cs_tablet_range.end_key_));
    }
    else
    {
      fullfilled_item_num = sharding_res.get_row_num();
      total_mem_size_used_ += sharding_res.get_used_mem_size();
      sharding_result_t sr;
      sr.init(sharding_res,query_range,*scan_param_, stored_rowkey, fullfilled_item_num);
      if (OB_SUCCESS != (err = sharding_result_arr_.push_back(sr)))
      {
        TBSYS_LOG(WARN, "failed to add sharding result, err=%d", err);
      }
      //add jinty [sort in cs bugfix] 20180324:b
      else if(is_fullfilled && is_sort_in_cs &&
                OB_SUCCESS != (err = fullfilled_result_arr_.push_back(sr)))
      {
        TBSYS_LOG(WARN,"failed to add fullfilled result scanner,err=%d,",err);
      }
      //add:e
    }
  }

  is_finish = false;
  //mod jinty [sort in cs bugfix] 20180324:b
  //if (OB_SUCCESS == err)
  //{
  //  sort(is_finish, &sharding_res);
  //}
  TBSYS_LOG(DEBUG,"is sort in cs ? [%s],total_request_query_range[%s]",is_sort_in_cs ? "YES":"NO",to_cstring(query_range));
  if(OB_SUCCESS == err && is_sort_in_cs)
  {
    if(is_fullfilled)
    {
      if(OB_SUCCESS != (err = sort_fullfilled_result(is_finish,&sharding_res)))
      {
        TBSYS_LOG(ERROR,"sort fullfiled result failed,err=%d",err);
      }
    }
  }
  else if(OB_SUCCESS == err && !is_sort_in_cs)
  {
    sort(is_finish, &sharding_res);
  }
  //mod:e
  TBSYS_LOG(DEBUG, "add sharding result. sharding_result_count_=%ld, is_finish=%d, err=%d", sharding_result_arr_.count(), is_finish, err);

  if (1)
  {
    static __thread int64_t total_row_num = 0;
    total_row_num += fullfilled_item_num;
    TBSYS_LOG(DEBUG, "last_rowkey=%s,query_range=%s,cs_tablet_range=%s,is_fullfilled=%d,fullfilled_item_num=%ld,total=%ld, is_finish=%d",
        to_cstring(last_row_key),to_cstring(query_range),to_cstring(cs_tablet_range),
        is_fullfilled,fullfilled_item_num,total_row_num,is_finish);
  }
  return err;
}


///////////////////////////////////////
//////////// Row inferface ////////////
///////////////////////////////////////

int oceanbase::mergeserver::ObMsSqlSortedOperator::get_next_row(oceanbase::common::ObRow &row)
{
  int err = OB_SUCCESS;
  while (OB_SUCCESS == err)
  {
    if (cur_sharding_result_idx_ >= seamless_result_count_)
    {
      err = OB_ITER_END;
    }
    else if (OB_SUCCESS == err)
    {
      if (OB_SUCCESS ==(err = sharding_result_arr_[cur_sharding_result_idx_].sharding_res_->get_next_row(row)))
      {
        break;
      }
      else if (OB_ITER_END == err)
      {
        total_mem_size_used_ -= sharding_result_arr_[cur_sharding_result_idx_].sharding_res_->get_used_mem_size();
        // since this sharding will never be used again, release it!
        sharding_result_arr_[cur_sharding_result_idx_].sharding_res_->clear();
        cur_sharding_result_idx_ ++;
        err = OB_SUCCESS;
      }
      else
      {
        TBSYS_LOG(WARN,"fail to get next cell from ObNewScanner [idx:%ld,err:%d]", cur_sharding_result_idx_, err);
      }
    }
  }
  return err;
}
