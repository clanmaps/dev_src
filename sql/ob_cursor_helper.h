/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_cursor_helper.h
* @brief this class  is cursor helper
*
* Created by zhounan: support curosr
*
* @version CEDAR 0.2 
* @author zhounan <zn4work@gmail.com>
* @date 2014_11_23
*/
#ifndef _OB_CURSOR_HELPER_H
#define _OB_CURSOR_HELPER_H 1
#include "common/ob_row.h"
namespace oceanbase
{
  namespace sql
  {
    /**
     * @brief The ObCursorHelper class
     */
    class ObCursorHelper
    {
      public:
        ObCursorHelper(){}
        virtual ~ObCursorHelper(){}
        virtual int get_next_row(const common::ObRow *&row) = 0;
      private:
        // disallow copy
        ObCursorHelper(const ObCursorHelper &other);
        ObCursorHelper& operator=(const ObCursorHelper &other);
    };
  } // end namespace sql
} // end namespace oceanbase

#endif /* _OB_CURSOR_HELPER_H */

