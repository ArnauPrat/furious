
#ifndef _FURIOUS_ENTITY_H_ 
#define _FURIOUS_ENTITY_H_

#include "../common/common.h"

namespace furious {

class Database;

using entity_id_t = int32_t;

class Entity final {
private:

  Entity( Database* database  );

public: 
  virtual ~Entity() = default;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

public:

  template<typename TComponent, typename...Args>
  void add_component(Args&&...args);

  template<typename TComponent>
  void remove_component();

  template<typename TComponent> 
  TComponent* get_component();
  
  /**
   * @brief Creates an entity 
   *
   * @return 
   */
  static Entity create_entity(Database* database);

  /**
   * @brief Removes the given entity
   *
   * @param The entity to remove
   */
  static void remove_entity( Entity* entity);

  Database* get_database();
private:

  entity_id_t m_id;
  Database*   p_database;

  static entity_id_t m_next_id;
  
};
  
} /* furious */ 

#include "entity.inl"

#endif /* ifndef _FURIOUS_ENTITY_H_H */
