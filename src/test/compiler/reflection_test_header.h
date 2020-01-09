
#ifndef _REFLECTION_TEST_HEADER_H_
#define _REFLECTION_TEST_HEADER_H_ value

#include "furious_macros.h"

FURIOUS_BEGIN_COMPONENT(TestComponent, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;

  struct {
    float m_q;
    float m_t;
  };

  union {
    float m_a;
    float m_b;
  } X;
FURIOUS_END_COMPONENT


#endif /* ifndef _BASIC_TEST_HEADER_H_ */

