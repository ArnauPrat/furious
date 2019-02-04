

#include "table.h"

namespace furious {

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
decode_id(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  uint32_t block_id  = id / TABLE_BLOCK_SIZE;
  uint32_t block_offset = id % TABLE_BLOCK_SIZE; 
  return DecodedId{block_id, block_offset};
}

/**
 * @brief Tests if a given block contains the given element enabled
 *
 * @param block The block to check at
 * @param id The id of the element to check for
 *
 * @return true if the block contains the given id enabled. false otherwise
 */
bool 
has_element(const TBlock* block, 
                 uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  return get_element(block, id).p_data != nullptr;
}

/**
 * @brief Gets an element from the given block, if it exists
 *
 * @param block The block to get the element from
 * @param id The id to get
 *
 * @return The row of the table containing the retrieved element 
 */
TRow
get_element(const TBlock* block, 
                 uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  assert(block->m_start == (id / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE) ;
  if(block->p_enabled->is_set(decoded_id.m_block_offset)) 
  {
    return TRow{block->m_start + decoded_id.m_block_offset, 
               &block->p_data[decoded_id.m_block_offset*block->m_esize], 
               (block->p_enabled->is_set(decoded_id.m_block_offset))};
  }
  return TRow{0,nullptr,false};
}

TBlockIterator::TBlockIterator(TBlock* block) : 
p_block(block),
m_next_position(0) 
{
  if(p_block != nullptr) {
    while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

bool 
TBlockIterator::has_next() const 
{
  return p_block != nullptr && m_next_position < TABLE_BLOCK_SIZE;
}

TRow 
TBlockIterator::next() 
{
  TRow row = get_element(p_block, p_block->m_start+m_next_position);
  m_next_position++; 
  while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
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
    while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

Table::Iterator::Iterator(const BTree<TBlock>* blocks) : 
  p_blocks(blocks),
  m_it(p_blocks->iterator())
{
}



bool 
Table::Iterator::has_next() const 
{ 
  return m_it.has_next();
}

TBlock* 
Table::Iterator::next() 
{
  TBlock* next = m_it.next();
  return next;
}

Table::Table(const std::string& name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr)) :
m_name(name),
m_id(id),
m_esize(esize),
m_num_elements(0),
m_destructor(destructor) 
{
}

Table::Table(std::string&& name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr)) :
m_name(name),
m_id(id),
m_esize(esize),
m_num_elements(0),
m_destructor(destructor)
{
}

Table::~Table() {
  clear();
}

size_t
Table::size() const {
  return m_num_elements;
}

void 
Table::clear() 
{
  Iterator iterator(&m_blocks);
  while(iterator.has_next()) 
  {
    TBlock* block = iterator.next();
    TBlockIterator b_iterator{block};
    while(b_iterator.has_next()) {
      m_destructor(b_iterator.next().p_data);
    }
    numa_free(block->p_data);
    delete block->p_enabled;
    delete block->p_exists;
    delete block;
  }
  m_blocks.clear();
  m_num_elements = 0;
}


void* 
Table::get_element(uint32_t id) const 
{
  assert(id != FURIOUS_INVALID_ID);
  DecodedId decoded_id = decode_id(id);
  void* element = m_blocks.get(decoded_id.m_block_id);
  if(element == nullptr) 
  {
    return nullptr;
  }
  TBlock* block = (TBlock*)element;
  if(block->p_exists->is_set(decoded_id.m_block_offset))
  {
    return &block->p_data[decoded_id.m_block_offset*m_esize];
  }
  return nullptr;
}

void* 
Table::alloc_element(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if (block == nullptr) 
  {
    block = new TBlock();
    block->p_data = static_cast<int8_t*>(numa_alloc(0, m_esize*TABLE_BLOCK_SIZE ));
    block->m_start = (id / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE;
    block->m_num_elements = 0;
    block->m_num_enabled_elements = 0;
    block->m_esize = m_esize;
    block->p_enabled = new Bitmap(TABLE_BLOCK_SIZE);
    block->p_exists = new Bitmap(TABLE_BLOCK_SIZE);
    m_blocks.insert(decoded_id.m_block_id, block);
  }

  if(!block->p_exists->is_set(decoded_id.m_block_offset)) 
  {
    m_num_elements++;
    block->m_num_elements++;
    block->m_num_enabled_elements++;
  }
  block->p_exists->set(decoded_id.m_block_offset);
  block->p_enabled->set(decoded_id.m_block_offset);
  return &block->p_data[decoded_id.m_block_offset*m_esize];
}

void  
Table::remove_element(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr) {
    return;
  }

  if(block->p_exists->is_set(decoded_id.m_block_offset)) 
  {
    m_num_elements--;
    block->m_num_elements--;
    block->m_num_enabled_elements--;
  }
  block->p_exists->unset(decoded_id.m_block_offset);
  block->p_enabled->unset(decoded_id.m_block_offset);
  m_destructor(&block->p_data[decoded_id.m_block_offset*m_esize]);
  memset(&block->p_data[decoded_id.m_block_offset*m_esize], '\0', m_esize);
}

void 
Table::enable_element(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr)
  {
    return;
  }
  block->p_enabled->set(decoded_id.m_block_offset);
  block->m_num_enabled_elements++;
}

void 
Table::disable_element(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr)
  {
    return;
  }
  block->p_enabled->unset(decoded_id.m_block_offset);
  block->m_num_enabled_elements--;
}

bool 
Table::is_enabled(uint32_t id) 
{
  assert(id != FURIOUS_INVALID_ID);

  DecodedId decoded_id = decode_id(id);
  TBlock* block = m_blocks.get(decoded_id.m_block_id);
  if(block == nullptr) 
  {
    return false;
  }
  return block->p_enabled->is_set(decoded_id.m_block_offset);
}

Table::Iterator 
Table::iterator() 
{
  return Iterator(&m_blocks);
}

std::string 
Table::name() const 
{
  return m_name;
}

TBlock* 
Table::get_block(uint32_t block_id) 
{
  assert(block_id != FURIOUS_INVALID_ID);
  TBlock* block = m_blocks.get(block_id);
  assert(block != nullptr);
  return block;
}
  
} /* furious */ 
