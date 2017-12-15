

#include "table.h"
#include <cmath>

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
  const uint32_t m_btree_id;
  const uint8_t  m_block_id;
  const uint32_t m_block_offset;
  const uint32_t m_bitmap_offset;
  const uint32_t m_bitmap_mask;
};

static DecodedId decode_id(uint32_t id) { 
  uint32_t btree_key_bits = sizeof(uint8_t)*8;
  uint32_t block_size_bits = std::log2(TABLE_BLOCK_SIZE);
  uint32_t btree_id = id >> (btree_key_bits + block_size_bits);

  uint32_t block_id_mask = (0xffffffff) >> (sizeof(uint32_t)*8 - (btree_key_bits)) << block_size_bits;
  uint8_t  block_id  = static_cast<uint8_t>((block_id_mask & id) >> block_size_bits);

  uint32_t block_offset_mask = (0xffffffff) >> (sizeof(uint32_t)*8 - block_size_bits);
  uint32_t block_offset = block_offset_mask & id; 

  uint32_t bitmap_offset = block_offset / (sizeof(uint8_t)*8);

  uint32_t bitmap_mask = bitmap_masks[block_offset % (sizeof(uint8_t)*8)];

  return DecodedId{btree_id, block_id, block_offset, bitmap_offset, bitmap_mask};
}


bool has_element(const TBlock* block, uint32_t id) {
  return get_element(block, id).p_data != nullptr;
}

TRow get_element(const TBlock* block, uint32_t id) {
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
  while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
    m_next_position++;
  }
}

bool TBlockIterator::has_next() const {
  return m_next_position < TABLE_BLOCK_SIZE;
}

TRow TBlockIterator::next() {
  TRow row = get_element(p_block, p_block->m_start+m_next_position);
  m_next_position++; 
  while(m_next_position < TABLE_BLOCK_SIZE && !has_element(p_block, p_block->m_start+m_next_position) ) {
    m_next_position++;
  }
  return row;
}

Table::Iterator::Iterator(std::vector<BTree<TBlock>*>* btrees) : 
  m_next_btree(0),
  p_btrees(btrees),
  p_iterator((*p_btrees)[m_next_btree]->iterator())
{
  m_next_btree++;
}

Table::Iterator::~Iterator() {
  delete p_iterator;
}

bool Table::Iterator::advance_iterator() const {
  while(m_next_btree < p_btrees->size() && (*p_btrees)[m_next_btree] == nullptr) {
    m_next_btree++;
  }

  if(m_next_btree < p_btrees->size()) {
    delete p_iterator;
    p_iterator = (*p_btrees)[m_next_btree]->iterator();
    m_next_btree++;
    return true;
  } 
  return false;
}

bool Table::Iterator::has_next() const { 
  if(!p_iterator->has_next()) {
    if(advance_iterator()) {
      return p_iterator->has_next();
    }
    return false;
  }
  return true;
}

TBlock* Table::Iterator::next() {
  TBlock* next = p_iterator->next();
  if(next == nullptr) {
    advance_iterator();
    return p_iterator->next();
  }
  return next;
}

Table::Table(std::string& name, size_t esize, void (*destructor)(void* ptr)) :
  m_name(name),
  m_esize(esize),
  m_num_elements(0),
  m_destructor(destructor) {
    m_btrees.push_back(new BTree<TBlock>());
  }

Table::Table(std::string&& name, size_t esize, void (*destructor)(void* ptr)) :
  m_name(name),
  m_esize(esize),
  m_num_elements(0),
  m_destructor(destructor){
    m_btrees.push_back(new BTree<TBlock>());
  }

Table::~Table() {
  clear();
  for (auto btree : m_btrees) {
    if(btree != nullptr) {
      delete btree;
    }
  }
}

size_t Table::size() const {
  return m_num_elements;
}

void Table::clear() {
  for (auto btree : m_btrees) {
    if(btree != nullptr) {
      auto iterator = btree->iterator();
      while(iterator->has_next()) {
        TBlock* block = iterator->next();
        TBlockIterator b_iterator{block};
        while(b_iterator.has_next()) {
          m_destructor(b_iterator.next().p_data);
        }
        numa_free(block->p_data);
        delete block;
      }
      btree->clear();
    }
  }
  m_num_elements = 0;
}


BTree<TBlock>* Table::get_btree(uint32_t btree_id) const {
  if(btree_id >= m_btrees.size()) {
    m_btrees.resize(btree_id+1, nullptr);
  }
  if(m_btrees[btree_id] == nullptr) {
    m_btrees[btree_id] = new BTree<TBlock>();
  }
  return m_btrees[btree_id];
}

void* Table::get_element(uint32_t id) const {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  if (block == nullptr) {
    return nullptr;
  }
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00) {
    return &block->p_data[decoded_id.m_block_offset*m_esize];
  }
  return nullptr;
}

void* Table::alloc_element(uint32_t id) {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  if (block == nullptr) {
    block = new TBlock();
    block->p_data = static_cast<uint8_t*>(numa_alloc(0, m_esize*TABLE_BLOCK_SIZE ));
    block->m_start = (id / TABLE_BLOCK_SIZE) * TABLE_BLOCK_SIZE;
    block->m_num_elements = 0;
    block->m_esize = m_esize;
    memset(&block->m_exists[0], '\0', TABLE_BLOCK_BITMAP_SIZE);
    btree->insert(decoded_id.m_block_id, block);
  }
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) == 0x00) {
    m_num_elements++;
    block->m_num_elements++;
  }
  block->m_exists[decoded_id.m_bitmap_offset] = block->m_exists[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
  return &block->p_data[decoded_id.m_block_offset*m_esize];
}

void  Table::remove_element(uint32_t id) {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  assert(block != nullptr);
  if((block->m_exists[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0x00) {
    m_num_elements--;
    block->m_num_elements--;
  }
  block->m_exists[decoded_id.m_bitmap_offset] = block->m_exists[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
  m_destructor(&block->p_data[decoded_id.m_block_offset*m_esize]);
  memset(&block->p_data[decoded_id.m_block_offset*m_esize], '\0', m_esize);
}

void Table::enable_element(uint32_t id) {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  assert(block != nullptr);
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] | decoded_id.m_bitmap_mask;
}

void Table::disable_element(uint32_t id) {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  assert(block != nullptr);
  block->m_enabled[decoded_id.m_bitmap_offset] = block->m_enabled[decoded_id.m_bitmap_offset] & ~(decoded_id.m_bitmap_mask);
}

bool Table::is_enabled(uint32_t id) {
  DecodedId decoded_id = decode_id(id);
  BTree<TBlock>* btree = get_btree(decoded_id.m_btree_id);
  TBlock* block = btree->get(decoded_id.m_block_id);
  assert(block != nullptr);
  return (block->m_enabled[decoded_id.m_bitmap_offset] & decoded_id.m_bitmap_mask) != 0;
}

Table::Iterator* Table::iterator() {
  return new Iterator{&m_btrees};
}

std::string Table::table_name() const {
  return m_name;
}
  
} /* furious */ 
