#include "ob_ups_procedure_special_executor.h"
#include "ob_ups_procedure.h"
#include "common/ob_common_stat.h"
#include "common/ob_row_fuse.h"
#include "ob_table_mgr.h"
#include "ob_ups_table_mgr.h"
#include "ob_update_server_main.h"

using namespace oceanbase::updateserver;
using namespace oceanbase::sql;

int ObUpsTpcc::execute_neworder(int w_id, int d_id, int c_id, int item_ids[], double i_prices[],
                           int supplier_w_ids[], int order_quantities[], int o_ol_cnt, int o_all_local)
{
  int ret = OB_SUCCESS;
  int ol_i_id = 0, d_next_o_id = 0, o_id = 0;
  int s_remote_cnt_increment = 0;
  double ol_amount = 0, total_amount = 0, i_price = 0, d_tax = 0;
  int o_quantity = 0, ol_supply_w_id = 0, s_quantity = 0;
  ObString i_name, i_data;
  ObString ol_dist_info, s_data;
  ObString s_dist_info[10];
  double ol_amounts[MAX_ITEM_COUNT];
  ObString ol_dist_infos[MAX_ITEM_COUNT];

  SelectStockParam select_stock_param;
  UpdateStockParam update_stock_param;
  SelectItemParam  select_item_param;

  UNUSED(i_prices);

  for(int itr = 0; itr < o_ol_cnt && OB_SUCCESS == ret; ++itr)
  {
    ol_i_id = item_ids[itr];
    ol_supply_w_id = supplier_w_ids[itr];
    o_quantity = order_quantities[itr];
//    i_price = i_prices[itr];

    //commit a select_item sql
    select_item_param.set_param(ol_i_id, &i_price, &i_name, &i_data);
    int64_t start_item_ts = tbsys::CTimeUtil::getTime();
    if( OB_SUCCESS != (ret = tpcc_select(select_item_param)))
    {
      TBSYS_LOG(TRACE, "failed to select item");
      break;
    }
    OB_STAT_INC(UPDATESERVER, UPS_PROC_LOOP, tbsys::CTimeUtil::getTime() - start_item_ts);

    //commit a select_stock sql
    select_stock_param.set_param(ol_i_id, ol_supply_w_id, &s_quantity, &s_data, s_dist_info);
    int64_t start_ts = tbsys::CTimeUtil::getTime();
    if( OB_SUCCESS != (ret = tpcc_select(select_stock_param)))
    {
      TBSYS_LOG(TRACE, "failed to select stock");
      break;
    }
    OB_STAT_INC(UPDATESERVER, UPS_PROC_E, tbsys::CTimeUtil::getTime() - start_ts);

    if( s_quantity - o_quantity >= 10 )
    {
      s_quantity -= o_quantity;
    }
    else
    {
      s_quantity = s_quantity - o_quantity + 91;
    }

    if( ol_supply_w_id == w_id )
    {
      s_remote_cnt_increment = 0;
    }
    else
    {
      s_remote_cnt_increment = 1;
    }

    update_stock_param.set_param(ol_i_id, ol_supply_w_id, s_quantity, (double)o_quantity, s_remote_cnt_increment);

    int64_t upd_stock_ts = tbsys::CTimeUtil::getTime();
    if( OB_SUCCESS != ret) {}
    else if( OB_SUCCESS != (ret = tpcc_update(update_stock_param)) )
    {
      TBSYS_LOG(TRACE, "failed to update stock");
      break;
    }
    OB_STAT_INC(UPDATESERVER, UPS_PROC_D, tbsys::CTimeUtil::getTime() - upd_stock_ts);

    ol_amount = o_quantity * i_price;
    total_amount = total_amount + ol_amount;

    ol_dist_info = s_dist_info[d_id - 1];

    ol_amounts[itr] = ol_amount;
    ol_dist_infos[itr] = ol_dist_info;

//    TBSYS_LOG(INFO, "ol_dist_infos[%d] %p, ol_dist_info %p, s_dist_info[%d] %p", itr, ol_dist_infos[itr].ptr(),
//              ol_dist_info.ptr(), d_id, s_dist_info[d_id].ptr());
  }

  int64_t district_start_ts = tbsys::CTimeUtil::getTime();
  SelectDistrictParam select_dist_param;
  UpdateDistrictParam update_dist_param;
  select_dist_param.set_param(w_id, d_id, &d_next_o_id, &d_tax);
  //commit a select district;
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = tpcc_select(select_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to select district");
  }
  else
  {
    TBSYS_LOG(TRACE, "d_next_o_id: %d, d_tax: %lf", d_next_o_id, d_tax);
  }

  //commit a update district;
  update_dist_param.set_param(w_id, d_id);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = tpcc_update(update_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to update district");
  }
  else
  {
    TBSYS_LOG(TRACE, "update district successfully");
  }

  o_id = d_next_o_id;

  //commit a replace oorder
  InsertOOrderParam insert_oorder_param;
  InsertNewOrderParam insert_neworder_param;
  InsertOrderLineParam insert_orderline_param;

  insert_oorder_param.set_param(o_id, d_id, w_id, c_id, tbsys::CTimeUtil::getTime(), o_ol_cnt, o_all_local);

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = insert_oorder(insert_oorder_param)))
  {
    TBSYS_LOG(TRACE, "failed to insert oorder");
  }
  else
  {
    TBSYS_LOG(TRACE, "insert oorder successfully");
  }

  //commit a replace new_order
  insert_neworder_param.set_param(o_id, d_id, w_id);

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = insert_neworder(insert_neworder_param)))
  {
    TBSYS_LOG(TRACE, "failed to insert neworder");
  }
  else
  {
    TBSYS_LOG(TRACE, "insert neworder successfully");
  }

  OB_STAT_INC(UPDATESERVER, UPS_PROC_IF, tbsys::CTimeUtil::getTime() - district_start_ts);

  int64_t orderline_start_ts = tbsys::CTimeUtil::getTime();
  for(int itr = 0; itr < o_ol_cnt && OB_SUCCESS == ret; ++itr)
  {
    //commit a replace order_line
    insert_orderline_param.set_param(o_id, d_id, w_id, itr+1, item_ids[itr],
                                     supplier_w_ids[itr],
                                     order_quantities[itr],
                                     ol_amounts[itr],
                                     ol_dist_infos[itr]);
    if( OB_SUCCESS != ret ) {}
    else if( OB_SUCCESS != (ret = insert_orderline(insert_orderline_param)))
    {
      TBSYS_LOG(TRACE, "failed to insert orderline");
      break;
    }
    else
    {
      TBSYS_LOG(TRACE, "insert orderline successfully");
    }
  }
  OB_STAT_INC(UPDATESERVER, UPS_PROC_DW, tbsys::CTimeUtil::getTime() - orderline_start_ts);
  return ret;
}



