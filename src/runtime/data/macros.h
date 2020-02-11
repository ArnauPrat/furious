

#ifndef _FDB_DATA_MACROS_H_
#define _FDB_DATA_MACROS_H_

#include "../../common/platform.h"
#include <string.h>

#define FDB_BEGIN_COMPONENT(Component, page_size) \
struct Component  \
{ 
  /*static const char* __component_name() {return #Component;} \
  static const uint64_t __component_page_size() {return page_size;} \
  static const uint64_t __component_is_global() {return false;}*/

#define FDB_BEGIN_GLOBAL_COMPONENT(Component) \
struct Component \
{ 
  /*static const char* __component_name() {return #Component;} \
  //static const uint64_t __component_page_size() {return KILOBYTES(4);} \
  //static const uint64_t __component_is_global() {return true;}*/

#define FDB_END_COMPONENT };

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FDB_CREATE_TABLE(database, component, dstr) \
  fdb_database_create_table(database,\
                        #component,\
                        sizeof(component),\
                        dstr)

#define FDB_REMOVE_TABLE(database, component)\
  fdb_database_remove_table(database,\
                        #component)

#define FDB_FIND_TABLE(database, component)\
  fdb_database_find_table(database,\
                      #component)

#define FDB_FIND_OR_CREATE_TABLE(database, component, dstr)\
  fdb_database_find_or_create_table(database,\
                                #component,\
                                sizeof(component),\
                                dstr)

#define FDB_CREATE_GLOBAL(database, component, dstr) \
  (component*)fdb_database_create_global(database,\
                                     #component,\
                                     sizeof(component),\
                                     dstr)

#define FDB_REMOVE_GLOBAL(database, component)\
  fdb_database_remove_global(database,\
                         #component)

#define FDB_FIND_GLOBAL(database, component)\
  (component*)fdb_database_find_global(database\
                       #component)

#define FDB_FIND_GLOBAL_NO_LOCK(database, component)\
  (component*)fdb_database_find_global_no_lock(database,\
                               #component)

#define FDB_FIND_OR_CREATE_GLOBAL(database, component, dstr)\
  (component*)fdb_database_find_or_create_global<Component>(database,\
                                            #component\
                                            sizeof(component)\
                                            destr)

#define FDB_FIND_REF_TABLE(database, ref_name)\
  fdb_database_find_reftable(database, ref_name)

#define FDB_FIND_OR_CREATE_REF_TABLE(database, ref_name)\
  fdb_database_find_or_create_reftable(database, ref_name)

#define FDB_CREATE_REF_TABLE(database, ref_name)\
  fdb_database_find_or_create_reftable(database, ref_name)

#define FDB_FIND_TAG_TABLE(database, tagname)\
  fdb_database_get_tagged_entities(database, tagname)

#define FDB_GET_REFL_DATA(database, component) \
  fdb_mregistry_get_mstruct(fdb_database_get_mregistry(database), #component);

#define FDB_CREATE_TEMP_TABLE(database, component, name, dstr) \
  fdb_database_create_table(database,\
                        name,\
                        sizeof(component),\
                        dstr)

#define FDB_REMOVE_TEMP_TABLE(database, component, name)\
  fdb_database_remove_table(database,\
                        name)

#define FDB_FIND_TEMP_TABLE(database, component, name)\
  fdb_database_find_table(database,\
                      name)

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FDB_ADD_COMPONENT(table, component, entity) \
  ((component*)fdb_table_create_component(table, entity));\
  FDB_ASSERT(strcmp(#component, table->m_name)==0 && "Trying to create a component from the wrong table");

#define FDB_REMOVE_COMPONENT(table, entity) \
  fdb_table_destroy_component(table, entity)

#define FDB_GET_COMPONENT(table, component, entity) \
  ((component*)fdb_table_get_component(table, entity))

#define FDB_ADD_TAG(bittable, entity) \
  fdb_bittable_add(bittable, entity)

#define FDB_REMOVE_TAG(entity, tag) \
  (entity).remove_tag(tag)

#define FDB_HAS_TAG(entity, tag) \
  (entity).has_tag(tag)

#define FDB_ADD_REFERENCE(reftable, tail, head) \
  fdb_reftable_add(reftable, tail, head)

#define FDB_REMOVE_REFERENCE(entity, reference, other) \
  (entity).remove_reference(reference, other)

#define FDB_GET_REFERENCE(entity, reference) \
  (entity).get_reference(reference)

#endif
