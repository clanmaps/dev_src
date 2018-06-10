/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_postfix_expression.cpp
 * @brief postfix expression class definition
 *
 * modified by longfei锛歛dd interface: get_expr()
 * modified by Qiushi FAN: add some functions to craete a new expression
 * modified by maoxiaoxiao: modify the if condition because of the added enum type "QUERY_ID"
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author Qiushi FAN <qsfan@ecnu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @date 2016_07_28
 */

/*
 * (C) 2007-2011 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * ob_postfix_expression.cpp is for what ...
 *
 * Version: $id: ob_postfix_expression.cpp, v 0.1 7/29/2011 14:39 xiaochu Exp $
 *
 * Authors:
 *   xiaochu <xiaochu.yh@taobao.com>
 *     - some work details if you want
 *
 * last update:
 * 2012/5/30 change name space oceanbase::common to oceanbase::sql
 *           modify calc interface to adapt ExprItem
 *
 */


#include "ob_postfix_expression.h"
#include "ob_type_convertor.h"
#include "common/utility.h"
#include "common/ob_tsi_factory.h"
#include "common/ob_obj_cast.h"
#include "common/ob_schema.h"
#include "sql/ob_item_type_str.h"
#include "ob_result_set.h"
#include "ob_sql_session_info.h"
//add fanqiushi [semi_join] [0.1] 20150910:b
#include "common/ob_array.h"
//add:e
#include "ob_sp_procedure.h" //add zt 20151109:be

using namespace oceanbase::sql;

namespace oceanbase
{
  namespace sql
  {
    //add longfei
    ObObj& ObPostfixExpression::get_expr_by_index(int64_t index)
    {
      //int64_t index;
      if(expr_.count() <= index){
        index = expr_.count()-1;
      }
      return expr_.at(index);
    }
    //add:e
    //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
     ObObj& ObPostfixExpression::get_expr(){
        int64_t index;
        if(expr_.count()>0){
            index=expr_.count()-1;
        }
        else index=0;
        return expr_.at(index);
     }
     //modify xsl ECNU_DECIMAL
     int ObPostfixExpression::fix_varchar_and_decimal(uint32_t p,uint32_t s)
     {
        int ret=OB_SUCCESS;
        ObObj& obj=get_expr();
        ObObjType type=obj.get_type();
        if(type==ObDecimalType)
        {
            //ObString os;
            ObDecimal od;
            uint64_t *t1 = NULL;
            TTInt tt;
            t1 = obj.get_ttint();
            tt.FromUInt_v2(t1,obj.get_nwords());
            od.from(obj.get_precision(),obj.get_scale(),obj.get_vscale(),tt);
            if(od.get_scale()>s)
            {
                if(OB_SUCCESS != (ret=od.modify_value(p,s)))
                {
                    TBSYS_LOG(INFO,"fix_varchar_deciaml error,err=[%d]",ret);  //modify xsl
                }
                else
                {
                    ObObj obj2;
                    obj2.set_decimal_v2(od,obj.get_nwords());
                    str_buf_.write_obj(obj2, &obj);
                }
            }
        }
        else if(type==ObVarcharType)
        {
                ObString os;
                ObDecimal od;
                obj.get_varchar(os);

                if(OB_SUCCESS!=(ret=od.from(os.ptr(),os.length())))
                {}
                else if(od.get_scale()>s)
                {
                    if(OB_SUCCESS!=(ret=od.modify_value(p,s)))
                    {
                        TBSYS_LOG(INFO,"fix_varchar_deciaml error,err=[%d]",ret);   //modify xsl
                    }
                    else{
                        char buf[MAX_PRINTABLE_SIZE];
                        memset(buf, 0, MAX_PRINTABLE_SIZE);
                        int64_t length=0;
                        length=od.to_string(buf, MAX_PRINTABLE_SIZE);
                        ObString os2;
                        os2.assign_ptr(buf,static_cast<int32_t>(length));
                        ObObj obj2;
                        obj2.set_varchar(os2);
                        str_buf_.write_obj(obj2, &obj);
                    }
                }
        }
        return ret;
     }
     //add e
     //modify e
    bool ObPostfixExpression::ExprUtil::is_column_idx(const ObObj &obj)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || COLUMN_IDX != val)
      {
        ret = false;
      }
      return ret;
    }
    bool ObPostfixExpression::ExprUtil::is_const_obj(const ObObj &obj)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || CONST_OBJ != val)
      {
        ret = false;
      }
      return ret;
    }
    bool ObPostfixExpression::ExprUtil::is_op(const ObObj &obj)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || OP != val)
      {
        ret = false;
      }
      return ret;
    }
    bool ObPostfixExpression::ExprUtil::is_end(const ObObj &obj)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || END != val)
      {
        ret = false;
      }
      return ret;
    }
    bool ObPostfixExpression::ExprUtil::is_op_of_type(const ObObj &obj, ObItemType type)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || type != val)
      {
        ret = false;
      }
      return ret;
    }

    bool ObPostfixExpression::ExprUtil::is_value(const ObObj &obj, int64_t value)
    {
      int64_t val = 0;
      bool ret = true;
      if (OB_SUCCESS != obj.get_int(val) || value != val)
      {
        ret = false;
      }
      return ret;
    }


    int ExprItem::assign(const common::ObObj &obj)
    {
      int ret = OB_SUCCESS;
      switch(obj.get_type())
      {
        case ObNullType:
          type_ = T_NULL;
          break;
        case ObIntType:
          type_ = T_INT;
          obj.get_int(value_.int_);
          break;
        case ObFloatType:
          type_ = T_FLOAT;
          obj.get_float(value_.float_);
          break;
        case ObDoubleType:
          type_ = T_DOUBLE;
          obj.get_double(value_.double_);
          break;
        case ObPreciseDateTimeType:
          type_ = T_DATE;
          obj.get_precise_datetime(value_.datetime_);
          break;
        case ObVarcharType:
          type_ = T_STRING;
          obj.get_varchar(string_);
          break;
        case ObBoolType:
          type_ = T_BOOL;
          obj.get_bool(value_.bool_);
          break;
        case ObDecimalType:
          //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
          /*
           *该函数是将参数Obj里面的值存到ExprItem里面
           *参数obj是中缀表达式里面的值，ExprItem里面的值最后会写到后缀表达式里面
           *因为中缀表达式里我把decimal的值按照varchar存的，所以在这里存到ExprItem里面的值也是按照varcahr存的
           *get_varchar_d函数是我们自己写的，主要功能跟ObObj的get_varchar函数相同。不同之处是get_varchar必须是type为
           *ObVarcharType的obj才能调用。而调用get_varchar_d函数的ObObj可以是其他类型
           */
          //ret = OB_NOT_SUPPORTED;      //old code
          type_ = T_DECIMAL;
          //modify xsl ECNU_DECIMAL 2017_2
           //obj.get_varchar_d(string_);
           obj.get_decimal_v2(dec);    //将obj中的值传到ExprItem中
           len = obj.get_nwords();
          //TBSYS_LOG(INFO,"xushilei,test assign dec=[%s]",to_cstring(dec));//add xsl
           //modify:e
          break;
        default:
          ret = OB_ERR_UNEXPECTED;
          break;
      }
      return ret;
    }

    /*     鍒濆鍖栨暟瀛﹁繍绠楁搷浣滆皟鐢ㄨ〃 */
    op_call_func_t ObPostfixExpression::call_func[T_MAX_OP - T_MIN_OP - 1] = {
      /*   WARNING: 下面的顺序不可以调换��
       *   需要与(ob_item_tpye.h) ExprType enum定义对应
       */
      ObPostfixExpression::minus_func, /* T_OP_NEG */
      ObPostfixExpression::plus_func, /* T_OP_POS */
      ObPostfixExpression::add_func, /* T_OP_ADD */
      ObPostfixExpression::sub_func, /* T_OP_MINUS */
      ObPostfixExpression::mul_func, /* T_OP_MUL */
      ObPostfixExpression::div_func, /* T_OP_DIV */
      ObPostfixExpression::nop_func, /* TODO: T_OP_POW */
      ObPostfixExpression::mod_func, /* T_OP_REM */
      ObPostfixExpression::mod_func, /* T_OP_MOD */
      ObPostfixExpression::eq_func,  /* T_OP_EQ */
      ObPostfixExpression::le_func,  /* T_OP_LE */
      ObPostfixExpression::lt_func,  /* T_OP_LT */
      ObPostfixExpression::ge_func,  /* T_OP_GE */
      ObPostfixExpression::gt_func,  /* T_OP_GT */
      ObPostfixExpression::neq_func, /* T_OP_NE */
      ObPostfixExpression::is_func,  /* T_OP_IS */
      ObPostfixExpression::is_not_func,/* T_OP_IS_NOT */
      ObPostfixExpression::btw_func, /* T_OP_BTW */
      ObPostfixExpression::not_btw_func, /* T_OP_NOT_BTW */
      ObPostfixExpression::like_func,/* T_OP_LIKE */
      ObPostfixExpression::not_like_func, /* T_OP_NOT_LIKE */
      ObPostfixExpression::not_func, /* T_OP_NOT */
      ObPostfixExpression::and_func, /* T_OP_AND */
      ObPostfixExpression::or_func,  /* T_OP_OR */
      ObPostfixExpression::in_func, /*  T_OP_IN */
      ObPostfixExpression::not_in_func, /* T_OP_NOT_IN */
      ObPostfixExpression::arg_case_func, /* T_OP_AGR_CASE */
      ObPostfixExpression::case_func, /* T_OP_CASE */
      ObPostfixExpression::row_func, /* T_OP_ROW */
      ObPostfixExpression::nop_func, /* TODO: T_OP_EXISTS */
      ObPostfixExpression::concat_func, /* T_OP_CNN */
      ObPostfixExpression::nop_func, // T_FUN_SYS
      ObPostfixExpression::left_param_end_func, // T_OP_LEFT_PARAM_END
    };

    // system function table
    op_call_func_t ObPostfixExpression::SYS_FUNCS_TAB[SYS_FUNC_NUM] =
    {
      ObPostfixExpression::sys_func_length, // SYS_FUNC_LENGTH
      ObPostfixExpression::sys_func_substr, // SYS_FUNC_SUBSTR
      ObPostfixExpression::sys_func_cast,             // SYS_FUNC_CAST
      ObPostfixExpression::sys_func_cur_user,         // SYS_FUNC_CUR_USER
      ObPostfixExpression::sys_func_trim,             // SYS_FUNC_TRIM
      ObPostfixExpression::sys_func_lower,            // SYS_FUNC_LOWER
      ObPostfixExpression::sys_func_upper,            // SYS_FUNC_UPPER
      ObPostfixExpression::sys_func_coalesce,         // SYS_FUNC_COALESCE
      ObPostfixExpression::sys_func_hex,         // SYS_FUNC_HEX
      ObPostfixExpression::sys_func_unhex,         // SYS_FUNC_UNHEX
      ObPostfixExpression::sys_func_ip_to_int,         // SYS_FUNC_IP_TO_INT
      ObPostfixExpression::sys_func_int_to_ip,         // SYS_FUNC_INT_TO_IP
      ObPostfixExpression::sys_func_greatest,         // SYS_FUNC_GREATEST
      ObPostfixExpression::sys_func_least,            // SYS_FUNC_LEAST
    };

    const char* const ObPostfixExpression::SYS_FUNCS_NAME[SYS_FUNC_NUM] =
    {
      "length",
      "substr",
      "cast",
      "current_user",
      "trim",
      "lower",
      "upper",
      "coalesce",
      "hex",
      "unhex",
      "ip2int",
      "int2ip",
      "greatest",
      "least"
    };

    int32_t ObPostfixExpression::SYS_FUNCS_ARGS_NUM[SYS_FUNC_NUM] =
    {
      1,
      TWO_OR_THREE,
      //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
      //2,   //old code
      TWO_OR_FOUR,
     //modify:e
      0,
      3,/*trim*/
      1,
      1,
      MORE_THAN_ZERO,
      1,
      1,
      1,
      1,
      MORE_THAN_ZERO,
      MORE_THAN_ZERO,
    };

    ObPostfixExpression::ObPostfixExpression()
      :expr_(64*1024, ModulePageAllocator(ObModIds::OB_SQL_EXPR)),
       stack_(NULL),
       did_int_div_as_double_(false),
       str_buf_(ObModIds::OB_SQL_EXPR, DEF_STRING_BUF_SIZE),
       owner_op_(NULL),
       calc_buf_(ObModIds::OB_SQL_EXPR_CALC, DEF_STRING_BUF_SIZE),
       bloom_filter_(NULL) //add weixing [implementation of sub_query]20160111
    {
    }

    ObPostfixExpression::~ObPostfixExpression()
    {
    }

    ObPostfixExpression& ObPostfixExpression::operator=(const ObPostfixExpression &other)
    {
      int ret = OB_SUCCESS;
      int i = 0;
      this->expr_.clear();
      this->calc_buf_.clear();
      if (OB_SUCCESS != (ret = str_buf_.reset()))
      {
        TBSYS_LOG(WARN, "fail to reset string buffer");
      }
      else if (&other != this)
      {
        ObObj obj;
        this->set_owner_op(other.owner_op_);
        expr_.reserve(other.expr_.count());
        for (i = 0; i < other.expr_.count(); i++)
        {
          if (ObVarcharType == other.expr_[i].get_type())
          {
            if(OB_SUCCESS != (ret = str_buf_.write_obj(other.expr_[i], &obj)))
            {
              TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);
            }
            else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
            {
              TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
            }
          }
          //add fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
          else if(ObDecimalType==other.expr_[i].get_type())
          {
            if(OB_SUCCESS != (ret = str_buf_.write_obj(other.expr_[i], &obj)))
            {
              TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);
            }
            else
            {

                obj.set_precision(other.expr_[i].get_precision());
                obj.set_scale(other.expr_[i].get_scale());

                if (OB_SUCCESS != (ret = expr_.push_back(obj)))
                {
                    TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
                }
            }

          }
          //add:e
          else
          {
            if (OB_SUCCESS != (ret = expr_.push_back(other.expr_[i])))
            {
              TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
            }
          }
        }
        //add weixing [implementation of sub_query]20160111
        this->bloom_filter_ = other.bloom_filter_;
        //add e
      }
      return *this;
    }

    int ObPostfixExpression::get_sys_func(const common::ObString &sys_func, ObSqlSysFunc &func_type) const
    {
#define OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_ID) \
      else if (sys_func.length() == static_cast<int64_t>(strlen(SYS_FUNCS_NAME[SYS_FUNC_ID])) \
          && 0 == strncasecmp(SYS_FUNCS_NAME[SYS_FUNC_ID], sys_func.ptr(), sys_func.length())) \
      { \
        func_type = SYS_FUNC_ID; \
      }

      int ret = OB_SUCCESS;
      if (0)
      {
        // if (0) so that the macro works
      }
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_LENGTH)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_SUBSTR)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_CAST)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_CUR_USER)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_TRIM)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_LOWER)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_UPPER)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_COALESCE)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_HEX)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_UNHEX)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_IP_TO_INT)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_INT_TO_IP)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_GREATEST)
      OB_POSTFIX_EXPRESSION_GET_SYS_FUNC(SYS_FUNC_LEAST)
      else
      {
        ret = OB_ERR_UNKNOWN_SYS_FUNC;
      }
      return ret;
