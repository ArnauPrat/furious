
#ifndef _FDB_TABLE_H_
#define _FDB_TABLE_H_

#include "../../common/memory/memory.h"
#include "../../common/bitmap.h"
#include "../../common/btree.h"
#include "../../common/mutex.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


  /**
   * \brief A row of a table block. This is used to conveniently access the
   * information of a row, eventhough data is not stored in rows in the block  
   */
  typedef struct fdb_table_entry_t
  {
    entity_id_t   m_id;
    void*         p_data;
    bool          m_enabled;
  } fdb_table_entry_t;


  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  /**
   * \brief Represents a block of data in a table. Each block contains
   * FDB_TABLE_BLOCK_SIZE elements
   */
  typedef struct fdb_table_block_t 
  {
    FDB_ALIGNED(void*, p_data, FDB_TABLE_BLOCK_DATA_ALIGNMENT);    // The pointer to the block data
    entity_id_t   m_start;                                         // The id of the first component in the block
    size_t        m_num_components;                                // The number of components in the block
    size_t        m_num_enabled_components;                        // The number of enabled components in the block
    size_t        m_esize;                                         // The size of the components contained in the block
    fdb_bitmap_t  m_exists;                                        // A bitmap used to test whether an component is in the block or not
    fdb_bitmap_t  m_enabled;                                       // A bitmap used to mark components that are enabled/disabled
  } fdb_table_block_t;

  void 
  fdb_table_block_init(fdb_table_block_t* tb, 
                       entity_id_t start, 
                       size_t esize, 
                       fdb_mem_allocator_t* data_allocator, 
                       fdb_mem_allocator_t* fdb_bitmap_allocator);

  void
  fdb_table_block_release(fdb_table_block_t* tblock, 
                          fdb_mem_allocator_t* data_allocator, 
                          fdb_mem_allocator_t* fdb_bitmap_allocator);





  /**
   * \brief Tests if a block contains component with the given id
   *
   * \param block The block to check the component
   * \param id The id of the component to check
   *
   * \return Returns true if the block contains such component
   */
  bool 
  fdb_table_block_has_component(const fdb_table_block_t* block, 
                                entity_id_t id);

  /**
   * \brief Gets the component of a block
   *
   * \param block The block to get the component from
   *
   * \return Returns a pointer to the component. Returns nullptr if the component does
   * not exist in the block
   */
  fdb_table_entry_t 
  fdb_table_block_get_component(const fdb_table_block_t *block, 
                                entity_id_t id);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  /**
   * \brief Iterator to iterate over the components of a table block. 
   */
  typedef struct fdb_tblock_iter_t
  {
    fdb_table_block_t*  p_block;          //< Pointer to the block being iterated
    uint32_t        m_next_position;  //< The next position in the table block to iterate to
  } fdb_tblock_iter_t;

  /**
   * \brief inits a table block iterator
   *
   * \param tblock The table block to iterate
   *
   * \return Returns a newly initd table block iterator
   */
  void
  fdb_tblock_iter_init(fdb_tblock_iter_t* iter, 
                   fdb_table_block_t* tblock);

  /**
   * \brief releases a table block iterator
   *
   * \param iter The table block iterator to release
   */
  void
  fdb_tblock_iter_release(fdb_tblock_iter_t* iter);

  /**
   * \brief Checks if there are new items to iterate in the table block iterator
   *
   * \return Returns true if there are remaining items to iterate
   */
  bool
  fdb_tblock_iter_has_next(fdb_tblock_iter_t* iter);

  /**
   * \brief Gets the next entry in the table block iterator
   *
   * \return  The next entry in the table block iterator
   */
  fdb_table_entry_t
  fdb_tblock_iter_next(fdb_tblock_iter_t* iter);

  /**
   * \brief Resets the table block iterator with the given block
   *
   * \param block The new block to reset the iterator with
   */
  void
  fdb_tblock_iter_reset(fdb_tblock_iter_t* iter, 
                    fdb_table_block_t* block);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////


  typedef struct fdb_table_t 
  {
    char                  m_name[FDB_MAX_TABLE_NAME];     //< The name of the table
    uint64_t              m_id;                           //< The id of the table
    size_t                m_esize;                        //< The size of each component in bytes
    fdb_btree_t           m_blocks;                       //< The btree with the table blocks
    size_t                m_num_components;               //< The number of components in this table
    void (*m_destructor)(void *ptr);                      //< The pointer to the destructor for the components
    fdb_mutex_t           m_mutex;                        //< The able mutex

    // fixed block memory allocators for the different parts
    fdb_pool_alloc_t       m_tblock_allocator;      //< The allocator for the table blocks structure
    fdb_pool_alloc_t       m_data_allocator;        //< The allocator for the actual components data in table blocks
    fdb_pool_alloc_t       m_bitmap_data_allocator; //< The allocator for the bitmaps (this is forwarded to bitmap allocation in table blocks)
  } fdb_table_t;


  /**
   * \brief inits a table
   *
   * \param name The name of the table
   * \param id The id of the table
   * \param esize The size of the components (rows) of the table
   * \param destructor The destructor to release the components
   *
   * \return 
   */
  void
  fdb_table_init(fdb_table_t* table, 
                 const char* name, 
                 int64_t id, 
                 size_t esize, 
                 void (*destructor)(void *ptr), 
                 fdb_mem_allocator_t* allocator);

  /**
   * \brief releases a table
   *
   * \param table The table to release
   */
  void
  fdb_table_release(fdb_table_t* table);

  /**
   * \brief Clears the table
   *
   * \param table The table to clear
   */
  void 
  fdb_table_clear(fdb_table_t* table);

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
  fdb_table_get_component(fdb_table_t* table, 
                          entity_id_t id);

  /**
   * \brief Enables an component of the table, only it it exists 
   *
   * \param table The table to enable the component from 
   * \param id The id of the component to enable 
   */
  void 
  fdb_table_enable_component(fdb_table_t* table, 
                             entity_id_t id);

  /**
   * \brief Disables an component of the table
   *
   * \param table The table to disable the component from
   * \param id The if of the component to disable 
   */
  void 
  fdb_table_disable_component(fdb_table_t* table, 
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
  fdb_table_is_enabled(fdb_table_t* table, entity_id_t id);


  /**
   * \brief Gets the given block
   *
   * \param table The table to get the block from
   * \param block_id The id of the block to retrieve
   *
   * \return A pointer to the block
   */
  fdb_table_block_t* 
  fdb_table_get_block(fdb_table_t* table, uint32_t block_id);

  /**
   * \brief Locks the table
   *
   * \param table The table to lock
   */
  void
  fdb_table_lock(fdb_table_t* table);

  /**
   * \brief Unlocks the table
   *
   * \param The table to release
   */
  void
  fdb_table_unlock(fdb_table_t* table);

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
  fdb_table_create_component(fdb_table_t* table, 
                             entity_id_t id);

  /**
   * \brief This calls the destructor on the component and removes and deallocates itllocated.   
   *
   * \param table The table to deallocate and release the component for
   * \param id The id of the entity to remove the component for
   *
   */
  void 
  fdb_table_destroy_component(fdb_table_t* table, 
                              entity_id_t id);

  /**
   * \brief Gets the number of components in table
   *
   * \param table The table to get the number of components for
   */
  size_t
  fdb_table_size(fdb_table_t* table);


  void
  fdb_table_set_component_destructor(fdb_table_t* table, void (*destr)(void *ptr));

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////


  /**
   * \brief Iterator of a table, used to iterate over the table blocks of a table
   */
  typedef struct fdb_table_iter_t 
  {
    fdb_btree_t*                          p_blocks;       //< The table btree with the blocks
    fdb_btree_iter_t                      m_it;           //< The btree iterator
    uint32_t                              m_chunk_size;   //< The size of consecutive blocks (chunks) to iterate
    uint32_t                              m_offset;       //< The offset in chunk size to start iterate from
    uint32_t                              m_stride;       //< The amount of blocks to skip (in chunk size) after a chunk has been consumed
    fdb_table_block_t*                    m_next;         //< The next table block to iterate
  } fdb_table_iter_t;

  /**
   * \brief inits a table iterator to iterate on a subset of qualifying table
   * blocks
   *
   * \param table The table to iterat
   * \param chunk_size The size of consecutive blocks (chunks) to iterate
   * \param offset The offset in chunk size to start iterate from
   * \param stride The amount of blocks to skip (in chunk size) after a chunk has
   * been consumed
   *
   * \return Returns the newly initd iterator
   */
  void
  fdb_table_iter_init(fdb_table_iter_t* iter, 
                      fdb_table_t* table,
                      uint32_t chunk_size,
                      uint32_t offset, 
                      uint32_t stride);

  /**
   * \brief releases a table iterator
   *
   * \param iter The iter to release
   */
  void
  fdb_table_iter_release(fdb_table_iter_t* iter);

  /**
   * \brief Checks if the iterator has members to consume
   *
   * \param iter The iterator to check
   *
   * \return Returns true if there are members to consume
   */
  bool
  fdb_table_iter_has_next(fdb_table_iter_t* iter);

  /**
   * \brief Checks if the iterator has table blocks to consume
   *
   * \param iter The iterator to check
   *
   * \return Returns true if there are members to consume
   */
  fdb_table_block_t*
  fdb_table_iter_next(fdb_table_iter_t* iter);

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
#ifdef __cplusplus
}
#endif

#endif
