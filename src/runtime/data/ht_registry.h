
#ifndef _FURIOUS_HASHTABLE_REGISTRY_H_
#define _FURIOUS_HASHTABLE_REGISTRY_H_ value

#include <mutex>

namespace furious
{

struct BTRoot;

struct ht_registry_t
{
  BTRoot* p_root;
  std::mutex m_mutex;
};

void
ht_registry_init(ht_registry_t* registry);

void
ht_registry_release(ht_registry_t* registry);

void**
ht_registry_insert(ht_registry_t* registry, 
                   const char* key);

void*
ht_registry_get(ht_registry_t* registry,
                const char* key);

void
ht_registry_remove(ht_registry_t* registyr, 
                   const char* key);

  
} /* furious */ 
#endif /* ifndef _FURIOUS_HASHTABLE_REGISTRY_H_ */
