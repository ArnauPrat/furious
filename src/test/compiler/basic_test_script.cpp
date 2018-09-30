
#include "furious.h"
#include "basic_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct UpdatePosition
{
  void run(furious::Context* context,
           int32_t id,
           Position* position,
           const Velocity* velocity)
  {
    position->m_x = position->m_x + velocity->m_x*context->m_dt;
    position->m_y = position->m_y + velocity->m_y*context->m_dt;
    position->m_z = position->m_z + velocity->m_z*context->m_dt;
  }
};

furious::select<Position,Velocity>().foreach<UpdatePosition>();

END_FURIOUS_SCRIPT
