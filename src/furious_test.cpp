
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
    position->m_x = direction->m_x*context->m_dt;
    position->m_y = direction->m_y*context->m_dt;
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

  furious::init();

  furious::register_component<PositionComponent>();
  furious::register_component<DirectionComponent>();
  furious::register_component<ComflabulationComponent>();

  furious::register_system<MovementSystem>();
  furious::register_system<ComflabSystem>();

  size_t nentities = 10000;
  for (size_t i = 0; i < nentities; i++) {
    auto entity = furious::create_entity();

    entity.add_component<PositionComponent>();
    entity.add_component<DirectionComponent>();

    if (i % 2) {
      entity.add_component<ComflabulationComponent>();
    }

  }

  furious::run(1.0f);

  furious::unregister_component<PositionComponent>();
  furious::unregister_component<DirectionComponent>();
  furious::unregister_system<MovementSystem>();
  furious::unregister_system<ComflabSystem>();

  furious::release();

  return 0;
}