int ObUpsTpcc::execute_neworder_strict_order(int w_id, int d_id, int c_id, int item_ids[], double i_prices[],
                           int supplier_w_ids[], int order_quantities[], int o_ol_cnt, int o_all_local)
{
  int ret = OB_SUCCESS;
  int ol_i_id = 0, d_next_o_id = 0, o_id = 0;
  int s_remote_cnt_increment = 0;
  double ol_amount = 0, total_amount = 0, i_price = 0, d_tax = 0;
  int o_quantity = 0, ol_supply_w_id = 0, s_quantity = 0;
  ObString i_name, i_data;
  ObString ol_dist_info, s_data;
  ObString s_dist_info[10];

  UNUSED(i_prices);

  SelectDistrictParam select_dist_param;
  UpdateDistrictParam update_dist_param;
  SelectStockParam select_stock_param;
  UpdateStockParam update_stock_param;
  SelectItemParam  select_item_param;

  select_dist_param.set_param(w_id, d_id, &d_next_o_id, &d_tax);

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = tpcc_select(select_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to select district");
  }
  else
  {
    TBSYS_LOG(TRACE, "d_next_o_id: %d, d_tax: %lf", d_next_o_id, d_tax);
  }

  update_dist_param.set_param(w_id, d_id);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = tpcc_update(update_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to update district");
  }
  else
  {
    TBSYS_LOG(TRACE, "update district successfully");
  }

  o_id = d_next_o_id;

  //commit a replace oorder
  InsertOOrderParam insert_oorder_param;
  InsertNewOrderParam insert_neworder_param;
  InsertOrderLineParam insert_orderline_param;

  insert_oorder_param.set_param(o_id, d_id, w_id, c_id, tbsys::CTimeUtil::getTime(), o_ol_cnt, o_all_local);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = insert_oorder(insert_oorder_param)))
  {
    TBSYS_LOG(TRACE, "failed to insert oorder");
  }
  else
  {
    TBSYS_LOG(TRACE, "insert oorder successfully");
  }

  insert_neworder_param.set_param(o_id, d_id, w_id);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = insert_neworder(insert_neworder_param)))
  {
    TBSYS_LOG(TRACE, "failed to insert neworder");
  }
  else
  {
    TBSYS_LOG(TRACE, "insert neworder successfully");
  }

  for(int itr = 0; itr < o_ol_cnt && OB_SUCCESS == ret; ++itr)
  {
    ol_i_id = item_ids[itr];
    ol_supply_w_id = supplier_w_ids[itr];
    o_quantity = order_quantities[itr];

    //commit a select_item sql
    select_item_param.set_param(ol_i_id, &i_price, &i_name, &i_data);
    if( OB_SUCCESS != (ret = tpcc_select(select_item_param)))
    {
      TBSYS_LOG(TRACE, "failed to select item");
      break;
    }

    select_stock_param.set_param(ol_i_id, ol_supply_w_id, &s_quantity, &s_data, s_dist_info);
    if( OB_SUCCESS != (ret = tpcc_select(select_stock_param)))
    {
      TBSYS_LOG(TRACE, "failed to select stock");
      break;
    }

    if( s_quantity - o_quantity >= 10 )
    {
      s_quantity -= o_quantity;
    }
    else
    {
      s_quantity = s_quantity - o_quantity + 91;
    }

    if( ol_supply_w_id == w_id )
    {
      s_remote_cnt_increment = 0;
    }
    else
    {
      s_remote_cnt_increment = 1;
    }

    update_stock_param.set_param(ol_i_id, ol_supply_w_id, s_quantity, (double)o_quantity, s_remote_cnt_increment);

    if( OB_SUCCESS != ret) {}
    else if( OB_SUCCESS != (ret = tpcc_update(update_stock_param)) )
    {
      TBSYS_LOG(TRACE, "failed to update stock");
      break;
    }
    ol_amount = o_quantity * i_price;
    total_amount = total_amount + ol_amount;

    ol_dist_info = s_dist_info[d_id - 1];

    insert_orderline_param.set_param(o_id, d_id, w_id, itr+1, ol_i_id, ol_supply_w_id,
                                     o_quantity, ol_amount, ol_dist_info);

    if( OB_SUCCESS != ret ) {}
    else if( OB_SUCCESS != (ret = insert_orderline(insert_orderline_param)))
    {
      TBSYS_LOG(TRACE, "failed to insert orderline");
      break;
    }
    else
    {
      TBSYS_LOG(TRACE, "insert orderline successfully");
    }
  }
  return ret;
}


/**
 * @brief ObUpsTpcc::execute_payment_merge_sql
 *    It works when there is no static data, for static data, we need for complex way to handle that.
 * @param w_id
 * @param d_id
 * @param c_id
 * @param c_w_id
 * @param c_d_id
 * @param h_amount
 * @return
 */
