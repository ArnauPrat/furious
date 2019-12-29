#include "types.h"
#include "platform.h"
#include "bitmap.h"
#include <string.h>

namespace furious
{

void
bitmap_refresh_num_set(bitmap_t* bitmap);

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

uint32_t
num_chunks(uint32_t max_bits)
{
  return (max_bits + 7) / 8;
}


/**
 * \brief Initializes a bitmap
 *
 * \return  The created bitmap
 */
bitmap_t
bitmap_create(uint32_t max_bits, mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")

  bitmap_t bitmap;
  bitmap.m_max_bits = max_bits;
  bitmap.m_num_set = 0;
  uint32_t nchunks = num_chunks(bitmap.m_max_bits);
  bitmap.p_data = (uint8_t*)mem_alloc(allocator, 
                                      FURIOUS_BITMAP_ALIGNMENT, 
                                      sizeof(uint8_t)*nchunks, 
                                      -1);
  memset(bitmap.p_data, 0, sizeof(uint8_t)*nchunks);
  return bitmap;
}

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */
void
bitmap_destroy(bitmap_t* bitmap, mem_allocator_t* allocator)
{
  FURIOUS_ASSERT(allocator != nullptr && "Allocator cannot be null");
  FURIOUS_ASSERT((allocator->p_mem_alloc != nullptr && allocator->p_mem_free != nullptr) &&
                 "Provided allocator is ill-formed.")
  mem_free(allocator, bitmap->p_data);
}

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */
void
bitmap_set(bitmap_t* bitmap, 
           uint32_t element)
{
  FURIOUS_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_true(&bitmap->p_data[chunk], offset);
  bitmap->m_num_set += res*1;
}

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */
void
bitmap_unset(bitmap_t* bitmap, 
             uint32_t element)
{
  FURIOUS_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_false(&bitmap->p_data[chunk], offset);
  bitmap->m_num_set -= res*1;
}

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */
void
bitmap_set_bit(bitmap_t* bitmap, 
               uint32_t element, 
               bool value)
{
  FURIOUS_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  if(value)
  {
    bool res = set_bit_true(&bitmap->p_data[chunk], offset);
    bitmap->m_num_set += res*1;
  } 
  else
  {
    bool res = set_bit_false(&bitmap->p_data[chunk], offset);
    bitmap->m_num_set -= res*1;
  }
}

/**
 * \brief Checks if a given element is set to 1
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to check
 *
 * \return True if the element is set to 1
 */
bool
bitmap_is_set(const bitmap_t* bitmap, 
              uint32_t element)
{
  FURIOUS_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  return read_bit(&bitmap->p_data[chunk], offset);
}

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
bitmap_set_bitmap(FURIOUS_RESTRICT(bitmap_t*) dst_bitmap, 
                  FURIOUS_RESTRICT(const bitmap_t*) src_bitmap)
{
  FURIOUS_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = num_chunks(dst_bitmap->m_max_bits);
  dst_bitmap->m_num_set = src_bitmap->m_num_set;
  FURIOUS_ALIGNED(FURIOUS_RESTRICT(void*), dst_data, FURIOUS_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  FURIOUS_ALIGNED(FURIOUS_RESTRICT(void*), src_data, FURIOUS_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  memcpy(dst_data, src_data, sizeof(uint8_t)*nchunks);
}

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
bitmap_set_and(FURIOUS_RESTRICT(bitmap_t*) dst_bitmap, 
               FURIOUS_RESTRICT(const bitmap_t*) src_bitmap)
{
  FURIOUS_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = num_chunks(dst_bitmap->m_max_bits);
  FURIOUS_ALIGNED(FURIOUS_RESTRICT(uint8_t*), dst_data, FURIOUS_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  FURIOUS_ALIGNED(FURIOUS_RESTRICT(uint8_t*), src_data, FURIOUS_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & src_data[i];
  }
  bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
bitmap_set_or(FURIOUS_RESTRICT(bitmap_t*) dst_bitmap, 
              FURIOUS_RESTRICT(const bitmap_t*) src_bitmap)
{
  FURIOUS_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = num_chunks(dst_bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] | src_bitmap->p_data[i];
  }

  // This needs to be improved with a lookup table
  bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
bitmap_set_diff(FURIOUS_RESTRICT(bitmap_t*) dst_bitmap, 
                FURIOUS_RESTRICT(const bitmap_t*) src_bitmap)
{
  FURIOUS_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = num_chunks(dst_bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] & (dst_bitmap->p_data[i] ^ src_bitmap->p_data[i]);
  }
  bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */
void
bitmap_negate(bitmap_t* bitmap)
{
  uint32_t nchunks = num_chunks(bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    bitmap->p_data[i] = ~bitmap->p_data[i];
  }
  bitmap_refresh_num_set(bitmap);
}

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
bitmap_nullify(bitmap_t* bitmap)
{
  uint32_t nchunks = num_chunks(bitmap->m_max_bits);
  bitmap->m_num_set = 0;
  memset(bitmap->p_data, 0, sizeof(uint8_t)*nchunks);
}

void
bitmap_refresh_num_set(bitmap_t* bitmap)
{
  bitmap->m_num_set = 0;
  uint32_t nchunks = num_chunks(bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    uint8_t left = bitmap->p_data[i] >> 4;
    uint8_t right = bitmap->p_data[i] & 0x0f;
    bitmap->m_num_set+=count_bits_lookup[left] + count_bits_lookup[right];
  }
}
  
} /* furious */ 
