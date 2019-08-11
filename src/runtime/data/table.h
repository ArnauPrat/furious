
#ifndef _FURIOUS_TABLE_IMPL_H_
#define _FURIOUS_TABLE_IMPL_H_

#include "../../common/btree.h"
#include "../../common/bitmap.h"

#include <mutex>

namespace furious
{

constexpr entity_id_t FURIOUS_INVALID_ID = 0xffffffff;

/**
 * \brief The number of components per block. The current number, 16 is not
 * arbitrarily chosen. Assuming a cache line of 64 bytes long, 16 4byte components
 * can be stored in a line.
 */
constexpr uint32_t TABLE_BLOCK_SIZE = 64;

/**
 * \brief Represents a block of data in a table. Each block contains
 * TABLE_BLOCK_SIZE elements
 */
struct TBlock
{
  TBlock(entity_id_t start, size_t esize);
  ~TBlock();
  
  char *p_data;                         // The pointer to the block data
  entity_id_t m_start;                  // The id of the first component in the block
  size_t m_num_components;              // The number of components in the block
  size_t m_num_enabled_components;      // The number of components in the block
  size_t m_esize;                       // The size of the components contained in the block
  Bitmap* p_exists;                     // A bitmap used to test whether an component is in the block or not
  Bitmap* p_enabled;                    // A bitmap used to mark components that are enabled/disabled
};

/**
 * \brief A row of a table block. This is used to conveniently access the
 * information of a row, eventhough data is not stored in rows in the block  
 */
struct TRow
{
  const entity_id_t m_id;
  char *const p_data;
  const bool m_enabled;
};

/**
 * \brief Iterator to iterate over the components of a table block. 
 */
struct TBlockIterator final
{
  TBlockIterator(TBlock *block);
  ~TBlockIterator() = default;

  bool has_next() const;

  TRow next();

  void reset(TBlock *block);

private:
  TBlock *p_block;
  uint32_t m_next_position;
};

/**
 * \brief Tests if a block contains component with the given id
 *
 * \param block The block to check the component
 * \param id The id of the component to check
 *
 * \return Returns true if the block contains such component
 */
bool 
has_component(const TBlock *block, entity_id_t id);

/**
 * \brief Gets the component of a block
 *
 * \param block The block to get the component from
 *
 * \return Returns a pointer to the component. Returns nullptr if the component does
 * not exist in the block
 */
TRow 
get_component(const TBlock *block, entity_id_t id);

struct Table
{
  struct Iterator
  {
    Iterator(const BTree<TBlock>* blocks);

    Iterator(const BTree<TBlock>* blocks, 
             uint32_t chunk_size,
             uint32_t offset, 
             uint32_t stride);

    ~Iterator() = default;

    /**
     * \brief Checks whether there is a another block in the table.
     *
     * \return Returns true if there is another block in the table.
     */
    bool has_next() const;

    /**
     * \brief Gets the next block in the table
     *
     * \return Returns the next block in the table. Returns nullptr if it does
     * not exist
     */
    TBlock *next();

  private:
    const BTree<TBlock>*          p_blocks;
    mutable BTree<TBlock>::Iterator       m_it;
    uint32_t                              m_chunk_size;
    uint32_t                              m_offset;
    uint32_t                              m_stride;
    mutable TBlock*                       m_next;
  };

  Table(const char* name, 
        int64_t id, 
        size_t esize, 
        void (*destructor)(void *ptr));
  ~Table();

  /**
   * \brief Clears the table
   */
  void 
  clear();

  /**
   * \brief Gets the component with the given id
   *
   * \param id The id of the component to get
   *
   * \return Returns a pointer to the component. Returns nullptr if the component
   * does not exist in the table
   */
  void* 
  get_component(entity_id_t id) const;

  /**
   * \brief Enables an component of the table, only it it exists 
   *
   * \param id The id of the component to enable 
   */
  void 
  enable_component(entity_id_t id);

  /**
   * \brief Disables an component of the table
   *
   * \param id The if of the component to disable 
   */
  void 
  disable_component(entity_id_t id);

  /**
   * \brief Tells if an component is enabled or not
   *
   * \param id The component to test if it is enabled or not
   *
   * \return True if the component is enabled. False if it is not 
   */
  bool 
  is_enabled(entity_id_t id);

  /**
   * \brief Gets an iterator of the table
   *
   * \return Returns an iterator of the table.
   */
  Iterator 
  iterator();

  /**
   * \brief Gets an iterator of the table with a specific chunksize, offset and
   * stride
   *
   * \return Returns an iterator of the table.
   */
  Iterator 
  iterator(uint32_t chunk_size, 
           uint32_t offset, 
           uint32_t stride);

  /**
   * \brief Gets the name of the table
   *
   * \return Returns the name of the able
   */
  const char* 
  name() const;

  /**
   * \brief Gets the size of the table
   *

   */
  size_t 
  size() const;

  /**
   * \brief Gets the given block
   *
   * \param block_id The id of the block to retrieve
   *
   * \return A pointer to the block
   */
  TBlock* 
  get_block(uint32_t block_id);

  /**
   * \brief Locks the table
   */
  void
  lock() const;

  /**
   * \brief Releases the table
   */
  void
  release() const;

  /**
   * \brief This function virtually allocates the space of an component and
   * returns a pointer to the reserved position. The component is marked as if it
   * was inserted and enabled.
   *
   * \param id The id of the entity to remove the component for
   *
   */
  void* 
  alloc_component(entity_id_t id);

  /**
   * \brief This  virtually deallocates the space of an component and
   * returns a pointer to the position where it was deallocated. The component is marked as if it
   * was removed.  
   *
   * \param id The id of the entity to remove the component for
   *
   * \return The pointer where the component for the entity of the given id was
   * allocated. nullptr if it did not exist.
   */
  void* 
  dealloc_component(entity_id_t id);


  /**
   * \brief This removes the component and deallocates its space by 
   * calling the destructor of the table on the memory position where it was allocated.   
   *
   * \param id The id of the entity to remove the component for
   *
   */
  void 
  dealloc_and_destroy_component(entity_id_t id);

private:
  char*                 p_name; // The name of the table
  uint64_t              m_id;
  size_t                m_esize; // The size of each component in bytes
  mutable BTree<TBlock> m_blocks;
  size_t                m_num_components;
  void (*m_destructor)(void *ptr);
  mutable std::mutex    m_mutex;
};

} // namespace furious

#endif
