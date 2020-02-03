
#ifndef _FDB_fdb_bcluster_H_H
#define _FDB_fdb_bcluster_H_H value

#include "../../common/bitmap.h"
#include "../../common/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_MAX_CLUSTER_SIZE 16


typedef struct fdb_table_block_t fdb_table_block_t;

/**
 * \brief This is used to represent a joined block of data, mostly table blocks.
 * However, it can also contain pointers to global data.
 */
typedef struct fdb_bcluster_t 
{
  uint32_t         m_num_columns;
  fdb_bitmap_t     m_enabled;
  fdb_bitmap_t     m_global;
  uint32_t         m_start;
  void*            p_blocks[FDB_MAX_CLUSTER_SIZE]; 
} fdb_bcluster_t;

/**
 * \brief inits a block cluster
 *
 * \param allocator The allocator to use
 *
 * \return Returns a block cluster
 */
void
fdb_bcluster_init(fdb_bcluster_t* bc, 
                  fdb_mem_allocator_t* allocator);

/**
 * \brief releases a block cluster
 *
 * \param bc The block cluster to release
 */
void
fdb_bcluster_release(fdb_bcluster_t* bc, 
                     fdb_mem_allocator_t* allocator);


/**
 * \brief Appends a table block to the block cluster
 *
 * \param bc The block cluster
 * \param block The block to append
 */
void 
fdb_bcluster_append_block(fdb_bcluster_t* bc, 
                          fdb_table_block_t* block);

/**
 * \brief Appends a global to the block cluster
 *
 * \param bc The block cluster
 * \param block The global to append
 */
void 
fdb_bcluster_append_global(fdb_bcluster_t* bc, 
                           void* global);

void 
fdb_bcluster_append_cluster(FDB_RESTRICT(fdb_bcluster_t*) bc,
                            FDB_RESTRICT(const fdb_bcluster_t*) other);

void 
fdb_bcluster_filter(FDB_RESTRICT(fdb_bcluster_t*) bc,
                    FDB_RESTRICT(const fdb_bcluster_t*) other);

fdb_table_block_t* 
fdb_bcluster_get_tblock(fdb_bcluster_t* bc,
                        uint32_t index);

void* 
fdb_bcluster_get_global(fdb_bcluster_t* bc,
                        uint32_t index);

bool
fdb_bcluster_has_elements(fdb_bcluster_t* bc);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_fdb_bcluster_H_H */
