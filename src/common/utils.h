

#ifndef _FURIOUS_UTILS_H_
#define _FURIOUS_UTILS_H_ value

#include "types.h"

namespace furious 
{

/**
 * @brief Computes a hash on the given string
 *
 * @param str The string to compute the hash for
 *
 * @return Returns the hash of the given string
 */
uint32_t
hash(const char* str);

}


#endif /* ifndef _FURIOUS_UTILS_H_ */
