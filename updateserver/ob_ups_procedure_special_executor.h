#ifndef OB_UPS_PROCEDURE_SPECIAL_EXECUTOR_H
#define OB_UPS_PROCEDURE_SPECIAL_EXECUTOR_H
#include "sql/ob_sp_procedure.h"
#include "ob_session_mgr.h"
#include "ob_table_mgr.h"
#include "ob_ups_table_mgr.h"
using namespace oceanbase::sql;
namespace oceanbase
{
  namespace updateserver
  {
    class ObUpsProcedure;

    inline int add_int(ObObj &dest, const ObObj &sour)
    {
      int64_t a = 0, b = 0;
      dest.get_int(a);
      sour.get_int(b);
      dest.set_int(a + b);
      return OB_SUCCESS;
    }

    inline int add_double(ObObj &dest, const ObObj &sour)
    {
      double a = 0, b = 0;
      dest.get_double(a);
      sour.get_double(b);
      dest.set_double(a + b);
      return OB_SUCCESS;
    }

    struct BasicParam
    {
        BasicParam(uint64_t table_id, int64_t column_count, ObLockFlag flag, int64_t static_data_id) :
          table_id_(table_id), column_count_(column_count), lock_flag_(flag), static_data_id_(static_data_id)
        {
          row_desc_.reset();

          version_range_.border_flag_.set_inclusive_start();
          version_range_.border_flag_.set_max_value();
          version_range_.start_version_.major_ = version_major;
          version_range_.start_version_.minor_ = version_minor;
          version_range_.start_version_.is_final_minor_ = is_final_minor;
        }

        //query structure
        ObObj keys_[4];

        uint64_t table_id_;
        uint64_t key_column_ids_[4];
        uint64_t column_ids_[16];
        int64_t  column_count_;

        ObLockFlag lock_flag_;
        ObVersionRange version_range_;
        ObRowDesc row_desc_;
        ObRowDesc write_desc_;
        int64_t static_data_id_;

        static const int32_t version_major = 2;
        static const int16_t version_minor = 0;
        static const int16_t is_final_minor = 0;

        virtual void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(row);
          UNUSED(str_buf);
        }
    };

    struct SelectItemParam : BasicParam
    {
        ObObj &o_i_id_;

        double *i_price_;
        ObString *i_name_;
        ObString *i_data_;

        SelectItemParam() : BasicParam(3004, 4, LF_NONE, 0), o_i_id_(keys_[0])
        {
          uint64_t temp_ids[] = {16, 18, 17, 19};
          for(int64_t i = 0; i < column_count_; ++i)
            column_ids_[i] = temp_ids[i];

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(table_id_, 16);
          row_desc_.add_column_desc(table_id_, 18);
          row_desc_.add_column_desc(table_id_, 17);
          row_desc_.add_column_desc(table_id_, 19);

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
        }

        void set_param(int o_i_id,
                       double *i_price, ObString *i_name, ObString *i_data)
        {
          o_i_id_.set_int(o_i_id);
          i_price_ = i_price;
          i_name_ = i_name;
          i_data_ = i_data;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          row.get_cell(table_id_, 18, cell);
          cell->get_double(*i_price_);

          row.get_cell(table_id_, 17, cell);
          cell->get_varchar(*i_name_);
          ob_write_string(*str_buf, *i_name_, *i_name_);

          row.get_cell(table_id_, 19, cell);
          cell->get_varchar(*i_data_);
          ob_write_string(*str_buf, *i_data_, *i_data_);
        }
    };

    struct SelectStockParam : BasicParam
    {
        //runtime parameters
        ObObj& ol_supply_w_id_;
        ObObj& o_i_id_;

