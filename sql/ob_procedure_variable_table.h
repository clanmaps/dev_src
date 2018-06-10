/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_procedure_variable_table.h
 * @brief procedure varoable table definition
 *
 * Created by zhutao
 *
 * @version __DaSE_VERSION
 * @author zhutao <zhutao@stu.ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_28
 */

#ifndef OB_PROCEDURE_VARIABLE_TABLE_H
#define OB_PROCEDURE_VARIABLE_TABLE_H

#include "common/ob_malloc.h"
#include "common/ob_pooled_allocator.h"
#include "common/hash/ob_hashmap.h"
#include "common/ob_string_buf.h"
#include "common/ob_se_array.h"
using namespace oceanbase::common;

namespace oceanbase{

  namespace sql
  {
    /**
     * @brief The ObProcedureVariableTable class
     * this class store procedure varible
     */
    class ObProcedureVariableTable
    {
      public:
        /**
         * @brief constructor
         */
        ObProcedureVariableTable();
        /**
         * @brief create
         * create a variable table
         * @return error code
         */
        inline int create();
        /**
         * @brief clear
         * clear this boject
         * @return error code
         */
        inline int clear();
        /**
         * @brief create_variable
         * create a variable
         * @param var_name variable name
         * @param type variable
         * @return error code
         */
        int create_variable(const ObString &var_name, const ObObjType type);
        /**
         * @brief create_array
         * create a array
         * @param array array name
         * @param type array type
         * @return error code
         */
        int create_array(const ObString &array, const ObObjType type);
        /**
         * @brief write
         * assgin a variable value
         * @param var_name viariable name
         * @param val variable value
         * @return errror code
         */
        int write(const ObString &var_name, const ObObj &val);
        /**
         * @brief write
         * assgin an array value
         * @param array_name array name
         * @param idx array index
         * @param val  array value
         * @return error code
         */
        int write(const ObString &array_name, int64_t idx, const ObObj &val);
        /**
         * @brief write
         * assgin array value
         * @param array_name
         * @param other array
         * @return error code
         */
        int write(const ObString &array_name, const ObIArray<ObObj> &other);
        /**
         * @brief read
         * read variable value
         * @param var_name variable name
         * @param val variable value
         * @return error code
         */
        int read(const ObString &var_name, const ObObj *&val) const;
        /**
         * @brief read
         * read array element value
         * @param array_name array name
         * @param idx array index
         * @param val array element value
         * @return error code
         */
        int read(const ObString &array_name, int64_t idx, const ObObj *&val) const;
        /**
         * @brief read
         * read array
         * @param array_name array name
         * @param array procedure variable array
         * @return error code
         */
        int read(const ObString &array_name, const ObIArray<ObObj> *&array) const;

      private:

        static const int64_t SMALL_BLOCK_SIZE = 4 * 1024LL;  ///<  a small block size
        /// typedef  VarNameValMapAllocer  type
        typedef common::ObPooledAllocator<common::hash::HashMapTypes<common::ObString, common::ObObj>::AllocType, common::ObWrapperAllocator> VarNameValMapAllocer;
        typedef common::hash::ObHashMap<common::ObString,
                                        common::ObObj,
                                        common::hash::NoPthreadDefendMode,
                                        common::hash::hash_func<common::ObString>,
                                        common::hash::equal_to<common::ObString>,
                                        VarNameValMapAllocer,
                                        common::hash::NormalPointer,
                                        common::ObSmallBlockAllocator<>
                                        > VarNameValMap;  ///<  typedef  VarNameValMap  type
        typedef ObSEArray<ObObj, 8> ObProcedureArray;  ///<  typedef  ObProcedureArray  type

        common::ObSmallBlockAllocator<> block_allocator_;  ///<  block allocator
        VarNameValMapAllocer var_name_val_map_allocer_;  ///<  variable name map allocator
        common::ObStringBuf name_pool_;  ///<  name manager pool

        VarNameValMap var_name_val_map_;  ///<  variable name value map
        ObSEArray<ObProcedureArray, 4> array_table_;  ///<  procedure array
    };

    int ObProcedureVariableTable::create()
    {
      return var_name_val_map_.create(hash::cal_next_prime(16), &var_name_val_map_allocer_, &block_allocator_);
    }

    int ObProcedureVariableTable::clear()
    {
      for(int64_t i = 0; i < array_table_.count(); ++i)
      {
        array_table_.at(i).clear();
      }
      array_table_.clear();
      var_name_val_map_.clear();
      name_pool_.clear();

      return OB_SUCCESS;
    }
  }
}

#endif // OB_PROCEDURE_VARIABLE_TABLE_H
