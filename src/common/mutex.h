

#ifndef _FURIOUS_MUTEX_H_
#define _FURIOUS_MUTEX_H_ value

namespace furious 
{

typedef void* mutex_handler_t;

struct mutex_t
{
  mutex_handler_t m_mutex_impl;
};
  

/**
 * \brief Creates a new mutex 
 *
 * \return The newly created mutex
 */
mutex_t
mutex_create();

/**
 * \brief Destroys a mutex
 *
 * \param mutex The mutex to destroy
 */
void
mutex_destroy(mutex_t* mutex);

/**
 * \brief Locks a mutex
 *
 * \param mutex The mutex to lock
 */
void
mutex_lock(mutex_t*  mutex);

/**
 * \brief Unlocks a mutex
 *
 * \param mutex The mutex to unlock
 */
void
mutex_unlock(mutex_t* mutex);

} /* tna */ 

#endif /* ifndef _FURIOUS_MUTEX_H_ */
