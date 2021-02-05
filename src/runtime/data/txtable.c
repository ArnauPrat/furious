
#include "txtable.h"
#include "../../common/memory/memory.h"
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
fdb_txtable_block_has_component(struct fdb_txtable_block_t* block, 
                                struct fdb_tx_t* tx, 
                                struct fdb_txthread_ctx_t* txtctx, 
                                entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  bool res =  fdb_txtable_block_get_component(block, 
                                              tx, 
                                              txtctx,  
                                              id, 
                                              false).p_data != NULL;
  return res;
}

void 
fdb_txtable_factory_init(struct fdb_txtable_factory_t* table_factory, 
                         struct fdb_mem_allocator_t* mem_allocator)
{
  *table_factory = (struct fdb_txtable_factory_t){.p_allocator = mem_allocator};

  table_factory->p_tblock_allocator = fdb_txpool_alloc_create(FDB_TXTABLE_BLOCK_ALIGNMENT, 
                                                              sizeof(struct fdb_txtable_block_t), 
                                                              FDB_TXTABLE_BLOCK_PAGE_SIZE,
                                                              table_factory->p_allocator);

  fdb_txbitmap_factory_init(&table_factory->m_bitmap_factory, 
                            table_factory->p_allocator, 
                            FDB_TXTABLE_BLOCK_SIZE); 
  fdb_txbtree_factory_init(&table_factory->m_btree_factory, 
                           table_factory->p_allocator);
  
}

void
fdb_txtable_factory_release(struct fdb_txtable_factory_t* table_factory,
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txbtree_factory_release(&table_factory->m_btree_factory, 
                              tx, 
                              txtctx);
  fdb_txbitmap_factory_release(&table_factory->m_bitmap_factory, 
                               tx, 
                               txtctx);
  fdb_txpool_alloc_destroy(table_factory->p_tblock_allocator, 
                           tx, 
                           txtctx);
}

void
fdb_txtable_block_init(struct fdb_txtable_block_t* tblock, 
                       struct fdb_txtable_t* table, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx, 
                       entity_id_t start, 
                       size_t esize)
{

  *tblock  = (struct fdb_txtable_block_t){};
  tblock->p_table = table;
  tblock->m_data = fdb_txpool_alloc_alloc(table->p_data_allocator, 
                                          tx, 
                                          txtctx, 
                                          FDB_TXTABLE_BLOCK_DATA_ALIGNMENT, 
                                          esize*FDB_TXTABLE_BLOCK_SIZE, 
                                          FDB_NO_HINT);
  tblock->m_start = start;
  tblock->m_num_components = 0;
  tblock->m_num_enabled_components = 0;
  tblock->m_esize = esize;
  fdb_txbitmap_init(&tblock->m_exists, 
                    &table->p_factory->m_bitmap_factory, 
                    tx, 
                    txtctx);
  fdb_txbitmap_init(&tblock->m_enabled, 
                    &table->p_factory->m_bitmap_factory, 
                    tx, 
                    txtctx);
}


void
fdb_txtable_block_release(struct fdb_txtable_block_t* tblock, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txbitmap_release(&tblock->m_enabled, 
                       tx, 
                       txtctx);
  fdb_txbitmap_release(&tblock->m_exists, 
                       tx,
                       txtctx);
  fdb_txpool_alloc_free(tblock->p_table->p_data_allocator, 
                        tx, 
                        txtctx,
                        tblock->m_data);
}

/**
 * @brief Gets an component from the given block, if it exists
 *
 * @param block The block to get the component from
 * @param id The id to get
 *
 * @return The row of the table containing the retrieved component 
 */
struct fdb_txtable_entry_t
fdb_txtable_block_get_component(struct fdb_txtable_block_t* block, 
                                struct fdb_tx_t* tx, 
                                struct fdb_txthread_ctx_t* txtctx,
                                entity_id_t id, 
                                bool dirty) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  assert(block->m_start == (id / FDB_TXTABLE_BLOCK_SIZE) * FDB_TXTABLE_BLOCK_SIZE) ;

  struct fdb_txtable_entry_t entry;  
  if(fdb_txbitmap_is_set(&block->m_enabled, tx, txtctx, decoded_id.m_block_offset)) 
  {
    entry.m_id = block->m_start + decoded_id.m_block_offset;
    void* data = fdb_txpool_alloc_ptr(block->p_table->p_data_allocator, 
                                      tx, 
                                      txtctx, 
                                      block->m_data, 
                                      dirty);
    entry.p_data = &(((char*)data)[decoded_id.m_block_offset*block->m_esize]);
    entry.m_enabled = fdb_txbitmap_is_set(&block->m_enabled, 
                                          tx, 
                                          txtctx,
                                          decoded_id.m_block_offset);
  }
  else
  {
    memset(&entry, 0, sizeof(struct fdb_txtable_entry_t));
  }
  return entry;
}

