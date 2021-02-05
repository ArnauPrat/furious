

#include "reflection.h"
#include "string.h"
#include "../../common/utils.h"


void
fdb_mregistry_init(struct fdb_mregistry_t* reg, 
                   struct fdb_mem_allocator_t* allocator)
{
  fdb_pool_alloc_init(&reg->m_mstruct_allocator,
                      64, 
                      sizeof(struct fdb_mstruct_t),
                      FDB_REFLECTION_STRUCT_PAGE_SIZE,
                      allocator);

  fdb_pool_alloc_init(&reg->m_mfield_allocator,
                      64, 
                      sizeof(struct fdb_mfield_t),
                      FDB_REFLECTION_FIELD_PAGE_SIZE,
                      allocator);

  fdb_btree_factory_init(&reg->m_btree_factory, 
                         allocator);
  fdb_btree_init(&reg->m_metadata,
                 &reg->m_btree_factory);
}

void
fdb_mregistry_release(struct fdb_mregistry_t* reg)
{
  struct fdb_btree_iter_t iter;
  fdb_btree_iter_init(&iter, &reg->m_metadata);
  while(fdb_btree_iter_has_next(&iter))
  {
    struct fdb_mstruct_t* mstruct = (struct fdb_mstruct_t*)(fdb_btree_iter_next(&iter).p_value);
    //fdb_mstruct_release(mstruct);
    fdb_pool_alloc_free(&reg->m_mstruct_allocator, mstruct);
  }
  fdb_btree_iter_release(&iter);
  fdb_btree_release(&reg->m_metadata);
  fdb_btree_factory_release(&reg->m_btree_factory);
  fdb_pool_alloc_release(&reg->m_mfield_allocator);
  fdb_pool_alloc_release(&reg->m_mstruct_allocator);
}


struct fdb_mstruct_t*
fdb_mregistry_init_mstruct(struct fdb_mregistry_t* reg, 
                           const char* name, 
                           bool is_union)
{
  struct fdb_mstruct_t* mstruct = (struct fdb_mstruct_t*)fdb_pool_alloc_alloc(&reg->m_mstruct_allocator, 
                                                                64, 
                                                                sizeof(struct fdb_mstruct_t), 
                                                                FDB_NO_HINT);
  memset(mstruct, 0, sizeof(struct fdb_mstruct_t));
  FDB_COPY_AND_CHECK_STR(mstruct->m_type_name, name, FDB_MAX_TABLE_NAME);
  mstruct->m_is_union = is_union;
  mstruct->m_nfields = 0;
  uint32_t hashvalue = hash(name);
  struct fdb_btree_insert_t insert = fdb_btree_insert(&reg->m_metadata, hashvalue, mstruct);
  if(!insert.m_inserted)
  {
    struct fdb_mstruct_t* old = (struct fdb_mstruct_t*)(*insert.p_place);
    *insert.p_place = mstruct;
    fdb_pool_alloc_free(&reg->m_mstruct_allocator, old);
  }
  return mstruct;
}

struct fdb_mstruct_t*
fdb_mregistry_init_mfield(struct fdb_mregistry_t* reg, 
                          struct fdb_mstruct_t* mstruct, 
                          const char* name, 
                          enum fdb_mtype_t type, 
                          size_t offset, 
                          bool is_anon)
{
  struct fdb_mfield_t* mfield = (struct fdb_mfield_t*)fdb_pool_alloc_alloc(&reg->m_mfield_allocator,
                                                             64,
                                                             sizeof(struct fdb_mfield_t), 
                                                             FDB_NO_HINT);
  memset(mfield, 0, sizeof(struct fdb_mfield_t));
  FDB_COPY_AND_CHECK_STR(mfield->m_name, name, FCC_MAX_FIELD_NAME);
  mfield->m_type = type;
  mfield->m_offset = offset;
  mfield->m_anonymous = is_anon;
  if(mfield->m_type == E_UNION || mfield->m_type == E_STRUCT)
  {
    mfield->p_strct_type = (struct fdb_mstruct_t*)fdb_pool_alloc_alloc(&reg->m_mstruct_allocator, 
                                                                64, 
                                                                sizeof(struct fdb_mstruct_t), 
                                                                FDB_NO_HINT);
    memset(mfield->p_strct_type, 0, sizeof(struct fdb_mstruct_t));
  }
  FDB_PERMA_ASSERT(mstruct->m_nfields < FDB_MAX_COMPONENT_FIELDS && "Maximum number of fields per component reached");
  mstruct->p_fields[mstruct->m_nfields++] = mfield;
  return mfield->p_strct_type;
}


struct fdb_mstruct_t*
fdb_mregistry_get_mstruct(struct fdb_mregistry_t* reg, 
                          const char* name)
{
  uint32_t hashvalue = hash(name);
  struct fdb_mstruct_t* mstruct = (struct fdb_mstruct_t*)fdb_btree_get(&reg->m_metadata, hashvalue);
  return mstruct;
}

