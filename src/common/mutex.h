

#ifndef _FDB_MUTEX_H_
#define _FDB_MUTEX_H_ value

#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct fdb_mutex_t
{
  pthread_mutex_t m_mutex;
} fdb_mutex_t;
  

/**
 * \brief inits a new mutex 
 *
 * \return The newly initd mutex
 */
void
fdb_mutex_init(fdb_mutex_t* mutex);

/**
 * \brief Destroys a mutex
 *
 * \param mutex The mutex to destroy
 */
void
fdb_mutex_release(fdb_mutex_t* mutex);

/**
 * \brief Locks a mutex
 *
 * \param mutex The mutex to lock
 */
void
fdb_mutex_lock(fdb_mutex_t*  mutex);

/**
 * \brief Unlocks a mutex
 *
 * \param mutex The mutex to unlock
 */
void
fdb_mutex_unlock(fdb_mutex_t* mutex);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FURIOUS_MUTEX_H_ */
