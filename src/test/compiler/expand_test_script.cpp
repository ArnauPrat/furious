
#include "lang/lang.h"
#include "expand_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct UpdatePosition
{
  UpdatePosition(float speed) : m_speed{speed} {}

  void run(furious::Context* context,
           uint32_t id,
           Position* position,
           const Velocity* velocity,
           const Position* parent_position)
  {
    position->m_x = parent_position->m_x + velocity->m_x*context->m_dt*m_speed;
    position->m_y = parent_position->m_y + velocity->m_y*context->m_dt*m_speed;
    position->m_z = parent_position->m_z + velocity->m_z*context->m_dt*m_speed;
  }

  float m_speed = 1.0;
};

struct DrawFieldMesh
{
  void run(furious::Context* context,
           uint32_t id,
           FieldMesh* field,
           const Position* parent_position,
           const Intensity* parent_intensity)
  {
    // DRAW FIELD MESH
  }

};

furious::match<Position,Velocity>().expand<Position>("parent").foreach<UpdatePosition>(1.0);
furious::match<FieldMesh>().expand<Position,Intensity>("parent").foreach<DrawFieldMesh>();

END_FURIOUS_SCRIPT