int ObUpsTpcc::execute_payment_merge_sql(int w_id, int d_id, int c_id, int c_w_id, int c_d_id, double h_amount)
{
  int ret = OB_SUCCESS;
  ObString c_credit;
  double c_balance = 0, c_ytd_payment = 0;
  int c_payment_cnt = 0;
  ObString w_street_1, w_street_2, w_city, w_state, w_zip, w_name;
  ObString d_street_1, d_street_2, d_city, d_state, d_zip, d_name;

  PaymentUpdateSelectWareParam update_select_ware_param;
  PaymentUpdateSelectDistParam update_select_dist_param;

  PaymentSelectCustParam select_cust_param;

  update_select_ware_param.w_id_.set_int(w_id);
  update_select_ware_param.h_amount_.set_double(h_amount);
  update_select_ware_param.set_param(w_street_1, w_street_2, w_city, w_state, w_zip, w_name);

  update_select_dist_param.d_w_id_.set_int(w_id);
  update_select_dist_param.d_id_.set_int(d_id);
  update_select_dist_param.h_amount_.set_double(h_amount);
  update_select_dist_param.set_param(d_street_1, d_street_2, d_city, d_state, d_zip, d_name);

  select_cust_param.c_w_id_.set_int(c_w_id);
  select_cust_param.c_d_id_.set_int(c_d_id);
  select_cust_param.c_id_.set_int(c_id);
  select_cust_param.set_param(c_credit, c_balance, c_ytd_payment, c_payment_cnt);

  if( OB_SUCCESS != (ret = tpcc_select(select_cust_param)))
  {
    TBSYS_LOG(TRACE, "failed to select customer");
  }
  else
  {
    c_balance = c_balance - h_amount;
    c_ytd_payment = c_ytd_payment + h_amount;
    c_payment_cnt = c_payment_cnt + 1;
  }

  if( OB_SUCCESS != ret ) {}
  else if( 0 == c_credit.compare(ObString::make_string("bc")) )
  {
    PaymentSelectCustCDataParam select_cust_cdata_param;
    PaymentUpdateCustCDataParam update_cust_cdata_param;
    ObString c_data;

    select_cust_cdata_param.c_w_id_.set_int(c_w_id);
    select_cust_cdata_param.c_d_id_.set_int(c_d_id);
    select_cust_cdata_param.c_id_.set_int(c_id);
    select_cust_cdata_param.set_param(c_data);

    if( OB_SUCCESS != (ret = tpcc_select(select_cust_cdata_param)))
    {
      TBSYS_LOG(TRACE, "failed to select cdata");
    }

    ObString ncdata;
    char ncdata_buf[10];
    int ncdata_len = snprintf(ncdata_buf, 10, "%d %d %d %d %d %lf %.*s",
                              c_id, c_d_id, c_w_id, d_id, w_id, h_amount, c_data.length(), c_data.ptr());
    ncdata.assign_ptr(ncdata_buf, ncdata_len);
    //compute ncdata

    update_cust_cdata_param.c_balance_.set_double(c_balance);
    update_cust_cdata_param.c_ytd_payment_.set_double(c_ytd_payment);
    update_cust_cdata_param.c_payment_cnt_.set_int(c_payment_cnt);
    update_cust_cdata_param.c_data_.set_varchar(ncdata);

    update_cust_cdata_param.c_w_id_.set_int(c_w_id);
    update_cust_cdata_param.c_d_id_.set_int(c_d_id);
    update_cust_cdata_param.c_id_.set_int(c_id);

    if( OB_SUCCESS != ret ) {}
    else if( OB_SUCCESS != (ret = payment_insert_cust_2(update_cust_cdata_param)))
    {
      TBSYS_LOG(TRACE, "failed to update cust cdata");
    }
  }
  else
  {
    PaymentUpdateCustParam update_cust_param;

    update_cust_param.c_balance_.set_double(c_balance);
    update_cust_param.c_ytd_payment_.set_double(c_ytd_payment);
    update_cust_param.c_payment_cnt_.set_int(c_payment_cnt);

    update_cust_param.c_w_id_.set_int(c_w_id);
    update_cust_param.c_d_id_.set_int(c_d_id);
    update_cust_param.c_id_.set_int(c_id);
    if( OB_SUCCESS != (ret = payment_insert_cust_1(update_cust_param)))
    {
      TBSYS_LOG(TRACE, "failed to update cust");
    }
  }

  if( OB_SUCCESS != ret ) {}
  if( OB_SUCCESS != (ret = payment_update_select_dist(update_select_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to update and select district");
  }
  else if( OB_SUCCESS != (ret = payment_update_select_ware(update_select_ware_param)))
  {
    TBSYS_LOG(TRACE, "failed to update and select warehouse");
  }
  else
  {
    ObString h_data;
    char h_data_buf[20];
    int h_data_buf_len = snprintf(h_data_buf, 20, "%.*s%.*s",
                                  w_name.length() < 10 ? w_name.length() : 10, w_name.ptr(),
                                  d_name.length() < 10 ? d_name.length() : 10, d_name.ptr());
    h_data.assign_ptr(h_data_buf, h_data_buf_len);

    //compute h_data
    PaymentInsertHistParam insert_hist_param;
    insert_hist_param.h_c_w_id_.set_int(c_w_id);
    insert_hist_param.h_c_d_id_.set_int(c_d_id);
    insert_hist_param.h_c_id_.set_int(c_id);
    insert_hist_param.h_w_id_.set_int(w_id);
    insert_hist_param.h_d_id_.set_int(d_id);
    insert_hist_param.h_date_.set_int(tbsys::CTimeUtil::getTime());
    insert_hist_param.h_amount_.set_double(h_amount);
    insert_hist_param.h_data_.set_varchar(h_data);

    if( OB_SUCCESS != (ret = payment_insert_hist(insert_hist_param)))
    {
      TBSYS_LOG(TRACE, "failed to insert history");
    }
  }
  return ret;
}

int ObUpsTpcc::execute_payment(int w_id, int d_id, int c_id, int c_w_id, int c_d_id, double h_amount)
{
  int ret = OB_SUCCESS;
  ObString c_credit;
  double c_balance = 0, c_ytd_payment = 0;
  int c_payment_cnt = 0;
  ObString w_street_1, w_street_2, w_city, w_state, w_zip, w_name;
  ObString d_street_1, d_street_2, d_city, d_state, d_zip, d_name;

  PaymentUpdateWareParam update_ware_param;
  PaymentSelectWareParam select_ware_param;
  PaymentUpdateDistParam update_dist_param;
  PaymentSelectDistParam select_dist_param;
  PaymentSelectCustParam select_cust_param;

  update_ware_param.w_id_.set_int(w_id);
  update_ware_param.h_amount_.set_double(h_amount);

  select_ware_param.w_id_.set_int(w_id);
  select_ware_param.set_param(w_street_1, w_street_2, w_city, w_state, w_zip, w_name);

  update_dist_param.d_w_id_.set_int(w_id);
  update_dist_param.d_id_.set_int(d_id);
  update_dist_param.h_amount_.set_double(h_amount);

  select_dist_param.d_w_id_.set_int(w_id);
  select_dist_param.d_id_.set_int(d_id);
  select_dist_param.set_param(d_street_1, d_street_2, d_city, d_state, d_zip, d_name);

  select_cust_param.c_w_id_.set_int(c_w_id);
  select_cust_param.c_d_id_.set_int(c_d_id);
  select_cust_param.c_id_.set_int(c_id);
  select_cust_param.set_param(c_credit, c_balance, c_ytd_payment, c_payment_cnt);

  if( OB_SUCCESS != (ret = tpcc_select(select_cust_param)))
  {
    TBSYS_LOG(TRACE, "failed to select customer");
  }
  else
  {
    c_balance = c_balance - h_amount;
    c_ytd_payment = c_ytd_payment + h_amount;
    c_payment_cnt = c_payment_cnt + 1;
  }

  if( OB_SUCCESS != ret ) {}
  else if( 0 == c_credit.compare(ObString::make_string("bc")) )
  {
    PaymentSelectCustCDataParam select_cust_cdata_param;
    PaymentUpdateCustCDataParam update_cust_cdata_param;
    ObString c_data;

    select_cust_cdata_param.c_w_id_.set_int(c_w_id);
    select_cust_cdata_param.c_d_id_.set_int(c_d_id);
    select_cust_cdata_param.c_id_.set_int(c_id);
    select_cust_cdata_param.set_param(c_data);

    if( OB_SUCCESS != (ret = tpcc_select(select_cust_cdata_param)))
    {
      TBSYS_LOG(TRACE, "failed to select cdata");
    }

    ObString ncdata;
    char ncdata_buf[10];
    int ncdata_len = snprintf(ncdata_buf, 10, "%d %d %d %d %d %lf %.*s",
                              c_id, c_d_id, c_w_id, d_id, w_id, h_amount, c_data.length(), c_data.ptr());
    ncdata.assign_ptr(ncdata_buf, ncdata_len);
    //compute ncdata

    update_cust_cdata_param.c_balance_.set_double(c_balance);
    update_cust_cdata_param.c_ytd_payment_.set_double(c_ytd_payment);
    update_cust_cdata_param.c_payment_cnt_.set_int(c_payment_cnt);
    update_cust_cdata_param.c_data_.set_varchar(ncdata);

    update_cust_cdata_param.c_w_id_.set_int(c_w_id);
    update_cust_cdata_param.c_d_id_.set_int(c_d_id);
    update_cust_cdata_param.c_id_.set_int(c_id);

    if( OB_SUCCESS != ret ) {}
    else if( OB_SUCCESS != (ret = tpcc_update(update_cust_cdata_param)))
    {
      TBSYS_LOG(TRACE, "failed to update cust cdata");
    }
  }
  else
  {
    PaymentUpdateCustParam update_cust_param;

    update_cust_param.c_balance_.set_double(c_balance);
    update_cust_param.c_ytd_payment_.set_double(c_ytd_payment);
    update_cust_param.c_payment_cnt_.set_int(c_payment_cnt);

    update_cust_param.c_w_id_.set_int(c_w_id);
    update_cust_param.c_d_id_.set_int(c_d_id);
    update_cust_param.c_id_.set_int(c_id);
    if( OB_SUCCESS != (ret = tpcc_update(update_cust_param)))
    {
      TBSYS_LOG(TRACE, "failed to update cust");
    }
  }

  if( OB_SUCCESS != ret ) {}
  if( OB_SUCCESS != (ret = tpcc_update(update_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to update district");
  }
  else if( OB_SUCCESS != (ret = tpcc_select(select_dist_param)))
  {
    TBSYS_LOG(TRACE, "failed to select district");
  }
  else if( OB_SUCCESS != (ret = tpcc_update(update_ware_param)) )
  {
    TBSYS_LOG(TRACE, "failed to update warehouse");
  }
  else if( OB_SUCCESS != (ret = tpcc_select(select_ware_param)))
  {
    TBSYS_LOG(TRACE, "failed to select warehouse");
  }
  else
  {
    ObString h_data;
    char h_data_buf[20];
    int h_data_buf_len = snprintf(h_data_buf, 20, "%.*s%.*s",
                                  w_name.length() < 10 ? w_name.length() : 10, w_name.ptr(),
                                  d_name.length() < 10 ? d_name.length() : 10, d_name.ptr());
    h_data.assign_ptr(h_data_buf, h_data_buf_len);

    //compute h_data
    PaymentInsertHistParam insert_hist_param;
    insert_hist_param.h_c_w_id_.set_int(c_w_id);
    insert_hist_param.h_c_d_id_.set_int(c_d_id);
    insert_hist_param.h_c_id_.set_int(c_id);
    insert_hist_param.h_w_id_.set_int(w_id);
    insert_hist_param.h_d_id_.set_int(d_id);
    insert_hist_param.h_date_.set_int(tbsys::CTimeUtil::getTime());
    insert_hist_param.h_amount_.set_double(h_amount);
    insert_hist_param.h_data_.set_varchar(h_data);

    if( OB_SUCCESS != (ret = payment_insert_hist(insert_hist_param)))
    {
      TBSYS_LOG(TRACE, "failed to insert history");
    }
  }
  return ret;
}

int ObUpsTpcc::prepare(BasicParam &param)
{
  int ret = OB_SUCCESS;
  ObUpdateServerMain *ups_main = ObUpdateServerMain::get_instance();
  bool is_final_mirror = false;
  uint64_t max_version = 0;
  table_mgr_ = ups_main? &ups_main->get_update_server().get_table_mgr() : NULL;

  if ( NULL == table_mgr_ )
  {
    TBSYS_LOG(ERROR, "table_mgr_ is NULL");
  }


  guard_ = new(guard_buf_)ITableEntity::Guard(table_mgr_->get_table_mgr()->get_resource_pool());

  table_mgr_->get_table_mgr()->acquire_table(param.version_range_,
                                             max_version /*max_valid_version*/,
                                             table_list_,
                                             is_final_mirror,
                                             param.table_id_);

  return ret;
}

int ObUpsTpcc::close()
{
  if( NULL != guard_ )
  {
    guard_->~Guard();
    guard_ = NULL;
  }
  return OB_SUCCESS;
}

int ObUpsTpcc::insert_oorder(InsertOOrderParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();
  result_row.set_row_desc(param.row_desc_);

  result_row.set_cell(param.table_id_, 16, param.o_w_id_);
  result_row.set_cell(param.table_id_, 17, param.o_d_id_);
  result_row.set_cell(param.table_id_, 18, param.o_id_);
  result_row.set_cell(param.table_id_, 19, param.o_c_id_);
  result_row.set_cell(param.table_id_, 23, param.o_entry_d_);
  result_row.set_cell(param.table_id_, 21, param.o_ol_cnt_);
  result_row.set_cell(param.table_id_, 22, param.o_all_local_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.row_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::insert_neworder(InsertNewOrderParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();
  result_row.set_row_desc(param.row_desc_);

  result_row.set_cell(param.table_id_, 16, param.no_w_id_);
  result_row.set_cell(param.table_id_, 17, param.no_d_id_);
  result_row.set_cell(param.table_id_, 18, param.no_o_id_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.row_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::insert_orderline(InsertOrderLineParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();
  result_row.set_row_desc(param.row_desc_);

  result_row.set_cell(param.table_id_, 16, param.ol_w_id_);
  result_row.set_cell(param.table_id_, 17, param.ol_d_id_);
  result_row.set_cell(param.table_id_, 18, param.ol_o_id_);
  result_row.set_cell(param.table_id_, 19, param.ol_number_);
  result_row.set_cell(param.table_id_, 20, param.ol_i_id_);
  result_row.set_cell(param.table_id_, 23, param.ol_supply_w_id_);
  result_row.set_cell(param.table_id_, 24, param.ol_quantity_);
  result_row.set_cell(param.table_id_, 22, param.ol_amount_);
  result_row.set_cell(param.table_id_, 25, param.ol_dist_info_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.row_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::tpcc_update(BasicParam &param)
{
  int ret = OB_SUCCESS;
  prepare(param);

  BaseSessionCtx *session_ctx = proc_->get_session_ctx();
  ITableEntity *table_entity;

  common::ObRow get_row;
  const common::ObRowkey *get_row_key;
  const common::ObRow *inc_row;

  ColumnFilter *get_column_filter = ITableEntity::get_tsi_columnfilter();

  get_column_filter->clear();
  for(int64_t i = 0; i < param.column_count_; ++i)
  {
    get_column_filter->add_column(param.column_ids_[i]);
  }

  get_row.set_row_desc(param.row_desc_);

  for(int64_t i = 0; i < param.row_desc_.get_rowkey_cell_count(); ++i)
  {
    get_row.set_cell(param.table_id_, param.key_column_ids_[i], param.keys_[i]);
  }

  get_row.get_rowkey(get_row_key);

  OB_ASSERT(table_list_.size() == 1);
  table_entity = *(table_list_.begin());

  ITableIterator *table_itr = NULL;
  ObRowIterAdaptor *table_row_iter = NULL;

  table_itr = table_entity->alloc_iterator(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);
  table_row_iter = table_entity->alloc_row_iter_adaptor(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);

  if( OB_SUCCESS != (ret = table_entity->get(*session_ctx,
                                             param.table_id_,
                                             *get_row_key,
                                             get_column_filter,
                                             param.lock_flag_,
                                             table_itr)) )
  {
    TBSYS_LOG(TRACE, "failed to read index");
  }
  else
  {
    table_row_iter->set_cell_iter(table_itr, param.row_desc_, false);
  }

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = table_row_iter->open()))
  {
    TBSYS_LOG(WARN, "failed to open iter");
  }
  else if( OB_SUCCESS != (ret = table_row_iter->get_next_row(inc_row)))
  {
    TBSYS_LOG(WARN, "failed to get next row");
  }
  else
  {
    TBSYS_LOG(TRACE, "inc_row: %s", to_cstring(*inc_row));
  }

//  ObRowStore *row_store = NULL;
//  proc_->get_static_data_by_id(param.static_data_id_, row_store);
//  common::ObRow static_row;
//  static_row.set_row_desc(param.row_desc_);

//  if( OB_SUCCESS != ret ) {}
//  else if (OB_SUCCESS != (ret = row_store->get_next_row(static_row)))
//  {
//    TBSYS_LOG(WARN, "failed to get static row from store");
//  }
//  else
//  {
//    TBSYS_LOG(TRACE, "static row: %s", to_cstring(static_row));
//  }

  bool is_row_empty = false;
  common::ObRow result_row;
  result_row.set_row_desc(param.row_desc_);
  result_row.reset(false, ObRow::DEFAULT_NOP);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            *inc_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse inc row");
  }
//  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
//                            static_row, result_row, is_row_empty, true)))
//  {
//    TBSYS_LOG(WARN, "failed to fuse static row");
//  }
  else
  {
    TBSYS_LOG(TRACE, "merge row: %s", to_cstring(result_row));

    //project and generate new input
    param.project_row(result_row, &str_buf_);

    //    const ObObj *cell;

    //    result_row.get_cell(param.table_id_, 17, cell);
    //    add_double(param.h_amount_, *cell);
    //    result_row.set_cell(param.table_id_, 17, param.h_amount_);

    //    result_row.set_row_desc(param.write_desc_);
  }

  //write back the result_row;
  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*session_ctx),result_row, param.write_desc_)) )
  {
    TBSYS_LOG(TRACE, "failed to write row");
  }
  close();
  return ret;
}

int ObUpsTpcc::tpcc_select(BasicParam &param)
{
  int ret = OB_SUCCESS;
  prepare(param);

  BaseSessionCtx *session_ctx = proc_->get_session_ctx();
  ITableEntity *table_entity;

  common::ObRow get_row;
  const common::ObRowkey *get_row_key;
  const common::ObRow *inc_row;

  ColumnFilter *get_column_filter = ITableEntity::get_tsi_columnfilter();

  get_column_filter->clear();
  for(int64_t i = 0; i < param.column_count_; ++i)
  {
    get_column_filter->add_column(param.column_ids_[i]);
  }

  get_row.set_row_desc(param.row_desc_);

  for(int64_t i = 0; i < param.row_desc_.get_rowkey_cell_count(); ++i)
  {
    get_row.set_cell(param.table_id_, param.key_column_ids_[i], param.keys_[i]);
  }

  get_row.get_rowkey(get_row_key);

  OB_ASSERT(table_list_.size() == 1);
  table_entity = *(table_list_.begin());

  ITableIterator *table_itr = NULL;
  ObRowIterAdaptor *table_row_iter = NULL;

  table_itr = table_entity->alloc_iterator(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);
  table_row_iter = table_entity->alloc_row_iter_adaptor(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);

  if( OB_SUCCESS != (ret = table_entity->get(*session_ctx,
                                             param.table_id_,
                                             *get_row_key,
                                             get_column_filter,
                                             param.lock_flag_,
                                             table_itr)) )
  {
    TBSYS_LOG(TRACE, "failed to read index");
  }
  else
  {
    table_row_iter->set_cell_iter(table_itr, param.row_desc_, false);
  }

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = table_row_iter->open()))
  {
    TBSYS_LOG(WARN, "failed to open iter");
  }
  else if( OB_SUCCESS != (ret = table_row_iter->get_next_row(inc_row)))
  {
    TBSYS_LOG(WARN, "failed to get next row");
  }
  else
  {
    TBSYS_LOG(TRACE, "inc_row: %s", to_cstring(*inc_row));
  }

//  ObRowStore *row_store = NULL;
//  proc_->get_static_data_by_id(param.static_data_id_, row_store);
//  common::ObRow static_row;
//  static_row.set_row_desc(param.row_desc_);

//  if( OB_SUCCESS != ret ) {}
//  else if (OB_SUCCESS != (ret = row_store->get_next_row(static_row)))
//  {
//    TBSYS_LOG(WARN, "failed to get static row from store");
//  }
//  else
//  {
//    TBSYS_LOG(TRACE, "static row: %s", to_cstring(static_row));
//  }

  bool is_row_empty = false;
  common::ObRow result_row;
  result_row.set_row_desc(param.row_desc_);
  result_row.reset(false, ObRow::DEFAULT_NOP);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            *inc_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse inc row");
  }
//  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
//                            static_row, result_row, is_row_empty, true)))
//  {
//    TBSYS_LOG(WARN, "failed to fuse static row");
//  }
  else
  {
    TBSYS_LOG(TRACE, "merge row: %s", to_cstring(result_row));
  }

  param.project_row(result_row, &str_buf_);

  close();
  return ret;
}

