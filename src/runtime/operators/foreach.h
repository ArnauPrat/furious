
#ifndef _FURIOUS_FOREACH_H_
#define _FURIOUS_FOREACH_H_

#include "../data/table_view.h"
#include <bitset>

namespace furious {

/**
 * @brief Applies the given function over all elements of the given 
 * component arrays, provided that they are not masked out by the bitset mask.
 *
 * @tparam Func The function type of the function to execute, whose parameters
 * must be of the types
 * specified in the TComponents parameters
 * @tparam TComponents The types of the component arrays to apply the function
 * to
 * @param fn The function to execute
 * @param mask The mask whose bits are set for those positions where the
 * function must be executed
 * @param args The arrays with the components
 */
template<typename Func, typename ... TComponents >
void foreach(Func&& fn, 
                   const std::bitset<TABLE_BLOCK_SIZE>& mask, 
                   TComponents* __restrict__ ... args ) {

  for (int i = 0; i < TABLE_BLOCK_SIZE ; ++i) {
    if(mask[i]) {
      fn(&args[i]...);
    }
  }

}

/**
 * @brief Applies the given function over all elements of the given 
 * component arrays.
 *
 * @tparam Func The function type of the function to execute, whose parameters
 * must be of the types
 * specified in the TComponents parameters
 * @tparam TComponents The types of the component arrays to apply the function
 * to
 * @param fn The function to execute
 * @param args The arrays with the components
 */
template<typename Func, typename ... TComponents >
void foreach(Func&& fn, 
                   TComponents* __restrict__ ... args ) {

  for (int i = 0; i < TABLE_BLOCK_SIZE ; ++i) {
      fn(&args[i]...);
  }

}


/**
 * @brief Applies the given function over all elemenets of the provided blocks. 
 *
 * @tparam Func
 * @tparam TBlocks
 * @param fn
 * @param args
 */
template<typename Func, typename ... TBlocks >
void block_foreach_join(Func&& fn, 
                        TBlocks& ... args ) {

  std::bitset<TABLE_BLOCK_SIZE> mask;
  mask.set();
  for( auto enabled : {args.get_enabled() ...} ) {
    mask &= enabled;
  }

  foreach(std::forward<Func>(fn),
          mask,
         static_cast<typename TBlocks::type*>(__builtin_assume_aligned(args.get_data(),32))...);

}


template<typename Func, typename ... TBlocks >
void block_foreach_nojoin(Func&& fn, 
                          TBlocks& ... args ) {
  foreach(std::forward<Func>(fn), 
         static_cast<typename TBlocks::type*>(__builtin_assume_aligned(args.get_data(),32))...);
}
  
} /* furious */ 

#endif 
