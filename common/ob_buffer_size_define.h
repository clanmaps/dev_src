

#ifndef OB_BUFFER_SIZE_DEFINE_H
#define OB_BUFFER_SIZE_DEFINE_H

#include "stdint.h"

//add by zhutao
#define OB_DEFAULT_BUFFER_SIZE 1966080L
//add :e
namespace oceanbase {
  namespace common {
    extern int64_t OB_MAX_LOG_BUFFER_SIZE;
    extern int64_t OB_LOG_BUFFER_MAX_SIZE;
    extern int64_t OB_DEFAULT_BLOCK_BITS;
    extern int32_t OB_MAX_THREAD_BUFFER_SIZE;
    extern int32_t OB_RPC_BUFFER_SIZE;
    extern int64_t OB_FLUSH_DISK_BUFFER_SIZE;
    extern int64_t OB_ALLOCATOR_PAGE_SIZE;
    extern int64_t OB_ALLOCATOR_TOTAL_LIMIT;
    extern int64_t OB_ALLOCATOR_HOLD_LIMIT;
  }
}


#endif // OB_BUFFER_SIZE_DEFINE_H
