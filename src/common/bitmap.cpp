

#include "bitmap.h"
#include <string.h>
#include <assert.h>

namespace furious
{

static bool 
set_bit_true(uint8_t* data, uint32_t bit)
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
set_bit_false(uint8_t* data, uint32_t bit)
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

static bool
read_bit(uint8_t* data, uint32_t bit)
{
  assert(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint32_t mask = 1 << bit;
  return ((*data & mask) != 0);
}

Bitmap::Bitmap(uint32_t size) :
m_max_bits(size),
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
  assert(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_true(&p_data[chunk], offset);
  m_num_set += res*1;
}

void
Bitmap::unset(uint32_t element)
{
  assert(element < m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_false(&p_data[chunk], offset);
  m_num_set -= res*1;
}

void
Bitmap::set_bit(uint32_t element, bool value)
{
  assert(element < m_max_bits && "Bit out of range");
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
  assert(element < m_max_bits && "Bit out of range");
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
  assert(this->m_max_bits == bitmap->m_max_bits && "Cannot set from a bitmap of a different size");
  m_max_bits = bitmap->m_max_bits;
  m_num_set = bitmap->m_num_set;
  uint32_t num_chunks = (m_max_bits + 7) / 8;
  memcpy(p_data, bitmap->p_data, sizeof(uint8_t)*num_chunks);
}

void
Bitmap::set_and(const Bitmap* bitmap)
{
  assert(this->m_max_bits == bitmap->m_max_bits && "Cannot AND two bitmaps of a different sizes");
  uint32_t num_chunks = (m_max_bits + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    p_data[i] = p_data[i] & bitmap->p_data[i];
  }

  // This needs to be improved with a lookup table
  m_num_set = 0;
  for(uint32_t i = 0; i < m_max_bits; ++i)
  {
    if(is_set(i))
    {
      m_num_set++;
    }
  }
}

void
Bitmap::set_or(const Bitmap* bitmap)
{
  assert(this->m_max_bits == bitmap->m_max_bits && "Cannot AND two bitmaps of a different sizes");
  uint32_t num_chunks = (m_max_bits + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    p_data[i] = p_data[i] | bitmap->p_data[i];
  }

  // This needs to be improved with a lookup table
  m_num_set = 0;
  for(uint32_t i = 0; i < m_max_bits; ++i)
  {
    if(is_set(i))
    {
      m_num_set++;
    }
  }
}

void
Bitmap::set_negate()
{
  uint32_t num_chunks = (m_max_bits + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    p_data[i] = ~p_data[i];
  }

  // This needs to be improved with a lookup table
  m_num_set = 0;
  for(uint32_t i = 0; i < m_max_bits; ++i)
  {
    if(is_set(i))
    {
      m_num_set++;
    }
  }
}

void
Bitmap::all_zeros()
{
  uint32_t num_chunks = (m_max_bits + 7) / 8;
  memset(p_data,0,num_chunks*sizeof(uint8_t));
}


} /* furious
*/ 
