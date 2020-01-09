
#include "mutex.h"
#include <mutex>

namespace furious 
{

mutex_t
mutex_create()
{
  mutex_t mutex;
  mutex.m_mutex_impl = new std::mutex();
  return mutex;
}

void
mutex_destroy(mutex_t* mutex)
{
  if(mutex->m_mutex_impl)
  {
    std::mutex* mutex_impl = (std::mutex*)mutex->m_mutex_impl;
    delete mutex_impl;
  }
}

void
mutex_lock(mutex_t*  mutex)
{
  std::mutex* mutex_impl = (std::mutex*)mutex->m_mutex_impl;
  mutex_impl->lock();
}

void
mutex_unlock(mutex_t* mutex)
{
  std::mutex* mutex_impl = (std::mutex*)mutex->m_mutex_impl;
  mutex_impl->unlock();
}
  
} /* mutex */ 
