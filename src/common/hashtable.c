
#include "hashtable.h"
#include "platform.h"
#include <string.h>



void
fdb_hashtable_init(fdb_hashtable_t* table, 
                   uint32_t capacity,
                   fdb_mem_allocator_t* allocator)
{

  FDB_ASSERT(((allocator == NULL) ||
              (allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL)) &&
             "Provided allocator is ill-formed.")

  if(allocator != NULL)
  {
    table->p_allocator = allocator; 
  }
  else
  {
    table->p_allocator = fdb_get_global_mem_allocator();
  }
  table->m_capacity = capacity;
  table->m_num_elements = 0;
  table->m_entries = (fdb_hashtable_node_t*)mem_alloc(table->p_allocator, 
                                                      1, 
                                                      sizeof(fdb_hashtable_node_t)*table->m_capacity, 
                                                      FDB_NO_HINT);
  memset(table->m_entries, 0, sizeof(fdb_hashtable_node_t)*table->m_capacity);
}

void
fdb_hashtable_release(fdb_hashtable_t* table)
{
  for (uint32_t i = 0; 
       i < table->m_capacity; 
       ++i) 
  {
    fdb_hashtable_node_t* next = table->m_entries[i].p_next;
    while(next != NULL)
    {
      fdb_hashtable_node_t* tmp = next->p_next;
      mem_free(table->p_allocator, next);
      next = tmp;
    }
  }
  mem_free(table->p_allocator, table->m_entries);
}


void*
fdb_hashtable_add(fdb_hashtable_t* table, 
                  uint32_t key, 
                  void* value)
{
  uint32_t index = key % table->m_capacity;
  fdb_hashtable_node_t* current = &table->m_entries[index];
  fdb_hashtable_node_t* last = NULL;
  while(current != NULL)
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
  if(last->m_num_elements < _FDB_HASHTABLE_NODE_CAPACITY)
  {
    last->m_keys[last->m_num_elements] = key;
    last->m_elements[last->m_num_elements] = value;
    ++last->m_num_elements;
    return NULL;
  }

  fdb_hashtable_node_t* next = (fdb_hashtable_node_t*)mem_alloc(table->p_allocator, 
                                                                1, 
                                                                sizeof(fdb_hashtable_node_t), 
                                                                -1);
  memset(next, 0, sizeof(fdb_hashtable_node_t));
  next->m_keys[0] = key;
  next->m_elements[0] = value;
  ++next->m_num_elements;
  last->p_next = next;
  return NULL;
}

void*
fdb_hashtable_get(fdb_hashtable_t* table, 
                  uint32_t key)
{
  uint32_t index = key % table->m_capacity;
  fdb_hashtable_node_t* current = &table->m_entries[index];
  while(current != NULL)
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
  return NULL;
}

fdb_hashtable_iter_t
fdb_hashtable_iter_init(fdb_hashtable_t* hashtable)
{
  fdb_hashtable_iter_t iter;
  iter.p_table = hashtable;
  iter.p_next_node = NULL;
  iter.m_next_entry = 0;
  iter.m_next_element = 0;
  for(uint32_t i = 0; 
      i < iter.p_table->m_capacity;
      ++i)
  {
    iter.m_next_entry = i+1;
    fdb_hashtable_node_t* next_node = &iter.p_table->m_entries[i];
    if(next_node->m_num_elements > 0)
    {
      iter.p_next_node = next_node;
      return iter;
    }
  }
  return iter;
}

void
fdb_hashtable_iter_release(fdb_hashtable_iter_t* iter)
{
}

bool
fdb_hashtable_iter_has_next(fdb_hashtable_iter_t* iter)
{
  return iter->p_next_node != NULL;
}

fdb_hashtable_entry_t
fdb_hashtable_iter_next(fdb_hashtable_iter_t* iter)
{
  fdb_hashtable_entry_t entry = {iter->p_next_node->m_keys[iter->m_next_element],
    iter->p_next_node->m_elements[iter->m_next_element]};
  ++iter->m_next_element;
  if(iter->m_next_element >= iter->p_next_node->m_num_elements)
  {
    iter->m_next_element = 0;
    iter->p_next_node = iter->p_next_node->p_next;
  }

  if(iter->p_next_node == NULL && iter->m_next_entry < iter->p_table->m_capacity)
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
      iter->p_next_node = NULL;
    }
  }
  return entry;
}