int ObUpsTpcc::payment_insert_hist(PaymentInsertHistParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();
  result_row.set_row_desc(param.row_desc_);

  result_row.set_cell(param.table_id_, 16, param.h_c_id_);
  result_row.set_cell(param.table_id_, 17, param.h_c_d_id_);
  result_row.set_cell(param.table_id_, 18, param.h_c_w_id_);
  result_row.set_cell(param.table_id_, 19, param.h_d_id_);
  result_row.set_cell(param.table_id_, 20, param.h_w_id_);
  result_row.set_cell(param.table_id_, 21, param.h_date_);
  result_row.set_cell(param.table_id_, 22, param.h_amount_);
  result_row.set_cell(param.table_id_, 23, param.h_data_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.row_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::payment_insert_cust_1(PaymentUpdateCustParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();

  result_row.set_row_desc(param.write_desc_);
  result_row.set_cell(param.table_id_, 16, param.c_w_id_);
  result_row.set_cell(param.table_id_, 17, param.c_d_id_);
  result_row.set_cell(param.table_id_, 18, param.c_id_);
  result_row.set_cell(param.table_id_, 24, param.c_balance_);
  result_row.set_cell(param.table_id_, 25, param.c_ytd_payment_);
  result_row.set_cell(param.table_id_, 26, param.c_payment_cnt_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.write_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::payment_insert_cust_2(PaymentUpdateCustCDataParam &param)
{
  int ret = OB_SUCCESS;
  common::ObRow result_row;
  BaseSessionCtx *ctx = proc_->get_session_ctx();

  result_row.set_row_desc(param.write_desc_);
  result_row.set_cell(param.table_id_, 16, param.c_w_id_);
  result_row.set_cell(param.table_id_, 17, param.c_d_id_);
  result_row.set_cell(param.table_id_, 18, param.c_id_);
  result_row.set_cell(param.table_id_, 24, param.c_balance_);
  result_row.set_cell(param.table_id_, 25, param.c_ytd_payment_);
  result_row.set_cell(param.table_id_, 26, param.c_payment_cnt_);
  result_row.set_cell(param.table_id_, 36, param.c_data_);

  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.write_desc_)))
  {
    TBSYS_LOG(WARN, "failed to write row");
  }
  return ret;
}

int ObUpsTpcc::payment_update_select_dist(PaymentUpdateSelectDistParam &param)
{
  int ret = OB_SUCCESS;
  if( OB_SUCCESS != (ret = tpcc_select(param)) )
  {
    TBSYS_LOG(WARN, "failed to select warehouse");
  }
  else
  {
    common::ObRow result_row;
    BaseSessionCtx *ctx = proc_->get_session_ctx();
    result_row.set_row_desc(param.write_desc_);

    result_row.set_cell(param.table_id_, 16, param.d_w_id_);
    result_row.set_cell(param.table_id_, 17, param.d_id_);
    result_row.set_cell(param.table_id_, 18, param.h_amount_);
    TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
    if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.write_desc_)))
    {
      TBSYS_LOG(WARN, "failed to write row");
    }
  }
  return ret;
}

