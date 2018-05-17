
#ifndef _FURIOUS_REFLECTION_H_
#define _FURIOUS_REFLECTION_H_

#include "reference.h"
#include <cassert>
#include <iostream>
#include <string>
#include <type_traits>

namespace furious {

template<typename T>                                 
  class has_name_method {                                                       
  private:
    template <typename U, U> struct type_check;                     
    template <typename U> static int32_t  check(type_check<std::string (*)(void), &U::name > *); 
    template <typename U> static int16_t check(...);                    
  public:
    static constexpr bool  value = (sizeof(check<T>(0)) == sizeof(int32_t));     
  };


template <typename T>
  typename std::enable_if<has_name_method<T>::value, std::string>::type type_name_func() {
    return T::name();
  } 

template <typename T>
  typename std::enable_if<!has_name_method<T>::value, std::string>::type type_name_func() {
    return typeid(T).name();
  }  

template <typename T>
  class type_name {
  public:
    static std::string name() {
      return type_name_func<T>();
    }
  };

template <typename T, const char* str>
  class type_name<Ref<T,str>> {
  public:
    static std::string name() {
      return type_name_func<T>();
    }
  };

enum class ComAccessType : uint8_t {
  E_READ,
  E_WRITE
};

template<typename T>
  typename std::enable_if<!std::is_const<typename std::remove_pointer<T>::type>::value,ComAccessType>::type access_type_func() {
    return ComAccessType::E_WRITE;
  }

template<typename T>
  typename std::enable_if<std::is_const<typename std::remove_pointer<T>::type>::value,ComAccessType>::type access_type_func() {
    return ComAccessType::E_READ;
  }

template<typename T>
class access_type {
public:
  static ComAccessType type() {
    return access_type_func<T>();
  }
};

template<typename T, const char* str>
class access_type<Ref<T,str>> {
public:
  static ComAccessType type() {
    return access_type_func<T>();
  }
};


template<typename T>
class caster {
public:
  inline static T* cast(void* ptr) {
    return static_cast<T*>(ptr);
  }
};

template<typename T, const char* str>
class caster<Ref<T, str>> {
public:
  inline static Ref<T,str> cast(void* ptr) {
    return Ref<T,str>(ptr);
  }
};


} /* furious */ 

#endif
