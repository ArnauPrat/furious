
#ifndef _FURIOUS_STATIC_SYSTEM_H_
#define _FURIOUS_STATIC_SYSTEM_H_ 

#include "data/table.h"
#include "data/context.h"
#include "../common/traits.h"

#include <bitset>

namespace furious {

template<typename T, typename...Components>
class SystemWrapper final 
{
public:

  SystemWrapper(T* system_object); 
  ~SystemWrapper();

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  /**
   * @brief Applies the system to a set of components of an entity
   *
   * @param context The execution context
   * @param id The id of the entity
   * @param mask The bitset indicating the elements of the block to apply the
   * system to
   * @param components Pointers to the components of the entity 
   */

  void apply_block(Context* __restrict__ context, 
                   int32_t block_start, 
                   const std::bitset<TABLE_BLOCK_SIZE>& mask,
                   Components* __restrict__ ...components);

  T* p_system_object;
};

} /* furious */ 

#include "system_wrapper.inl"
#endif
