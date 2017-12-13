

#ifndef _FURIOUS_EXECUTION_ENGINE_H_
#define _FURIOUS_EXECUTION_ENGINE_H_

#include "logic/logic_join.h"
#include "logic/logic_plan.h"
#include "common.h"
#include "system.h"
#include "database.h"
#include "physical/physical_plan.h"
#include <map>
#include <memory>

namespace furious {

class ExecutionEngine {

public:
  ExecutionEngine( const ExecutionEngine& ) = delete;
  ExecutionEngine( ExecutionEngine&& ) = delete;

  virtual ~ExecutionEngine();

  ExecutionEngine& operator=( const ExecutionEngine& ) = delete;
  ExecutionEngine& operator=( ExecutionEngine&& ) = delete;

  //////////////////////////////////////////////
  //////////////////////////////////////////////
  //////////////////////////////////////////////

  /**
   * Adds a system into the the execution engine
   * */
  template <typename T, typename...Args>
    SystemId register_system(Args&&...x);

  /**
   * Runs the registered systems
   */
  void run_systems() const ;

  /**
   * Gets the specified system
   **/
  System* get_system(SystemId system);

  /**
   * @brief Gets the pointer to the instance of the ExecutionEngine singleton
   *
   * @return Returns the pointer to the instance to the ExecutionEngine singleton
   */
  static ExecutionEngine* get_instance();

  /*
   * Build the logic plan for the current registered systems
   */
  LogicPlan build_logic_plan() const ;

  /**
   * Builds a physical plan out of a logic plan
   */
  PhysicalPlan  build_physical_plan( const LogicPlan& logic_plan) const;

  /**
   * Executes the given physical plan
   */
  void execute_physical_plan( const PhysicalPlan& physical_plan ) const;

  /**
   * Clears the registered systems 
   */
  void clear();

private:

  ExecutionEngine() = default;

  SystemId                  m_next_id = 0; /** The next id to assign to a sysstem **/
  SystemMap                 m_systems;     /** Holds the list of registered systems **/
};

template <typename T, typename...Args>
SystemId ExecutionEngine::register_system(Args&&...x) {
  System* sp = static_cast<System*>(new T(m_next_id,std::forward<Args>(x)...));
  m_systems.insert(SystemMapPair(m_next_id,sp));
  return m_next_id++;
}

} /* furious */ 

#endif
