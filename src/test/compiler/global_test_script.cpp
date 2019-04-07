
#include "lang/lang.h"
#include "global_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct UpdatePosition
{
  UpdatePosition() {}

  void run(furious::Context* context,
           uint32_t id,
           Position* position,
           const GlobalComponent* global)
  {
    position->m_x = (position->m_x + global->m_x);
    position->m_y = (position->m_y + global->m_y);
    position->m_z = (position->m_z + global->m_z);
  }

};

furious::match<Position,furious::Global<GlobalComponent>>().foreach<UpdatePosition>();


END_FURIOUS_SCRIPT
