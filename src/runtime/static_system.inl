
#include <utility>

namespace furious {


template<typename T, typename...Components>
StaticSystem<T,Components...>::StaticSystem(T* system_object) : System(),
  m_types{SysComDescriptor{type_name<Components>::name(), access_type<Components>::type()}...},
  m_system_object(system_object)
  {

  }

template<typename T, typename...Components>
StaticSystem<T,Components...>::~StaticSystem() {
  delete m_system_object;
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply_block( Context* context, 
                                                 const std::vector<TBlock*>& component_blocks ) {
  apply_block( context, component_blocks, indices_list<sizeof...(Components)>());
}

template<typename T, typename...Components>
template<std::size_t...Indices>
void StaticSystem<T,Components...>::apply_block( Context* context, 
                                                 const std::vector<TBlock*>& component_blocks, 
                                                 indices<Indices...> ) {
  bitset result;
  result.resize(TABLE_BLOCK_SIZE, true);
  for(auto block : component_blocks) {
    result &= block->m_enabled;
  }

  int32_t start = component_blocks[0]->m_start;
  if(result.count() == TABLE_BLOCK_SIZE) {
    apply_block(context, 
                start, 
                caster<Components>::cast(__builtin_assume_aligned(component_blocks[Indices]->p_data,32))...);
  } else {
    apply_block(context, 
                start, 
                result, 
                caster<Components>::cast(__builtin_assume_aligned(component_blocks[Indices]->p_data,32))...);
  }
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply_block(Context* context, 
                                                int32_t block_start,
                                                Components* __restrict__ ...components) {
  for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i) {
    m_system_object->run(context, block_start+i, &components[i]...);
  }
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply_block(Context* context, 
                                                int32_t block_start,
                                                const bitset& mask,
                                                Components* __restrict__ ...components) {
  for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i) {
    if(mask[i]) {
      m_system_object->run(context, block_start+i, &components[i]...);
    }
  }
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply( Context* context, 
                                           int32_t id, 
                                           const std::vector<void*>& components ) {
  apply(context, id, components,indices_list<sizeof...(Components)>());
}

template<typename T, typename...Components>
template<std::size_t...Indices>
void StaticSystem<T,Components...>::apply( Context* context, 
                                           int32_t id, 
                                           const std::vector<void*>& components, 
                                           indices<Indices...> ) {
  m_system_object->run(context, id, (caster<Components>::cast(components[Indices]))...);
}

template<typename T, typename...Components>
std::vector<SysComDescriptor> StaticSystem<T,Components...>::components() const {
  return m_types;
}

template<typename T, typename...Components> 
StaticSystem<T, Components...>* create_static_system(T* system, 
                                                     void (T::*)(Context*, int32_t id, Components*...)  ) {
  return new StaticSystem<T,Components...>(system);
}

template<typename TSystem, typename...TArgs>
auto create_static_system(TArgs&&...args) {
  TSystem* system_object = new TSystem(std::forward<TArgs>(args)...);
  return create_static_system(system_object, &TSystem::run );
}
  
} /* furious */ 
