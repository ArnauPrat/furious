#include "types.h"
#include "platform.h"

namespace furious
{

// Lookup table used to efficiently implement refresh_num_set operation
uint8_t count_bits_lookup[16] = {
  0, // 0:0000 
  1, // 1:0001 
  1, // 2:0010 
  2, // 3:0011
  1, // 4:0100
  2, // 5:0101
  2, // 6:0110
  3, // 7:0111
  1, // 8:1000
  2, // 9:1001
  2, // 10:1010
  3, // 11:1011
  2, // 12:1100
  3, // 13:1101
  3, // 14:1110
  4, // 15:1111
};

bool 
set_bit_true(uint8_t* data, uint32_t bit)
{
  FURIOUS_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint8_t val = *data;
  uint8_t mask =  1 << bit;
  val = val | mask;
  bool changed = *data != val;
  *data = val;
  return changed;
}

bool 
set_bit_false(uint8_t* data, uint32_t bit)
{
  FURIOUS_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint8_t val = *data;
  uint8_t mask =  1 << bit;
  mask = ~mask;
  val = val & mask;
  bool changed = *data != val;
  *data = val;
  return changed;
}

bool
read_bit(const uint8_t* data, uint32_t bit)
{
  FURIOUS_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint32_t mask = 1 << bit;
  return (*data & mask);
}
  
} /* furious */ 
