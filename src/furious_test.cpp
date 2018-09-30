
#include "furious.h"
#include <string>
#include <iostream>

struct PositionComponent {
  float m_x; 
  float m_y;
};

struct DirectionComponent {
  float m_x; 
  float m_y;
};

struct ComflabulationComponent {
  float       m_thingy; 
  int         m_dingy;
  bool        m_mingy;
  std::string m_stringy;
};

struct MovementSystem {
  void run(furious::Context* context, int32_t id, PositionComponent* position, const DirectionComponent*  direction) {
    position->m_x = direction->m_x*context->get_dt();
    position->m_y = direction->m_y*context->get_dt();
  }
};

struct ComflabSystem {

  void run(furious::Context* context, int32_t id, ComflabulationComponent*  comflab) {
    comflab->m_thingy *= 1.000001f;
    comflab->m_mingy = !comflab->m_mingy;
    comflab->m_dingy++;
  }

};

int main(int argc, char** argv) {

  /*
  furious::init();

  furious::Database* database = furious::create_database();

  database->add_table<PositionComponent>();
  database->add_table<DirectionComponent>();
  database->add_table<ComflabulationComponent>();

  furious::Workload* workload = furious::create_workload();
  workload->add_system<MovementSystem>();
  workload->add_system<ComflabSystem>();

  size_t nentities = 10000;
  for (size_t i = 0; i < nentities; i++) {
    auto entity = furious::create_entity(database);

    entity.add_component<PositionComponent>();
    entity.add_component<DirectionComponent>();

    if (i % 2) {
      entity.add_component<ComflabulationComponent>();
    }
  }

  furious::Backend* backend = new furious::Basic();

  backend->compile(workload, database);
  backend->run(1.0);

  database->remove_table<PositionComponent>();
  database->remove_table<DirectionComponent>();
  workload->remove_system<MovementSystem>();
  workload->remove_system<ComflabSystem>();

  furious::destroy_workload(workload);
  furious::destroy_database(database);
  furious::release();
  */

  return 0;
}
