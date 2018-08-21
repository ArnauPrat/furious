

#ifndef _FURIOUS_CONTEXT_H_
#define _FURIOUS_CONTEXT_H_ value

#include "../../common/common.h"
#include <vector>

namespace furious {

class Database;

struct IdComponentPair{
  int32_t id;
  int64_t table_id;
};

class Context {
public:
  Context(float dt, Database* database);
  ~Context() = default;

  template<typename TComponent>
    void enable_component(int32_t id);

  template<typename TComponent>
    void disable_component(int32_t id);

  float get_dt();

  Database* get_database();

private:
  float m_dt;                 
  std::vector<IdComponentPair> m_to_enable;
  std::vector<IdComponentPair> m_to_disable;

  Database*     p_database;

};
  
} /* furious */ 

#include "context.inl"
#endif /* ifndef _FURIOUS_CONTEXT_H_ */
