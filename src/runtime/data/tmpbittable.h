

#ifndef _FDB_TMPBITTABLE_H_
#define _FDB_TMPBITTABLE_H_

#include "../../common/platform.h"
#include "../../common/bitmap.h"
#include "../../common/btree.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum fdb_tmpbittable_op_t
{
  E_TMPBITTABLE_AND,
  E_TMPBITTABLE_OR,
  E_TMPBITTABLE_DIFF
};

struct fdb_tmpbittable_factory_t
{
  struct fdb_btree_factory_t   m_btree_factory;
  struct fdb_bitmap_factory_t  m_bitmap_factory;
  struct fdb_pool_alloc_t      m_bitmap_alloc;
};

struct fdb_tmpbittable_t 
{
  struct fdb_tmpbittable_factory_t* p_factory;
  struct fdb_btree_t                m_bitmaps;
  uint32_t                          m_size;
};

/**
 * \brief Initializes a tmpbittable factoryu
 *
 * \param factory The factory to initialize
 * \param allocator The memory allocator or NULL
 */
void
fdb_tmpbittable_factory_init(struct fdb_tmpbittable_factory_t* factory,
                             struct fdb_mem_allocator_t* allocator);

/**
 * \brief Releases the bittable factory
 *
 * \param factory The bittable factory to release
 */
void
fdb_tmpbittable_factory_release(struct fdb_tmpbittable_factory_t* factory);

/**
 * \brief inits a bittable
 *
 * \param allocator The memory allocator to use in the bittable
 *
 * \return Returns a newly initd bittable
 */
void
fdb_tmpbittable_init(struct fdb_tmpbittable_t* bt, 
                     struct fdb_tmpbittable_factory_t* factory);


/**
 * \brief releases a bittable
 *
 * \param bittable
 */
void
fdb_tmpbittable_release(struct fdb_tmpbittable_t* bittable);

/**
 * \brief Tests if an element exists in the bit table
 *
 * \param bittable The bittable to check the existence of the entity
 * \param id The id of the element to test
 *
 * \return Returns true if the element exists. false otherwise.
 */
bool
fdb_tmpbittable_exists(const struct fdb_tmpbittable_t* bittable, 
                       entity_id_t id);

/**
 * \brief Adds an element to the bit table 
 *
 * \param bittable The bittable to add the entity to
 * \param id The id of the entity to add
 */
void
fdb_tmpbittable_add(struct fdb_tmpbittable_t* bittable, 
                    entity_id_t id);

/**
 * \brief Removes an element from the bit table
 *
 * \param bittable The bittable to remove the entity from
 * \param id The id of the entity to remove
 */
void
fdb_tmpbittable_remove(struct fdb_tmpbittable_t* bittable, 
                       entity_id_t id);

/**
 * \brief Gets the bitmap of the bit table for a specific entity id.
 *
 * \param bittable The bittable to get the bitmap from
 * \param id The id of the entity to get the bitmap of 
 *
 * \return Returns a pointer to the bitmap. Returns nullptr if the bitmap does
 * not exist.
 */
struct fdb_bitmap_t* 
fdb_tmpbittable_get_bitmap(struct fdb_tmpbittable_t* bittable,
                           entity_id_t id);

/**
 * \brief Gets the size of the bittable (number of bits set to 1)
 *
 * \param bittable The bittable to get the size of
 *
 * \return  The size of the bit table
 */
uint32_t
fdb_tmpbittable_size(const struct fdb_tmpbittable_t* bittable);

/**
 * \brief Clears the bittable
 */
void
fdb_tmpbittable_clear(struct fdb_tmpbittable_t* bittable);

void
fdb_tmpbittable_union(FDB_RESTRICT(struct fdb_tmpbittable_t*) first, 
                      FDB_RESTRICT(const struct fdb_tmpbittable_t*) second);

void
fdb_tmpbittable_difference(FDB_RESTRICT(struct fdb_tmpbittable_t*) first, 
                           FDB_RESTRICT(const struct fdb_tmpbittable_t*) second);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_BIT_TABLE_H_ */
