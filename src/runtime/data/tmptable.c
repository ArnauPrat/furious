

#include "tmptable.h"
#include "../../common/memory/memory.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/platform.h"

#include <string.h>


/**
 * @brief Represents a decoded id, split into the block id and the offset
 * within the block
 */
struct fdb_decoded_id_t 
{
  const uint32_t m_block_id;
  const uint32_t m_block_offset;
};

/**
 * @brief Decodes the given id into the block id and offset
 *
 * @param id
 *
 * @return 
 */
static struct fdb_decoded_id_t
decode_id(entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t did = {
                          .m_block_id = id / FDB_TXTABLE_BLOCK_SIZE, 
                          .m_block_offset = id % FDB_TXTABLE_BLOCK_SIZE
                         };
  return did;
}

/**
 * @brief Tests if a given block contains the given component enabled
 *
 * @param block The block to check at
 * @param id The id of the component to check for
 *
 * @return true if the block contains the given id enabled. false otherwise
 */
bool 
fdb_tmptable_block_has_component(struct fdb_tmptable_block_t* block, 
                              entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  bool res =  fdb_tmptable_block_get_component(block, id).p_data != NULL;
  return res;
}

void
fdb_tmptable_block_init(struct fdb_tmptable_block_t* tblock, 
                        struct fdb_tmptable_t* table,
                        entity_id_t start, 
                        size_t esize)
{
  tblock->p_table = table;
  tblock->p_data = fdb_pool_alloc_alloc(&tblock->p_table->m_data_allocator, 
                                        64, 
                                        esize*FDB_TXTABLE_BLOCK_SIZE, 
                                        start / FDB_TXTABLE_BLOCK_SIZE);
  tblock->m_start = start;
  tblock->m_num_components = 0;
  tblock->m_num_enabled_components = 0;
  tblock->m_esize = esize;
  fdb_bitmap_init(&tblock->m_exists, 
                  &tblock->p_table->p_factory->m_bitmap_factory);
  fdb_bitmap_init(&tblock->m_enabled, 
                  &tblock->p_table->p_factory->m_bitmap_factory);
}


void
fdb_tmptable_block_release(struct fdb_tmptable_block_t* tblock)
{

  fdb_bitmap_release(&tblock->m_enabled);
  fdb_bitmap_release(&tblock->m_exists);
  fdb_pool_alloc_free(&tblock->p_table->m_data_allocator, 
                      tblock->p_data);
}

/**
 * @brief Gets an component from the given block, if it exists
 *
 * @param block The block to get the component from
 * @param id The id to get
 *
 * @return The row of the table containing the retrieved component 
 */
struct fdb_tmptable_entry_t
fdb_tmptable_block_get_component(struct fdb_tmptable_block_t* block, 
                                 entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  assert(block->m_start == (id / FDB_TXTABLE_BLOCK_SIZE) * FDB_TXTABLE_BLOCK_SIZE) ;

  struct fdb_tmptable_entry_t entry;  
  if(fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset)) 
  {
    entry.m_id = block->m_start + decoded_id.m_block_offset;
    entry.p_data = &(((char*)block->p_data)[decoded_id.m_block_offset*block->m_esize]);
    entry.m_enabled = fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  }
  else
  {
    memset(&entry, 0, sizeof(struct fdb_tmptable_entry_t));
  }
  
  return entry;
}

void
fdb_tmptblock_iter_init(struct fdb_tmptblock_iter_t* tblock_iter, 
                     struct fdb_tmptable_block_t* tblock)
{
  tblock_iter->p_block = tblock;
  tblock_iter->m_next_position = 0;
  if(tblock_iter->p_block != NULL) {
    while(tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE && 
          !fdb_tmptable_block_has_component(tblock_iter->p_block, 
                                         tblock_iter->p_block->m_start+tblock_iter->m_next_position) ) 
    {
      tblock_iter->m_next_position++;
    }
  }
}

void
fdb_tmptblock_iter_release(struct fdb_tmptblock_iter_t* tblock_iter)
{
}

bool 
fdb_tmptblock_iter_has_next(struct fdb_tmptblock_iter_t* tblock_iter) 
{
  return  tblock_iter->p_block != NULL && 
          tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE;
}

struct fdb_tmptable_entry_t 
fdb_tmptblock_iter_next(struct fdb_tmptblock_iter_t* tblock_iter) 
{
  struct fdb_tmptable_entry_t fdb_tmptable_entry = fdb_tmptable_block_get_component(tblock_iter->p_block, 
                                                        tblock_iter->p_block->m_start + 
                                                        tblock_iter->m_next_position);
  tblock_iter->m_next_position++; 
  while(tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE && 
        !fdb_tmptable_block_has_component(tblock_iter->p_block, 
                       tblock_iter->p_block->m_start+tblock_iter->m_next_position)) 
  {
    tblock_iter->m_next_position++;
  }
  return fdb_tmptable_entry;
}

