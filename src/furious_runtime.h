
#ifndef _FURIOUS_RUNTIME_H_
#define _FURIOUS_RUNTIME_H_ value

#include "furious.h"

namespace furious
{

class Database;

/**
 * @brief Initializes the Furious library
 */
void init();

/**
 * @brief Releases the resources of the furious library
 */
void release();

/**
 * @brief Updates one frame
 *
 * @param delta The elapsed delta used to update the frame
 */
void update(float delta);

/**
 * @brief Creates a new database
 *
 * @return  The newly created database
 */
Database* get_database();
  
} /* furious
 */ 

#endif /* ifndef _FURIOUS_RUNTIME_H_ */