void
fdb_txtblock_iter_init(struct fdb_txtblock_iter_t* tblock_iter, 
                       struct fdb_txtable_t* table, 
                       struct fdb_txpool_alloc_ref_t tblock_ref, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx, 
                       bool write)
{
  *tblock_iter = (struct fdb_txtblock_iter_t){};
  tblock_iter->m_block_ref = tblock_ref;
  tblock_iter->p_table = table;
  tblock_iter->p_tx = tx;
  tblock_iter->p_txtctx = txtctx;
  tblock_iter->m_next_position = 0;
  tblock_iter->m_write = write;
  if(tblock_iter->m_block_ref.p_main != NULL) {
    struct fdb_txtable_block_t* tblock = fdb_txpool_alloc_ptr(tblock_iter->p_table->p_factory->p_tblock_allocator, 
                                                              tblock_iter->p_tx, 
                                                              tblock_iter->p_txtctx, 
                                                              tblock_iter->m_block_ref, 
                                                              false);
    while(tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE && 
          !fdb_txtable_block_has_component(tblock, 
                                           tx, 
                                           txtctx,
                                           tblock->m_start+tblock_iter->m_next_position) ) 
    {
      tblock_iter->m_next_position++;
    }
  }
}

void
fdb_txtblock_iter_release(struct fdb_txtblock_iter_t* tblock_iter)
{
}

bool 
fdb_txtblock_iter_has_next(struct fdb_txtblock_iter_t* tblock_iter) 
{
    if(tblock_iter->m_block_ref.p_main == NULL) return false;
    return tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE;
}

struct fdb_txtable_entry_t 
fdb_txtblock_iter_next(struct fdb_txtblock_iter_t* tblock_iter) 
{
  struct fdb_txtable_block_t* tblock = fdb_txpool_alloc_ptr(tblock_iter->p_table->p_factory->p_tblock_allocator, 
                                                     tblock_iter->p_tx, 
                                                     tblock_iter->p_txtctx, 
                                                     tblock_iter->m_block_ref, 
                                                     false);
  struct fdb_txtable_entry_t fdb_table_entry = fdb_txtable_block_get_component(tblock, 
                                                                        tblock_iter->p_tx,
                                                                        tblock_iter->p_txtctx,
                                                                        tblock->m_start + 
                                                                        tblock_iter->m_next_position, 
                                                                        tblock_iter->m_write);
  tblock_iter->m_next_position++; 
  while(tblock_iter->m_next_position < FDB_TXTABLE_BLOCK_SIZE && 
        !fdb_txtable_block_has_component(tblock, 
                                         tblock_iter->p_tx, 
                                         tblock_iter->p_txtctx,
                                         tblock->m_start+tblock_iter->m_next_position)) 
  {
    tblock_iter->m_next_position++;
  }
  return fdb_table_entry;
}

