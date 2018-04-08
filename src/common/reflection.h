
#ifndef _FURIOUS_REFLECTION_H_
#define _FURIOUS_REFLECTION_H_

#include <string>
#include <cassert>
#include <iostream>
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
  typename std::enable_if<has_name_method<T>::value, std::string>::type type_name() {
    return T::name();
  } 

template <typename T>
  typename std::enable_if<!has_name_method<T>::value, std::string>::type type_name() {
    return typeid(T).name();
  }  

enum class ComAccessType : uint8_t {
  E_READ,
  E_WRITE
};

template<typename T>
  typename std::enable_if<!std::is_const<T>::value,ComAccessType>::type access_type() {
    return ComAccessType::E_WRITE;
  }

template<typename T>
  typename std::enable_if<std::is_const<T>::value,ComAccessType>::type access_type() {
    return ComAccessType::E_READ;
  }

} /* furious */ 

#endif
