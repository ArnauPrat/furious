
#ifndef _FDB_TXTABLE_H_
#define _FDB_TXTABLE_H_

#include "../../common/memory/memory.h"
#include "txbitmap.h"
#include "txbtree.h"
#include "tx/txpool_allocator.h"
#include "../../common/mutex.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



  /**
   * \brief A row of a table block. This is used to conveniently access the
   * information of a row, eventhough data is not stored in rows in the block  
   */
  struct fdb_txtable_entry_t
  {
    entity_id_t   m_id;
    void*         p_data;
    bool          m_enabled;
  };


  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  struct fdb_txtable_t;
  /**
   * \brief Represents a block of data in a table. Each block contains
   * FDB_TXTABLE_BLOCK_SIZE elements
   */
  struct fdb_txtable_block_t 
  {
    struct fdb_txtable_t*           p_table;                                          // The pointer to the parent table of this block
    struct fdb_txpool_alloc_ref_t   m_data;                                           // The reference to the block of data with the actual components data
    entity_id_t                     m_start;                                          // The id of the first component in the block
    size_t                          m_num_components;                                 // The number of components in the block
    size_t                          m_num_enabled_components;                         // The number of enabled components in the block
    size_t                          m_esize;                                          // The size of the components contained in the block
    struct fdb_txbitmap_t           m_exists;                                         // A bitmap used to test whether an component is in the block or not
    struct fdb_txbitmap_t           m_enabled;                                        // A bitmap used to mark components that are enabled/disabled
  };



  /**
   * \brief Factory used to create table. Tables created through this factory
   * share allocators for the tables, blocks and bitmaps. Data allocators are
   * passed during table creation as the allocation block size depends on the
   * actual table. 
   */
  struct fdb_txtable_factory_t
  {
    struct fdb_txpool_alloc_t*    p_tblock_allocator;  //< Tx allocator for txblocks
    struct fdb_txbitmap_factory_t m_bitmap_factory;    //< Factory for txblocks bitmaps
    struct fdb_txbtree_factory_t  m_btree_factory;     //< Factory for table btrees
    struct fdb_mem_allocator_t*   p_allocator;         //< The parent allocator. NULL to use the global allocator
  };

  struct fdb_txtable_t
  {
    struct fdb_txtable_factory_t*     p_factory;                    //< The factory this table was created from
    char                              m_name[FDB_MAX_TABLE_NAME];   //< The name of the table
    struct fdb_txpool_alloc_t*        p_data_allocator;             //< Tx allocator used to create the actual data blocks
    struct fdb_txpool_alloc_ref_t     m_table_impl;                 //< A reference to the table header implementation
    uint64_t                          m_id;                         //< The id of the table
    size_t                            m_esize;                      //< The size of each component in bytes
    struct fdb_txbtree_t              m_blocks;                     //< The btree with the table blocks
    void (*m_destructor)(void *ptr);                          //< The pointer to the destructor for the components
  };

  /**
   * \brief Iterator to iterate over the components of a table block. 
   */
  struct fdb_txtblock_iter_t
  {
    struct fdb_txpool_alloc_ref_t   m_block_ref;    //< Reference to the block being iterated
    struct fdb_txtable_t*           p_table;        //< The table the block belongs to
    uint32_t                        m_next_position;  //< The next position in the table block to iterate to
    struct fdb_tx_t*                p_tx;             //< The transaction
    struct fdb_txthread_ctx_t*      p_txtctx;         //< The transaction thread context
    bool                  m_write;          //< Whether this is an iterator meant to write the iterated data
  };

  /**
   * \brief Iterator of a table, used to iterate over the table blocks of a table
   */
  struct fdb_txtable_iter_t 
  {
    struct fdb_txtable_t*                        p_table;        //< The table to iterate
    struct fdb_txbtree_iter_t                    m_it;           //< The btree iterator
    struct fdb_tx_t*                             p_tx;           //< Pointer to the transaction
    struct fdb_txthread_ctx_t*                   p_txtctx;       //< Pointer to the transaction thread context
    uint32_t                              m_chunk_size;   //< The size of consecutive blocks (chunks) to iterate
    uint32_t                              m_offset;       //< The offset in chunk size to start iterate from
    uint32_t                              m_stride;       //< The amount of blocks to skip (in chunk size) after a chunk has been consumed
    struct fdb_txpool_alloc_ref_t                m_next;         //< The next table block to iterate
    bool                                  m_write;        //< Wether this is a write iterator or not
  };

  /**
   * \brief Initializes a table factory
   *
   * \param table_factory The table factory to initialize
   * \param allocator The allocator to use to allocate memory for the tables
   * created through the factory. NULL to use the global allocator. 
   */
  void
  fdb_txtable_factory_init(struct fdb_txtable_factory_t* table_factory, 
                           struct fdb_mem_allocator_t* allocator);

  /**
   * \brief Releases a table factory
   *
   * \param table_factory The table factory to release
   */
  void
  fdb_txtable_factory_release(struct fdb_txtable_factory_t* table_factory, 
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Initializes a table block for a table
   *
   * \param tb The table block to initialize
   * \param table The table this table block belongs to
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param start The starting id of the block
   * \param esize The element size
   */
  void 
  fdb_txtable_block_init(struct fdb_txtable_block_t* tb, 
                         struct fdb_txtable_t* table, 
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx,
                         entity_id_t start, 
                         size_t esize);

  /**
   * \brief Releases a table block
   *
   * \param tblock The table block to release
   * \param table The table the block belongs to
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txtable_block_release(struct fdb_txtable_block_t* tblock, 
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx);


  /**
   * \brief Tests if a block contains component with the given id
   *
   * \param block The block to check the component
   * \param id The id of the component to check
   *
   * \return Returns true if the block contains such component
   */
  bool 
  fdb_txtable_block_has_component(struct fdb_txtable_block_t* block, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx,
                                  entity_id_t id);

  /**
   * \brief Gets the component of a block
   *
   * \param block The block to get the component from
   * \param table The table the block belongs to
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The id of the component to get
   * \param dirty Whether the component will be modified or not
   *
   * \return Returns a txtrable entry with the component.  
   */
  struct fdb_txtable_entry_t 
  fdb_txtable_block_get_component(struct fdb_txtable_block_t *block, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx,
                                  entity_id_t id, 
                                  bool dirty);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  /**
   * \brief inits a table block iterator
   *
   * \param iter The iterator to initialize
   * \param tblock_ref The reference to the table block to iterate
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param write Whether this is a write iterator or not
   *
   */
  void
  fdb_txtblock_iter_init(struct fdb_txtblock_iter_t* iter, 
                         struct fdb_txtable_t* table, 
                         struct fdb_txpool_alloc_ref_t tblock_ref,
                         struct fdb_tx_t* tx, 
                         struct fdb_txthread_ctx_t* txtctx, 
                         bool write);

  /**
   * \brief releases a table block iterator
   *
   * \param iter The table block iterator to release
   */
  void
  fdb_txtblock_iter_release(struct fdb_txtblock_iter_t* iter);

  /**
   * \brief Checks if there are new items to iterate in the table block iterator
   *
   * \return Returns true if there are remaining items to iterate
   */
  bool
  fdb_txtblock_iter_has_next(struct fdb_txtblock_iter_t* iter);

  /**
   * \brief Gets the next entry in the table block iterator
   *
   * \return  The next entry in the table block iterator
   */
  struct fdb_txtable_entry_t
  fdb_txtblock_iter_next(struct fdb_txtblock_iter_t* iter);

  /**
   * \brief Resets the table block iterator with the given block
   *
   * \param iter The iterator to reset
   * \param block_ref The new block reference to reset the iterator with
   * \param tx The transaction
   * \param txtctx The transaction context
   * \param write Wether we want to iterate in writing mode or not
   */
  void
  fdb_txtblock_iter_reset(struct fdb_txtblock_iter_t* iter, 
                          struct fdb_txtable_t* table, 
                          struct fdb_txpool_alloc_ref_t block_ref, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx, 
                          bool write);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////



  /**
   * \brief inits a table
   *
   * \param table The table to initialize
   * \param table_factory The factory used to create the table
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param name The name of the table
   * \param id The id of the table
   * \param esize The size of the components (rows) of the table
   * \param destructor The destructor to release the components
   *
   */
  void
  fdb_txtable_init(struct fdb_txtable_t* table, 
                   struct fdb_txtable_factory_t* table_factory, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   const char* name, 
                   int64_t id, 
                   size_t esize, 
                   void (*destructor)(void *ptr), 
                   struct fdb_mem_allocator_t* allocator);

  /**
   * \brief releases a table
   *
   * \param table The table to release
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void
  fdb_txtable_release(struct fdb_txtable_t* table,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Clears the table
   *
   * \param table The table to clear
   * \param tx The transaction
   * \param txtctx The transaction thread context
   */
  void 
  fdb_txtable_clear(struct fdb_txtable_t* table,
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx);

  /**
   * \brief Gets the component with the given id
   *
   * \param table The table to clear
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The id of the component to get
   * \param write Whether we want to modify the component or not
   *
   */
  void* 
  fdb_txtable_get_component(struct fdb_txtable_t* table,
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx, 
                          entity_id_t id, 
                          bool write);

  /**
   * \brief Enables an component of the table, only it it exists 
   *
   * \param table The table to enable the component from 
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The id of the component to enable 
   */
  void 
  fdb_txtable_enable_component(struct fdb_txtable_t* table,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             entity_id_t id);

  /**
   * \brief Disables an component of the table
   *
   * \param table The table to disable the component from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The if of the component to disable 
   */
  void 
  fdb_txtable_disable_component(struct fdb_txtable_t* table,
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              entity_id_t id);

  /**
   * \brief Tells if an component is enabled or not
   *
   * \param table The table to check if component is enabled 
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The component to test if it is enabled or not
   *
   * \return True if the component is enabled. False if it is not 
   */
  bool 
  fdb_txtable_is_enabled(struct fdb_txtable_t* table,
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx, 
                       entity_id_t id);


  /**
   * \brief Gets the given block
   *
   * \param table The table to get the block from
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param block_id The id of the block to retrieve
   *
   * \return A reference to the block
   */
  struct fdb_txpool_alloc_ref_t 
  fdb_txtable_get_block(struct fdb_txtable_t* table,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, 
                      uint32_t block_id);

  /**
   * \brief This function virtually allocates the space of an component and
   * returns a pointer to the reserved position. The component is marked as if it
   * was inserted and enabled.
   *
   *
   * \param table The table to allocate the component for
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The id of the entity to remove the component for
   *
   *
   * \return Pointer to the memory region where the component space was
   * allocated
   */
  void* 
  fdb_txtable_create_component(struct fdb_txtable_t* table,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx, 
                             entity_id_t id);

  /**
   * \brief This calls the destructor on the component and removes and deallocates itllocated.   
   *
   * \param table The table to deallocate and release the component for
   * \param tx The transaction
   * \param txtctx The transaction thread context
   * \param id The id of the entity to remove the component for
   *
   */
  void 
  fdb_txtable_destroy_component(struct fdb_txtable_t* table,
                              struct fdb_tx_t* tx, 
                              struct fdb_txthread_ctx_t* txtctx, 
                              entity_id_t id);

  /**
   * \brief Gets the number of components in table
   *
   * \param table The table to get the number of components for
   */
  size_t
  fdb_txtable_size(struct fdb_txtable_t* table,
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx);


  /**
   * \brief Sets the destructor for this table components
   *
   * \param table The table to set the destructor for
   * \param destr
   */
  void
  fdb_txtable_set_component_destructor(struct fdb_txtable_t* table, void (*destr)(void *ptr));

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////



  /**
   * \brief inits a table iterator to iterate on a subset of qualifying table
   * blocks
   *
   * \param iter The iterator to initialize
   * \param table The table to iterat
   * \param tx The transaction 
   * \param txtctx The transaction thread context
   * \param chunk_size The size of consecutive blocks (chunks) to iterate
   * \param offset The offset in chunk size to start iterate from
   * \param stride The amount of blocks to skip (in chunk size) after a chunk has
   * been consumed
   *
   * \return Returns the newly initd iterator
   */
  void
  fdb_txtable_iter_init(struct fdb_txtable_iter_t* iter, 
                        struct fdb_txtable_t* table,
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx, 
                        uint32_t chunk_size,
                        uint32_t offset, 
                        uint32_t stride, 
                        bool write);

  /**
   * \brief releases a table iterator
   *
   * \param iter The iter to release
   */
  void
  fdb_txtable_iter_release(struct fdb_txtable_iter_t* iter);

  /**
   * \brief Checks if the iterator has members to consume
   *
   * \param iter The iterator to check
   *
   * \return Returns true if there are members to consume
   */
  bool
  fdb_txtable_iter_has_next(struct fdb_txtable_iter_t* iter);

  /**
   * \brief Checks if the iterator has table blocks to consume
   *
   * \param iter The iterator to check
   *
   * \return Returns true if there are members to consume
   */
  struct fdb_txtable_block_t*
  fdb_txtable_iter_next(struct fdb_txtable_iter_t* iter);

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif
