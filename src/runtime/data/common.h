
#ifndef _FURIOUS_DATA_COMMON_H_
#define _FURIOUS_DATA_COMMON_H_

#include "../../common/common.h"

namespace furious {

using entity_id_t = uint32_t;
using table_id_t  = int32_t;

constexpr table_id_t INVALID_TABLE_ID = 0xffffffff;

/**
 * \brief The number of components per block.
 */
constexpr uint32_t FURIOUS_TABLE_BLOCK_SIZE = 256;

constexpr entity_id_t FURIOUS_INVALID_ID = 0xffffffff;

} /* furious */ 

#endif
