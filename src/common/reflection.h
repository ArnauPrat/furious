
#ifndef _FURIOUS_REFLECTION_H_
#define _FURIOUS_REFLECTION_H_

#include <string>
#include <cassert>
#include <iostream>

namespace furious {

template <typename T>
  std::string type_name() {
    return typeid(T).name();
  }  
} /* furious */ 

#endif
