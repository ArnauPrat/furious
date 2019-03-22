
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
    position->m_x = (position->m_x + parent_position->m_x) * velocity->m_x*context->m_dt*m_speed;
    position->m_y = (position->m_y + parent_position->m_y) * velocity->m_y*context->m_dt*m_speed;
    position->m_z = (position->m_z + parent_position->m_z) * velocity->m_z*context->m_dt*m_speed;
  }

  float m_speed = 1.0f;
};

struct IncrementPosition
{
  void run(furious::Context* context,
           uint32_t id,
           Position* position)
  {
    position->m_x += 1.0f;
    position->m_y += 1.0f;
    position->m_z += 1.0f;
  }
};

struct PropagateIntensity
{
  void run(furious::Context* context,
           uint32_t id,
           FieldMesh* field,
           const Position* parent_position,
           const Intensity* parent_intensity)
  {
    // DRAW FIELD MESH
    field->m_x = parent_position->m_x*parent_intensity->m_intensity;
    field->m_y = parent_position->m_y*parent_intensity->m_intensity;
    field->m_z = parent_position->m_z*parent_intensity->m_intensity;
  }

};

struct DrawFieldMesh 
{
  void run(furious::Context* context,
           uint32_t id,
           const FieldMesh* field)
  {
    // DRAW FIELD MESH
  }

 float m_x;
 float m_y;
 float m_z;

};

furious::match<Position>().foreach<IncrementPosition>().outputwriteonly();
furious::match<Position,Velocity>().expand<Position>("parent").foreach<UpdatePosition>(1.0f);
furious::match<FieldMesh>().foreach<DrawFieldMesh>();
furious::match<FieldMesh>().expand<Position,Intensity>("parent").foreach<PropagateIntensity>();

END_FURIOUS_SCRIPT
