/*
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *
 * Version: 0.1: ob_string_buf.cpp,v 0.1 2010/08/19 16:19:47 chuanhui Exp $
 *
 * Authors:
 *   chuanhui <rizhao.ych@taobao.com>
 *     - some work details if you want
 *
 */

#include "ob_string_buf.h"
#include "common/ob_object.h"
#include "common/ob_malloc.h"

namespace oceanbase
{
  namespace common
  {
    template <typename PageAllocatorT, typename PageArenaT>
    const int64_t ObStringBufT<PageAllocatorT, PageArenaT>::DEF_MEM_BLOCK_SIZE = 64 * 1024L;
    template <typename PageAllocatorT, typename PageArenaT>
    const int64_t ObStringBufT<PageAllocatorT, PageArenaT>::MIN_DEF_MEM_BLOCK_SIZE = OB_COMMON_MEM_BLOCK_SIZE;

    template <typename PageAllocatorT, typename PageArenaT>
    ObStringBufT<PageAllocatorT, PageArenaT>::ObStringBufT(const int32_t mod_id /*=0*/,
                                                           const int64_t block_size /*= DEF_MEM_BLOCK_SIZE*/)
      :local_arena_(block_size < MIN_DEF_MEM_BLOCK_SIZE ? MIN_DEF_MEM_BLOCK_SIZE : block_size, PageAllocatorT(mod_id)),
       arena_(local_arena_)
    {
    }

    template <typename PageAllocatorT, typename PageArenaT>
    ObStringBufT<PageAllocatorT, PageArenaT>::ObStringBufT(PageArenaT &arena)
      : arena_(arena)
    {
    }

    template <typename PageAllocatorT, typename PageArenaT>
    ObStringBufT<PageAllocatorT, PageArenaT> :: ~ObStringBufT()
    {
      clear();
    }

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: clear()
    {
      local_arena_.free();
      return OB_SUCCESS;
    }

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: reset()
    {
      local_arena_.reuse();
      return OB_SUCCESS;
    }

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: reuse()
    {
      local_arena_.reuse();
      return OB_SUCCESS;
    }

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: write_string(const ObString& str, ObString* stored_str)
    {
      int err = OB_SUCCESS;

      if (OB_UNLIKELY(0 == str.length() || NULL == str.ptr()))
      {
        if (NULL != stored_str)
        {
          stored_str->assign(NULL, 0);
        }
      }
      else
      {
        int64_t str_length = str.length();
        char* str_clone = arena_.dup(str.ptr(), str_length);
        if (OB_UNLIKELY(NULL == str_clone))
        {
          err = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(WARN, "failed to dup string");
        }
        else if (NULL != stored_str)
        {
          stored_str->assign(str_clone, static_cast<int32_t>(str_length));
        }
      }

      return err;
    }
    class ObRowkey;
    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: write_string(const ObRowkey& rowkey, ObRowkey* stored_rowkey)
    {
      int err = OB_SUCCESS;
      if (0 == rowkey.length() || NULL == rowkey.ptr())
      {
        if (NULL != stored_rowkey)
        {
          stored_rowkey->assign(NULL, 0);
        }
      }
      else
      {
        int64_t str_length = rowkey.get_deep_copy_size();
        char* buf = arena_.alloc(str_length);
        if (OB_UNLIKELY(NULL == buf))
        {
          err = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(WARN, "no memory");
        }
        else
        {
          ObRawBufAllocatorWrapper allocator(buf, str_length);
          if (NULL != stored_rowkey)
          {
            rowkey.deep_copy(*stored_rowkey, allocator);
          }
        }
      }
      return err;
    }
    //add xsl ECNU_DECIMAL 2016_12

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT>::write_decimal(const ObDecimal& dec, ObDecimal *stored_dec,uint32_t len)  //在堆上分配一块内存，并且将值copy下来
    {
      int err = OB_SUCCESS;
      if (OB_UNLIKELY(NULL == const_cast<ObDecimal&>(dec).get_words()))
      {
        if (NULL != stored_dec)
        {
          stored_dec=NULL;
        }
      }
      else
      {
        uint64_t* str_clone = reinterpret_cast<uint64_t *>(arena_.alloc(sizeof(uint64_t)*len));
        if (OB_UNLIKELY(NULL == str_clone))
        {
          err = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(WARN, "failed to dup string");
        }
        memcpy(str_clone,const_cast<uint64_t *>(const_cast<ObDecimal&>(dec).get_words()->ToUInt_v2()),sizeof(uint64_t)*len);
        stored_dec->set_precision(dec.get_precision());
        stored_dec->set_scale(dec.get_scale());
        stored_dec->set_vscale(dec.get_vscale());
        stored_dec->set_word(str_clone,len);   //使得decimal指针指向这块内存
      }
      return err;
    }

    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT>::write_decimal(const uint64_t* dec, uint64_t*& stored_dec,uint32_t len)  //在堆上分配一块内存，并且将值copy下来
    {
      int err = OB_SUCCESS;
      if (OB_UNLIKELY(NULL == dec))
      {
        if (NULL != stored_dec)
        {
          stored_dec=NULL;
        }
      }
      else
      {
        char* str_clone = arena_.alloc(sizeof(uint64_t)*len);
        if (OB_UNLIKELY(NULL == str_clone))
        {
          err = OB_ALLOCATE_MEMORY_FAILED;
          TBSYS_LOG(WARN, "failed to dup string");
        }
        stored_dec=reinterpret_cast<uint64_t *>(str_clone);   //使得decimal指针指向这块内存
        memcpy(stored_dec,const_cast<uint64_t *>(dec),sizeof(uint64_t)*len);
      }
      return err;
    }
    //add e
    template <typename PageAllocatorT, typename PageArenaT>
    int ObStringBufT<PageAllocatorT, PageArenaT> :: write_obj(const ObObj& obj, ObObj* stored_obj)
    {
      int err = OB_SUCCESS;
      if (NULL != stored_obj)
      {
        *stored_obj = obj;
      }
      ObObjType type = obj.get_type();
      if (ObVarcharType == type)
      {
        ObString value;
        ObString new_value;
        obj.get_varchar(value);
        err = write_string(value, &new_value);
        if (OB_SUCCESS == err)
        {
          if (NULL != stored_obj)
          {
            stored_obj->set_varchar(new_value);
          }
        }
      }
      //modify xsl ECNU_DECIMAL 2016_12
      //add fanqiushi DECIMAL OceanBase_BankCommV0.3 2014_7_19:b
      else if (ObDecimalType == type)
      {
        uint64_t *src_val = NULL;
        uint64_t *dst_val = NULL;
        src_val = obj.get_ttint();
        err = write_decimal(src_val, dst_val,obj.get_nwords());     //在内存分配器中分配一块内存并赋值，使得decimal指针指向它.
        if (OB_SUCCESS == err)
        {
          if (NULL != stored_obj &&  dst_val !=NULL)
          {
            stored_obj->set_decimal(dst_val,obj.get_precision(),obj.get_scale(),obj.get_vscale(),obj.get_nwords());   //modify xsl ECNU_DECIMAL 2017_2
          }
        }
      }
      //add e
      //modify e
      return err;
    }


  }
}
