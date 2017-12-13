

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

/**
 * @brief Given an id, it returns the block this id belongs to
 *
 * @param id The id to get the block id for
 *
 * @return  The block_id of the block this id belongs to
 */
static uint8_t get_block_id(uint32_t id) {
  uint32_t btree_key_bits = sizeof(uint8_t)*8;
  uint32_t block_size_bits = std::log2(TABLE_BLOCK_SIZE);
  uint32_t mask = (0xffffffff) >> (sizeof(uint32_t)*8 - (btree_key_bits)) << block_size_bits;
  return static_cast<uint8_t>(mask & id);
}

/**
 * @brief Given an id, returns the offset within a block this id belongs to 
 *
 * @param id The block_offset of the id.  
 *
 * @return Returns the offset in the block where this id belongs to.
 */
static uint32_t get_block_offset(uint32_t id) {
  uint32_t block_size_bits = std::log2(TABLE_BLOCK_SIZE);
  uint32_t mask = (0xffffffff) >> (sizeof(uint32_t)*8 - block_size_bits);
  return (mask & id);
}

bool has_element(const TBlock* block, uint32_t id) {
  return get_element(block, id) != nullptr;
}

void* get_element(const TBlock* block, uint32_t id) {
  assert(block->m_start == get_block_id(id) * TABLE_BLOCK_SIZE);
  uint32_t block_offset = get_block_offset(id);
  uint32_t bitmap_offset = block_offset / (sizeof(uint8_t)*8);
  uint32_t mask_index = block_offset % (sizeof(uint8_t)*8);
  if((block->m_exists[bitmap_offset] & bitmap_masks[mask_index]) != 0x00) {
    return &block->p_data[block_offset*block->m_esize];
  }
  return nullptr;
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
        for(uint32_t i = 0; i < TABLE_BLOCK_SIZE; ++i) {
          uint32_t id = i + block->m_start;
          void * ptr = ::furious::get_element(block, id);
          if(ptr != nullptr) {
            m_destructor(ptr);
          }
        }
        numa_free(block->p_data);
        delete block;
      }
      btree->clear();
    }
  }
  m_num_elements = 0;
}


BTree<TBlock>* Table::get_btree(uint32_t id) const {
  uint32_t btree_bits = sizeof(uint8_t)*8;
  uint32_t block_bits = std::log2(TABLE_BLOCK_SIZE);
  uint32_t btree_index = id >> (btree_bits + block_bits);
  if(btree_index >= m_btrees.size()) {
    m_btrees.resize(btree_index+1, nullptr);
  }
  if(m_btrees[btree_index] == nullptr) {
    m_btrees[btree_index] = new BTree<TBlock>();
  }
  return m_btrees[btree_index];
}

void* Table::get_element(uint32_t id) const {
  BTree<TBlock>* btree = get_btree(id);
  uint8_t block_id = get_block_id(id);
  TBlock* block = btree->get(block_id);
  if (block == nullptr) {
    return nullptr;
  }
  uint32_t block_offset = get_block_offset(id);
  uint32_t bitmap_offset = block_offset / sizeof(uint8_t);
  uint32_t mask_index = block_offset % sizeof(uint8_t);
  if((block->m_exists[bitmap_offset] & bitmap_masks[mask_index]) != 0x00) {
    return &block->p_data[block_offset*m_esize];
  }
  return nullptr;
}

void* Table::alloc_element(uint32_t id) {
  BTree<TBlock>* btree = get_btree(id);
  uint8_t block_id = get_block_id(id);
  TBlock* block = btree->get(block_id);
  if (block == nullptr) {
    block = new TBlock();
    block->p_data = static_cast<uint8_t*>(numa_alloc(0, m_esize*TABLE_BLOCK_SIZE ));
    block->m_start = id / TABLE_BLOCK_SIZE * TABLE_BLOCK_SIZE;
    block->m_num_elements = 0;
    block->m_esize = m_esize;
    memset(&block->m_exists[0], '\0', TABLE_BLOCK_BITMAP_SIZE);
    btree->insert(block_id, block);
  }
  uint32_t block_offset = get_block_offset(id);
  uint32_t bitmap_offset = block_offset / (sizeof(uint8_t)*8);
  uint32_t mask_index = block_offset % (sizeof(uint8_t)*8);
  if((block->m_exists[bitmap_offset] & bitmap_masks[mask_index]) == 0x00) {
    m_num_elements++;
    block->m_num_elements++;
  }
  block->m_exists[bitmap_offset] = block->m_exists[bitmap_offset] | bitmap_masks[mask_index];
  block->m_enabled[bitmap_offset] = block->m_enabled[bitmap_offset] | bitmap_masks[mask_index];
  return &block->p_data[block_offset*m_esize];
}

void  Table::remove_element(uint32_t id) {
  BTree<TBlock>* btree = get_btree(id);
  uint8_t block_id = get_block_id(id);
  TBlock* block = btree->get(block_id);
  if (block == nullptr) {
    return;
  }
  uint32_t block_offset = get_block_offset(id);
  uint32_t bitmap_offset = block_offset / (sizeof(uint8_t)*8);
  uint32_t mask_index = block_offset % (sizeof(uint8_t)*8);
  if((block->m_exists[bitmap_offset] & bitmap_masks[mask_index]) != 0x00) {
    m_num_elements--;
    block->m_num_elements--;
  }
  block->m_exists[bitmap_offset] = block->m_exists[bitmap_offset] & ~(bitmap_masks[mask_index]);
  block->m_enabled[bitmap_offset] = block->m_enabled[bitmap_offset] & ~(bitmap_masks[mask_index]);
  m_destructor(&block->p_data[block_offset*m_esize]);
  memset(&block->p_data[block_offset*m_esize], '\0', m_esize);
}

void Table::enable_element(uint32_t id) {
}

void Table::disable_element(uint32_t id) {
}

bool Table::is_enabled(uint32_t id) {
  return false;
}

Table::Iterator* Table::iterator() {
  return new Iterator{&m_btrees};
}

std::string Table::table_name() const {
  return m_name;
}
  
} /* furious */ 
