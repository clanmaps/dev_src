/**
 * Copyright (C) 2013-2016 ECNU_DaSE.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_schema_service_impl.cpp
 * @brief implementation of schema service
 *
 * modified by longfei：add some function for new a core table
 * modified by WengHaixing: add some funcfion for secondary index status/columnchecksum
 * modified by maoxiaoxiao:add implementations of functions to check column checksum, clean column checksum and get column checksum
 *
 * @version CEDAR 0.2 
 * @author longfei <longfei@stu.ecnu.edu.cn>
 * @author maoxiaoxiao <51151500034@ecnu.edu.cn>
 * @author wangdonghui <zjnuwangdonghui@163.com>
 * @date 2016_07_26
 */
 
#include "ob_schema_service_impl.h"
#include "ob_extra_tables_schema.h"
#include "ob_schema_service.h"
#include "utility.h"

using namespace oceanbase;
using namespace common;
using namespace nb_accessor;

#define DEL_ROW(table_name, rowkey) \
if (OB_SUCCESS == ret) \
{ \
  ret = mutator->del_row(table_name, rowkey); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "insert del to mutator fail:ret[%d]", ret); \
  } \
}

#define ADD_VARCHAR(table_name, rowkey, column_name, value) \
if (OB_SUCCESS == ret) \
{ \
  ObObj vchar_value; \
  vchar_value.set_varchar(OB_STR(value)); \
  ret = mutator->insert(table_name, rowkey, OB_STR(column_name), vchar_value); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "insert value to mutator fail:column_name[%s], ret[%d]", column_name, ret); \
  } \
}

#define ADD_INT(table_name, rowkey, column_name, value) \
if(OB_SUCCESS == ret) \
{ \
  ObObj int_value; \
  int_value.set_int(value); \
  ret = mutator->insert(table_name, rowkey, OB_STR(column_name), int_value); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "insert value to mutator fail:column_name[%s], ret[%d]", column_name, ret); \
  } \
}
#define ADD_CREATE_TIME(table_name, rowkey, column_name, value) \
if(OB_SUCCESS == ret) \
{ \
  ObObj time_value; \
  time_value.set_createtime(value); \
  ret = mutator->insert(table_name, rowkey, OB_STR(column_name), time_value); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "insert value to mutator fail:column_name[%s], ret[%d]", column_name, ret); \
  } \
}
#define ADD_MODIFY_TIME(table_name, rowkey, column_name, value) \
if(OB_SUCCESS == ret) \
{ \
  ObObj time_value; \
  time_value.set_modifytime(value); \
  ret = mutator->insert(table_name, rowkey, OB_STR(column_name), time_value); \
  if(OB_SUCCESS != ret) \
  { \
    TBSYS_LOG(WARN, "insert value to mutator fail:column_name[%s], ret[%d]", column_name, ret); \
  } \
}

int ObSchemaServiceImpl::add_join_info(ObMutator* mutator, const TableSchema& table_schema)
{
  int ret = OB_SUCCESS;

  if(NULL == mutator)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "mutator is null");
  }

  JoinInfo join_info;
  ObRowkey rowkey;

  ObObj value[4];


  if(OB_SUCCESS == ret)
  {
    for(int32_t i=0;i<table_schema.join_info_.count();i++)
    {
      ret = table_schema.join_info_.at(i, join_info);
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "get joininfo from table_schema fail:ret[%d], i[%d]", ret, i);
        break;
      }

      value[0].set_int(join_info.left_table_id_);
      value[1].set_int(join_info.left_column_id_);
      value[2].set_int(join_info.right_table_id_);
      value[3].set_int(join_info.right_column_id_);
      rowkey.assign(value, 4);

      //to be delete start
     // ADD_INT(joininfo_table_name, rowkey, "left_table_id", join_info.left_table_id_);
     // ADD_INT(joininfo_table_name, rowkey, "left_column_id", join_info.left_column_id_);
     // ADD_INT(joininfo_table_name, rowkey, "right_table_id", join_info.right_table_id_);
     // ADD_INT(joininfo_table_name, rowkey, "right_column_id", join_info.right_column_id_);
      // to be delete end
      ADD_VARCHAR(joininfo_table_name, rowkey, "left_table_name", join_info.left_table_name_);
      ADD_VARCHAR(joininfo_table_name, rowkey, "left_column_name", join_info.left_column_name_);
      ADD_VARCHAR(joininfo_table_name, rowkey, "right_table_name", join_info.right_table_name_);
      ADD_VARCHAR(joininfo_table_name, rowkey, "right_column_name", join_info.right_column_name_);

      TBSYS_LOG(DEBUG, "insert mutate join info[%s]", to_cstring(join_info));
    }
  }

  return ret;
}


int ObSchemaServiceImpl::add_column(ObMutator* mutator, const TableSchema& table_schema)
{
  int ret = OB_SUCCESS;

  if(NULL == mutator)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "mutator is null");
  }

  ColumnSchema column;
  ObRowkey rowkey;
  ObString column_name;

  ObObj value[2];
  value[0].set_int(table_schema.table_id_);


  if (OB_SUCCESS == ret)
  {
    for(int32_t i=0;i<table_schema.columns_.count() && OB_SUCCESS == ret;i++)
    {
      ret = table_schema.columns_.at(i, column);
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "get column from table_schema fail:ret[%d], i[%d]", ret, i);
      }

      if(OB_SUCCESS == ret)
      {
        column_name.assign_ptr(column.column_name_, static_cast<int32_t>(strlen(column.column_name_)));
        value[1].set_varchar(column_name);
        rowkey.assign(value, 2);
        ADD_INT(column_table_name, rowkey, "column_id", column.column_id_);
        ADD_INT(column_table_name, rowkey, "column_group_id", column.column_group_id_);
        ADD_INT(column_table_name, rowkey, "rowkey_id", column.rowkey_id_);
        ADD_INT(column_table_name, rowkey, "join_table_id", column.join_table_id_);
        ADD_INT(column_table_name, rowkey, "join_column_id", column.join_column_id_);
        ADD_INT(column_table_name, rowkey, "data_type", column.data_type_);
        ADD_INT(column_table_name, rowkey, "data_length", column.data_length_);
        ADD_INT(column_table_name, rowkey, "data_precision", column.data_precision_);
        ADD_INT(column_table_name, rowkey, "data_scale", column.data_scale_);
        ADD_INT(column_table_name, rowkey, "nullable", column.nullable_);
        //add lbzhong [auto_increment] 20161123:b
        ADD_INT(column_table_name, rowkey, "auto_increment", column.auto_increment_);
        //add:e
        ADD_INT(column_table_name, rowkey, "length_in_rowkey", column.length_in_rowkey_);
        ADD_INT(column_table_name, rowkey, "order_in_rowkey", column.order_in_rowkey_);
      }
    }
  }

  return ret;
}

ObSchemaServiceImpl::ObSchemaServiceImpl()
  :client_proxy_(NULL), is_id_name_map_inited_(false), only_core_tables_(true)
{
}

ObSchemaServiceImpl::~ObSchemaServiceImpl()
{
  client_proxy_ = NULL;
  is_id_name_map_inited_ = false;
}

bool ObSchemaServiceImpl::check_inner_stat()
{
  bool ret = true;
  tbsys::CThreadGuard guard(&mutex_);
  if(!is_id_name_map_inited_)
  {
    int err = init_id_name_map();
    if(OB_SUCCESS != err)
    {
      ret = false;
      TBSYS_LOG(WARN, "init id name map fail:ret[%d]", err);
    }
    else
    {
      is_id_name_map_inited_ = true;
    }
  }

  if(ret && NULL == client_proxy_)
  {
    TBSYS_LOG(ERROR, "client proxy is NULL");
    ret = false;
  }
  return ret;
}

int ObSchemaServiceImpl::init(ObScanHelper* client_proxy, bool only_core_tables)
{
  int ret = OB_SUCCESS;
  tbsys::CThreadGuard guard(&mutex_);
  if (NULL == client_proxy)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "client proxy is null");
  }
  else if (true == only_core_tables)
  {
    // do not clear id_name_map
    if (!id_name_map_.created())
    {
      if (OB_SUCCESS != (ret = id_name_map_.create(1000)))
      {
        TBSYS_LOG(WARN, "create id_name_map_ fail:ret[%d]", ret);
      }
    }
  }
  else if (id_name_map_.created())
  {
    if (OB_SUCCESS != (ret = id_name_map_.clear()))
    {
      TBSYS_LOG(WARN, "fail to clear id name hash map. ret=%d", ret);
    }
    else
    {
      is_id_name_map_inited_ = false;
      string_buf_.reuse();
    }
  }
  else if (OB_SUCCESS != (ret = id_name_map_.create(1000)))
  {
    TBSYS_LOG(WARN, "create id_name_map_ fail:ret[%d]", ret);
  }

  if (OB_SUCCESS == ret)
  {
    this->client_proxy_ = client_proxy;
    this->only_core_tables_ = only_core_tables;
    ret = nb_accessor_.init(client_proxy_);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "init nb accessor fail:ret[%d]", ret);
    }
    else
    {
      nb_accessor_.set_is_read_consistency(true);
    }
  }
  return ret;
}

int ObSchemaServiceImpl::create_table_mutator(const TableSchema& table_schema, ObMutator* mutator)
{
  int ret = OB_SUCCESS;

  ObString table_name;
  table_name.assign_ptr(const_cast<char*>(table_schema.table_name_), static_cast<int32_t>(strlen(table_schema.table_name_)));

  ObObj table_name_value;
  table_name_value.set_varchar(table_name);

  ObRowkey rowkey;
  rowkey.assign(&table_name_value, 1);

  //ADD_VARCHAR(first_tablet_entry_name, rowkey, "table_name", table_schema.table_name_);
  ADD_INT(first_tablet_entry_name, rowkey, "table_id", table_schema.table_id_);
  ADD_INT(first_tablet_entry_name, rowkey, "table_type", table_schema.table_type_);
  ADD_INT(first_tablet_entry_name, rowkey, "load_type", table_schema.load_type_);
  ADD_INT(first_tablet_entry_name, rowkey, "table_def_type", table_schema.table_def_type_);
  ADD_INT(first_tablet_entry_name, rowkey, "rowkey_column_num", table_schema.rowkey_column_num_);
  ADD_INT(first_tablet_entry_name, rowkey, "replica_num", table_schema.replica_num_);
  ADD_INT(first_tablet_entry_name, rowkey, "max_used_column_id", table_schema.max_used_column_id_);
  ADD_INT(first_tablet_entry_name, rowkey, "create_mem_version", table_schema.create_mem_version_);
  ADD_INT(first_tablet_entry_name, rowkey, "tablet_max_size", table_schema.tablet_max_size_);
  ADD_INT(first_tablet_entry_name, rowkey, "tablet_block_size", table_schema.tablet_block_size_);
  ADD_VARCHAR(first_tablet_entry_name, rowkey, "compress_func_name", table_schema.compress_func_name_);

  ADD_INT(first_tablet_entry_name, rowkey, "is_use_bloomfilter", table_schema.is_use_bloomfilter_);
  ADD_INT(first_tablet_entry_name, rowkey, "is_pure_update_table", table_schema.is_pure_update_table_);
  //ADD_INT(first_tablet_entry_name, rowkey, "consistency_level", table_schema.consistency_level_);
  ADD_INT(first_tablet_entry_name, rowkey, "is_read_static", table_schema.consistency_level_);
  ADD_INT(first_tablet_entry_name, rowkey, "rowkey_split", table_schema.rowkey_split_);
  ADD_INT(first_tablet_entry_name, rowkey, "max_rowkey_length", table_schema.max_rowkey_length_);
  ADD_INT(first_tablet_entry_name, rowkey, "merge_write_sstable_version", table_schema.merge_write_sstable_version_);
  ADD_INT(first_tablet_entry_name, rowkey, "schema_version", table_schema.schema_version_);
  ADD_VARCHAR(first_tablet_entry_name, rowkey, "expire_condition", table_schema.expire_condition_);
  //ADD_VARCHAR(first_tablet_entry_name, rowkey, "comment_str", table_schema.comment_str_);
  ADD_INT(first_tablet_entry_name, rowkey, "create_time_column_id", table_schema.create_time_column_id_);
  ADD_INT(first_tablet_entry_name, rowkey, "modify_time_column_id", table_schema.modify_time_column_id_);
  if(OB_SUCCESS == ret)
  {
    ret = add_column(mutator, table_schema);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "add column to mutator fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    ret = add_join_info(mutator, table_schema);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "add join info to mutator fail:ret[%d]", ret);
    }
  }
  //add lbzhong [auto_increment] 20161201:b
  if (OB_SUCCESS == ret)
  {
    uint64_t column_id = OB_INVALID_ID;
    if (OB_SUCCESS != (ret = check_auto_increment(table_schema, column_id)))
    {
      TBSYS_LOG(WARN, "check auto_increment fail:ret[%d]", ret);
    }
    else if (OB_INVALID_ID != column_id && OB_SUCCESS != (ret = auto_increment_mutator(table_schema.table_id_, column_id, mutator)))
    {
      TBSYS_LOG(WARN, "add auto_increment mutator fail:ret[%d]", ret);
    }
  }
  //add:e

  return ret;
}

