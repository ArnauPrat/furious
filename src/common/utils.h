

#ifndef _FDB_UTILS_H_
#define _FDB_UTILS_H_ 

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Computes a hash on the given string
 *
 * @param str The string to compute the hash for
 *
 * @return Returns the hash of the given string
 */
uint32_t
hash(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FURIOUS_UTILS_H_ */
