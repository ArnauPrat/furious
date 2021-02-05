

#ifndef _FDB_TXBITTABLE_H_
#define _FDB_TXBITTABLE_H_

#include "../../common/platform.h"
#include "txbitmap.h"
#include "txbtree.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum fdb_txbittable_op_t
{
  E_TXBITTABLE_AND,
  E_TXBITTABLE_OR,
  E_TXBITTABLE_DIFF
};

struct fdb_txbittable_factory_t
{
  struct fdb_txpool_alloc_t*    p_txbitmap_alloc;
  struct fdb_txbitmap_factory_t m_txbitmap_factory;
  struct fdb_txbtree_factory_t  m_btree_factory;
};

struct fdb_txbittable_t 
{
  struct fdb_txbittable_factory_t*  p_factory;
  struct fdb_txbtree_t              m_bitmaps;
};


/**
 * \brief Initializes the txbittable factory
 *
 * \param factory The factory to initialize
 * \param allocator The memory allocator to use in the allocations. NULL to use
 * the global mem allocator
 */
void
fdb_txbittable_factory_init(struct fdb_txbittable_factory_t* factory, 
                            struct fdb_mem_allocator_t* allocator);

/**
 * \brief Releases a txbittable factory
 *
 * \param factory The factory to release
 */
void
fdb_txbittable_factory_release(struct fdb_txbittable_factory_t* factory, 
                               struct fdb_tx_t* tx, 
                               struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief inits a bittable
 *
 * \param bt The the bittable to initialize
 * \param factory The factory to use to allocate the bittable
 *
 */
void
fdb_txbittable_init(struct fdb_txbittable_t* bt, 
                    struct fdb_txbittable_factory_t* factory, 
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx);


/**
 * \brief releases a bittable
 *
 * \param bittable
 */
void
fdb_txbittable_release(struct fdb_txbittable_t* bittable, 
                       struct fdb_tx_t* tx, 
                       struct fdb_txthread_ctx_t* txtctx);

/**
 * \brief Tests if an element exists in the bit table
 *
 * \param bittable The bittable to check the existence of the entity
 * \param id The id of the element to test
 *
 * \return Returns true if the element exists. false otherwise.
 */
bool
fdb_txbittable_exists(struct fdb_txbittable_t* bittable, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, 
                      entity_id_t id);

/**
 * \brief Adds an element to the bit table 
 *
 * \param bittable The bittable to add the entity to
 * \param id The id of the entity to add
 */
void
fdb_txbittable_add(struct fdb_txbittable_t* bittable, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx, 
                   entity_id_t id);

/**
 * \brief Removes an element from the bit table
 *
 * \param bittable The bittable to remove the entity from
 * \param id The id of the entity to remove
 */
void
fdb_txbittable_remove(struct fdb_txbittable_t* bittable, 
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx, entity_id_t id);

/**
 * \brief Gets the bitmap of the bit table for a specific entity id.
 *
 * \param bittable The bittable to get the bitmap from
 * \param id The id of the entity to get the bitmap of 
 *
 * \return Returns a pointer to the bitmap. Returns nullptr if the bitmap does
 * not exist.
 */
struct fdb_txbitmap_t* 
fdb_txbittable_get_bitmap(struct fdb_txbittable_t* bittable, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx,
                          entity_id_t id) ;

/**
 * \brief Clears the bittable
 */
void
fdb_txbittable_clear(struct fdb_txbittable_t* bittable, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx);

void
fdb_txbittable_union(FDB_RESTRICT(struct fdb_txbittable_t*) first, 
                     FDB_RESTRICT(struct fdb_txbittable_t*) second, 
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx);

void
fdb_txbittable_difference(FDB_RESTRICT(struct fdb_txbittable_t*) first, 
                          FDB_RESTRICT(struct fdb_txbittable_t*) second, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_BIT_TABLE_H_ */
