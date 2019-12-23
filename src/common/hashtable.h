


#ifndef _FURIOUS_HASHTABLE_H_
#define _FURIOUS_HASHTABLE_H_ 

#include "types.h"
#include "memory/memory.h"

namespace furious
{

#define _FURIOUS_HASHTABLE_NODE_CAPACITY 8

struct hashtable_node_t
{
  void*              m_elements[_FURIOUS_HASHTABLE_NODE_CAPACITY]; 
  uint32_t           m_keys[_FURIOUS_HASHTABLE_NODE_CAPACITY];
  uint32_t           m_num_elements;
  hashtable_node_t* p_next;
};

struct hashtable_t
{
  uint32_t            m_capacity;
  uint32_t            m_num_elements;
  hashtable_node_t*   m_entries;
  mem_allocator_t     m_allocator;
};

struct hashtable_iter_t
{
  hashtable_t*      p_table;
  hashtable_node_t* p_next_node;
  uint32_t          m_next_entry;
  uint32_t          m_next_element;
};

struct hashtable_entry_t
{
  uint32_t  m_key;
  void*     p_value;
};

/**
 * \brief Initializes a hash table
 *
 * \param table The hash table to initialize
 */
hashtable_t
hashtable_create(uint32_t capacity, 
                 mem_allocator_t* allocator = nullptr);

/**
 * \brief Releases a hash table
 *
 * \param table The hash table to release
 */
void
hashtable_destroy(hashtable_t* table);


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
hashtable_add(hashtable_t* table, 
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
hashtable_get(hashtable_t* table, 
              uint32_t key);

/**
 * \brief Initialize a hashtable iterator
 *
 * \param iter The iterator to initialize
 * \param hashtable The hashtable this iterator iterates over
 */
hashtable_iter_t
hashtable_iter_create(hashtable_t* hashtable);

/**
 * \brief Releases the given iterator
 *
 * \param iter The iterator to release
 */
void
hashtable_iter_destroy(hashtable_iter_t* iter);

/**
 * \brief Checks if the iterator has next elements
 *
 * \param iter The iterator to check for has next
 */
bool
hashtable_iter_has_next(hashtable_iter_t* iter);

/**
 * \brief Checks if the iterator has next elements
 *
 * \param iter The iterator to check for has next
 * 
 * \return Returns a hashtable entry structure 
 */
hashtable_entry_t
hashtable_iter_next(hashtable_iter_t* iter);

} /* furious */ 
#endif /* ifndef _FURIOUS_hashtable_ */
