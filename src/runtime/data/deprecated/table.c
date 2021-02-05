

#include "table.h"
#include "../../common/memory/memory.h"
#include "../../common/memory/pool_allocator.h"
#include "../../common/platform.h"

#include <string.h>


/**
 * @brief Represents a decoded id, split into the block id and the offset
 * within the block
 */
typedef struct fdb_decoded_id_t 
{
  const uint32_t m_block_id;
  const uint32_t m_block_offset;
} fdb_decoded_id_t;

/**
 * @brief Decodes the given id into the block id and offset
 *
 * @param id
 *
 * @return 
 */
static fdb_decoded_id_t
decode_id(entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  fdb_decoded_id_t did = {
                          .m_block_id = id / FDB_TABLE_BLOCK_SIZE, 
                          .m_block_offset = id % FDB_TABLE_BLOCK_SIZE
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
fdb_table_block_has_component(const fdb_table_block_t* block, 
                 entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  bool res =  fdb_table_block_get_component(block, id).p_data != NULL;
  return res;
}

void
fdb_table_block_init(fdb_table_block_t* tblock, 
                     entity_id_t start, 
                     size_t esize, 
                     fdb_mem_allocator_t* data_allocator, 
                     fdb_mem_allocator_t* bitmap_allocator)
{
  FDB_ASSERT(data_allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((data_allocator->p_mem_alloc != NULL && data_allocator->p_mem_free != NULL) &&
                 "Provided allocator is ill-formed.")

  FDB_ASSERT(bitmap_allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((bitmap_allocator->p_mem_alloc != NULL && bitmap_allocator->p_mem_free != NULL) &&
                 "Provided allocator is ill-formed.")

  tblock->p_data = mem_alloc(data_allocator, 64, esize*FDB_TABLE_BLOCK_SIZE, start / FDB_TABLE_BLOCK_SIZE);
  tblock->m_start = start;
  tblock->m_num_components = 0;
  tblock->m_num_enabled_components = 0;
  tblock->m_esize = esize;
  fdb_bitmap_init(&tblock->m_exists, 
                  FDB_TABLE_BLOCK_SIZE, bitmap_allocator);
  fdb_bitmap_init(&tblock->m_enabled, 
                  FDB_TABLE_BLOCK_SIZE, bitmap_allocator);
}


void
fdb_table_block_release(fdb_table_block_t* tblock, 
                    fdb_mem_allocator_t* data_allocator, 
                    fdb_mem_allocator_t* bitmap_allocator)
{

  FDB_ASSERT(data_allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((data_allocator->p_mem_alloc != NULL && data_allocator->p_mem_free != NULL) &&
                 "Provided allocator is ill-formed.")

  FDB_ASSERT(bitmap_allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((bitmap_allocator->p_mem_alloc != NULL && bitmap_allocator->p_mem_free != NULL) &&
                 "Provided allocator is ill-formed.")

  fdb_bitmap_release(&tblock->m_enabled, bitmap_allocator);
  fdb_bitmap_release(&tblock->m_exists, bitmap_allocator);
  mem_free(data_allocator, 
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
fdb_table_entry_t
fdb_table_block_get_component(const fdb_table_block_t* block, 
                 entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  fdb_decoded_id_t decoded_id = decode_id(id);
  assert(block->m_start == (id / FDB_TABLE_BLOCK_SIZE) * FDB_TABLE_BLOCK_SIZE) ;

  fdb_table_entry_t entry;  
  if(fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset)) 
  {
    entry.m_id = block->m_start + decoded_id.m_block_offset;
    entry.p_data = &(((char*)block->p_data)[decoded_id.m_block_offset*block->m_esize]);
    entry.m_enabled = fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  }
  else
  {
    memset(&entry, 0, sizeof(fdb_table_entry_t));
  }
  
  return entry;
}

void
fdb_tblock_iter_init(fdb_tblock_iter_t* tblock_iter, 
                 fdb_table_block_t* tblock)
{
  tblock_iter->p_block = tblock;
  tblock_iter->m_next_position = 0;
  if(tblock_iter->p_block != NULL) {
    while(tblock_iter->m_next_position < FDB_TABLE_BLOCK_SIZE && 
          !fdb_table_block_has_component(tblock_iter->p_block, 
                                         tblock_iter->p_block->m_start+tblock_iter->m_next_position) ) 
    {
      tblock_iter->m_next_position++;
    }
  }
}

void
fdb_tblock_iter_release(fdb_tblock_iter_t* tblock_iter)
{
}

bool 
fdb_tblock_iter_has_next(fdb_tblock_iter_t* tblock_iter) 
{
  return  tblock_iter->p_block != NULL && 
          tblock_iter->m_next_position < FDB_TABLE_BLOCK_SIZE;
}

fdb_table_entry_t 
fdb_tblock_iter_next(fdb_tblock_iter_t* tblock_iter) 
{
  fdb_table_entry_t fdb_table_entry = fdb_table_block_get_component(tblock_iter->p_block, 
                                                        tblock_iter->p_block->m_start + 
                                                        tblock_iter->m_next_position);
  tblock_iter->m_next_position++; 
  while(tblock_iter->m_next_position < FDB_TABLE_BLOCK_SIZE && 
        !fdb_table_block_has_component(tblock_iter->p_block, 
                       tblock_iter->p_block->m_start+tblock_iter->m_next_position)) 
  {
    tblock_iter->m_next_position++;
  }
  return fdb_table_entry;
}

void 
fdb_tblock_iter_reset(fdb_tblock_iter_t* tblock_iter, fdb_table_block_t* block) 
{
  fdb_tblock_iter_init(tblock_iter, block);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fdb_table_iter_init(fdb_table_iter_t* iter, 
                    fdb_table_t* table, 
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
fdb_table_iter_release(fdb_table_iter_t* iter)
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
fdb_table_iter_has_next(fdb_table_iter_t* iter) 
{ 
  while(fdb_btree_iter_has_next(&iter->m_it) && 
       (iter->m_next == NULL))
  {
    fdb_btree_entry_t entry = fdb_btree_iter_next(&iter->m_it);
    bool is_next = is_selected(entry.m_key, 
                               iter->m_chunk_size, 
                               iter->m_offset, 
                               iter->m_stride);
    if(is_next)
    {
      iter->m_next = (fdb_table_block_t*)entry.p_value;
      break;
    }
    iter->m_next = NULL;
  } 
  return iter->m_next != NULL;;
}

fdb_table_block_t* 
fdb_table_iter_next(fdb_table_iter_t* iter) 
{
  fdb_table_block_t* next = NULL;
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
      next = (fdb_table_block_t*)fdb_btree_iter_next(&iter->m_it).p_value;
      if(next != NULL)
      {
        uint32_t chunk_id = next->m_start / (FDB_TABLE_BLOCK_SIZE);
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
fdb_table_init(fdb_table_t* table, 
               const char* name, 
               int64_t id, 
               size_t esize, 
               void (*destructor)(void* ptr), 
               fdb_mem_allocator_t* allocator)
{
  table->m_id = id;
  table->m_esize = esize;
  table->m_num_components = 0;
  table->m_destructor = destructor;
  FDB_COPY_AND_CHECK_STR(&table->m_name[0], name, FDB_MAX_TABLE_NAME);

  // Initializing Allocators
  fdb_mem_allocator_t* mallocator = allocator != NULL ? allocator : fdb_get_global_mem_allocator();


  fdb_pool_alloc_init(&table->m_tblock_allocator, 
                      FDB_TABLE_BLOCK_ALIGNMENT,
                      sizeof(fdb_table_block_t), 
                      FDB_TABLE_BLOCK_PAGE_SIZE, 
                      mallocator);

  fdb_pool_alloc_init(&table->m_data_allocator, 
                      FDB_TABLE_BLOCK_DATA_ALIGNMENT, 
                      esize*FDB_TABLE_BLOCK_SIZE, 
                      FDB_TABLE_BLOCK_DATA_PAGE_SIZE, 
                      mallocator);

  fdb_pool_alloc_init(&table->m_bitmap_data_allocator, 
                      FDB_BITMAP_DATA_ALIGNMENT, 
                      FDB_BITMAP_NUM_CHUNKS(FDB_TABLE_BLOCK_SIZE), 
                      FDB_BITMAP_DATA_PAGE_SIZE, 
                      mallocator);


  fdb_btree_init(&table->m_blocks, mallocator);
  fdb_mutex_init(&table->m_mutex);
}


void
fdb_table_release(fdb_table_t* table)
{
  fdb_table_clear(table);
  fdb_mutex_release(&table->m_mutex);
  fdb_btree_release(&table->m_blocks);

  fdb_pool_alloc_release(&table->m_bitmap_data_allocator);
  fdb_pool_alloc_release(&table->m_data_allocator);
  fdb_pool_alloc_release(&table->m_tblock_allocator);
}

size_t
fdb_table_size(fdb_table_t* table)
{
  fdb_table_lock(table);
  size_t size =  table->m_num_components;
  fdb_table_unlock(table);
  return size;
}

void 
fdb_table_clear(fdb_table_t* table) 
{
  fdb_table_lock(table);
  fdb_table_iter_t titer;
  fdb_table_iter_init(&titer,table, 1, 0, 1);
  while(fdb_table_iter_has_next(&titer)) 
  {
    fdb_table_block_t* block = fdb_table_iter_next(&titer);
    if(table->m_destructor)
    {
      fdb_tblock_iter_t b_iterator;
      fdb_tblock_iter_init(&b_iterator, block);
      while(fdb_tblock_iter_has_next(&b_iterator)) 
      {
        table->m_destructor(fdb_tblock_iter_next(&b_iterator).p_data);
      }
      fdb_tblock_iter_release(&b_iterator);
    }
    fdb_table_block_release(block, 
                            &table->m_data_allocator.m_super, 
                            &table->m_bitmap_data_allocator.m_super);

    fdb_pool_alloc_free(&table->m_tblock_allocator, 
                        block);
  }
  fdb_table_iter_release(&titer);
  fdb_btree_clear(&table->m_blocks);
  table->m_num_components = 0;
  fdb_table_unlock(table);
}


void* 
fdb_table_get_component(fdb_table_t* table, 
                    entity_id_t id) 
{
  fdb_table_lock(table);
  assert(id != FDB_INVALID_ID);
  fdb_decoded_id_t decoded_id = decode_id(id);
  void* component = fdb_btree_get(&table->m_blocks, 
                              decoded_id.m_block_id);
  if(component == NULL) 
  {
    fdb_table_unlock(table);
    return NULL;
  }
  fdb_table_block_t* block = (fdb_table_block_t*)component;
  if(fdb_bitmap_is_set(&block->m_exists, decoded_id.m_block_offset))
  {
    fdb_table_unlock(table);
    return &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  }
  fdb_table_unlock(table);
  return NULL;
}

void* 
fdb_table_create_component(fdb_table_t* table, 
                      entity_id_t id) 
{
  fdb_decoded_id_t decoded_id = decode_id(id);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, decoded_id.m_block_id);
  if (block == NULL) 
  {
    block = (fdb_table_block_t*)fdb_pool_alloc_alloc(&table->m_tblock_allocator, 
                                                     FDB_TABLE_BLOCK_ALIGNMENT, 
                                                     sizeof(fdb_table_block_t), 
                                                     decoded_id.m_block_id);

    fdb_table_block_init(block, 
                         decoded_id.m_block_id*FDB_TABLE_BLOCK_SIZE,
                         table->m_esize, 
                         &table->m_data_allocator.m_super, 
                         &table->m_bitmap_data_allocator.m_super);

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
fdb_table_destroy_component(fdb_table_t* table, 
                            entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  fdb_table_lock(table);
  assert(id != FDB_INVALID_ID);
  fdb_decoded_id_t decoded_id = decode_id(id);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, 
                                                               decoded_id.m_block_id);
  if(block == NULL) {
    fdb_table_unlock(table);
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
  fdb_table_unlock(table);
  void* ptr =  &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  if(table->m_destructor)
  {
    table->m_destructor(ptr);
  }
  memset(ptr, 0, table->m_esize);
}

void 
fdb_table_enable_component(fdb_table_t* table, 
                       entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  fdb_table_lock(table);
  fdb_decoded_id_t decoded_id = decode_id(id);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, 
                                                               decoded_id.m_block_id);
  if(block == NULL)
  {
    fdb_table_unlock(table);
    return;
  }
  fdb_bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components++;
  fdb_table_unlock(table);
}

void 
fdb_table_disable_component(fdb_table_t* table, 
                        entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  fdb_table_lock(table);

  fdb_decoded_id_t decoded_id = decode_id(id);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == NULL)
  {
    fdb_table_unlock(table);
    return;
  }
  fdb_bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components--;
  fdb_table_unlock(table);
}

bool 
fdb_table_is_enabled(fdb_table_t* table, 
                 entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  fdb_table_lock(table);

  fdb_decoded_id_t decoded_id = decode_id(id);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == NULL) 
  {
    fdb_table_unlock(table);
    return false;
  }
  bool res = fdb_bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  fdb_table_unlock(table);
  return res;
}



fdb_table_block_t* 
fdb_table_get_block(fdb_table_t* table, 
                entity_id_t block_id) 
{
  fdb_table_lock(table);
  FDB_ASSERT(block_id != FDB_INVALID_ID);
  fdb_table_block_t* block = (fdb_table_block_t*)fdb_btree_get(&table->m_blocks, block_id);
  fdb_table_unlock(table);
  return block;
}

void
fdb_table_set_component_destructor(fdb_table_t* table, void (*destr)(void *ptr))
{
  table->m_destructor = destr;
}

void
fdb_table_lock(fdb_table_t* table)
{
  fdb_mutex_lock(&table->m_mutex);
}

void
fdb_table_unlock(fdb_table_t* table)
{
  fdb_mutex_unlock(&table->m_mutex);
}

uint32_t
block_get_offset(uint32_t block_id, 
                 uint32_t chunk_size,
                 uint32_t stride)
{
  uint32_t chunk_id = block_id / chunk_size;
  return chunk_id % stride;
}
