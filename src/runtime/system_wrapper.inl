

namespace furious 
{

template<typename T, typename...Components>
SystemWrapper<T,Components...>::SystemWrapper(T* system_object) : 
p_system_object(system_object)
{

}

template<typename T, typename...Components>
SystemWrapper<T,Components...>::~SystemWrapper()
{
  delete p_system_object;
}

template<typename T, typename...Components>
inline 
void SystemWrapper<T,Components...>::apply_block(Context* context, 
                                                 uint32_t block_start,
                                                 const Bitmap* mask,
                                                 Components* __restrict__ ...components) 
{
  if(mask->num_set() == TABLE_BLOCK_SIZE) 
  {
    for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i) 
    {
      p_system_object->run(context, block_start+i, &components[i]...);
    }
  } 
  else 
  {
    for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i) 
    {
      if(mask->is_set(i)) 
      {
        p_system_object->run(context, block_start+i, &components[i]...);
      }
    }
  }
}


template<typename T, typename...Components> 
SystemWrapper<T, Components...>* 
create_system(T* system, 
              void (T::*)(Context*, uint32_t id, Components*...)  ) 
{
  return new SystemWrapper<T,Components...>(system);
}

template<typename TSystem, typename...TArgs>
auto 
create_system(TArgs&&...args) -> decltype(create_system(new TSystem(std::forward<TArgs>(args)...), &TSystem::run))
{
  TSystem* system_object = new TSystem(std::forward<TArgs>(args)...);
  return create_system(system_object, &TSystem::run);
}

template<typename TSystem, typename...Components>
void 
destroy_system(SystemWrapper<TSystem,Components...>* system) 
{
  delete system;
}

} /* furious */ 