void
fdb_txtblock_iter_reset(struct fdb_txtblock_iter_t* iter, 
                        struct fdb_txtable_t* table, 
                        struct fdb_txpool_alloc_ref_t block_ref, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        bool write)
{
  fdb_txtblock_iter_release(iter);
  fdb_txtblock_iter_init(iter, 
                         table, 
                         block_ref, 
                         tx, 
                         txtctx, 
                         write);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fdb_txtable_iter_init(struct fdb_txtable_iter_t* iter, 
                      struct fdb_txtable_t* table,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, 
                      uint32_t chunk_size,
                      uint32_t offset, 
                      uint32_t stride, 
                      bool write)
{
  *iter = (struct fdb_txtable_iter_t){};
  iter->p_table = table;
  iter->p_tx = tx;
  iter->p_txtctx = txtctx;
  fdb_txbtree_iter_init(&iter->m_it, 
                        &iter->p_table->m_blocks, 
                        tx, 
                        txtctx);
  iter->m_chunk_size = chunk_size;
  iter->m_offset = offset;
  iter->m_stride = stride;
  iter->m_next = (struct fdb_txpool_alloc_ref_t){.p_main = NULL};
  iter->m_write = write;
}

void
fdb_txtable_iter_release(struct fdb_txtable_iter_t* iter)
{
  fdb_txbtree_iter_release(&iter->m_it);
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
fdb_txtable_iter_has_next(struct fdb_txtable_iter_t* iter) 
{ 
  while(fdb_txbtree_iter_has_next(&iter->m_it) && 
       (iter->m_next.p_main == NULL))
  {
    struct fdb_txbtree_entry_t entry = fdb_txbtree_iter_next(&iter->m_it);
    bool is_next = is_selected(entry.m_key, 
                               iter->m_chunk_size, 
                               iter->m_offset, 
                               iter->m_stride);
    if(is_next)
    {
      iter->m_next = entry.m_value_ref;
      break;
    }
    iter->m_next.p_main = NULL;
  } 
  return iter->m_next.p_main != NULL;;
}

struct fdb_txtable_block_t* 
fdb_txtable_iter_next(struct fdb_txtable_iter_t* iter) 
{
  struct fdb_txtable_block_t* block = NULL;
  if(iter->m_next.p_main != NULL)
  {
    block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(iter->p_table->p_factory->p_tblock_allocator, 
                                                       iter->p_tx, 
                                                       iter->p_txtctx, 
                                                       iter->m_next, 
                                                       false);
    iter->m_next.p_main = NULL;
  }
  else
  {
    bool found = false;
    struct fdb_txpool_alloc_ref_t next = {.p_main = NULL};
    do 
    {
      block = NULL;
      next = fdb_txbtree_iter_next(&iter->m_it).m_value_ref;
      if(next.p_main != NULL)
      {
        block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(iter->p_table->p_factory->p_tblock_allocator, 
                                                                  iter->p_tx, 
                                                                  iter->p_txtctx, 
                                                                  next, 
                                                                  false);
        uint32_t chunk_id = block->m_start / (FDB_TXTABLE_BLOCK_SIZE);
        found = is_selected(chunk_id, 
                            iter->m_chunk_size, 
                            iter->m_offset, 
                            iter->m_stride);
      }
    } while (next.p_main != NULL && !found);
  }
  return block;
}

void
fdb_txtable_init(struct fdb_txtable_t* table, 
               struct fdb_txtable_factory_t* table_factory, 
               struct fdb_tx_t* tx, 
               struct fdb_txthread_ctx_t* txtctx, 
               const char* name, 
               int64_t id, 
               size_t esize, 
               void (*destructor)(void* ptr), 
               struct fdb_mem_allocator_t* allocator)
{
  table->p_factory = table_factory;
  table->m_id = id;
  table->m_esize = esize;
  table->m_destructor = destructor;
  FDB_COPY_AND_CHECK_STR(&table->m_name[0], name, FDB_MAX_TABLE_NAME);

  table->p_data_allocator =  fdb_txpool_alloc_create(FDB_TXTABLE_BLOCK_DATA_ALIGNMENT, 
                                                     esize*FDB_TXTABLE_BLOCK_SIZE, 
                                                     FDB_TXTABLE_BLOCK_DATA_PAGE_SIZE, 
                                                     allocator);

  fdb_txbtree_init(&table->m_blocks, 
                   &table->p_factory->m_btree_factory, 
                   tx, 
                   txtctx);
}


void
fdb_txtable_release(struct fdb_txtable_t* table, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txbtree_release(&table->m_blocks, 
                      tx, 
                      txtctx);

  fdb_txpool_alloc_destroy(table->p_data_allocator, 
                           tx, 
                           txtctx);
}

void 
fdb_txtable_clear(struct fdb_txtable_t* table, 
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx) 
{
  struct fdb_txbtree_iter_t iter;
  fdb_txbtree_iter_init(&iter,
                        &table->m_blocks, 
                        tx, 
                        txtctx);
  while(fdb_txbtree_iter_has_next(&iter)) 
  {
    struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_iter_next(&iter).m_value_ref;
    if(table->m_destructor)
    {
      struct fdb_txtblock_iter_t b_iterator;
      fdb_txtblock_iter_init(&b_iterator, 
                             table, 
                             block_ref, 
                             tx, 
                             txtctx, 
                             true);
      while(fdb_txtblock_iter_has_next(&b_iterator)) 
      {
        void* data = fdb_txtblock_iter_next(&b_iterator).p_data;
        table->m_destructor(data);
      }
      fdb_txtblock_iter_release(&b_iterator);
    }
    struct fdb_txtable_block_t* block = fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                      tx, 
                                                      txtctx, 
                                                      block_ref, 
                                                      true);
    fdb_txtable_block_release(block, 
                              tx, 
                              txtctx);

    fdb_txpool_alloc_free(table->p_factory->p_tblock_allocator, 
                          tx, 
                          txtctx,
                          block_ref);
  }
  fdb_txbtree_iter_release(&iter);
  fdb_txbtree_clear(&table->m_blocks, 
                    tx, 
                    txtctx);
}


void* 
fdb_txtable_get_component(struct fdb_txtable_t* table, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t id, 
                          bool write) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_get(&table->m_blocks, 
                                                         tx, 
                                                         txtctx,
                                                         decoded_id.m_block_id);
  if(block_ref.p_main == NULL) 
  {
    return NULL;
  }
  struct fdb_txtable_block_t* block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                                          tx, 
                                                                          txtctx, 
                                                                          block_ref, 
                                                                          false);
  if(fdb_txbitmap_is_set(&block->m_exists, tx, txtctx, decoded_id.m_block_offset))
  {
    void* data = fdb_txpool_alloc_ptr(table->p_data_allocator, 
                                      tx, 
                                      txtctx, 
                                      block->m_data, 
                                      write);
    return &(((char*)data)[decoded_id.m_block_offset*table->m_esize]);
  }
  return NULL;
}

