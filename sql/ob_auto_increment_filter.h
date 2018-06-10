#ifndef _OB_AUTO_INCREMENT_FILTER_H
#define _OB_AUTO_INCREMENT_FILTER_H

#include "ob_single_child_phy_operator.h"

namespace oceanbase
{
  namespace sql
  {
    class ObAutoIncrementFilter : public ObSingleChildPhyOperator
    {
      public:
        ObAutoIncrementFilter();

        int open();
        virtual void reset();
        virtual void reuse();
        int get_next_row(const common::ObRow *&row);
        int get_row_desc(const common::ObRowDesc *&row_desc) const;
        int64_t to_string(char* buf, const int64_t buf_len) const;
        enum ObPhyOperatorType get_type() const;

        void set_auto_column(const uint64_t auto_column_id, const int64_t auto_value);
        int64_t get_cur_auto_value() const;

        DECLARE_PHY_OPERATOR_ASSIGN;
        NEED_SERIALIZE_AND_DESERIALIZE;
      private:
        common::ObRowDesc cur_row_desc_;
        common::ObRow cur_row_;
        uint64_t auto_column_id_;
        int64_t auto_value_;
        bool is_assigned_;
    };
  }
}

#endif // _OB_AUTO_INCREMENT_FILTER_H