//add by wangdonghui 20160125 :b
int ObSchemaServiceImpl::create_procedure_mutator(const common::ObString& proc_name, const common::ObString& proc_source_code, ObMutator* mutator)
{
  int ret = OB_SUCCESS;
  ObObj procedure_name_value;
  procedure_name_value.set_varchar(proc_name);

  ObRowkey rowkey;
  rowkey.assign(&procedure_name_value, 1);

  //ADD_VARCHAR(procedure_table_name, rowkey, "proc_name", proc_name.ptr());
  ADD_VARCHAR(procedure_table_name, rowkey, "source", proc_source_code.ptr());
  ADD_VARCHAR(procedure_table_name, rowkey, "type","procedure");
  ADD_VARCHAR(procedure_table_name, rowkey, "note", "");

  return ret;
}
//add :e

int ObSchemaServiceImpl::alter_table_mutator(const AlterTableSchema& table_schema, ObMutator* mutator, const int64_t old_schema_version)
{
  int ret = OB_SUCCESS;
  ObObj value[2];
  value[0].set_int(table_schema.table_id_);
  ObRowkey rowkey;
  ObString column_name;
  uint64_t max_column_id = 0;
  AlterTableSchema::AlterColumnSchema alter_column;
  for (int32_t i = 0; (OB_SUCCESS == ret) && (i < table_schema.get_column_count()); ++i)
  {
    ret = table_schema.columns_.at(i, alter_column);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "get column from table_schema fail:ret[%d], i[%d]", ret, i);
    }
    else
    {
      column_name.assign_ptr(alter_column.column_.column_name_,
          static_cast<int32_t>(strlen(alter_column.column_.column_name_)));
      value[1].set_varchar(column_name);
      rowkey.assign(value, 2);
      switch (alter_column.type_)
      {
        case AlterTableSchema::ADD_COLUMN:
          {
            if (alter_column.column_.column_id_ <= max_column_id)
            {
              TBSYS_LOG(WARN, "check column id failed:column_id[%lu], max[%ld]",
                  alter_column.column_.column_id_, max_column_id);
              ret = OB_INVALID_ARGUMENT;
            }
            else
            {
              max_column_id = alter_column.column_.column_id_;
            }
          }
        case AlterTableSchema::MOD_COLUMN:
          {
            // add column succ
            if (OB_SUCCESS == ret)
            {
              ret = update_column_mutator(mutator, rowkey, alter_column.column_);
            }
            break;
          }
        case AlterTableSchema::DEL_COLUMN:
          {
            DEL_ROW(column_table_name, rowkey);
            break;
          }
        default :
          {
            ret = OB_INVALID_ARGUMENT;
            break;
          }
      }
    }
  }
  // reset table max used column id
  if ((OB_SUCCESS == ret) && (max_column_id != 0))
  {
    ret = reset_column_id_mutator(mutator, table_schema, max_column_id);
  }
  if ((OB_SUCCESS == ret) && (old_schema_version >= 0))
  {
    ret = reset_schema_version_mutator(mutator, table_schema, old_schema_version);
  }
  return ret;
}

int ObSchemaServiceImpl::update_column_mutator(ObMutator* mutator, ObRowkey & rowkey, const ColumnSchema & column)
{
  int ret = OB_SUCCESS;
  ADD_INT(column_table_name, rowkey, "column_id", column.column_id_);
  ADD_INT(column_table_name, rowkey, "column_group_id", column.column_group_id_);
  ADD_INT(column_table_name, rowkey, "rowkey_id", column.rowkey_id_);
  ADD_INT(column_table_name, rowkey, "join_table_id", column.join_table_id_);
  ADD_INT(column_table_name, rowkey, "join_column_id", column.join_column_id_);
  ADD_INT(column_table_name, rowkey, "data_type", column.data_type_);
  ADD_INT(column_table_name, rowkey, "data_length", column.data_length_);
  ADD_INT(column_table_name, rowkey, "data_precision", column.data_precision_);
  ADD_INT(column_table_name, rowkey, "data_scale", column.data_scale_);
  ADD_INT(column_table_name, rowkey, "nullable", column.nullable_);
  //add lbzhong [auto_increment] 20161123:b
  ADD_INT(column_table_name, rowkey, "auto_increment", column.auto_increment_);
  //add:e
  ADD_INT(column_table_name, rowkey, "length_in_rowkey", column.length_in_rowkey_);
  ADD_INT(column_table_name, rowkey, "order_in_rowkey", column.order_in_rowkey_);
  return ret;
}

int ObSchemaServiceImpl::reset_column_id_mutator(ObMutator* mutator, const AlterTableSchema & schema, const uint64_t max_column_id)
{
  int ret = OB_SUCCESS;
  if ((mutator != NULL) && (max_column_id > OB_APP_MIN_COLUMN_ID))
  {
    ObString table_name;
    table_name.assign_ptr(const_cast<char*>(schema.table_name_), static_cast<int32_t>(strlen(schema.table_name_)));
    ObObj table_name_value;
    table_name_value.set_varchar(table_name);
    ObRowkey rowkey;
    rowkey.assign(&table_name_value, 1);
    ADD_INT(first_tablet_entry_name, rowkey, "max_used_column_id", max_column_id);
  }
  else
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "check input param failed:table_name[%s], max_column_id[%lu]", schema.table_name_, max_column_id);
  }
  return ret;
}

int ObSchemaServiceImpl::reset_schema_version_mutator(ObMutator* mutator, const AlterTableSchema & schema, const int64_t old_schema_version)
{
  int ret = OB_SUCCESS;
  if ((mutator != NULL) && (old_schema_version >= 0))
  {
    ObString table_name;
    table_name.assign_ptr(const_cast<char*>(schema.table_name_), static_cast<int32_t>(strlen(schema.table_name_)));
    ObObj table_name_value;
    table_name_value.set_varchar(table_name);
    ObRowkey rowkey;
    rowkey.assign(&table_name_value, 1);
    int64_t new_schema_version = old_schema_version+1;
    ADD_INT(first_tablet_entry_name, rowkey, "schema_version", new_schema_version);
  }
  else
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "check input param failed:table_name[%s], old_schema_version[%ld]", schema.table_name_, old_schema_version);
  }
  return ret;
}

int ObSchemaServiceImpl::create_table(const TableSchema& table_schema)
{
  int ret = OB_SUCCESS;

  if (!table_schema.is_valid())
  {
    TBSYS_LOG(WARN, "invalid table schema, tid=%lu", table_schema.table_id_);
    ret = OB_ERR_INVALID_SCHEMA;
  }
  else if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }

  tbsys::CThreadGuard guard(&mutex_);

  ObMutator* mutator = NULL;

  if(OB_SUCCESS == ret)
  {
    mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
    if(NULL == mutator)
    {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      TBSYS_LOG(WARN, "get thread specific Mutator fail");
    }
  }

  if(OB_SUCCESS == ret)
  {
    ret = mutator->reset();
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "reset ob mutator fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
	//mod longfei [create index]
	if(OB_INVALID_ID == table_schema.original_table_id_)
	{
	    ret = create_table_mutator(table_schema, mutator);
	    if(OB_SUCCESS != ret)
	    {
	      TBSYS_LOG(WARN, "create table mutator fail:ret[%d]", ret);
	    }
	}
	else
	{
		ret = create_index_mutator(table_schema, mutator);
	    if(OB_SUCCESS != ret)
	    {
	      TBSYS_LOG(WARN, "create index mutator fail:ret[%d]", ret);
	    }
	}
  }

  if(OB_SUCCESS == ret)
  {
    ret = client_proxy_->mutate(*mutator);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "apply mutator fail:ret[%d]", ret);
    }
  }

  ObString table_name;
  table_name.assign_ptr(const_cast<char*>(table_schema.table_name_), static_cast<int32_t>(strlen(table_schema.table_name_)));

  ObString table_name_store;
  if(OB_SUCCESS == ret)
  {
    tbsys::CThreadGuard buf_guard(&string_buf_write_mutex_);
    ret = string_buf_.write_string(table_name, &table_name_store);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "write string fail:ret[%d]", ret);
    }
  }

  int err = 0;
  if(OB_SUCCESS == ret)
  {
    err = id_name_map_.set(table_schema.table_id_, table_name_store);
    if(hash::HASH_INSERT_SUCC != err)
    {
      if(hash::HASH_EXIST == err)
      {
        TBSYS_LOG(ERROR, "bug table exist:table_id[%lu], table_name_store[%.*s]",
          table_schema.table_id_, table_name_store.length(), table_name_store.ptr());
      }
      else
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "id name map set fail:err[%d], table_id[%lu], table_name_store[%.*s]", err,
          table_schema.table_id_, table_name_store.length(), table_name_store.ptr());
      }
    }
  }
  return ret;
}


//add by wangdonghui 20160125 this is not a wise choice,
//maybe we can build a new file which names ob_procedure_service :b
int ObSchemaServiceImpl::create_procedure(const common::ObString& proc_name, const common::ObString& proc_source_code)
{
  int ret = OB_SUCCESS;

  tbsys::CThreadGuard guard(&mutex_);

  ObMutator* mutator = NULL;

  if ( OB_SUCCESS == ret )
  {
    mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
    if ( NULL == mutator )
    {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      TBSYS_LOG(WARN, "get thread specific Mutator fail");
    }
  }

  if ( OB_SUCCESS == ret )
  {
    ret = mutator->reset();
    if ( OB_SUCCESS != ret )
    {
      TBSYS_LOG(WARN, "reset ob mutator fail:ret[%d]", ret);
    }
  }

  if ( OB_SUCCESS == ret )
  {
    ret = create_procedure_mutator(proc_name, proc_source_code, mutator);
    if( OB_SUCCESS != ret )
    {
      TBSYS_LOG(WARN, "create procedures mutator fail:ret[%d]", ret);
    }
  }

  if ( OB_SUCCESS == ret )
  {
    ret = client_proxy_->mutate(*mutator);
    if ( OB_SUCCESS != ret )
    {
      TBSYS_LOG(WARN, "apply mutator fail:ret[%d]", ret);
    }
  }
  return ret;
}
//add :e

