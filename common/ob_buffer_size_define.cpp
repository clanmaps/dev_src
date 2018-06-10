#include "ob_buffer_size_define.h"
namespace oceanbase {
  namespace common {

    int64_t OB_MAX_LOG_BUFFER_SIZE = OB_DEFAULT_BUFFER_SIZE;  // 1.875MB  //modify by zhutao, add default variable
    int64_t OB_LOG_BUFFER_MAX_SIZE = 1<<21;  //2MB
    int64_t OB_DEFAULT_BLOCK_BITS = 22;
    int32_t OB_MAX_THREAD_BUFFER_SIZE = 1<<21;
    int32_t OB_RPC_BUFFER_SIZE = 1<<21;

    int64_t OB_FLUSH_DISK_BUFFER_SIZE = 1<<22;
  }
}
