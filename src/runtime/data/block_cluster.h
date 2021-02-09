
#ifndef _FDB_fdb_bcluster_H_H
#define _FDB_fdb_bcluster_H_H value

#include "../../common/bitmap.h"
#include "../../common/platform.h"
#include "../../common/memory/pool_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FDB_MAX_CLUSTER_SIZE 16

struct fdb_txtable_block_t;
struct fdb_tx_t;
struct fdb_txthread_ctx_t;
struct fdb_tmptable_block_t;

struct fdb_bcluster_factory_t
{
  struct fdb_bitmap_factory_t m_cbitmap_factory;
  struct fdb_bitmap_factory_t m_glbitmap_factory;
};

/**
 * \brief This is used to represent a joined block of data, mostly table blocks.
 * However, it can also contain pointers to global data.
 */
struct fdb_bcluster_t 
{
  struct fdb_bcluster_factory_t*    p_factory;
  uint32_t                          m_num_columns;
  struct fdb_bitmap_t               m_enabled;
  struct fdb_bitmap_t               m_global;
  uint32_t                          m_start;
  uint32_t                          m_sizes[FDB_MAX_CLUSTER_SIZE];
  void*                             p_blocks[FDB_MAX_CLUSTER_SIZE]; 
};

/**
 * \brief Initializes a bcluster factory
 *
 * \param bcluster_factory The bcluster factory to initialize
 * \param allocator The parent allocator to use. NULL to use the global
 * allocator
 */
void
fdb_bcluster_factory_init(struct fdb_bcluster_factory_t* bcluster_factory, 
                          struct fdb_mem_allocator_t* allocator);

/**
 * \brief Releases a blcuster factory
 *
 * \param bcluster_factory The bcluster factory to release
 */
void
fdb_bcluster_factory_release(struct fdb_bcluster_factory_t* bcluster_factory);

/**
 * \brief inits a block cluster
 *
 * \param bc The bcluster to initialize
 * \param factory The factory to allocate the cluster from
 *
 */
void
fdb_bcluster_init(struct fdb_bcluster_t* bc, 
                  struct fdb_bcluster_factory_t* factory);

/**
 * \brief releases a block cluster
 *
 * \param bc The block cluster to release
 */
void
fdb_bcluster_release(struct fdb_bcluster_t* bc);


/**
 * \brief Appends a txtable block to the block cluster
 *
 * \param bc The block cluster
 * \param block The block to append
 * \param tx The transaction
 * \param txtctx The transaction thread context
 * \param write Whether this is a block to be written to or not.
 */
void 
fdb_bcluster_append_txtable_block(struct fdb_bcluster_t* bc, 
                                  struct fdb_txtable_block_t* block, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx, 
                                  bool write);

/**
 * \brief Appends a tmptable block to the block cluster
 *
 * \param bc The block cluster
 * \param block The block to append
 * \param tx The transaction
 * \param txtctx The transaction thread context
 * \param write Whether this is a block to be written to or not.
 */
void 
fdb_bcluster_append_tmptable_block(struct fdb_bcluster_t* bc, 
                                   struct fdb_tmptable_block_t* block);

/**
 * \brief Appends a global to the block cluster
 *
 * \param bc The block cluster
 * \param block The global to append
 */
void 
fdb_bcluster_append_global(struct fdb_bcluster_t* bc, 
                           void* global, 
                           size_t esize);

void 
fdb_bcluster_append_cluster(FDB_RESTRICT(struct fdb_bcluster_t*) bc,
                            FDB_RESTRICT(const struct fdb_bcluster_t*) other);

void 
fdb_bcluster_filter(FDB_RESTRICT(struct fdb_bcluster_t*) bc,
                    FDB_RESTRICT(const struct fdb_bcluster_t*) other);

bool
fdb_bcluster_has_elements(struct fdb_bcluster_t* bc);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _FDB_fdb_bcluster_H_H */