        int *s_quantity_;
        ObString *s_data_;
        ObString *s_dist_;
        SelectStockParam() : BasicParam(3008, 14, LF_WRITE, 1), ol_supply_w_id_(keys_[0]), o_i_id_(keys_[1])
        {
          uint64_t temp_ids[] = {16, 17, 18, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

          for(int64_t i = 0; i < column_count_; ++i)
            column_ids_[i] = temp_ids[i];

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          row_desc_.add_column_desc(3008, 16);
          row_desc_.add_column_desc(3008, 17);
          row_desc_.add_column_desc(3008, 18);

          for(uint64_t i = 22; i <= 32; ++i)
          {
            row_desc_.add_column_desc(3008, i);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);
        }

        void set_param(int ol_i_id, int ol_supply_w_id, int *s_quantity, ObString *s_data, ObString s_dist[])
        {
          o_i_id_.set_int(ol_i_id);
          ol_supply_w_id_.set_int(ol_supply_w_id);

          s_quantity_ = s_quantity;
          s_data_ = s_data;
          s_dist_ = s_dist;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          int ret = OB_SUCCESS;
          double s_tmp = 0;
          if( OB_SUCCESS != ret ) {}
          else if( OB_SUCCESS != (ret = row.get_cell(table_id_, 18, cell)) )
          {
            TBSYS_LOG(WARN, "failed to get s_quantity");
          }
          else
          {
            cell->get_double(s_tmp);
            *s_quantity_ = (int)s_tmp;
            TBSYS_LOG(TRACE, "s_quantity: %d", *s_quantity_);
          }
          if( OB_SUCCESS != ret ) {}
          else if( OB_SUCCESS != (ret = row.get_cell(table_id_, 22, cell)))
          {
            TBSYS_LOG(WARN, "failed to get s_data");
          }
          else
          {
            cell->get_varchar(*s_data_);
            ob_write_string(*str_buf, *s_data_, *s_data_);
            TBSYS_LOG(TRACE, "s_data: %.*s", s_data_->length(), s_data_->ptr());
          }

          for(int i = 0; i < 10 && OB_SUCCESS == ret; ++i)
          {
            if( OB_SUCCESS != (ret = row.get_cell(table_id_, 23 + i, cell)) )
            {
              TBSYS_LOG(WARN, "failed to get s_dist_info");
            }
            else
            {
              cell->get_varchar(s_dist_[i]);
              ob_write_string(*str_buf, s_dist_[i], s_dist_[i]);
              TBSYS_LOG(TRACE, "s_dist_%d: %.*s", i, s_dist_[i].length(), s_dist_[i].ptr());
            }
          }
        }
    };


    struct UpdateStockParam : BasicParam
    {
        ObObj &ol_supply_w_id_;
        ObObj &o_i_id_;

        ObObj s_quantity_;
        ObObj o_quantity_;
        ObObj s_remote_cnt_increment_;


        UpdateStockParam() : BasicParam(3008, 6, LF_WRITE, 2), ol_supply_w_id_(keys_[0]), o_i_id_(keys_[1])
        {
          uint64_t temp_ids[] = {16, 17, 18, 19, 20, 21};

          for(int64_t i = 0; i < column_count_; ++i)
            column_ids_[i] = temp_ids[i];

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          write_desc_.reset();
          for(uint64_t i = 16; i <= 21; ++i)
          {
            write_desc_.add_column_desc(3008, i);
            row_desc_.add_column_desc(3008, i);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          write_desc_.set_rowkey_cell_count(2);
          row_desc_.set_rowkey_cell_count(2);
        }

        void set_param(int ol_i_id, int ol_supply_w_id, int s_quantity,
                       double o_quantity, int s_remote_cnt_increment)
        {
          ol_supply_w_id_.set_int(ol_supply_w_id);
          o_i_id_.set_int(ol_i_id);

          s_quantity_.set_int(s_quantity);
          o_quantity_.set_double(o_quantity);
          s_remote_cnt_increment_.set_int(s_remote_cnt_increment);
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          ObObj *cell;
          row.set_cell(table_id_, 18, s_quantity_);

          row.get_cell(table_id_, 19, cell);
          add_double(o_quantity_, *cell);
          row.set_cell(table_id_, 19, o_quantity_);

          row.get_cell(table_id_, 20, cell);
          ObObj tmp;
          tmp.set_int(1);
          add_int(tmp, *cell);
          row.set_cell(table_id_, 20, tmp);

          row.get_cell(table_id_, 21, cell);
          add_int(s_remote_cnt_increment_, *cell);
          row.set_cell(table_id_, 21, s_remote_cnt_increment_);

          row.set_row_desc(write_desc_);
        }
    };

    struct SelectDistrictParam : BasicParam
    {
        ObObj &d_w_id_;
        ObObj &d_id_;

        int *d_next_o_id_;
        double *d_tax_;
        SelectDistrictParam() : BasicParam(3002, 4, LF_WRITE, 3), d_w_id_(keys_[0]), d_id_(keys_[1])
        {
          uint64_t column_ids[] = {16, 17, 20, 19};

          for(int64_t i = 0; i < column_count_; ++i)
            column_ids_[i] = column_ids[i];

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          row_desc_.add_column_desc(3002, 16);
          row_desc_.add_column_desc(3002, 17);
          row_desc_.add_column_desc(3002, 20);
          row_desc_.add_column_desc(3002, 19);

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);
        }

        void set_param(int w_id, int d_id, int *d_next_o_id, double *d_tax)
        {
          d_w_id_.set_int(w_id);
          d_id_.set_int(d_id);
          d_next_o_id_ = d_next_o_id;
          d_tax_ = d_tax;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          UNUSED(str_buf);
          int64_t i_tmp = 0;
          row.get_cell(table_id_, 20, cell);
          cell->get_int(i_tmp);
          *d_next_o_id_ = (int)i_tmp;

          row.get_cell(table_id_, 19, cell);
          cell->get_double(*d_tax_);
        }
    };

    struct UpdateDistrictParam : BasicParam
    {
        ObObj &d_w_id_;
        ObObj &d_id_;

