
#ifndef _FURIOUS_WORKLOAD_H_
#define _FURIOUS_WORKLOAD_H_

#include "../common/common.h"
#include "system.h"

#include <string>
#include <vector>
#include <map>

namespace furious {

class System;

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
    void register_system(TArgs&&...args);

  template<typename TSystem>
    void remove_system();

private:

  std::map<std::string, System*> m_systems;

};

} /* furious */ 

#include "workload.inl"
#endif /* ifndef _FURIOUS_WORKLOAD_H_ */
