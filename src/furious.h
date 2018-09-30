

#ifndef _FURIOUS_H_
#define _FURIOUS_H_

#include "common/common.h"
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

/**
 * @brief Creates a new database
 *
 * @return  The newly created database
 */
Database* create_database();

/**
 * @brief Destroys a given database
 *
 * @param database A pointer to the database to destroy
 */
void destroy_database(Database* database);

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
void destroy_entity(Entity* entiy);

} /* furious */ 

#endif /* ifndef _FURIOUS_H_ */
