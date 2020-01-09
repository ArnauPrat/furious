
#ifndef _FURIOUS_TABLE_IMPL_H_
#define _FURIOUS_TABLE_IMPL_H_

#include "../../common/impl/btree_impl.h"
#include "../../common/bitmap.h"
#include "../memory/pool_allocator.h"
#include "../../common/mutex.h"

namespace furious
{


/**
 * \brief A row of a table block. This is used to conveniently access the
 * information of a row, eventhough data is not stored in rows in the block  
 */
struct table_entry_t
{
  entity_id_t   m_id;
  char *const   p_data;
  const bool    m_enabled;
};

/**
 * \brief Represents a block of data in a table. Each block contains
 * FURIOUS_TABLE_BLOCK_SIZE elements
 */
struct table_block_t 
{
  FURIOUS_ALIGNED(void*, p_data, FURIOUS_TBLOCK_DATA_ALIGNMENT);    // The pointer to the block data
  entity_id_t m_start;                                              // The id of the first component in the block
  size_t m_num_components;                                          // The number of components in the block
  size_t m_num_enabled_components;                                  // The number of enabled components in the block
  size_t m_esize;                                                   // The size of the components contained in the block
  bitmap_t m_exists;                                                // A bitmap used to test whether an component is in the block or not
  bitmap_t m_enabled;                                               // A bitmap used to mark components that are enabled/disabled
};

table_block_t 
table_block_create(entity_id_t start, 
                   size_t esize, 
                   mem_allocator_t* data_allocator, 
                   mem_allocator_t* bitmap_allocator);

void
table_block_destroy(table_block_t* tblock, 
                   mem_allocator_t* data_allocator, 
                   mem_allocator_t* bitmap_allocator);





/**
 * \brief Tests if a block contains component with the given id
 *
 * \param block The block to check the component
 * \param id The id of the component to check
 *
 * \return Returns true if the block contains such component
 */
bool 
table_block_has_component(const table_block_t* block, 
                          entity_id_t id);

/**
 * \brief Gets the component of a block
 *
 * \param block The block to get the component from
 *
 * \return Returns a pointer to the component. Returns nullptr if the component does
 * not exist in the block
 */
table_entry_t 
table_block_get_component(const table_block_t *block, 
                          entity_id_t id);


/**
 * \brief Iterator to iterate over the components of a table block. 
 */
struct tblock_iter_t
{
  table_block_t*  p_block;          //< Pointer to the block being iterated
  uint32_t        m_next_position;  //< The next position in the table block to iterate to
};

/**
 * \brief Creates a table block iterator
 *
 * \param tblock The table block to iterate
 *
 * \return Returns a newly created table block iterator
 */
tblock_iter_t
tblock_iter_create(table_block_t* tblock);

/**
 * \brief Destroys a table block iterator
 *
 * \param iter The table block iterator to destroy
 */
void
tblock_iter_destroy(tblock_iter_t* iter);

/**
 * \brief Checks if there are new items to iterate in the table block iterator
 *
 * \return Returns true if there are remaining items to iterate
 */
bool
tblock_iter_has_next(tblock_iter_t* iter);

/**
 * \brief Gets the next entry in the table block iterator
 *
 * \return  The next entry in the table block iterator
 */
table_entry_t
tblock_iter_next(tblock_iter_t* iter, 
                 table_block_t* block);

/**
 * \brief Resets the table block iterator with the given block
 *
 * \param block The new block to reset the iterator with
 */
void
tblock_iter_reset(tblock_iter_t* iter, 
             table_block_t* block);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


struct table_t 
{
  char*                 p_name;           //< The name of the table
  uint64_t              m_id;             //< The id of the table
  size_t                m_esize;          //< The size of each component in bytes
  btree_t               m_blocks;         //< The btree with the table blocks
  size_t                m_num_components; //< The number of components in this table
  void (*m_destructor)(void *ptr);        //< The pointer to the destructor for the components
  mutex_t               m_mutex;          //< The able mutex

