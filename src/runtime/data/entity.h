
#ifndef _FURIOUS_ENTITY_H_ 
#define _FURIOUS_ENTITY_H_

#include "../../common/types.h"
#include "common.h"

#include <string>

namespace furious 
{

struct Database;

class Entity 
{
public: 
  Entity();
  Entity(Database* database,
          entity_id_t id);
  Entity( Database* database  );
  ~Entity() = default;

  Entity( const Entity& entity) = default;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

public:

  template<typename TComponent, typename...Args>
  TComponent* 
  add_component(Args&&...args);

  template<typename TComponent>
  void 
  remove_component();

  template<typename TComponent> 
  TComponent* 
  get_component();

  void 
  add_tag(const char* tag);

  void 
  remove_tag(const char* tag);

  bool 
  has_tag(const char* tag);

  void
  add_reference(const char* ref_name,
                Entity other);

  void
  remove_reference(const char* ref_name);

  Entity
  get_reference(const char* ref_name);

  bool
  is_valid();
  
  /**
   * @brief Creates an entity 
   *
   * @return 
   */
  static Entity 
  create_entity(Database* database);

  /**
   * @brief Removes the given entity
   *
   * @param The entity to remove
   */
  static void 
  remove_entity(Entity entity);

  Database* get_database();

  uint32_t m_id;

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

/**
 * \brief Gets the handler to an existing entity
 *
 * \param database
 * \param id
 *
 * \return 
 */
Entity
get_entity(Database* database,
           entity_id_t id);
  
} /* furious */ 


#endif /* ifndef _FURIOUS_ENTITY_H_H */

#include "entity.inl"
