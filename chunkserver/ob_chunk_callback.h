/*
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation.
 *
 * ob_chunk_callback.h is for what ...
 *
 * Version: ***: ob_chunk_callback.h  Tue May 22 11:25:49 2012 fangji.hcm Exp $
 *
 * Authors:
 *   Author fangji
 *   Email: fangji.hcm@taobao.com
 *     -some work detail if you want 
 *
 */

#ifndef OB_CHUNK_CALLBACK_H_
#define OB_CHUNK_CALLBACK_H_

#include "onev_struct.h"

namespace oceanbase
{
  namespace chunkserver
  {
    class ObChunkCallback
    {
    public:
      static int process(onev_request_e* req);
    };
  }
}
#endif
