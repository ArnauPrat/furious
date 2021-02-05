


#include "context.h"

void
fdb_context_init(struct fdb_context_t* ctx,
                 float dt, 
                 struct fdb_database_t* database, 
                 void* user_data,
                 uint32_t chunk_size,
                 uint32_t thread_id, 
                 uint32_t num_threads)
{
  ctx->m_dt = dt; 
  ctx->p_user_data = user_data;
  ctx->m_chunk_size = chunk_size;
  ctx->m_thread_id = thread_id;
  ctx->m_num_threads = num_threads;
  ctx->p_database = database;
}

void
fdb_context_release(struct fdb_context_t* ctx)
{
}
