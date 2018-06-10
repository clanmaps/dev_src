#include "ob_tbnet_callback.h"
#include "data_buffer.h"
#include "tblog.h"
#include "ob_define.h"
#include "onev_io.h"
#include "onev_define.h"
#include "ob_single_server.h"
#include <byteswap.h>
#include "utility.h"
#include "ob_shadow_server.h"
#include "ob_trace_id.h"
#include "ob_profile_log.h"
#include "ob_common_stat.h"
namespace oceanbase
{
  namespace common
  {

    //decode packet from networ using m->pool memory for packet
    //m->pool will destroyed when request done
    void* ObTbnetCallback::decode(onev_message_e *m)
    {
      int flag = 0;
      int pcode = 0;
      int packetlen = 0;
      int datalen = 0;
      int len = 0;
      int buflen = 0;
      uint32_t chid = 0;
      uint16_t ob_packet_header_size = 0;
      int32_t api_version = 0;
      int32_t timeout = 0;
      int64_t session_id = 0;
      uint64_t trace_id = 0;
      uint64_t req_sign = 0;
      ObDataBuffer* buffer = NULL;
      ObPacket* packet = NULL;
      char* buff = NULL;

      if (NULL == m || NULL == m->input)
      {
        TBSYS_LOG(ERROR, "invalid argument m is %p", m);
      }
      else if (NULL == m->input)
      {
        TBSYS_LOG(ERROR, "invalid argument m->input is %p", m->input);
      }
      else
      {
        //check tbnet header readed
        if ((len = static_cast<int>(m->input->last - m->input->pos)) >= OB_TBNET_HEADER_LENGTH)
        {
          //decode member from m->input buffer
          flag = bswap_32(*((uint32_t *)m->input->pos));
          chid = bswap_32(*((uint32_t *)(m->input->pos + 4)));
          pcode = bswap_32(*((uint32_t *)(m->input->pos + 8)));
          packetlen = bswap_32(*((uint32_t *)(m->input->pos + 12)));

          //check tbnet flag and packet len
          if (OB_TBNET_PACKET_FLAG != flag || packetlen <= 0 || packetlen > 0x7fffffff)  //modify by qx 20161229: 64MB -> (2GB-1)
          {
            TBSYS_LOG(ERROR, "tbnet flag:%x<>%x, datalen:%d, peer is %s", flag, OB_TBNET_PACKET_FLAG,
                      packetlen, inet_ntoa_r(m->c->addr));
            m->status = ONEV_ERROR;
          }
          else
          {
            len -= OB_TBNET_HEADER_LENGTH;

            if (len < packetlen)
            {
              m->next_read_len = packetlen - len;
              //TBSYS_LOG(WARN, "not enough data in socket packetlen is %d, data in socket is %d", packetlen, len);
            }
            else
            {
              //add rpc bytes in statistics
              OB_STAT_INC(COMMON, RPC_BYTES_IN, packetlen + OB_TBNET_HEADER_LENGTH);
              int64_t header_size = 0;
              m->input->pos += OB_TBNET_HEADER_LENGTH;
              uint32_t tmp = 0;
              //太恶心了，bswap_16 用不了
              tmp = bswap_32(*reinterpret_cast<int32_t*>(m->input->pos));
              ob_packet_header_size = static_cast<uint16_t>(tmp >> 16);
              api_version = 0x0000FFFF & tmp;
              session_id = bswap_64(*reinterpret_cast<uint64_t *>(m->input->pos + 4));
              timeout = bswap_32(*reinterpret_cast<uint32_t *>(m->input->pos + 12));
#if !defined(_OB_VERSION) || _OB_VERSION<=300
              // 0.3 server 接到了0.3 的包
              if (0 == ob_packet_header_size)
              {
                /* 0.3 ObPacket 头格式: 16 bytes */
                /*         2 bytes                2 bytes         8 bytes     4 bytes
                 *----------------------------------------------------------------------
                 *| ob_packet_header_size_== 0 | api_version_ | session_id_ | timeout_ |
                 *----------------------------------------------------------------------
                */
                header_size = sizeof(ob_packet_header_size) + sizeof(int16_t)/* api_version_ */ + sizeof(session_id) + sizeof(timeout);
              }
              // 0.3 server 接到了 0.4 的包
              else
              {
                /* 0.4 的ObPacket 头格式: 32 bytes */
                /*         2 bytes            2 bytes         8 bytes     4 bytes   8 bytes   8 bytes
                 *--------------------------------------------------------------------------------------
                 *| ob_packet_header_size_ | api_version_ | session_id_ | timeout_ |trace_id_|req_sign_|
                 *--------------------------------------------------------------------------------------
                */
                trace_id = bswap_64(*reinterpret_cast<uint64_t *>(m->input->pos + 16));
                req_sign = bswap_64(*reinterpret_cast<uint64_t *>(m->input->pos + 16 + 8));
                UNUSED(trace_id);
                UNUSED(req_sign);
                header_size = ob_packet_header_size;
              }
#elif _OB_VERSION>300
              // 0.4 server 接到了 0.3 的包
              if (0 == ob_packet_header_size)
              {
                header_size = sizeof(ob_packet_header_size) + sizeof(int16_t)/* api_version_ */ + sizeof(session_id) + sizeof(timeout);
              }
              // 0.4 server 接到了 0.4 的包
              else
              {
                trace_id = bswap_64(*reinterpret_cast<uint64_t *>(m->input->pos + 16));
                req_sign = bswap_64(*reinterpret_cast<uint64_t *>(m->input->pos + 16 + 8));
                header_size = ob_packet_header_size;
              }
#endif
              buflen = packetlen - static_cast<int>(header_size);
              //alloc mem from m->pool
              buff = reinterpret_cast<char*>(onev_pool_alloc(m->pool,
                                                              static_cast<uint32_t>(sizeof(ObPacket) + buflen)));
              //alloc mem failed just set libonev message status to ONEV_ERROR will destroy current connection
              if (NULL == buff)
              {
                TBSYS_LOG(WARN, "alloc packet buffer from m->pool failed");
                m->status = ONEV_ERROR;
                m->input->pos -= OB_TBNET_HEADER_LENGTH;
              }
              else
              {
                packet = new(buff)ObPacket();
                // inner buffer
                packet->set_packet_buffer(buff + sizeof(ObPacket), buflen);

                packet->set_channel_id(chid);
                packet->set_packet_code(pcode);
                packet->set_packet_len(packetlen);

                packet->set_api_version(api_version);
                packet->set_session_id(session_id);
                packet->set_source_timeout(timeout);
                packet->set_ob_packet_header_size(ob_packet_header_size);
#if !defined(_OB_VERSION) || _OB_VERSION<=300
                // nothing
#elif _OB_VERSION>300
                // 0.4 server 接到了 0.4 的包
                if (ob_packet_header_size > 0)
                {
                  packet->set_trace_id(trace_id);
                  packet->set_req_sign(req_sign);
                }
#endif
                m->input->pos += header_size;

                datalen = buflen - OB_RECORD_HEADER_LENGTH;
                packet->set_data_length(datalen);

                buffer = packet->get_inner_buffer();
                if (buffer->get_remain() >= buflen)
                {
                  memcpy(buffer->get_data(), m->input->pos, buflen);
                  m->input->pos += buflen;
                  buffer->get_position() = buflen;
                  int64_t recv_time = tbsys::CTimeUtil::getTime();
                  packet->set_receive_ts(recv_time);
                  //deserialize record header and do sum check
                  int err = packet->deserialize();
                  //deserialize packet failed dataflow error close current connection
                  if (OB_SUCCESS != err)
                  {
                    m->status = ONEV_ERROR;
                    TBSYS_LOG(ERROR, "deserialize packet failed packet channel is %d packet code is %d, err=%d", packet->get_channel_id(),
                              packet->get_packet_code(), err);
                    packet = NULL;
                  }
                  else
                  {
                    TBSYS_LOG(DEBUG, "decode packet from network packet channel is %d packet code is %d, err=%d", packet->get_channel_id(),
                            packet->get_packet_code(), err);
                  }
                }
                else
                {
                  //buff not enough
                  m->input->pos += buflen;
                  TBSYS_LOG(WARN, "inner buffer is not enough, need: %d, real: %ld skip it", buflen, buffer->get_remain());
                }
              }
            }
          }
        }
      }
      return packet;
    }

