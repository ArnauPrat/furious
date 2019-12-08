
#ifndef _FURIOUS_BITMAP_H_
#define _FURIOUS_BITMAP_H_ value

#include "types.h"

namespace furious
{

extern uint8_t count_bits_lookup[16];

template <int MAX_BITS>
struct bitmap_t 
{
  uint32_t m_num_set;           //< The number of bits set to one
  uint8_t  m_data[MAX_BITS];    //< The buffer with the bitmap
};

} /* furious
 */ 

#include "bitmap.inl"

#endif /* ifndef _FURIOUS_BITMAP_H_ */



