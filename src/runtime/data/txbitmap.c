#include "txbitmap.h"
#include <string.h>



void
fdb_txbitmap_refresh_num_set(struct fdb_txbitmap_t* bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx);

// Lookup table used to efficiently implement refresh_num_set operation
static 
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
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
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
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint8_t val = *data;
  uint8_t mask =  1 << bit;
  mask = ~mask;
  val = val & mask;
  bool changed = *data != val;
  *data = val;
  return changed;
}

static bool
read_bit(const uint8_t* data, uint32_t bit)
{
  FDB_ASSERT(bit >= 0 && bit <= 7 && "Cannot set bit. Bit is out of range");
  uint32_t mask = 1 << bit;
  return (*data & mask);
}

void
fdb_txbitmap_factory_init(struct fdb_txbitmap_factory_t* txbitmap_factory, 
                          struct fdb_mem_allocator_t* allocator, 
                          uint32_t max_bits)
{
  *txbitmap_factory = (struct fdb_txbitmap_factory_t){.m_max_bits = max_bits};
  txbitmap_factory->p_txbitmap_alloc =  fdb_txpool_alloc_create(FDB_TXBITMAP_ALIGNMENT, 
                                                                sizeof(struct fdb_txbitmap_impl_t),
                                                                FDB_TXBITMAP_PAGE_SIZE, 
                                                                allocator);

  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(txbitmap_factory->m_max_bits);
  txbitmap_factory->p_data_alloc = fdb_txpool_alloc_create(FDB_TXBITMAP_DATA_ALIGNMENT, 
                                                         sizeof(uint8_t)*nchunks,
                                                         FDB_TXBITMAP_DATA_PAGE_SIZE, 
                                                         allocator);

}

/**
 * \brief Releases a bitmap factory
 *
 * \param txbitmap_factory The bitmap factory to release
 */
void
fdb_txbitmap_factory_release(struct fdb_txbitmap_factory_t* txbitmap_factory, 
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx)
{
  fdb_txpool_alloc_destroy(txbitmap_factory->p_txbitmap_alloc, 
                           tx, 
                           txtctx);
  fdb_txpool_alloc_destroy(txbitmap_factory->p_data_alloc, 
                           tx, txtctx);
}


/**
 * \brief Initializes a bitmap
 *
 * \return  The initd bitmap
 */
void
fdb_txbitmap_init(struct fdb_txbitmap_t* bitmap, 
                  struct fdb_txbitmap_factory_t* bitmap_factory, 
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx)
{
  *bitmap = (struct fdb_txbitmap_t){.p_factory = bitmap_factory};
  bitmap->m_bitmap_ref = fdb_txpool_alloc_alloc(bitmap_factory->p_txbitmap_alloc, 
                                                tx, 
                                                txtctx,
                                                FDB_TXBITMAP_ALIGNMENT, 
                                                sizeof(struct fdb_txbitmap_impl_t), 
                                                FDB_NO_HINT);

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          bitmap->m_bitmap_ref, 
                                                          true);
  bitmap_impl->m_num_set = 0;
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(bitmap->p_factory->m_max_bits);
  bitmap_impl->m_data_ref = fdb_txpool_alloc_alloc(bitmap->p_factory->p_data_alloc, 
                                                   tx, 
                                                   txtctx,
                                                   FDB_TXBITMAP_DATA_ALIGNMENT, 
                                                   sizeof(uint8_t)*nchunks, 
                                                   FDB_NO_HINT);

  void* pdata = fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                     tx, 
                                     txtctx, 
                                     bitmap_impl->m_data_ref, 
                                     true);
  memset(pdata, 0, sizeof(uint8_t)*nchunks);
}


/**
 * \brief  Releases a bitmap
 *
 * \param bitmap
 */
void
fdb_txbitmap_release(struct fdb_txbitmap_t* bitmap,
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx)
{
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          bitmap->m_bitmap_ref, 
                                                          true);
  fdb_txpool_alloc_free(bitmap->p_factory->p_data_alloc, 
                        tx, 
                        txtctx, 
                        bitmap_impl->m_data_ref);

  fdb_txpool_alloc_free(bitmap->p_factory->p_txbitmap_alloc, 
                        tx, 
                        txtctx, 
                        bitmap->m_bitmap_ref);
  bitmap->m_bitmap_ref = (struct fdb_txpool_alloc_ref_t){};
}

