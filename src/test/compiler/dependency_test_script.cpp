
#include "lang/lang.h"
#include "dependency_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct UpdateVelocity
{
  void run(furious::Context* context,
           uint32_t id,
           Velocity* velocity)
  {
    velocity->m_x *=-1.0;
    velocity->m_y *=-1.0;
    velocity->m_z *=-1.0;
  }

};

struct UpdatePosition
{
  UpdatePosition(float speed) : m_speed{speed} {}

  void run(furious::Context* context,
           uint32_t id,
           Position* position,
           const Velocity* velocity)
  {
    position->m_x = position->m_x + velocity->m_x*context->m_dt*m_speed;
    position->m_y = position->m_y + velocity->m_y*context->m_dt*m_speed;
    position->m_z = position->m_z + velocity->m_z*context->m_dt*m_speed;
  }

  float m_speed = 1.0;
};

furious::select<Position,Velocity>().foreach<UpdatePosition>(1.0);
furious::select<Velocity>().foreach<UpdateVelocity>();

END_FURIOUS_SCRIPT
