

#include "atomic_counter.h"

int32_t 
fdb_atomic_counter_set(struct fdb_atomic_counter_t* counter,
                        int32_t value) 
{
  return __sync_lock_test_and_set(&(counter->m_counter),value);
}

int32_t 
fdb_atomic_counter_fetch_increment(struct fdb_atomic_counter_t* counter) 
{
  return __sync_fetch_and_add(&(counter->m_counter), 1); 
}

int32_t 
fdb_atomic_counter_fetch_decrement(struct fdb_atomic_counter_t* counter)
{
  return __sync_fetch_and_add(&(counter->m_counter), -1); 
}

int32_t 
fdb_atomic_counter_get(struct fdb_atomic_counter_t* counter)
{
  return counter->m_counter;
}

void 
fdb_atomic_counter_join(struct fdb_atomic_counter_t* counter)
{
  while(fdb_atomic_counter_get(counter) > 0)
  {
  }
}
