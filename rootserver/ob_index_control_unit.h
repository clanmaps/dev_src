/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_control_unit.h
 * @brief control unit of index construction
 *  icu start when common daily merge begin,and will submit mission while common merge finished.
 *  icu will control main procedure of index construction one by one
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */


#ifndef OB_INDEX_CONTROL_UNIT_H
#define OB_INDEX_CONTROL_UNIT_H
#include "common/ob_define.h"
#include "common/ob_array.h"
#include "ob_root_server2.h"
#include "ob_index_cs_handler.h"
#include "ob_index_service.h"
#include "ob_index_designer.h"

namespace oceanbase
{
  namespace rootserver
  {
    class ObRootWorker;
    /**
     * @brief The ObIndexControlUnit class
     * ObIndexControlUnit is designed for
     * index construction procedure monitor
     * .......
     */
    class ObIndexControlUnit
    {
      public:

        /**
         * @brief constructor
         */
        ObIndexControlUnit();

        /**
         * @brief destructor
         */
        ~ObIndexControlUnit();

        /**
         * @brief init function of icu
         * @param worker need link field of RootWorker.
         */
        void    init(ObRootWorker *worker);

        /**
         * @brief start icu
         * general start while daily merge begin,
         * it will lock icu mutex
         */
        void    start();

        /**
         * @brief stop icu
         * when all index construction finished,icu will stop
         * it will clean up icu env
         */
        void    stop();

        /**
         * @brief start index construct mission
         * it will lock icu mutex, and submit a job packet to server
         * @return err code if success or not.
         */
        int     start_mission();

        /**
         * @brief main procedure of index construction
         * it will drive index construction one by one
         * @return err code if success or not.
         */
        int     schedule();

        /**
         * @brief set version of daily merge here
         * this version will be index tablet version in future
         */
        void    set_version(int64_t start_version);

        /**
         * @brief checkout if icu is start
         * @return boolean if icu start.
         */
        inline  bool  is_start(){return start_;}

      public:

        /**
         * @brief push all index which are init status into job_list
         * job_list_ maintaince index to be constructed
         * @return err code if success.
         */
        int     generate_job_list();

        /**
         * @brief handle histogram of report tablet info from chunkserver.
         * @param add_tablets report info list, which is reported by chunkserver while index constructed.
         * @param server_index index of chunkserver which report tablet info list.
         * @return err code if success or not.
         */
        int     handle_histograms(const common::ObTabletHistogramReportInfoList& add_tablets, const int server_index);

        /**
         * @brief run index job one by one by called pipe run job
         * iterator the index of job list, if one construct failed, push it into failed_list_
         */
        void    run_job();

        /**
         * @brief prepare stage env before one index construction beginning.
         * @param index_tid index id which will be constructed.
         * @return err code if success or not.
         */
        int     stage_prepare(uint64_t index_tid);

        /**
         * @brief called by run_job, run one single index job.
         * @param index_tid index id which will be constructed.
         * @return err code if success or not.
         */
        int     pipe_run_job(uint64_t index_tid);

        /**
         * @brief get index heart beat
         * IndexBeat has info of index tid & status
         * @return struct IndexBeat.
         */
        IndexBeat& get_beat(){return ch_.get_beat();}

        /**
         * @brief get cs handler
         * chunkserver handler has control cs selection of index construct procedure
         * @return class of cshandler.
         */
        ObCSHandler& get_cs_handler(){return ch_;}
      public:

        /**
         * @brief called index designer to sort all histogram
         * this is an indispensable link to design global index
         * @return err code if success or not.
         */
        int     fill_all_samples();

        /**
         * @brief clean mess while one index constructed failed.It will clean designer
         *        and root_table(if needed)
         * @param idx_id failed index id.
         * @param need_delete_rt if tablet info of this index needed be deleted.
         * @return err code if success or not.
         */
        int     clean_mess(const uint64_t idx_id, const bool need_delete_rt);

        /**
         * @brief set start version.
         * @param version daily merge version, it will be index tablet version.
         */
        inline  void set_start_version(int64_t version){version_ = version;}

        /**
         * @brief get IndexService class
         * @return IndexService.
         */
        ObIndexService &get_service(){return service_;}

        bool check_create_index_over();


      private:

        int64_t version_;                               ///< index tablet version
        int64_t finish_index_num_;                      ///< number of finish construct index
        volatile int64_t mission_start_time_;           ///< time stamp of index mission start
        bool start_;                                    ///< boolean if icu start
        bool job_running_;                              ///< boolean if index mission start
        ObRootWorker    *root_woker_;                   ///< link field of root worker
        ObIndexService  service_;                       ///< IndexService, provide interface to roottable/inner sys table
        ObIndexDesigner *designer_;                     ///< link filed of index designer
        ObCSHandler     ch_;                            ///< chunkserver handler ,select cs to construct local/global index
        ObArray<uint64_t> job_list_;                    ///< index list to be constructed
        ObArray<uint64_t> failed_list_;                 ///< failed index list
        tbsys::CThreadMutex icu_mutex_;                 ///< mutex lock of icu
        mutable tbsys::CThreadMutex designer_mutex_;    ///< mutex lock of designer
    };

    inline bool ObIndexControlUnit::check_create_index_over()
    {
      return !job_running_;
    }
  }
}


#endif // OB_INDEX_CONTROL_UNIT_H

