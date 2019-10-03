
#ifndef _EXPAND_TEST_HEADER_H_
#define _EXPAND_TEST_HEADER_H_ value

#include "components.h"

struct Position
{
  FURIOUS_COMPONENT(Position);

  float m_x;
  float m_y;
  float m_z;
};

struct Velocity
{
  FURIOUS_COMPONENT(Velocity);

  float m_x;
  float m_y;
  float m_z;
};

struct Intensity 
{
  FURIOUS_COMPONENT(Intensity);

  float m_intensity;
};

struct FieldMesh 
{
  FURIOUS_COMPONENT(FieldMesh);

  float m_x;
  float m_y;
  float m_z;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct TestComponent1
{
  FURIOUS_COMPONENT(TestComponent1);

  float m_a;
  float m_b;
  float m_c;
};

struct TestComponent2
{
  FURIOUS_COMPONENT(TestComponent2);

  float m_a;
  float m_b;
  float m_c;
};

#endif /* ifndef _BASIC_TEST_HEADER_H_ */
