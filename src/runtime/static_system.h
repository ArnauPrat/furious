
#ifndef _FURIOUS_STATIC_SYSTEM_H_
#define _FURIOUS_STATIC_SYSTEM_H_ 

#include "system.h"
#include "data/table.h"
#include "data/context.h"
#include "../common/traits.h"

#include <typeinfo>
#include <vector>
#include <bitset>

namespace furious {

/** Basic struct to hold the index sequence **/
template <std::size_t... Indices>
  struct indices {};

/** Induction case of the indices trick **/
template <std::size_t N, std::size_t... Is>
  struct build_indices
  : build_indices<N-1, N-1, Is...> {};

/** Base case of the indices trick **/
template <std::size_t... Is>
  struct build_indices<0, Is...> : indices<Is...> {};

template <size_t N>
  using indices_list = build_indices<N>;


template<typename T, typename...Components>
  class StaticSystem final : public System {
  public:

    StaticSystem(int32_t id, T* system_object); 

    ~StaticSystem();

    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////

    /**
     * @brief Applies the system to a set of aligned component blocks
     *
     * @param context The execution context
     * @param block_start The starting id of the block
     * @param component_blocks Pointers to the component blocks to apply the
     * system to
     */
    void apply_block( Context* context, 
                      const std::vector<TBlock*>& component_blocks ) override;

    /**
     * @brief Applies the system to a set of components of an entity
     *
     * @param context The execution context
     * @param id The id of the entity
     * @param components Pointers to the components of the entity 
     */
    void apply( Context* context, 
                int32_t id, 
                const std::vector<void*>& components ) override;

    /**
     * @brief Gets the name of the components of this system
     *
     * @return Returns a vector with the names of the components of this system
     */
    std::vector<SysComDescriptor> components() const override;

    /**
     * @brief Returns the name of the system;
     *
     * @return 
     */
    std::string name() const;

    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    //////////////////////////////////////////////// 

  private:

    template<std::size_t...Indices>
      void apply_block( Context* context, 
                        const std::vector<TBlock*>& components, 
                        indices<Indices...> );

    template<std::size_t...Indices>
      void apply( Context* context, 
                  int32_t id,
                  const std::vector<void*>& components, 
                  indices<Indices...> );

    void apply_block(Context* __restrict__ context, 
                     int32_t block_start, 
                     Components* __restrict__ ...components);

    void apply_block(Context* __restrict__ context, 
                     int32_t block_start, 
                     const std::bitset<TABLE_BLOCK_SIZE>& mask,
                     Components* __restrict__ ...components);

    ////////////////////////////////////////////////
    ////////////////////////////////////////////////
    ////////////////////////////////////////////////

    std::vector<SysComDescriptor>       m_types;
    T*                                  m_system_object;
  };

} /* furious */ 

#include "static_system.inl"
#endif