    int ObTbnetCallback::encode(onev_request_e *r, void *data)
    {
      int ret = ONEV_OK;
      onev_buf_e* b = NULL;
      char* buff = NULL;
      ObPacket* packet = reinterpret_cast<ObPacket*>(data);
      int64_t header_size = 0;
      uint16_t ob_packet_header_size = 0;
#if !defined(_OB_VERSION) || _OB_VERSION<=300
      /* 0.3 ObPacket 头格式: 16 bytes */
      /*         2 bytes                 2 bytes       8 bytes     4 bytes
       *----------------------------------------------------------------------
       *| ob_packet_header_size_== 0 | api_version_ | session_id_ | timeout_ |
       *----------------------------------------------------------------------
       */
      ob_packet_header_size = 0;
      header_size = sizeof(uint16_t)/* ob_packet_header_size_ */ + sizeof(int16_t) /* api_version_ */+ sizeof(int64_t) /* session_id_ */+ sizeof(int32_t)/* timeout_ */;
#elif _OB_VERSION>300
      /* 0.4 ObPacket 头格式： 24 bytes */
      /*         2 bytes            2 bytes       8 bytes     4 bytes   8 bytes     8 bytes
       *--------------------------------------------------------------------------------------
       *| ob_packet_header_size_ | api_version_ | session_id_ | timeout_ |trace_id_|req_sign_|
       *--------------------------------------------------------------------------------------
       */
      ob_packet_header_size = sizeof(ob_packet_header_size) + sizeof(int16_t)/* api_version_ */ + sizeof(int64_t) /* session_id_ */ + sizeof(int32_t)/* timeout_ */ + sizeof(uint64_t)/* trace_id_ */ + sizeof(uint64_t) /* req_sign_ */;
      header_size = ob_packet_header_size;
#endif
      packet->set_ob_packet_header_size(ob_packet_header_size);
      int64_t size = packet->get_inner_buffer()->get_position() + OB_TBNET_HEADER_LENGTH + header_size;
      b = reinterpret_cast<onev_buf_e *>(onev_pool_alloc(r->ms->pool,
                                                          static_cast<uint32_t>(sizeof(onev_buf_e) + size)));
      if (NULL == b)
      {
        TBSYS_LOG(WARN, "alloc mem for send buffer failed buf=%p size is %lu", b,
                  sizeof(onev_buf_e) + size);
        ret = ONEV_ERROR;
      }
      else
      {
        //skip sizeof(onev_buf_e) bytes
        buff = reinterpret_cast<char *>(b + 1);
        //b->pos = buff;
        //b->end = buff + size;
        //set packet length
        init_onev_buf(b, buff, r, size);
        packet->set_packet_len(static_cast<int>(
                                 packet->get_inner_buffer()->get_position() + header_size));

        if (false == packet->encode(buff, size))
        {
          ret = ONEV_ERROR;
          TBSYS_LOG(WARN, "encode failed packet is %p, buff is %p, size is %ld", packet, buff, size);
        }

        if (ONEV_OK == ret)
        {
          onev_buf_set_data(r->ms->pool, b, buff, static_cast<uint32_t>(size));
          onev_request_addbuf(r, b);
          //add rpc bytes out statistics
          OB_STAT_INC(COMMON, RPC_BYTES_OUT, size);
          TBSYS_LOG(DEBUG, "encode packet success packet code is %d, c is %s",packet->get_packet_code(),
                    onev_connection_to_str(r->ms->c));
        }
      }

      return ret;
    }

