
#include "ht_registry.h"
#include "../../common/btree.h"
#include "../../common/memory/memory.h"
#include "../../common/utils.h"

namespace furious
{
  
void
ht_registry_init(ht_registry_t* registry)
{
  registry->m_registry = btree_create();
  registry->m_mutex = mutex_create();
}

void
ht_registry_release(ht_registry_t* registry)
{
  mutex_destroy(&registry->m_mutex);
  btree_destroy(&registry->m_registry);
}

void
ht_registry_insert(ht_registry_t* registry, 
                   const char* key,
                   void* value)
{
  mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  btree_insert_t insert = btree_insert(&registry->m_registry, hash_key, value);
  if(insert.m_inserted == false)
  {
    *insert.p_place = value;
  }
  mutex_unlock(&registry->m_mutex);
}

void*
ht_registry_get(ht_registry_t* registry,
                const char* key)
{
  mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  void* ptr =  btree_get(&registry->m_registry,
                         hash_key);
  mutex_unlock(&registry->m_mutex);
  return ptr;
}

void
ht_registry_remove(ht_registry_t* registry, 
                   const char* key)
{
  mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  btree_remove(&registry->m_registry, 
               hash_key);
  mutex_unlock(&registry->m_mutex);
}


} /* furious */ 