int ObUpsTpcc::payment_update_select_ware(PaymentUpdateSelectWareParam &param)
{
  int ret = OB_SUCCESS;

  if( OB_SUCCESS != (ret = tpcc_select(param)) )
  {
    TBSYS_LOG(WARN, "failed to select warehouse");
  }
  else
  {
    common::ObRow result_row;
    BaseSessionCtx *ctx = proc_->get_session_ctx();
    result_row.set_row_desc(param.write_desc_);

    result_row.set_cell(param.table_id_, 16, param.w_id_);
    result_row.set_cell(param.table_id_, 17, param.h_amount_);
    TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
    if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*ctx), result_row, param.write_desc_)))
    {
      TBSYS_LOG(WARN, "failed to write row");
    }
  }
  return ret;
}

int ObUpsTpcc::write_row(RWSessionCtx &ctx, const ObRow &row, const ObRowDesc &row_desc)
{
  int ret = OB_SUCCESS;
  UpsSchemaMgrGuard sm_guard;
  const ObSchemaManagerV2 *schem_mgr = table_mgr_->get_schema_mgr().get_schema_mgr(sm_guard);

  cia_.reset();
  cia_.get_och().reset(row_desc, *schem_mgr);
  cia_.set_row(&row, row_desc.get_rowkey_cell_count());

  if( OB_SUCCESS != (ret = table_mgr_->apply(
                       ctx,
                       cia_,
                       OB_DML_UPDATE)) )
  {
    TBSYS_LOG(WARN, "failed to write memtable");
  }
  return ret;
}

