

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

#define FDB_CREATE_TABLE(database, tx, txtctx, component, dstr) \
  fdb_database_create_table(database,\
                            tx,\
                            txtctx,\
                            #component,\
                            sizeof(component),\
                            dstr)

#define FDB_REMOVE_TABLE(database, tx, txtctx, component)\
  fdb_database_remove_table(database,\
                            tx, \
                            txtctx,\
                        #component)

#define FDB_FIND_TABLE(database, component)\
  fdb_database_find_table(database,\
                      #component)

#define FDB_FIND_OR_CREATE_TABLE(database, tx, txtctx, component, dstr)\
  fdb_database_find_or_create_table(database,\
                                    tx,\
                                    txtctx,\
                                #component,\
                                sizeof(component),\
                                dstr)

#define FDB_CREATE_GLOBAL(database, tx, txtctx, component, dstr) \
  (component*)fdb_database_create_global(database,\
                                         tx,\
                                         txtctx,\
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
  fdb_database_find_table(database, ref_name)

#define FDB_FIND_OR_CREATE_REF_TABLE(database, tx, txtctx, ref_name)\
  fdb_database_find_or_create_table(database, tx, txtctx, ref_name, sizeof(entity_id_t), NULL)

#define FDB_CREATE_REF_TABLE(database, tx, txtctx, ref_name)\
  fdb_database_create_table(database, tx, txtctx, ref_name, sizeof(entity_id_t), NULL)

#define FDB_FIND_TAG_TABLE(database, tx, txtctx, tagname)\
  fdb_database_get_tagged_entities(database, tx, txtctx, tagname)

#define FDB_GET_REFL_DATA(database, component) \
  fdb_mregistry_get_mstruct(fdb_database_get_mregistry(database), #component);

#define FDB_CREATE_TEMP_TABLE(database, component, name, dstr) \
  fdb_database_create_table(database,\
                        name,\
                        sizeof(component),\
                        dstr)

#define FDB_REMOVE_TEMP_TABLE(database, tx, txtctx, component, name)\
  fdb_database_remove_table(database,\
                            tx,\
                            txtctx,\
                        name)

#define FDB_FIND_TEMP_TABLE(database, component, name)\
  fdb_database_find_table(database,\
                      name)

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

#define FDB_ADD_COMPONENT(table, tx, txtctx, component, entity) \
  ((component*)fdb_txtable_create_component(table, tx, txtctx, entity));\
  FDB_ASSERT(strcmp(#component, table->m_name)==0 && "Trying to create a component from the wrong table");

#define FDB_REMOVE_COMPONENT(table, tx, txtctx, entity) \
  fdb_txtable_destroy_component(table, tx, txtctx, entity)

#define FDB_GET_COMPONENT(table, tx, txtctx, component, entity, write) \
  ((component*)fdb_txtable_get_component(table, tx, txtctx, entity, write))

#define FDB_ADD_TAG(bittable, tx, txtctx, entity) \
  fdb_txbittable_add(bittable, tx, txtctx, entity)

#define FDB_ADD_REFERENCE(reftable, tx, txtctx, tail, head) \
  fdb_txtable_add_reference(reftable, tx, txtctx, tail, head)

#endif
