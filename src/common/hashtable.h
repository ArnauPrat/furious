
#ifndef _FDB_HASHTABLE_H_
#define _FDB_HASHTABLE_H_ 

#include "types.h"
#include "memory/memory.h"

#include <stdbool.h>
 
#ifdef __cplusplus
extern "C" {
#endif


#define _FDB_HASHTABLE_NODE_CAPACITY 8

typedef struct fdb_hashtable_node_t
{
  void*                 m_elements[_FDB_HASHTABLE_NODE_CAPACITY]; 
  uint32_t              m_keys[_FDB_HASHTABLE_NODE_CAPACITY];
  uint32_t              m_num_elements;
  struct fdb_hashtable_node_t* p_next;
} fdb_hashtable_node_t;

typedef struct fdb_hashtable_t
{
  uint32_t                m_capacity;
  uint32_t                m_num_elements;
  fdb_hashtable_node_t*   m_entries;
  fdb_mem_allocator_t*     p_allocator;
} fdb_hashtable_t;

typedef struct fdb_hashtable_iter_t
{
  fdb_hashtable_t*      p_table;
  fdb_hashtable_node_t* p_next_node;
  uint32_t              m_next_entry;
  uint32_t              m_next_element;
} fdb_hashtable_iter_t;

typedef struct fdb_hashtable_entry_t
{
  uint32_t  m_key;
  void*     p_value;
} fdb_hashtable_entry_t;

/**
 * \brief Initializes a hash table
 *
 * \param table The hash table to initialize
 */
void
fdb_hashtable_init(fdb_hashtable_t* ht, 
                   uint32_t capacity, 
                   fdb_mem_allocator_t* allocator);

/**
 * \brief Releases a hash table
 *
 * \param table The hash table to release
 */
void
fdb_hashtable_release(fdb_hashtable_t* table);


/**
 * \brief Adds an element to the hash table
 *
 * \param table The hash table to add the element to
 * \param key The key of the element to add
 * \param value The value of the element to add
 *
 * \return Returns the former element with the given added key if it existed
 */
void*
fdb_hashtable_add(fdb_hashtable_t* table, 
                  uint32_t key, 
                  void* value);

/**
 * \brief Adds an element to the hash table
 *
 * \param table The hash table to add the element to
 * \param key The key of the element to add
 * \param value The value of the element to add
 */
void*
fdb_hashtable_get(fdb_hashtable_t* table, 
                  uint32_t key);

/**
 * \brief Initialize a hashtable iterator
 *
 * \param iter The iterator to initialize
 * \param hashtable The hashtable this iterator iterates over
 */
fdb_hashtable_iter_t
fdb_hashtable_iter_init(fdb_hashtable_t* hashtable);

/**
 * \brief Releases the given iterator
 *
 * \param iter The iterator to release
 */
void
fdb_hashtable_iter_release(fdb_hashtable_iter_t* iter);

/**
 * \brief Checks if the iterator has next elements
 *
 * \param iter The iterator to check for has next
 */
bool
fdb_hashtable_iter_has_next(fdb_hashtable_iter_t* iter);

/**
 * \brief Checks if the iterator has next elements
 *
 * \param iter The iterator to check for has next
 * 
 * \return Returns a hashtable entry typedef structure 
 */
fdb_hashtable_entry_t
fdb_hashtable_iter_next(fdb_hashtable_iter_t* iter);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_fdb_hashtable_ */
