
#include "furious.h"

namespace furious {

Database* database = nullptr;
Workload* workload = nullptr;

void init() {
  database = Database::get_instance();
  workload = new Workload();
}

void release() {
  database->clear();
  database = nullptr;

  delete workload;
  workload = nullptr;
}

Entity create_entity() {
 return Entity::create_entity();
}

void remove_entity(Entity entity) {
  Entity::remove_entity(entity);
}

} /* furious */ 

