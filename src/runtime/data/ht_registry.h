
#ifndef _FDB_HASHTABLE_REGISTRY_H_
#define _FDB_HASHTABLE_REGISTRY_H_ value

#include "../../common/mutex.h" 
#include "../../common/btree.h" 

#ifdef __cplusplus
extern "C" {
#endif


typedef struct fdb_htregistry_t
{
  fdb_btree_t   m_registry;
  fdb_mutex_t   m_mutex;
} fdb_htregistry_t;

void
fdb_htregistry_init(fdb_htregistry_t* registry, fdb_mem_allocator_t* allocator);

void
fdb_htregistry_release(fdb_htregistry_t* registry);

void
fdb_htregistry_insert(fdb_htregistry_t* registry, 
                   const char* key, 
                   void* value);

void*
fdb_htregistry_get(fdb_htregistry_t* registry,
                const char* key);

void
fdb_htregistry_remove(fdb_htregistry_t* registyr, 
                   const char* key);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FDB_HASHTABLE_REGISTRY_H_ */
