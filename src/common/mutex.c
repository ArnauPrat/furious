
#include "mutex.h"

void
fdb_mutex_init(fdb_mutex_t* mutex)
{
  pthread_mutex_init(&mutex->m_mutex, NULL);
}

void
fdb_mutex_release(fdb_mutex_t* mutex)
{
  pthread_mutex_destroy(&mutex->m_mutex);
}

void
fdb_mutex_lock(fdb_mutex_t*  mutex)
{
  pthread_mutex_lock(&mutex->m_mutex);
}

void
fdb_mutex_unlock(fdb_mutex_t* mutex)
{
  pthread_mutex_unlock(&mutex->m_mutex);
}
