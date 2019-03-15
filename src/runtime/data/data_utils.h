

#ifndef _FURIOUS_DATA_UTILS_H_
#define _FURIOUS_DATA_UTILS_H_

#include "../../common/btree.h"
#include "../../common/dyn_array.h"

#include "table_view.h"


namespace furious
{

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
template <typename TComponent>
void
copy_component_raw(TableView<TComponent>* table_view, 
                   entity_id_t id, 
                   const TComponent* component );


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
                 TableView<entity_id_t>::Block* block);

/**
 * \brief Given a set of reference groups and a components block, performs the
 * gather operation of this block into the given table view
 *
 * @tparam TComponent The componnet type to gather
 * \param groups The reference groups 
 * \param block The block to perform the gather from
 * \param table_view The table_view to store the gathered components
 */
template <typename TComponent>
void
gather(const BTree<DynArray<entity_id_t>>* groups, 
       const typename TableView<TComponent>::Block* block,
       TableView<TComponent>* table_view);

  
} /* furious */ 

#include "data_utils.inl"


#endif
