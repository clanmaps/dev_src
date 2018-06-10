/**
* Copyright (C) 2013-2016 DaSE .
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* @file ob_column_checksum.cpp
* @brief for column checksum of table
*
* Created by maoxiaoxiao:operations to column checksum, such as add, compare and so on
*
* @version CEDAR 0.2 
* @author maoxiaoxiao <51151500034@ecnu.edu.cn>
* @date 2016_01_21
*/

#ifndef OB_COLUMN_CHECKSUM_H
#define OB_COLUMN_CHECKSUM_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <map>
#include <vector>
#include <string.h>

namespace oceanbase
{
  namespace common
  {
    static const int64_t OB_MAX_COL_CHECKSUM_COLUMN_COUNT = 100;
    static const int64_t OB_MAX_COL_CHECKSUM_STR_LEN = 2560;
    struct Token
    {
      Token() : token(NULL), len(0) {}
      const char *token;
      int64_t len;
    };

    /**
     * @brief The ObColumnChecksum class
     * ObColumnChecksum is designed for
     * column checksum
     */
    class ObColumnChecksum
    {
    public:

      /**
       * @brief constructor
       */
      ObColumnChecksum()
      {
        memset(checksum_str, 0, OB_MAX_COL_CHECKSUM_STR_LEN);
      }

      /**
       * @brief reset
       */
      inline void reset()
      {
        memset(checksum_str, 0, OB_MAX_COL_CHECKSUM_STR_LEN);
      }

      /**
       * @brief get_str
       * @return checksum_str
       */
      inline char* get_str()
      {
        return checksum_str;
      }

      /**
       * @brief get_str_const
       * @return checksum_str
       */
      inline const char* get_str_const() const
      {
        return checksum_str;
      }

      /**
       * @brief deepcopy
       * @param col column checksum string
       * @param len length
       */
      void deepcopy(const char* col, const int32_t len);

      /**
       * @brief deepcopy
       * @param col column checksum string
       */
      void deepcopy(const char* col);

      /**
       * @brief add
       * add two column checksum
       * @param col column checksum
       * @return OB_SUCCESS or other ERROR
       */
      int add(const ObColumnChecksum &col);

      /**
       * @brief equal
       * compare two column checksum
       * @param col col column checksum
       * @param is_equal flag of whether two column checksums are equal
       * @return OB_SUCCESS or other ERROR
       */
      int equal(const ObColumnChecksum &col, bool &is_equal);

    private:
      /**
       * @brief transform_str_to_int
       * transform from string type to int type
       * @param data
       * @param dlen
       * @param value
       * @return 0 if success or -1 if failed
       */
      int transform_str_to_int(const char* data, const int64_t &dlen, uint64_t &value);

      /**
       * @brief tokenize
       * parse character array by a character
       * @param data
       * @param dlen
       * @param delima
       * @param token_nr
       * @param tokens
       * @return 0 if success or -1 if failed
       */
      int tokenize(const char *data, int64_t dlen, char delima, int &token_nr, Token *tokens);

    private:
      char checksum_str[OB_MAX_COL_CHECKSUM_STR_LEN]; ///<string of column checksum
    };
  }
}

#endif // OB_COLUMN_CHECKSUM_H