#undef OB_POSTFIX_EXPRESSION_GET_SYS_FUNC
    }

    int ObPostfixExpression::add_expr_obj(const ObObj &obj)
    {
      int ret = OB_SUCCESS;
      ObObj obj2;
      if (obj.get_type() == ObVarcharType)
      {
        if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
        {
          TBSYS_LOG(WARN, "fail to write object to string buffer. err=%d", ret);
        }
        else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
        {
          TBSYS_LOG(WARN, "fail to push object to expr array. err=%d", ret);
        }
      }
      else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
      {
        TBSYS_LOG(WARN, "fail to push object to expr array. err=%d", ret);
      }
      return ret;
    }

    int ObPostfixExpression::add_expr_item(const ExprItem &item)
    {
      int ret = OB_SUCCESS;
      ObObj item_type;
      ObObj obj;
      ObObj obj2;
      ObSqlSysFunc sys_func;
      switch(item.type_)
      {
        case T_STRING:
        case T_BINARY:
          item_type.set_int(CONST_OBJ);
          obj.set_varchar(item.string_);
          if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
          {
            TBSYS_LOG(WARN, "fail to write object to string buffer. err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          break;
        case T_FLOAT:
          item_type.set_int(CONST_OBJ);
          obj.set_float(item.value_.float_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_DOUBLE:
          item_type.set_int(CONST_OBJ);
          obj.set_double(item.value_.double_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_DECIMAL:
          //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
          /*
          *     生成后缀表达式，将值存到obj2里面，再把obj2放到后缀表达式的数组expr_中
          *     这里obj2的类型是ObDecimalType.
          */
          item_type.set_int(CONST_OBJ);
          obj.set_decimal_v2(item.dec,item.len);   //add xsl ECNU_DECIMAL
          //obj.set_decimal(item.dec);   //转换成功
          //obj.set_varchar(item.string_);
          if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
          {
            TBSYS_LOG(WARN, "fail to write object to string buffer. err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {
              TBSYS_LOG(WARN, "fail to push item.type. err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {
              TBSYS_LOG(WARN, "fail to push object. err=%d", ret);
          }
          break;
          //modify e
        case T_INT:
          item_type.set_int(CONST_OBJ);
          obj.set_int(item.value_.int_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_BOOL:
          item_type.set_int(CONST_OBJ);
          obj.set_bool(item.value_.bool_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_DATE:
          item_type.set_int(CONST_OBJ);
          obj.set_precise_datetime(item.value_.datetime_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_CUR_TIME:
          item_type.set_int(CUR_TIME_OP);
          obj.set_int(item.value_.int_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_QUESTIONMARK:
          item_type.set_int(PARAM_IDX);
          obj.set_int(item.value_.int_);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_SYSTEM_VARIABLE:
        case T_TEMP_VARIABLE:
          item_type.set_int(item.type_ == T_SYSTEM_VARIABLE ? SYSTEM_VAR : TEMP_VAR);
          obj.set_varchar(item.string_);
          if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
          {
            TBSYS_LOG(WARN, "fail to write variable name to string buffer. err=%d", ret);
          }
          else if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          break;
        //add zt 20151126:b
        case T_ARRAY:
          item_type.set_int(ARRAY_VAR);
          obj.set_varchar(item.string_);
          if( OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
          {
            TBSYS_LOG(WARN, "fail to write variable name to string buffer. err=%d", ret);
          }
          else if( OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if ( OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          break;
        //add zt 20151126:e
        case T_NULL:
          item_type.set_int(CONST_OBJ);
          obj.set_null();
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          break;
        case T_REF_COLUMN:
          item_type.set_int(COLUMN_IDX);
          obj.set_int(item.value_.cell_.tid);
          obj2.set_int(item.value_.cell_.cid);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          break;
        case T_REF_QUERY:
        // mod weixing [implementation of sub_query]20160119:b
//          TBSYS_LOG(ERROR, "TODO... not implement yet");
//          ret = OB_NOT_SUPPORTED;
        item_type.set_int(QUERY_ID);
        obj.set_int(item.value_.int_);
        if(OB_SUCCESS != (ret = expr_.push_back(item_type)))
        {
          TBSYS_LOG(ERROR,"expr_ push back item_type failed");
        }
        if(OB_SUCCESS != (ret = expr_.push_back(obj)))
        {
          TBSYS_LOG(ERROR,"expr_ push back obj failed");
        }
        //mod 20160119:e
          break;
        case T_OP_EXISTS:
        case T_OP_POS:
        case T_OP_NEG:
        case T_OP_ADD:
        case T_OP_MINUS:
        case T_OP_MUL:
        case T_OP_DIV:
        case T_OP_REM:
        case T_OP_POW:
        case T_OP_MOD:
        case T_OP_LE:
        case T_OP_LT:
        case T_OP_EQ:
        case T_OP_GE:
        case T_OP_GT:
        case T_OP_NE:
        case T_OP_LIKE:
        case T_OP_NOT_LIKE:
        case T_OP_AND:
        case T_OP_OR:
        case T_OP_NOT:
        case T_OP_IS:
        case T_OP_IS_NOT:
        case T_OP_BTW:
        case T_OP_NOT_BTW:
        case T_OP_CNN:
        case T_OP_IN:
        case T_OP_NOT_IN:
        case T_OP_ARG_CASE:
        case T_OP_CASE:
        case T_OP_ROW:
        case T_OP_LEFT_PARAM_END:
          item_type.set_int(OP);
          obj.set_int(item.type_); // op type
          obj2.set_int(item.value_.int_); // operands count
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          break;
        case T_CUR_TIME_OP:
          item_type.set_int(UPS_TIME_OP);
          if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {
            TBSYS_LOG(WARN, "failed to push item_type, ret=%d", ret);
          }
          break;
        case T_FUN_SYS:
          item_type.set_int(OP);
          obj.set_int(item.type_); // system function
          obj2.set_int(item.value_.int_); // operands count
          if (OB_SUCCESS != (ret = get_sys_func(item.string_, sys_func)))
          {
            TBSYS_LOG(WARN, "unknown system function=%.*s", item.string_.length(), item.string_.ptr());
          }
          else if (OB_SUCCESS != (ret = expr_.push_back(item_type)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj)))
          {}
          else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
          {}
          else
          {
            obj2.set_int(sys_func);
            ret = expr_.push_back(obj2);
          }
          break;
        default:
          TBSYS_LOG(WARN, "unknown expr item type=%d", item.type_);
          ret = OB_INVALID_ARGUMENT;
          break;
      }
      return ret;
    }

    int ObPostfixExpression::add_expr_item_end()
    {
      int ret = OB_SUCCESS;
      ObObj obj;
      obj.set_int(END);
      if (OB_SUCCESS != (ret = expr_.push_back(obj)))
      {
        TBSYS_LOG(WARN, "failed to add END, err=%d", ret);
      }
      return ret;
    }

    //add fanqiushi [semi_join] [0.1] 20150910:b
    int ObPostfixExpression::set_for_semi_join(common::ObArray<common::ObObj> *tmp_set,uint64_t tid,uint64_t cid)
    {
      int ret=OB_SUCCESS;
      //add fanqiushi [semi_join] [0.1] 20151109:b
      if(tmp_set->count()<=10)
      {
          ObObj tmp1,tmp2,tmp3;
          tmp1.set_int(1);
          expr_.push_back(tmp1);
          tmp2.set_int(tid);
          expr_.push_back(tmp2);
          tmp3.set_int(cid);
          expr_.push_back(tmp3);

          ObObj tmp4,tmp5,tmp6,tmp7,tmp8,tmp9,tmp10;
          tmp4.set_int(OP);
          tmp5.set_int(T_OP_ROW);
          tmp6.set_int(T_OP_LEFT_PARAM_END);
          tmp7.set_int(CONST_OBJ);
          tmp8.set_int(tmp_set->count());
          tmp9.set_int(T_OP_IN);
          tmp10.set_int(END);
          expr_.push_back(tmp4);
          expr_.push_back(tmp5);
          expr_.push_back(tmp1);
          expr_.push_back(tmp4);
          expr_.push_back(tmp6);
          expr_.push_back(tmp7);

          for(int i=0;i<tmp_set->count();i++)
          {
             expr_.push_back(tmp7);
             ObObj in_value=tmp_set->at(i);
             if(in_value.get_type()==ObVarcharType)
             {
                ObObj obj_var;
                if(OB_SUCCESS != (ret = str_buf_.write_obj(in_value, &obj_var)))
                {
                  TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);
                }
                else
                {
                    expr_.push_back(obj_var);
                }
             }
             else
             {
                expr_.push_back(tmp_set->at(i));
             }
             expr_.push_back(tmp4);
             expr_.push_back(tmp5);
             expr_.push_back(tmp1);
          }

          expr_.push_back(tmp4);
          expr_.push_back(tmp5);
          expr_.push_back(tmp8);

          expr_.push_back(tmp4);
          expr_.push_back(tmp9);
          expr_.push_back(tmp7);

          expr_.push_back(tmp10);
      }
      else
      {
            //todo between
          ObObj tmp1,tmp2,tmp3;
          tmp1.set_int(1);
          expr_.push_back(tmp1);
          tmp2.set_int(tid);
          expr_.push_back(tmp2);
          tmp3.set_int(cid);
          expr_.push_back(tmp3);

          ObObj tmp4;
          tmp4.set_int(2);

          expr_.push_back(tmp4);
          ObObj in_value=tmp_set->at(0);
         // TBSYS_LOG(ERROR,"test::fanqs,,in_value=%s",to_cstring(in_value));
          if(in_value.get_type()==ObVarcharType)
          {
             ObObj obj_var;
             if(OB_SUCCESS != (ret = str_buf_.write_obj(in_value, &obj_var)))
             {
               TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);
             }
             else
             {
                 expr_.push_back(obj_var);
             }
          }
          else
          {
             expr_.push_back(tmp_set->at(0));
          }

          expr_.push_back(tmp4);
          ObObj in_value2=tmp_set->at(tmp_set->count()-1);
           //TBSYS_LOG(ERROR,"test::fanqs,,in_value2=%s",to_cstring(in_value2));
          if(in_value2.get_type()==ObVarcharType)
          {
             ObObj obj_var2;
             if(OB_SUCCESS != (ret = str_buf_.write_obj(in_value2, &obj_var2)))
             {
               TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);
             }
             else
             {
                 expr_.push_back(obj_var2);
             }
          }
          else
          {
             expr_.push_back(tmp_set->at(tmp_set->count()-1));
          }

          ObObj tmp11,tmp12,tmp13,tmp14;
          tmp11.set_int(OP);
          expr_.push_back(tmp11);
          tmp12.set_int(118);
          expr_.push_back(tmp12);
          tmp13.set_int(3);
          expr_.push_back(tmp13);
          tmp14.set_int(END);
          expr_.push_back(tmp14);
      }
      return ret;
    }
    //add:e

    int ObPostfixExpression::merge_expr(const ObPostfixExpression &expr1, const ObPostfixExpression &expr2, const ExprItem &op)
    {
      int ret = OB_SUCCESS;
      for(int64_t i = 0; ret == OB_SUCCESS && i < expr1.expr_.count() - 1; i++)
      {
        ret = this->expr_.push_back(expr1.expr_[i]);

      }
      for(int64_t i = 0; ret == OB_SUCCESS && i < expr2.expr_.count() - 1; i++)
      {
        ret = this->expr_.push_back(expr2.expr_[i]);

      }
      if (ret == OB_SUCCESS)
      {
        ret = add_expr_item(op);
      }
      if (ret == OB_SUCCESS)
      {
        ret = add_expr_item_end();
      }
      if (ret != OB_SUCCESS)
      {
        TBSYS_LOG(ERROR, "Generate new expression faild, ret=%d", ret);
      }
      return ret;
    }

    //mod weixing [implementation of sub_query]20160111
    //int ObPostfixExpression::calc(const common::ObRow &row, const ObObj *&composite_val)
    int ObPostfixExpression::calc(const common::ObRow &row, const ObObj *&composite_val, hash::ObHashMap<common::ObRowkey, common::ObRowkey, common::hash::NoPthreadDefendMode>* hash_map, bool second_check)
    //mod e
    {
      int ret = OB_SUCCESS;
      int64_t type = 0;
      int64_t value = 0;
      int64_t value2 = 0;
      int64_t sys_func = 0;
      int idx = 0;
      ObExprObj result;
      int idx_i = 0;
      //add weixing [implementation of sub_query]20160116
      int sub_query_idx = 0;
      //add e
      ObPostExprExtraParams *extra_params = GET_TSI_MULT(ObPostExprExtraParams, TSI_SQL_EXPR_EXTRA_PARAMS_1);
      // get the stack for calculation
      ObPostfixExpressionCalcStack *stack = GET_TSI_MULT(ObPostfixExpressionCalcStack, TSI_SQL_EXPR_STACK_1);
      if (NULL == stack || NULL == extra_params)
      {
        TBSYS_LOG(ERROR, "no memory for postfix expression extra params and stack");
        ret = OB_ALLOCATE_MEMORY_FAILED;
      }
      else
      {
        stack_ = stack->stack_;
        extra_params->did_int_div_as_double_ = did_int_div_as_double_;
        calc_buf_.reuse();
      }
      bool quick_path = false;
      const ObObj *var = NULL;
      while (OB_SUCCESS == ret)
      {
        // 获得数据类型:列id、数字、操作符、结束标��
        if (OB_SUCCESS != (ret = expr_[idx++].get_int(type)))
        {
          TBSYS_LOG(WARN, "fail to get int value. unexpected! ret=%d idx=%d,type=%ld", ret, idx-1,type);
          ret = OB_ERR_UNEXPECTED;
          break;
        }
        /*modify maoxx [bloomfilter_join] 20160714*/
        //else if(type <= BEGIN_TYPE || type >= END_TYPE)
        else if(type <= BEGIN_TYPE || type > QUERY_ID)
        /*modify e*/
        {
          TBSYS_LOG(WARN, "unsupported operand type [type:%ld]", value);
          ret = OB_ERR_UNEXPECTED;
          break;
        }
        else if (END == type)   // expr_涓互END绗﹀彿琛ㄧず缁撴潫
        {
          if (idx_i != 1)
          {
            TBSYS_LOG(WARN, "calculation stack must be empty. check the code for bugs. idx_i=%d", idx_i);
            ret = OB_ERR_UNEXPECTED;
          }
          else //idx_i==1?
          {
              //modify xsl ECNU_DECIMAL 2016.12
              //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
              ObExprObj E_obj=stack_[idx_i-1];
              if(E_obj.get_type()==ObDecimalType)
              {
                  ObDecimal *od =NULL;
                  od=const_cast<ObDecimal *>(&E_obj.get_decimal());
                  uint32_t len = E_obj.get_len();
                  ObObj obj;
                  ret=obj.set_decimal_v2(*od,len);
                  if(OB_SUCCESS != (ret =str_buf_.write_obj(obj, &result_)))   //自己申请一块内存存放着
                  {
                      TBSYS_LOG(ERROR, "fail to write object to string buffer. ret=%d", ret);

                  }
                  composite_val = &result_;
                  idx_i--;
              }
              //modify e
              else         //not decimal
              {
                  if (OB_SUCCESS != (ret = stack_[--idx_i].to(result_)))    //Exprobj->obj
                  {
                      TBSYS_LOG(WARN, "failed to convert exprobj to obj, err=%d", ret);
                  }

                  else
                  {
                      composite_val = &result_;
                  }
              }
              //modify:e
          }
          break;
        }
        else if (idx_i < 0
                 || idx_i >= ObPostfixExpressionCalcStack::STACK_SIZE
                 || idx > expr_.count())
        {
          TBSYS_LOG(WARN,"calculation stack overflow [stack.index:%d] "
                    "or run out of operand [operand.used:%d,operand.avaliable:%ld]", idx_i, idx, expr_.count());
          ret = OB_ERR_UNEXPECTED;
          break;
        }
        else
        {
          // do calculation
          switch(type)
          {
            case COLUMN_IDX:
              if (OB_SUCCESS != (ret = expr_[idx++].get_int(value)))
              {
                TBSYS_LOG(WARN,"get_int error [err:%d]", ret);
              }
              else if (OB_SUCCESS  != (ret = expr_[idx++].get_int(value2)))
              {
                TBSYS_LOG(WARN,"get_int error [err:%d]", ret);
              }
              else
              {
                if (4 == expr_.count())
                {
                  // quick path
                  if (OB_SUCCESS != (ret = row.get_cell(static_cast<uint64_t>(value),
                                                      static_cast<uint64_t>(value2), composite_val)))
                  {
                    TBSYS_LOG(WARN, "fail to get cell from row. err=%d tid=%ld cid=%ld",
                              ret, value, value2);
                  }
                  quick_path = true;
                }
                else
                {
                  const ObObj *cell = NULL;
                  if (OB_SUCCESS != (ret = row.get_cell(static_cast<uint64_t>(value),
                                                        static_cast<uint64_t>(value2), cell)))
                  {
                    TBSYS_LOG(WARN, "fail to get cell from row. err=%d tid=%ld cid=%ld",
                              ret, value, value2);
                  }
                  else
                  {
                    stack_[idx_i++].assign(*cell);
                  }
                }
              }
              break;
            case CONST_OBJ:
              stack_[idx_i++].assign(expr_[idx++]);
              break;
            case PARAM_IDX:
            case SYSTEM_VAR:
            case TEMP_VAR:
            case CUR_TIME_OP:
              if (OB_SUCCESS == (ret = get_var_obj(static_cast<ObPostExprNodeType>(type), expr_[idx++], var)))
              {
                stack_[idx_i++].assign(*var);
              }
              else
              {
                TBSYS_LOG(WARN, "Can not get value ObObj. err=%d", ret);
              }
              break;
            //add zt 20151126:b
            case ARRAY_VAR:
            {
              //array read, (ARRAY_VAR, ARRAY_NAME, INDEX_TYPE, INDEX_VALUE)
              int64_t idx_type = -1;
              if( OB_SUCCESS != (ret = expr_[idx + 1].get_int(idx_type)) )
              {
                TBSYS_LOG(WARN, "get index type failed");
              }
              else if( idx_type != CONST_OBJ && idx_type != TEMP_VAR )
              {
                TBSYS_LOG(WARN, "index type is not correct");
              }
              else if( OB_SUCCESS == (ret = get_array_var(expr_[idx], idx_type, expr_[idx + 2], var)) )
              {
                stack_[idx_i++].assign(*var);
                idx += 3;
              }
//              if( idx_i < 1 || stack_[idx_i-1].get_type() != ObIntType )
//              {
//                TBSYS_LOG(WARN, "array is not indexed by right value");
//                ret =  OB_ERR_ILLEGAL_INDEX;
//              }
//              else if( OB_SUCCESS == (ret = get_array_var(expr_[idx++], stack_[--idx_i].get_int(), var)))
//              {
//                stack_[idx_i++].assign(*var);
//              }
              break;
            }
            //add zt 20151126:e
            case OP:
              // 鏍规嵁OP鐨勭被鍨嬶紝浠庡爢鏍堜腑寮瑰嚭1涓垨澶氫釜鎿嶄綔鏁帮紝杩涜璁＄畻
              if (OB_SUCCESS != (ret = expr_[idx++].get_int(value)))
              {
                TBSYS_LOG(WARN,"get operator type failed [err:%d]", ret);
              }
              else if (OB_UNLIKELY(value <= T_MIN_OP || value >= T_MAX_OP))
              {
                TBSYS_LOG(WARN,"unsupported operator type [type:%ld]", value);
                ret = OB_ERR_UNEXPECTED;
              }
              else if(OB_SUCCESS != (ret = expr_[idx++].get_int(value2 /*param_count*/)))
              {
                 TBSYS_LOG(WARN,"get_int error [err:%d]", ret);
              }
              else
              {
                extra_params->operand_count_ = static_cast<int32_t>(value2);
                extra_params->str_buf_ = &calc_buf_;
                if (OB_UNLIKELY(T_FUN_SYS == value))
                {
                  if(OB_SUCCESS != (ret = expr_[idx++].get_int(sys_func)))
                  {
                    TBSYS_LOG(WARN, "failed to get sys func, err=%d", ret);
                  }
                  else if (0 > sys_func || sys_func >= SYS_FUNC_NUM)
                  {
                    TBSYS_LOG(WARN, "invalid sys function type=%ld", sys_func);
                    ret = OB_ERR_UNEXPECTED;
                  }
                  else if (OB_SUCCESS != (ret = SYS_FUNCS_TAB[sys_func](stack_, idx_i, result, *extra_params)))
                  {
                    TBSYS_LOG(WARN, "failed to call sys func, err=%d func=%ld", ret, sys_func);
                  }
                  else
                  {
                    stack_[idx_i++] = result;
                  }
                }
                else
                {
                  //add weixing [implementation of sub_query]20160111
                  bool special_process = false;
                  if ((T_OP_IN == value ||T_OP_NOT_IN == value) && (NULL != bloom_filter_ || NULL != hash_map))
                  {
                    int64_t right_elem_count = 0;
                    if (OB_SUCCESS != (ret = stack_[idx_i-1].get_int(right_elem_count)))
                    {
                      TBSYS_LOG(WARN, "fail to get_int from stack. top=%d, ret=%d", idx_i, ret);
                    }
                    else if(-1 == right_elem_count)
                    {
                      special_process = true;
                      if (OB_LIKELY(OB_SUCCESS == (ret = this->in_sub_query_func(stack_, idx_i, result, *extra_params, bloom_filter_, hash_map, sub_query_idx, second_check))))
                      {
                        if(T_OP_NOT_IN == value)
                        {
                          result.lnot(result);
                        }
                        stack_[idx_i++] = result;
                      }
                      else if (OB_NO_RESULT == ret)
                      {
                        ret = OB_SUCCESS;
                      }
                      else
                      {
                        TBSYS_LOG(WARN, "call calculation function error [value:%ld, idx_i:%d, err:%d]",
                                  value, idx_i, ret);
                      }
                    }
                  }
                  if(!special_process)
                  {
                    //add e
                    if (OB_LIKELY(OB_SUCCESS == (ret = (this->call_func[value - T_MIN_OP - 1])(stack_, idx_i, result, *extra_params))))
                    {
                        //add xsl ECNU_DECIMAL 2017_5
                        uint32_t len = 1;
                        if(const_cast<ObDecimal &>(result.get_decimal()).get_words()[0].table[1] != 0)
                        {
                            len =2;
                        }
                        result.set_len(len);
                        stack_[idx_i++] = result;
                        //add e
                    }
                    else if (OB_NO_RESULT == ret)
                    {
                      // nop
                      ret = OB_SUCCESS;
                    }
                    else
                    {
                      TBSYS_LOG(WARN, "call calculation function error [value:%ld, idx_i:%d, err:%d]",
                                value, idx_i, ret);
                    }
                  }
                }
              }
              break;
            case UPS_TIME_OP:
              if (OB_SUCCESS != (ret = sys_func_current_timestamp(stack_, idx_i, result, *extra_params)))
              {
                TBSYS_LOG(WARN, "failed to call sys_func_current_timestamp, err=%d", ret);
              }
              else
              {
                stack_[idx_i++] = result;
              }
              break;
            //add weixing [implementation of sub_query]20160116
            case QUERY_ID:
              if(OB_SUCCESS != (ret = expr_[idx++].get_int(value)))
              {
                TBSYS_LOG(WARN,"get_int error [err:%d]", ret);
              }
              else
              {
                int64_t temp_value = -1;
                result.set_int(temp_value);
                stack_[idx_i++] = result;
              }
              break;
            //add e
            default:
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN,"unexpected [type:%ld]", type);
              break;
          } // end switch
        }   // end else
        if (quick_path)
          break;
      } // end while

      return ret;
    }

    //add by zt 20160616:b
    const SpProcedure * get_running_procedure(const ObPhysicalPlan *plan)
    {
      const SpProcedure *proc = NULL;
      if( plan->get_main_query()->get_type() == PHY_PROCEDURE )
      {
        proc = static_cast<SpProcedure*>(plan->get_main_query());
      }
      else if( const_cast<ObPhysicalPlan*>(plan)->get_result_set() != NULL )
      {
        proc = const_cast<ObPhysicalPlan*>(plan)->get_result_set()->get_running_procedure();
      }
      return proc;
    }
    //add by zt 20160116:e

    int ObPostfixExpression::get_var_obj(
                    ObPostExprNodeType type,
                    const ObObj& expr_node,
                    const ObObj*& val) const
    {
      int ret = OB_SUCCESS;
      ObResultSet *result_set = NULL;
      if (type != PARAM_IDX && type != SYSTEM_VAR && type != TEMP_VAR && type != CUR_TIME_OP
            && type != ARRAY_VAR //add zt 20151126
          )
      {
        val = &expr_node;
      }
      else
      {
        //add by zt: 20160615 :b
        const SpProcedure *proc = NULL;
        if( !owner_op_ || ! owner_op_->get_phy_plan() )
        {
          ret = OB_ERR_UNEXPECTED;
          TBSYS_LOG(WARN, "Can not get phyplan. err=%d", ret);
        }
        else if( NULL != (proc = get_running_procedure(owner_op_->get_phy_plan())) )
        {

          if( type == PARAM_IDX || (type == CUR_TIME_OP &&(result_set = owner_op_->get_phy_plan()->get_result_set())== NULL))

          {
            TBSYS_LOG(WARN, "Unsupported read");
            ret = OB_NOT_SUPPORTED;
          }
          else if( type == CUR_TIME_OP )
          {
            if ((val = result_set->get_cur_time_place()) == NULL)
            {
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN, "Can not get current time. err=%d", ret);
            }
          }
          else
          {
             ObString var_name;
             if( OB_SUCCESS != (ret = expr_node.get_varchar(var_name)) )
             {
               TBSYS_LOG(ERROR, "Can not get variable name, %.*s", var_name.length(), var_name.ptr());
             }
             else if(OB_SUCCESS != (ret = proc->read_variable(var_name, val)) )
             {
               TBSYS_LOG(ERROR, "Variable %.*s does not exists", var_name.length(), var_name.ptr());
             }
             else
             {
                 TBSYS_LOG(TRACE, "Get variable name success, %.*s", var_name.length(), var_name.ptr());
             }
          }
        }
        else
        //add by zt: 20160615 :e
        if (!owner_op_ || !owner_op_->get_phy_plan()
                  || !(result_set = owner_op_->get_phy_plan()->get_result_set()))
        {
          ret = OB_ERR_UNEXPECTED;
          TBSYS_LOG(WARN, "Can not get result set.err=%d", ret);
        }
        else if (type == PARAM_IDX)
        {
          int64_t param_idx = OB_INVALID_INDEX;
          if ((ret = expr_node.get_int(param_idx)) != OB_SUCCESS)
          {
            TBSYS_LOG(ERROR, "Can not get param index, ret=%d", ret);
          }
          else if (param_idx < 0 || param_idx >= result_set->get_params().count())
          {
            ret = OB_ERR_ILLEGAL_INDEX;
            TBSYS_LOG(ERROR, "Wrong index of question mark position, pos = %ld\n", param_idx);
          }
          else
          {
            val = result_set->get_params().at(param_idx);
          }
        }
        else if (type == SYSTEM_VAR || type == TEMP_VAR)
        {
          ObString var_name;
          ObSQLSessionInfo *session_info = result_set->get_session();
          if (!session_info)
          {
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "Can not get session info.err=%d", ret);
          }
          else if ((ret = expr_node.get_varchar(var_name)) != OB_SUCCESS)
          {
            TBSYS_LOG(ERROR, "Can not get variable name");
          }
          else if (type == SYSTEM_VAR
                   && (val = session_info->get_sys_variable_value(var_name)) == NULL)
          {
            ret = OB_ERR_VARIABLE_UNKNOWN;
            TBSYS_LOG(USER_ERROR, "System variable %.*s does not exists", var_name.length(), var_name.ptr());
          }
          else if (type == TEMP_VAR
                   && (val = session_info->get_variable_value(var_name)) == NULL)
          {
            ret = OB_ERR_VARIABLE_UNKNOWN;
            TBSYS_LOG(USER_ERROR, "Variable %.*s does not exists", var_name.length(), var_name.ptr());
          }
        }
        else if (type == CUR_TIME_OP)
        {
          if ((val = result_set->get_cur_time_place()) == NULL)
          {
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN, "Can not get current time. err=%d", ret);
          }
        }
      }
      return ret;
    }

    //add zt 20151126:b
    int ObPostfixExpression::get_array_var(const ObObj &expr_node, int64_t idx_type, const ObObj &idx_val, const ObObj *&val) const
    {
      int ret = OB_SUCCESS;
      ObResultSet *result_set = owner_op_->get_phy_plan()->get_result_set();
      ObString array_name;
      const ObObj *tmp_obj;
      int64_t idx_value = -1;

      const SpProcedure *proc = NULL;
      if( OB_SUCCESS != (ret = expr_node.get_varchar(array_name)) )
      {
        TBSYS_LOG(USER_ERROR, "Variable %.*s does not exists", array_name.length(), array_name.ptr());
        ret = OB_ERR_VARIABLE_UNKNOWN;
      }
      else if( NULL != (proc = get_running_procedure(owner_op_->get_phy_plan())) )
      { //read array value in procedure
        if( CONST_OBJ == idx_type ) idx_val.get_int(idx_value);
        else if( TEMP_VAR == idx_type )
        {
          ObString var_name;
          idx_val.get_varchar(var_name);

          if( OB_SUCCESS != (ret = proc->read_variable(var_name, tmp_obj)) )
          {
            TBSYS_LOG(WARN, "can not read index variable [%s]", to_cstring(idx_val));
          }
          else if( OB_SUCCESS != (ret = tmp_obj->get_int(idx_value)))
          {
            TBSYS_LOG(WARN, "index variables does not contain in type value, %s", to_cstring(idx_val));
          }
        }

        if( OB_SUCCESS != ret || OB_SUCCESS != (ret = proc->read_variable(array_name, idx_value, val)) )
        {
          TBSYS_LOG(WARN, "read %.*s[%ld] from procedure %p failed", array_name.length(), array_name.ptr(), idx_value, proc);
        }
        else
        {
          TBSYS_LOG(TRACE, "read %.*s[%ld] = %s", array_name.length(), array_name.ptr(), idx_value, to_cstring(*val));
        }
      }
      else if( NULL != result_set )
      { //read array value on the ms outsize of procedure
        if( CONST_OBJ == idx_type ) idx_val.get_int(idx_value);
        else if( TEMP_VAR == idx_type )
        {
          ObString var_name;
          idx_val.get_varchar(var_name);
          if( NULL == (tmp_obj = result_set->get_session()->get_variable_value(var_name)) )
          {
            ret = OB_ERR_VARIABLE_UNKNOWN;
            TBSYS_LOG(WARN, "index variables does not contain int type value, %s", to_cstring(idx_val));
          }
          else if( OB_SUCCESS != (ret = tmp_obj->get_int(idx_value)) )
          {
            TBSYS_LOG(WARN, "index variables does not contain int type value, %s", to_cstring(idx_val));
          }
        }

        if( OB_SUCCESS != ret || OB_SUCCESS != (ret = result_set->get_session()->get_variable_value(array_name, idx_value, val)) )
        {
          TBSYS_LOG(WARN, "read %.*s[%ld] from session_info fail, ret=%d", array_name.length(), array_name.ptr(), idx_value, ret);
        }
        else
        {
          TBSYS_LOG(TRACE, "read %.*s[%ld] = %s", array_name.length(), array_name.ptr(), idx_value, to_cstring(*val));
        }
      }
      else
      {
        TBSYS_LOG(WARN, "can not use array without result or procedure");
        ret = OB_ERR_UNEXPECTED;
      }
      return ret;
    }

    int ObPostfixExpression::is_var_expr(bool &is_type, ObObj &var_name) const
    {
      int ret = OB_SUCCESS;
      if( OB_SUCCESS == (ret = check_expr_type((int64_t)TEMP_VAR, is_type, 3)) )
      {
        if( is_type )
        {
          var_name = expr_.at(1);
        }
      }
      return ret;
    }

    //add zt 20151126:e

    int ObPostfixExpression::is_const_expr(bool &is_type) const
    {
      return check_expr_type((int64_t)CONST_OBJ, is_type, 3);
    }

    int ObPostfixExpression::is_column_index_expr(bool &is_type) const
    {
      return ObPostfixExpression::check_expr_type((int64_t)COLUMN_IDX, is_type, 4);
    }


    int ObPostfixExpression::check_expr_type(const int64_t type_val, bool &is_type, const int64_t stack_size) const
    {
      int err = OB_SUCCESS;
      int64_t expr_type = -1;
      is_type = false;
      if (expr_.count() == stack_size)
      {
        if (ObIntType == expr_[0].get_type())
        {
          if (OB_SUCCESS != (err = expr_[0].get_int(expr_type)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
          }
          else if (type_val == expr_type)
          {
            is_type = true;
          }
        }
      }
      return err;
    }

    int ObPostfixExpression::get_column_index_expr(uint64_t &tid, uint64_t &cid, bool &is_type) const
    {
      int err = OB_SUCCESS;
      int64_t expr_type = -1;
      is_type = false;
      if (4 == expr_.count())
      {
        if (ObIntType == expr_[0].get_type())
        {
          if (OB_SUCCESS != (err = expr_[0].get_int(expr_type)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
          }
          else if ((int64_t)COLUMN_IDX == expr_type)
          {
            if (OB_SUCCESS != (err = expr_[1].get_int((int64_t&)tid)))
            {
              TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            }
            else if (OB_SUCCESS != (err = expr_[2].get_int((int64_t&)cid)))
            {
              TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            }
            else
            {
              is_type = true;
            }
          }
        }
      }
      return err;
    }


    inline static bool test_expr_int(const ObPostfixExpression::ExprArray &expr, int i, int64_t expected)
    {
      int64_t val = 0;
      return ObIntType == expr[i].get_type()
        && OB_SUCCESS == expr[i].get_int(val)
        && expected == val;
    }

    bool ObPostfixExpression::is_equijoin_cond(ExprItem::SqlCellInfo &c1, ExprItem::SqlCellInfo &c2) const
    {
      bool ret = false;
      // COL_IDX|tid|cid|COL_IDX|tid|cid|OP|EQ|2|END
      if (expr_.count() == 10)
      {
        if (test_expr_int(expr_, 0, COLUMN_IDX)
            && test_expr_int(expr_, 3, COLUMN_IDX)
            && test_expr_int(expr_, 6, OP)
            && test_expr_int(expr_, 7, T_OP_EQ)
            && test_expr_int(expr_, 8, 2))
        {
          ret = true;
          int64_t val = 0;
          if (ObIntType == expr_[1].get_type()
              && OB_SUCCESS == expr_[1].get_int(val))
          {
            c1.tid = val;
          }
          else
          {
            ret = false;
          }
          if (ObIntType == expr_[2].get_type()
              && OB_SUCCESS == expr_[2].get_int(val))
          {
            c1.cid = val;
          }
          else
          {
            ret = false;
          }
          if (ObIntType == expr_[4].get_type()
              && OB_SUCCESS == expr_[4].get_int(val))
          {
            c2.tid = val;
          }
          else
          {
            ret = false;
          }
          if (ObIntType == expr_[5].get_type()
              && OB_SUCCESS == expr_[5].get_int(val))
          {
            c2.cid = val;
          }
          else
          {
            ret = false;
          }
        }
      }
      return ret;
    }

    bool ObPostfixExpression::is_simple_condition(bool real_val, uint64_t &column_id, int64_t &cond_op,
                                                  ObObj &const_val, ObPostExprNodeType *val_type) const
    {
      int err = OB_SUCCESS;
      int64_t type_val = -1;
      int64_t v_type = -1;
      int64_t cid = OB_INVALID_ID;
      bool is_simple_cond_type = false;

      do{
        if (expr_.count() == (3+2+3+1)) /*cid(3) + const_operand(2) + operator(3) + end(1) */
        {
          /* (1) cid */
          if (ObIntType != expr_[0].get_type())
          {
            // not int val, pass
            break;
          }
          else if(OB_SUCCESS != (err = expr_[0].get_int(type_val)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val != COLUMN_IDX)
          {
            break;
          }
          else if (OB_SUCCESS != (err = expr_[2].get_int(cid)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          /* (2) const_opr */
          else if (ObIntType != expr_[3].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[3].get_int(v_type)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (v_type != CONST_OBJ && v_type != PARAM_IDX
            && v_type != SYSTEM_VAR && v_type != TEMP_VAR && v_type != CUR_TIME_OP)
          {
            break;
          }
          /* (3) op */
          else if (ObIntType != expr_[5].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[5].get_int(type_val)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val != OP)
          {
            break;
          }
          else if (ObIntType != expr_[6].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[6].get_int(type_val)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if ((type_val < T_OP_EQ || type_val >= T_OP_NE) && (type_val != T_OP_IS))
          {
            break;
          }
          /* (4) result */
          else
          {
            if (real_val)
            {
              const ObObj *var_ptr = NULL;
              if ((err = get_var_obj(static_cast<ObPostExprNodeType>(v_type), expr_[4], var_ptr)) == OB_SUCCESS)
              {
                const_val = *var_ptr;
              }
              else
              {
                TBSYS_LOG(WARN, "Fail to get real value.err=%d", err);
                break;
              }
            }
            else
            {
              const_val = expr_[4];
            }
            if (val_type)
            {
              *val_type = static_cast<ObPostExprNodeType>(v_type);
            }
            column_id = (uint64_t)cid;
            cond_op = type_val;
            is_simple_cond_type = true;
          }
        }
      } while(0);
      return is_simple_cond_type;
    }

    bool ObPostfixExpression::is_simple_between(bool real_val, uint64_t &column_id, int64_t &cond_op,
                                                ObObj &cond_start, ObObj &cond_end) const
    {
      int err = OB_ERROR;
      int64_t type_val1 = -1;
      int64_t type_val2 = -1;
      int64_t cid = OB_INVALID_ID;
      bool is_simple_cond_type = false;
      do{
        if (expr_.count() == (3+2+2+3+1)) /*cid(3) + const_opr(2) + const_opr(2) + operator(3) + end(1) */
        {
          /* (1) cid */
          if (ObIntType != expr_[0].get_type())
          {
            // not int val, pass
            break;
          }
          else if(OB_SUCCESS != (err = expr_[0].get_int(type_val1)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val1 != COLUMN_IDX)
          {
            break;
          }
          else if (OB_SUCCESS != (err = expr_[2].get_int(cid)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          /* (2) const_opr 1 and const_opr 2 */
          else if (ObIntType != expr_[3].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[3].get_int(type_val1)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val1 != CONST_OBJ && type_val1 != PARAM_IDX
            && type_val1 != SYSTEM_VAR && type_val1 != TEMP_VAR && type_val1 != CUR_TIME_OP)
          {
            break;
          }
          else if (ObIntType != expr_[5].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[5].get_int(type_val2)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val2 != CONST_OBJ && type_val2 != PARAM_IDX
            && type_val2 != SYSTEM_VAR && type_val2 != TEMP_VAR && type_val2 != CUR_TIME_OP)
          {
            break;
          }
          else
          {
            if (real_val)
            {
              const ObObj *val = NULL;
              if (OB_SUCCESS == (err = get_var_obj(static_cast<ObPostExprNodeType>(type_val1), expr_[4], val)))
              {
                cond_start = *val;
              }
              else
              {
                break;
              }
              if (OB_SUCCESS == (err = get_var_obj(static_cast<ObPostExprNodeType>(type_val2), expr_[6], val)))
              {
                cond_end = *val;
              }
              else
              {
                break;
              }
            }
            else
            {
              cond_start = expr_[4];
              cond_end = expr_[6];
            }
          }
          /* (3) op */
          if (OB_SUCCESS != err)
          {
            // pass
            break;
          }
          else if (ObIntType != expr_[7].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[7].get_int(type_val2)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val2 != OP)
          {
            break;
          }
          else if (ObIntType != expr_[8].get_type())
          {
            // not int val, pass
            break;
          }
          else if (OB_SUCCESS != (err = expr_[8].get_int(type_val2)))
          {
            TBSYS_LOG(WARN, "fail to get int value.err=%d", err);
            break;
          }
          else if (type_val2 != T_OP_BTW)
          {
            break;
          }
          else
          {
            /* result */
            column_id = (uint64_t)cid;
            cond_op = type_val2;
            is_simple_cond_type = true;
            err = OB_SUCCESS;
          }
        }
        else
        {
          TBSYS_LOG(DEBUG, "not a valid simple between function");
          err = OB_INVALID_ARGUMENT;
        }
      } while(0);
      return is_simple_cond_type;
    }

    //add zt 20151109:b
    int ObPostfixExpression::serialize_variables(
        char *buf, const int64_t buf_len, int64_t &pos,
        int64_t type, const ObObj &expr_node) const
    {
      int ret = OB_SUCCESS;
      /*
        * PARAM_IDX could be resolved on ups
        * we send the real value to ups
        */
      if( !owner_op_->get_phy_plan()->is_group_exec() || type == PARAM_IDX )
      { //normal execution path
        if( type == ARRAY_VAR )
        {
          TBSYS_LOG(WARN, "serialize array variable outsize of stored procedure is not supported");
          ret = OB_NOT_SUPPORTED;
        }
        else
        {
          ObObj new_type;
          new_type.set_int(CONST_OBJ);
          const ObObj *val = NULL;
          if (OB_SUCCESS != (ret = new_type.serialize(buf, buf_len, pos)))
          {
            TBSYS_LOG(WARN, "Fail to serialize type CONST_OBJ");
          }
          else if (OB_SUCCESS != (ret = get_var_obj(static_cast<ObPostExprNodeType>(type), expr_node, val)))
          {
            ret = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN,"Get value ObObj failed [err:%d]", ret);
          }
          else if (OB_SUCCESS != (ret = val->serialize(buf, buf_len, pos)))
          {
            TBSYS_LOG(WARN, "fail to serialize expr. ret=%d", ret);
          }
        }
      }
      else
      { //procedure execution path
        ObObj new_type;
        new_type.set_int(type);
        if (OB_SUCCESS != (ret = new_type.serialize(buf, buf_len, pos)))
        {
          TBSYS_LOG(WARN, "Fail to serialize type %ld", type);
        }
        else if (OB_SUCCESS != (ret = expr_node.serialize(buf, buf_len, pos)))
        {
          TBSYS_LOG(WARN, "fail to serialize expr. ret=%d", ret);
        }
      }
      return ret;
    }
    //add zt 20151109:e


    DEFINE_SERIALIZE(ObPostfixExpression)
    {
      int ret = OB_SUCCESS;
      int i = 0;
      ObObj obj;
      obj.set_int(expr_.count());

      if (OB_SUCCESS != (ret = obj.serialize(buf, buf_len, pos)))
      {
        TBSYS_LOG(WARN, "fail to serialize postfix expression size. ret=%d", ret);
      }
      else
      {
        for (i = 0; i < expr_.count(); i++)
        {
          int64_t type = BEGIN_TYPE;
          int64_t num = 0;
          if (OB_SUCCESS != (ret = expr_[i].get_int(type)))
          {
            TBSYS_LOG(WARN, "Fail to get type. unexpected! ret=%d idx=%d", ret, i);
            ret = OB_ERR_UNEXPECTED;
            break;
          }
          //add zt 20151121:b
          else if( type == CUR_TIME_OP )
          {
            ObObj new_type;
            new_type.set_int(CONST_OBJ);
            const ObObj *val = NULL;
            if (OB_SUCCESS != (ret = new_type.serialize(buf, buf_len, pos)))
            {
              TBSYS_LOG(WARN, "Fail to serialize type CONST_OBJ");
              break;
            }
            else if (i >= expr_.count() - 1
              || OB_SUCCESS != (ret = get_var_obj(static_cast<ObPostExprNodeType>(type), expr_[++i], val)))
            {
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN,"Get value ObObj failed [err:%d]", ret);
              break;
            }
            else if (OB_SUCCESS != (ret = val->serialize(buf, buf_len, pos)))
            {
              TBSYS_LOG(WARN, "fail to serialize expr[%d]. ret=%d", i, ret);
              break;
            }
          }
          //add zt 20151121:e
          //else if( type == PARAM_IDX || type == SYSTEM_VAR || type == TEMP_VAR || type == CUR_TIME_OP )  //delete by zt
          else if (type == PARAM_IDX || type == SYSTEM_VAR || type == TEMP_VAR || type == ARRAY_VAR) //add by zt
          {
            //delete zt 20151109:b
//            ObObj new_type;
//            new_type.set_int(CONST_OBJ);
//            const ObObj *val = NULL;
//            if (OB_SUCCESS != (ret = new_type.serialize(buf, buf_len, pos)))
//            {
//              TBSYS_LOG(WARN, "Fail to serialize type CONST_OBJ");
//              break;
//            }
//            else if (i >= expr_.count() - 1
//              || OB_SUCCESS != (ret = get_var_obj(static_cast<ObPostExprNodeType>(type), expr_[++i], val)))
//            {
//              ret = OB_ERR_UNEXPECTED;
//              TBSYS_LOG(WARN,"Get value ObObj failed [err:%d]", ret);
//              break;
//            }
//            else if (OB_SUCCESS != (ret = val->serialize(buf, buf_len, pos)))
//            {
//              TBSYS_LOG(WARN, "fail to serialize expr[%d]. ret=%d", i, ret);
//              break;
//            }
            //delte zt 20151109:e
            //add zt 20151109:b
            if( i >= expr_.count() - 1 )
            {
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN, "Get value ObObj failed [err:%d]", ret);
              break;
            }
            else if( OB_SUCCESS != (ret = serialize_variables(buf, buf_len, pos, type, expr_[++i])) )
            {
              //serialize variables error here
              TBSYS_LOG(WARN, "fail to serialize expr[%d]. ret=%d", i, ret);
              break;
            }
            //add zt 20151109:e
            continue;
          }
          else if (type == OP)
          {
            num = 3;
            int64_t op_type = 0;
            if (OB_SUCCESS != (ret = expr_[i+1].get_int(op_type)))
            {
              TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d idx=%d", ret, i+1);
              ret = OB_ERR_UNEXPECTED;
              break;
            }
            else if (T_FUN_SYS == op_type)
            {
              ++num;
            }
          }
          else if (type == COLUMN_IDX)
          {
            num = 3;
          }
          // mod weixing [implementation of sub_query]20160119:b
          //else if (type == CONST_OBJ)
          else if (type == CONST_OBJ||type == QUERY_ID)
          //mod e
          {
            num = 2;
          }
          else if (type == END || type == UPS_TIME_OP)
          {
            num = 1;
          }
          else
          {
            TBSYS_LOG(WARN, "Unkown type %ld", type);
            ret = OB_ERR_UNEXPECTED;
            break;
          }
          for (int64_t j = 0; j < num; j++)
          {
            if (i >= expr_.count()
              || OB_SUCCESS != (ret = expr_[i].serialize(buf, buf_len, pos)))
            {
              ret = OB_ERR_UNEXPECTED;
              TBSYS_LOG(WARN, "Fail to serialize expr[%d]. ret=%d count=%ld", i, ret, expr_.count());
              break;
            }
            else if (j < num - 1)
            {
              i++;
            }
          }
        }
      }
      return ret;
    }

    // single row expression checker
    // ONLY: (x,x,x) in ((x,x,x))
    bool ObPostfixExpression::is_simple_in_expr(bool real_val, const ObRowkeyInfo &info, ObIArray<ObRowkey> &rowkey_array,
        common::PageArena<ObObj,common::ModulePageAllocator> &rowkey_objs_allocator) const
    {
      int err = OB_SUCCESS;
      int64_t rowkey_column_count = 0;
      int64_t index = 0;
      int64_t len = expr_.count();
      int64_t dim = 0, dim2 = 0;
      int64_t row_count = 0;
      int64_t row = 0;
      int64_t val_idx = 0;
      bool is_simple_expr = false;
      ObRowkey rowkey;
      ObObj *rowkey_objs = NULL;
      ObRowkeyColumn rowkey_column;
      int64_t cid = OB_INVALID_ID;
      if (len > 16)
      {
        // check 1: (OP, T_OP_IN, 2), (END)
        if (!ExprUtil::is_end(expr_.at(len-1)) || !ExprUtil::is_value(expr_.at(len-2), 2L) ||
            !ExprUtil::is_op_of_type(expr_.at(len-3), T_OP_IN) || !ExprUtil::is_op(expr_.at(len-4)))
        {
          // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld. %d, %d, %d, %d", len,
          //    ExprUtil::is_end(expr_.at(len-1)), ExprUtil::is_value(expr_.at(len-2), 2L),
          //    ExprUtil::is_op_of_type(expr_.at(len-3), T_OP_IN), ExprUtil::is_op(expr_.at(len-4)));
        }
        // check 2: (OP, T_OP_ROW, row_count)
        // support getting multi row,
        // for example, (a,b) in ((1,2), (3,4), (5,6)), row_count = 3, dim = 2
        else if (OB_SUCCESS != expr_.at(len-5).get_int(row_count) || !ExprUtil::is_op_of_type(expr_.at(len-6), T_OP_ROW) ||
            !ExprUtil::is_op(expr_.at(len-7)))
        {
          // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld", len);
        }
        // check 3: (OP, T_OP_ROW, dim)
        else if (OB_SUCCESS != expr_.at(len-8).get_int(dim) || !ExprUtil::is_op_of_type(expr_.at(len-9), T_OP_ROW) ||
            !ExprUtil::is_op(expr_.at(len-10)))
        {
          // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld", len);
        }
        else
        {
          const int64_t single_row_len = dim * 2 + 3;
          len = (len - 7) - (row_count * single_row_len);  // support multi row, 'dim' columns, each column takes two objects
          val_idx = len;
          OB_ASSERT(dim > 0);
          OB_ASSERT(len > 6);
          if (dim <= 0 || len <= 6)
          {
            // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld", len);
          }
          // check 4: (OP, T_OP_LEFT_PARAM_END, 2)
          else if (!ExprUtil::is_value(expr_.at(len-1), 2L) || !ExprUtil::is_op_of_type(expr_.at(len-2), T_OP_LEFT_PARAM_END) ||
              !ExprUtil::is_op(expr_.at(len-3)))
          {
            // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld", len);
          }
          // check 5: (OP, T_OP_ROW, ?)
          else if (OB_SUCCESS != expr_.at(len-4).get_int(dim2) || !ExprUtil::is_op_of_type(expr_.at(len-5), T_OP_ROW) ||
              !ExprUtil::is_op(expr_.at(len-6)))
          {
            // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld.%ld, %d, %d", len,
            //    dim2, ExprUtil::is_op_of_type(expr_.at(len-5), T_OP_ROW), ExprUtil::is_op(expr_.at(len-6)));
          }
          else if (dim != dim2)
          {
            // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld. dim=%ld", len, dim);
          }
          else if (dim >= info.get_size())
          {
            len = len - (dim * 3 + 6); // 3 = COLUMN_IDX, TID, CID
            if (len == 0)
            {
              // extra values from expression
              // extra 1: rowkey columns
              int64_t size = info.get_size();
              for (index = 0; index < size && index < dim && OB_SUCCESS == err; index++)
              {
                if (OB_SUCCESS != (err = info.get_column(index, rowkey_column)))
                {
                  TBSYS_LOG(ERROR, "get rowkey column fail. index=%ld, size=%ld", index, size);
                }
                else
                {
                  if (OB_SUCCESS != expr_.at(index * 3 + 2).get_int(cid)) // 3=COLUMN_REF, TID, CID
                  {
                    TBSYS_LOG(ERROR, "fail to get int value from expr_.at(2)");
                  }
                  if (rowkey_column.column_id_ == static_cast<uint64_t>(cid))
                  {
                    rowkey_column_count++;
                  }
                  else
                  {
                    break;
                  }
                }
              }
              if ((OB_SUCCESS == err) && (rowkey_column_count == size))
              {
                if (NULL != (rowkey_objs = rowkey_objs_allocator.alloc(row_count * rowkey_column_count * sizeof(ObObj))))
                {
                  for (row = 0; row < row_count; row++)
                  {
                    // extra 2: values
                    //for (index = 0; index < rowkey_column_count; index++) //modify by zhutao: fix bugs
                    for (index = 0; (OB_SUCCESS == err) && index < rowkey_column_count; index++)
                    {
                      // TODO: check every T_OP_ROW dim, all must be equal. currently skipped this step
                      const int64_t type_offset = val_idx + row * single_row_len + (index * 2);
                      const int64_t val_offset = val_idx + row * single_row_len + (index * 2 + 1); // 2=CONST,VALUE
                      int64_t type = 0;
                      const ObObj *val = NULL;
                      if (OB_SUCCESS != (err = expr_.at(type_offset).get_int(type)))
                      {
                        TBSYS_LOG(ERROR, "Can not get value type. err=%d", err);
                      }
                      else if (real_val)
                      {
                        if (OB_SUCCESS != (err = get_var_obj(static_cast<ObPostExprNodeType>(type), expr_.at(val_offset), val)))
                        {
                          TBSYS_LOG(ERROR, "Can not get value. err=%d", err);
                        }
                        else
                        {
                          rowkey_objs[index] = *val;
                        }
                      }
                      else
                      {
                        rowkey_objs[index] = expr_.at(val_offset);
                      }
                      // TBSYS_LOG(DEBUG, "index=%ld, at=%ld, val=%s", index, offset, to_cstring(rowkey_objs[index]));
                    }

                    if( OB_SUCCESS == err ) //add by zhutao: fix bugs
                    {
                      rowkey.assign(rowkey_objs, rowkey_column_count);
                      if (OB_SUCCESS!= (err = rowkey_array.push_back(rowkey)))
                      {
                        TBSYS_LOG(ERROR, "fail to push rowkey to array. err=%d", err);
                      }
                      rowkey_objs += rowkey_column_count;
                    }
                  }
                }
                else
                {
                  TBSYS_LOG(ERROR, "fail to alloc memory");
                  err = OB_ALLOCATE_MEMORY_FAILED;
                }
                if (OB_SUCCESS == err)
                {
                  is_simple_expr = true;
                }
              }
            }
            else
            {
              // TBSYS_LOG(DEBUG, "not simple in expr. len=%ld. dim=%ld", len, dim);
            }
          }
        }
      }
      return is_simple_expr;
    }

    DEFINE_DESERIALIZE(ObPostfixExpression)
    {
      int ret = OB_SUCCESS;
      int i = 0;
      ObObj obj;
      ObObj obj2;
      int64_t val = 0;
      reset();
      if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))
      {
        TBSYS_LOG(WARN, "fail to deserialize obj. ret=%d. buf=%p, data_len=%ld, pos=%ld",
            ret, buf, data_len, pos);
      }
      else if (ObIntType != obj.get_type())
      {
        TBSYS_LOG(WARN, "unexpected obj type. actual type:%d, expected:%d", obj.get_type(), ObIntType);
        ret = OB_ERR_UNEXPECTED;
      }
      else
      {
        if ((OB_SUCCESS != (ret = obj.get_int(val))) || (val <= 0))
        {
          TBSYS_LOG(WARN, "fail to get int value. ret=%d, expr_.count()%ld", ret, val);
        }
        else
        {
          int expr_count = static_cast<int32_t>(val);
          for (i = 0; i < expr_count; i++)
          {
            if (OB_SUCCESS != (ret = obj.deserialize(buf, data_len, pos)))    //xsl序列化入口
            {
              TBSYS_LOG(WARN, "fail to deserialize obj. ret=%d. buf=%p, data_len=%ld, pos=%ld",
                  ret, buf, data_len, pos);
              break;
            }
            else if (ObVarcharType == obj.get_type())
            {
              if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
              {
                TBSYS_LOG(WARN, "fail to write object to string buffer. ret=%d", ret);
              }
              else if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
              {
                TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
              }
            }
            //add  fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
            else if(ObDecimalType==obj.get_type())
            {
                if (OB_SUCCESS != (ret = str_buf_.write_obj(obj, &obj2)))
                {
                    TBSYS_LOG(WARN, "fail to write object to string buffer. ret=%d", ret);
                }
                else
                {
                    // TBSYS_LOG(WARN, "test::fan2=,,,,,p=%d,,,s=%d",obj.get_precision() ,obj.get_scale());
                    obj2.set_precision(obj.get_precision());
                    obj2.set_scale(obj.get_scale());
                    if (OB_SUCCESS != (ret = expr_.push_back(obj2)))
                    {
                        TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
                    }
                }
                //TBSYS_LOG(ERROR,"DEFINE_DESERIALIZE");
            }
            //add e
            else
            {
              if (OB_SUCCESS != (ret = expr_.push_back(obj)))
              {
                TBSYS_LOG(WARN, "failed to add item, err=%d", ret);
              }
            }
          }
        }
      }
      return ret;
    }

    //TODO COMMENT BY ZT, serialize methods have changed, we have to update the get_serialize methods
    DEFINE_GET_SERIALIZE_SIZE(ObPostfixExpression)
    {
      int64_t size = 0;
      ObObj obj;
      obj.set_int(expr_.count());
      size += obj.get_serialize_size();
      for (int i = 0; i < expr_.count(); i++)
      {
        int64_t type = BEGIN_TYPE;
        int64_t num = 0;
        if (OB_SUCCESS != expr_[i].get_int(type))
        {
          TBSYS_LOG(WARN, "Fail to get type. unexpected! idx=%d", i);
          break;
        }
        else if (type == PARAM_IDX || type == SYSTEM_VAR || type == TEMP_VAR || type == CUR_TIME_OP)
        {
          ObObj new_type;
          new_type.set_int(CONST_OBJ);
          const ObObj *val = NULL;
          size += new_type.get_serialize_size();
          if (i >= expr_.count() - 1
            || OB_SUCCESS != get_var_obj(static_cast<ObPostExprNodeType>(type), expr_[++i], val))
          {
            TBSYS_LOG(WARN,"Get value ObObj failed ");
            break;
          }
          size += val->get_serialize_size();
          continue;
        }
        else if (type == OP)
        {
          num = 3;
          int64_t op_type = 0;
          if (OB_SUCCESS != expr_[i + 1].get_int(op_type))
          {
            TBSYS_LOG(WARN, "Fail to get op type. unexpected! idx=%d", i+1);
            break;
          }
          else if (T_FUN_SYS == op_type)
          {
            ++num;
          }
        }
        else if (type == COLUMN_IDX)
        {
          num = 3;
        }
        else if (type == CONST_OBJ)
        {
          num = 2;
        }
        else if (type == END || type == UPS_TIME_OP)
        {
          num = 1;
        }
        else
        {
          TBSYS_LOG(WARN, "Unkown type %ld", type);
          break;
        }
        for (int64_t j = 0; j < num; j++)
        {
          if (i >= expr_.count())
          {
            TBSYS_LOG(WARN, "Fail to serialize expr[%d]. count=%ld", i, expr_.count());
            break;
          }
          else
          {
            size += expr_[i].get_serialize_size();
          }
          if (j < num - 1)
          {
            i++;
          }
        }
      }
      return size;
    }

    /************************************************************************/
    /*****************   function implementation     ************************/
    /************************************************************************/
    inline int ObPostfixExpression::nop_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      UNUSED(params);
      UNUSED(stack_i);
      UNUSED(idx_i);
      UNUSED(result);
      TBSYS_LOG(WARN, "function not implemented!");
      return OB_NOT_IMPLEMENT;
    }

    inline int ObPostfixExpression::reserved_func(const ObExprObj &obj1, const ObExprObj &obj2, ObExprObj &result)
    {
      int err = OB_INVALID_ARGUMENT;
      UNUSED(obj1);
      UNUSED(obj2);
      UNUSED(result);
      return err;
    }


    /* compare function list:
     * >   gt_func
     * >=  ge_func
     * <=  le_func
     * <   lt_func
     * ==  eq_func
     * !=  neq_func
     */
    inline int ObPostfixExpression::gt_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].gt(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::ge_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].ge(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::le_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].le(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::lt_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].lt(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::eq_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].eq(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::neq_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].ne(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::is_not_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].is_not(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::is_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].is(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }


    inline int ObPostfixExpression::add_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].add(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }


    inline int ObPostfixExpression::sub_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].sub(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }


    inline int ObPostfixExpression::mul_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].mul(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }


    inline int ObPostfixExpression::div_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].div(stack_i[idx_i-1], result, params.did_int_div_as_double_);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::mod_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].mod(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }


    inline int ObPostfixExpression::and_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].land(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::or_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].lor(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::minus_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 1)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-1].negate(result);
        idx_i -= 1;
      }
      return err;
    }


    inline int ObPostfixExpression::plus_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 1)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        // don't touch it whatever the type is
        result = stack_i[idx_i-1];
        idx_i -= 1;
      }
      return err;
    }



    inline int ObPostfixExpression::not_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 1)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-1].lnot(result);
        idx_i -= 1;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_substr(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      ObString varchar;
      ObString res_varchar;

      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        //TBSYS_LOG(INFO, "params.operand_count_ = %d, [3]=%s, [2]=%s, [1]=%s",
        //    params.operand_count_, to_cstring(stack_i[idx_i-3]), to_cstring(stack_i[idx_i-2]), to_cstring(stack_i[idx_i-1]));

        if (params.operand_count_ == 2)
        {
          err = stack_i[idx_i-2].substr(stack_i[idx_i-1], result, *params.str_buf_);
        }
        else if (params.operand_count_ == 3)
        {
          err = stack_i[idx_i-3].substr(stack_i[idx_i-2], stack_i[idx_i-1], result, *params.str_buf_);
        }
        else
        {
          err = OB_ERR_UNEXPECTED;
          TBSYS_LOG(WARN, " unexpected operand count %d", params.operand_count_);
        }
        idx_i -= params.operand_count_;
      }
      return err;

    }
    inline int ObPostfixExpression::like_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        err = stack_i[idx_i-2].like(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    inline int ObPostfixExpression::left_param_end_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      int dim = params.operand_count_;
      if (OB_SUCCESS != (ret = const_cast<ObPostExprExtraParams &>(params).in_row_operator_.push_row(stack_i, idx_i, 1)))
      {
        TBSYS_LOG(WARN, "fail to push row into in_row_operator_. ret=%d", ret);
      }
      else
      {
        result.set_int((int64_t)dim);
      }
      return ret;
    }

    inline int ObPostfixExpression::row_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      int row_count = params.operand_count_;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < row_count)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d, row_count=%d", idx_i, row_count);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (ret = const_cast<ObPostExprExtraParams &>(params).in_row_operator_.push_row(stack_i, idx_i, row_count)))
        {
          TBSYS_LOG(WARN, "fail to push row to row operator. ret=%d", ret);
        }
        else
        {
          //TBSYS_LOG(INFO, "idx_i=%d, row_count=%d", idx_i, row_count);
          result.set_int((int64_t)row_count);
        }
      }
      return ret;
    }

    inline int ObPostfixExpression::in_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      // in的算��
      //
      // ��.
      // 2 IN (3, 4) 的后缀表达式为
      // [栈顶] 2, 3, 4, Row(2), IN(2)
      // in_row_operator_中数据的layout为：
      // 2 3 4
      // width = 1
      //
      // ��.
      // (1, 3) IN ((3, 4), (1, 2))的后缀表达式为
      // [栈顶] 1, 3, Row(2), 3, 4, Row(2), 1, 2, Row(2), Row(2), IN(2)
      // in_row_operator_中数据的layout为：
      // 1 3 3 4 1 2
      // width = 2
      //
      // 显然，根据width，取出前width个数，逐个往后比较即可计算得到in的结��
      // note: 1. 该后缀表达式计算中，左操作数先出栈 2. 最后一个T_OP_ROW操作被实际上忽略
      //
      int ret = OB_SUCCESS;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (ret = const_cast<ObPostExprExtraParams &>(params).in_row_operator_.get_result(stack_i, idx_i, result)))
        {
          TBSYS_LOG(WARN, "fail to get IN operation result. ret=%d", ret);
        }
      }
      return ret;
    }

    inline int ObPostfixExpression::not_in_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = in_func(stack_i, idx_i, result, params);
      result.lnot(result);
      return ret;
    }

    //add weixing [implementatioin of sub_query]20160116
    inline int ObPostfixExpression::in_sub_query_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params, ObBloomFilterV1* bloom_filter, hash::ObHashMap<common::ObRowkey, common::ObRowkey, common::hash::NoPthreadDefendMode>* hash_map, int &sub_query_idx, bool second_check)
    {
      int ret = OB_SUCCESS;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
         int64_t right_elem_count = 0;
         if (OB_SUCCESS != (ret = stack_i[idx_i-1].get_int(right_elem_count)))
        {
          TBSYS_LOG(WARN, "fail to get_int from stack. top=%d, ret=%d", idx_i, ret);
        }
        else if(-1 == right_elem_count)
        {

          if(OB_SUCCESS != (ret = get_result(stack_i, idx_i, result, params, bloom_filter, hash_map, sub_query_idx, second_check)))
          {
            TBSYS_LOG(WARN, "fail to get IN operation result. ret=%d", ret);
          }

        }
        else
        {
          //should not be here
          TBSYS_LOG(WARN, "invialid argument");
          ret = OB_INVALID_ARGUMENT;
        }
      }
      return ret;
    }
    //add e

    int ObPostfixExpression::not_like_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-2].not_like(stack_i[idx_i-1], result);
        idx_i -= 2;
      }
      return err;
    }

    int ObPostfixExpression::btw_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 3)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-3].btw(stack_i[idx_i-2], stack_i[idx_i-1], result);
        idx_i -= 3;
      }
      return ret;
    }

    int ObPostfixExpression::not_btw_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      UNUSED(params);
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 3)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-3].not_btw(stack_i[idx_i-2], stack_i[idx_i-1], result);
        idx_i -= 3;
      }
      return ret;
    }

    const char * ObPostfixExpression::get_sys_func_name(enum ObSqlSysFunc func_id)
    {
      const char *ret;
      if (func_id >= SYS_FUNC_LENGTH && func_id < SYS_FUNC_NUM)
      {
        ret = SYS_FUNCS_NAME[func_id];
      }
      else
      {
        ret = "";
        TBSYS_LOG(ERROR, "unexpected sql sys func type %d", func_id);
      }
      return ret;
    }

    int ObPostfixExpression::get_sys_func_param_num(const ObString& name, int32_t& param_num)
    {
      int ret = OB_SUCCESS;
      int32_t i = 0;
      for (i = SYS_FUNC_LENGTH; i < SYS_FUNC_NUM; i++)
      {
        if (static_cast<int32_t>(strlen(SYS_FUNCS_NAME[i])) == name.length()
          && strncasecmp(SYS_FUNCS_NAME[i], name.ptr(), name.length()) == 0)
          break;
      }
      if (i >= SYS_FUNC_LENGTH && i < SYS_FUNC_NUM)
      {
        param_num = SYS_FUNCS_ARGS_NUM[i];
      }
      else
      {
        ret = OB_ERR_FUNCTION_UNKNOWN;
        TBSYS_LOG(WARN, "Unknown function '%.*s', ret=%d", name.length(), name.ptr(), ret);
      }
      return ret;
    }

    inline int ObPostfixExpression::sys_func_ip_to_int(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (OB_UNLIKELY(NULL == stack_i))
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_UNLIKELY(idx_i < 1))
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i - 1].ip_to_int(result);
        idx_i -= 1;
      }
      return err;
    }
    inline int ObPostfixExpression::sys_func_int_to_ip(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (OB_UNLIKELY(NULL == stack_i))
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_UNLIKELY(idx_i < 1))
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i - 1].int_to_ip(result, *(params.str_buf_));
        idx_i -= 1;
      }
      return err;
    }
    inline int ObPostfixExpression::sys_func_unhex(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (OB_UNLIKELY(NULL == stack_i))
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_UNLIKELY(idx_i < 1))
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i - 1].unhex(result, *(params.str_buf_));
        idx_i -= 1;
      }
      return err;
    }
    inline int ObPostfixExpression::sys_func_hex(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (OB_UNLIKELY(NULL == stack_i))
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_UNLIKELY(idx_i < 1))
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i - 1].hex(result, *(params.str_buf_));
        idx_i -= 1;
      }
      return err;
    }
    inline int ObPostfixExpression::sys_func_length(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      UNUSED(params);
      if (OB_UNLIKELY(NULL == stack_i))
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (OB_UNLIKELY(idx_i < 1))
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        stack_i[idx_i-1].varchar_length(result);
        idx_i -= 1;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_cast(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
        //modify fanqiushi ECNU_DECIMAL V0.1 2016_5_29:b
        /*int err = OB_SUCCESS;
        if (OB_UNLIKELY(NULL == stack_i || NULL == params.str_buf_))
        {
          TBSYS_LOG(WARN, "stack_i=%p, str_buf_=%p.", stack_i, params.str_buf_);
          err = OB_INVALID_ARGUMENT;
        }
        else if (OB_UNLIKELY(idx_i < 2))
        {
          TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
          err = OB_INVALID_ARGUMENT;
        }
        else
        {
          int64_t literal_type = 0;
          int32_t dest_type = 0;
          if (OB_SUCCESS != (err = stack_i[idx_i-1].get_int(literal_type)))
          {
            TBSYS_LOG(WARN, "fail to get int value. actual type = %d. err=%d", stack_i[idx_i-1].get_type(), err);
          }
          else
          {
            // convert literal data type to inner data type
            dest_type = convert_item_type_to_obj_type(static_cast<ObItemType>(literal_type));
            if (OB_SUCCESS == err)
            {
              if (OB_SUCCESS != (err = stack_i[idx_i-2].cast_to(dest_type, result, *params.str_buf_)))
              {
                TBSYS_LOG(WARN, "fail to cast data from type %d to type %d. err=%d", stack_i[idx_i-2].get_type(), dest_type, err);
              }
            }
          }
          idx_i -= 2;
        }
        return err;*/          //old code
        int err = OB_SUCCESS;
        if (OB_UNLIKELY(NULL == stack_i || NULL == params.str_buf_))
        {
          TBSYS_LOG(WARN, "stack_i=%p, str_buf_=%p.", stack_i, params.str_buf_);
          err = OB_INVALID_ARGUMENT;
        }
        else if (OB_UNLIKELY(idx_i < 2))
        {
          TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
          err = OB_INVALID_ARGUMENT;
        }
        else
        {
          int64_t literal_type = 0;
          int32_t dest_type = 0;
          if(params.operand_count_==4)
          {
              if (OB_SUCCESS != (err = stack_i[idx_i-3].get_int(literal_type)))
              {
              TBSYS_LOG(WARN, "fail to get int value. actual type = %d. err=%d", stack_i[idx_i-3].get_type(), err);
              }
              else
              {
                  // convert literal data type to inner data type
                  dest_type = convert_item_type_to_obj_type(static_cast<ObItemType>(literal_type));
                  int64_t pre=0;
                  int64_t scale=0;
                  stack_i[idx_i-2].get_int(pre);
                  stack_i[idx_i-1].get_int(scale);
                  if (OB_SUCCESS == err)
                  {
                          if (OB_SUCCESS != (err = stack_i[idx_i-4].cast_toV2(dest_type, result, *params.str_buf_,(uint32_t)(pre),(uint32_t)(scale))))
                          {
                              TBSYS_LOG(WARN, "fail to cast data from type %d to type %d. err=%d", stack_i[idx_i-4].get_type(), dest_type, err);
                          }
                  }
              }
              idx_i -= 4;
          }
          else
          {

              if (OB_SUCCESS != (err = stack_i[idx_i-1].get_int(literal_type)))
              {
              TBSYS_LOG(WARN, "fail to get int value. actual type = %d. err=%d", stack_i[idx_i-1].get_type(), err);
              }
              else
              {
            // convert literal data type to inner data type
              dest_type = convert_item_type_to_obj_type(static_cast<ObItemType>(literal_type));
              if (OB_SUCCESS == err)
              {
                  if (OB_SUCCESS != (err = stack_i[idx_i-2].cast_to(dest_type, result, *params.str_buf_)))
                  {
                TBSYS_LOG(WARN, "fail to cast data from type %d to type %d. err=%d", stack_i[idx_i-2].get_type(), dest_type, err);
                  }
              }
              }
              idx_i -= 2;
          }
        }
        return err;
        //modify:e
    }

    inline int ObPostfixExpression::sys_func_current_timestamp(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      UNUSED(stack_i);
      UNUSED(idx_i);
      UNUSED(params);

      result.set_precise_datetime(tbsys::CTimeUtil::getTime());
      return OB_SUCCESS;
    }

    inline int ObPostfixExpression::sys_func_cur_user(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      UNUSED(stack_i);
      UNUSED(idx_i);
      UNUSED(result);
      UNUSED(params);
      return OB_NOT_SUPPORTED;
    }

    inline int ObPostfixExpression::sys_func_trim(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      if (NULL == stack_i || NULL == params.str_buf_)
      {
        TBSYS_LOG(WARN, "stack_i=%p, str_buf_=%p.", stack_i, params.str_buf_);
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 3)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (err = stack_i[idx_i-1].trim(stack_i[idx_i-3], stack_i[idx_i-2], result, *params.str_buf_)))
        {
          TBSYS_LOG(WARN, "fail to trim value. err=%d", err);
        }
        idx_i -= 3;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_lower(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      if (NULL == stack_i || NULL == params.str_buf_)
      {
        TBSYS_LOG(WARN, "stack_i=%p, str_buf_=%p.", stack_i, params.str_buf_);
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 1)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (err = stack_i[idx_i-1].lower_case(result, *params.str_buf_)))
        {
          TBSYS_LOG(WARN, "fail to get lower value. err=%d", err);
        }
        idx_i -= 1;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_upper(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      if (NULL == stack_i || NULL == params.str_buf_)
      {
        TBSYS_LOG(WARN, "stack_i=%p, str_buf_=%p.", stack_i, params.str_buf_);
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 1)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (err = stack_i[idx_i-1].upper_case(result, *params.str_buf_)))
        {
          TBSYS_LOG(WARN, "fail to get lower value. err=%d", err);
        }
        idx_i -= 1;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_coalesce(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      ObObj obj;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        result.set_null();
        for (int i = params.operand_count_; i > 0; i--)
        {
          if (!stack_i[idx_i - i].is_null())
          {
            if (OB_SUCCESS != (err = stack_i[idx_i - i].to(obj)))
            {
              TBSYS_LOG(WARN, "fail to copy object. err=%d", err);
            }
            else
            {
              result.assign(obj);
            }
            break;
          }
        }
        idx_i -= params.operand_count_;
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_greatest(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      ObObj obj;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        int i = params.operand_count_;
        int cmp = 0;
        ObExprObj *greatest = NULL;
        result.set_null();
        
        if (i > 0)
        {
          greatest = &stack_i[idx_i - i];
          i--;
        }

        for (; i > 0; i--)
        {
          if (OB_SUCCESS == (err = stack_i[idx_i-i].compare(*greatest, cmp)))
          {
            if (0 < cmp)
            {
              greatest = &stack_i[idx_i-i];
            }
          }
          else
          {
            break;
          }
        }
        if (OB_SUCCESS == err)
        {
          if (NULL != greatest)
          {
            greatest->to(obj);
            result.assign(obj);
            idx_i -= params.operand_count_;
          }
          else
          {
            err = OB_INVALID_ARGUMENT;
          }
        }
        else if (OB_RESULT_UNKNOWN == err)
        {
          result.set_null(); // default is null
          idx_i -= params.operand_count_;
          err = OB_SUCCESS;
        }
      }
      return err;
    }

    inline int ObPostfixExpression::sys_func_least(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      ObObj obj;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        int i = params.operand_count_;
        int cmp = 0;
        ObExprObj *greatest = NULL;
        result.set_null();
        
        if (i > 0)
        {
          greatest = &stack_i[idx_i - i];
          i--;
        }

        for (; i > 0; i--)
        {
          if (OB_SUCCESS == (err = stack_i[idx_i-i].compare(*greatest, cmp)))
          {
            if (0 > cmp)
            {
              greatest = &stack_i[idx_i-i];
            }
          }
          else
          {
            break;
          }
        }
        if (OB_SUCCESS == err)
        {
          if (NULL != greatest)
          {
            greatest->to(obj);
            result.assign(obj);
            idx_i -= params.operand_count_;
          }
          else
          {
            err = OB_INVALID_ARGUMENT;
          }
        }
        else if (OB_RESULT_UNKNOWN == err)
        {
          result.set_null(); // default is null
          idx_i -= params.operand_count_;
          err = OB_SUCCESS;
        }
      }
      return err;
    }


    inline int ObPostfixExpression::concat_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int err = OB_SUCCESS;
      if (OB_UNLIKELY(NULL == stack_i || NULL == params.str_buf_))
      {
        TBSYS_LOG(WARN, "stack pointer=%p, str_buf_=%p", stack_i, params.str_buf_);
        err = OB_INVALID_ARGUMENT;
      }
      else if (idx_i < 2)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d", idx_i);
        err = OB_INVALID_ARGUMENT;
      }
      else
      {
        if (OB_SUCCESS != (err = stack_i[idx_i-2].concat(stack_i[idx_i-1], result, *params.str_buf_)))
        {
          err = OB_SUCCESS;
          result.set_null();
        }
        idx_i -= 2;
      }
      return err;
    }

    //add weixing [implementation of sub_query]20160116
    int ObPostfixExpression::get_result(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params,
    common::ObBloomFilterV1* bloom_filter,
    hash::ObHashMap<common::ObRowkey,
    common::ObRowkey,
    common::hash::NoPthreadDefendMode>* hash_map, int &sub_query_idx, bool second_check)
    {
        int ret = OB_SUCCESS;
        ObArray<ObExprObj> &vec_ = const_cast<ObPostExprExtraParams &>(params).in_row_operator_.vec_;
        int64_t left_start_idx = 0;
        int64_t right_elem_count = 0;
        int64_t width = 0;
        int64_t dim = 0;
        int64_t vec_top = 0;
        ObExprObj width_obj;

        OB_ASSERT(NULL != stack_i);
        OB_ASSERT(idx_i >= 2);

        if (OB_SUCCESS != (ret = stack_i[--idx_i].get_int(right_elem_count)))
        {
          TBSYS_LOG(WARN, "fail to get_int from stack. top=%d, ret=%d", idx_i, ret);
        }
        else if (OB_SUCCESS != (ret = stack_i[--idx_i].get_int(dim)))
        {
          TBSYS_LOG(WARN, "fail to get_int from stack. top=%d, ret=%d", idx_i, ret);
        }
        else
        {
          vec_top = vec_.count();
          if (OB_SUCCESS != (ret = vec_.at(vec_top - 1, width_obj)))
          {
            TBSYS_LOG(WARN, "fail to get width_obj from array. vec_top=%ld, ret=%d", vec_top, ret);
          }
          else if (OB_SUCCESS != (ret = width_obj.get_int(width)))
          {
           TBSYS_LOG(WARN, "fail to get_int from stack. top=%d, ret=%d", idx_i, ret);
          }
          else
          {
            left_start_idx  = vec_top - 1 - width;
            //Reverse order output
            ObObj *value = new ObObj[width];
            for(int i = 0; i < width; i++)
            {
              ObExprObj temp_obj;
              ObObj temp_value;
              if (OB_SUCCESS != (ret = vec_.at(left_start_idx+i, temp_obj)))
              {
                TBSYS_LOG(WARN, "fail to get width_obj from array. vec_top=%ld, ret=%d", vec_top, ret);
              }
              else if(OB_SUCCESS != (ret = temp_obj.to(temp_value)))
              {
                TBSYS_LOG(WARN, "fail to chang ObExprObj to ObObj. ");
              }
              value[width-1-i] = temp_value;
            }

            if(OB_SUCCESS != ret )
            {
              TBSYS_LOG(WARN, "get result ,get data error");
            }
            else if(second_check)
            {
              ObRowkey p;
              p.assign(value, width);
              if(NULL != hash_map[sub_query_idx ].get(p) )
              {
                result.set_bool(true);
              }
              else
              {
                result.set_bool(false);
              }
              sub_query_idx ++;
            }
            else if(NULL == bloom_filter)
            {
              TBSYS_LOG(WARN,"bloom filter should not be null");
            }
            else
            {
              if(bloom_filter->may_contain(value, width) )
              {
                result.set_bool(true);
              }
              else
              {
                result.set_bool(false);
              }
            }

            delete [] value;
            vec_.pop_back();//left param num

            for(int i = 0; i < width; i++)
            {
              vec_.pop_back();//left param
            }
          }
        }
     return ret;
    }
    //add e

    int ObPostfixExpression::arg_case_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      int arg_pos = idx_i - params.operand_count_;
      ObExprObj cmp;
      int i = 0;
      bool match_when = false;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (params.operand_count_ < 3 || idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d. expect more than:%d", idx_i, params.operand_count_);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // case 1:
        // case c1 when 10 then expr1 else expr2  ==postfix stack==>
        // c1(idx_i-4), 10(idx_i-3), expr1(idx_i-2), expr2(idx_i-1), T_OP_ARG_CASE
        // params.operand_count_ = 4
        //
        // case 2:
        // case c1 when 10 then expr1  ==postfix stack==>
        // c1(idx_i-3), 10(idx_i-2), expr1(idx_i-1), T_OP_ARG_CASE
        // params.operand_count_ = 3
        //
        for (i = idx_i - (params.operand_count_ - 1); i < idx_i - 1; i += 2)
        {
          stack_i[arg_pos].eq(stack_i[i], cmp);
          //TBSYS_LOG(DEBUG, "i=%d,arg_pos=%d,cmp_case_when(%s,%s)", i, arg_pos, to_cstring(stack_i[arg_pos]), to_cstring(stack_i[i]));
          if (cmp.is_true()) // match when
          {
            result = stack_i[i+1];
            match_when = true;
            break;
          }
        }
        if (false == match_when)
        {
          if (params.operand_count_ % 2 == 0)
          {
            result = stack_i[idx_i - 1]; // match else
          }
          else
          {
            result.set_null(); // no else in expression, then match nothing
          }
        }
        if (OB_SUCCESS == ret)
        {
          idx_i -= params.operand_count_;
        }
      }
      return ret;
    }

    int ObPostfixExpression::case_func(ObExprObj *stack_i, int &idx_i, ObExprObj &result, const ObPostExprExtraParams &params)
    {
      int ret = OB_SUCCESS;
      int i = 0;
      bool match_when = false;
      if (NULL == stack_i)
      {
        TBSYS_LOG(WARN, "stack pointer is NULL.");
        ret = OB_INVALID_ARGUMENT;
      }
      else if (params.operand_count_ < 3 || idx_i < params.operand_count_)
      {
        TBSYS_LOG(WARN, "no enough operand in the stack. current size:%d. expect more than:%d", idx_i, params.operand_count_);
        ret = OB_INVALID_ARGUMENT;
      }
      else
      {
        // case 1:
        // case when false then expr1 else expr2  ==postfix stack==>
        // false(idx_i-3), expr1(idx_i-2), expr2(idx_i-1), T_OP_ARG_CASE
        // params.operand_count_ = 3
        //
        // case 2:
        // case when false then expr1  ==postfix stack==>
        // false(idx_i-2), expr1(idx_i-1), T_OP_ARG_CASE
        // params.operand_count_ = 2
        //
        for (i = idx_i - params.operand_count_; i < idx_i; i += 2)
        {
          if (stack_i[i].is_true())
          {
            result = stack_i[i+1];
            match_when = true;
            break;
          }
        }
        if (false == match_when)
        {
          if (params.operand_count_ % 2 != 0)
          {
            result = stack_i[idx_i - 1]; // match else
          }
          else
          {
            result.set_null(); // no else in expression, then match nothing
          }
        }
        if (OB_SUCCESS == ret)
        {
          idx_i -= params.operand_count_;
        }
      }
      return ret;
    }



    int64_t ObPostfixExpression::to_string(char* buf, const int64_t buf_len) const
    {
      int64_t pos = 0;
      int err = OB_SUCCESS;
      int idx = 0;
      int64_t type = 0;
      int64_t value = 0;
      int64_t value2 = 0;
      int64_t sys_func = 0;
      //add fanqiushi [semi_join] [0.1] 20150910:b
      /*for(int i=0;i<expr_.count();i++)
      {
        TBSYS_LOG(ERROR,"expr_[idx]=%s,idx=%d", to_cstring(expr_[i]),i);
      }*/
      //add:e
      while(idx < expr_.count() && OB_SUCCESS == err)
      {
        expr_[idx++].get_int(type);
        if (END == type)
        {
          break;
        }
        else if(type <= BEGIN_TYPE || type >= END_TYPE)
        {
          break;
        }
        switch(type)
        {
          case COLUMN_IDX:
            if (OB_SUCCESS  != (err = expr_[idx++].get_int(value)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else if (OB_SUCCESS  != (err = expr_[idx++].get_int(value2)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else
            {
              uint64_t tid = static_cast<uint64_t>(value);
              if (OB_INVALID_ID == tid)
              {
                databuff_printf(buf, buf_len, pos, "COL<NULL,%lu>|", static_cast<uint64_t>(value2));
              }
              else
              {
                databuff_printf(buf, buf_len, pos, "COL<%lu,%lu>|", tid, static_cast<uint64_t>(value2));
              }
            }
            break;
          case CONST_OBJ:
            pos += expr_[idx].to_string(buf+pos, buf_len-pos);
            databuff_printf(buf, buf_len, pos, "|");
            idx++;
            break;
          case CUR_TIME_OP:
            databuff_printf(buf, buf_len, pos, "current_timestamp()");
            idx++;  // skip place holder
            break;
          case PARAM_IDX:
            if (OB_SUCCESS  != (err = expr_[idx++].get_int(value)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else
            {
              databuff_printf(buf, buf_len, pos, "PARAM<%ld>|", value);
            }
            break;
          case SYSTEM_VAR:
            databuff_printf(buf, buf_len, pos, "@@");
            pos += expr_[idx++].to_string(buf+pos, buf_len-pos);
            break;
          case TEMP_VAR:
            databuff_printf(buf, buf_len, pos, "@");
            pos += expr_[idx++].to_string(buf+pos, buf_len-pos);
            break;
            //add zt 20151208:b
          case ARRAY_VAR:
            databuff_printf(buf, buf_len, pos, "@");
            pos += expr_[idx++].to_string(buf+pos, buf_len-pos);
            databuff_printf(buf, buf_len, pos, "[]");
            //add zt 20151208:e
          case OP:
            // 鏍规嵁OP鐨勭被鍨嬶紝浠庡爢鏍堜腑寮瑰嚭1涓垨澶氫釜鎿嶄綔鏁帮紝杩涜璁＄畻
            if (OB_SUCCESS != (err = expr_[idx++].get_int(value)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else if (value <= T_MIN_OP || value >= T_MAX_OP)
            {
              TBSYS_LOG(WARN,"unsupported operator type [type:%ld]", value);
              err = OB_INVALID_ARGUMENT;
            }
            else if(OB_SUCCESS != (err = expr_[idx++].get_int(value2 /*param_count*/)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else
            {
              if (OB_UNLIKELY(T_FUN_SYS == value))
              {
                if(OB_SUCCESS != (err = expr_[idx++].get_int(sys_func)))
                {
                  TBSYS_LOG(WARN, "failed to get sys func, err=%d", err);
                }
                else if (0 > sys_func || sys_func >= SYS_FUNC_NUM)
                {
                  TBSYS_LOG(WARN, "invalid sys function type=%ld", sys_func);
                  err = OB_ERR_UNEXPECTED;
                }
                else
                {
                  databuff_printf(buf, buf_len, pos, "%s<%ld>|", SYS_FUNCS_NAME[sys_func], value2);
                }
              }
              else
              {
                databuff_printf(buf, buf_len, pos, "%s<%ld>|", ob_op_func_str(static_cast<ObItemType>(value)), value2);
              }
            }
            break;
          //weixing [implementation of sub_query]20160119:b
          case QUERY_ID:
            if (OB_SUCCESS  != (err = expr_[idx++].get_int(value)))
            {
              TBSYS_LOG(WARN,"get_int error [err:%d]", err);
            }
            else
            {
              databuff_printf(buf, buf_len, pos, "sub_query_select_num<%ld>|", value);
            }
            break;
          //add e
          default:
            databuff_printf(buf, buf_len, pos, "unexpected [type:%ld]", type);
            err = OB_ERR_UNEXPECTED;
            TBSYS_LOG(WARN,"unexpected [type:%ld]", type);
            break;
        }
      } // end while
      return pos;
    }

    int64_t ObPostfixExpression::get_type_num(int64_t idx,int64_t type) const
    {
      int64_t num = 0;
      int ret = OB_SUCCESS;
      if(type == BEGIN_TYPE)
      {
        num = 0;
      }
      else if (type == OP)
      {
        num = 3;
        int64_t op_type = 0;
        if (OB_SUCCESS != (ret = expr_[idx+1].get_int(op_type)))
        {
          TBSYS_LOG(WARN, "Fail to get op type. unexpected! ret=%d", ret);
        }
        else if (T_FUN_SYS == op_type)
        {
          ++num;
        }
      }
      else if (type == COLUMN_IDX || type == T_OP_ROW)
      {
        num = 3;
      }
      //mod longfei [bugfix:prepare] 2016-04-16 13:42:51
      else if (type == CONST_OBJ || PARAM_IDX == type)
//      else if (type == CONST_OBJ )
      //mod e
      {
        num = 2;
      }
      else if (type == END || type == UPS_TIME_OP)
      {
        num = 1;
      }
      else
      {
        TBSYS_LOG(WARN, "Unkown type %ld", type);
      }
      return num;
    }

  } /* sql */
} /* namespace */
