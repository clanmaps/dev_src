/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_black_list.h
 * @brief   This is a black list for building partitional or global Index
 * when a cs build a index failed,then push this tablet/range in BlackList and send list to other cs
 * another cs recieved the list to redo index mission, if failed, send list to third cs
 * in the end of phase, cs found that all replication are failed,then the index create failed.
 *
 * Created by longfei：for re-try building secondary index
 * WARNING:
 * Now index building will only handle 3 replication of data;
 * that is failed in 3 replicate, this index build failed;
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @date 2015_12_10
 */

#ifndef COMMON_OB_INDEX_BLACK_LIST_H_
#define COMMON_OB_INDEX_BLACK_LIST_H_

#include "ob_range2.h"
#include "ob_server.h"
#include "ob_define.h"
#include "ob_array.h"
#include "page_arena.h"
#include "location/ob_tablet_location_list.h"
#include <tbsys.h>
#include <Mutex.h>
#include <Monitor.h>

namespace oceanbase
{
  namespace common
  {
    static const int64_t OB_INDEX_HANDLE_REP = 3;
    /**
     * @brief The BlackList class
     * for deal with failed index
     */
    class BlackList
    {
    public:
      /**
       * @brief BlackList constructor
       */
      BlackList();
      /**
       * @brief ~BlackList destructor
       */
      ~BlackList();
      /**
       * @brief init
       * @return err code
       */
      int init();
      /**
       * @brief add_in_black_list
       * @param range
       * @param server
       * @return err code
       */
      int add_in_black_list(ObNewRange &range, ObServer &server);
      /**
       * @brief is_all_repli_failed
       * @return err code
       */
      bool is_all_repli_failed();
      /**
       * @brief get_range
       * @param range
       */
      void get_range(ObNewRange &range);
      /**
       * @brief set_range
       * @param range
       */
      void set_range(ObNewRange &range);
      /**
       * @brief get_server
       * @param i
       * @param server
       * @return err code
       */
      int get_server(int i, ObServer &server);
      /**
       * @brief get_server_count
       * @return
       */
      int8_t get_server_count();
      /**
       * @brief set_server_unserved
       * @param server
       */
      void set_server_unserved(ObServer server);
      /**
       * @brief write_list
       * @param range
       * @param list
       * @return err code
       */
      int write_list(ObNewRange &range, ObTabletLocationList &list);
      /**
       * @brief next_replic_server
       * @param server
       * @return err code
       */
      int next_replic_server(ObServer &server);
      /**
       * @brief set_wok_send
       * @param value
       */
      void set_work_send(int8_t value = 1)
      {
        work_send_ = value;
      }
      /**
       * @brief get_work_send
       * @return err code
       */
      int8_t get_work_send()
      {
        return work_send_;
      }
      NEED_SERIALIZE_AND_DESERIALIZE;
    private:
      /**
       * @brief set_rowkey_obj_array
       * @param buf
       * @param buf_len
       * @param pos
       * @param array
       * @param size
       * @return
       */
      int set_rowkey_obj_array(
          char* buf,
          const int64_t buf_len,
          int64_t & pos,
          const ObObj* array,
          const int64_t size);
      /**
       * @brief set_int_obj_value
       * @param buf
       * @param buf_len
       * @param pos
       * @param value
       * @return
       */
      int set_int_obj_value(
          char * buf,
          const int64_t buf_len,
          int64_t & pos,
          const int64_t value);
      /**
       * @brief get_rowkey_compatible
       * @param buf
       * @param buf_len
       * @param pos
       * @param array
       * @param size
       * @return
       */
      int get_rowkey_compatible(
          const char* buf,
          const int64_t buf_len,
          int64_t & pos,
          ObObj* array,
          int64_t& size);
    private:
      ObNewRange range_; ///< faild range or tablet's range
      ObObj array_[OB_MAX_ROWKEY_COLUMN_NUMBER * 2];
      int8_t server_count_; ///< the number of cs
      ObServer server_[OB_INDEX_HANDLE_REP]; ///< cs
      int8_t unserved_[OB_INDEX_HANDLE_REP]; ///< flag of cs, if has been dealt with,set to 1
      int8_t work_send_; ///<
    };

    /**
     * @brief The BlackListArray class
     */
    class BlackListArray
    {
    public:
      BlackListArray();
      ~BlackListArray();
      int init();
      int push(BlackList &list);
      int64_t get_list_count();
      /*only provide an interface,not used until now*/
      BlackList& get_black_list(int64_t i, int &err);
      int get_next_black_list(BlackList &list);
      int check_range_in_list(ObNewRange &range, bool &in, int64_t &index);
      bool is_list_array_bleach();
      void reset();

    private:
      pthread_mutex_t mutex_;
      ObArray <BlackList> list_array_;
      ObArray <int8_t> list_bleach_;
      CharArena arena_;
      BlackList error_;

    };


  } /* namespace common */
} /* namespace oceanbase */

#endif /* COMMON_OB_INDEX_BLACK_LIST_H_ */
