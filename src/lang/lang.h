

#ifndef _FURIOUS_LANG_H_
#define _FURIOUS_LANG_H_

#include "../common/common.h"
#include "../runtime/data/context.h"

#include <type_traits>

/**
 * This file contain objects and functions that conform the DSL used to
 * implement the game logic. The code calling the functions of this file is not
 * meant to be compiled by the regular compiler, but by the furious compiler. 
 *
 * Symbols defined here DO NOTHING but are defined in in such that the code
 * using them is both semantically and syntactically correct from a C++
 * perspective. Thus, IDEs parsing .cpp and .hpp files that use the code defined
 * here should catch errors due to a missuse of the API.
 *
 * Then, these .cpp and .hpp files are meant to be passed to the furious
 * compiled insetead of the regular compiler. The furious compiler will
 * translate the code using the furious DSL to an equivalent .cpp file to be
 * compiled and linked with the final program.
 **/

#define BEGIN_FURIOUS_SCRIPT   \
  static void furious_script() \
  {

#define END_FURIOUS_SCRIPT \
  }

namespace furious
{

/**
 * @brief Object used to tune the set of entities the system runs on
 *
 * @tparam TSystem
 * @tparam typename...Components
 */
template <typename TSystem, typename... Components>
struct RegisterSystemInfo
{

  /**
   * @brief Executes the system on the entities that qualify the given
   * predicate
   *
   * @param  The predicate 
   *
   * @return This RegisterSystemInfo
   */
  RegisterSystemInfo<TSystem, Components...> &
  filter(bool (*)(const typename std::remove_pointer<Components>::type *...))
  {
    return *this;
  }

  /**
   * @brief Executes the system on those entities with the given tag
   *
   * @param tag The tag to qualify
   *
   * @return This RegisterSystemInfo
   */
  RegisterSystemInfo<TSystem, Components...> &
  with_tag(const char *tag)
  {
    return *this;
  }

  /**
   * @brief Executes the system on those entities without the given tag
   *
   * @param tag The tag to qualify
   *
   * @return This RegisterSystemInfo
   */
  RegisterSystemInfo<TSystem, Components...> &
  without_tag(const char *tag)
  {
    return *this;
  }

  /**
   * @brief Executes the system on those entities with the given component
   *
   * @tparam TComponent The component to qualify
   *
   * @return This RegisterSystemInfo
   */
  template <typename TComponent>
  RegisterSystemInfo<TSystem, Components...> &
  with_component()
  {
    return *this;
  }

  /**
   * @brief Executes the system on those entities without the given component
   *
   * @tparam TComponent The component to qualify
   *
   * @return This RegisterSystemInfo
   */
  template <typename TComponent>
  RegisterSystemInfo<TSystem, Components...> &
  without_component()
  {
    return *this;
  }
};

/**
 * @brief Helper function used to extract the Component types of the run method
 * of the system. These Component types are then used to parametrize the
 * RegisterSystemInfo structure, specially methods such as filter in order to make sure
 * that these have the required parameters, namely the components.
 *
 * @tparam T The type of the system object
 * @tparam typename...Components The component types of the run method
 * @param system A pointer to the system object
 * @param T::* The function of the system object, which will always be "run".
 *
 * @return 
 */
template <typename T, typename... Components>
typename std::enable_if<(std::is_pointer<Components>::value && ...), RegisterSystemInfo<T, Components...>>::type
__register_foreach(T *system,
      void (T::*)(Context *, int32_t id, Components...))
{
  return RegisterSystemInfo<T, Components...>();
}

/**
 * @brief This method is called in order to run a system over a set of component
 * types. The component types are specified in the system's run method, and thus
 * are implicitly extracted from it. 
 *
 * @tparam TSystem The type of the system to run
 * @tparam typename...TArgs The type of the arguments to initialize the system
 * @param ...args  The arguments to initialize the system
 *
 * @return Returns a RegisterSystemInfo object
 */
template <typename TSystem, typename... TArgs>
auto register_foreach(TArgs &&... args)
{
  TSystem *system_object = new TSystem(std::forward<TArgs>(args)...);
  return __register_foreach(system_object, &TSystem::run);
}

} // namespace furious

#endif /* ifndef _FURIOUS_LANG_H_ */
