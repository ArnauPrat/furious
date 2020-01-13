

#ifndef _FURIOUS_CONTEXT_H_
#define _FURIOUS_CONTEXT_H_ value

#include "../../common/platform.h"

namespace furious 
{

struct database_t;

struct IdComponentPair
{
  int32_t id;
  int64_t table_id;
};

class Context 
{
public:
  Context(float dt, 
          database_t* database,
          void* user_data,
          uint32_t chunk_size,
          uint32_t thread_id,
          uint32_t num_threads);
  ~Context() = default;

  //template<typename TComponent>
  //  void enable_component(const char* component_name, int32_t id);

  //template<typename TComponent>
  //  void disable_component(const char* component_name, int32_t id);

  const float m_dt;                 
  void*       p_user_data;
  uint32_t    m_chunk_size;
  uint32_t    m_thread_id;
  uint32_t    m_num_threads;

private:
  database_t*     p_database;
};
  
} /* furious */ 

#include "context.inl"
#endif /* ifndef _FURIOUS_CONTEXT_H_ */
