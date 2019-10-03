

#ifndef _FURIOUS_DATA_MACROS_H_
#define _FURIOUS_DATA_MACROS_H_

#define FURIOUS_CREATE_TABLE(database, Component) \
   (database)->create_table<Component>()

#define FURIOUS_CREATE_TABLE_DESTRUCTOR(database, Component, Destructor) \
   (database)->create_table<Component, Destructor>()

#define FURIOUS_REMOVE_TABLE(database, Component)\
  (database)->remove_table<Component>()

#define FURIOUS_FIND_TABLE(database, Component)\
  (database)->find_table<Component>()

#define FURIOUS_FIND_OR_CREATE_TABLE(database, Component)\
  (database)->find_or_create_table<Component>()

#define FURIOUS_FIND_OR_CREATE_TABLE_DESTRUCTOR(database, Component, Destructor)\
  (database)->find_or_create_table<Component, Destructor>()

#define FURIOUS_EXISTS_TABLE(database, Component)\
  (database)->exists_table<Component>()

#define FURIOUS_CREATE_GLOBAL(database, Component, ...) \
   (database)->create_global<Component>(__VA_ARGS__)

#define FURIOUS_REMOVE_GLOBAL(database, Component)\
  (database)->remove_global<Component>()

#define FURIOUS_FIND_GLOBAL(database, Component)\
  (database)->find_global<Component>()

#define FURIOUS_FIND_OR_CREATE_GLOBAL(database, Component, ...)\
  (database)->find_or_create_global<Component>(__VA_ARGS__)

#define FURIOUS_EXISTS_GLOBAL(database, Component)\
  (database)->exists_global<Component>()

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FURIOUS_CREATE_ENTITY(database) \
  create_entity(database)

#define FURIOUS_DESTROY_ENTITY(entity) \
  destroy_entity(entity)

#define FURIOUS_ADD_COMPONENT(entity,Component, ...) \
  (entity).add_component<Component>(__VA_ARGS__)

#define FURIOUS_REMOVE_COMPONENT(entity, Component) \
  (entity).remove_component<Component>()

#define FURIOUS_GET_COMPONENT(entity, Component) \
  (entity).get_component<Component>()

#define FURIOUS_ADD_TAG(entity, tag) \
  (entity).add_tag(tag)

#define FURIOUS_REMOVE_TAG(entity, tag) \
  (entity).remove_tag(tag)

#define FURIOUS_HAS_TAG(entity, tag) \
  (entity).has_tag(tag)

#define FURIOUS_ADD_REFERENCE(entity, reference, other) \
  (entity).add_reference(reference, other)

#define FURIOUS_REMOVE_REFERENCE(entity, reference, other) \
  (entity).remove_reference(reference, other)

#define FURIOUS_GET_REFERENCE(entity, reference) \
  (entity).get_reference(reference)

#endif
