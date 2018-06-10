/* (C) 2010-2012 Alibaba Group Holding Limited.
 *
 * This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  version 2 as published by the Free Software Foundation.
 *
 * Version: 0.1
 *
 * Authors:
 *    Wu Di <lide.wd@alipay.com>
 */

#ifndef OB_LIBONEV_MEM_POOL_H
#define OB_LIBONEV_MEM_POOL_H

#include "ob_malloc.h"
namespace oceanbase
{
  namespace common
  {
    void* ob_onev_realloc(void *ptr, size_t size);
  }
}

#endif
