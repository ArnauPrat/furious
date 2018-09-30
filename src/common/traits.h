
#ifndef _FURIOUS_REFLECTION_H_
#define _FURIOUS_REFLECTION_H_

#include "reference.h"
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

namespace furious {

/**
 * @brief Auxiliary class used for trait type_name 
 *
 * @tparam T The tamplate argument
 */
template<typename T>                                 
  class has_name_method {                                                       
  private:
    template <typename U, U> struct type_check;                     
    template <typename U> static int32_t  check(type_check<std::string (*)(void), &U::name > *); 
    template <typename U> static int16_t check(...);                    
  public:
    static constexpr bool  value = (sizeof(check<T>(0)) == sizeof(int32_t));     
  };


/**
 * @brief Function used for trait typename that calls the static method name()
 *
 * @tparam T
 *
 * @return 
 */
template <typename T>
  typename std::enable_if<has_name_method<T>::value, std::string>::type type_name_func() {
    return T::name();
  } 

/**
 * @brief Function used for trait typename that returns the compiler generated
 * type name 
 *
 * @tparam T
 *
 * @return 
 */
template <typename T>
  typename std::enable_if<!has_name_method<T>::value, std::string>::type type_name_func() {
    return typeid(T).name();
  }  



/**
 * @brief Static class used to extract the name of a regular type
 *
 * @tparam T
 */
template <typename T>
  class type_name {
  public:
    static std::string name() {
      return type_name_func<T>();
    }
  };

/**
 * @brief Static class used to extract the name of a Ref type
 *
 * @tparam T
 */
template <typename T, const char* str>
  class type_name<Ref<T,str>> {
  public:
    static std::string name() {
      return type_name_func<T>();
    }
  };

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * @brief Component access type
 */
enum class ComAccessType : uint8_t {
  E_READ,
  E_WRITE
};

/**
 * @brief Axuliary function used to extract the access type of a non-const component
 *
 * @tparam T
 *
 * @return 
 */
template<typename T>
  typename std::enable_if<!std::is_const<typename std::remove_pointer<T>::type>::value,ComAccessType>::type access_type_func() {
    return ComAccessType::E_WRITE;
  }

/**
 * @brief Axuliary function used to extract the access type of a const component
 *
 * @tparam T
 *
 * @return 
 */
template<typename T>
  typename std::enable_if<std::is_const<typename std::remove_pointer<T>::type>::value,ComAccessType>::type access_type_func() {
    return ComAccessType::E_READ;
  }


/**
 * @brief Staic class used to extract the access type of a regular Component
 *
 * @tparam T
 */
template<typename T>
class access_type {
public:
  static ComAccessType type() {
    return access_type_func<T>();
  }
};

/**
 * @brief Staic class used to extract the access type of a ref Component
 *
 * @tparam T
 */
template<typename T, const char* str>
class access_type<Ref<T,str>> {
public:
  static ComAccessType type() {
    return access_type_func<T>();
  }
};


/**
 * @brief Static class used to cast a void* to the actual component type, for
 * regular types
 *
 * @tparam T
 */
template<typename T>
class caster {
public:
  inline static T* cast(void* ptr) {
    return static_cast<T*>(ptr);
  }
};

/**
 * @brief Static class used to cast a void* to the actual component type, for
 * reference types
 *
 * @tparam T
 */
template<typename T, const char* str>
class caster<Ref<T, str>> {
public:
  inline static Ref<T,str> cast(void* ptr) {
    return Ref<T,str>(static_cast<T*>(ptr));
  }
};

/**
 * @brief Static trait class used check if a type is a Ref type
 *
 * @tparam T
 */
template<typename T>
class is_ref {
public:
  static constexpr bool value() {
    return false;
  }
};

/**
 * @brief Static trait class used check if a type is a Ref type
 *
 * @tparam T
 */
template<typename T, const char* str>
class is_ref<Ref<T,str>> {
public:
  static constexpr bool value() {
    return true;
  }
};


/**
 * @brief Static trait class used get the name of a Ref type 
 *
 * @tparam T
 */
template<typename T>
class ref_name {
public:
  static constexpr const char* value() {
    return nullptr; 
  }

};

/**
 * @brief Static trait class used get the name of a Ref type 
 *
 * @tparam T
 */
template<typename T, const char* str>
class ref_name<Ref<T,str>> {
public:
  static constexpr const char* value() {
    return str;
  }
};


} /* furious */ 

#endif
