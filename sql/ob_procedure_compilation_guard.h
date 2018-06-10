#ifndef OBPROCEDURECOMPILATIONGUARD_H
#define OBPROCEDURECOMPILATIONGUARD_H

#include "ob_sp_procedure.h"
namespace oceanbase
{
  namespace sql
  {
    class ObTransformer;

    /**
     * @brief The ObProcedureCompilationContext struct
     *  manage the compilation context of a sql in procedure
     */
    struct ObProcedureCompilationContext
    {
      SpRwDeltaInst  *rw_delta_inst_;
      SpRdBaseInst   *rd_base_inst_;
      SpRwCompInst   *rd_all_inst_;
      SpPlainSQLInst *sql_inst_;

      ObSEArray<const ObSqlRawExpr *, 16> key_where_;
      ObSEArray<const ObSqlRawExpr *, 16> nonkey_where_;
      ObSEArray<const ObSqlRawExpr *, 16> value_project_;
      ObSEArray<uint64_t, 8>              access_tids_;

      bool is_full_key_;
      bool using_index_;

    public:
      void fill_variable_info(ObTransformer *trans);
      void clear();
      void bind_ups_executor(ObPhyOperator *ups_exec, int32_t idx);
    };

    /**
     * @brief The ObProcedureCompilationGuard class
     * automatic set and reset of the compilation context
     * remember using it whenever transforming a sql in procedure
     */
    class ObProcedureCompilationGuard
    {
    public:
      ObProcedureCompilationGuard(ObTransformer *trans, ObProcedureCompilationContext &context)
        : trans_(trans), context_(context)
      {
        context_.clear();
      }

      virtual ~ObProcedureCompilationGuard()
      {
        context_.fill_variable_info(trans_);
        context_.clear();
        trans_ = NULL;
      }
    private:
      ObTransformer *trans_;
      ObProcedureCompilationContext &context_;
    };
  }
}


#endif // OBPROCEDURECOMPILATIONGUARD_H
