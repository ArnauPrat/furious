
#ifndef _FILTER_FUNC_TEST_HEADER_H_
#define _FILTER_FUNC_TEST_HEADER_H_ value

#include "furious_macros.h"

FDB_BEGIN_COMPONENT(Position, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FDB_END_COMPONENT

FDB_BEGIN_COMPONENT(Velocity, KILOBYTES(4))
  float m_x;
  float m_y;
  float m_z;
FDB_END_COMPONENT

#endif /* ifndef _BASIC_TEST_HEADER_H_ */
