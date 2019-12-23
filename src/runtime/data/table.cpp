

#include "table.h"
#include "../../common/memory/memory.h"

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
has_component(const TBlock* block, 
                 entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  bool res =  get_component(block, id).p_data != nullptr;
  return res;
}

TBlock::TBlock(entity_id_t start, size_t esize) :
    p_data(static_cast<char*>(mem_alloc(&global_mem_allocator, 64, esize*FURIOUS_TABLE_BLOCK_SIZE, start / FURIOUS_TABLE_BLOCK_SIZE))),
    m_start(start),
    m_num_components(0),
    m_num_enabled_components(0),
    m_esize(esize)
{
  m_exists = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, &global_mem_allocator);
  m_enabled = bitmap_create(FURIOUS_TABLE_BLOCK_SIZE, &global_mem_allocator);
}

TBlock::~TBlock()
{
  bitmap_destroy(&m_enabled, &global_mem_allocator);
  bitmap_destroy(&m_exists, &global_mem_allocator);
  mem_free(&global_mem_allocator, 
           p_data);
}

/**
 * @brief Gets an component from the given block, if it exists
 *
 * @param block The block to get the component from
 * @param id The id to get
 *
 * @return The row of the table containing the retrieved component 
 */
TRow
get_component(const TBlock* block, 
                 entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  assert(block->m_start == (id / FURIOUS_TABLE_BLOCK_SIZE) * FURIOUS_TABLE_BLOCK_SIZE) ;
  if(bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset)) 
  {
    return TRow{block->m_start + decoded_id.m_block_offset, 
               &(((char*)block->p_data)[decoded_id.m_block_offset*block->m_esize]), 
               (bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset))};
  }
  return TRow{0,nullptr,false};
}

