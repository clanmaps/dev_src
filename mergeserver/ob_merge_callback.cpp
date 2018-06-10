#include "common/ob_packet_factory.h"
#include "ob_merge_callback.h"
#include "tblog.h"
#include "mergeserver/ob_merge_server.h"
#include "onev_struct.h"
#include "common/ob_packet.h"
#include "ob_ms_rpc_event.h"
#include "ob_ms_sql_rpc_event.h"
#include "ob_merge_server_main.h"
#include "ob_ms_async_rpc.h"
#include "common/utility.h"
#include "common/ob_profile_fill_log.h"

namespace oceanbase
{
  namespace mergeserver
  {
    ObPacketFactory ObMergeCallback::packet_factory_;
    int ObMergeCallback::process(onev_request_e* r)
    {
      int ret = ONEV_OK;

      if (NULL == r)
      {
        TBSYS_LOG(WARN, "request is empty, r = %p", r);
        ret = ONEV_BREAK;
      }
      else if (NULL == r->ipacket)
      {
        TBSYS_LOG(WARN, "request is empty, r->ipacket = %p", r->ipacket);
        ret = ONEV_BREAK;
      }
      else
      {
        ObMergeServer* server = reinterpret_cast<ObMergeServer*>(r->ms->c->handler->user_data);
        ObPacket* req = reinterpret_cast<ObPacket*>(r->ipacket);
        req->set_request(r);
        //response will be send by handle_request
        if (OB_REQUIRE_HEARTBEAT == req->get_packet_code())
        {
          server->handle_request(req);
          ret = ONEV_OK;
        }
        else
        {
          ret = server->handlePacket(req);
          if (OB_SUCCESS == ret)
          {
            r->ms->c->pool->ref++;
            onev_atomic_inc(&r->ms->pool->ref);
            onev_pool_set_lock(r->ms->pool);
            ret = ONEV_AGAIN;
          }
          else
          {
            ret = ONEV_OK;
            TBSYS_LOG(WARN, "can not push packet(src is %s, pcode is %u) to packet queue", 
                      inet_ntoa_r(r->ms->c->addr), req->get_packet_code());

          }
        }
      }
      return ret;
    }
    
    int ObMergeCallback::rpc_process(onev_request_e* r)
    {
      int ret = ONEV_OK;
      if (NULL == r)
      {
        TBSYS_LOG(WARN, "async response is empty, r = %p", r);
        ret = ONEV_ERROR;
      }
      else if (NULL == r->user_data)
      {
        TBSYS_LOG(WARN, "async response r->user_data = %p", r->user_data);
        ret = ONEV_ERROR;
      }
      else //call handle_packet whatever r->ipacket is null or not
      {
        ObMergerRpcEvent* event = reinterpret_cast<ObMergerRpcEvent*>(r->user_data);
        ObPacket* packet = reinterpret_cast<ObPacket*>(r->ipacket);
        if (NULL == packet)
        {
          TBSYS_LOG(WARN, "r = %p r->ipacket = %p", r, r->ipacket);
        }
        // 这里是libonev网络线程，所以直接将trace id，chid打印出来,忽略profile log头部打印出的东西
        else
        {
          //设置包收到的时间
          packet->set_receive_ts(tbsys::CTimeUtil::getTime());
        }
        ret = event->handle_packet(packet, NULL);
        if (OB_SUCCESS == ret)
        {
          ret = ONEV_OK;
        }
        else
        {
          ret = ONEV_ERROR;
        }
      }
      onev_destroy_session(r->ms);
      return ONEV_OK;
    }

    int ObMergeCallback::sql_process(onev_request_e* r)
    {
      int ret = ONEV_OK;
      if (NULL == r)
      {
        TBSYS_LOG(WARN, "async response is empty, r = %p", r);
        ret = ONEV_ERROR;
      }
      else if (NULL == r->user_data)
      {
        TBSYS_LOG(WARN, "async response r->user_data = %p", r->user_data);
        ret = ONEV_ERROR;
      }
      else //call handle_packet whatever r->ipacket is null or not
      {
        ObMsSqlRpcEvent* event = reinterpret_cast<ObMsSqlRpcEvent*>(r->user_data);
        ObPacket* packet = reinterpret_cast<ObPacket*>(r->ipacket);
        if (NULL == r->ipacket)
        {
          TBSYS_LOG(WARN, "message timeout,r = %p r->ipacket = %p", r, r->ipacket);
        }
        else
        {
          packet->set_receive_ts(tbsys::CTimeUtil::getTime());
          int32_t pcode = packet->get_packet_code();
          if (pcode == OB_SQL_SCAN_RESPONSE || pcode == OB_SQL_GET_RESPONSE)
          {
            event->set_channel_id(packet->get_channel_id());
          }
        }
        ret = event->handle_packet(packet, r->ms->c);
        r->user_data = NULL;
        if (OB_SUCCESS == ret)
        {
          ret = ONEV_OK;
        }
        else
        {
          ret = ONEV_ERROR;
        }
      }
      onev_destroy_session(r->ms);
      return ONEV_OK;
    }
  }//namespace mergeserver
}//namespace oceanbase
