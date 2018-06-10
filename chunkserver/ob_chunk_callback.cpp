#include "ob_chunk_callback.h"
#include "tblog.h"
#include "ob_chunk_server.h"
#include "common/ob_packet.h"

using namespace oceanbase::common;
namespace oceanbase
{
  namespace chunkserver
  {
    int ObChunkCallback::process(onev_request_e *r)
    {
      int ret = ONEV_OK;

      if (NULL == r)
      {        
        TBSYS_LOG(WARN, "request is NULL, r = %p", r);
        ret = ONEV_BREAK;
      }
      else if (NULL == r->ipacket)
      {
        TBSYS_LOG(WARN, "request is NULL, r->ipacket = %p", r->ipacket);
        ret = ONEV_BREAK;
      }
      else
      {
        ObChunkServer* server = (ObChunkServer*)r->ms->c->handler->user_data;
        ObPacket* packet = (ObPacket*)r->ipacket;
        packet->set_request(r);
        //handle_request will send response
        if (OB_REQUIRE_HEARTBEAT == packet->get_packet_code())
        {
          server->handle_request(packet);
          ret = ONEV_OK;
        }
        else
        {
          ret = server->handlePacket(packet);
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
                      inet_ntoa_r(r->ms->c->addr), packet->get_packet_code());

          }
        }
      }
      return ret;
    }
  }
}
