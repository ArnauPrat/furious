

#ifndef _FURIOUS_CONTEXT_H_
#define _FURIOUS_CONTEXT_H_ value

#include "../common/common.h"

namespace furious {

class Context {
public:
  Context() = default;
  ~Context() = default;

  template<typename TComponent>
    void enable_component(uint32_t id);

  template<typename TComponent>
    void disable_component(uint32_t id);

};
  
} /* furious */ 
#endif /* ifndef _FURIOUS_CONTEXT_H_ */
