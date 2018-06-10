/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor.h
* @brief this class  present a cursor physical plan in oceanbase
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef _OB_CURSOR_H
#define _OB_CURSOR_H 1
#include "ob_single_child_phy_operator.h"
#include "common/ob_string.h"
#include "common/ob_array.h"
#include "ob_in_memory_cursor.h"
#include "ob_merge_cursor.h"
#include "sql/ob_basic_stmt.h"
#include "ob_sql_session_info.h"
namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObCursor class
     */
    class ObCursor: public ObSingleChildPhyOperator
    {
      public:
        ObCursor();
        virtual ~ObCursor();
        virtual void reset();
        virtual void reuse();
        void set_mem_size_limit(const int64_t limit);
        int set_run_filename(const common::ObString &filename);
        int get_run_file();
        virtual int open();
        virtual int close();
        virtual int get_next_row(const common::ObRow *&row);
        virtual int get_row_desc(const common::ObRowDesc *&row_desc) const;
        virtual int64_t to_string(char* buf, const int64_t buf_len) const;
        virtual ObPhyOperatorType get_type() const;
        /**
         * @brief get_mem_size_limit
         * @return
         */
        int64_t get_mem_size_limit() const;
        /**
         * @brief inc_pos
         */
        void inc_pos();
        /**
         * @brief dec_pos
         */
        void dec_pos();
        /**
         * @brief set_is_opened
         * @param is_opened
         */
        void set_is_opened(bool is_opened);
        /**
         * @brief get_is_opened
         * @return
         */
        bool get_is_opened();
        /**
         * @brief get_row_num
         * @return
         */
        int64_t get_row_num() const;
        /**
         * @brief get_ab_row
         * @param ab_num_
         * @param row
         * @return
         */
        int get_ab_row(int64_t ab_num_,const common::ObRow *&row);
        /**
         * @brief get_first_row
         * @param row
         * @return
         */
        int get_first_row(const common::ObRow *&row);
        /**
         * @brief get_last_row
         * @param row
         * @return
         */
        int get_last_row(const common::ObRow *&row);
        /**
         * @brief get_prior_row
         * @param row
         * @return
         */
        int get_prior_row(const common::ObRow *&row);
        /**
         * @brief get_rela_row
         * @param is_next
         * @param rela_count
         * @param row
         * @return
         */
        int get_rela_row(bool is_next,int64_t rela_count,const common::ObRow *&row);
        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
      private:
        // disallow copy
        ObCursor(const ObCursor &other);
        ObCursor& operator=(const ObCursor &other);
        // function members
        bool need_dump() const;
        int do_cursor();
      private:
        // data members
        bool is_opened_; ///> cursor open flag
        int64_t mem_size_limit_;///>memroy limit
        int64_t row_offset_;///>row offset
        int64_t row_count_;///>row count
        int64_t run_idx_;///>run file id
        int64_t row_num_;///> row number
        common::ObArray<int64_t> run_array_;
        ObInMemoryCursor in_mem_cursor_;
        ObMergeCursor merge_cursor_;
        ObCursorHelper *cursor_reader_;
    };

    inline int64_t ObCursor::get_mem_size_limit() const
    {
      return mem_size_limit_;
    }

    inline int64_t ObCursor::get_row_num() const
    {
      return row_num_;
    }
    inline void ObCursor::inc_pos()
    {
      in_mem_cursor_.inc_pos();
    }
    inline void ObCursor::set_is_opened(bool is_opened)
    {
       is_opened_ = is_opened;
    }
    inline bool ObCursor::get_is_opened()
    {
      return  is_opened_;
    }
    inline void ObCursor::dec_pos()
    {
      in_mem_cursor_.dec_pos();
    }
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_CURSOR_H */
