

#include "table.h"
#include "../../common/memory/memory.h"
#include "../../common/platform.h"

#include <string.h>

namespace furious 
{

/**
 * @brief Represents a decoded id, split into the block id and the offset
 * within the block
 */
struct DecodedId 
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
static DecodedId 
decode_id(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  uint32_t block_id  = id / FURIOUS_TABLE_BLOCK_SIZE;
  uint32_t block_offset = id % FURIOUS_TABLE_BLOCK_SIZE; 
  return DecodedId{block_id, block_offset};
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
table_block_has_component(const table_block_t* block, 
                 entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  bool res =  table_block_get_component(block, id).p_data != nullptr;
  return res;
}

table_block_t 
table_block_create(entity_id_t start, 
                   size_t esize, 
                   mem_allocator_t* data_allocator, 
                   mem_allocator_t* bitmap_allocator)
{
  FURIOUS_ASSERT(data_allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((data_allocator->p_mem_alloc != nullptr && data_allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")

  FURIOUS_ASSERT(bitmap_allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((bitmap_allocator->p_mem_alloc != nullptr && bitmap_allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")

  table_block_t tblock;
  tblock.p_data = mem_alloc(data_allocator, 64, esize*FURIOUS_TABLE_BLOCK_SIZE, start / FURIOUS_TABLE_BLOCK_SIZE);
  tblock.m_start = start;
  tblock.m_num_components = 0;
  tblock.m_num_enabled_components = 0;
  tblock.m_esize = esize;
  tblock.m_exists = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, bitmap_allocator);
  tblock.m_enabled = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, bitmap_allocator);
  return tblock;
}


void
table_block_destroy(table_block_t* tblock, 
                    mem_allocator_t* data_allocator, 
                    mem_allocator_t* bitmap_allocator)
{

  FURIOUS_ASSERT(data_allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((data_allocator->p_mem_alloc != nullptr && data_allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")

  FURIOUS_ASSERT(bitmap_allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((bitmap_allocator->p_mem_alloc != nullptr && bitmap_allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")

  bitmap_destroy(&tblock->m_enabled, bitmap_allocator);
  bitmap_destroy(&tblock->m_exists, bitmap_allocator);
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
table_entry_t
table_block_get_component(const table_block_t* block, 
                 entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  assert(block->m_start == (id / FURIOUS_TABLE_BLOCK_SIZE) * FURIOUS_TABLE_BLOCK_SIZE) ;
  if(bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset)) 
  {
    return table_entry_t{block->m_start + decoded_id.m_block_offset, 
                         &(((char*)block->p_data)[decoded_id.m_block_offset*block->m_esize]), 
                        (bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset))};
  }
  return table_entry_t{0,nullptr,false};
}

tblock_iter_t
tblock_iter_create(table_block_t* tblock)
{
  tblock_iter_t tblock_iter;
  tblock_iter.p_block = tblock;
  tblock_iter.m_next_position = 0;
  if(tblock_iter.p_block != nullptr) {
    while(tblock_iter.m_next_position < FURIOUS_TABLE_BLOCK_SIZE && 
          !table_block_has_component(tblock_iter.p_block, 
                                     tblock_iter.p_block->m_start+tblock_iter.m_next_position) ) 
    {
      tblock_iter.m_next_position++;
    }
  }
  return tblock_iter;
}

void
tblock_iter_destroy(tblock_iter_t* tblock_iter)
{
  *tblock_iter = {0};
}

bool 
tblock_iter_has_next(tblock_iter_t* tblock_iter) 
{
  return  tblock_iter->p_block != nullptr && 
          tblock_iter->m_next_position < FURIOUS_TABLE_BLOCK_SIZE;
}

table_entry_t 
tblock_iter_next(tblock_iter_t* tblock_iter) 
{
  table_entry_t table_entry = table_block_get_component(tblock_iter->p_block, 
                                                        tblock_iter->p_block->m_start + 
                                                        tblock_iter->m_next_position);
  tblock_iter->m_next_position++; 
  while(tblock_iter->m_next_position < FURIOUS_TABLE_BLOCK_SIZE && 
        !table_block_has_component(tblock_iter->p_block, 
                       tblock_iter->p_block->m_start+tblock_iter->m_next_position)) 
  {
    tblock_iter->m_next_position++;
  }
  return table_entry;
}

void 
tblock_iter_reset(tblock_iter_t* tblock_iter, table_block_t* block) 
{
  *tblock_iter = tblock_iter_create(block);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

table_iter_t
table_iter_create(table_t* table)  
{
  table_iter_t iter;
  iter.p_blocks = &table->m_blocks;
  iter.m_it = btree_iter_create(iter.p_blocks);
  iter.m_chunk_size = 1;
  iter.m_offset = 0;
  iter.m_stride = 1;
  iter.m_next = nullptr;
  return iter;
}

table_iter_t
table_iter_create(table_t* table, 
                  uint32_t chunk_size,
                  uint32_t offset, 
                  uint32_t stride) 
{

  table_iter_t iter;
  iter.p_blocks = &table->m_blocks;
  iter.m_it = btree_iter_create(iter.p_blocks);
  iter.m_chunk_size = chunk_size;
  iter.m_offset = offset;
  iter.m_stride = stride;
  iter.m_next = nullptr;
  return iter;
}

void
table_iter_destroy(table_iter_t* iter)
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
table_iter_has_next(table_iter_t* iter) 
{ 
  while(btree_iter_has_next(&iter->m_it) && 
       (iter->m_next == nullptr))
  {
    btree_entry_t entry = btree_iter_next(&iter->m_it);
    bool is_next = is_selected(entry.m_key, 
                               iter->m_chunk_size, 
                               iter->m_offset, 
                               iter->m_stride);
    if(is_next)
    {
      iter->m_next = (table_block_t*)entry.p_value;
      break;
    }
    iter->m_next = nullptr;
  } 
  return iter->m_next != nullptr;;
}

table_block_t* 
table_iter_next(table_iter_t* iter) 
{
  table_block_t* next = nullptr;
  if(iter->m_next)
  {
    next = iter->m_next;
    iter->m_next = nullptr;
  }
  else
  {
    bool found = false;
    do 
    {
      next = (table_block_t*)btree_iter_next(&iter->m_it).p_value;
      if(next != nullptr)
      {
        uint32_t chunk_id = next->m_start / (FURIOUS_TABLE_BLOCK_SIZE);
        found = is_selected(chunk_id, 
                            iter->m_chunk_size, 
                            iter->m_offset, 
                            iter->m_stride);
      }
    } while (next != nullptr && !found);
  }
  return next;
}

table_t
table_create(const char* name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr))
{
  table_t table;
  table.m_id = id;
  table.m_esize = esize;
  table.m_num_components = 0;
  table.m_destructor = destructor;
  FURIOUS_COPY_AND_CHECK_STR(&table.m_name[0], name, FURIOUS_MAX_TABLE_NAME);

  table.m_tblock_allocator = pool_alloc_create(FURIOUS_TABLE_BLOCK_ALIGNMENT,
                                          sizeof(table_block_t), 
                                          FURIOUS_TABLE_BLOCK_PAGE_SIZE);

  table.m_data_allocator   = pool_alloc_create(FURIOUS_TABLE_BLOCK_DATA_ALIGNMENT, 
                                          esize*FURIOUS_TABLE_BLOCK_SIZE, 
                                          FURIOUS_TABLE_BLOCK_DATA_PAGE_SIZE);

  table.m_bitmap_allocator = pool_alloc_create(FURIOUS_BITMAP_ALIGNMENT, 
                                          FURIOUS_BITMAP_NUM_CHUNKS(FURIOUS_TABLE_BLOCK_SIZE), 
                                          FURIOUS_TABLE_BLOCK_BITMAP_PAGE_SIZE);

  table.m_btree_allocator = pool_alloc_create(FURIOUS_BITMAP_ALIGNMENT, 
                                         sizeof(btree_node_t), 
                                         FURIOUS_TABLE_BTREE_PAGE_SIZE);

  table.m_blocks = btree_create(&table.m_btree_allocator);
  table.m_mutex = mutex_create();
  return table;
}


void
table_destroy(table_t* table)
{
  table_clear(table);
  mutex_destroy(&table->m_mutex);
  btree_destroy(&table->m_blocks);

  pool_alloc_destroy(&table->m_btree_allocator);
  pool_alloc_destroy(&table->m_bitmap_allocator);
  pool_alloc_destroy(&table->m_data_allocator);
  pool_alloc_destroy(&table->m_tblock_allocator);
}

size_t
table_size(table_t* table)
{
  table_lock(table);
  size_t size =  table->m_num_components;
  table_release(table);
  return size;
}

void 
table_clear(table_t* table) 
{
  table_lock(table);
  table_iter_t titer = table_iter_create(table);
  while(table_iter_has_next(&titer)) 
  {
    table_block_t* block = table_iter_next(&titer);
    if(table->m_destructor != nullptr)
    {
      tblock_iter_t b_iterator = tblock_iter_create(block);
      while(tblock_iter_has_next(&b_iterator)) 
      {
        table->m_destructor(tblock_iter_next(&b_iterator).p_data);
      }
      tblock_iter_destroy(&b_iterator);
    }
    table_block_destroy(block, 
                        &table->m_data_allocator, 
                        &table->m_bitmap_allocator);

    mem_free(&table->m_tblock_allocator, 
             block);
  }
  table_iter_destroy(&titer);
  btree_destroy(&table->m_blocks);
  table->m_blocks = btree_create();
  table->m_num_components = 0;
  table_release(table);
}


void* 
table_get_component(table_t* table, 
                    entity_id_t id) 
{
  table_lock(table);
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  void* component = btree_get(&table->m_blocks, 
                              decoded_id.m_block_id);
  if(component == nullptr) 
  {
    table_release(table);
    return nullptr;
  }
  table_block_t* block = (table_block_t*)component;
  if(bitmap_is_set(&block->m_exists, decoded_id.m_block_offset))
  {
    table_release(table);
    return &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  }
  table_release(table);
  return nullptr;
}

void* 
table_alloc_component(table_t* table, 
                      entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, decoded_id.m_block_id);
  if (block == nullptr) 
  {
    block = (table_block_t*)mem_alloc(&table->m_tblock_allocator, 
                                      FURIOUS_TABLE_BLOCK_ALIGNMENT, 
                                      sizeof(table_block_t), 
                                      decoded_id.m_block_id);

    *block = table_block_create(decoded_id.m_block_id*FURIOUS_TABLE_BLOCK_SIZE,
                                table->m_esize, 
                                &table->m_data_allocator, 
                                &table->m_bitmap_allocator);

    btree_insert(&table->m_blocks, 
                 decoded_id.m_block_id,
                 block);
  }

  if(!bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    table->m_num_components++;
    block->m_num_components++;
    block->m_num_enabled_components++;
  }
  bitmap_set(&block->m_exists, decoded_id.m_block_offset);
  bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  return &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
}

void  
table_dealloc_component(table_t* table, 
                        entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  table_lock(table);
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == nullptr) {
    table_release(table);
    return;
  }

  if(bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    table->m_num_components--;
    block->m_num_components--;
    block->m_num_enabled_components--;
  }
  bitmap_unset(&block->m_exists, decoded_id.m_block_offset);
  bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  table_release(table);
  void* ptr =  &(((char*)block->p_data)[decoded_id.m_block_offset*table->m_esize]);
  table->m_destructor(ptr);
  memset(ptr, 0, table->m_esize);
}

void 
table_enable_component(table_t* table, 
                       entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  table_lock(table);
  DecodedId decoded_id = decode_id(id);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == nullptr)
  {
    table_release(table);
    return;
  }
  bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components++;
  table_release(table);
}

void 
table_disable_component(table_t* table, 
                        entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  table_lock(table);

  DecodedId decoded_id = decode_id(id);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == nullptr)
  {
    table_release(table);
    return;
  }
  bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components--;
  table_release(table);
}

bool 
table_is_enabled(table_t* table, 
                 entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  table_lock(table);

  DecodedId decoded_id = decode_id(id);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, 
                                                   decoded_id.m_block_id);
  if(block == nullptr) 
  {
    table_release(table);
    return false;
  }
  bool res = bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  table_release(table);
  return res;
}



table_block_t* 
table_get_block(table_t* table, 
                entity_id_t block_id) 
{
  table_lock(table);
  assert(block_id != FURIOUS_INVALID_ID);
  table_block_t* block = (table_block_t*)btree_get(&table->m_blocks, block_id);
  table_release(table);
  return block;
}

void
table_lock(table_t* table)
{
  mutex_lock(&table->m_mutex);
}

void
table_release(table_t* table)
{
  mutex_unlock(&table->m_mutex);
}

uint32_t
block_get_offset(uint32_t block_id, 
                 uint32_t chunk_size,
                 uint32_t stride)
{
  uint32_t chunk_id = block_id / chunk_size;
  return chunk_id % stride;
}
  
} /* furious */ 
