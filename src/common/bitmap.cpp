

#include "bitmap.h"
#include <string.h>
#include <assert.h>

namespace furious
{

static bool 
set_bit(uint8_t* data, uint32_t bit)
{
  assert(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint8_t val = *data;
  uint8_t mask =  1 << bit;
  val = val | mask;
  bool changed = *data != val;
  *data = val;
  return changed;
}

static bool 
unset_bit(uint8_t* data, uint32_t bit)
{
  assert(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint8_t val = *data;
  uint8_t mask =  1 << bit;
  mask = ~mask;
  val = val & mask;
  bool changed = *data != val;
  *data = val;
  return changed;
}

Bitmap::Bitmap(uint32_t size) :
m_max_bits(size),
m_num_set(0)
{
  uint32_t buffer_size = 0;
  buffer_size = (size + 7)/ 8;
  p_data = new uint8_t[buffer_size];
  memset(p_data, 0, sizeof(uint8_t)*buffer_size);
}

Bitmap::~Bitmap()
{
  delete [] p_data;
}


void
Bitmap::set(uint32_t element)
{
  assert(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit(&p_data[chunk], offset);
  m_num_set += res*1;
}

void
Bitmap::unset(uint32_t element)
{
  assert(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = unset_bit(&p_data[chunk], offset);
  m_num_set -= res*1;
}

bool
Bitmap::is_set(uint32_t element) const
{
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  uint32_t mask = 1 << offset;
  return ((p_data[chunk] & mask) != 0);
}

uint32_t
Bitmap::num_set() const
{
  return m_num_set;
}

  
} /* furious
 */ 
