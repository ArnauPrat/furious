
#include "furious.h"
#include "filter_tag_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct UpdatePosition
{
  UpdatePosition(float speed) : m_speed{speed} {}

  void run(furious::Context* context,
           int32_t id,
           Position* position,
           const Velocity* velocity)
  {
    position->m_x = position->m_x + velocity->m_x*context->m_dt*m_speed;
    position->m_y = position->m_y + velocity->m_y*context->m_dt*m_speed;
    position->m_z = position->m_z + velocity->m_z*context->m_dt*m_speed;
  }

  float m_speed = 1.0f;
};

furious::select<Position,Velocity>().has_tag("affected")
                                    .has_not_tag("not_affected")
                                    .foreach<UpdatePosition>(1.0f);

END_FURIOUS_SCRIPT