#define ASSIGN_INT_FROM_ROWKEY(column, rowkey_index, field, type) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * ci = NULL; \
  int64_t int_value = 0; \
  ci = table_row->get_cell_info(column); \
  if (NULL != ci && NULL != ci->row_key_.ptr() \
      && rowkey_index < ci->row_key_.length() \
      && ci->row_key_.ptr()[rowkey_index].get_type() == ObIntType) \
  { \
    ci->row_key_.ptr()[rowkey_index].get_int(int_value); \
    field = static_cast<type>(int_value); \
    TBSYS_LOG(DEBUG, "get cell info:column[%s], value[%ld]", column, int_value); \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get column[%s] with error cell info %s ", \
        column, NULL == ci ? "nil": print_cellinfo(ci)); \
  } \
}

#define ASSIGN_VARCHAR_FROM_ROWKEY(column, rowkey_index, field, max_length) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * ci = NULL; \
  ObString str_value; \
  ci = table_row->get_cell_info(column); \
  if (NULL != ci && NULL != ci->row_key_.ptr() \
      && rowkey_index < ci->row_key_.length() \
      && ci->row_key_.ptr()[rowkey_index].get_type() == ObVarcharType) \
  { \
    ci->row_key_.ptr()[rowkey_index].get_varchar(str_value);  \
    if(str_value.length() >= max_length) \
    { \
      ret = OB_SIZE_OVERFLOW; \
      TBSYS_LOG(WARN, "field max length is not enough:max_length[%ld], str length[%d]", max_length, str_value.length()); \
    } \
    else \
    { \
      memcpy(field, str_value.ptr(), str_value.length()); \
      field[str_value.length()] = '\0'; \
    } \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get column[%s] with error cell info %s ", \
        column, NULL == ci ? "nil": print_cellinfo(ci)); \
  } \
}

#define ASSIGN_VARCHAR(column, field, max_length) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * cell_info = NULL; \
  ObString str_value; \
  cell_info = table_row->get_cell_info(column); \
  if(NULL != cell_info) \
  { \
    if (cell_info->value_.get_type() == ObNullType) \
    { \
      field[0] = '\0'; \
    } \
    else if (cell_info->value_.get_type() == ObVarcharType)\
    { \
      cell_info->value_.get_varchar(str_value); \
      if(str_value.length() >= max_length) \
      { \
        ret = OB_SIZE_OVERFLOW; \
        TBSYS_LOG(WARN, "field max length is not enough:max_length[%ld], str length[%d]", max_length, str_value.length()); \
      } \
      else \
      { \
        memcpy(field, str_value.ptr(), str_value.length()); \
        field[str_value.length()] = '\0'; \
      } \
    } \
    else \
    { \
      ret = OB_ERROR; \
      TBSYS_LOG(WARN, "get column[%s] with error type %s ", \
          column, print_cellinfo(cell_info)); \
    } \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get column[%s] with error null cell.", column); \
  } \
}

#define ASSIGN_INT(column, field, type) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * cell_info = NULL; \
  int64_t int_value = 0; \
  cell_info = table_row->get_cell_info(column); \
  if(NULL != cell_info && cell_info->value_.get_type() == ObIntType) \
  { \
    cell_info->value_.get_int(int_value); \
    field = static_cast<type>(int_value); \
    TBSYS_LOG(DEBUG, "get cell info:column[%s], value[%ld]", column, int_value); \
  } \
  else if (NULL != cell_info && cell_info->value_.get_type() == ObNullType) \
  { \
    field = static_cast<type>(0); \
    TBSYS_LOG(WARN, "get cell value null:column[%s]", column); \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get column[%s] with error cell info %s ", \
        column, NULL == cell_info ? "nil": print_cellinfo(cell_info)); \
  } \
}

#define ASSIGN_CREATE_TIME(column, field, type) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * cell_info = NULL; \
  ObCreateTime value = false; \
  cell_info = table_row->get_cell_info(column); \
  if(NULL != cell_info) \
  { \
    cell_info->value_.get_createtime(value); \
    field = static_cast<type>(value); \
    TBSYS_LOG(DEBUG, "get cell info:column[%s], value[%ld]", column, value); \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get cell info:column[%s]", column); \
  } \
}
#define ASSIGN_MODIFY_TIME(column, field, type) \
if(OB_SUCCESS == ret) \
{ \
  ObCellInfo * cell_info = NULL; \
  ObModifyTime value = false; \
  cell_info = table_row->get_cell_info(column); \
  if(NULL != cell_info) \
  { \
    cell_info->value_.get_modifytime(value); \
    field = static_cast<type>(value); \
    TBSYS_LOG(DEBUG, "get cell info:column[%s], value[%ld]", column, value); \
  } \
  else \
  { \
    ret = OB_ERROR; \
    TBSYS_LOG(WARN, "get cell info:column[%s]", column); \
  } \
}



int ObSchemaServiceImpl::drop_table(const ObString& table_name)
{
  int ret = OB_SUCCESS;

  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }


  uint64_t table_id = 0;

  ret = get_table_id(table_name, table_id);
  if(OB_SUCCESS != ret)
  {
    TBSYS_LOG(WARN, "get table id fail:table_name[%.*s]", table_name.length(), table_name.ptr());
  }

  // called after get_table_id() to prevent dead lock
  tbsys::CThreadGuard guard(&mutex_);

  ObRowkey rowkey;
  ObObj table_name_obj;
  table_name_obj.set_varchar(table_name);
  rowkey.assign(&table_name_obj, 1);

  if(OB_SUCCESS == ret)
  {
    // mod longfei [drop index] 20151026
    // delect index table's schema in __all_secondary_index

    //-------------------old-------------//
    //ret = nb_accessor_.delete_row(FIRST_TABLET_TABLE_NAME, rowkey);

    //-------------------new-------------//
    if(is_index_table_or_not(table_name))
    {
      ret = nb_accessor_.delete_row(OB_ALL_SECONDAYR_INDEX_TABLE_NAME, rowkey);
    }
    else
    {
      ret = nb_accessor_.delete_row(FIRST_TABLET_TABLE_NAME, rowkey);
    }
    //mod e
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "delete rwo from first tablet table fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    ret = nb_accessor_.delete_row(OB_ALL_COLUMN_TABLE_NAME, SC("table_name")("column_id"),
        ScanConds("table_id", EQ, table_id));
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "delete rwo from first tablet table fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    ret = nb_accessor_.delete_row(OB_ALL_JOININFO_TABLE_NAME, SC("left_table_id")("left_column_id")("right_table_id")("right_column_id"),
        ScanConds("left_table_id", EQ, table_id));
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "delete rwo from first tablet table fail:ret[%d]", ret);
    }
  }

  //add lbzhong [auto_increment] 20161201:b
  if (OB_SUCCESS == ret)
  {
    ret = delete_auto_increment(table_id);
  }
  //add:e

  int err = 0;
  if(OB_SUCCESS == ret)
  {
    err = id_name_map_.erase(table_id);
    if(hash::HASH_EXIST != err)
    {
      ret = hash::HASH_NOT_EXIST == err ? OB_ENTRY_NOT_EXIST : OB_SUCCESS;
      TBSYS_LOG(WARN, "id name map erase fail:err[%d], table_id[%lu]", err, table_id);
    }
  }

  return ret;
}

//add by wangdonghui 20160225 [drop procedure] :b
int ObSchemaServiceImpl::drop_procedure(const ObString& proc_name)
{
  int ret = OB_SUCCESS;

  if (!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }

  tbsys::CThreadGuard guard(&mutex_);

  ObRowkey rowkey;
  ObObj proc_name_obj;
  proc_name_obj.set_varchar(proc_name);
  rowkey.assign(&proc_name_obj, 1);

  if (OB_SUCCESS == ret)
  {
    ret = nb_accessor_.delete_row(OB_ALL_PROCEDURE_TABLE_NAME, rowkey);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "delete row from __all_procedure fail:ret[%d]", ret);
    }
  }

  return ret;
}
//add :e

int ObSchemaServiceImpl::init_id_name_map()
{
  int ret = OB_SUCCESS;
  ObTableIdNameIterator iterator;
  if(OB_SUCCESS == ret)
  {
    ret = iterator.init(client_proxy_, only_core_tables_);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "init iterator fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    if (OB_SUCCESS != (ret = init_id_name_map(iterator)))
    {
      TBSYS_LOG(WARN, "failed init id_name_map, err=%d", ret);
    }
  }

  iterator.destroy();
  return ret;
}

int ObSchemaServiceImpl::init_id_name_map(ObTableIdNameIterator& iterator)
{
  int ret = OB_SUCCESS;

  ObTableIdName * table_id_name = NULL;
  ObString tmp_str;

  while(OB_SUCCESS == ret && OB_SUCCESS == (ret = iterator.next()))
  {
    ret = iterator.get(&table_id_name);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "get table id name fail:ret[%d]", ret);
    }

    if(OB_SUCCESS == ret)
    {
      tbsys::CThreadGuard buf_guard(&string_buf_write_mutex_);
      ret = string_buf_.write_string(table_id_name->table_name_, &tmp_str);
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "write string to string buf fail:ret[%d]", ret);
      }
      else
      {
        //TBSYS_LOG(INFO, "get_table_id_name = %.*s", table_id_name->table_name_.length(),table_id_name->table_name_.ptr());
      }
    }

    int err = 0;
    if(OB_SUCCESS == ret)
    {
      err = id_name_map_.set(table_id_name->table_id_, tmp_str);
      if(hash::HASH_INSERT_SUCC != err)
      {
        ret = hash::HASH_EXIST == err ? OB_ENTRY_EXIST : OB_ERROR;
        TBSYS_LOG(WARN, "id name map set fail:err[%d], table_id[%lu]", err, table_id_name->table_id_);
      }
      else
      {
        TBSYS_LOG(DEBUG, "add id_name_map, tname=%.*s tid=%lu",
          tmp_str.length(), tmp_str.ptr(), table_id_name->table_id_);
      }
    }
  }

  if(OB_ITER_END == ret)
  {
    ret = OB_SUCCESS;
  }

  return ret;
}


int ObSchemaServiceImpl::get_table_name(uint64_t table_id, ObString& table_name)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(DEBUG, "table_id = %lu", table_id);
  if (OB_FIRST_TABLET_ENTRY_TID ==  table_id)
  {
    table_name = first_tablet_entry_name;
  }
  else if (OB_ALL_ALL_COLUMN_TID == table_id)
  {
    table_name = column_table_name;
  }
  else if (OB_ALL_JOIN_INFO_TID == table_id)
  {
    table_name = joininfo_table_name;
  }
  //longfei [create index]
  else if (OB_ALL_SECONDARY_INDEX_TID == table_id)
  {
    table_name = secondary_index_table_name;
  }
  else if (!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  else
  {
    int err = id_name_map_.get(table_id, table_name);
    if(hash::HASH_EXIST != err)
    {
      ret = hash::HASH_NOT_EXIST == err ? OB_ENTRY_NOT_EXIST : OB_ERROR;
      TBSYS_LOG(WARN, "id name map get fail:err[%d], table_id[%lu]", err, table_id);
      //add by qx 20170225 :b for test
//      hash::ObHashMap<uint64_t, ObString>::const_iterator iter1=id_name_map_.begin();
//      for (;iter1!=id_name_map_.end();iter1++)
//      {
//        int64_t id = (uint64_t)(iter1->first);
//        ObString name = iter1->second ;
//        TBSYS_LOG(WARN, "reseting name hash map iter->first=%ld  second=%s", id, name.ptr());
//      }
      //add :e
    }
  }
  TBSYS_LOG(DEBUG, "get table_name=%.*s", table_name.length(), table_name.ptr());
  return ret;
}