int ObUpsTpcc::db_update(BasicParam &param)
{
  int ret = OB_SUCCESS;
  prepare(param);

  BaseSessionCtx *session_ctx = proc_->get_session_ctx();
  ITableEntity *table_entity;

  common::ObRow get_row;
  const common::ObRowkey *get_row_key;
  const common::ObRow *inc_row;

  ColumnFilter *get_column_filter = ITableEntity::get_tsi_columnfilter();

  get_column_filter->clear();
  for(int64_t i = 0; i < param.column_count_; ++i)
  {
    get_column_filter->add_column(param.column_ids_[i]);
  }

  get_row.set_row_desc(param.row_desc_);

  for(int64_t i = 0; i < param.row_desc_.get_rowkey_cell_count(); ++i)
  {
    get_row.set_cell(param.table_id_, param.key_column_ids_[i], param.keys_[i]);
  }

  get_row.get_rowkey(get_row_key);

//  OB_ASSERT(table_list_.size() == 1);
  table_entity = *(table_list_.begin());

  ITableIterator *table_itr = NULL;
  ObRowIterAdaptor *table_row_iter = NULL;

  table_itr = table_entity->alloc_iterator(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);
  table_row_iter = table_entity->alloc_row_iter_adaptor(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);

  if( OB_SUCCESS != (ret = table_entity->get(*session_ctx,
                                             param.table_id_,
                                             *get_row_key,
                                             get_column_filter,
                                             param.lock_flag_,
                                             table_itr)) )
  {
    TBSYS_LOG(TRACE, "failed to read index");
  }
  else
  {
    table_row_iter->set_cell_iter(table_itr, param.row_desc_, false);
  }

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = table_row_iter->open()))
  {
    TBSYS_LOG(WARN, "failed to open iter");
  }
  else if( OB_SUCCESS != (ret = table_row_iter->get_next_row(inc_row)))
  {
    TBSYS_LOG(WARN, "failed to get next row");
  }
  else
  {
    TBSYS_LOG(TRACE, "inc_row: %s", to_cstring(*inc_row));
  }

  ObRowStore *row_store = NULL;
  proc_->get_static_data_by_id(param.static_data_id_, row_store);
  common::ObRow static_row;
  static_row.set_row_desc(param.row_desc_);

  if( OB_SUCCESS != ret ) {}
  else if (OB_SUCCESS != (ret = row_store->get_next_row(static_row)))
  {
    TBSYS_LOG(WARN, "failed to get static row from store");
  }
  else
  {
    TBSYS_LOG(TRACE, "static row: %s", to_cstring(static_row));
  }

  bool is_row_empty = false;
  common::ObRow result_row;
  result_row.set_row_desc(param.row_desc_);
  result_row.reset(false, ObRow::DEFAULT_NOP);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            *inc_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse inc row");
  }
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            static_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse static row");
  }
  else
  {
    TBSYS_LOG(TRACE, "merge row: %s", to_cstring(result_row));

    //project and generate new input
    param.project_row(result_row, &str_buf_);
  }

  //write back the result_row;
  TBSYS_LOG(TRACE, "result_row: %s", to_cstring(result_row));
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = write_row(dynamic_cast<RWSessionCtx&>(*session_ctx),result_row, param.write_desc_)) )
  {
    TBSYS_LOG(TRACE, "failed to write row");
  }
  close();
  return ret;
}