        UpdateDistrictParam() : BasicParam(3002, 3, LF_WRITE, 4), d_w_id_(keys_[0]), d_id_(keys_[1])
        {
          uint64_t column_ids[] = {16, 17, 20};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          write_desc_.reset();

          row_desc_.add_column_desc(3002, 16);
          row_desc_.add_column_desc(3002, 17);
          row_desc_.add_column_desc(3002, 20);
          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);

          write_desc_.add_column_desc(table_id_, 16); //table_id_ = 3002
          write_desc_.add_column_desc(table_id_, 17);
          write_desc_.add_column_desc(table_id_, 20);
          write_desc_.set_rowkey_cell_count(2);
        }

        void set_param(int w_id, int d_id)
        {
          d_w_id_.set_int(w_id);
          d_id_.set_int(d_id);
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          const ObObj *cell;
          ObObj next_o_id;
          next_o_id.set_int(1);

          row.get_cell(table_id_, 20, cell);
          add_int(next_o_id, *cell);
          row.set_cell(table_id_, 20, next_o_id);

          row.set_row_desc(write_desc_);
        }
    };

    struct InsertOOrderParam
    {
        ObObj o_id_;         //16
        ObObj o_d_id_;       //17
        ObObj o_w_id_;       //18
        ObObj o_c_id_;       //19
        ObObj o_entry_d_;    //23
        ObObj o_ol_cnt_;     //21
        ObObj o_all_local_;  //22

        int64_t table_id_;
        ObRowDesc row_desc_;

        InsertOOrderParam() : table_id_(3006)
        {
          row_desc_.reset();
          uint64_t temp_cols[]= {16, 17, 18, 19, 23, 21, 22};
          for(int64_t i = 0; i < 7; ++i)
          {
            row_desc_.add_column_desc(table_id_, temp_cols[i]);
          }

          row_desc_.set_rowkey_cell_count(3);
        }

        void set_param(int o_id, int d_id, int w_id, int c_id,
                       int64_t ts, int o_ol_cnt, int o_all_local)
        {
          o_id_.set_int(o_id);
          o_d_id_.set_int(d_id);
          o_w_id_.set_int(w_id);
          o_c_id_.set_int(c_id);
          o_entry_d_.set_int(ts);
          o_ol_cnt_.set_int(o_ol_cnt);
          o_all_local_.set_int(o_all_local);
        }
    };

    struct InsertNewOrderParam
    {
        ObObj no_o_id_; //18
        ObObj no_d_id_; //17
        ObObj no_w_id_; //16

        int64_t table_id_;
        ObRowDesc row_desc_;

        InsertNewOrderParam() : table_id_(3005)
        {
          row_desc_.reset();
          row_desc_.add_column_desc(table_id_, 16);
          row_desc_.add_column_desc(table_id_, 17);
          row_desc_.add_column_desc(table_id_, 18);

          row_desc_.set_rowkey_cell_count(3);
        }

        void set_param(int o_id, int d_id, int w_id)
        {
          no_o_id_.set_int(o_id);
          no_d_id_.set_int(d_id);
          no_w_id_.set_int(w_id);
        }
    };

    struct InsertOrderLineParam
    {
        ObObj ol_o_id_;         //18
        ObObj ol_d_id_;         //17
        ObObj ol_w_id_;         //16
        ObObj ol_number_;       //19
        ObObj ol_i_id_;         //20
        ObObj ol_supply_w_id_;  //23
        ObObj ol_quantity_;     //24
        ObObj ol_amount_;       //22
        ObObj ol_dist_info_;    //25

        int64_t table_id_;
        ObRowDesc row_desc_;

        InsertOrderLineParam() : table_id_(3007)
        {
          uint64_t temp_cols[] = {16, 17, 18, 19, 20, 23, 24, 22, 25};

          row_desc_.reset();
          for(int i = 0; i < 9; ++i)
          {
            row_desc_.add_column_desc(table_id_, temp_cols[i]);
          }
          row_desc_.set_rowkey_cell_count(4);
        }

        void set_param(int o_id, int d_id, int w_id,
                       int ol_number, int ol_i_id, int ol_supply_w_id,
                       int ol_quantity, double ol_amount, const ObString &ol_dist_info)
        {
          ol_o_id_.set_int(o_id);
          ol_d_id_.set_int(d_id);
          ol_w_id_.set_int(w_id);
          ol_number_.set_int(ol_number);
          ol_i_id_.set_int(ol_i_id);
          ol_supply_w_id_.set_int(ol_supply_w_id);
          ol_quantity_.set_int(ol_quantity);
          ol_amount_.set_double(ol_amount);
          ol_dist_info_.set_varchar(ol_dist_info);
        }
    };


    //Payment Parameters
    struct PaymentUpdateWareParam : BasicParam
    {
        ObObj &w_id_;

        ObObj h_amount_;


