

#ifndef _FURIOUS_REFERENCE_H_
#define _FURIOUS_REFERENCE_H_

#include <string>

namespace furious {

template<typename T, const char * str> 
  class Ref {

  public:
    using type = T;

    Ref(void* data) : 
			p_data{static_cast<T*>(data)} {
    }

    std::string name() {
      return str;
    }


  private:
    T* p_data;
  };
  
} /* furious */ 

#endif  

