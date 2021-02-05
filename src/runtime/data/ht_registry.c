
#include "ht_registry.h"
#include "../../common/btree.h"
#include "../../common/memory/memory.h"
#include "../../common/utils.h"
  
void
fdb_htregistry_init(struct fdb_htregistry_t* registry, 
                    struct fdb_mem_allocator_t* allocator)
{
  *registry = (struct fdb_htregistry_t){};
  fdb_btree_factory_init(&registry->m_btree_factory, allocator);
  fdb_btree_init(&registry->m_registry,
                 &registry->m_btree_factory);
  fdb_mutex_init(&registry->m_mutex);
}

void
fdb_htregistry_release(struct fdb_htregistry_t* registry)
{
  fdb_mutex_release(&registry->m_mutex);
  fdb_btree_release(&registry->m_registry);
  fdb_btree_factory_release(&registry->m_btree_factory);
}

void
fdb_htregistry_insert(struct fdb_htregistry_t* registry, 
                      const char* key,
                      void* value)
{
  fdb_mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  struct fdb_btree_insert_t insert = fdb_btree_insert(&registry->m_registry, hash_key, value);
  if(insert.m_inserted == false)
  {
    *insert.p_place = value;
  }
  fdb_mutex_unlock(&registry->m_mutex);
}

void*
fdb_htregistry_get(struct fdb_htregistry_t* registry,
                const char* key)
{
  fdb_mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  void* ptr =  fdb_btree_get(&registry->m_registry,
                         hash_key);
  fdb_mutex_unlock(&registry->m_mutex);
  return ptr;
}

void
fdb_htregistry_remove(struct fdb_htregistry_t* registry, 
                      const char* key)
{
  fdb_mutex_lock(&registry->m_mutex);
  uint32_t hash_key = hash(key);
  fdb_btree_remove(&registry->m_registry, 
                   hash_key);
  fdb_mutex_unlock(&registry->m_mutex);
}
