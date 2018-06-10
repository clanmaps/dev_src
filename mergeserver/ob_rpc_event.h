/*
 * (C) 2007-2010 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *
 *
 * Version: 0.1: ob_rpc_event.h,v 0.1 2011/09/26 14:01:30 zhidong Exp $
 *
 * Authors:
 *   zhidong <xielun.szd@taobao.com>
 *     - some work details if you want
 *
 */

#ifndef OCEANBASE_COMMON_RPC_EVENT_H_
#define OCEANBASE_COMMON_RPC_EVENT_H_

#include "common/ob_packet.h"
#include "common/ob_define.h"
#include "common/ob_server.h"
#include "common/ob_scanner.h"
#include "onev_struct.h"

namespace oceanbase
{
  namespace mergeserver
  {
    class ObCommonRpcEvent
    {
    public:
      ObCommonRpcEvent();
      virtual ~ObCommonRpcEvent();

    protected:
      /// reset for reuse
      virtual void reset(void);

    public:
      /// get/set event id
      uint64_t get_event_id(void) const;

      // get/set server for debug info
      const common::ObServer & get_server(void) const;
      void set_server(const common::ObServer & server);

      /// get/set result code
      int32_t get_result_code(void) const;
      void set_result_code(const int32_t code);

      //get handler
      onev_io_process_pe* get_handler() const;

      /// get response for serialize the result from net io buffer
      common::ObScanner & get_result(void);
      /// get response for read and response code
      common::ObScanner & get_result(int32_t & response_code);

      /// print info
      void print_info(FILE * file) const;

      void start();
      void end();
      int64_t get_time_used()const;

      static const int64_t INVALID_SESSION_ID = 0;
      static const int64_t TERMINATED_SESSION_ID = -1;

      inline void set_session_id(const int64_t session_id)
      {
        session_id_ = session_id;
      }

      inline int64_t get_session_id()const
      {
        return session_id_;
      }

      inline bool is_session_end() const
      {
        return TERMINATED_SESSION_ID == session_id_;
      }

      inline void set_session_end()
      {
        session_id_ = TERMINATED_SESSION_ID;
      }

    private:
      // event id allocator
      static uint64_t id_allocator_;

    protected:
      // the event_id get from the condition manager
      uint64_t event_id_;
      int32_t result_code_;
      common::ObServer server_;
      common::ObScanner result_;

      int64_t start_time_us_;
      int64_t end_time_us_;
      int64_t session_id_;
      //packet handler
      onev_io_process_pe *handler_;
    };
  }
}


#endif // OCEANBASE_COMMON_RPC_EVENT_H_
