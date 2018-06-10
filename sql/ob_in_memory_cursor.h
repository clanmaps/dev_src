/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_in_memory_cursor.h
* @brief this class  is a cursor helper implement
*
* Created by zhounan: support cursor
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/

#ifndef _OB_IN_MEMORY_CURSOR_H
#define _OB_IN_MEMORY_CURSOR_H 1
#include "common/ob_array.h"
#include "common/ob_string.h"
#include "common/ob_object.h"
#include "common/ob_row.h"
#include "ob_cursor_helper.h"
#include "common/ob_row_store.h"

namespace oceanbase
{
  namespace sql
  {

    /**
     * @brief The ObInMemoryCursor class
     */
    class ObInMemoryCursor: public ObCursorHelper
    {
      public:
        ObInMemoryCursor();
        virtual ~ObInMemoryCursor();
        virtual void reset();
        virtual void reuse();
        int add_row(const common::ObRow &row);
        virtual int get_next_row(const common::ObRow *&row);
        int get_next_compact_row(common::ObString &compact_row);
        int get_next_row(common::ObRow &row);
        const common::ObRowDesc* get_row_desc() const;
        void set_pos(int64_t pos);
        void inc_pos();
        void dec_pos();

        int64_t get_row_count() const;
        int64_t get_used_mem_size() const;
      private:
        // disallow copy
        ObInMemoryCursor(const ObInMemoryCursor &other);
        ObInMemoryCursor& operator=(const ObInMemoryCursor &other);
      private:
        // data members
        common::ObRowStore row_store_;
        common::ObArray<const common::ObRowStore::StoredRow*> cursor_array_;
        int64_t cursor_array_get_pos_;
        common::ObRow curr_row_;
        const common::ObRowDesc *row_desc_;
    };

    inline const common::ObRowDesc* ObInMemoryCursor::get_row_desc() const
    {
      return row_desc_;
    }

    inline void ObInMemoryCursor::set_pos(int64_t pos)
     {
        cursor_array_get_pos_ = pos;
     }

    inline void ObInMemoryCursor::inc_pos()
     {
        cursor_array_get_pos_++;
     }

    inline void ObInMemoryCursor::dec_pos()
     {
        cursor_array_get_pos_--;
     }
 

  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_IN_MEMORY_CURSOR_H */
