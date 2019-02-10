
#ifndef _FILTER_LAMBDA_TEST_HEADER_H_
#define _FILTER_LAMBDA_TEST_HEADER_H_ value

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

#endif /* ifndef _BASIC_TEST_HEADER_H_ */
