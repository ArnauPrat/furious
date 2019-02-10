
#ifndef _FURIOUS_ENTITY_H_ 
#define _FURIOUS_ENTITY_H_

#include "../../common/common.h"

namespace furious 
{

class Database;

using entity_id_t = int32_t;

class Entity final 
{
public: 
  Entity( Database* database  );
  ~Entity() = default;

  Entity( const Entity& entity) = default;

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

  void add_tag(const std::string& tag);

  void remove_tag(const std::string& tag);

  bool has_tag(const std::string& tag);
  
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
  static void remove_entity(Entity entity);

  Database* get_database();

  entity_id_t m_id;

private:
  Database*   p_database;
};

/**
 * @brief Creates an entity
 *
 * @return A newly created entity
 */
Entity 
create_entity(Database* database);

/**
 * @brief Removes the given entity
 *
 * @param entiy The entity to remove. 
 */
void 
destroy_entity(Entity entity);
  
} /* furious */ 

#include "entity.inl"

#endif /* ifndef _FURIOUS_ENTITY_H_H */
