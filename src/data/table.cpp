

#include "table.h"
#include <cmath>
#include <cstring>

namespace furious {

  


uint8_t bitmap_masks[8] = {0x01, 
                           0x02,
                           0x04,
                           0x08,
                           0x10,
                           0x20,
                           0x40,
                           0x80
};

struct DecodedId {
  const int32_t m_block_id;
  const int32_t m_block_offset;
  const int32_t m_bitmap_offset;
  const int32_t m_bitmap_mask;
};

static DecodedId decode_id(int32_t id) { 
  
  int32_t block_id  = id / TABLE_BLOCK_SIZE;

  int32_t block_offset = id % TABLE_BLOCK_SIZE; 

  int32_t bitmap_offset = block_offset / (sizeof(int8_t)*8);

  int32_t bitmap_mask = bitmap_masks[block_offset % (sizeof(int8_t)*8)];

  return DecodedId{block_id, block_offset, bitmap_offset, bitmap_mask};
}


bool has_element(const TBlock* block, int32_t id) {
  return get_element(block, id).p_data != nullptr;
}

TRow get_element(const TBlock* block, int32_t id) {
  DecodedId decoded_id = decode_id(id);
  assert(block->m_start == (id / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE) ;
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00) {
    return TRow{block->m_start + decoded_id.m_block_offset, 
               &block->p_data[decoded_id.m_block_offset*block->m_esize], 
               (block->m_enabled[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00};
  }
  return TRow{0,nullptr,false};
}

TBlockIterator::TBlockIterator(TBlock* block) : p_block(block),
m_next_position(0) {
  if(p_block != nullptr) {
    while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

bool TBlockIterator::has_next() const {
  return p_block != nullptr && m_next_position < TABLE_BLOCK_SIZE;
}

TRow TBlockIterator::next() {
  TRow row = get_element(p_block, p_block->m_start+m_next_position);
  m_next_position++; 
  while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
    m_next_position++;
  }
  return row;
}

void TBlockIterator::reset(TBlock* block) {
  p_block = block;
  m_next_position = 0;
  if(p_block != nullptr) {
    while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
      m_next_position++;
    }
  }
}

Table::Iterator::Iterator(const std::map<int32_t, TBlock*>& blocks) : 
  m_blocks(blocks),
  m_it(blocks.cbegin())
{
}


bool Table::Iterator::has_next() const { 
  return m_it != m_blocks.cend();
}

TBlock* Table::Iterator::next() {
  TBlock* next = m_it->second;
  m_it++;
  return next;
}

Table::Table(std::string& name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr)) :
  m_name(name),
  m_id(id),
  m_esize(esize),
  m_num_elements(0),
  m_destructor(destructor) {
  }

Table::Table(std::string&& name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void* ptr)) :
  m_name(name),
  m_id(id),
  m_esize(esize),
  m_num_elements(0),
  m_destructor(destructor){
  }

Table::~Table() {
  clear();
}

size_t Table::size() const {
  return m_num_elements;
}

void Table::clear() {
  Iterator iterator(m_blocks);
  while(iterator.has_next()) {
    TBlock* block = iterator.next();
    TBlockIterator b_iterator{block};
    while(b_iterator.has_next()) {
      m_destructor(b_iterator.next().p_data);
    }
    numa_free(block->p_data);
    delete block;
  }
  m_blocks.clear();
  m_num_elements = 0;
}


void* Table::get_element(int32_t id) const {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  if(it == m_blocks.end()) {
    return nullptr;
  }
  TBlock* block = it->second;
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00) {
    return &block->p_data[decoded_id.m_block_offset*m_esize];
  }
  return nullptr;
}

void* Table::alloc_element(int32_t id) {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  TBlock* block = nullptr;
  if (it == m_blocks.end()) {
    block = new TBlock();
    block->p_data = static_cast<int8_t*>(numa_alloc(0, m_esize*TABLE_BLOCK_SIZE ));
    block->m_start = (id / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE;
    block->m_num_elements = 0;
    block->m_esize = m_esize;
    std::memset(&block->m_exists[0], '\0', TABLE_BLOCK_BITMAP_SIZE);
    m_blocks.insert(std::make_pair(decoded_id.m_block_id, block));
  } else {
    block = it->second;
  }

  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) == 0x00) {
    m_num_elements++;
    block->m_num_elements++;
  }
  block->m_exists[decoded_id.m_bitmap_offset] = block->m_exists[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
  return &block->p_data[decoded_id.m_block_offset*m_esize];
}

void  Table::remove_element(int32_t id) {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  if(it == m_blocks.end()) {
    return;
  }

  TBlock* block = it->second; 
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00) {
    m_num_elements--;
    block->m_num_elements--;
  }
  block->m_exists[decoded_id.m_bitmap_offset] = block->m_exists[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
  m_destructor(&block->p_data[decoded_id.m_block_offset*m_esize]);
  memset(&block->p_data[decoded_id.m_block_offset*m_esize], '\0', m_esize);
}

void Table::enable_element(int32_t id) {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  if(it == m_blocks.end()) {
    return;
  }
  TBlock* block = it->second; 
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
}

void Table::disable_element(int32_t id) {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  if(it == m_blocks.end()) {
    return;
  }
  TBlock* block = it->second; 
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
}

bool Table::is_enabled(int32_t id) {
  DecodedId decoded_id = decode_id(id);
  auto it = m_blocks.find(decoded_id.m_block_id);
  if(it == m_blocks.end()) {
    return false;
  }
  TBlock* block = it->second; 
  return (block->m_enabled[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0;
}

Table::Iterator Table::iterator() {
  return Iterator{m_blocks};
}

std::string Table::table_name() const {
  return m_name;
}
  
} /* furious */ 
