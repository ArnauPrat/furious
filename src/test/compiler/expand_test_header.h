
#ifndef _EXPAND_TEST_HEADER_H_
#define _EXPAND_TEST_HEADER_H_ value

#include "furious_macros.h"

FURIOUS_BEGIN_COMPONENT(SimpleComponent1, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(SimpleComponent2, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(Position, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(Velocity, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(Intensity, KILOBYTES(4))
  float m_intensity;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(FieldMesh, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FURIOUS_END_COMPONENT

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FURIOUS_BEGIN_COMPONENT(TestComponent1, KILOBYTES(4))
  float m_a;
  float m_b;
  float m_c;
FURIOUS_END_COMPONENT

FURIOUS_BEGIN_COMPONENT(TestComponent2, KILOBYTES(4))
  float m_a;
  float m_b;
  float m_c;
FURIOUS_END_COMPONENT

#endif /* ifndef _BASIC_TEST_HEADER_H_ */
