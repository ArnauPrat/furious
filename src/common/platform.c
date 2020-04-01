

#include "platform.h"

uint32_t
fdb_os_pow2_bit(uint64_t x)
{
#ifdef FDB_COMPILER_GCC
return __builtin_ffs(x);
#endif
}