/**
 * \brief Sets to one the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set
 */
void
fdb_txbitmap_set(struct fdb_txbitmap_t* bitmap, 
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx,
                 uint32_t element)
{
  FDB_ASSERT(element < bitmap->p_factory->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          bitmap->m_bitmap_ref, 
                                                          true);

  uint8_t* pdata = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                  tx, 
                                                  txtctx, 
                                                  bitmap_impl->m_data_ref, 
                                                  true);
  bool res = set_bit_true(&pdata[chunk], offset);
  bitmap_impl->m_num_set += res*1;
}

/**
 * \brief Sets to 0 the ith element of the bitmap
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The ith element to set
 */
void
fdb_txbitmap_unset(struct fdb_txbitmap_t* bitmap, 
                 struct fdb_tx_t* tx, 
                 struct fdb_txthread_ctx_t* txtctx,
                 uint32_t element)
{
  FDB_ASSERT(element < bitmap->p_factory->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);
  uint8_t* pdata = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                  tx, 
                                                  txtctx, 
                                                  bitmap_impl->m_data_ref, 
                                                  true);
  bool res = set_bit_false(&pdata[chunk], offset);
  bitmap_impl->m_num_set -= res*1;
}

/**
 * \brief Sets the bit of the given element to the specified value
 *
 * \param bitmap The bitmap to perform the operation
 * \param element The element to set the bit for
 * \param value The value to set the bit to
 */
void
fdb_txbitmap_set_bit(struct fdb_txbitmap_t* bitmap, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx,
                   uint32_t element, 
                   bool value)
{
  FDB_ASSERT(element < bitmap->p_factory->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);
  uint8_t* pdata = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                  tx, 
                                                  txtctx, 
                                                  bitmap_impl->m_data_ref, 
                                                  true);
  if(value)
  {
    bool res = set_bit_true(&pdata[chunk], offset);
    bitmap_impl->m_num_set += res*1;
  } 
  else
  {
    bool res = set_bit_false(&pdata[chunk], offset);
    bitmap_impl->m_num_set -= res*1;
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
fdb_txbitmap_is_set(struct fdb_txbitmap_t* bitmap, 
                  struct fdb_tx_t* tx, 
                  struct fdb_txthread_ctx_t* txtctx, 
                  uint32_t element)
{
  FDB_ASSERT(element < bitmap->p_factory->m_max_bits && "Bit out of range");
  uint32_t chunk = element / 8;
  uint32_t offset = element % 8;

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);

  uint8_t* pdata = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                  tx, 
                                                  txtctx, 
                                                  bitmap_impl->m_data_ref, 
                                                  true);

  return read_bit(&pdata[chunk], offset);
}

