
#ifndef _FURIOUS_TABLE_IMPL_H_
#define _FURIOUS_TABLE_IMPL_H_ 

#include "common.h"
#include "btree.h"
#include <vector>
#include <string>
#include <set>

namespace furious {


/**
 * @brief The number of elements per block. The current number, 16 is not
 * arbitrarily chosen. Assuming a cahce line of 64 bytes long, 16 4byte elements
 * can be stored in a line.
 */
constexpr size_t TABLE_BLOCK_SIZE = 16;
constexpr size_t TABLE_BLOCK_BITMAP_SIZE=(TABLE_BLOCK_SIZE + 7) >> 3;

/**
 * @brief Represents a block of data in a table
 */
struct TBlock {
  uint8_t*          p_data;                             // The pointer to the block data
  uint32_t          m_start;                            // The id of the first element in the block
  size_t            m_num_elements;                     // The number of elements in the block 
  uint32_t          m_esize;                            // The size of the elements contained in the block
  uint8_t           m_exists[TABLE_BLOCK_BITMAP_SIZE];  // A vector of bits used to test whether an element is in the block or not
  uint8_t           m_enabled[TABLE_BLOCK_BITMAP_SIZE]; // A vector of bits used to mark components that are enabled/disabled 
};

/**
 * @brief A row of a table block. This is used to conveniently access the
 * information of a row, eventhough data is not stored in rows in the block  
 */
struct TRow {
  const uint32_t  m_id;
  uint8_t* const  p_data;
  const bool      m_enabled;
};

/**
 * @brief Iterator to iterate over the elements of a table block. 
 */
class TBlockIterator {
public:
  TBlockIterator(TBlock* block);

  bool has_next() const;

  TRow next();

private:
  TBlock*  p_block;
  uint32_t m_next_position;
};

/**
 * @brief Tests if a block contains element with the given id
 *
 * @param block The block to check the element
 * @param id The id of the element to check
 *
 * @return Returns true if the block contains such element
 */
bool has_element(const TBlock* block, uint32_t id);


/**
 * @brief Gets the element of a block
 *
 * @param block The block to get the element from
 *
 * @return Returns a pointer to the element. Returns nullptr if the element does
 * not exist in the block
 */
TRow get_element(const TBlock* block, uint32_t id);

class Table {

public:
  class Iterator {
  public:
    Iterator(std::vector<BTree<TBlock>*>* btrees);
    virtual ~Iterator();

    /**
     * @brief Checks whether there is a another block in the table.
     *
     * @return Returns true if there is another block in the table.
     */
    bool has_next() const;

    /**
     * @brief Gets the next block in the table
     *
     * @return Returns the next block in the table. Returns nullptr if it does
     * not exist
     */
    TBlock* next();
    
  private:
    bool advance_iterator() const;

    mutable uint32_t                      m_next_btree;
    mutable std::vector<BTree<TBlock>*>*  p_btrees;
    mutable BTree<TBlock>::Iterator*      p_iterator;

  };


public:

  Table(std::string& name, size_t esize, void (*destructor)(void* ptr));
  Table(std::string&& name, size_t esize, void (*destructor)(void* ptr));
  virtual ~Table();

  /**
   * @brief Clears the table
   */
  void clear();

  /**
   * @brief Gets the element with the given id
   *
   * @param id The id of the element to get
   *
   * @return Returns a pointer to the element. Returns nullptr if the element
   * does not exist in the table
   */
  void* get_element(uint32_t id) const ;

  /**
   * @brief Cre 
   *
   * @tparam TComponent
   * @tparam typename...Args
   * @param id
   * @param ...args
   */
  template<typename TComponent, typename...Args>
  void  insert_element(uint32_t id, Args&&...args);

  /**
   * @brief Drops the element with the given id
   *
   * @param id
   */
  void  remove_element(uint32_t id);

  /**
   * @brief Enables an element of the table, only it it exists 
   *
   * @param id The id of the element to enable 
   */
  void enable_element(uint32_t id);

  /**
   * @brief Disables an element of the table
   *
   * @param id The if of the element to disable 
   */
  void disable_element(uint32_t id);

  /**
   * @brief Tells if an element is enabled or not
   *
   * @param id The element to test if it is enabled or not
   *
   * @return True if the element is enabled. False if it is not 
   */
  bool is_enabled(uint32_t id);

  /**
   * @brief Gets an iterator of the table
   *
   * @return Returns an iterator of the table.
   */
  Iterator* iterator();

  /**
   * @brief Gets the name of the table
   *
   * @return Returns the name of the able
   */
  std::string table_name() const;

  size_t size() const;

private:

  BTree<TBlock>* get_btree(uint32_t btree_id) const;

  /**
   * @brief This is a helper function that virtually allocates the space of an element and
   * returns a pointer to the reserved position. The element is marked as if it
   * was inserted. This method is aimed to be called from insert_element, which
   * is a templated function. 
   *
   * @param id
   *
   * @return 
   */
  void* alloc_element(uint32_t id);

  /**
   * @brief This is a helper function that virtually deallocates the space of an element and
   * returns a pointer to the position where it was deallocated. The element is marked as if it
   * was removed. This method is aimed to be called from remove_element, which
   * is a templated function. 
   *
   * @param id
   *
   * @return 
   */
  void* dealloc_element(uint32_t id);


  std::string                         m_name;   // The name of the table
  size_t                              m_esize;  // The size of each element in bytes
  mutable std::vector<BTree<TBlock>*> m_btrees;
  size_t                              m_num_elements;
  void                                (*m_destructor)(void* ptr);
};

} /* furious */ 

#include "table.inl"

#endif