        PaymentUpdateWareParam() : BasicParam(3009, 2, LF_WRITE, 0), w_id_(keys_[0])
        {
          uint64_t temp_ids[] = {16, 17};

          write_desc_.reset();;
          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = temp_ids[i];
            write_desc_.add_column_desc(table_id_, temp_ids[i]);
            row_desc_.add_column_desc(table_id_, temp_ids[i]);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          write_desc_.set_rowkey_cell_count(1);
          row_desc_.set_rowkey_cell_count(1);

          key_column_ids_[0] = 16;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          UNUSED(str_buf);
          row.get_cell(table_id_, 17, cell);
          add_double(h_amount_, *cell);
          row.set_cell(table_id_, 17, h_amount_);

          row.set_row_desc(write_desc_);
        }
    };

    struct PaymentSelectWareParam : BasicParam
    {
        ObObj &w_id_;

        ObString *w_street_1_;
        ObString *w_street_2_;
        ObString *w_city_;
        ObString *w_state_;
        ObString *w_zip_;
        ObString *w_name_;

        PaymentSelectWareParam() : BasicParam(3009, 7, LF_NONE, 1), w_id_(keys_[0])
        {
          uint64_t temp_ids[] = {16, 20, 21, 22, 23, 24, 19};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = temp_ids[i];
            row_desc_.add_column_desc(table_id_, temp_ids[i]);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);

          key_column_ids_[0] = 16;
        }