/**
 * \brief Sets the bits based on the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_txbitmap(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                          FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                          struct fdb_tx_t* tx, 
                          struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);

  struct fdb_txbitmap_impl_t* dst_bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     dst_bitmap->m_bitmap_ref, 
                                                                     true);

  FDB_ALIGNED(FDB_RESTRICT(void*), dst_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    dst_bitmap_impl->m_data_ref, 
                                                                                                    true);

  struct fdb_txbitmap_impl_t* src_bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     src_bitmap->m_bitmap_ref, 
                                                                     false);

  FDB_ALIGNED(FDB_RESTRICT(void*), src_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    src_bitmap_impl->m_data_ref, 
                                                                                                    false);


  dst_bitmap_impl->m_num_set = src_bitmap_impl->m_num_set;
  memcpy(dst_data, src_data, sizeof(uint8_t)*nchunks);
}

/**
 * \brief Ands this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_and(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                     FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap,
                     struct fdb_tx_t* tx, 
                     struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);

  struct fdb_txbitmap_impl_t* dst_bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     dst_bitmap->m_bitmap_ref, 
                                                                     true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                                                                                       tx, 
                                                                                                       txtctx, 
                                                                                                       dst_bitmap_impl->m_data_ref, 
                                                                                                       true);

  struct fdb_txbitmap_impl_t* src_bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     src_bitmap->m_bitmap_ref, 
                                                                     false);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                                                                                       tx, 
                                                                                                       txtctx, 
                                                                                                       src_bitmap_impl->m_data_ref, 
                                                                                                       false);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & src_data[i];
  }
  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

/**
 * \brief Ors this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_or(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                    FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap,
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  struct fdb_txbitmap_impl_t* dst_bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     dst_bitmap->m_bitmap_ref, 
                                                                     true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                                                                                       tx, 
                                                                                                       txtctx, 
                                                                                                       dst_bitmap_impl->m_data_ref, 
                                                                                                       true);

  struct fdb_txbitmap_impl_t* src_bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     src_bitmap->m_bitmap_ref, 
                                                                     false);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    src_bitmap_impl->m_data_ref, 
                                                                                                    false);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] | src_data[i];
  }

  // This needs to be improved with a lookup table
  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

/**
 * \brief Diffs this bitmap with the given bitmap
 *
 * \param dst_bitmap The bitmap to set the bits to 
 * \param src_bitmap The bitmap to set the bits from
 */
void
fdb_txbitmap_set_diff(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                      FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap,
                      struct fdb_tx_t* tx, 
                      struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  struct fdb_txbitmap_impl_t* dst_bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                              tx, 
                                                              txtctx, 
                                                              dst_bitmap->m_bitmap_ref, 
                                                              true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                                  tx, 
                                                  txtctx, 
                                                  dst_bitmap_impl->m_data_ref, 
                                                  true);

  struct fdb_txbitmap_impl_t* src_bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                                     tx, 
                                                                     txtctx, 
                                                                     src_bitmap->m_bitmap_ref, 
                                                                     false);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                                                                                       tx, 
                                                                                                       txtctx, 
                                                                                                       src_bitmap_impl->m_data_ref, 
                                                                                                       false);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & (dst_data[i] ^ src_data[i]);
  }
  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

/**
 * \brief Negates the contents of this bitmap
 *
 * \param bitmap The bitmap to set the bits to 
 */
void
fdb_txbitmap_negate(struct fdb_txbitmap_t* bitmap,
                    struct fdb_tx_t* tx, 
                    struct fdb_txthread_ctx_t* txtctx)
{
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(bitmap->p_factory->m_max_bits);

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), pdata, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    bitmap_impl->m_data_ref, 
                                                                                                    true);

  for(uint32_t i = 0; i < nchunks; ++i)
  {
    pdata[i] = ~pdata[i];
  }
  fdb_txbitmap_refresh_num_set(bitmap, 
                               tx, 
                               txtctx);
}

/**
 * \brief Sets the bitmap to all zeros
 *
 * \param bitmap The bitmap to nullify
 */
void
fdb_txbitmap_nullify(struct fdb_txbitmap_t* bitmap,
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx)
{
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(bitmap->p_factory->m_max_bits);
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), pdata, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    bitmap_impl->m_data_ref, 
                                                                                                    true);
  bitmap_impl->m_num_set = 0;
  memset(pdata, 0, sizeof(uint8_t)*nchunks);
}

void
fdb_txbitmap_refresh_num_set(struct fdb_txbitmap_t* bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx)
{

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 bitmap->m_bitmap_ref, 
                                                                 true);

  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), pdata, FDB_BITMAP_ALIGNMENT) = (uint8_t*)fdb_txpool_alloc_ptr(bitmap->p_factory->p_data_alloc, 
                                                                                                    tx, 
                                                                                                    txtctx, 
                                                                                                    bitmap_impl->m_data_ref, 
                                                                                                    false);

  bitmap_impl->m_num_set = 0;
  uint32_t nchunks = FDB_TXBITMAP_NUM_CHUNKS(bitmap->p_factory->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    uint8_t left = pdata[i] >> 4;
    uint8_t right = pdata[i] & 0x0f;
    bitmap_impl->m_num_set+=count_bits_lookup[left] + count_bits_lookup[right];
  }
}
