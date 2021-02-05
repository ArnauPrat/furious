

#ifndef _FDB_ATOMIC_COUNTER_H_
#define _FDB_ATOMIC_COUNTER_H_

#include "platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fdb_atomic_counter_t 
{
  volatile int32_t m_counter;
};


/**
 * @brief Sets the value of the counter
 *
 * @param value
 */
int32_t 
fdb_atomic_counter_set(struct fdb_atomic_counter_t* counter, 
                   int32_t value);

/**
 * @brief Fetches and Increments the value of the counter
 *
 * @return 
 */
int32_t 
fdb_atomic_counter_fetch_increment(struct fdb_atomic_counter_t* counter);

/**
 * @brief Fetches and Decrements the value of the counter
 */
int32_t 
fdb_atomic_counter_fetch_decrement(struct fdb_atomic_counter_t* counter);

/**
 * \brief Gets the current value of the atomic counter 
 *
 * \param counter The atomic counter to get the value from
 *
 * \return Returns the current value of the atomic counter
 */
int32_t
fdb_atomic_counter_get(struct fdb_atomic_counter_t* counter);

/**
 * @brief Blocks until the counter is set to zero
 */
void 
fdb_atomic_counter_join(struct fdb_atomic_counter_t* counter);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _SMILE_TASKING_SYNC_COUNTER_H_ */