void* 
fdb_txtable_create_component(struct fdb_txtable_t* table, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx,
                             entity_id_t id) 
{
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_get(&table->m_blocks, 
                                                     tx, 
                                                     txtctx, 
                                                     decoded_id.m_block_id);
  struct fdb_txtable_block_t* block = NULL;

  if (block_ref.p_main == NULL) 
  {
    block_ref = fdb_txpool_alloc_alloc(table->p_factory->p_tblock_allocator, 
                                       tx, 
                                       txtctx, 
                                       FDB_TXTABLE_BLOCK_ALIGNMENT, 
                                       sizeof(struct fdb_txtable_block_t), 
                                       decoded_id.m_block_id);

    block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                              tx, 
                                                              txtctx, 
                                                              block_ref, 
                                                              true);

    fdb_txtable_block_init(block, 
                           table, 
                           tx, 
                           txtctx, 
                           decoded_id.m_block_id*FDB_TXTABLE_BLOCK_SIZE,
                           table->m_esize);

    fdb_txbtree_insert(&table->m_blocks, 
                       tx, 
                       txtctx,
                       decoded_id.m_block_id,
                       block_ref);
  }
  else
  {
    block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                              tx, 
                                                              txtctx, 
                                                              block_ref, 
                                                              true);
  }

  if(!fdb_txbitmap_is_set(&block->m_exists, 
                          tx, 
                          txtctx, 
                          decoded_id.m_block_offset)) 
  {
    block->m_num_components++;
    block->m_num_enabled_components++;
  }
  fdb_txbitmap_set(&block->m_exists, 
                   tx, 
                   txtctx, 
                   decoded_id.m_block_offset);
  fdb_txbitmap_set(&block->m_enabled, 
                   tx, 
                   txtctx, 
                   decoded_id.m_block_offset);
  void* data = fdb_txpool_alloc_ptr(table->p_data_allocator, 
                                    tx, 
                                    txtctx, 
                                    block->m_data, 
                                    true);
  return &(((char*)data)[decoded_id.m_block_offset*table->m_esize]);
}