int ObSchemaServiceImpl::get_table_id(const ObString& table_name, uint64_t& table_id)
{
  int ret = OB_SUCCESS;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }

  QueryRes* res = NULL;
  TableRow* table_row = NULL;

  ObRowkey rowkey;
  ObObj table_name_obj;
  table_name_obj.set_varchar(table_name);
  rowkey.assign(&table_name_obj, 1);

  if(OB_SUCCESS == ret)
  {
    // mod longfei [drop index] 20151026
    // search index table's schema in __all_secondary_index

    //----------------old----------------//
    //ret = nb_accessor_.get(res, FIRST_TABLET_TABLE_NAME, rowkey, SC("table_id"));

    //----------------new----------------//
    if(is_index_table_or_not(table_name))
    {
      ret = nb_accessor_.get(res, OB_ALL_SECONDAYR_INDEX_TABLE_NAME, rowkey, SC("table_id"));
    }
    else
    {
      ret = nb_accessor_.get(res, FIRST_TABLET_TABLE_NAME, rowkey, SC("table_id"));
    }
    // mod e
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "get table schema fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    table_row = res->get_only_one_row();
    if(NULL != table_row)
    {
      ASSIGN_INT("table_id", table_id, uint64_t);
    }
    else
    {
      ret = OB_ENTRY_NOT_EXIST;
      TBSYS_LOG(DEBUG, "get table row fail:table_name[%.*s]", table_name.length(), table_name.ptr());
    }
  }

  nb_accessor_.release_query_res(res);
  res = NULL;

  return ret;
}

int ObSchemaServiceImpl::assemble_table(const TableRow* table_row, TableSchema& table_schema)
{
  int ret = OB_SUCCESS;
  ASSIGN_VARCHAR("table_name", table_schema.table_name_, OB_MAX_COLUMN_NAME_LENGTH);
  /* !! OBSOLETE CODE !! no need extract rowkey field when updateserver supports ROWKEY column query.
  if (table_schema.table_name_[0] == '\0' || OB_SUCCESS != ret)
  {
    ret = OB_SUCCESS;
    ASSIGN_VARCHAR_FROM_ROWKEY("table_name", 0, table_schema.table_name_, OB_MAX_COLUMN_NAME_LENGTH);
    TBSYS_LOG(WARN, "assemble_table table_name=%s", table_schema.table_name_);
  }
  */

  ASSIGN_INT("table_id", table_schema.table_id_, uint64_t);
  ASSIGN_INT("table_type", table_schema.table_type_, TableSchema::TableType);
  ASSIGN_INT("load_type", table_schema.load_type_, TableSchema::LoadType);
  ASSIGN_INT("table_def_type", table_schema.table_def_type_, TableSchema::TableDefType);
  ASSIGN_INT("rowkey_column_num", table_schema.rowkey_column_num_, int32_t);
  ASSIGN_INT("replica_num", table_schema.replica_num_, int32_t);
  ASSIGN_INT("max_used_column_id", table_schema.max_used_column_id_, int64_t);
  ASSIGN_INT("create_mem_version", table_schema.create_mem_version_, int64_t);
  ASSIGN_INT("tablet_max_size", table_schema.tablet_max_size_, int64_t);
  ASSIGN_INT("tablet_block_size", table_schema.tablet_block_size_, int64_t);
  if (OB_SUCCESS == ret && table_schema.tablet_block_size_ <= 0)
  {
    TBSYS_LOG(WARN, "set tablet sstable block size to default value:read[%ld]", table_schema.tablet_block_size_);
    table_schema.tablet_block_size_ = OB_DEFAULT_SSTABLE_BLOCK_SIZE;
  }
  ASSIGN_INT("max_rowkey_length", table_schema.max_rowkey_length_, int64_t);
  ASSIGN_INT("merge_write_sstable_version", table_schema.merge_write_sstable_version_, int64_t);
  ASSIGN_INT("schema_version", table_schema.schema_version_, int64_t);
  ASSIGN_VARCHAR("compress_func_name", table_schema.compress_func_name_, OB_MAX_COLUMN_NAME_LENGTH);
  ASSIGN_VARCHAR("expire_condition", table_schema.expire_condition_, OB_MAX_EXPIRE_CONDITION_LENGTH);
  //ASSIGN_VARCHAR("comment_str", table_schema.comment_str_, OB_MAX_TABLE_COMMENT_LENGTH);
  ASSIGN_INT("is_use_bloomfilter", table_schema.is_use_bloomfilter_, int64_t);
  ASSIGN_INT("is_pure_update_table", table_schema.is_pure_update_table_, int64_t);
  ASSIGN_INT("is_read_static", table_schema.consistency_level_, int64_t);
  //ASSIGN_INT("consistency_level", table_schema.consistency_level_, int64_t);
  ASSIGN_INT("rowkey_split", table_schema.rowkey_split_, int64_t);
  ASSIGN_INT("create_time_column_id", table_schema.create_time_column_id_, uint64_t);
  ASSIGN_INT("modify_time_column_id", table_schema.modify_time_column_id_, uint64_t);
  TBSYS_LOG(DEBUG, "table schema version is %ld, maxcolid=%lu, compress_fuction=%s, expire_condition=%s",
      table_schema.schema_version_, table_schema.max_used_column_id_, table_schema.compress_func_name_, table_schema.expire_condition_);
  return ret;
}

int ObSchemaServiceImpl::assemble_column(const TableRow* table_row, ColumnSchema& column)
{
  int ret = OB_SUCCESS;

  ASSIGN_VARCHAR("column_name", column.column_name_, OB_MAX_COLUMN_NAME_LENGTH);
  /* !! OBSOLETE CODE !! no need extract rowkey field when updateserver supports ROWKEY column query.
  if (column.column_name_[0] == '\0' || OB_SUCCESS != ret)
  {
    ret = OB_SUCCESS;
    // __all_all_column rowkey (table_id,column_name);
    ASSIGN_VARCHAR_FROM_ROWKEY("column_name", 1, column.column_name_, OB_MAX_COLUMN_NAME_LENGTH);
    TBSYS_LOG(WARN, "assemble_column column_name_=%s", column.column_name_);
  }
  */

  ASSIGN_INT("column_id", column.column_id_, uint64_t);
  ASSIGN_INT("column_group_id", column.column_group_id_, uint64_t);
  ASSIGN_INT("rowkey_id", column.rowkey_id_, int64_t);
  ASSIGN_INT("join_table_id", column.join_table_id_, uint64_t);
  ASSIGN_INT("join_column_id", column.join_column_id_, uint64_t);
  ASSIGN_INT("data_type", column.data_type_, ColumnType);
  ASSIGN_INT("data_length", column.data_length_, int64_t);
  ASSIGN_INT("data_precision", column.data_precision_, int64_t);
  ASSIGN_INT("data_scale", column.data_scale_, int64_t);
  ASSIGN_INT("nullable", column.nullable_, int64_t);
  //add lbzhong [auto_increment] 20161123:b
  ASSIGN_INT("auto_increment", column.auto_increment_, int64_t);
  //add:e
  ASSIGN_INT("length_in_rowkey", column.length_in_rowkey_, int64_t);
  ASSIGN_INT("order_in_rowkey", column.order_in_rowkey_, int32_t);
  ASSIGN_CREATE_TIME("gm_create", column.gm_create_, ObCreateTime);
  ASSIGN_MODIFY_TIME("gm_modify", column.gm_modify_, ObModifyTime);

  return ret;
}

int ObSchemaServiceImpl::assemble_join_info(const TableRow* table_row, JoinInfo& join_info)
{
  int ret = OB_SUCCESS;
  ASSIGN_VARCHAR("left_table_name", join_info.left_table_name_, OB_MAX_TABLE_NAME_LENGTH);
  ASSIGN_INT("left_table_id", join_info.left_table_id_, uint64_t);
  //ASSIGN_INT_FROM_ROWKEY("left_table_id", 0, join_info.left_table_id_, uint64_t);
  ASSIGN_VARCHAR("left_column_name", join_info.left_column_name_, OB_MAX_COLUMN_NAME_LENGTH);
  ASSIGN_INT("left_column_id", join_info.left_column_id_, uint64_t);
  //ASSIGN_INT_FROM_ROWKEY("left_column_id", 1, join_info.left_column_id_, uint64_t);
  ASSIGN_VARCHAR("right_table_name", join_info.right_table_name_, OB_MAX_TABLE_NAME_LENGTH);
  ASSIGN_INT("right_table_id", join_info.right_table_id_, uint64_t);
  //ASSIGN_INT_FROM_ROWKEY("right_table_id", 2, join_info.right_table_id_, uint64_t);
  ASSIGN_VARCHAR("right_column_name", join_info.right_column_name_, OB_MAX_COLUMN_NAME_LENGTH);
  ASSIGN_INT("right_column_id", join_info.right_column_id_, uint64_t);
  //ASSIGN_INT_FROM_ROWKEY("right_column_id", 3, join_info.right_column_id_, uint64_t);

  return ret;
}

int ObSchemaServiceImpl::get_table_schema(const ObString& table_name, TableSchema& table_schema)
{
  int ret = OB_SUCCESS;
  table_schema.clear();
  if (table_name == first_tablet_entry_name)
  {
    ret = ObExtraTablesSchema::first_tablet_entry_schema(table_schema);
  }
  else if (table_name == column_table_name)
  {
    ret = ObExtraTablesSchema::all_all_column_schema(table_schema);
  }
  else if (table_name == joininfo_table_name)
  {
    ret = ObExtraTablesSchema::all_join_info_schema(table_schema);
  }
  else if (table_name == secondary_index_table_name)
  {
    ret = ObExtraTablesSchema::all_secondary_index_schema(table_schema); //longfei [create index]
  }
  else
  {
    if(!check_inner_stat())
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "check inner stat fail");
    }
    else
    {
      ret = fetch_table_schema(table_name, table_schema);
    }
  }

  if(OB_SUCCESS != ret)
  {
    TBSYS_LOG(WARN, "get table schema fail:ret[%d]", ret);
  }

  return ret;
}

//add by wangdonghui 20160308 :b
int ObSchemaServiceImpl::get_procedure_info(QueryRes *&res_)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(TRACE, "fetch procedure info begin");
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  ObNewRange range;
  range.border_flag_.unset_inclusive_start();
  range.border_flag_.set_inclusive_end();
  range.set_whole_range();
  if (OB_SUCCESS != (ret = nb_accessor_.scan(res_, OB_ALL_PROCEDURE_TABLE_NAME, range, SC("proc_name")("source"))))
  {
    TBSYS_LOG(WARN, "nb accessor scan fail:ret[%d]", ret);
  }
  else
  {
    TBSYS_LOG(DEBUG, "scan OB_ALL_PROCEDURE_TABLE success. scanner row count =%ld", res_->get_scanner()->get_row_num());
  }
  if (NULL == &res_)
  {
    ret = OB_ERR_UNEXPECTED;
    TBSYS_LOG(ERROR, "results is NULL");
  }
  return ret;
}
//add :e


