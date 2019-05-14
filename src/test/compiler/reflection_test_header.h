
#ifndef _REFLECTION_TEST_HEADER_H_
#define _REFLECTION_TEST_HEADER_H_ value

#include "components.h"

struct TestComponent 
{
  FURIOUS_COMPONENT(TestComponent);

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


};

#endif /* ifndef _BASIC_TEST_HEADER_H_ */

