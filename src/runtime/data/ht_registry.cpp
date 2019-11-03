
#include "ht_registry.h"

#include "../../common/impl/btree_impl.h"
#include "../../common/utils.h"

namespace furious
{
  
void
ht_registry_init(ht_registry_t* registry)
{
  registry->p_root = btree_create_root();
}

void
ht_registry_release(ht_registry_t* registry)
{
  btree_destroy_root(registry->p_root);
}

void**
ht_registry_insert(ht_registry_t* registry, 
                   const char* key)
{
  registry->m_mutex.lock();
  uint32_t hash_key = hash(key);
  void** ptr =  btree_insert_root(registry->p_root, 
                                      hash_key).p_place;
  registry->m_mutex.unlock();
  return ptr;
}

void*
ht_registry_get(ht_registry_t* registry,
                const char* key)
{
  registry->m_mutex.lock();
  uint32_t hash_key = hash(key);
  void* ptr =  btree_get_root(registry->p_root,
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
  btree_remove_root(registry->p_root, 
                    hash_key);
  registry->m_mutex.unlock();
}


} /* furious */ 
