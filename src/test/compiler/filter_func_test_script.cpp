
#include "lang/lang.h"
#include "filter_func_test_header.h"

static bool filter(const Position* pos, const Velocity* vel)
{
  return vel->m_x < 2.0f && vel->m_y < 2.0f && vel->m_z < 2.0f;
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

  float m_speed = 1.0f;
};

match<Position,Velocity>().filter(filter)
                          .foreach<UpdatePosition>(1.0f);

END_FDB_SCRIPT
