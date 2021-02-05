


#ifndef _FURIOUS_STACK_ALLOCATOR_H_
#define _FURIOUS_STACK_ALLOCATOR_H_ value

#include "../../common/types.h"
#include "memory.h"
#include "../mutex.h"
#include "pool_allocator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct fdb_stack_alloc_fheader_t
{
  void* p_next_page;
  void* p_previous_page;
  uint32_t m_next_free;
};

struct fdb_stack_alloc_t
{
  uint32_t                   m_page_size;
  uint64_t                   m_page_mask;
  struct fdb_mem_allocator_t*       p_allocator;
  void*                      p_next_page;
  uint32_t                   m_next_offset;
  void*                      p_last_frame;
  uint32_t                   m_max_frame_offset; 
  struct fdb_mem_allocator_t        m_super;
  struct fdb_mem_stats_t            m_stats;
  struct fdb_mutex_t                m_mutex;
};

/**
 * \brief inits a stack allocator
 *
 * \param allocator The allocator to use 
 *
 * \return Returns a new stack allocator
 */
void
fdb_stack_alloc_init(struct fdb_stack_alloc_t* salloc, 
                     uint32_t page_size, 
                     struct fdb_mem_allocator_t* allocator);

/**
 * \brief releases a stack allocator
 *
 * \param fdb_stack_alloc The stack allocator to release
 */
void
fdb_stack_alloc_release(struct fdb_stack_alloc_t* allocator);

void* 
fdb_stack_alloc_alloc(struct fdb_stack_alloc_t* state, 
                      uint32_t alignment, 
                      uint32_t size,
                      uint32_t hint);
void
fdb_stack_alloc_free(struct fdb_stack_alloc_t* state, 
                     void* ptr);

/**
 * \brief Frees the last allocation performed by the stack allocator
 *
 * \param state The stack allocator
 * \param ptr The ptr to the data to be freed. This is used for error checking
 *  purposes
 */
void
fdb_stack_alloc_pop(struct fdb_stack_alloc_t* state, 
                    void* ptr);

struct fdb_mem_stats_t
fdb_stack_alloc_stats(struct fdb_stack_alloc_t* palloc);

#ifdef __cplusplus
}
#endif
  
#endif /* ifndef _FURIOUS_STACK_ALLOCATOR_H_ */
