
#include "ht_registry.h"
#include "../../common/memory/memory.h"

#include "../../common/impl/btree_impl.h"
#include "../../common/utils.h"

namespace furious
{
  
void
ht_registry_init(ht_registry_t* registry)
{
  registry->p_root = (btree_t*) mem_alloc(&global_mem_allocator, 
                                          1, sizeof(btree_t), -1);
  *registry->p_root = btree_create();
}

void
ht_registry_release(ht_registry_t* registry)
{
  btree_destroy(registry->p_root);
  mem_free(&global_mem_allocator, 
           registry->p_root);
}

void
ht_registry_insert(ht_registry_t* registry, 
                   const char* key,
                   void* value)
{
  registry->m_mutex.lock();
  uint32_t hash_key = hash(key);
  void** ptr =  btree_insert(registry->p_root, 
                                      hash_key).p_place;
  *ptr = value;
  registry->m_mutex.unlock();
}

void*
ht_registry_get(ht_registry_t* registry,
                const char* key)
{
  registry->m_mutex.lock();
  uint32_t hash_key = hash(key);
  void* ptr =  btree_get(registry->p_root,
                        hash_key);
  registry->m_mutex.unlock();
  return ptr;
}

void
ht_registry_remove(ht_registry_t* registry, 
                   const char* key)
{
  registry->m_mutex.lock();
  uint32_t hash_key = hash(key);
  btree_remove(registry->p_root, 
                    hash_key);
  registry->m_mutex.unlock();
}


} /* furious */ 