int ObSchemaServiceImpl::fetch_table_schema(const ObString& table_name, TableSchema& table_schema)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(TRACE, "fetch_table_schema begin: table_name=%.*s,", table_name.length(), table_name.ptr());

  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }

  QueryRes* res = NULL;

  ObRowkey rowkey;

  ObObj table_name_obj;
  table_name_obj.set_varchar(table_name);
  rowkey.assign(&table_name_obj, 1);

  TableRow* table_row = NULL;

  // mod longfei [create index]
  if(OB_SUCCESS == ret)
  {

	  if(is_index_table_or_not(table_name))
	 // if(0)
	  {
		  ret = nb_accessor_.get(res, OB_ALL_SECONDAYR_INDEX_TABLE_NAME, rowkey, SC("table_name")("table_id")
		          ("table_type")("load_type")("table_def_type")("rowkey_column_num")("replica_num")
		          ("max_used_column_id")("create_mem_version")("tablet_max_size")("tablet_block_size")
		          ("max_rowkey_length")("compress_func_name")("expire_condition")("is_use_bloomfilter")
		          ("is_read_static")("merge_write_sstable_version")("schema_version")("is_pure_update_table")("rowkey_split")
				  ("original_table_id")("index_status")
		          ("create_time_column_id")("modify_time_column_id"));
	  }
	  // mod e
	  else
	  {
		ret = nb_accessor_.get(res, FIRST_TABLET_TABLE_NAME, rowkey, SC("table_name")("table_id")
			("table_type")("load_type")("table_def_type")("rowkey_column_num")("replica_num")
			("max_used_column_id")("create_mem_version")("tablet_max_size")("tablet_block_size")
			("max_rowkey_length")("compress_func_name")("expire_condition")("is_use_bloomfilter")
			("is_read_static")("merge_write_sstable_version")("schema_version")("is_pure_update_table")("rowkey_split")
			("create_time_column_id")("modify_time_column_id"));
	  }
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "get table schema fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    table_row = res->get_only_one_row();
    if(NULL != table_row)
    {
    	//mod longfei [create index]
      if(is_index_table_or_not(table_name))
      //if(0)
      {
    	  ret = assemble_index_table(table_row, table_schema);
      }
      // mod e
      else
      {
    	  ret = assemble_table(table_row, table_schema);
      }
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "assemble table fail:ret[%d]", ret);
      }
    }
    else
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "get table row fail:table_name[%.*s]", table_name.length(), table_name.ptr());
    }
  }
  nb_accessor_.release_query_res(res);
  res = NULL;

  ObNewRange range;
  int32_t rowkey_column = 2;
  ObObj start_rowkey[rowkey_column];
  ObObj end_rowkey[rowkey_column];
  start_rowkey[0].set_int(table_schema.table_id_);
  start_rowkey[1].set_min_value();
  end_rowkey[0].set_int(table_schema.table_id_);
  end_rowkey[1].set_max_value();
  if (OB_SUCCESS == ret)
  {
    range.start_key_.assign(start_rowkey, rowkey_column);
    range.end_key_.assign(end_rowkey, rowkey_column);
  }
  if(OB_SUCCESS == ret)
  {
    //mod lbzhong [auto_increment] 20161123:b
    //add ("auto_increment")
    /*
    ret = nb_accessor_.scan(res, OB_ALL_COLUMN_TABLE_NAME, range,
        SC("column_name")("column_id")("gm_create")("gm_modify")("column_group_id")("rowkey_id")
        ("join_table_id")("join_column_id")("data_type")("data_length")("data_precision")
        ("data_scale")("nullable")("length_in_rowkey")("order_in_rowkey"),
        ScanConds("table_id", EQ, table_schema.table_id_));
        */
    ret = nb_accessor_.scan(res, OB_ALL_COLUMN_TABLE_NAME, range,
        SC("column_name")("column_id")("gm_create")("gm_modify")("column_group_id")("rowkey_id")
        ("join_table_id")("join_column_id")("data_type")("data_length")("data_precision")
        ("data_scale")("nullable")("auto_increment")("length_in_rowkey")("order_in_rowkey"),
        ScanConds("table_id", EQ, table_schema.table_id_));
    //mod:e
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "scan column table fail:ret[%d]", ret);
    }
  }

  ColumnSchema column;

  if(OB_SUCCESS == ret)
  {
    int i = 0;
    while(OB_SUCCESS == res->next_row() && OB_SUCCESS == ret)
    {
      res->get_row(&table_row);
      if(NULL != table_row)
      {
        i ++;
        ret = assemble_column(table_row, column);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "assemble column fail:ret[%d]", ret);
        }

        if(OB_SUCCESS == ret)
        {
          ret = table_schema.add_column(column);
          if(OB_SUCCESS != ret)
          {
            TBSYS_LOG(WARN, "add column to table schema fail:ret[%d]", ret);
          }
        }
      }
      else
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "get column fail");
      }
    }
  }

  nb_accessor_.release_query_res(res);
  res = NULL;

  ObNewRange join_range;
  int32_t rowkey_column_num = 4;
  ObObj start_obj[rowkey_column_num];
  ObObj end_obj[rowkey_column_num];
  start_obj[0].set_int(table_schema.table_id_);
  end_obj[0].set_int(table_schema.table_id_);
  for (int32_t i = 1; i < rowkey_column_num; i++)
  {
    start_obj[i].set_min_value();
    end_obj[i].set_max_value();
  }
  join_range.start_key_.assign(start_obj, rowkey_column_num);
  join_range.end_key_.assign(end_obj, rowkey_column_num);
  if(OB_SUCCESS == ret)
  {
    ret = nb_accessor_.scan(res, OB_ALL_JOININFO_TABLE_NAME, join_range,
        SC("left_table_id")("left_column_id")("right_table_id")("right_column_id")
        ("left_table_name")("left_column_name")("right_table_name")("right_column_name"),
        ScanConds("left_table_id", EQ, table_schema.table_id_));
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "scan join info table fail:ret[%d]", ret);
    }
  }

  JoinInfo join_info;

  if(OB_SUCCESS == ret)
  {
    while(OB_SUCCESS == res->next_row() && OB_SUCCESS == ret)
    {
      res->get_row(&table_row);
      if(NULL != table_row)
      {
        ret = assemble_join_info(table_row, join_info);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "assemble join info fail:ret[%d]", ret);
        }

        if(OB_SUCCESS == ret)
        {
          ret = table_schema.add_join_info(join_info);
          if(OB_SUCCESS != ret)
          {
            TBSYS_LOG(WARN, "add join info to table schema fail:ret[%d]", ret);
          }
        }
      }
      else
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "get join info fail");
      }
    }
  }

  if (OB_SUCCESS == ret)
  {
    if (!table_schema.is_valid())
    {
      ret = OB_ERR_UNEXPECTED;
      TBSYS_LOG(ERROR, "check the table schema is invalid:table_name[%s]", table_schema.table_name_);
    }
  }

  nb_accessor_.release_query_res(res);
  res = NULL;

  return ret;
}

int ObSchemaServiceImpl::set_max_used_table_id(const uint64_t max_used_tid)
{
  int ret = OB_SUCCESS;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  else
  {
    ObObj rowkey_objs[2];
    rowkey_objs[0].set_int(0); // cluster_id
    rowkey_objs[1].set_varchar(ObString::make_string("ob_max_used_table_id")); // name
    ObRowkey rowkey;
    rowkey.assign(rowkey_objs, 2);
    ObString value;
    char buf[64] = "";
    snprintf(buf, sizeof(buf), "%lu", max_used_tid);
    value.assign(buf, static_cast<int32_t>(strlen(buf)));
    KV new_value("value", value);
    // TODO should using add 1 operator
    if (OB_SUCCESS != (ret = nb_accessor_.update(OB_ALL_SYS_STAT_TABLE_NAME, rowkey, new_value)))
    {
      TBSYS_LOG(WARN, "failed to update the row, err=%d", ret);
    }
  }
  return ret;
}

int ObSchemaServiceImpl::get_max_used_table_id(uint64_t &max_used_tid)
{
  int ret = OB_SUCCESS;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  else
  {
    ObObj rowkey_objs[2];
    rowkey_objs[0].set_int(0); // cluster_id
    rowkey_objs[1].set_varchar(ObString::make_string("ob_max_used_table_id")); // name
    ObRowkey rowkey;
    rowkey.assign(rowkey_objs, 2);
    QueryRes* res = NULL;
    if (OB_SUCCESS != (ret = nb_accessor_.get(res, OB_ALL_SYS_STAT_TABLE_NAME, rowkey, SC("value"))))
    {
      TBSYS_LOG(WARN, "failed to access row, err=%d", ret);
    }
    else
    {
      TableRow* table_row = res->get_only_one_row();
      if (NULL == table_row)
      {
        TBSYS_LOG(WARN, "failed to get row from query results");
        ret = OB_ERR_UNEXPECTED;
      }
      else
      {
        char value_buf[TEMP_VALUE_BUFFER_LEN] = "";
        ASSIGN_VARCHAR("value", value_buf, TEMP_VALUE_BUFFER_LEN);
        max_used_tid = strtoul(value_buf, NULL, 10);
        TBSYS_LOG(TRACE, "get max used id succ:id[%lu]", max_used_tid);
      }
      nb_accessor_.release_query_res(res);
      res = NULL;
    }
  }
  return ret;
}

