
#ifndef _FURIOUS_WORKLOAD_H_
#define _FURIOUS_WORKLOAD_H_

#include "../common/common.h"
#include "system.h"

#include <string>
#include <vector>
#include <map>

namespace furious {

class System;
class Database;
class Workload;


class ScopeModifier {
public:
  ScopeModifier(Workload* workload, 
                System* system);

  ScopeModifier& restrict_to(const std::vector<std::string>& tags);

private:
  Workload*   p_workload;
  System*     p_system;

};

class SystemExecInfo {
public:
  SystemExecInfo(System* system);

  System * const                    p_system;
  std::vector<std::string>          m_tags;
};

class Workload final {
public:

  Workload();
  Workload(const Workload& workload) = delete;
  ~Workload();
  void operator=(const Workload& workload ) = delete;

  ////////////////////////////////////////////////
  ////////////////////////////////////////////////
  ////////////////////////////////////////////////

  /**
   * @brief Registers a system to the set of running systems
   *
   * @tparam TSystem The system to register  
   * @tparam typename...TComponent The components this system works with
   */
  template<typename TSystem, typename...TArgs>
    ScopeModifier add_system(TArgs&&...args);

  template<typename TSystem>
    void remove_system();

  const std::vector<SystemExecInfo>& get_systems() const;

private:
  friend class ScopeModifier;

  void reset_execution_plan();

  int32_t                     m_next_id;
  std::vector<SystemExecInfo> m_systems;

};

} /* furious */ 

#include "workload.inl"
#endif /* ifndef _FURIOUS_WORKLOAD_H_ */