void 
fdb_tmptblock_iter_reset(struct fdb_tmptblock_iter_t* tblock_iter, struct fdb_tmptable_block_t* block) 
{
  fdb_tmptblock_iter_init(tblock_iter, block);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fdb_tmptable_iter_init(struct fdb_tmptable_iter_t* iter, 
                    struct fdb_tmptable_t* table, 
                    uint32_t chunk_size,
                    uint32_t offset, 
                    uint32_t stride) 
{
  iter->p_blocks = &table->m_blocks;
  fdb_btree_iter_init(&iter->m_it, iter->p_blocks);
  iter->m_chunk_size = chunk_size;
  iter->m_offset = offset;
  iter->m_stride = stride;
  iter->m_next = NULL;
}

void
fdb_tmptable_iter_release(struct fdb_tmptable_iter_t* iter)
{
}

static bool
is_selected(uint32_t chunk_id, 
            uint32_t chunk_size,
            uint32_t offset,
            uint32_t stride)
{
  // we add stride to -offset to avoid computing modulo over negative numbers
  return ((((chunk_id / chunk_size) + (stride - offset))) % stride) == 0;
}

bool 
fdb_tmptable_iter_has_next(struct fdb_tmptable_iter_t* iter) 
{ 
  while(fdb_btree_iter_has_next(&iter->m_it) && 
       (iter->m_next == NULL))
  {
    struct fdb_btree_entry_t entry = fdb_btree_iter_next(&iter->m_it);
    bool is_next = is_selected(entry.m_key, 
                               iter->m_chunk_size, 
                               iter->m_offset, 
                               iter->m_stride);
    if(is_next)
    {
      iter->m_next = (struct fdb_tmptable_block_t*)entry.p_value;
      break;
    }
    iter->m_next = NULL;
  } 
  return iter->m_next != NULL;;
}

struct fdb_tmptable_block_t* 
fdb_tmptable_iter_next(struct fdb_tmptable_iter_t* iter) 
{
  struct fdb_tmptable_block_t* next = NULL;
  if(iter->m_next)
  {
    next = iter->m_next;
    iter->m_next = NULL;
  }
  else
  {
    bool found = false;
    do 
    {
      next = (struct fdb_tmptable_block_t*)fdb_btree_iter_next(&iter->m_it).p_value;
      if(next != NULL)
      {
        uint32_t chunk_id = next->m_start / (FDB_TXTABLE_BLOCK_SIZE);
        found = is_selected(chunk_id, 
                            iter->m_chunk_size, 
                            iter->m_offset, 
                            iter->m_stride);
      }
    } while (next != NULL && !found);
  }
  return next;
}

void
fdb_tmptable_factory_init(struct fdb_tmptable_factory_t* factory, 
                          struct fdb_mem_allocator_t* allocator)
{
  *factory = (struct fdb_tmptable_factory_t){};
  factory->p_allocator = allocator;

  fdb_btree_factory_init(&factory->m_btree_factory, 
                         allocator);

  fdb_pool_alloc_init(&factory->m_tblock_allocator, 
                      FDB_TXTABLE_BLOCK_ALIGNMENT,
                      sizeof(struct fdb_tmptable_block_t), 
                      FDB_TXTABLE_BLOCK_PAGE_SIZE, 
                      factory->p_allocator);

  fdb_bitmap_factory_init(&factory->m_bitmap_factory, 
                          FDB_TXTABLE_BLOCK_SIZE, 
                          factory->p_allocator);

}

void
fdb_tmptable_factory_release(struct  fdb_tmptable_factory_t* factory)
{
  fdb_btree_factory_release(&factory->m_btree_factory);
  fdb_bitmap_factory_release(&factory->m_bitmap_factory);
  fdb_pool_alloc_release(&factory->m_tblock_allocator);
}

void
fdb_tmptable_init(struct fdb_tmptable_t* table, 
               struct fdb_tmptable_factory_t* factory,
               const char* name, 
               int64_t id, 
               size_t esize, 
               void (*destructor)(void* ptr))
{
  table->p_factory = factory;
  table->m_id = id;
  table->m_esize = esize;
  table->m_num_components = 0;
  table->m_destructor = destructor;
  FDB_COPY_AND_CHECK_STR(&table->m_name[0], name, FDB_MAX_TABLE_NAME);

  fdb_pool_alloc_init(&table->m_data_allocator, 
                      FDB_TXTABLE_BLOCK_DATA_ALIGNMENT, 
                      esize*FDB_TXTABLE_BLOCK_SIZE, 
                      FDB_TXTABLE_BLOCK_DATA_PAGE_SIZE, 
                      table->p_factory->p_allocator);

  fdb_btree_init(&table->m_blocks, 
                 &table->p_factory->m_btree_factory);
}


void
fdb_tmptable_release(struct fdb_tmptable_t* table)
{
  fdb_tmptable_clear(table);
  fdb_btree_release(&table->m_blocks);

  fdb_pool_alloc_release(&table->m_data_allocator);
}

size_t
fdb_tmptable_size(struct fdb_tmptable_t* table)
{
  size_t size =  table->m_num_components;
  return size;
}

void 
fdb_tmptable_clear(struct fdb_tmptable_t* table) 
{
  struct fdb_tmptable_iter_t titer;
  fdb_tmptable_iter_init(&titer,table, 1, 0, 1);
  while(fdb_tmptable_iter_has_next(&titer)) 
  {
    struct fdb_tmptable_block_t* block = fdb_tmptable_iter_next(&titer);
    if(table->m_destructor)
    {
      struct fdb_tmptblock_iter_t b_iterator;
      fdb_tmptblock_iter_init(&b_iterator, block);
      while(fdb_tmptblock_iter_has_next(&b_iterator)) 
      {
        table->m_destructor(fdb_tmptblock_iter_next(&b_iterator).p_data);
      }
      fdb_tmptblock_iter_release(&b_iterator);
    }
    fdb_tmptable_block_release(block);

    fdb_pool_alloc_free(&table->p_factory->m_tblock_allocator, 
                        block);
  }
  fdb_tmptable_iter_release(&titer);
  fdb_btree_clear(&table->m_blocks);
  table->m_num_components = 0;
}


void* 
fdb_tmptable_get_component(struct fdb_tmptable_t* table, 
                    entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  void* component = fdb_btree_get(&table->m_blocks, 
                              decoded_id.m_block_id);
  if(component == NULL) 
  {
    return NULL;
  }
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)component;
  if(fdb_bitmap_is_set(&block->m_exists, decoded_id.m_block_offset))
  {
    return &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  }
  return NULL;
}

