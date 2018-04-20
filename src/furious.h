

#ifndef _FURIOUS_H_
#define _FURIOUS_H_ value

#include "common/common.h"
#include "memory/memory.h"
#include "data/data.h"
#include "runtime/runtime.h"

namespace furious {

/**
 * @brief Initializes the Furious library
 */
void init();

/**
 * @brief Releases the resources of the furious library
 */
void release();

Database* create_database();

void destroy_database(Database* database);


/**
 * @brief Registers a component to the furious framework
 *
 * @tparam TComponent The component to register
 */
template<typename TComponent>
void register_component();

/**
 * @brief Removes a component from the furious framework
 *
 * @tparam TComponent The component to remove 
 */
template<typename TComponent>
void unregister_component();

/**
 * @brief Gets a table view of a component table 
 *
 * @tparam TComponent The component to get the table view for 
 *
 * @return A table view of the component table. 
 */
template<typename TComponent>
TableView<TComponent> get_table();

/**
 * @brief Adds a system to the furious framework
 *
 * @tparam typename TSystem The system to register to the furious framework
 * @tparam typename...TArgs The arguments to initialize the system with 
 */
template<typename TSystem, typename...TArgs> 
void register_system(TArgs&&...);

/**
 * @brief Removes a System from the furious framework
 *
 * @tparam TSystem The system to remove from the furious framework
 */
template<typename TSystem>
void unregister_system();

/**
 * @brief Creates an entity
 *
 * @return A newly created entity
 */
Entity create_entity(Database* database);

/**
 * @brief Removes the given entity
 *
 * @param entiy The entity to remove. 
 */
void remove_entity(Entity entiy);

/**
 * @brief Creates a workload object
 *
 * @return  The newly created workload object
 */
Workload* create_workload();

/**
 * @brief Destroys a workload
 *
 * @param workload A pointer to the workload to destroy
 */
void destroy_workload(Workload* workload);
  
} /* furious */ 

#endif /* ifndef _FURIOUS_H_ */