int ObSchemaServiceImpl::alter_table(const AlterTableSchema & schema, const int64_t old_schema_version)
{
  int ret = OB_SUCCESS;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  tbsys::CThreadGuard guard(&mutex_);
  ObMutator* mutator = NULL;
  if(OB_SUCCESS == ret)
  {
    mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
    if(NULL == mutator)
    {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      TBSYS_LOG(WARN, "get thread specific Mutator fail");
    }
  }

  if (OB_SUCCESS == ret)
  {
    ret = mutator->reset();
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "reset ob mutator fail:ret[%d]", ret);
    }
  }

  if (OB_SUCCESS == ret)
  {
    ret = alter_table_mutator(schema, mutator, old_schema_version);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "set alter table mutator fail:ret[%d]", ret);
    }
  }

  if (OB_SUCCESS == ret)
  {
    ret = client_proxy_->mutate(*mutator);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "apply mutator fail:ret[%d]", ret);
    }
    else
    {
      TBSYS_LOG(INFO, "send alter table to ups succ.");
    }
  }
  return ret;
}
int ObSchemaServiceImpl::generate_new_table_name(char* buf, const uint64_t length, const char* table_name, const uint64_t table_name_length)
{
  int ret = OB_SUCCESS;
  if (table_name_length + sizeof(TMP_PREFIX) >= length
      || NULL == buf
      || NULL == table_name)
  {
    TBSYS_LOG(WARN, "invalid buf_len. need size=%ld, exist_buf_size=%ld, buf=%p, table_name=%s",
        table_name_length + 1, length, buf, table_name);
    ret = OB_INVALID_ARGUMENT;
  }
  if (OB_SUCCESS == ret)
  {
    if (0 >= snprintf(buf, length, "%s", TMP_PREFIX))
    {
      TBSYS_LOG(WARN, "fail to print prefix to buf. buf_length=%ld, prefix_lenght=%ld", length, strlen(TMP_PREFIX));
    ret = OB_ERROR;
    }
    else if (0 >= snprintf(buf + strlen(TMP_PREFIX), length - strlen(TMP_PREFIX), "%s", table_name))
    {
      ret = OB_ERROR;
      TBSYS_LOG(WARN, "fail to print table_name to buf. pos=%ld, length=%ld", strlen(TMP_PREFIX), table_name_length);
    }
    else
    {
      buf[strlen(TMP_PREFIX) + table_name_length + 1] = '\0';
      TBSYS_LOG(INFO, "new table name is %s, tmp_prefix_len=%ld, table_name_length=%ld", buf, strlen(TMP_PREFIX), table_name_length);
    }
  }
  return ret;
}
int ObSchemaServiceImpl::modify_table_id(TableSchema& table_schema, const int64_t new_table_id)
{
  int ret = OB_SUCCESS;
  TBSYS_LOG(INFO, "modify table id. old_table_id=%ld, new_table_id=%ld", table_schema.table_id_, new_table_id);
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  int64_t old_table_id = table_schema.table_id_;
  char table_name_buf[OB_MAX_TABLE_NAME_LENGTH];
  char old_table_name_buf[OB_MAX_TABLE_NAME_LENGTH];
  ObString table_name;
  memcpy(old_table_name_buf, table_schema.table_name_, strlen(table_schema.table_name_) + 1);
  table_name.assign_ptr(old_table_name_buf, static_cast<int32_t>(strlen(table_schema.table_name_)));
  if (OB_SUCCESS == ret)
  {
    ret = generate_new_table_name(table_name_buf, OB_MAX_TABLE_NAME_LENGTH, table_schema.table_name_, strlen(table_schema.table_name_));
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "fail to genearte new table_name. table_name=%s, length=%ld",
          table_schema.table_name_, strlen(table_schema.table_name_));
    }
  }
  // common::TableSchema new_table_schema = table_schema;
  if (OB_SUCCESS == ret)
  {
    memcpy(table_schema.table_name_, table_name_buf, strlen(table_name_buf) + 1);
    table_schema.table_name_[strlen(table_name_buf)] = '\0';
    table_schema.table_id_ = new_table_id;
    ret = create_table(table_schema);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "fail to create table. table_name=%s", table_name_buf);
    }
    else
    {
      TBSYS_LOG(INFO, "create tmp table for bypass success. table_name=%s", table_schema.table_name_);
    }
  }
  ObMutator* mutator = NULL;
  if (OB_SUCCESS == ret)
  {
    mutator = GET_TSI_MULT(ObMutator, TSI_COMMON_MUTATOR_1);
    if(NULL == mutator)
    {
      ret = OB_ALLOCATE_MEMORY_FAILED;
      TBSYS_LOG(WARN, "get thread specific ObMutator fail");
    }

    if(OB_SUCCESS == ret)
    {
      ret = mutator->reset();
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "reset ob mutator fail:ret[%d]", ret);
      }
    }
    if (OB_SUCCESS == ret)
    {
      ObRowkey rowkey;
      ObObj rowkey_value;
      ObString new_table_name;
      new_table_name.assign_ptr(table_schema.table_name_,
          static_cast<int32_t>(strlen(table_schema.table_name_)));
      rowkey_value.set_varchar(new_table_name);
      rowkey.assign(&rowkey_value, 1);
      ObObj value;
      value.set_int(old_table_id);
      ret = mutator->update(first_tablet_entry_name, rowkey, table_name_str, value);
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "fail to add update cell to mutator. ret=%d, rowkey=%s", ret, to_cstring(new_table_name));
      }
      else
      {
        TBSYS_LOG(INFO, "change table id for bypass table. table_name=%s, table_id=%ld",
            table_schema.table_name_, old_table_id);
      }
    }
    if (OB_SUCCESS == ret)
    {
      ObRowkey rowkey;
      ObObj rowkey_value;
      rowkey_value.set_varchar(table_name);
      rowkey.assign(&rowkey_value, 1);
      ObObj value;
      value.set_int(new_table_id);
      ret = mutator->update(first_tablet_entry_name, rowkey, table_name_str, value);
      if (OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "fail to add update cell to mutator. ret=%d, rowkey=%s", ret, to_cstring(table_name));
      }
      else
      {
        TBSYS_LOG(INFO, "change table id for bypass table. table_name=%s, table_id=%ld",
            to_cstring(table_name), new_table_id);
      }
    }
  }
  //add privilege
  QueryRes* res = NULL;
  TableRow* table_row = NULL;
  ObNewRange range;
  int32_t rowkey_column = 2;
  ObObj start_rowkey[rowkey_column];
  ObObj end_rowkey[rowkey_column];
  start_rowkey[1].set_int(old_table_id);
  start_rowkey[0].set_min_value();
  end_rowkey[1].set_int(old_table_id);
  end_rowkey[0].set_max_value();
  if (OB_SUCCESS == ret)
  {
    range.start_key_.assign(start_rowkey, rowkey_column);
    range.end_key_.assign(end_rowkey, rowkey_column);
  }
  if (OB_SUCCESS == ret)
  {
    ret = nb_accessor_.scan(res, OB_ALL_TABLE_PRIVILEGE_TABLE_NAME, range, SC("user_id")("priv_all")("priv_alter")("priv_create")("priv_create_user")("priv_delete")("priv_drop")("priv_grant_option")("priv_insert")("priv_update")("priv_select")("priv_replace"));
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "fail to get privilege info for table, table_id=%ld", old_table_id);
    }
  }
  if (OB_SUCCESS == ret)
  {
    while(OB_SUCCESS == res->next_row() && OB_SUCCESS == ret)
    {
      res->get_row(&table_row);
      if (NULL != table_row)
      {
        ret = prepare_privilege_for_table(table_row, mutator, new_table_id);
        if (OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "fail to get privilege info for table %ld, ret=%d", new_table_id, ret);
        }
      }
    }
  }
  nb_accessor_.release_query_res(res);
  res = NULL;
  if (OB_SUCCESS == ret)
  {
    ret = client_proxy_->mutate(*mutator);
    if (OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "apply mutator to update server fail. ret=%d", ret);
      while (OB_SUCCESS == mutator->next_cell())
      {
        ObMutatorCellInfo *ci = NULL;
        bool is_row_changed = false;
        bool is_row_finished = false;
        mutator->get_cell(&ci, &is_row_changed, &is_row_finished);
        TBSYS_LOG(INFO, "%s\n", common::print_cellinfo(&ci->cell_info));
      }
    }
  }
  ObString drop_table_name;
  drop_table_name.assign_ptr(table_schema.table_name_, static_cast<int32_t>(strlen(table_schema.table_name_)));
  if (OB_SUCCESS != drop_table(drop_table_name))
  {
    TBSYS_LOG(WARN, "fail to drop table. table_name=%s", table_schema.table_name_);
  }
  else
  {
    TBSYS_LOG(INFO, "drop tmp table for bypass. table_name=%s", table_schema.table_name_);
  }
  return ret;
}

int ObSchemaServiceImpl::prepare_privilege_for_table(const TableRow* table_row, ObMutator *mutator, const int64_t table_id)
{
  int ret = OB_SUCCESS;
  int64_t user_id = 0;
  ASSIGN_INT("user_id", user_id, uint64_t);
  int64_t priv_all = 0;
  ASSIGN_INT("priv_all", priv_all, uint64_t);
  int64_t priv_alter = 0;
  ASSIGN_INT("priv_alter", priv_alter, uint64_t);
  int64_t priv_create = 0;
  ASSIGN_INT("priv_create", priv_create, uint64_t);
  int64_t priv_create_user = 0;
  ASSIGN_INT("priv_create_user", priv_create_user, uint64_t);
  int64_t priv_delete = 0;
  ASSIGN_INT("priv_delete", priv_delete, uint64_t);
  int64_t priv_drop = 0;
  ASSIGN_INT("priv_drop", priv_drop, uint64_t);
  int64_t priv_grant_option = 0;
  ASSIGN_INT("priv_grant_option", priv_grant_option, uint64_t);
  int64_t priv_insert = 0;
  ASSIGN_INT("priv_insert", priv_insert, uint64_t);
  int64_t priv_update = 0;
  ASSIGN_INT("priv_update", priv_update, uint64_t);
  int64_t priv_select = 0;
  ASSIGN_INT("priv_select", priv_select, uint64_t);
  int64_t priv_replace = 0;
  ASSIGN_INT("priv_replace", priv_replace, uint64_t);
  TBSYS_LOG(INFO, "use id=%ld, priv_all=%ld, priv_alter=%ld, priv_create=%ld, priv_create_user=%ld, priv_delete=%ld, priv_drop=%ld, priv_grant_option=%ld, priv_insert=%ld, priv_update=%ld, priv_select=%ld, priv_replace=%ld,",
      user_id, priv_all, priv_alter, priv_create, priv_create_user, priv_delete, priv_drop, priv_grant_option, priv_insert, priv_update, priv_select, priv_replace);
  ObRowkey rowkey;
  ObObj value[2];
  value[0].set_int(user_id);
  value[1].set_int(table_id);
  rowkey.assign(value, 2);
  ADD_INT(privilege_table_name, rowkey, "priv_all", priv_all);
  ADD_INT(privilege_table_name, rowkey, "priv_alter", priv_alter);
  ADD_INT(privilege_table_name, rowkey, "priv_create", priv_create);
  ADD_INT(privilege_table_name, rowkey, "priv_create_user", priv_create_user);
  ADD_INT(privilege_table_name, rowkey, "priv_delete", priv_delete);
  ADD_INT(privilege_table_name, rowkey, "priv_drop", priv_drop);
  ADD_INT(privilege_table_name, rowkey, "priv_grant_option", priv_grant_option);
  ADD_INT(privilege_table_name, rowkey, "priv_insert", priv_insert);
  ADD_INT(privilege_table_name, rowkey, "priv_update", priv_update);
  ADD_INT(privilege_table_name, rowkey, "priv_select", priv_select);
  ADD_INT(privilege_table_name, rowkey, "priv_replace", priv_replace);
  return ret;
}

int ObSchemaServiceImpl::create_index_mutator(const TableSchema& table_schema, ObMutator* mutator)
{
  int ret = OB_SUCCESS;

  ObString table_name;
  table_name.assign_ptr(const_cast<char*>(table_schema.table_name_), static_cast<int32_t>(strlen(table_schema.table_name_)));

  ObObj table_name_value;
  table_name_value.set_varchar(table_name);

  ObRowkey rowkey;
  rowkey.assign(&table_name_value, 1);

  //ADD_VARCHAR(secondary_index_table_name, rowkey, "table_name", table_schema.table_name_);
  ADD_INT(secondary_index_table_name, rowkey, "table_id", table_schema.table_id_);
  ADD_INT(secondary_index_table_name, rowkey, "table_type", table_schema.table_type_);
  ADD_INT(secondary_index_table_name, rowkey, "load_type", table_schema.load_type_);
  ADD_INT(secondary_index_table_name, rowkey, "table_def_type", table_schema.table_def_type_);
  ADD_INT(secondary_index_table_name, rowkey, "rowkey_column_num", table_schema.rowkey_column_num_);
  ADD_INT(secondary_index_table_name, rowkey, "replica_num", table_schema.replica_num_);
  ADD_INT(secondary_index_table_name, rowkey, "max_used_column_id", table_schema.max_used_column_id_);
  ADD_INT(secondary_index_table_name, rowkey, "create_mem_version", table_schema.create_mem_version_);
  ADD_INT(secondary_index_table_name, rowkey, "tablet_max_size", table_schema.tablet_max_size_);
  ADD_INT(secondary_index_table_name, rowkey, "tablet_block_size", table_schema.tablet_block_size_);
  ADD_VARCHAR(secondary_index_table_name, rowkey, "compress_func_name", table_schema.compress_func_name_);

  ADD_INT(secondary_index_table_name, rowkey, "is_use_bloomfilter", table_schema.is_use_bloomfilter_);
  ADD_INT(secondary_index_table_name, rowkey, "is_pure_update_table", table_schema.is_pure_update_table_);
  //ADD_INT(secondary_index_table_name, rowkey, "consistency_level", table_schema.consistency_level_);
  ADD_INT(secondary_index_table_name, rowkey, "is_read_static", table_schema.consistency_level_);
  ADD_INT(secondary_index_table_name, rowkey, "rowkey_split", table_schema.rowkey_split_);
  ADD_INT(secondary_index_table_name, rowkey, "max_rowkey_length", table_schema.max_rowkey_length_);
  ADD_INT(secondary_index_table_name, rowkey, "merge_write_sstable_version", table_schema.merge_write_sstable_version_);
  ADD_INT(secondary_index_table_name, rowkey, "schema_version", table_schema.schema_version_);
  ADD_VARCHAR(secondary_index_table_name, rowkey, "expire_condition", table_schema.expire_condition_);
  //ADD_VARCHAR(secondary_index_table_name, rowkey, "comment_str", table_schema.comment_str_);
  ADD_INT(secondary_index_table_name, rowkey, "original_table_id", table_schema.original_table_id_);
  ADD_INT(secondary_index_table_name, rowkey, "index_status", table_schema.index_status_);
  ADD_INT(secondary_index_table_name, rowkey, "create_time_column_id", table_schema.create_time_column_id_);
  ADD_INT(secondary_index_table_name, rowkey, "modify_time_column_id", table_schema.modify_time_column_id_);

  if(OB_SUCCESS == ret)
  {
    ret = add_column(mutator, table_schema);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "add column to mutator fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    ret = add_join_info(mutator, table_schema);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "add join info to mutator fail:ret[%d]", ret);
    }
  }

  //add lbzhong [auto_increment] 20161201:b
  if (OB_SUCCESS == ret)
  {
    uint64_t column_id = OB_INVALID_ID;
    if (OB_SUCCESS != (ret = check_auto_increment(table_schema, column_id)))
    {
      TBSYS_LOG(WARN, "check auto_increment fail:ret[%d]", ret);
    }
    else if (OB_INVALID_ID != column_id && OB_SUCCESS != (ret = auto_increment_mutator(table_schema.table_id_, column_id, mutator)))
    {
      TBSYS_LOG(WARN, "add auto_increment mutator fail:ret[%d]", ret);
    }
  }
  //add:e
  return ret;
}

