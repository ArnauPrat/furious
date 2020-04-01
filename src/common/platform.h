

#ifndef _FDB_PLATFORM_H_
#define _FDB_PLATFORM_H_

#include "types.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


#ifdef FDB_ENABLE_ASSERTS
#define FDB_ASSERT(_cond) if(!(_cond)) abort();
#else
#define FDB_ASSERT(_cond)
#endif

#define FDB_PERMA_ASSERT(_cond) if(!(_cond)) { \
    printf(#_cond);\
    abort();\
  }

#define FDB_COPY_AND_CHECK_STR(dest, origin, capacity) \
        strncpy(dest, origin, capacity);\
        FDB_PERMA_ASSERT(strlen(origin) < capacity && "String exceeds maximum capacity");
    
#define FDB_RESTRICT(type) type  __restrict__
#define FDB_ALIGNED(type, name, alignment) type __attribute__((aligned(alignment))) name

/// RUNTIME


#define FDB_MIN_ALIGNMENT                     16

#define FDB_BTREE_ALIGNMENT                   64 
#define FDB_BTREE_PAGE_SIZE                   KILOBYTES(4)

#define FDB_BITMAP_ALIGNMENT                  64 
#define FDB_BITMAP_PAGE_SIZE                  KILOBYTES(4)
#define FDB_BITMAP_DATA_ALIGNMENT             64 
#define FDB_BITMAP_DATA_PAGE_SIZE             KILOBYTES(4)

#define FDB_TABLE_BLOCK_ALIGNMENT             64
#define FDB_TABLE_BLOCK_DATA_ALIGNMENT        64

#define FDB_TABLE_BLOCK_PAGE_SIZE             KILOBYTES(4)
#define FDB_TABLE_BLOCK_DATA_PAGE_SIZE        KILOBYTES(64)

#define FDB_DATABASE_TABLE_ALIGNMENT          64
#define FDB_DATABASE_BITTABLE_ALIGNMENT       64 
#define FDB_DATABASE_GLOBAL_ALIGNMENT         64 
#define FDB_DATABASE_TABLE_PAGE_SIZE          KILOBYTES(4)
#define FDB_DATABASE_BITTABLE_PAGE_SIZE       KILOBYTES(4)
#define FDB_DATABASE_GLOBAL_PAGE_SIZE         KILOBYTES(4)

#define FDB_INVALID_ID                        0xffffffff
#define FDB_INVALID_TABLE_ID                  FDB_INVALID_ID 
#define FDB_TABLE_BLOCK_SIZE                  256
#define FDB_MAX_TABLE_NAME                    256

#define FDB_MAX_COMPONENT_FIELDS              32

#define FDB_REFLECTION_STRUCT_PAGE_SIZE       KILOBYTES(4)
#define FDB_REFLECTION_FIELD_PAGE_SIZE        KILOBYTES(4)

#define FDB_MAX_WEBSERVER_PORT_SIZE           64
#define FDB_MAX_WEBSERVER_ADDRESS_SIZE        64
/// FCC_COMPILER 

#define FCC_INVALID_ID                            FDB_INVALID_ID
#define FCC_MAX_TYPE_NAME                         FDB_MAX_TABLE_NAME
#define FCC_MAX_QUALIFIED_TYPE_NAME               FCC_MAX_TYPE_NAME+32
#define FCC_MAX_FIELD_NAME                        256
#define FCC_MAX_REF_NAME                          256
#define FCC_MAX_TAG_NAME                          256
#define FCC_MAX_TABLE_VARNAME                     FCC_MAX_TYPE_NAME+32
#define FCC_MAX_REF_TABLE_VARNAME                 FCC_MAX_TYPE_NAME+32
#define FCC_MAX_TAG_TABLE_VARNAME                 FCC_MAX_TYPE_NAME+32
#define FCC_MAX_SYSTEM_WRAPPER_VARNAME            FCC_MAX_TYPE_NAME+32
#define FCC_MAX_HASHTABLE_VARNAME                 256
#define FCC_MAX_CLUSTER_VARNAME                   256
#define FCC_MAX_ITER_VARNAME                      256
#define FCC_MAX_BLOCK_VARNAME                     256
#define FCC_MAX_INCLUDE_PATH_LENGTH               2048
#define FCC_MAX_OPERATOR_NAME                     32
#define FCC_MAX_OPERATOR_NUM_COLUMNS              16
#define FCC_MAX_SUBPLAN_NODES                     32
#define FCC_MAX_TASK_PARENTS                      FCC_MAX_OPERATOR_NUM_COLUMNS
#define FCC_MAX_TASK_CHILDREN                     FCC_MAX_OPERATOR_NUM_COLUMNS

#define FCC_MAX_CTOR_PARAMS                       16
#define FCC_MAX_SYSTEM_COMPONENTS                 FCC_MAX_OPERATOR_NUM_COLUMNS
#define FCC_MAX_HAS_COMPONENTS                    FCC_MAX_OPERATOR_NUM_COLUMNS
#define FCC_MAX_HAS_NOT_COMPONENTS                FCC_MAX_HAS_COMPONENTS
#define FCC_MAX_HAS_TAGS                          16 
#define FCC_MAX_HAS_NOT_TAGS                      FCC_MAX_HAS_TAGS
#define FCC_MAX_FILTER_FUNC                       8 

/**
 * \brief Returns the index (starting from 1) of the least significant bit of the given
 * number
 *
 * \param x The number to get the index for 
 *
 * \return The index of the least significant bit
 */
uint32_t
fdb_os_pow2_bit(uint64_t x);

#endif
