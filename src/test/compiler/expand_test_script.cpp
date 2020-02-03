
#include "lang/lang.h"
#include "expand_test_header.h"

BEGIN_FDB_SCRIPT

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


struct SimpleSystem 
{
  void run(fdb_context_t* context,
           uint32_t id,
           SimpleComponent1* simple_component1,
           const SimpleComponent2* simple_component2)
  {
    simple_component1->m_x = simple_component2->m_x;
    simple_component1->m_y = simple_component2->m_y;
    simple_component1->m_z = simple_component2->m_z;
  }
};

match<SimpleComponent1>().expand<SimpleComponent2>("parent").foreach<SimpleSystem>();

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct UpdatePosition
{
  UpdatePosition(float speed) : m_speed{speed} {}

  void run(fdb_context_t* context,
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

struct ResetPosition
{
  void run(fdb_context_t* context,
           uint32_t id,
           Position* position)
  {
    position->m_x = 1.0f;
    position->m_y = 1.0f;
    position->m_z = 1.0f;
  }
};

struct PropagateIntensity
{
  void run(fdb_context_t* context,
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
  void run(fdb_context_t* context,
           uint32_t id,
           const FieldMesh* field)
  {
    // DRAW FIELD MESH
  }

};

struct IncrementFieldMesh
{
  void run(fdb_context_t* context,
           uint32_t id,
           FieldMesh* field)
  {

    // DRAW FIELD MESH
    field->m_x += 1.0;
    field->m_y += 1.0;
    field->m_z += 1.0;
  }
};

match<Position>().foreach<ResetPosition>();
match<Position,Velocity>().expand<Position>("parent").foreach<UpdatePosition>(1.0f).set_priority(2);
match<FieldMesh>().foreach<DrawFieldMesh>();
match<FieldMesh>().expand<Position,Intensity>("parent").foreach<PropagateIntensity>();
match<FieldMesh>().expand<>("parent").has_tag("test").foreach<IncrementFieldMesh>().set_priority(2);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


struct TestSystemCascading
{
  void run(fdb_context_t* context, 
           uint32_t id, 
           TestComponent1* comp_1,
           const TestComponent1* parent_comp_1,
           const TestComponent2* parent_comp_2)
  {
  }
};

match<TestComponent1>().expand<TestComponent1, TestComponent2>("parent").foreach<TestSystemCascading>();

END_FDB_SCRIPT
