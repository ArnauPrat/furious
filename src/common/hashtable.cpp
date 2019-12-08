
#include "hashtable.h"
#include "platform.h"
#include <string.h>

namespace furious
{


void
hashtable_init(hashtable_t* table, 
                uint32_t capacity, 
                furious_alloc_t alloc_func, 
                furious_free_t free_func)
{
  FURIOUS_ASSERT( ((alloc_func == nullptr && free_func == nullptr) ||
                  (alloc_func != nullptr && free_func != nullptr)) &&
                  "Both allocator functions must be non-null or both null");

  table->p_mem_alloc = alloc_func != nullptr ? alloc_func : mem_alloc;
  table->p_mem_free = free_func != nullptr ? free_func : mem_free;
  table->m_capacity = capacity;
  table->m_num_elements = 0;
  table->m_entries = (hashtable_node_t*)table->p_mem_alloc(1, sizeof(hashtable_node_t)*table->m_capacity, -1);
  memset(table->m_entries, 0, sizeof(hashtable_node_t)*table->m_capacity);
}

void
hashtable_release(hashtable_t* table)
{
  for (uint32_t i = 0; 
       i < table->m_capacity; 
       ++i) 
  {
    hashtable_node_t* next = table->m_entries[i].p_next;
    while(next != nullptr)
    {
      hashtable_node_t* tmp = next->p_next;
      table->p_mem_free(next);
      next = tmp;
    }
  }
  table->p_mem_free(table->m_entries);
}


void*
hashtable_add(hashtable_t* table, 
               uint32_t key, 
               void* value)
{
  uint32_t index = key % table->m_capacity;
  hashtable_node_t* current = &table->m_entries[index];
  hashtable_node_t* last = nullptr;
  while(current != nullptr)
  {
    last = current;
    for(uint32_t i = 0; 
        i < current->m_num_elements; 
        ++i)
    {
      if(current->m_keys[i] == key)
      {
        void* prev = current->m_elements[i];
        current->m_elements[i] = value; 
        return prev;
      }
    }
    current = current->p_next;
  }
  table->m_num_elements++;
  if(last->m_num_elements < _FURIOUS_HASHTABLE_NODE_CAPACITY)
  {
    last->m_keys[last->m_num_elements] = key;
    last->m_elements[last->m_num_elements] = value;
    ++last->m_num_elements;
    return nullptr;
  }

  hashtable_node_t* next = (hashtable_node_t*)table->p_mem_alloc(1, sizeof(hashtable_node_t), -1);
  *next = {{0}};
  next->m_keys[0] = key;
  next->m_elements[0] = value;
  ++next->m_num_elements;
  last->p_next = next;
  return nullptr;
}

void*
hashtable_get(hashtable_t* table, 
               uint32_t key)
{
  uint32_t index = key % table->m_capacity;
  hashtable_node_t* current = &table->m_entries[index];
  while(current != nullptr)
  {
    for(uint32_t i = 0; 
        i < current->m_num_elements; 
        ++i)
    {
      if(current->m_keys[i] == key)
      {
        return current->m_elements[i]; 
      }
    }
    current = current->p_next;
  }
  return nullptr;
}

void
hashtable_iter_init(hashtable_iter_t* iter, 
                    hashtable_t* hashtable)
{
  iter->p_table = hashtable;
  iter->p_next_node = nullptr;
  iter->m_next_entry = 0;
  iter->m_next_element = 0;
  for(uint32_t i = 0; 
      i < iter->p_table->m_capacity;
      ++i)
  {
    iter->m_next_entry = i+1;
    hashtable_node_t* next_node = &iter->p_table->m_entries[i];
    if(next_node->m_num_elements > 0)
    {
      iter->p_next_node = next_node;
      return;
    }
  }
}

void
hashtable_iter_release(hashtable_iter_t* iter)
{
}

bool
hashtable_iter_has_next(hashtable_iter_t* iter)
{
  return iter->p_next_node != nullptr;
}

hashtable_entry_t
hashtable_iter_next(hashtable_iter_t* iter)
{
  hashtable_entry_t entry = {iter->p_next_node->m_keys[iter->m_next_element],
                             iter->p_next_node->m_elements[iter->m_next_element]};
  ++iter->m_next_element;
  if(iter->m_next_element >= iter->p_next_node->m_num_elements)
  {
    iter->m_next_element = 0;
    iter->p_next_node = iter->p_next_node->p_next;
  }

  if(iter->p_next_node == nullptr && iter->m_next_entry < iter->p_table->m_capacity)
  {
    do
    {
      iter->p_next_node = &iter->p_table->m_entries[iter->m_next_entry];
      ++iter->m_next_entry;
    }
    while(iter->p_next_node->m_num_elements == 0 && 
          iter->m_next_entry < iter->p_table->m_capacity);

    if(iter->p_next_node->m_num_elements == 0)
    {
      iter->p_next_node = nullptr;
    }
  }
  return entry;
}
  
} /* furious */ 

