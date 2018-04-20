
#include "furious.h"

namespace furious {

void init() {
}

void release() {
}

Database* create_database() {
  return new Database();
}

void destroy_database(Database* database) {
  delete database;
}

Entity create_entity(Database* database) {
 return Entity::create_entity(database);
}

void remove_entity(Entity entity, Database* database) {
  Entity::remove_entity(&entity);
}


Workload* create_workload() {
  return new Workload();
}

void destroy_workload(Workload* workload) {
  delete workload;
}


} /* furious */ 

