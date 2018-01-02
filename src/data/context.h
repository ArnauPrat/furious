

#ifndef _FURIOUS_CONTEXT_H_
#define _FURIOUS_CONTEXT_H_ value

#include "../common/common.h"
#include <vector>

namespace furious {

struct IdComponentPair{
  uint32_t id;
  uint64_t table_id;
};

class Context {
public:
  Context() = default;
  ~Context() = default;

  template<typename TComponent>
    void enable_component(uint32_t id);

  template<typename TComponent>
    void disable_component(uint32_t id);

  std::vector<IdComponentPair> m_to_enable;
  std::vector<IdComponentPair> m_to_disable;

};
  
} /* furious */ 

#include "context.inl"
#endif /* ifndef _FURIOUS_CONTEXT_H_ */
