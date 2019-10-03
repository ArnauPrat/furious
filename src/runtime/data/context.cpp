


#include "context.h"

namespace furious 
{

Context::Context(float dt, 
                 Database* database, 
                 void* user_data,
                 uint32_t chunk_size,
                 uint32_t thread_id, 
                 uint32_t num_threads) : 
  m_dt(dt), 
  p_user_data(user_data),
  m_chunk_size(chunk_size),
  m_thread_id(thread_id),
  m_num_threads(num_threads),
  p_database(database)
  {
  }

}
