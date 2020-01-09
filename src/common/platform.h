

#ifndef _FURIOUS_PLATFORM_H_
#define _FURIOUS_PLATFORM_H_

#include "types.h"
#include <assert.h>
#include <stdlib.h>

#ifdef FURIOUS_ENABLE_ASSERTS
#define FURIOUS_ASSERT(_cond) if(!(_cond)) abort();
#else
#define FURIOUS_ASSERT(_cond)
#endif

#define FURIOUS_PERMA_ASSERT(_cond) if(!(_cond)) abort();

#define FURIOUS_COPY_AND_CHECK_STR(dest, origin, capacity) \
        strncpy(dest, origin, capacity);\
        FURIOUS_PERMA_ASSERT( strlen(origin) < capacity && "String exceeds maximum capacity");
    
#define FURIOUS_RESTRICT(type) type  __restrict__
#define FURIOUS_ALIGNED(type, name, alignment) type __attribute__((aligned(alignment))) name



/// RUNTIME
#define FURIOUS_MIN_ALIGNMENT 16
#define FURIOUS_TBLOCK_ALIGNMENT 64
#define FURIOUS_TBLOCK_DATA_ALIGNMENT 64

#define FURIOUS_TBLOCK_PAGE_SIZE KILOBYTES(4)
#define FURIOUS_TBLOCK_DATA_PAGE_SIZE KILOBYTES(64)
#define FURIOUS_TBLOCK_BITMAP_PAGE_SIZE KILOBYTES(4)
#define FURIOUS_TABLE_BTREE_PAGE_SIZE KILOBYTES(4)

#define FURIOUS_INVALID_TABLE_ID 0xffffffff
#define FURIOUS_INVALID_ID  0xffffffff
#define FURIOUS_TABLE_BLOCK_SIZE 256

/// FCC_COMPILER 
#define FCC_MAX_TYPE_NAME 256
#define FCC_MAX_QUALIFIED_TYPE_NAME FCC_MAX_TYPE_NAME+32
#define FCC_MAX_FIELD_NAME 256
#define FCC_MAX_REF_NAME 256
#define FCC_MAX_TAG_NAME 256
#define FCC_MAX_TABLE_VARNAME FCC_MAX_TYPE_NAME+32
#define FCC_MAX_REF_TABLE_VARNAME FCC_MAX_TYPE_NAME+32
#define FCC_MAX_TAG_TABLE_VARNAME FCC_MAX_TYPE_NAME+32
#define FCC_MAX_SYSTEM_WRAPPER_VARNAME FCC_MAX_TYPE_NAME+32
#define FCC_MAX_HASHTABLE_VARNAME 256
#define FCC_MAX_CLUSTER_VARNAME 256
#define FCC_MAX_ITER_VARNAME 256
#define FCC_MAX_BLOCK_VARNAME 256
#define FCC_MAX_INCLUDE_PATH_LENGTH 2048



#endif
