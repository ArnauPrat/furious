#include "txbitmap_utils.h"
#include "string.h"

void fdb_bitmap_refresh_num_set(struct fdb_bitmap_t* bitmap);
void fdb_txbitmap_refresh_num_set(struct fdb_txbitmap_t* bitmap, 
                                  struct fdb_tx_t* tx, 
                                  struct fdb_txthread_ctx_t* txtctx);

void
fdb_bitmap_set_txbitmap(FDB_RESTRICT(struct fdb_bitmap_t*) dst_bitmap, 
                        FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          src_bitmap->m_bitmap_ref, 
                                                          false);
  dst_bitmap->m_num_set = bitmap_impl->m_num_set;
  FDB_ALIGNED(FDB_RESTRICT(void*), dst_data, FDB_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  void* data = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    false);
  memcpy(dst_data, data, sizeof(uint8_t)*nchunks);
}

void
fdb_txbitmap_set_bitmap(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                        FDB_RESTRICT(struct fdb_bitmap_t*) src_bitmap, 
                        struct fdb_tx_t* tx, 
                        struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 dst_bitmap->m_bitmap_ref, 
                                                                 false);
  FDB_ALIGNED(FDB_RESTRICT(void*), src_data, FDB_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  void* data = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    true);
  memcpy(data, src_data, sizeof(uint8_t)*nchunks);

  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

void
fdb_bitmap_set_txbitmap_and(FDB_RESTRICT(struct fdb_bitmap_t*) dst_bitmap, 
                   FDB_RESTRICT(struct fdb_txbitmap_t*) src_bitmap, 
                   struct fdb_tx_t* tx, 
                   struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = dst_bitmap->p_data;
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 src_bitmap->m_bitmap_ref, 
                                                                 false);
  void* data = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    false);
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = data;
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & src_data[i];
  }
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

void
fdb_txbitmap_set_bitmap_and(FDB_RESTRICT(struct fdb_txbitmap_t*) dst_bitmap, 
                            FDB_RESTRICT(struct fdb_bitmap_t*) src_bitmap, 
                            struct fdb_tx_t* tx, 
                            struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), src_data, FDB_BITMAP_ALIGNMENT) = src_bitmap->p_data;
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                                 tx, 
                                                                 txtctx, 
                                                                 dst_bitmap->m_bitmap_ref, 
                                                                 false);
  void* data = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    true);
  FDB_ALIGNED(FDB_RESTRICT(uint8_t*), dst_data, FDB_BITMAP_ALIGNMENT) = data;
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_data[i] = dst_data[i] & src_data[i];
  }

  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

void
fdb_bitmap_set_txbitmap_or(struct fdb_bitmap_t* dst_bitmap, 
                           struct fdb_txbitmap_t* src_bitmap, 
                           struct fdb_tx_t* tx,
                           struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          src_bitmap->m_bitmap_ref, 
                                                          false);
  uint8_t* data = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    false);
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] | data[i];
  }

  // This needs to be improved with a lookup table
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

void
fdb_txbitmap_set_bitmap_or(struct fdb_txbitmap_t* dst_bitmap, 
                           struct fdb_bitmap_t* src_bitmap, 
                           struct fdb_tx_t* tx,
                           struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");

  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          dst_bitmap->m_bitmap_ref, 
                                                          false);
  uint8_t* data = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    true);
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    data[i] = data[i] | src_bitmap->p_data[i];
  }
  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}

void
fdb_bitmap_set_txbitmap_diff(struct fdb_bitmap_t* dst_bitmap, 
                             struct fdb_txbitmap_t* src_bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          src_bitmap->m_bitmap_ref, 
                                                          false);
  uint8_t* data = (uint8_t*)fdb_txpool_alloc_ptr(src_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    false);
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    dst_bitmap->p_data[i] = dst_bitmap->p_data[i] & (dst_bitmap->p_data[i] ^ data[i]);
  }
  fdb_bitmap_refresh_num_set(dst_bitmap);
}

void
fdb_txbitmap_set_bitmap_diff(struct fdb_txbitmap_t* dst_bitmap, 
                             struct fdb_bitmap_t* src_bitmap,
                             struct fdb_tx_t* tx, 
                             struct fdb_txthread_ctx_t* txtctx)
{
  FDB_ASSERT(dst_bitmap->p_factory->m_max_bits == src_bitmap->p_factory->m_max_bits && "Incompatible bitmaps");
  struct fdb_txbitmap_impl_t* bitmap_impl = fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_txbitmap_alloc, 
                                                          tx, 
                                                          txtctx, 
                                                          dst_bitmap->m_bitmap_ref, 
                                                          false);
  uint8_t* data = (uint8_t*)fdb_txpool_alloc_ptr(dst_bitmap->p_factory->p_data_alloc, 
                                    tx, 
                                    txtctx, 
                                    bitmap_impl->m_data_ref, 
                                    true);
  uint32_t nchunks = FDB_BITMAP_NUM_CHUNKS(dst_bitmap->p_factory->m_max_bits);
  for(uint32_t i = 0; i < nchunks; ++i)
  {
    data[i] = data[i] & (data[i] ^ src_bitmap->p_data[i]);
  }
  fdb_txbitmap_refresh_num_set(dst_bitmap, 
                               tx, 
                               txtctx);
}
