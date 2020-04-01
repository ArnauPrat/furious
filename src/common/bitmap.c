#include "types.h"
#include "platform.h"
#include "bitmap.h"
#include <string.h>

void
fdb_bitmap_refresh_num_set(fdb_bitmap_t* bitmap);

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
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
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
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
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
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint32_t mask = 1 << bit;
  return (*data & mask);
}


/**
 * \brief Initializes a bitmap
 *
 * \return  The initd bitmap
 */
void
fdb_bitmap_init(fdb_bitmap_t* bitmap, 
                uint32_t max_bits, 
                fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL) &&
                 "Provided allocator is ill-formed.")

  bitmap->m_max_bits = max_bits;
  bitmap->m_num_set = 0;
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(bitmap->m_max_bits);
  bitmap->p_data = (uint8_t*)mem_alloc(allocator, 
                                      FDB_BITMAP_ALIGNMENT, 
                                      sizeof(uint8_t)*nchunks, 
                                      -1);
  memset(bitmap->p_data, 0, sizeof(uint8_t)*nchunks);
}

/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */
void
fdb_bitmap_release(fdb_bitmap_t* bitmap, fdb_mem_allocator_t* allocator)
{
  FDB_ASSERT(allocator != NULL && "Allocator cannot be null");
  FDB_ASSERT((allocator->p_mem_alloc != NULL && allocator->p_mem_free != NULL) &&
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
fdb_bitmap_set(fdb_bitmap_t* bitmap, 
           uint32_t element)
{
  FDB_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
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
fdb_bitmap_unset(fdb_bitmap_t* bitmap, 
             uint32_t element)
{
  FDB_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
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
fdb_bitmap_set_bit(fdb_bitmap_t* bitmap, 
               uint32_t element, 
               bool value)
{
  FDB_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
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
fdb_bitmap_is_set(const fdb_bitmap_t* bitmap, 
              uint32_t element)
{
  FDB_ASSERT(element < bitmap->m_max_bits && "Bit out of range");
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
fdb_bitmap_set_bitmap(FDB_RESTRICT(fdb_bitmap_t*) dst_bitmap, 
                  FDB_RESTRICT(const fdb_bitmap_t*) src_bitmap)
{
  FDB_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->m_max_bits);
  dst_bitmap->m_num_set = src_bitmap->m_num_set;
  FDB_ALIGNED(FDB_RESTRICT(void*), dst_data, FDB_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  FDB_ALIGNED(FDB_RESTRICT(void*), src_data, FDB_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  memcpy(dst_data, src_data, sizeof(uint8_t)*nchunks);
}

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_and(FDB_RESTRICT(fdb_bitmap_t*) dst_bitmap, 
               FDB_RESTRICT(const fdb_bitmap_t*) src_bitmap)
{
  FDB_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->m_max_bits);
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & src_data[i];
  }
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_or(FDB_RESTRICT(fdb_bitmap_t*) dst_bitmap, 
              FDB_RESTRICT(const fdb_bitmap_t*) src_bitmap)
{
  FDB_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] | src_bitmap->p_data[i];
  }

  // This needs to be improved with a lookup table
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_bitmap_set_diff(FDB_RESTRICT(fdb_bitmap_t*) dst_bitmap, 
                FDB_RESTRICT(const fdb_bitmap_t*) src_bitmap)
{
  FDB_ASSERT(dst_bitmap->m_max_bits == src_bitmap->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] & (dst_bitmap->p_data[i] ^ src_bitmap->p_data[i]);
  }
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */
void
fdb_bitmap_negate(fdb_bitmap_t* bitmap)
{
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    bitmap->p_data[i] = ~bitmap->p_data[i];
  }
  fdb_bitmap_refresh_num_set(bitmap);
}

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
fdb_bitmap_nullify(fdb_bitmap_t* bitmap)
{
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(bitmap->m_max_bits);
  bitmap->m_num_set = 0;
  memset(bitmap->p_data, 0, sizeof(uint8_t)*nchunks);
}

void
fdb_bitmap_refresh_num_set(fdb_bitmap_t* bitmap)
{
  bitmap->m_num_set = 0;
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(bitmap->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    uint8_t left = bitmap->p_data[i] >> 4;
    uint8_t right = bitmap->p_data[i] & 0x0f;
    bitmap->m_num_set+=count_bits_lookup[left] + count_bits_lookup[right];
  }
}
