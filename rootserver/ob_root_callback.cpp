#include "ob_root_callback.h"
#include "tblog.h"
#include "rootserver/ob_root_worker.h"
#include "onev_struct.h"
#include "common/ob_packet.h"

using namespace oceanbase::common;
namespace oceanbase
{
  namespace rootserver
  {
    int ObRootCallback::process(onev_request_e *r)
    {
      int ret = ONEV_OK;
      if (NULL == r || NULL == r->ipacket)
      {
        char buff[32];
        onev_addr_e addr = r->ms->c->addr;
        if (NULL == r)
        {
          TBSYS_LOG(ERROR, "request is empty, r = %p", r);
        }
        else
        {
          TBSYS_LOG(ERROR, "request is empty, r->ipacket = %p", r->ipacket);
        }
        TBSYS_LOG(ERROR, "receive packet from server:%s faild", onev_inet_addr_to_str(&addr, buff, 32));
        ret = ONEV_BREAK;
      }
      else
      {
        ObRootWorker *worker = (ObRootWorker* )r->ms->c->handler->user_data;
        ObPacket* req = (ObPacket*) r->ipacket;
        TBSYS_LOG(DEBUG, "handle packet code is %d", req->get_packet_code());
        req->set_request(r);
        //handlePacket will handle those packet can not distributed to queue
        ret = worker->handlePacket(req);
        if (OB_SUCCESS == ret)
        {
          r->ms->c->pool->ref ++;
          onev_atomic_inc(&r->ms->pool->ref);
          onev_pool_set_lock(r->ms->pool);
          ret = ONEV_AGAIN;

        }
        else
        {
          TBSYS_LOG(WARN, "can not push packet(src is %s, pcode is %u) to packet queue", 
                    inet_ntoa_r(r->ms->c->addr), req->get_packet_code());
          ret = ONEV_OK;
        }
      }
      return ret;
    }
  }//namespace rootserver
}//namespace oceanbase