int ObUpsTpcc::db_select(BasicParam &param)
{
  int ret = OB_SUCCESS;
  prepare(param);

  BaseSessionCtx *session_ctx = proc_->get_session_ctx();
  ITableEntity *table_entity;

  common::ObRow get_row;
  const common::ObRowkey *get_row_key;
  const common::ObRow *inc_row;

  ColumnFilter *get_column_filter = ITableEntity::get_tsi_columnfilter();

  get_column_filter->clear();
  for(int64_t i = 0; i < param.column_count_; ++i)
  {
    get_column_filter->add_column(param.column_ids_[i]);
  }

  get_row.set_row_desc(param.row_desc_);

  for(int64_t i = 0; i < param.row_desc_.get_rowkey_cell_count(); ++i)
  {
    get_row.set_cell(param.table_id_, param.key_column_ids_[i], param.keys_[i]);
  }

  get_row.get_rowkey(get_row_key);

//  OB_ASSERT(table_list_.size() == 1);
  table_entity = *(table_list_.begin());

  ITableIterator *table_itr = NULL;
  ObRowIterAdaptor *table_row_iter = NULL;

  table_itr = table_entity->alloc_iterator(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);
  table_row_iter = table_entity->alloc_row_iter_adaptor(table_mgr_->get_table_mgr()->get_resource_pool(), *guard_);

  if( OB_SUCCESS != (ret = table_entity->get(*session_ctx,
                                             param.table_id_,
                                             *get_row_key,
                                             get_column_filter,
                                             param.lock_flag_,
                                             table_itr)) )
  {
    TBSYS_LOG(TRACE, "failed to read index");
  }
  else
  {
    table_row_iter->set_cell_iter(table_itr, param.row_desc_, false);
  }

  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = table_row_iter->open()))
  {
    TBSYS_LOG(WARN, "failed to open iter");
  }
  else if( OB_SUCCESS != (ret = table_row_iter->get_next_row(inc_row)))
  {
    TBSYS_LOG(WARN, "failed to get next row");
  }
  else
  {
    TBSYS_LOG(TRACE, "inc_row: %s", to_cstring(*inc_row));
  }

  ObRowStore *row_store = NULL;
  proc_->get_static_data_by_id(param.static_data_id_, row_store);
  common::ObRow static_row;
  static_row.set_row_desc(param.row_desc_);

  if( OB_SUCCESS != ret ) {}
  else if (OB_SUCCESS != (ret = row_store->get_next_row(static_row)))
  {
    TBSYS_LOG(WARN, "failed to get static row from store");
  }
  else
  {
    TBSYS_LOG(TRACE, "static row: %s", to_cstring(static_row));
  }

  bool is_row_empty = false;
  common::ObRow result_row;
  result_row.set_row_desc(param.row_desc_);
  result_row.reset(false, ObRow::DEFAULT_NOP);
  if( OB_SUCCESS != ret ) {}
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            *inc_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse inc row");
  }
  else if( OB_SUCCESS != (ret = common::ObRowFuse::fuse_row(
                            static_row, result_row, is_row_empty, true)))
  {
    TBSYS_LOG(WARN, "failed to fuse static row");
  }
  else
  {
    TBSYS_LOG(TRACE, "merge row: %s", to_cstring(result_row));
  }

  param.project_row(result_row, &str_buf_);

  close();
  return ret;
}


