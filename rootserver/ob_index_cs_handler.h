/**
 * Copyright (C) 2013-2016 DaSE .
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * @file ob_index_cs_handler.h
 * @brief handle chunkserver while index construct,include cs selection/server off_line and so on
 *
 * Created by Wenghaixing
 *
 * @version CEDAR 0.2 
 * @author wenghaixing <wenghaixing@ecnu.cn>
 * @date  2016_01_24
 */


#ifndef OB_INDEX_CS_HANDLER_H
#define OB_INDEX_CS_HANDLER_H

#include "common/hash/ob_hashmap.h"
#include "common/ob_server.h"
#include "common/ob_schema_service.h"
#include "common/ob_define.h"
using namespace oceanbase::common;
using namespace oceanbase::common::hash;
namespace oceanbase
{
  namespace rootserver
  {
    ///HASH ERR CODE, IS SAME AS HASH.H
    enum
    {
      HASH_EXIST = 0xf,
      HASH_NOT_EXIST,
      HASH_OVERWRITE_SUCC,
      HASH_INSERT_SUCC,
     HASH_GET_TIMEOUT,
    };
    const static int MAX_CHUNKSERVER_NUM = 256; ///< max chunkserver number limit
    class ObRootWorker;
    class ObRootServer;

    /**
     * @brief The ObCSHandler class
     * ObCSHandler is designed for
     * handle chunkserver selection & off_line while index construction beginning
     */
    class ObCSHandler
    {

      public:
        /**
         * @brief constructor
         */
        ObCSHandler();

        /**
         * @brief destructor
         */
        ~ObCSHandler();

        /**
         * @brief reset handler param
         */
        void reset();

        /**
         * @brief set index_tid to cs handler.
         * @param idx index id of handler
         */
        void set_index_tid(uint64_t idx);

        /**
         * @brief init inner env of cs handler.
         * @return err code if success or not.
         */
        int  init(ObRootWorker *worker);

        /**
         * @brief get hitogram width
         *
         * @return width.
         */
        int64_t get_width(){return width_;}
      public:

        /**
         * @brief handle off_line of chunkserver while index constructing.
         * @param cs chunkserver which is off_line.
         * @return if success or not.
         */
        int   server_off_line(ObServer &cs);

        /**
         * @brief set stage into index beat.
         * @param ph index stage .
         */
        void   set_index_beat(ConIdxStage ph);

        /**
         * @brief construct handler core, which is a hash map of chunkserver/alive status.
         * @param refresh_width boolean if need re-construct hash map core
         * @return err code if success or not.
         */
        int   construct_handler_core(bool refresh_width = true);

        /**
         * @brief if chunkserver hit core hash map.
         * @param cs chunkserver to checkout.
         * @return if hit hashmap.
         */
        bool  cs_hit_hashmap(ObServer &cs);

        /**
         * @brief get heart beat to send to chunkserver
         * @return IndexBeat.
         */
        IndexBeat& get_beat();

        /**
         * @brief get default heart beat
         * when need not to construct index, send default heartbeat
         * @return IndexBeat.
         */
        IndexBeat& get_default_beat();
      private:
        /**
         * @brief fetch tablet info from root table.
         * @param table_id table id which to be fetch
         * @param row_key start key to  search.generally begin with MIN_ROWKEY
         * @param out param , put it into scannner
         * @return err code if success or not.
         */
        int fetch_tablet_info(const uint64_t table_id,
                              const ObRowkey & row_key, ObScanner & scanner);

        /**
         * @brief fill core hash map and calculate index width.
         * @param scanner the out scanner include tablet info fetched
         * @param row_key start key begining
         * @param table_id table id to fill
         * @param refresh_width boolean if need re-calculate width,default is false
         * @return err code if success or not.
         */
        int fill_cm_and_calc(ObScanner &scanner, ObRowkey &row_key, uint64_t table_id, bool refresh_width = false);

        /**
         * @brief dump core hash map into LOG
         */
        void dump_core();
      private:
        hash::ObHashMap<ObServer,bool,hash::NoPthreadDefendMode> cm_;///< core hash mapkey chunkserver, value is_alive
        ObRootWorker *root_worker_;                                  ///< link field of Root Worker
        IndexBeat beat_;                                             ///< Index Beat to be send to chunkserver
        IndexBeat default_beat_;                                     ///< default heartbeat to be send to chunkserver
        uint64_t index_tid_;                                         ///< index tid
        int64_t  width_;                                             ///< means every width rows take a histogram
        bool     init_;                                              ///< boolean if inited
    };
  }


}


#endif // OB_INDEX_CS_HANDLER_H

