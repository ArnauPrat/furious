

#ifndef _FURIOUS_DATA_MACROS_H_
#define _FURIOUS_DATA_MACROS_H_


#define FURIOUS_BEGIN_COMPONENT(Component, page_size) \
struct Component  \
{ \
  static const char* __component_name() {return #Component;} \
  static const uint64_t __component_page_size() {return page_size;} \
  static const uint64_t __component_is_global() {return false;}

#define FURIOUS_BEGIN_GLOBAL_COMPONENT(Component) \
struct Component \
{ \
  static const char* __component_name() {return #Component;} \
  static const uint64_t __component_page_size() {return KILOBYTES(4);} \
  static const uint64_t __component_is_global() {return true;}

#define FURIOUS_END_COMPONENT };

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

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
  (database)->exists_table()

#define FURIOUS_CREATE_GLOBAL(database, Component, ...) \
   (database)->create_global<Component>(__VA_ARGS__)

#define FURIOUS_REMOVE_GLOBAL(database, Component)\
  (database)->remove_global()

#define FURIOUS_FIND_GLOBAL(database, Component)\
  (database)->find_global<Component>()

#define FURIOUS_FIND_GLOBAL_NO_LOCK(database, Component)\
  (database)->find_global_no_lock<Component>()

#define FURIOUS_FIND_OR_CREATE_GLOBAL(database, Component, ...)\
  (database)->find_or_create_global<Component>( __VA_ARGS__)

#define FURIOUS_EXISTS_GLOBAL(database, Component)\
  (database)->exists_global()

#define FURIOUS_FIND_REF_TABLE(database, ref_name)\
  (database)->find_ref_table(ref_name)

#define FURIOUS_FIND_OR_CREATE_REF_TABLE(database, ref_name)\
  (database)->find_or_create_ref_table(ref_name)

#define FURIOUS_CREATE_REF_TABLE(database, ref_name)\
  (database)->find_or_create_ref_table(ref_name)

#define FURIOUS_FIND_TAG_TABLE(database, tagname)\
  (database)->get_tagged_entities(tagname)

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FURIOUS_CREATE_ENTITY(database) \
  create_entity(database)

#define FURIOUS_DESTROY_ENTITY(entity) \
  destroy_entity(entity)

#define FURIOUS_ADD_COMPONENT(entity, Component, ...) \
  (entity).add_component<Component>( __VA_ARGS__)

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

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FURIOUS_ADD_REFL_DATA(database, Component, ref_data)\
  (database)->add_refl_data<Component>(ref_data)

#define FURIOUS_GET_REFL_DATA(database, Component) \
  (database)->get_refl_data<TestComponent>();

#endif