    int ObTbnetCallback::on_disconnect(onev_connection_e* c)
    {
      int ret = ONEV_OK;
      if (NULL == c)
      {
        TBSYS_LOG(WARN, "invalid argument c is %p", c);
        ret = ONEV_ERROR;
      }
      else
      {
        if (c->reconn_fail > 10)
        {
          c->auto_reconn = 0;
        }
      }
      return ret;
    }

    int ObTbnetCallback::batch_process(onev_message_e *m)
    {
      UNUSED(m);
      return ONEV_OK;
    }

    uint64_t ObTbnetCallback::get_packet_id(onev_connection_e *c, void *packet)
    {
      UNUSED(c);
      return ((ObPacket*)packet)->get_channel_id();
    }

    int ObTbnetCallback::default_callback(onev_request_e* r)
    {
      int ret = ONEV_OK;
      if (NULL == r || NULL == r->ms)
      {
        TBSYS_LOG(WARN, "request is null or r->ms is null");
      }
      else
      {
        onev_destroy_session(r->ms);
      }
      return ret;
    }

    int ObTbnetCallback::shadow_process(onev_request_e* r)
    {
      int ret = ONEV_OK;
      if (NULL == r || NULL == r->ipacket)
      {
        TBSYS_LOG(ERROR, "request is empty, r = %p, r->ipacket = %p", r, r->ipacket);
        ret = ONEV_BREAK;
      }
      else
      {
        ObShadowServer *server = (ObShadowServer*)r->ms->c->handler->user_data;
        ObPacket *req = (ObPacket*) r->ipacket;
        req->set_request(r);
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
      return ret;
    }
  }
}
