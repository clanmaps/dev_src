/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_stmt.cpp
 * @brief the ObProcedureStmt class definition that warp procedure statement
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#include "ob_procedure_stmt.h"
using namespace oceanbase::common;

namespace oceanbase
{
  namespace sql
  {
    void ObProcedureStmt::print(FILE* fp, int32_t level, int32_t index)
    {
      UNUSED(index);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureStmt %d begin>\n", index);
      //print_indentation(fp, level + 1);
      //fprintf(fp, "Expires Count = %d\n",(int32_t)var_val.);
      print_indentation(fp, level);
      fprintf(fp, "<ObProcedureStmt %d End>\n", index);
    }

    int ObProcedureStmt::set_proc_name(const ObString &proc_name)
    {
      proc_name_=proc_name;
      return OB_SUCCESS;
    }

    //add by wdh 20160705 :b
    int ObProcedureStmt::set_flag(const bool &flag)
    {
      is_declare_legal=flag;
      return OB_SUCCESS;
    }
    bool ObProcedureStmt::get_flag() const
    {
      return is_declare_legal;
    }
    //add :e

    int ObProcedureStmt::add_proc_param(const ObParamDef &proc_param)
    {
      int ret = OB_SUCCESS;
      for(int64_t i = 0; i < params_.count(); ++i)
      {
        if( 0 == params_.at(i).param_name_.compare(proc_param.param_name_) )
        {
          TBSYS_LOG(WARN, "%.*s conflict with other paramters", proc_param.param_name_.length(), proc_param.param_name_.ptr());
          ret = OB_ERR_SP_DUP_PARAM;
          break;
        }
      }
      if( OB_SUCCESS == ret )
      {
        ret = params_.push_back(proc_param);
      }
      return ret;
    }
    void ObProcedureStmt::delete_var()//add by wdh 20160714
    {
        declare_variable_.pop_back();
    }

    int ObProcedureStmt::add_declare_var(const ObString &var)
    {
      int ret = OB_SUCCESS;
      for (int64_t i = 0;i < declare_variable_.count();i++) //is there var_name conflict
      {
        if( 0 == declare_variable_.at(i).compare(var) )
        {
          TBSYS_LOG(WARN, "%.*s conflict with other variables", var.length(), var.ptr());
          ret = OB_ERR_SP_DUP_VAR;
          break;
        }
      }
      for(int64_t i = 0; i < params_.count(); ++i)
      {
        if( 0 == params_.at(i).param_name_.compare(var) )
        {
          TBSYS_LOG(WARN, "%.*s conflict with parameters", var.length(), var.ptr());
          ret = OB_ERR_SP_DUP_VAR;
          break;
        }
      }
      if(OB_SUCCESS == ret)
      {
        ret = declare_variable_.push_back(var);
      }
      return ret;
    }

    bool ObProcedureStmt::check_var_exist(const ObString &var) const
    {
      bool ret = false;
      for(int64_t i = 0; i < declare_variable_.count(); ++i)
      {
        if( 0 == declare_variable_.at(i).compare(var) )
        {
          TBSYS_LOG(WARN, "%.*s conflict with other variables", var.length(), var.ptr());
          ret = true;
          break;
        }
      }
      for(int64_t i = 0; i < params_.count(); ++i)
      {
        if( 0 == params_.at(i).param_name_.compare(var) )
        {
          TBSYS_LOG(WARN, "%.*s conflict with other parameters", var.length(), var.ptr());
          ret = true;
          break;
        }
      }
      return ret;
    }

    int ObProcedureStmt::add_stmt(uint64_t& stmt_id)
    {
      return proc_block_.push_back(stmt_id);
    }

    const ObString& ObProcedureStmt::get_proc_name() const
    {
      return proc_name_;
    }

    uint64_t ObProcedureStmt::get_stmt(int64_t index) const
    {
      return proc_block_.at(index);
    }

    const ObString& ObProcedureStmt::get_declare_var(int64_t index) const
    {
      return declare_variable_.at(index);
    }

    int64_t ObProcedureStmt::get_param_size() const
    {
      return params_.count();
    }

    int64_t ObProcedureStmt::get_stmt_size() const
    {
      return proc_block_.count();
    }

    int64_t ObProcedureStmt::get_declare_var_size() const
    {
      return declare_variable_.count();
    }

    const ObParamDef &ObProcedureStmt::get_param(int64_t index) const
    {
      return params_.at(index);
    }
  }
}



