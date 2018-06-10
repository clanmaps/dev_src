/*
 * Copyright (C) 2007-2012 Taobao Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Description here
 *
 * Version: $Id$
 *
 * Authors:
 *   Zhifeng YANG <zhuweng.yzf@taobao.com>
 *     - some work details here
 */

#include "ob_base_client.h"
using namespace oceanbase::common;

ObBaseClient::ObBaseClient()
  :init_(false), eio_(NULL)
{
}

ObBaseClient::~ObBaseClient()
{
  this->destroy();
}

int ObBaseClient::initialize(const ObServer& server)
{
  int ret = OB_ERROR;
  int rc = ONEV_OK;
  if (init_)
  {
    TBSYS_LOG(WARN, "already init");
    ret = OB_INIT_TWICE;
  }
  else
  {
    server_ = server;
    //create io thread
    eio_ = onev_create_io(eio_, 1);
    eio_->do_signal = 0;
    eio_->force_destroy_second = OB_CONNECTION_FREE_TIME_S;
    eio_->checkdrc = 1;
    if (NULL == eio_)
    {
      ret = OB_ERROR;
      TBSYS_LOG(ERROR, "onev_io_create error");
    }
    memset(&client_handler_, 0, sizeof(onev_io_handler_pe));
    client_handler_.encode = ObTbnetCallback::encode;
    client_handler_.decode = ObTbnetCallback::decode;
    client_handler_.get_packet_id = ObTbnetCallback::get_packet_id;
    client_handler_.on_disconnect = ObTbnetCallback::on_disconnect;
    if (OB_SUCCESS != (ret = client_mgr_.initialize(eio_, &client_handler_)))
    {
      TBSYS_LOG(ERROR, "failed to init client_mgr, err=%d", ret);
    }
    else
    {
      //start io thread
      if (ret == OB_SUCCESS)
      {
        rc = onev_start_io(eio_);
        if (ONEV_OK == rc)
        {
          ret = OB_SUCCESS;
          TBSYS_LOG(INFO, "start io thread");
        }
        else
        {
          TBSYS_LOG(ERROR, "onev_start_io failed");
          ret = OB_ERROR;
        }
      }
      if (OB_SUCCESS == ret)
      {
        init_ = true;
      }
    }
  }
  return ret;
}

void ObBaseClient::destroy()
{
  if (init_)
  {
    onev_stop_io(eio_);
    onev_wait_io(eio_);
    onev_destroy_io(eio_);
    init_ = false;
  }
  TBSYS_LOG(INFO, "client stoped");
}

int ObBaseClient::send_recv(const int32_t pcode, const int32_t version, const int64_t timeout,
                            ObDataBuffer& message_buff)
{
  return client_mgr_.send_request(server_, pcode, version, timeout, message_buff);
}
