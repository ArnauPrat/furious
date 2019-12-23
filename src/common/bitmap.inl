

#include "platform.h"
#include "memory/memory.h"

#include <string.h>

namespace furious
{


/**
 * \brief Refreshes the number of bits set
 *
 * \param bitmap The bitmap to refresh
 */
void
bitmap_refresh_num_set(bitmap_t<MAX_BITS>* bitmap);


/**
 * \brief Initializes a bitmap
 *
 * \return  The created bitmap
 */
template <int MAX_BITS>
void
bitmap_init(bitmap_t<MAX_BITS>* bitmap)
{
  memset(bitmap, 0, sizeof(bitmap_t<MAX_BITS>));
}

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */
template <int MAX_BITS>
void
bitmap_release(bitmap_t<MAX_BITS>* bitmap)
{
}

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */
template <int MAX_BITS>
void
bitmap_set(bitmap_t<MAX_BITS>* bitmap, 
           uint32_t element)
{
  FURIOUS_ASSERT(element < MAX_BITS && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_true(&bitmap->m_data[chunk], offset);
  bitmap->m_num_set += res*1;
}

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */
template <int MAX_BITS>
void
bitmap_unset(bitmap_t<MAX_BITS>* bitmap, 
             uint32_t element)
{
  FURIOUS_ASSERT(element < MAX_BITS && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  bool res = set_bit_false(&bitmap->m_data[chunk], offset);
  bitmap->m_num_set -= res*1;
}

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */
template <int MAX_BITS>
void
bitmap_set_bit(bitmap_t<MAX_BITS>* bitmap, 
               uint32_t element, 
               bool value)
{
  FURIOUS_ASSERT(element < MAX_BITS && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  if(value)
  {
    bool res = set_bit_true(&bitmap->m_data[chunk], offset);
    bitmap->m_num_set += res*1;
  } 
  else
  {
    bool res = set_bit_false(&bitmap->m_data[chunk], offset);
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
template <int MAX_BITS>
bool
bitmap_is_set(const bitmap_t<MAX_BITS>* bitmap, 
              uint32_t element)
{
  FURIOUS_ASSERT(element < MAX_BITS && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;
  return read_bit(&bitmap->m_data[chunk], offset);
}

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
template <int MAX_BITS>
void
bitmap_set_bitmap(bitmap_t<MAX_BITS>* dst_bitmap, 
                  const bitmap_t<MAX_BITS>* src_bitmap)
{
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  dst_bitmap->m_num_set = src_bitmap->m_num_set;
  memcpy(dst_bitmap->m_data, src_bitmap->m_data, sizeof(uint8_t)*num_chunks);
}

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
template <int MAX_BITS>
void
bitmap_set_and(bitmap_t<MAX_BITS>* dst_bitmap, 
               const bitmap_t<MAX_BITS>* src_bitmap)
{
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    dst_bitmap->m_data[i] = dst_bitmap->m_data[i] & src_bitmap->m_data[i];
  }
  bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
template <int MAX_BITS>
void
bitmap_set_or(bitmap_t<MAX_BITS>* dst_bitmap, 
              const bitmap_t<MAX_BITS>* src_bitmap)
{
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    dst_bitmap->m_data[i] = dst_bitmap->m_data[i] | src_bitmap->m_data[i];
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
template <int MAX_BITS>
void
bitmap_set_diff(bitmap_t<MAX_BITS>* dst_bitmap, 
                const bitmap_t<MAX_BITS>* src_bitmap)
{
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    dst_bitmap->m_data[i] = dst_bitmap->m_data[i] & (dst_bitmap->m_data[i] ^ src_bitmap->m_data[i]);
  }
  bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */
template <int MAX_BITS>
void
bitmap_negate(bitmap_t<MAX_BITS>* bitmap)
{
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    bitmap->m_data[i] = ~bitmap->m_data[i];
  }
  bitmap_refresh_num_set(bitmap);
}

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
template <int MAX_BITS>
void
bitmap_nullify(bitmap_t<MAX_BITS>* bitmap)
{
  memset(bitmap, 0, sizeof(bitmap_t<MAX_BITS>));
}

template <int MAX_BITS>
void
bitmap_refresh_num_set(bitmap_t<MAX_BITS>* bitmap)
{
  bitmap->m_num_set = 0;
  constexpr uint32_t num_chunks = (MAX_BITS + 7) / 8;
  for(uint32_t i = 0; i < num_chunks; ++i)
  {
    uint8_t left = bitmap->m_data[i] >> 4;
    uint8_t right = bitmap->m_data[i] & 0x0f;
    bitmap->m_num_set+=count_bits_lookup[left] + count_bits_lookup[right];
  }
}

} /* furious
*/ 