        void set_param(ObString &w_street_1, ObString &w_street_2, ObString &w_city,
                       ObString &w_state, ObString &w_zip, ObString &w_name)
        {
          w_street_1_ = &w_street_1;
          w_street_2_ = &w_street_2;
          w_city_  = &w_city;
          w_state_ = &w_state;
          w_zip_  = &w_zip;
          w_name_ = &w_name;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;

          row.get_cell(table_id_, 20, cell);
          cell->get_varchar(*w_street_1_);

          row.get_cell(table_id_, 21, cell);
          cell->get_varchar(*w_street_2_);

          row.get_cell(table_id_, 22, cell);
          cell->get_varchar(*w_city_);

          row.get_cell(table_id_, 23, cell);
          cell->get_varchar(*w_state_);

          row.get_cell(table_id_, 24, cell);
          cell->get_varchar(*w_zip_);

          row.get_cell(table_id_, 19, cell);
          cell->get_varchar(*w_name_);

          ob_write_string(*str_buf, *w_name_, *w_name_);
          TBSYS_LOG(TRACE, "w_street_1: %.*s\n"
                           "w_street_2: %.*s\n"
                           "w_city: %.*s\n"
                           "w_state: %.*s\n"
                           "w_zip: %.*s\n"
                           "w_name: %.*s\n",
                    w_street_1_->length(), w_street_1_->ptr(),
                    w_street_2_->length(), w_street_2_->ptr(),
                    w_city_->length(), w_city_->ptr(),
                    w_state_->length(), w_state_->ptr(),
                    w_zip_->length(), w_zip_->ptr(),
                    w_name_->length(), w_name_->ptr());
        }
    };

    struct PaymentUpdateSelectWareParam : BasicParam
    {
        ObObj &w_id_;
        ObObj h_amount_; //input

        ObString *w_street_1_; //output
        ObString *w_street_2_;
        ObString *w_city_;
        ObString *w_state_;
        ObString *w_zip_;
        ObString *w_name_;

        PaymentUpdateSelectWareParam() : BasicParam(3009, 8, LF_NONE, -1), w_id_(keys_[0])
        {
          uint64_t temp_ids[] = {16, 17, 20, 21, 22, 23, 24, 19};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = temp_ids[i];
            row_desc_.add_column_desc(table_id_, temp_ids[i]);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);

          write_desc_.add_column_desc(table_id_, 16);
          write_desc_.add_column_desc(table_id_, 17);
          write_desc_.set_rowkey_cell_count(1);

          key_column_ids_[0] = 16;
        }

        void set_param(ObString &w_street_1, ObString &w_street_2, ObString &w_city,
                       ObString &w_state, ObString &w_zip, ObString &w_name)
        {
          w_street_1_ = &w_street_1;
          w_street_2_ = &w_street_2;
          w_city_  = &w_city;
          w_state_ = &w_state;
          w_zip_  = &w_zip;
          w_name_ = &w_name;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;

          row.get_cell(table_id_, 17, cell);
          add_double(h_amount_, *cell);

          row.get_cell(table_id_, 20, cell);
          cell->get_varchar(*w_street_1_);

          row.get_cell(table_id_, 21, cell);
          cell->get_varchar(*w_street_2_);

          row.get_cell(table_id_, 22, cell);
          cell->get_varchar(*w_city_);

          row.get_cell(table_id_, 23, cell);
          cell->get_varchar(*w_state_);

          row.get_cell(table_id_, 24, cell);
          cell->get_varchar(*w_zip_);

          row.get_cell(table_id_, 19, cell);
          cell->get_varchar(*w_name_);

          ob_write_string(*str_buf, *w_name_, *w_name_);
          TBSYS_LOG(TRACE, "w_street_1: %.*s\n"
                           "w_street_2: %.*s\n"
                           "w_city: %.*s\n"
                           "w_state: %.*s\n"
                           "w_zip: %.*s\n"
                           "w_name: %.*s\n",
                    w_street_1_->length(), w_street_1_->ptr(),
                    w_street_2_->length(), w_street_2_->ptr(),
                    w_city_->length(), w_city_->ptr(),
                    w_state_->length(), w_state_->ptr(),
                    w_zip_->length(), w_zip_->ptr(),
                    w_name_->length(), w_name_->ptr());
        }
    };

    struct PaymentUpdateDistParam : BasicParam
    {
        ObObj &d_w_id_;
        ObObj &d_id_;

        ObObj h_amount_;


        PaymentUpdateDistParam() : BasicParam(3002, 3, LF_WRITE, 2), d_w_id_(keys_[0]), d_id_(keys_[1])
        {
          uint64_t temp_ids[] = {16, 17, 18};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = temp_ids[i];
            row_desc_.add_column_desc(table_id_, temp_ids[i]);
            write_desc_.add_column_desc(table_id_, temp_ids[i]);
          }

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);
          write_desc_.set_rowkey_cell_count(2);

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          UNUSED(str_buf);

          row.get_cell(table_id_, 18, cell);
          add_double(h_amount_, *cell);
          row.set_cell(table_id_, 18, h_amount_);
          row.set_row_desc(write_desc_);
        }
    };

    struct PaymentSelectDistParam : BasicParam
    {
        ObObj &d_w_id_;
        ObObj &d_id_;

        ObString *d_street_1_;
        ObString *d_street_2_;
        ObString *d_city_;
        ObString *d_state_;
        ObString *d_zip_;
        ObString *d_name_;

        PaymentSelectDistParam() : BasicParam(3002, 8, LF_WRITE, 3), d_w_id_(keys_[0]), d_id_(keys_[1])
        {
          uint64_t column_ids[] = {16, 17, 22, 23, 24, 25, 26, 21};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);
        }

        void set_param(ObString &d_street_1, ObString &d_street_2, ObString &d_city,
                       ObString &d_state, ObString &d_zip, ObString &d_name)
        {
          d_street_1_ = &d_street_1;
          d_street_2_ = &d_street_2;
          d_city_ = &d_city;
          d_state_ = &d_state;
          d_zip_ = &d_zip;
          d_name_ = &d_name;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;

          row.get_cell(table_id_, 22, cell);
          cell->get_varchar(*d_street_1_);

          row.get_cell(table_id_, 23, cell);
          cell->get_varchar(*d_street_2_);

          row.get_cell(table_id_, 24, cell);
          cell->get_varchar(*d_city_);

          row.get_cell(table_id_, 25, cell);
          cell->get_varchar(*d_state_);

          row.get_cell(table_id_, 26, cell);
          cell->get_varchar(*d_zip_);

          row.get_cell(table_id_, 21, cell);
          cell->get_varchar(*d_name_);

          ob_write_string(*str_buf, *d_name_, *d_name_);

          TBSYS_LOG(TRACE, "d_street_1: %.*s\n"
                           "d_street_2: %.*s\n"
                           "d_city: %.*s\n"
                           "d_state: %.*s\n"
                           "d_zip: %.*s\n"
                           "d_name: %.*s\n",
                    d_street_1_->length(), d_street_1_->ptr(),
                    d_street_2_->length(), d_street_2_->ptr(),
                    d_city_->length(), d_city_->ptr(),
                    d_state_->length(), d_state_->ptr(),
                    d_zip_->length(), d_zip_->ptr(),
                    d_name_->length(), d_name_->ptr());
        }
    };

    struct PaymentUpdateSelectDistParam : BasicParam
    {
        ObObj &d_w_id_;
        ObObj &d_id_;

        ObObj h_amount_;

        ObString *d_street_1_;
        ObString *d_street_2_;
        ObString *d_city_;
        ObString *d_state_;
        ObString *d_zip_;
        ObString *d_name_;

        PaymentUpdateSelectDistParam() : BasicParam(3002, 9, LF_WRITE, 3), d_w_id_(keys_[0]), d_id_(keys_[1])
        {
          uint64_t column_ids[] = {16, 17, 18, 22, 23, 24, 25, 26, 21};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(2);

          write_desc_.add_column_desc(table_id_, 16);
          write_desc_.add_column_desc(table_id_, 17);
          write_desc_.add_column_desc(table_id_, 18);
          write_desc_.set_rowkey_cell_count(2);
        }

        void set_param(ObString &d_street_1, ObString &d_street_2, ObString &d_city,
                       ObString &d_state, ObString &d_zip, ObString &d_name)
        {
          d_street_1_ = &d_street_1;
          d_street_2_ = &d_street_2;
          d_city_ = &d_city;
          d_state_ = &d_state;
          d_zip_ = &d_zip;
          d_name_ = &d_name;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;

          row.get_cell(table_id_, 18, cell);
          add_double(h_amount_, *cell);

          row.get_cell(table_id_, 22, cell);
          cell->get_varchar(*d_street_1_);

          row.get_cell(table_id_, 23, cell);
          cell->get_varchar(*d_street_2_);

          row.get_cell(table_id_, 24, cell);
          cell->get_varchar(*d_city_);

          row.get_cell(table_id_, 25, cell);
          cell->get_varchar(*d_state_);

          row.get_cell(table_id_, 26, cell);
          cell->get_varchar(*d_zip_);

          row.get_cell(table_id_, 21, cell);
          cell->get_varchar(*d_name_);

          ob_write_string(*str_buf, *d_name_, *d_name_);

          TBSYS_LOG(TRACE, "d_street_1: %.*s\n"
                           "d_street_2: %.*s\n"
                           "d_city: %.*s\n"
                           "d_state: %.*s\n"
                           "d_zip: %.*s\n"
                           "d_name: %.*s\n",
                    d_street_1_->length(), d_street_1_->ptr(),
                    d_street_2_->length(), d_street_2_->ptr(),
                    d_city_->length(), d_city_->ptr(),
                    d_state_->length(), d_state_->ptr(),
                    d_zip_->length(), d_zip_->ptr(),
                    d_name_->length(), d_name_->ptr());
        }
    };

    struct PaymentSelectCustParam : BasicParam
    {
        ObObj &c_w_id_;
        ObObj &c_d_id_;
        ObObj &c_id_;

        ObString *c_credit_;
        double *c_balance_;
        double *c_ytd_payment_;
        int *c_payment_cnt_;

        PaymentSelectCustParam() : BasicParam(3001, 7, LF_NONE, 4), c_w_id_(keys_[0]), c_d_id_(keys_[1]), c_id_(keys_[2])
        {
          uint64_t column_ids[] = {16, 17, 18, 20, 24, 25, 26};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;
          key_column_ids_[2] = 18;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(3);
        }

        void set_param(ObString &c_credit, double &c_balance, double c_ytd_payment, int c_payment_cnt)
        {
          c_credit_ = &c_credit;
          c_balance_ = &c_balance;
          c_ytd_payment_ = &c_ytd_payment;
          c_payment_cnt_ = &c_payment_cnt;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          int64_t tmp = 0;
          row.get_cell(table_id_, 20, cell);
          cell->get_varchar(*c_credit_);
          ob_write_string(*str_buf, *c_credit_, *c_credit_);

          row.get_cell(table_id_, 24, cell);
          cell->get_double(*c_balance_);

          row.get_cell(table_id_, 25, cell);
          cell->get_double(*c_ytd_payment_);

          row.get_cell(table_id_, 26, cell);
          cell->get_int(tmp);
          *c_payment_cnt_ = (int)tmp;

          TBSYS_LOG(TRACE, "c_credit: %.*s"
                           "c_balance: %lf"
                           "c_ytd_payment: %lf"
                           "c_payment_cnt: %d",
                    c_credit_->length(), c_credit_->ptr(),
                    *c_balance_,
                    *c_ytd_payment_,
                    *c_payment_cnt_);
        }
    };

    struct PaymentSelectCustCDataParam : BasicParam
    {
        ObObj &c_w_id_;
        ObObj &c_d_id_;
        ObObj &c_id_;

        ObString *c_data_;

        PaymentSelectCustCDataParam() : BasicParam(3001, 4, LF_NONE, 5), c_w_id_(keys_[0]), c_d_id_(keys_[1]), c_id_(keys_[2])
        {
          uint64_t column_ids[] = {16, 17, 18, 36};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;
          key_column_ids_[2] = 18;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(3);
        }

        void set_param(ObString &c_data)
        {
          c_data_ = &c_data;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          row.get_cell(table_id_, 36, cell);
          cell->get_varchar(*c_data_);
          ob_write_string(*str_buf, *c_data_, *c_data_);

          TBSYS_LOG(TRACE, "c_data: %.*s",
                    c_data_->length(), c_data_->ptr());
        }
    };

    struct PaymentUpdateCustCDataParam : BasicParam
    {
        ObObj &c_w_id_;
        ObObj &c_d_id_;
        ObObj &c_id_;

        ObObj c_balance_;
        ObObj c_ytd_payment_;
        ObObj c_payment_cnt_;
        ObObj c_data_;


        PaymentUpdateCustCDataParam() : BasicParam(3001, 7, LF_WRITE, 6), c_w_id_(keys_[0]), c_d_id_(keys_[1]), c_id_(keys_[2])
        {
          uint64_t column_ids[] = {16, 17, 18, 24, 25, 26, 36};

          write_desc_.reset();
          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
            write_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;
          key_column_ids_[2] = 18;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(3);
          write_desc_.set_rowkey_cell_count(3);
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          row.set_cell(table_id_, 24, c_balance_);
          row.set_cell(table_id_, 25, c_ytd_payment_);
          row.set_cell(table_id_, 26, c_payment_cnt_);
          row.set_cell(table_id_, 36, c_data_);
          row.set_row_desc(write_desc_);
        }
    };

    struct PaymentUpdateCustParam : BasicParam
    {
        ObObj &c_w_id_;
        ObObj &c_d_id_;
        ObObj &c_id_;

        ObObj c_balance_;
        ObObj c_ytd_payment_;
        ObObj c_payment_cnt_;


        PaymentUpdateCustParam() : BasicParam(3001, 6, LF_WRITE, 7), c_w_id_(keys_[0]), c_d_id_(keys_[1]), c_id_(keys_[2])
        {
          uint64_t column_ids[] = {16, 17, 18, 24, 25, 26};

          write_desc_.reset();
          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
            write_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;
          key_column_ids_[1] = 17;
          key_column_ids_[2] = 18;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(3);
          write_desc_.set_rowkey_cell_count(3);
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          row.set_cell(table_id_, 24, c_balance_);
          row.set_cell(table_id_, 25, c_ytd_payment_);
          row.set_cell(table_id_, 26, c_payment_cnt_);
          row.set_row_desc(write_desc_);
        }
    };

    struct PaymentInsertHistParam
    {
        ObObj h_c_id_;
        ObObj h_c_d_id_;
        ObObj h_c_w_id_;
        ObObj h_d_id_;
        ObObj h_w_id_;
        ObObj h_date_;
        ObObj h_amount_;
        ObObj h_data_;

        int64_t table_id_;
        ObRowDesc row_desc_;

        PaymentInsertHistParam() : table_id_(3003)
        {
          row_desc_.reset();
          uint64_t temp_cols[] = {16, 17, 18, 19, 20, 21, 22, 23};
          for(int64_t i = 0; i < 8; ++i)
          {
            row_desc_.add_column_desc(table_id_, temp_cols[i]);
          }

          row_desc_.set_rowkey_cell_count(8);
        }
    };

    struct DepParam : BasicParam
    {
        ObObj &c_id_;

        ObObj c_value_;

        DepParam() : BasicParam(3001, 2, LF_WRITE, -1), c_id_(keys_[0])
        {
          uint64_t column_ids[] = {16, 18};

          write_desc_.reset();

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
            write_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
          write_desc_.set_rowkey_cell_count(1);
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          const ObObj *cell = NULL;
          row.get_cell(table_id_, 18, cell);
          add_int(c_value_, *cell);

          row.set_cell(table_id_, 18, c_value_);
          row.set_row_desc(write_desc_);
        }
    };

    struct SelectSavingParam : BasicParam
    {
        ObObj &acct_id_;

        double *bal_;

        SelectSavingParam(int64_t static_data_id) : BasicParam(3002, 2, LF_WRITE, static_data_id), acct_id_(keys_[0])
        {
          uint64_t column_ids[] = {16, 17};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
        }

        void set_param(int acct_id, double &bal)
        {
          acct_id_.set_int(acct_id);
          bal_ = &bal;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          float tmp = 0;
          UNUSED(str_buf);
          row.get_cell(table_id_, 17, cell);
//          cell->get_double(*bal_);
          cell->get_float(tmp);
          *bal_ = (double)tmp;

          TBSYS_LOG(TRACE, "saving balance: %lf", *bal_);
        }
    };

    struct SelectCheckParam : BasicParam
    {
        ObObj &acct_id_;

        double *bal_;

        SelectCheckParam(int64_t static_data_id) : BasicParam(3003, 2, LF_WRITE, static_data_id), acct_id_(keys_[0])
        {
          uint64_t column_ids[] = {16, 17};

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
        }

        void set_param(int acct_id, double &bal)
        {
          acct_id_.set_int(acct_id);
          bal_ = &bal;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          const ObObj *cell;
          float tmp = 0;
          UNUSED(str_buf);
          row.get_cell(table_id_, 17, cell);
//          cell->get_double(*bal_);
          cell->get_float(tmp);
          *bal_ = tmp;
          TBSYS_LOG(TRACE, "check balance: %lf", *bal_);
        }
    };

    typedef void (*bal_func)(double &base_bal, double bal);
    struct UpdateSavingParam : BasicParam
    {
        ObObj &acct_id_;

        double bal_;
        bal_func fp_;

        UpdateSavingParam(int64_t static_data_id) : BasicParam(3002, 2, LF_WRITE, static_data_id), acct_id_(keys_[0]), fp_(NULL)
        {
          uint64_t column_ids[] = {16, 17};

          write_desc_.reset();

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
            write_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
          write_desc_.set_rowkey_cell_count(1);
        }

        void set_param(int acct_id, double bal, bal_func fp)
        {
          acct_id_.set_int(acct_id);
          bal_ = bal;
          fp_ = fp;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          const ObObj *cell = NULL;
          double base_bal;
          float tmp = 0;
          ObObj new_val;
          row.get_cell(table_id_, 17, cell);
//          cell->get_double(base_bal);
          cell->get_float(tmp);
          base_bal = (double)tmp;

          if( NULL != fp_ ) fp_(base_bal, bal_);

          new_val.set_double(base_bal);
          row.set_cell(table_id_, 17, new_val);
          row.set_row_desc(write_desc_);
          TBSYS_LOG(TRACE, "savings update row: %s", to_cstring(row));
        }
    };

    struct UpdateCheckParam : BasicParam
    {
        ObObj &acct_id_;

        double bal_;
        bal_func fp_;

        UpdateCheckParam(int64_t static_data_id) : BasicParam(3003, 2, LF_WRITE, static_data_id), acct_id_(keys_[0]), fp_(NULL)
        {
          uint64_t column_ids[] = {16, 17};

          write_desc_.reset();

          for(int64_t i = 0; i < column_count_; ++i)
          {
            column_ids_[i] = column_ids[i];
            row_desc_.add_column_desc(table_id_, column_ids[i]);
            write_desc_.add_column_desc(table_id_, column_ids[i]);
          }

          key_column_ids_[0] = 16;

          row_desc_.add_column_desc(OB_INVALID_ID, OB_ACTION_FLAG_COLUMN_ID);
          row_desc_.set_rowkey_cell_count(1);
          write_desc_.set_rowkey_cell_count(1);
        }

        void set_param(int acct_id, double bal, bal_func fp)
        {
          acct_id_.set_int(acct_id);
          bal_ = bal;
          fp_ = fp;
        }

        void project_row(ObRow &row, ObStringBuf *str_buf)
        {
          UNUSED(str_buf);
          const ObObj *cell = NULL;
          double base_bal;
          float tmp = 0;
          ObObj new_val;
          row.get_cell(table_id_, 17, cell);
//          cell->get_double(base_bal);
          cell->get_float(tmp);
          base_bal = (double)tmp;

          if( NULL != fp_ ) fp_(base_bal, bal_);
          TBSYS_LOG(TRACE, "%p, %f, %f", fp_, base_bal, bal_);

          new_val.set_double(base_bal);
          row.set_cell(table_id_, 17, new_val);
          row.set_row_desc(write_desc_);
          TBSYS_LOG(TRACE, "check update row: %s", to_cstring(row));
        }
    };

    class ObUpsTpcc
    {
      public:
        ObUpsTpcc(ObUpsProcedure *proc) : proc_(proc) {}

        int execute_neworder(int w_id, int d_id, int c_id, int item_ids[],
                    double i_prices[], int supplier_w_ids[],
                    int order_quantities[], int o_ol_cnt, int o_all_local);

        int execute_neworder_strict_order(int w_id, int d_id, int c_id, int item_ids[],
                             double i_prices[], int supplier_w_ids[],
                             int order_quantities[], int o_ol_cnt, int o_all_local);


        int execute_payment(int w_id, int d_id, int c_id,
                            int c_w_id ,int c_d_id, double h_amount);

        int execute_payment_merge_sql(int w_id, int d_id, int c_id,
                                      int c_w_id, int c_d_id, double h_amount);

        int execute_update(int a_id, int b_id, int c_id, int d_id, int e_id, int value);

        int execute_transact_savings(int acctId, double amount);

        int execute_send_payment(int sendAcct, int destAcct, double amount);

        int execute_write_check(int acctId, double amount);

        int execute_amalgamate(int acctId0, int acctId1);

      private:

        int prepare(BasicParam &param);

        int close();

        int insert_oorder(InsertOOrderParam &param);

        int insert_neworder(InsertNewOrderParam &param);

        int insert_orderline(InsertOrderLineParam &param);

        //payment sql
        int tpcc_update(BasicParam &param);

        int tpcc_select(BasicParam &param);

        int db_update(BasicParam &param);

        int db_select(BasicParam &param);

        int payment_insert_hist(PaymentInsertHistParam &param);

        int payment_insert_cust_1(PaymentUpdateCustParam &param);

        int payment_insert_cust_2(PaymentUpdateCustCDataParam &param);

        int payment_update_select_dist(PaymentUpdateSelectDistParam &param);

        int payment_update_select_ware(PaymentUpdateSelectWareParam &param);

        int write_row(RWSessionCtx &ctx, const ObRow &row, const ObRowDesc &row_desc);

        static const int32_t MAX_ITEM_COUNT = 16;
      private:
        ObUpsProcedure *proc_;

        ObUpsTableMgr *table_mgr_;
        ITableEntity::Guard* guard_;
        char guard_buf_[sizeof(ITableEntity::Guard)];
        TableList table_list_;
        ObCellAdaptor cia_;
        ObStringBuf str_buf_;
    };

    class ObUpsProcedureSpecialExecutor
    {
    public:
      ObUpsProcedureSpecialExecutor();
    };
  }
}

#endif // OB_UPS_PROCEDURE_SPECIAL_EXECUTOR_H
