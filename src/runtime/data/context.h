

#ifndef _FURIOUS_CONTEXT_H_
#define _FURIOUS_CONTEXT_H_ value

#include "../../common/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fdb_context_t 
{
  float                   m_dt;                 
  void*                   p_user_data;
  uint32_t                m_chunk_size;
  uint32_t                m_thread_id;
  uint32_t                m_num_threads;
  struct fdb_database_t*  p_database;
};


void
fdb_context_init(struct fdb_context_t* ctx,
                 float dt, 
                 struct fdb_database_t* database,
                 void* user_data,
                 uint32_t chunk_size,
                 uint32_t thread_id,
                 uint32_t num_threads);

void
fdb_context_release(struct fdb_context_t* ctx);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_CONTEXT_H_ */