  // fixed block memory allocators for the different parts
  mem_allocator_t       m_tblock_allocator; //< The allocator for the table blocks structure
  mem_allocator_t       m_data_allocator;   //< The allocator for the actual components data in table blocks
  mem_allocator_t       m_bitmap_allocator; //< The allocator for the bitmaps (this is forwarded to bitmap allocation in table blocks)
  mem_allocator_t       m_btree_allocator;  //< The btree allocator for btree nodes, forwarded to the m_blocks btree
};


/**
 * \brief Creates a table
 *
 * \param name The name of the table
 * \param id The id of the table
 * \param esize The size of the components (rows) of the table
 * \param destructor The destructor to destroy the components
 *
 * \return 
 */
table_t
table_create(const char* name, 
             int64_t id, 
             size_t esize, 
             void (*destructor)(void *ptr));

/**
 * \brief Destroys a table
 *
 * \param table The table to destroy
 */
void
table_destroy(table_t* table);

/**
 * \brief Clears the table
 *
 * \param table The table to clear
 */
void 
table_clear(table_t* table);

/**
 * \brief Gets the component with the given id
 *
 * \param table The table to clear
 * \param id The id of the component to get
 *
 * \return Returns a pointer to the component. Returns nullptr if the component
 * does not exist in the table
 */
void* 
table_get_component(table_t* table, 
                    entity_id_t id);

/**
 * \brief Enables an component of the table, only it it exists 
 *
 * \param table The table to enable the component from 
 * \param id The id of the component to enable 
 */
void 
table_enable_component(table_t* table, 
                       entity_id_t id);

/**
 * \brief Disables an component of the table
 *
 * \param table The table to disable the component from
 * \param id The if of the component to disable 
 */
void 
table_disable_component(table_t* table, 
                        entity_id_t id);

/**
 * \brief Tells if an component is enabled or not
 *
 * \param table The table to check if component is enabled 
 * \param id The component to test if it is enabled or not
 *
 * \return True if the component is enabled. False if it is not 
 */
bool 
table_is_enabled(table_t* table, entity_id_t id);


/**
 * \brief Gets the given block
 *
 * \param table The table to get the block from
 * \param block_id The id of the block to retrieve
 *
 * \return A pointer to the block
 */
table_block_t* 
table_get_block(table_t* table, uint32_t block_id);

/**
 * \brief Locks the table
 *
 * \param table The table to lock
 */
void
table_lock(table_t* table);

/**
 * \brief Releases the table
 *
 * \param The table to release
 */
void
table_release(table_t* table);

/**
 * \brief This function virtually allocates the space of an component and
 * returns a pointer to the reserved position. The component is marked as if it
 * was inserted and enabled.
 *
 *
 * \param table The table to allocate the component for
 * \param id The id of the entity to remove the component for
 *
 */
void* 
table_alloc_component(table_t* table, 
                      entity_id_t id);

/**
 * \brief This calls the destructor on the component and removes and deallocates itllocated.   
 *
 * \param table The table to deallocate and destroy the component for
 * \param id The id of the entity to remove the component for
 *
 */
void 
table_dealloc_component(table_t* table, 
                        entity_id_t id);

/**
 * \brief Gets the number of components in table
 *
 * \param table The table to get the number of components for
 */
size_t
table_size(table_t* table);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct table_iter_t 
{
  btree_t*                              p_blocks;
  btree_iter_t                          m_it;
  uint32_t                              m_chunk_size;
  uint32_t                              m_offset;
  uint32_t                              m_stride;
  table_block_t*                        m_next;
};

/**
 * \brief Creaates a table iterator
 *
 * \param table
 *
 * \return  The newly created table iterator
 */
table_iter_t 
table_iter_create(table_t* table);

/**
 * \brief Creates a table iterator to iterate on a subset of qualifying table
 * blocks
 *
 * \param table The table to iterat
 * \param chunk_size The size of consecutive blocks (chunks) to iterate
 * \param offset The offset in chunk size to start iterate from
 * \param stride The amount of blocks to skip (in chunk size) after a chunk has
 * been consumed
 *
 * \return Returns the newly created iterator
 */
table_iter_t 
table_iter_create(table_t* table,
                  uint32_t chunk_size,
                  uint32_t offset, 
                  uint32_t stride);

/**
 * \brief Destroys a table iterator
 *
 * \param iter The iter to destroy
 */
void
table_iter_destroy(table_iter_t* iter);

/**
 * \brief Checks if the iterator has members to consume
 *
 * \param iter The iterator to check
 *
 * \return Returns true if there are members to consume
 */
bool
table_iter_has_next(table_iter_t* iter);

/**
 * \brief Checks if the iterator has table blocks to consume
 *
 * \param iter The iterator to check
 *
 * \return Returns true if there are members to consume
 */
table_block_t*
table_iter_next(table_iter_t* iter);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

/**
 * \brief Given a block id, a chunk size and a stride, returns the offset
 * (thread id) responsible of the block start.
 *
 * \param block_start The block id 
 * \param chunk_size The chunk size
 * \param stride The stride (num threads)
 *
 * \return Returns the offset (thread id)
 */
uint32_t
block_get_offset(uint32_t block_id, 
                 uint32_t chunk_size,
                 uint32_t stride);

} // namespace furious

#endif
