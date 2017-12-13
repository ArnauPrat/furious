
#include <utility>
#include <type_traits>

namespace furious {


template<typename T>
  typename std::enable_if<!std::is_const<T>::value,ComAccessType>::type access_type() {
    return ComAccessType::E_WRITE;
  }

template<typename T>
  typename std::enable_if<std::is_const<T>::value,ComAccessType>::type access_type() {
    return ComAccessType::E_READ;
  }

template<typename T, typename...Components>
StaticSystem<T,Components...>::StaticSystem(T* system_object) : System(),
  m_types{SysComDescriptor{type_name<Components>(), access_type<Components>()}...},
  m_system_object(system_object)
  {

  }

template<typename T, typename...Components>
StaticSystem<T,Components...>::~StaticSystem() {
  delete m_system_object;
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply_block( const std::vector<void*>& component_blocks ) {
  apply_block( component_blocks, indices_list<sizeof...(Components)>());
}

template<typename T, typename...Components>
template<std::size_t...Indices>
void StaticSystem<T,Components...>::apply_block( const std::vector<void*>& component_blocks, 
                                         indices<Indices...> ) {
  apply_block(static_cast<Components*>(__builtin_assume_aligned(component_blocks[Indices],32))...);
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply_block(Components* __restrict__ ...components) {
  for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i) {
    m_system_object->run(&components[i]...);
  }
}

template<typename T, typename...Components>
void StaticSystem<T,Components...>::apply( const std::vector<void*>& components ) {
  apply(components,indices_list<sizeof...(Components)>());
}

template<typename T, typename...Components>
template<std::size_t...Indices>
void StaticSystem<T,Components...>::apply( const std::vector<void*>& components, 
                                         indices<Indices...> ) {
  m_system_object->run((static_cast<Components*>(components[Indices]))...);
}

template<typename T, typename...Components>
std::vector<SysComDescriptor> StaticSystem<T,Components...>::components() const {
  return m_types;
}

template<typename T, typename...Components> 
StaticSystem<T, Components...>* create_static_system(T* system, void (T::*)(Components*...)  ) {
  return new StaticSystem<T,Components...>(system);
}

template<typename TSystem, typename...TArgs>
auto create_static_system(TArgs&&...args) {
  TSystem* system_object = new TSystem(std::forward<TArgs>(args)...);
  return create_static_system(system_object, &TSystem::run );
}
  
} /* furious */ 
