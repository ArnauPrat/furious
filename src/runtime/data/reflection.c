

#include "reflection.h"
#include "string.h"
#include "../../common/utils.h"


void
fdb_mregistry_init(fdb_mregistry_t* reg, 
                   fdb_mem_allocator_t* allocator)
{
  fdb_mem_allocator_t* mallocator = fdb_get_global_mem_allocator();
  if(allocator != NULL)
  {
    mallocator = allocator; 
  }

  fdb_pool_alloc_init(&reg->m_mstruct_allocator,
                      64, 
                      sizeof(fdb_mstruct_t),
                      FDB_REFLECTION_STRUCT_PAGE_SIZE,
                      mallocator);

  fdb_pool_alloc_init(&reg->m_mfield_allocator,
                      64, 
                      sizeof(fdb_mfield_t),
                      FDB_REFLECTION_FIELD_PAGE_SIZE,
                      mallocator);

  fdb_btree_init(&reg->m_metadata, 
                 mallocator);
}

void
fdb_mregistry_release(fdb_mregistry_t* reg)
{
  fdb_btree_iter_t iter;
  fdb_btree_iter_init(&iter, &reg->m_metadata);
  while(fdb_btree_iter_has_next(&iter))
  {
    fdb_mstruct_t* mstruct = (fdb_mstruct_t*)(fdb_btree_iter_next(&iter).p_value);
    //fdb_mstruct_release(mstruct);
    fdb_pool_alloc_free(&reg->m_mstruct_allocator, mstruct);
  }
  fdb_btree_iter_release(&iter);
  fdb_btree_release(&reg->m_metadata);
  fdb_pool_alloc_release(&reg->m_mfield_allocator);
  fdb_pool_alloc_release(&reg->m_mstruct_allocator);
}


fdb_mstruct_t*
fdb_mregistry_init_mstruct(fdb_mregistry_t* reg, 
                           const char* name, 
                           bool is_union)
{
  fdb_mstruct_t* mstruct = (fdb_mstruct_t*)fdb_pool_alloc_alloc(&reg->m_mstruct_allocator, 
                                                                64, 
                                                                sizeof(fdb_mstruct_t), 
                                                                FDB_NO_HINT);
  memset(mstruct, 0, sizeof(fdb_mstruct_t));
  FDB_COPY_AND_CHECK_STR(mstruct->m_type_name, name, FDB_MAX_TABLE_NAME);
  mstruct->m_is_union = is_union;
  uint32_t hashvalue = hash(name);
  fdb_btree_insert_t insert = fdb_btree_insert(&reg->m_metadata, hashvalue, mstruct);
  if(!insert.m_inserted)
  {
    fdb_mstruct_t* old = (fdb_mstruct_t*)(*insert.p_place);
    *insert.p_place = mstruct;
    //fdb_mstruct_release(old);
    fdb_pool_alloc_free(&reg->m_mstruct_allocator, old);
  }
  return mstruct;
}

fdb_mstruct_t*
fdb_mregistry_init_mfield(fdb_mregistry_t* reg, 
                          fdb_mstruct_t* mstruct, 
                          const char* name, 
                          fdb_mtype_t type, 
                          size_t offset, 
                          bool is_anon)
{
  fdb_mfield_t* mfield = (fdb_mfield_t*)fdb_pool_alloc_alloc(&reg->m_mfield_allocator,
                                                             64,
                                                             sizeof(fdb_mfield_t), 
                                                             FDB_NO_HINT);
  memset(mfield, 0, sizeof(fdb_mfield_t));
  FDB_COPY_AND_CHECK_STR(mfield->m_name, name, FCC_MAX_FIELD_NAME);
  mfield->m_type = type;
  mfield->m_offset = offset;
  mfield->m_anonymous = is_anon;
  if(mfield->m_type == E_UNION || mfield->m_type == E_STRUCT)
  {
    mfield->p_strct_type = (fdb_mstruct_t*)fdb_pool_alloc_alloc(&reg->m_mstruct_allocator, 
                                                                64, 
                                                                sizeof(fdb_mstruct_t), 
                                                                FDB_NO_HINT);
  }
  mstruct->p_fields[mstruct->m_nfields++] = mfield;
  return mfield->p_strct_type;
}


fdb_mstruct_t*
fdb_mregistry_get_mstruct(fdb_mregistry_t* reg, 
                          const char* name)
{
  uint32_t hashvalue = hash(name);
  fdb_mstruct_t* mstruct = (fdb_mstruct_t*)fdb_btree_get(&reg->m_metadata, hashvalue);
  return mstruct;
}