void  
fdb_txtable_destroy_component(struct fdb_txtable_t* table, 
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx,
                              entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_txpool_alloc_ref_t block_ref =fdb_txbtree_get(&table->m_blocks, 
                                                    tx, 
                                                    txtctx,
                                                    decoded_id.m_block_id);

  if(block_ref.p_main == NULL) {
    return;
  }

  struct fdb_txtable_block_t* block 
          = (struct fdb_txtable_block_t*) fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                               tx, 
                                                               txtctx, 
                                                               block_ref, 
                                                               true);

  if(fdb_txbitmap_is_set(&block->m_exists, 
                         tx, 
                         txtctx, 
                         decoded_id.m_block_offset)) 
  {
    block->m_num_components--;
    block->m_num_enabled_components--;
  }
  fdb_txbitmap_unset(&block->m_exists, 
                     tx, 
                     txtctx, 
                     decoded_id.m_block_offset);
  fdb_txbitmap_unset(&block->m_enabled, 
                     tx, 
                     txtctx, 
                     decoded_id.m_block_offset);

  void* data = fdb_txpool_alloc_ptr(table->p_data_allocator,
                                    tx, 
                                    txtctx, 
                                    block->m_data, 
                                    true);
  void* ptr =  &(((char*)data)[decoded_id.m_block_offset*table->m_esize]);
  if(table->m_destructor)
  {
    table->m_destructor(ptr);
  }
  memset(ptr, 0, table->m_esize);
}

void 
fdb_txtable_enable_component(struct fdb_txtable_t* table, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);
  struct fdb_decoded_id_t decoded_id = decode_id(id);
  struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_get(&table->m_blocks, 
                                                     tx, 
                                                     txtctx, 
                                                     decoded_id.m_block_id);
  if(block_ref.p_main == NULL)
  {
    return;
  }

  struct fdb_txtable_block_t* block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                                                        tx, 
                                                                                        txtctx, 
                                                                                        block_ref, 
                                                                                        true);
  fdb_txbitmap_set(&block->m_enabled, 
                   tx, 
                   txtctx, 
                   decoded_id.m_block_offset);
  block->m_num_enabled_components++;
}

void 
fdb_txtable_disable_component(struct fdb_txtable_t* table, 
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx,
                              entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  struct fdb_decoded_id_t decoded_id = decode_id(id);

  struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_get(&table->m_blocks, 
                                                     tx, 
                                                     txtctx, 
                                                     decoded_id.m_block_id);
  if(block_ref.p_main == NULL)
  {
    return;
  }

  struct fdb_txtable_block_t* block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                                                        tx, 
                                                                                        txtctx, 
                                                                                        block_ref, 
                                                                                        true);
  fdb_txbitmap_unset(&block->m_enabled, 
                     tx, 
                     txtctx, 
                     decoded_id.m_block_offset);
  block->m_num_enabled_components--;
}

bool 
fdb_txtable_is_enabled(struct fdb_txtable_t* table, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx,
                       entity_id_t id) 
{
  assert(id != FDB_INVALID_ID);

  struct fdb_decoded_id_t decoded_id = decode_id(id);

  struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_get(&table->m_blocks, 
                                                     tx, 
                                                     txtctx, 
                                                     decoded_id.m_block_id);
  if(block_ref.p_main == NULL)
  {
    return false;
  }

  struct fdb_txtable_block_t* block = (struct fdb_txtable_block_t*)fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                                                        tx, 
                                                                                        txtctx, 
                                                                                        block_ref, 
                                                                                        false);
  bool res = fdb_txbitmap_is_set(&block->m_enabled, 
                                 tx, 
                                 txtctx, 
                                 decoded_id.m_block_offset);
  return res;
}



struct fdb_txpool_alloc_ref_t 
fdb_txtable_get_block(struct fdb_txtable_t* table, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx,
                      entity_id_t block_id) 
{
  FDB_ASSERT(block_id != FDB_INVALID_ID);
  return fdb_txbtree_get(&table->m_blocks, 
                         tx, 
                         txtctx, 
                         block_id);
}

void
fdb_txtable_set_component_destructor(struct fdb_txtable_t* table, void (*destr)(void *ptr))
{
  table->m_destructor = destr;
}

size_t
fdb_txtable_size(struct fdb_txtable_t* table,
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx)
{
  size_t ret = 0;
  struct fdb_txbtree_iter_t btree_iter;
  fdb_txbtree_iter_init(&btree_iter, &table->m_blocks, tx, txtctx);
  while(fdb_txbtree_iter_has_next(&btree_iter))
  {
    struct fdb_txpool_alloc_ref_t block_ref = fdb_txbtree_iter_next(&btree_iter).m_value_ref;
    struct fdb_txtable_block_t* tblock = fdb_txpool_alloc_ptr(table->p_factory->p_tblock_allocator, 
                                                              tx, 
                                                              txtctx, 
                                                              block_ref, 
                                                              false);
    ret += tblock->m_num_components;
    
  }
  return ret;
}

