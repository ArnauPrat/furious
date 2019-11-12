

#include "platform.h"
#include "bitmap.h"
#include <string.h>

namespace furious
{

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

static bool 
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

static bool 
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

static bool
read_bit(uint8_t* data, uint32_t bit)
{
  FURIOUS_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint32_t mask = 1 << bit;
  return (*data & mask);
}

Bitmap::Bitmap(uint32_t size) :
m_max_bits(size),
m_num_chunks((m_max_bits + 7) / 8),
m_num_set(0)
{
  uint32_t buffer_size = (m_max_bits + 7) / 8;
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
  FURIOUS_ASSERT(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_true(&p_data[chunk], offset);
  m_num_set += res*1;
}

void
Bitmap::unset(uint32_t element)
{
  FURIOUS_ASSERT(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_false(&p_data[chunk], offset);
  m_num_set -= res*1;
}

void
Bitmap::set_bit(uint32_t element, bool value)
{
  FURIOUS_ASSERT(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  if(value)
  {
    bool res = set_bit_true(&p_data[chunk], offset);
    m_num_set += res*1;
  } 
  else
  {
    bool res = set_bit_false(&p_data[chunk], offset);
    m_num_set -= res*1;
  }
}

bool
Bitmap::is_set(uint32_t element) const
{
  FURIOUS_ASSERT(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  return read_bit(&p_data[chunk], offset);
}

uint32_t
Bitmap::num_set() const
{
  return m_num_set;
}

uint32_t
Bitmap::max_bits() const
{
  return m_max_bits;
}

void
Bitmap::set_bitmap(const Bitmap* bitmap)
{
  FURIOUS_ASSERT(this->m_max_bits == bitmap->m_max_bits && "Cannot set from a bitmap of a different size");
  m_max_bits = bitmap->m_max_bits;
  m_num_set = bitmap->m_num_set;
  memcpy(p_data, bitmap->p_data, sizeof(uint8_t)*m_num_chunks);
}

void
Bitmap::set_and(const Bitmap* bitmap)
{
  FURIOUS_ASSERT(this->m_max_bits == bitmap->m_max_bits && "Cannot AND two bitmaps of a different sizes");
  for(uint32_t i = 0; i < m_num_chunks; ++i)
  {
    p_data[i] = p_data[i] & bitmap->p_data[i];
  }

  refresh_num_set();
}

void
Bitmap::set_or(const Bitmap* bitmap)
{
  FURIOUS_ASSERT(this->m_max_bits == bitmap->m_max_bits && "Cannot AND two bitmaps of a different sizes");
  for(uint32_t i = 0; i < m_num_chunks; ++i)
  {
    p_data[i] = p_data[i] | bitmap->p_data[i];
  }

  // This needs to be improved with a lookup table
  refresh_num_set();
}

void
Bitmap::set_diff(const Bitmap* bitmap)
{
  FURIOUS_ASSERT(this->m_max_bits == bitmap->m_max_bits && "Cannot AND two bitmaps of a different sizes");
  for(uint32_t i = 0; i < m_num_chunks; ++i)
  {
    p_data[i] = p_data[i] & (p_data[i] ^ bitmap->p_data[i]);
  }
  refresh_num_set();
}

void
Bitmap::set_negate()
{
  for(uint32_t i = 0; i < m_num_chunks; ++i)
  {
    p_data[i] = ~p_data[i];
  }
  refresh_num_set();
}

void
Bitmap::all_zeros()
{
  memset(p_data,0,m_num_chunks*sizeof(uint8_t));
}

void
Bitmap::refresh_num_set()
{
  m_num_set = 0;
  for(uint32_t i = 0; i < m_num_chunks; ++i)
  {
    uint8_t left = p_data[i] >> 4;
    uint8_t right = p_data[i] & 0x0f;
    m_num_set+=count_bits_lookup[left] + count_bits_lookup[right];
  }
  if(m_max_bits % 8 != 0)
  {
    uint8_t left = p_data[m_num_chunks-1] >> 4;
    uint8_t right = p_data[m_num_chunks-1] & 0x0f;
    m_num_set-=count_bits_lookup[left] + count_bits_lookup[right];
    for(uint32_t i = (m_num_chunks-1)*8; i < m_max_bits; ++i)
    {
      if(is_set(i))
      {
        m_num_set+=1;
      }
    }
  }
}

} /* furious
*/ 
