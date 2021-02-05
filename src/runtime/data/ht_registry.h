
#ifndef _FDB_HASHTABLE_REGISTRY_H_
#define _FDB_HASHTABLE_REGISTRY_H_

#include "../../common/mutex.h" 
#include "../../common/btree.h" 

#ifdef __cplusplus
extern "C" {
#endif


struct fdb_htregistry_t
{
  struct fdb_btree_factory_t  m_btree_factory;
  struct fdb_btree_t          m_registry;
  struct fdb_mutex_t          m_mutex;
};

void
fdb_htregistry_init(struct fdb_htregistry_t* registry, 
                    struct fdb_mem_allocator_t* allocator);

void
fdb_htregistry_release(struct fdb_htregistry_t* registry);

void
fdb_htregistry_insert(struct fdb_htregistry_t* registry, 
                      const char* key, 
                      void* value);

void*
fdb_htregistry_get(struct fdb_htregistry_t* registry,
                   const char* key);

void
fdb_htregistry_remove(struct fdb_htregistry_t* registyr, 
                      const char* key);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FDB_HASHTABLE_REGISTRY_H_ */