/**
 * @brief ObSchemaServiceImpl::is_index_table_or_not 判断是否是索引表
 * @param table_name
 * @return true or false
 */
bool ObSchemaServiceImpl::is_index_table_or_not(const ObString& table_name)
{

	int64_t len = 1;
	bool flag = true;
  char temp[table_name.length()];
	len = table_name.to_string(temp,table_name.length());
	if(len <= 0)
	{
		TBSYS_LOG(WARN,"table name can not be empty.");
		flag = 0;
	}
	else if(len < 3)
	{
		TBSYS_LOG(WARN,"this is not a index name");
		flag = 0;
	}
	else
	{
		if(temp[0] != '_' || temp[1] != '_' || temp[2] != '_')
		{
			flag = 0;
		}
	}
	if(flag)
	{
		return true;
	}
	else
	{
		return false;
	}
}
int ObSchemaServiceImpl::assemble_index_table(const TableRow* table_row, TableSchema& table_schema)
{
  int ret = OB_SUCCESS;
  ASSIGN_VARCHAR("table_name", table_schema.table_name_, OB_MAX_TABLE_NAME_LENGTH);
  /* !! OBSOLETE CODE !! no need extract rowkey field when updateserver supports ROWKEY column query.
  if (table_schema.table_name_[0] == '\0' || OB_SUCCESS != ret)
  {
    ret = OB_SUCCESS;
    ASSIGN_VARCHAR_FROM_ROWKEY("table_name", 0, table_schema.table_name_, OB_MAX_COLUMN_NAME_LENGTH);
    TBSYS_LOG(WARN, "assemble_table table_name=%s", table_schema.table_name_);
  }
  */

  ASSIGN_INT("table_id", table_schema.table_id_, uint64_t);
  ASSIGN_INT("table_type", table_schema.table_type_, TableSchema::TableType);
  ASSIGN_INT("load_type", table_schema.load_type_, TableSchema::LoadType);
  ASSIGN_INT("table_def_type", table_schema.table_def_type_, TableSchema::TableDefType);
  ASSIGN_INT("rowkey_column_num", table_schema.rowkey_column_num_, int32_t);
  ASSIGN_INT("replica_num", table_schema.replica_num_, int32_t);
  ASSIGN_INT("max_used_column_id", table_schema.max_used_column_id_, int64_t);
  ASSIGN_INT("create_mem_version", table_schema.create_mem_version_, int64_t);
  ASSIGN_INT("tablet_max_size", table_schema.tablet_max_size_, int64_t);
  ASSIGN_INT("tablet_block_size", table_schema.tablet_block_size_, int64_t);
  if (OB_SUCCESS == ret && table_schema.tablet_block_size_ <= 0)
  {
    TBSYS_LOG(WARN, "set tablet sstable block size to default value:read[%ld]", table_schema.tablet_block_size_);
    table_schema.tablet_block_size_ = OB_DEFAULT_SSTABLE_BLOCK_SIZE;
  }
  ASSIGN_INT("max_rowkey_length", table_schema.max_rowkey_length_, int64_t);
  ASSIGN_INT("merge_write_sstable_version", table_schema.merge_write_sstable_version_, int64_t);
  ASSIGN_INT("schema_version", table_schema.schema_version_, int64_t);
  ASSIGN_VARCHAR("compress_func_name", table_schema.compress_func_name_, OB_MAX_COLUMN_NAME_LENGTH);
  ASSIGN_VARCHAR("expire_condition", table_schema.expire_condition_, OB_MAX_EXPIRE_CONDITION_LENGTH);
  //ASSIGN_VARCHAR("comment_str", table_schema.comment_str_, OB_MAX_TABLE_COMMENT_LENGTH);
  ASSIGN_INT("is_use_bloomfilter", table_schema.is_use_bloomfilter_, int64_t);
  ASSIGN_INT("is_pure_update_table", table_schema.is_pure_update_table_, int64_t);
  ASSIGN_INT("is_read_static", table_schema.consistency_level_, int64_t);
  //ASSIGN_INT("consistency_level", table_schema.consistency_level_, int64_t);
  ASSIGN_INT("rowkey_split", table_schema.rowkey_split_, int64_t);
  ASSIGN_INT("original_table_id", table_schema.original_table_id_, uint64_t);
  ASSIGN_INT("index_status", table_schema.index_status_,  IndexStatus);
  ASSIGN_INT("create_time_column_id", table_schema.create_time_column_id_, uint64_t);
  ASSIGN_INT("modify_time_column_id", table_schema.modify_time_column_id_, uint64_t);
  TBSYS_LOG(DEBUG, "table schema version is %ld, maxcolid=%lu, compress_fuction=%s, expire_condition=%s",
      table_schema.schema_version_, table_schema.max_used_column_id_, table_schema.compress_func_name_, table_schema.expire_condition_);
  return ret;
}

//add maoxx
int ObSchemaServiceImpl::check_column_checksum(const int64_t orginal_table_id, const int64_t index_table_id, const int64_t cluster_id, const int64_t current_version, bool &column_checksum_flag)
{
    int ret = OB_SUCCESS;
    QueryRes* res = NULL;
    TableRow* table_row = NULL;
    ObNewRange range;
    int64_t cell_index = 0;//因为只取出column checksum列的数据，所以为0
    int64_t rowkey_column_num = 4;
    ObObj start_rowkey[rowkey_column_num];
    ObObj end_rowkey[rowkey_column_num];
    ObColumnChecksum original_table_column_checksum;
    ObColumnChecksum index_table_column_checksum;
    ObColumnChecksum row_column_checksum;

    ObString tmp_string;
    char tmp_char[OB_MAX_COL_CHECKSUM_STR_LEN];
    tmp_string.assign_ptr(tmp_char, OB_MAX_COL_CHECKSUM_STR_LEN);

    original_table_column_checksum.reset();
    index_table_column_checksum.reset();

    start_rowkey[0].set_int(orginal_table_id);
    end_rowkey[0].set_int(orginal_table_id);
    start_rowkey[1].set_int(cluster_id);
    end_rowkey[1].set_int(cluster_id);
    start_rowkey[2].set_int(current_version);
    end_rowkey[2].set_int(current_version);
    start_rowkey[3].set_min_value();
    end_rowkey[3].set_max_value();

    range.start_key_.assign(start_rowkey, rowkey_column_num);
    range.end_key_.assign(end_rowkey, rowkey_column_num);
    range.border_flag_.inclusive_end();
    range.border_flag_.inclusive_start();

    if(OB_SUCCESS != (ret = nb_accessor_.scan(res, OB_ALL_COLUMN_CHECKSUM_INFO_TABLE_NAME, range, SC("column_checksum"))))
    {
      TBSYS_LOG(WARN, "fail to nb scan column checksum info of orginal table %d", ret);
    }
    else
    {
      TBSYS_LOG(INFO, "start calculate column checksum of orginal_table_id:%ld", orginal_table_id);
      while(OB_SUCCESS == (ret = res->next_row()))
      {
        if(OB_SUCCESS != (ret = res->get_row(&table_row)))
        {
          TBSYS_LOG(ERROR, "failed to get next row of column checksum");
          break;
        }
        else if(NULL != table_row)
        {
          if(OB_SUCCESS != (ret = table_row->get_cell_info(cell_index)->value_.get_varchar(tmp_string)))
          {
            TBSYS_LOG(ERROR, "failed calculate column checksum of orginal_table_id:%ld", orginal_table_id);
            break;
          }
          else
          {
            row_column_checksum.deepcopy(tmp_string.ptr(),(int32_t)tmp_string.length());
            ret = original_table_column_checksum.add(row_column_checksum);
            row_column_checksum.reset();
            tmp_string.reset();
          }
        }
        else
        {
          ret = OB_ERROR;
          TBSYS_LOG(ERROR, "failed calculate column checksum of orginal_table_id:%ld", orginal_table_id);
          break;
        }
      }
      if (OB_ITER_END != ret)
      {
        TBSYS_LOG(WARN, "failed to get index table column checksum tabe_id:%ld", orginal_table_id);
      }
      else
      {
          ret = OB_SUCCESS;
      }
    }

    if(OB_SUCCESS == ret)
    {
      start_rowkey[0].set_int(index_table_id);
      end_rowkey[0].set_int(index_table_id);
      start_rowkey[1].set_int(cluster_id);
      end_rowkey[1].set_int(cluster_id);
      start_rowkey[2].set_int(current_version);
      end_rowkey[2].set_int(current_version);
      start_rowkey[3].set_min_value();
      end_rowkey[3].set_max_value();

      range.start_key_.assign(start_rowkey, rowkey_column_num);
      range.end_key_.assign(end_rowkey, rowkey_column_num);
      range.border_flag_.inclusive_end();
      range.border_flag_.inclusive_start();

      nb_accessor_.release_query_res(res);
      res = NULL;

      if(OB_SUCCESS != (ret = nb_accessor_.scan(res, OB_ALL_COLUMN_CHECKSUM_INFO_TABLE_NAME, range, SC("column_checksum"))))
      {
        TBSYS_LOG(WARN, "fail to nb scan column checksum info of index table %d", ret);
      }
      else
      {
        TBSYS_LOG(INFO, "start calculate column checksum of index_table_id:%ld", index_table_id);
        while(OB_SUCCESS == (ret = res->next_row()))
        {
          if(OB_SUCCESS != (ret = res->get_row(&table_row)))
          {
            TBSYS_LOG(ERROR, "failed to get next row of column checksum");
            break;
          }
          else if(NULL != table_row)
          {
            if(OB_SUCCESS != (ret = table_row->get_cell_info(cell_index)->value_.get_varchar(tmp_string)))
            {
              TBSYS_LOG(ERROR, "failed calculate column checksum of index_table_id:%ld", index_table_id);
              break;
            }
            else
            {
              row_column_checksum.deepcopy(tmp_string.ptr(),(int32_t)tmp_string.length());
              ret = index_table_column_checksum.add(row_column_checksum);
              row_column_checksum.reset();
              tmp_string.reset();
            }
          }
          else
          {
            ret = OB_ERROR;
            TBSYS_LOG(WARN, "failed calculate column checksum of index_table_id:%ld", index_table_id);
            break;
          }
        }
        if (OB_ITER_END != ret)
        {
          TBSYS_LOG(WARN, "failed to get index table column checksum tabe_id:%ld", index_table_id);
        }
        else
        {
          ret = OB_SUCCESS;
        }
      }
    }

    if (OB_SUCCESS == ret)
    {
      bool is_equal = false;
      if(OB_SUCCESS == (ret = original_table_column_checksum.equal(index_table_column_checksum, is_equal)) && is_equal)
      {
        column_checksum_flag = true;
        TBSYS_LOG(INFO, "this index table column checksum is correct table_id:%ld", index_table_id);
      }
      else
      {
        column_checksum_flag = false;
        TBSYS_LOG(WARN, "this index table column checksum is incorrect table_id:%ld", index_table_id);
      }
    }

    return ret;
}