void* 
fdb_tmptable_create_component(struct fdb_tmptable_t* table, 
                      entity_id_t id) 
{
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, decoded_id.m_block_id);
  if (block == NULL) 
  {
    block = (struct fdb_tmptable_block_t*)fdb_pool_alloc_alloc(&table->p_factory->m_tblock_allocator, 
                                                               FDB_TXTABLE_BLOCK_ALIGNMENT, 
                                                               sizeof(struct fdb_tmptable_block_t), 
                                                               decoded_id.m_block_id);

    fdb_tmptable_block_init(block, 
                         table,
                         decoded_id.m_block_id*FDB_TXTABLE_BLOCK_SIZE,
                         table->m_esize);

    fdb_btree_insert(&table->m_blocks, 
                     decoded_id.m_block_id,
                     block);
  }

  if(!fdb_bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    table->m_num_components++;
    block->m_num_components++;
    block->m_num_enabled_components++;
  }
  fdb_bitmap_set(&block->m_exists, decoded_id.m_block_offset);
  fdb_bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  return &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
}

void  
fdb_tmptable_destroy_component(struct fdb_tmptable_t* table, 
                            entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, 
                                                               decoded_id.m_block_id);
  if(block == NULL) {
    return;
  }

  if(fdb_bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    table->m_num_components--;
    block->m_num_components--;
    block->m_num_enabled_components--;
  }
  fdb_bitmap_unset(&block->m_exists, decoded_id.m_block_offset);
  fdb_bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  void* ptr =  &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  if(table->m_destructor)
  {
    table->m_destructor(ptr);
  }
  memset(ptr, 0, table->m_esize);
}

void 
fdb_tmptable_enable_component(struct fdb_tmptable_t* table, 
                       entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, 
                                                               decoded_id.m_block_id);
  if(block == NULL)
  {
    return;
  }
  fdb_bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components++;
}

void 
fdb_tmptable_disable_component(struct fdb_tmptable_t* table, 
                        entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == NULL)
  {
    return;
  }
  fdb_bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components--;
}

bool 
fdb_tmptable_is_enabled(struct fdb_tmptable_t* table, 
                 entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == NULL) 
  {
    return false;
  }
  bool res = fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  return res;
}



struct fdb_tmptable_block_t* 
fdb_tmptable_get_block(struct fdb_tmptable_t* table, 
                entity_id_t block_id) 
{
  FDB_ASSERT(block_id != FDB_INVALID_ID);
  struct fdb_tmptable_block_t* block = (struct fdb_tmptable_block_t*)fdb_btree_get(&table->m_blocks, block_id);
  return block;
}

void
fdb_tmptable_set_component_destructor(struct fdb_tmptable_t* table, void (*destr)(void *ptr))
{
  table->m_destructor = destr;
}
