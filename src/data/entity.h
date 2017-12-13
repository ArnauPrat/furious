
#ifndef _FURIOUS_ENTITY_H_ 
#define _FURIOUS_ENTITY_H_

#include "../common/common.h"

namespace furious {

using entity_id_t = uint32_t;

class Entity final {
private:

  Entity( entity_id_t id  );

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
  static Entity create_entity();

  /**
   * @brief Removes the given entity
   *
   * @param The entity to remove
   */
  static void remove_entity( Entity& entity );

private:

  entity_id_t m_id;

  static entity_id_t m_next_id;
  
};
  
} /* furious */ 

#include "entity.inl"

#endif /* ifndef _FURIOUS_ENTITY_H_H */
