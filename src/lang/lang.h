

#ifndef _FURIOUS_LANG_H_
#define _FURIOUS_LANG_H_

#include "macros.h"
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
 * \brief Object used to tune the set of entities the system runs on
 *
 * \tparam TSystem
 * \tparam typename...Components
 */
template <typename TSystem, typename... TComponents>
struct RegisterSystemInfo
{

};

/**
 * \brief Helper function used to extract the Component types of the run method
 * of the system. These Component types are then used to parametrize the
 * RegisterSystemInfo structure, specially methods such as filter in order to make sure
 * that these have the required parameters, namely the components.
 *
 * \tparam T The type of the system object
 * \tparam typename...Components The component types of the run method
 * \param system A pointer to the system object
 * \param T::* The function of the system object, which will always be "run".
 *
 * \return 
 */

template <typename T, typename ... TComponents>
typename std::enable_if<(std::is_pointer<TComponents>::value && ...), RegisterSystemInfo<T, TComponents ...> >::type
__register_foreach(T *system,
      void (T::*)(Context *, uint32_t id, TComponents...))
{
  return RegisterSystemInfo<T, TComponents ...>();
}

template<typename...TComponents>
struct MatchQueryBuilder
{
  /**
   * \brief Executes the system on the entities that qualify the given
   * predicate
   *
   * \param  The predicate 
   *
   * \return This RegisterSystemInfo
   */
  MatchQueryBuilder<TComponents...>
  filter(bool (*)(const typename std::remove_pointer<TComponents>::type *...))
  {
    return *this;
  }

  /**
   * \brief Executes the system on those entities with the given tag
   *
   * \param tag The tag to qualify
   *
   * \return This MatchQueryBuilder
   */
  MatchQueryBuilder<TComponents...>
  has_tag(const char *tag)
  {
    return *this;
  }

  /**
   * \brief Executes the system on those entities that do not have the given tag
   *
   * \param tag The tag to qualify
   *
   * \return This MatchQueryBuilder
   */
  MatchQueryBuilder<TComponents...>
  has_not_tag(const char *tag)
  {
    return *this;
  }

  /**
   * \brief Executes the system on those entities with the given component
   *
   * \tparam TComponent The component to qualify
   *
   * \return This MatchQueryBuilder
   */
  template<typename TComponent>
  MatchQueryBuilder<TComponents...>
  has_component()
  {
    return *this;
  }

  /**
   * \brief Executes the system on those entities that do not have the given component
   *
   * \tparam TComponent The component to qualify
   *
   * \return This MatchQueryBuilder
   */
  template<typename TComponent>
  MatchQueryBuilder<TComponents...>
  has_not_component()
  {
    return *this;
  }

  template<typename...QComponents>
  MatchQueryBuilder<TComponents..., QComponents...>
  expand(const std::string& str)
  {
    return MatchQueryBuilder<TComponents..., QComponents...>();
  }

  /**
   * \brief Applies a system for each selected element
   *
   * \tparam TSystem The type of the system to apply
   * \tparam typename...TArgs The type arguments for the system constructor
   * \param TArgs...args The arguments for the system constructor
   *
   * \return 
   */
  template <typename TSystem, typename...TArgs>
  auto
  foreach(TArgs&&...args)
  {
    TSystem *system_object = new TSystem(std::forward<TArgs>(args)...);
    return __register_foreach(system_object, &TSystem::run);
  }
  
};

/**
 * \brief Method use to start a query in a furious script by selecting those
 * entitties with the given components
 *
 * \tparam typename...TComponents The components to select
 *
 * \return Returns a MatchQueryBuilder 
 */
template<typename...TComponents>
MatchQueryBuilder<TComponents...> match() 
{
  return MatchQueryBuilder<TComponents...>();
}


} // namespace furious

#endif /* ifndef _FURIOUS_LANG_H_ */