TBlockIterator::TBlockIterator(TBlock* block) : 
p_block(block),
m_next_position(0) 
{
  if(p_block != nullptr) {
    while(m_next_position < FURIOUS_TABLE_BLOCK_SIZE && !has_component(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

bool 
TBlockIterator::has_next() const 
{
  return p_block != nullptr && m_next_position < FURIOUS_TABLE_BLOCK_SIZE;
}

TRow 
TBlockIterator::next() 
{
  TRow row = get_component(p_block, p_block->m_start+m_next_position);
  m_next_position++; 
  while(m_next_position < FURIOUS_TABLE_BLOCK_SIZE && !has_component(p_block, p_block->m_start+m_next_position) ) {
    m_next_position++;
  }
  return row;
}

void 
TBlockIterator::reset(TBlock* block) 
{
  p_block = block;
  m_next_position = 0;
  if(p_block != nullptr) {
    while(m_next_position < FURIOUS_TABLE_BLOCK_SIZE && !has_component(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

Table::Iterator::Iterator(const BTree<TBlock>* blocks) : 
  p_blocks(blocks),
  m_it(p_blocks->iterator()),
  m_chunk_size(1),
  m_offset(0),
  m_stride(1),
  m_next(nullptr)
{
}

Table::Iterator::Iterator(const BTree<TBlock>* blocks, 
                          uint32_t chunk_size,
                          uint32_t offset, 
                          uint32_t stride) : 
  p_blocks(blocks),
  m_it(p_blocks->iterator()),
  m_chunk_size(chunk_size),
  m_offset(offset),
  m_stride(stride),
  m_next(nullptr)
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
Table::Iterator::has_next() const 
{ 
  while(m_it.has_next() && (m_next == nullptr))
  {
    BTree<TBlock>::Entry entry = m_it.next();
    bool is_next = is_selected(entry.m_key, m_chunk_size, m_offset, m_stride);
    if(is_next)
    {
      m_next = entry.p_value;
      break;
    }
    m_next = nullptr;
  } 
  return m_next != nullptr;;
}

TBlock* 
Table::Iterator::next() 
{
  TBlock* next = nullptr;
  if(m_next)
  {
    next = m_next;
    m_next = nullptr;
  }
  else
  {
    bool found = false;
    do 
    {
      next = m_it.next().p_value;
      if(next != nullptr)
      {
        uint32_t chunk_id = next->m_start / (FURIOUS_TABLE_BLOCK_SIZE);
        //found = (((chunk_id / m_chunk_size) - m_offset) % m_stride) == 0;
        found = is_selected(chunk_id, m_chunk_size, m_offset, m_stride);
        //found = (((chunk_id / m_chunk_size) + (m_chunk_size - m_offset)) % m_stride) == 0;
      }
    } while (next != nullptr && !found);
  }
  return next;
}

Table::Table(const char* name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr)) :
m_id(id),
m_esize(esize),
m_num_components(0),
m_destructor(destructor) 
{
  size_t name_length =strlen(name)+1;
  p_name = (char*)mem_alloc(&global_mem_allocator, 
                            64, sizeof(char)*name_length, 0);
  strncpy(p_name, name, name_length);
}


Table::~Table() {
  clear();
  mem_free(&global_mem_allocator, 
           p_name);
}

size_t
Table::size() const 
{
  lock();
  size_t size =  m_num_components;
  release();
  return size;
}

void 
Table::clear() 
{
  lock();
  if(m_destructor != nullptr)
  {
    Iterator iterator(&m_blocks);
    while(iterator.has_next()) 
    {
      TBlock* block = iterator.next();
      TBlockIterator b_iterator{block};
      while(b_iterator.has_next()) 
      {
        m_destructor(b_iterator.next().p_data);
      }
    }
  }
  m_blocks.clear();
  m_num_components = 0;
  release();
}


void* 
Table::get_component(entity_id_t id) const 
{
  lock();
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  void* component = m_blocks.get(decoded_id.m_block_id);
  if(component == nullptr) 
  {
    release();
    return nullptr;
  }
  TBlock* block = (TBlock*)component;
  if(bitmap_is_set(&block->m_exists, decoded_id.m_block_offset))
  {
    release();
    return &(((char*)block->p_data)[decoded_id.m_block_offset*m_esize]);
  }
  release();
  return nullptr;
}

void* 
Table::alloc_component(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if (block == nullptr) 
  {
    block = m_blocks.insert_new(decoded_id.m_block_id,
                                decoded_id.m_block_id*FURIOUS_TABLE_BLOCK_SIZE,
                                m_esize);
  }

  if(!bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    m_num_components++;
    block->m_num_components++;
    block->m_num_enabled_components++;
  }
  bitmap_set(&block->m_exists, decoded_id.m_block_offset);
  bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  return &(((char*)block->p_data)[decoded_id.m_block_offset*m_esize]);
}

void* 
Table::dealloc_component(entity_id_t id)
{
  lock();
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr) {
    release();
    return nullptr;
  }

  if(bitmap_is_set(&block->m_exists, decoded_id.m_block_offset)) 
  {
    m_num_components--;
    block->m_num_components--;
    block->m_num_enabled_components--;
  }
  bitmap_unset(&block->m_exists, decoded_id.m_block_offset);
  bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  release();
  return &(((char*)block->p_data)[decoded_id.m_block_offset*m_esize]);
}

void  
Table::dealloc_and_destroy_component(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  void* ptr = dealloc_component(id);
  if(ptr != nullptr)
  {
    m_destructor(ptr);
    memset(ptr, 0, m_esize);
  }
}

void 
Table::enable_component(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  lock();
  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr)
  {
    release();
    return;
  }
  bitmap_set(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components++;
  release();
}

void 
Table::disable_component(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  lock();

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr)
  {
    release();
    return;
  }
  bitmap_unset(&block->m_enabled, decoded_id.m_block_offset);
  block->m_num_enabled_components--;
  release();
}

bool 
Table::is_enabled(entity_id_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  lock();

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr) 
  {
    release();
    return false;
  }
  bool res = bitmap_is_set(&block->m_enabled, decoded_id.m_block_offset);
  release();
  return res;
}

Table::Iterator 
Table::iterator() 
{
  return Iterator(&m_blocks);
}

Table::Iterator 
Table::iterator(uint32_t chunk_size, 
                uint32_t offset,
                uint32_t stride) 
{
  return Iterator(&m_blocks, chunk_size, offset, stride);
}

const char* 
Table::name() const 
{
  return p_name;
}

TBlock* 
Table::get_block(entity_id_t block_id) 
{
  lock();
  assert(block_id != FURIOUS_INVALID_ID);
  TBlock* block = m_blocks.get(block_id);
  release();
  return block;
}

void
Table::lock() const
{
  m_mutex.lock();
}

void
Table::release() const
{
  m_mutex.unlock();
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
