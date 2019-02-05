

#ifndef _FURIOUS_DATA_MACROS_H_
#define _FURIOUS_DATA_MACROS_H_

#define FURIOUS_CREATE_TABLE(database, Component) \
   database->create_table<Component>(#Component)

#define FURIOUS_REMOVE_TABLE(database, Component)\
  database->remove_table<Component>(#Component)

#define FURIOUS_FIND_TABLE(database, Component)\
  database->find_table<Component>(#Component)

#define FURIOUS_FIND_OR_CREATE_TABLE(database, Component)\
  database->find_or_create_table<Component>(#Component)

#define FURIOUS_EXISTS_TABLE(database, Component)\
  database->exists_table<Component>(#Component)

#define FURIOUS_ADD_COMPONENT(entity,Component, ...) \
  entity->add_component<Component>(#Component, ##__VA_ARGS__)

#define FURIOUS_REMOVE_COMPONENT(entity, Component) \
  entity->remove_component<Component>(#Component)

#define FURIOUS_GET_COMPONENT(entity, Component) \
  entity->get_component<Component>(#Component)

#endif
