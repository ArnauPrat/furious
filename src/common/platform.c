

#include "platform.h"

uint32_t
fdb_os_pow2_bit(uint64_t x)
{
#ifdef FDB_COMPILER_GCC
return __builtin_ffs(x);
#endif
}

void
fdb_mem_barrier()
{
#ifdef FDB_COMPILER_GCC
      __sync_synchronize();
#endif
}


uint32_t
fdb_block_get_offset(uint32_t block_id, 
                 uint32_t chunk_size,
                 uint32_t stride)
{
  uint32_t chunk_id = block_id / chunk_size;
  return chunk_id % stride;
}
