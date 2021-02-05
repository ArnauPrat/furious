

#ifndef _FDB_BITTABLE_H_
#define _FDB_BITTABLE_H_

#include "../../common/platform.h"
#include "../../common/bitmap.h"
#include "../../common/btree.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum fdb_bittable_op_t
{
  E_AND,
  E_OR,
  E_DIFF
};

struct fdb_bittable_t 
{
  struct fdb_btree_t           m_bitmaps;
  uint32_t              m_size;
  struct fdb_pool_alloc_t      m_bitmap_allocator;
  struct fdb_pool_alloc_t      m_bitmap_data_allocator;
};

/**
 * \brief inits a bittable
 *
 * \param allocator The memory allocator to use in the bittable
 *
 * \return Returns a newly initd bittable
 */

void
fdb_bittable_init(struct fdb_bittable_t* bt, 
                  struct fdb_mem_allocator_t* allocator);


/**
 * \brief releases a bittable
 *
 * \param bittable
 */
void
fdb_bittable_release(struct fdb_bittable_t* bittable);

/**
 * \brief Tests if an element exists in the bit table
 *
 * \param bittable The bittable to check the existence of the entity
 * \param id The id of the element to test
 *
 * \return Returns true if the element exists. false otherwise.
 */
bool
fdb_bittable_exists(const struct fdb_bittable_t* bittable, 
                    entity_id_t id);

/**
 * \brief Adds an element to the bit table 
 *
 * \param bittable The bittable to add the entity to
 * \param id The id of the entity to add
 */
void
fdb_bittable_add(struct fdb_bittable_t* bittable, 
             entity_id_t id);

/**
 * \brief Removes an element from the bit table
 *
 * \param bittable The bittable to remove the entity from
 * \param id The id of the entity to remove
 */
void
fdb_bittable_remove(struct fdb_bittable_t* bittable, entity_id_t id);

/**
 * \brief Gets the bitmap of the bit table for a specific entity id.
 *
 * \param bittable The bittable to get the bitmap from
 * \param id The id of the entity to get the bitmap of 
 *
 * \return Returns a pointer to the bitmap. Returns nullptr if the bitmap does
 * not exist.
 */
const struct fdb_bitmap_t* 
fdb_bittable_get_bitmap(const struct fdb_bittable_t* bittable,
                    entity_id_t id) ;

/**
 * \brief Gets the size of the bittable (number of bits set to 1)
 *
 * \param bittable The bittable to get the size of
 *
 * \return  The size of the bit table
 */
uint32_t
fdb_bittable_size(const struct fdb_bittable_t* bittable);

/**
 * \brief Clears the bittable
 */
void
fdb_bittable_clear(struct fdb_bittable_t* bittable);

void
fdb_bittable_union(FDB_RESTRICT(struct fdb_bittable_t*) first, FDB_RESTRICT(const struct fdb_bittable_t*) second);

void
fdb_bittable_difference(FDB_RESTRICT(struct fdb_bittable_t*) first, FDB_RESTRICT(const struct fdb_bittable_t*) second);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_BIT_TABLE_H_ */
