
#include "lang/lang.h"
#include "basic_test_header.h"

bool test(const Position* pos, const Velocity* vel)
{
  return true;
}

BEGIN_FDB_SCRIPT

struct UpdatePosition
{
  UpdatePosition(float speed) : m_speed{speed} {}

  void run(fdb_context_t* context,
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

match<Position,Velocity>().filter(test).foreach<UpdatePosition>(1.0);

END_FDB_SCRIPT
