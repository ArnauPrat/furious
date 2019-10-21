

#ifndef _FURIOUS_PLATFORM_H_
#define _FURIOUS_PLATFORM_H_

#include "types.h"
#include <assert.h>
#include <stdlib.h>

#define FURIOUS_ASSERT(_cond) assert(_cond);
#define FURIOUS_PERMA_ASSERT(_cond) if(!(_cond)) abort();

#define FURIOUS_COPY_AND_CHECK_STR(dest, origin, capacity) \
        strncpy(dest, origin, capacity);\
        FURIOUS_PERMA_ASSERT( strlen(origin) < capacity && "String exceeds maximum capacity");
    

namespace furious 
{

constexpr uint32_t MAX_TYPE_NAME = 256;
constexpr uint32_t MAX_QUALIFIED_TYPE_NAME=MAX_TYPE_NAME+32;
constexpr uint32_t MAX_FIELD_NAME = 256;
constexpr uint32_t MAX_REF_NAME = 256;
constexpr uint32_t MAX_TAG_NAME = 256;
constexpr uint32_t MAX_TABLE_VARNAME=MAX_TYPE_NAME+32;
constexpr uint32_t MAX_REF_TABLE_VARNAME=MAX_TYPE_NAME+32;
constexpr uint32_t MAX_TAG_TABLE_VARNAME=MAX_TYPE_NAME+32;
constexpr uint32_t MAX_SYSTEM_WRAPPER_VARNAME=MAX_TYPE_NAME+32;
constexpr uint32_t MAX_HASHTABLE_VARNAME=256;
constexpr uint32_t MAX_CLUSTER_VARNAME=256;
constexpr uint32_t MAX_ITER_VARNAME=256;
constexpr uint32_t MAX_BLOCK_VARNAME=256;
constexpr uint32_t MAX_INCLUDE_PATH_LENGTH=2048;

class Platform {

  Platform() = delete;
  ~Platform() = delete;

public: 
  /** Platform dependant values **/

};
}

#endif
