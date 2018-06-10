/**
 * (C) 2010-2011 Alibaba Group Holding Limited.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * 
 * Version: $Id$
 *
 * ob_onev_array.h
 *
 * Authors:
 *   Junquan Chen <jianming.cjq@taobao.com>
 *
 */

#ifndef _OB_ONEV_ARRAY_H
#define _OB_ONEV_ARRAY_H

#include "common/ob_array.h"
#include "common/ob_object.h"

namespace oceanbase
{
  namespace common
  {
    template<class T>
    class OnevArray
    {
    public:
      OnevArray();
      OnevArray(T element);

      //重载()来支持 OnevArray("name")("value")("info")这样的使用方式
      OnevArray& operator()(T element);
      int at(int32_t index, T& element) const;
      int64_t count() const; //列名的个数

      inline int get_exec_status() const
      {
        return exec_status_;
      }

    private:
      ObArray<T> array_;
      int exec_status_;

    };

    template<class T>
    OnevArray<T>::OnevArray() : exec_status_(OB_SUCCESS)
    {
    }

    template<class T>
    OnevArray<T>::OnevArray(T element)
    {
      exec_status_ = OB_SUCCESS;
      
      if(OB_SUCCESS == exec_status_)
      {
        exec_status_ = array_.push_back(element);
        if(OB_SUCCESS != exec_status_)
        {
          TBSYS_LOG(WARN, "push back fail:exec_status_[%d]", exec_status_);
        }
      }
    }

    template<class T>
    OnevArray<T>& OnevArray<T>::operator()(T element)
    {
      if(OB_SUCCESS == exec_status_)
      {
        exec_status_ = array_.push_back(element);
        if(OB_SUCCESS != exec_status_)
        {
          TBSYS_LOG(WARN, "push back fail:exec_status_[%d]", exec_status_);
        }
      }
      return *this;
    }

    template<class T>
    int OnevArray<T>::at(int32_t index, T& element) const
    {
      int ret = OB_SUCCESS;
      ret = array_.at(index, element);
      return ret;
    }

    template<class T>
    int64_t OnevArray<T>::count() const
    {
      return array_.count();
    }
  }
}

#endif /* _OB_ONEV_ARRAY_H */