//modify wenghaixing [secondary index.static_index]20160118
int ObSchemaServiceImpl::clean_column_checksum(const int64_t max_draution_of_version, const int64_t current_version)
{
    int ret = OB_SUCCESS;
    QueryRes* res = NULL;
    TableRow* table_row = NULL;
    ObNewRange range;
    int64_t cell_index = 0;
    //int64_t version_index  = 2;
    int64_t filter_version = current_version - max_draution_of_version;
    range.set_whole_range();
    ScanConds conds;
    conds("version", LT, filter_version);
    TBSYS_LOG(INFO, "clean version less than %ld, conds count = %ld",filter_version, conds.count());

    if(OB_SUCCESS != (ret = nb_accessor_.scan(res, OB_ALL_COLUMN_CHECKSUM_INFO_TABLE_NAME, range, SC("version"), conds)))
    {
      TBSYS_LOG(WARN, "failed to scan data to delete from column_checksum ret[%d]" , ret);
    }
    else
    {
      while(OB_SUCCESS == res->next_row()&& OB_SUCCESS == ret)
      {
        if(OB_SUCCESS != (ret = res->get_row(&table_row)))
        {
          TBSYS_LOG(ERROR, "failed to get next row of column checksum");
          break;
        }
        else if(NULL != table_row)
        {
          table_row->dump_test();
          if(OB_SUCCESS != (ret = nb_accessor_.delete_row(OB_ALL_COLUMN_CHECKSUM_INFO_TABLE_NAME, table_row->get_cell_info(cell_index)->row_key_)))
          {
            TBSYS_LOG(WARN, "failed to delete one row from column cheksum ret[%d]", ret);
            break;
          }
        }
        else
        {
          ret = OB_ERROR;
          TBSYS_LOG(WARN, "delete column checksum row is NULL");
          break;
        }
      }
    }
    return ret;
}
//modify e

int ObSchemaServiceImpl::get_column_checksum(const ObNewRange range, const int64_t cluster_id, const int64_t required_version, ObString& column_checksum)
{
  int ret = OB_SUCCESS;
  QueryRes* res = NULL;
  TableRow* table_row = NULL;
  ObRowkey rowkey;
  ObObj rowkey_list[4];

  char range_buf[OB_RANGE_STR_BUFSIZ];
  range.to_string(range_buf, sizeof(range_buf));
  int32_t len = static_cast<int32_t>(strlen(range_buf));
  ObString str_range(0, len, range_buf);

  rowkey_list[0].set_int(range.table_id_);
  rowkey_list[1].set_int(cluster_id);
  rowkey_list[2].set_int(required_version);
  rowkey_list[3].set_varchar(str_range);
  rowkey.assign(rowkey_list,4);

  char varchar_column_checksum[common::OB_MAX_COL_CHECKSUM_STR_LEN];
  if(OB_SUCCESS != ( ret = nb_accessor_.get(res, OB_ALL_COLUMN_CHECKSUM_INFO_TABLE_NAME, rowkey, SC("column_checksum"))))
  {
    TBSYS_LOG(ERROR, "faild to get nb column checksum ret = %d", ret);
  }
  else
  {
    table_row = res->get_only_one_row();
    if(NULL != table_row)
    {
      ASSIGN_VARCHAR("column_checksum", varchar_column_checksum, common::OB_MAX_COL_CHECKSUM_STR_LEN);
      len = static_cast<int32_t>(strlen(varchar_column_checksum));
      column_checksum.write(varchar_column_checksum, len);
    }
    else
    {
      ret = OB_ENTRY_NOT_EXIST;
      TBSYS_LOG(WARN, "get table row fail ret = %d", ret);
    }
  }
  memset(range_buf, 0, OB_RANGE_STR_BUFSIZ*sizeof(char));
  return ret;
}
//add e

//add wenghaixing [secondary index.static_index]20151217
int ObSchemaServiceImpl::get_index_stat(const uint64_t table_id, const int64_t cluster_count, IndexStatus &stat)
{
  int ret = OB_SUCCESS;
  stat = INDEX_INIT;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  else
  {
    QueryRes* res = NULL;
    ObNewRange range;
    int32_t rowkey_column = 2;
    ObObj start_rowkey[rowkey_column];
    ObObj end_rowkey[rowkey_column];
    start_rowkey[0].set_int(table_id);
    start_rowkey[1].set_min_value();
    end_rowkey[0].set_int(table_id);
    end_rowkey[1].set_max_value();
    if (OB_SUCCESS == ret)
    {
      range.start_key_.assign(start_rowkey, rowkey_column);
      range.end_key_.assign(end_rowkey, rowkey_column);
    }
    if(OB_SUCCESS == ret)
    {
      ret = nb_accessor_.scan(res, OB_INDEX_SERVICE_INFO_TABLE_NAME, range, SC("status"));
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "scan __index_process_info fail:ret[%d]", ret);
      }
    }

    if(OB_SUCCESS == ret)
    {
      int64_t count = 0;
      int32_t st = -1;
      IndexStatus tmp_stat = INDEX_INIT;
      TableRow* table_row = NULL;
      int err = OB_SUCCESS;
      while(OB_SUCCESS == (err = res->next_row()) && OB_SUCCESS == ret)
      {
        res->get_row(&table_row);
        if (NULL == table_row)
        {
          TBSYS_LOG(WARN, "failed to get row from query results");
          ret = OB_ERROR;
        }
        else
        {
          count++;
          ASSIGN_INT("status", st, int32_t);
          switch (st) {
            case 0:
              tmp_stat = NOT_AVALIBALE;
              break;
            case 1:
              tmp_stat = AVALIBALE;
              break;
            case 2:
              tmp_stat = ERROR;
              break;
            case 3:
              tmp_stat = WRITE_ONLY;
              break;
            case 4:
              tmp_stat = INDEX_INIT;
              break;
            default:
              break;
          }
          if (ERROR == tmp_stat)
          {
            break;
          }
        }
      }//end while

      if (OB_SUCCESS == ret)
      {
        if (ERROR == tmp_stat ||
            (NOT_AVALIBALE == tmp_stat && count == cluster_count))
        {
          stat = tmp_stat;
        }
        /*else
        {
          ret = OB_EAGAIN;//记录数不等于cluster数
        }*/
      }
    }

    nb_accessor_.release_query_res(res);
    res = NULL;
  }
  return ret;
}

int ObSchemaServiceImpl::get_cluster_count(int64_t &cc)
{
  int ret = OB_SUCCESS;
  if(!check_inner_stat())
  {
    ret = OB_ERROR;
    TBSYS_LOG(WARN, "check inner stat fail");
  }
  else
  {
    QueryRes* res = NULL;
    ObNewRange range;
    ObObj start_obj, end_obj;
    start_obj.set_min_value();
    end_obj.set_max_value();
    range.start_key_.assign(&start_obj, 1);
    range.end_key_.assign(&end_obj, 1);
    if(OB_SUCCESS == ret)
    {
      ret = nb_accessor_.scan(res, OB_ALL_CLUSTER, range, SC("cluster_id"));
      if(OB_SUCCESS != ret)
      {
        TBSYS_LOG(WARN, "scan __all_cluster fail:ret[%d]", ret);
      }
      else
      {
        int64_t cluster_count = 0;
        while(OB_SUCCESS == res->next_row())
        {
          cluster_count++;
        }
        cc = cluster_count;
      }
    }

    nb_accessor_.release_query_res(res);
    res = NULL;
  }
  return ret;
}
//add e

//add lbzhong [auto_increment] 20161201:b
int ObSchemaServiceImpl::check_auto_increment(const TableSchema& table_schema, uint64_t& column_id)
{
  int ret = OB_SUCCESS;
  ColumnSchema column;
  for(int32_t i=0; i<table_schema.columns_.count() && OB_SUCCESS == ret; i++)
  {
    ret = table_schema.columns_.at(i, column);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "get column from table_schema fail:ret[%d], i[%d]", ret, i);
    }
    else if (column.auto_increment_)
    {
      column_id = column.column_id_;
      break;
    }
  }
  return ret;
}

int ObSchemaServiceImpl::auto_increment_mutator(const uint64_t table_id, const uint64_t column_id, ObMutator* mutator)
{
  int ret = OB_SUCCESS;

  if(NULL == mutator)
  {
    ret = OB_INVALID_ARGUMENT;
    TBSYS_LOG(WARN, "mutator is null");
  }

  ObRowkey rowkey;

  ObObj value[2];
  value[0].set_int(table_id);
  value[1].set_int(column_id);
  rowkey.assign(value, 2);

  if (OB_SUCCESS == ret)
  {
    ADD_INT(auto_increment_table_name, rowkey, "max_value", OB_DEFAULT_AUTO_INCREMENT_VALUE);
  }
  return ret;
}

int ObSchemaServiceImpl::fetch_auto_column_id(const uint64_t table_id, uint64_t& auto_column_id)
{
  int ret = OB_SUCCESS;
  QueryRes* res = NULL;

  ObNewRange range;
  int32_t rowkey_column = 2;
  ObObj start_rowkey[rowkey_column];
  ObObj end_rowkey[rowkey_column];
  start_rowkey[0].set_int(table_id);
  start_rowkey[1].set_min_value();
  end_rowkey[0].set_int(table_id);
  end_rowkey[1].set_max_value();
  if (OB_SUCCESS == ret)
  {
    range.start_key_.assign(start_rowkey, rowkey_column);
    range.end_key_.assign(end_rowkey, rowkey_column);
  }
  if(OB_SUCCESS == ret)
  {
    ret = nb_accessor_.scan(res, OB_ALL_COLUMN_TABLE_NAME, range, SC("column_id")("auto_increment"), ScanConds("table_id", EQ, table_id));
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "scan column table fail:ret[%d]", ret);
    }
  }

  if(OB_SUCCESS == ret)
  {
    TableRow* table_row = NULL;
    while(OB_SUCCESS == res->next_row() && OB_SUCCESS == ret)
    {
      res->get_row(&table_row);
      if(NULL != table_row)
      {
        uint64_t column_id = OB_INVALID_ID;
        int64_t auto_increment = 0;
        ASSIGN_INT("column_id", column_id, uint64_t);
        ASSIGN_INT("auto_increment", auto_increment, int64_t);
        if(OB_SUCCESS != ret)
        {
          TBSYS_LOG(WARN, "get auto_column_id fail:ret[%d]", ret);
        }
        else if (auto_increment == 1)
        {
          auto_column_id = column_id;
          break;
        }
      }
      else
      {
        ret = OB_ERROR;
        TBSYS_LOG(WARN, "get column fail");
      }
    }
  }
  nb_accessor_.release_query_res(res);
  res = NULL;

  return ret;
}

int ObSchemaServiceImpl::delete_auto_increment(const int64_t table_id)
{
  int ret = OB_SUCCESS;
  uint64_t auto_column_id = OB_INVALID_ID;
  if (OB_SUCCESS != (ret = fetch_auto_column_id(table_id, auto_column_id)))
  {
    TBSYS_LOG(WARN, "fail to fetch auto_column_id, ret=%d", ret);
  }
  else
  {
    ObObj value[2];
    value[0].set_int(table_id);
    value[1].set_int(auto_column_id);
    ObRowkey rowkey;
    rowkey.assign(value, 2);
    ret = nb_accessor_.delete_row(OB_ALL_AUTO_INCREMENT_TABLE_NAME, rowkey);
    if(OB_SUCCESS != ret)
    {
      TBSYS_LOG(WARN, "delete row from auto increment table fail:ret[%d], rowkey=%s", ret, to_cstring(rowkey));
    }
  }
  return ret;
}

//add:e
