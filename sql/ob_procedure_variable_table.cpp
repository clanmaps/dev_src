/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_variable_table.cpp
 * @brief procedure varoable table definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "common/ob_define.h"
#include "ob_procedure_variable_table.h"

using namespace oceanbase::sql;

int ObProcedureVariableTable::create_variable(const ObString &var_name, const ObObjType type)
{
  int ret = OB_SUCCESS;
  ObString var;
  ObObj tmp;
  tmp.set_type(type);
  if( OB_SUCCESS != (ret = name_pool_.write_string(var_name, &var)))
  {}
  else if( hash::HASH_INSERT_SUCC == var_name_val_map_.set(var_name, tmp) )
  {}
  else
  {
    ret = OB_ERROR; //variable existed;
  }
  return ret;
}

int ObProcedureVariableTable::create_array(const ObString &array, const ObObjType type)
{
  int ret = OB_SUCCESS;
  int64_t count = array_table_.count();
  ObString tab;
  ObObj idx;

  idx.set_int(count);
  if(OB_SUCCESS != (ret = name_pool_.write_string(array, &tab)))
  {}
  else if( hash::HASH_INSERT_SUCC == var_name_val_map_.set(tab, idx))
  {
    ObProcedureArray tmp_array;
    ObObj obj_type;
    obj_type.set_type(type);
    tmp_array.push_back(obj_type);
    array_table_.push_back(tmp_array);
  }
  else
  {
    ret = OB_ERROR; //variable existed;
  }
  return ret;
}

int ObProcedureVariableTable::write(const ObString &var_name, const ObObj &val)
{
  int ret = OB_SUCCESS;
  ObString tmp_var;
  ObObj tmp_val;

  if (var_name.length() <= 0)
  {
    ret = OB_ERROR;
  }
  else if ( OB_SUCCESS !=  (ret = name_pool_.write_obj(val, &tmp_val))
           || ((ret = var_name_val_map_.set(tmp_var, tmp_val, 1)) != hash::HASH_OVERWRITE_SUCC))
  {
    ret = OB_ERROR;
  }
  else
  {
    TBSYS_LOG(TRACE, "write variable %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(val));
    ret = OB_SUCCESS;
  }
  return ret;
}

int ObProcedureVariableTable::write(const ObString &array_name, int64_t idx_value, const ObObj &val)
{
  int ret = idx_value >= 0 ? OB_SUCCESS : OB_ERR_ILLEGAL_INDEX;
  const ObObj *array_idx_obj;

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = read(array_name, array_idx_obj)) )
  {}
  else
  {
    int64_t array_idx = -1;
    array_idx_obj->get_int(array_idx);
    ObProcedureArray &array = array_table_.at(array_idx);

    if( idx_value >= array.count() )
    {
      while( OB_SUCCESS == ret && idx_value >= array.count() )
      {
        ObObj tmp;
        tmp.set_null();
        ret = array.push_back(tmp);
      }
    }
    if( OB_SUCCESS == ret &&
        OB_SUCCESS == (ret = name_pool_.write_obj(val, &array.at(idx_value))) )
    {
    }
  }
  return ret;
}

int ObProcedureVariableTable::write(const ObString &array_name, const ObIArray<ObObj> &other)
{
  int ret = OB_SUCCESS;
  const ObObj *array_idx_obj;
  ObObj tmp;

  if( OB_SUCCESS != (ret = read(array_name, array_idx_obj)) )
  {}
  else
  {
    int64_t array_idx = -1;
    array_idx_obj->get_int(array_idx);
    ObProcedureArray &array = array_table_.at(array_idx);

    array.clear();
    for(int64_t i = 0; OB_SUCCESS == ret && i < other.count(); ++i)
    {
      if( OB_SUCCESS != (ret = name_pool_.write_obj(other.at(i), &tmp)) )
      {}
      else if( OB_SUCCESS != (ret = array.push_back(tmp)) )
      {}
    }
  }
  return ret;
}

int ObProcedureVariableTable::read(const ObString &var_name, const ObObj *&val) const
{
  int ret = OB_SUCCESS;
  if ((val=var_name_val_map_.get(var_name)) == NULL)
  {
    ret = OB_ERR_VARIABLE_UNKNOWN;
  }
  else
  {
    TBSYS_LOG(TRACE, "read var %.*s = %s", var_name.length(), var_name.ptr(), to_cstring(*val));
  }
  return ret;
}

int ObProcedureVariableTable::read(const ObString &array_name, int64_t idx, const ObObj *&val) const
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = read(array_name, val)))
  {}
  else
  {
    int64_t i = -1;
    val->get_int(i);
    ret = (i < array_table_.count() && i >= 0) ? OB_SUCCESS : OB_ERR_ILLEGAL_INDEX;

    const ObProcedureArray &arr = array_table_.at(i);
    if( OB_SUCCESS == ret && idx >= 0 && idx < arr.count() )
    {
      val = & (arr.at(idx));
    }
    else
    {
      TBSYS_LOG(WARN, "array index is invalid, %ld", idx);
      ret = OB_ERR_ILLEGAL_INDEX;
    }
  }
  return ret;
}

int ObProcedureVariableTable::read(const ObString &array_name, const ObIArray<ObObj> *&array) const
{
  int ret = OB_SUCCESS;
  const ObObj *val = NULL;
  if( OB_SUCCESS != (ret = read(array_name, val)) )
  {}
  else
  {
    int64_t i = -1;
    val->get_int(i);
    ret = (i < array_table_.count() && i >= 0) ? OB_SUCCESS : OB_ERR_ILLEGAL_INDEX;

    array = &(array_table_.at(i));
  }
  return ret;
}
