

#include "reftable.h"



void
fdb_reftable_init(fdb_reftable_t* rt, 
                  const char* rname, 
                  uint32_t id, 
                  fdb_mem_allocator_t* allocator)
{
  fdb_table_init(&rt->m_table, 
                 rname, 
                 id, 
                 sizeof(entity_id_t), NULL, allocator);
}

/**
 * \brief Destroys a reference table
 *
 * \param rt The reference table to destroy
 */
void
fdb_reftable_release(fdb_reftable_t* rt)
{
  fdb_table_release(&rt->m_table);
}
  
void
fdb_reftable_add(fdb_reftable_t* rt, 
              entity_id_t tail, 
              entity_id_t head)
{
  entity_id_t* phead = (entity_id_t*)fdb_table_create_component(&rt->m_table, 
                                                           tail);
  *phead = head;
}

entity_id_t*
fdb_reftable_get(fdb_reftable_t* rt, 
              entity_id_t tail)
{
  return (entity_id_t*) (entity_id_t*)fdb_table_get_component(&rt->m_table,
                                                          tail); 
}

void
fdb_reftable_remove(fdb_reftable_t* rt, 
                 entity_id_t tail, 
                 entity_id_t head)
{
  fdb_table_destroy_component(&rt->m_table, 
                          tail);
}

bool
fdb_reftable_exists(fdb_reftable_t* rt, 
                 entity_id_t tail, 
                 entity_id_t head)
{
  entity_id_t* phead = (entity_id_t*)fdb_table_get_component(&rt->m_table, 
                                                         tail);
  return phead != NULL && *phead == head;
}

