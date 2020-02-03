

#ifndef _FDB_DATA_UTILS_H_
#define _FDB_DATA_UTILS_H_

#include "../../common/btree.h"
#include "table.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fdb_bcluster_t fdb_bcluster_t;
typedef struct fdb_bittable_t fdb_bittable_t;

/**
 * \brief Copies the ptr to the given component to the table at the given
 * id.
 *
 * @tparam TComponent
 * \param table_view The view of the temporal table
 * \param id The id of the element to copy the component to
 * \param component The component to be copied
 */
void
copy_component_ptr(uint32_t chunk_size, 
                   uint32_t stride,
                   entity_id_t source,
                   entity_id_t target,
                   FDB_RESTRICT(fdb_btree_t*)* hash_tables, 
                   FDB_RESTRICT(fdb_table_t*)* tables,
                   uint32_t num_tables);


/**
 * \brief Computes the blacklist of entities those entities that cannot be
 * computed (that are not roots of the induced tree) 
 * given a block cluster with a single column of references
 *
 * \param blacklist The block_cluster to find the roots for
 * \param blacklist The bittable to plalce the roots to
 */
void
find_roots_and_blacklist(fdb_bcluster_t* block_cluster, 
                         FDB_RESTRICT(fdb_bittable_t*) roots,
                         FDB_RESTRICT(fdb_bittable_t*) blacklist);

/**
 * \brief Filters the contents of a frontier by the partial black lists in the
 * array
 *
 * \param partial_lists The array of partial blacklists
 * \param current_frontier The current frontier
 */
void
filter_blacklists(FDB_RESTRICT(fdb_bittable_t*) partial_lists[],
                  uint32_t cpartial_lists,
                  FDB_RESTRICT(fdb_bittable_t*) current_frontier);

/**
 * \brief Performs the union of a set of frontiers into a destinaion one
 *
 * \param partial_lists The array of next frontiers 
 * \param current_frontier The destination frontier
 */
void
frontiers_union(FDB_RESTRICT(fdb_bittable_t*) next_frontiers[],
                uint32_t num_frontiers,
                FDB_RESTRICT(fdb_bittable_t*) current_frontier);


/**
 * \brief Given a set of reference groups and a components block, performs the
 * gather operation of this block into the given table view. This operation
 * assumes that the given cluster contains a single column of references
 *
 * \tparam TComponent The componnet type to gather
 * \param cluster The cluster with the references  
 * \param hash_tables An array with the different hash_tables
 * \param chunk_size The chunk_size (used to compute which hash_table to
 * reference)
 * \param stride The stride (used to compute which hash_table to reference)
 * \param table_view The temporal tables to store the references to the components
 */
void
gather(fdb_bcluster_t* cluster,
       FDB_RESTRICT(fdb_btree_t*) hash_tables[],
       uint32_t   chunk_size, 
       uint32_t   stride,
       FDB_RESTRICT(fdb_table_t*)*  tables, 
       uint32_t   num_tables);

/**
 * \brief Builds a block cluster by fetching the blocks with the given start id 
 * from the given tables
 *
 * @tparam typename...TComponents The components of the tables
 * \param ref_cluster The cluster of references 
 * \param current_frontier The current frontier
 * \param next_frontier The next frontier
 * \param cluster The cluster to append to blocks to
 * \param ...table_views The tables to fetch the blocks from
 */
void
build_fdb_bcluster_from_refs(FDB_RESTRICT(fdb_bcluster_t*) ref_cluster,  
                              FDB_RESTRICT(const fdb_bittable_t*) current_frontier,
                              FDB_RESTRICT(fdb_bittable_t*) next_frontier,
                              FDB_RESTRICT(fdb_bcluster_t*) cluster, 
                              FDB_RESTRICT(fdb_table_t*)*  tables, 
                              uint32_t   num_tables);

/**
 * \brief This operation assumes that the "column" column in the block cluster
 * is of type #REFERENCE. This functions filters those rows in the blockcluster
 * such that the entity_id_t corresponding in the column "column" 
 * exists in BitTable.
 *
 * \param bittable The BitTable to check
 * \param block_cluster The block cluster to manipulate
 * \param column The column of the block cluster
 */
void
filter_bittable_exists(const fdb_bittable_t* bittable, 
                       fdb_bcluster_t* block_cluster,
                       uint32_t column);

/**
 * \brief This operation assumes that the "column" column in the block cluster
 * is of type #REFERENCE. This functions filters those rows in the blockcluster
 * such that the entity_id_t corresponding in the column "column" does not
 * exist in BitTable
 *
 * \param bittable The BitTable to check
 * \param block_cluster The block cluster to manipulate
 * \param column The column of the block cluster
 */
void
filter_bittable_not_exists(const fdb_bittable_t* bittable, 
                           fdb_bcluster_t* block_cluster,
                           uint32_t column);
#ifdef __cplusplus
}
#endif
#endif
