

#ifndef _FURIOUS_DATA_UTILS_H_
#define _FURIOUS_DATA_UTILS_H_

#include "../../common/btree.h"
#include "../../common/dyn_array.h"

#include "table_view.h"


namespace furious
{

struct BlockCluster;
struct BitTable;

/**
 * \brief Copies the raw data of the given component to the table at the given
 * id. This is a raw copy, so it is not performed using any copy constructor nor
 * operator=. This is meant to be performed on temporal tables, where the
 * destructor of the components is not called when the table is destroyed.
 *
 * @tparam TComponent
 * \param table_view The view of the temporal table
 * \param id The id of the element to copy the component to
 * \param component The component to be copied
 */
void
copy_component_raw(const BlockCluster* cluster,
                   entity_id_t source,
                   entity_id_t target,
                   Table** tables,
                   uint32_t num_tables);


/**
 * \brief Performs a grouping operation over the elements in the given block.
 * That is, for each value in the block, it groups the entities that have that
 * value and put them in the provided btree.
 *
 * \param btree The btree to perform the groups 
 * \param block The block to process
 */
void
group_references(BTree<DynArray<entity_id_t>>* groups, 
                 TBlock* block);

/**
 * \brief Finds the roots of a groups (those entities that form the roots of the
 * tree induced by the references)
 *
 * \param groups The groups to get the roots from
 * \param roots The bittable to plalce the roots to
 */
void
find_roots(const BTree<DynArray<entity_id_t>>* groups, 
           BitTable* roots);

/**
 * \brief Given a set of reference groups and a components block, performs the
 * gather operation of this block into the given table view
 *
 * \tparam TComponent The componnet type to gather
 * \param groups The reference groups 
 * \param block The block to perform the gather from
 * \param table_view The table_view to store the gathered components
 */
template <typename...TComponents>
void
gather(const BTree<DynArray<entity_id_t>>* groups, 
       const BlockCluster* cluster,
       TableView<TComponents>*...table_view);

/**
 * \brief Given a set of reference groups and a components block, performs the
 * gather operation of this block into the given table view
 *
 * \tparam TComponent The componnet type to gather
 * \param groups The reference groups 
 * \param block The block to perform the gather from
 * \param current_frontier The bittable with the current_frontier to filter the gather. 
 * \param next_frontier The bittable with the next_frontier to filter the gather. 
 * \param table_view The table_view to store the gathered components
 */
template <typename...TComponents>
void
gather(const BTree<DynArray<entity_id_t>>* groups, 
       const BlockCluster* cluster,
       BitTable* current_frontier,
       BitTable* next_frontier,
       TableView<TComponents>*...table_view);

  
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
filter_bittable_exists(const BitTable* bittable, 
                       BlockCluster* block_cluster,
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
filter_bittable_not_exists(const BitTable* bittable, 
                           BlockCluster* block_cluster,
                           uint32_t column);
} /* furious */ 

#include "data_utils.inl"


#endif