int ObUpsTpcc::execute_update(int a_id, int b_id, int c_id, int d_id, int e_id, int value)
{
  int ret = OB_SUCCESS;

  DepParam param_a, param_b, param_c, param_d, param_e;

  param_a.c_id_.set_int(a_id);
  param_b.c_id_.set_int(b_id);
  param_c.c_id_.set_int(c_id);
  param_d.c_id_.set_int(d_id);
  param_e.c_id_.set_int(e_id);

  param_a.c_value_.set_int(value);
  param_b.c_value_.set_int(value);
  param_c.c_value_.set_int(value);
  param_d.c_value_.set_int(value);
  param_e.c_value_.set_int(value);

  param_a.static_data_id_ = 6;
  param_b.static_data_id_ = 7;
  param_c.static_data_id_ = 8;
  param_d.static_data_id_ = 9;
  param_e.static_data_id_ = 10;

  if( OB_SUCCESS != (ret = db_update(param_a)))
  {
    TBSYS_LOG(TRACE, "failed to update a node");
  }
  else if( OB_SUCCESS != (ret = db_update(param_b)))
  {
    TBSYS_LOG(TRACE, "failed to update b node");
  }
  else if( OB_SUCCESS != (ret = db_update(param_c)))
  {
    TBSYS_LOG(TRACE, "failed to update c node");
  }
  else if( OB_SUCCESS != (ret = db_update(param_d)))
  {
    TBSYS_LOG(TRACE, "failed to update d node");
  }
  else if( OB_SUCCESS != (ret = db_update(param_e)))
  {
    TBSYS_LOG(TRACE, "failed to udpate e node");
  }
  return ret;
}

void assign_bal(double &base_bal, double bal)
{
  base_bal = bal;
}

void add_bal(double &base_bal, double bal)
{
  base_bal += bal;
}

int ObUpsTpcc::execute_amalgamate(int acctId0, int acctId1)
{
  int ret = OB_SUCCESS;
  double bal0, bal1, total;


  SelectSavingParam select_saving_param(1);   //select savings of acctId0
  SelectCheckParam select_check_param(2);     //select checkings of acctId1

  select_saving_param.set_param(acctId0, bal0);
  select_check_param.set_param(acctId1, bal1);

  if( OB_SUCCESS != (ret = db_select(select_saving_param)) )
  {
    TBSYS_LOG(WARN, "amalgamte, select saving failed");
  }
  else if( OB_SUCCESS != (ret = db_select(select_check_param)) )
  {
    TBSYS_LOG(WARN, "amalgamate, select checking failed");
  }
  else
  {
    total = bal0 + bal1;

    UpdateCheckParam upd_check_param(3);
    upd_check_param.set_param(acctId0, 0, assign_bal);

    UpdateSavingParam upd_saving_param(4);
    upd_saving_param.set_param(acctId1, -total, add_bal);
    if( OB_SUCCESS != (ret = db_update(upd_check_param)))
    {
      TBSYS_LOG(WARN, "amalgamate, update check failed");
    }
    else if( OB_SUCCESS != (ret = db_update(upd_saving_param)))
    {
      TBSYS_LOG(WARN, "amalgamate, update saving failed");
    }
  }
  return ret;
}

int ObUpsTpcc::execute_write_check(int acctId, double amount)
{
  int ret = OB_SUCCESS;
  SelectSavingParam select_saving_param(1);
  SelectCheckParam select_check_param(2);
  UpdateCheckParam upd_check_param1(3);
  UpdateCheckParam upd_check_param2(4);

  double bal0, bal1, total;
  select_saving_param.set_param(acctId, bal0);
  select_check_param.set_param(acctId, bal1);

  if( OB_SUCCESS != (ret = db_select(select_saving_param)) )
  {
    TBSYS_LOG(WARN, "write_check, failed to select saving");
  }
  else if( OB_SUCCESS != (ret =  db_select(select_check_param)) )
  {
    TBSYS_LOG(WARN, "write_check, failed to select check");
  }
  else
  {

    total = bal0 + bal1;

    if( total > amount )
    {
      upd_check_param1.set_param(acctId, -amount, add_bal);
      if( OB_SUCCESS != (ret = db_update(upd_check_param1)) )
      {
        TBSYS_LOG(WARN, "write_check, failed to update check");
      }
    }
    else
    {
      upd_check_param2.set_param(acctId, 1 - total, add_bal);
      if( OB_SUCCESS != (ret = db_update(upd_check_param2)) )
      {
        TBSYS_LOG(WARN, "write_check, failed to update check");
      }
    }
  }
  return ret;
}

int ObUpsTpcc::execute_send_payment(int sendAcct, int destAcct, double amount)
{
  int ret = OB_SUCCESS;
  SelectCheckParam select_check_param(1);

  double bal = 0;
  select_check_param.set_param(sendAcct, bal);

  if( OB_SUCCESS != (ret = db_select(select_check_param)) )
  {
    TBSYS_LOG(WARN, "send_payment, failed to select check");
  }
  else
  {
    if (bal > amount) {

      UpdateCheckParam check1(2);
      UpdateCheckParam check2(3);

      check1.set_param(sendAcct, -amount, add_bal);
      check2.set_param(destAcct, amount, add_bal);

      if( OB_SUCCESS != (ret = db_update(check1)) )
      {
        TBSYS_LOG(WARN, "send_payemnt, failed to update check");
      }
      else if( OB_SUCCESS != (ret = db_update(check2)))
      {
        TBSYS_LOG(WARN, "send_payment, failed to update check");
      }
    }
  }
  return ret;
}

int ObUpsTpcc::execute_transact_savings(int acctId, double amount)
{
  int ret = OB_SUCCESS;
  SelectSavingParam select_saving_param(1);

  double bal = 0;
  select_saving_param.set_param(acctId, bal);

  if( OB_SUCCESS != (ret = db_select(select_saving_param)))
  {
    TBSYS_LOG(WARN, "transact_savings, failed to select saving");
  }
  else if (bal > amount) {
    UpdateSavingParam upd_saving(2);

    upd_saving.set_param(acctId, -amount, add_bal);
    if( OB_SUCCESS != (ret = db_update(upd_saving)) )
    {
      TBSYS_LOG(WARN, "failed to update saving");
    }
  }
  return ret;
}

ObUpsProcedureSpecialExecutor::ObUpsProcedureSpecialExecutor()
{
}
