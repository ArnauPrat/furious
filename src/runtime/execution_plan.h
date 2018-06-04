
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/common.h"
#include "workload.h"

namespace furious {

class Table;

enum class OperatorType {
  E_JOIN,
  E_FILTER,
  E_SCAN,
  E_FOREACH,
};

struct Operator {
  Operator(OperatorType type);
  virtual ~Operator() = default;

  OperatorType m_type;
};

struct Scan : public Operator {
  Scan(const std::string& table_name);
  virtual ~Scan() = default;
  
  std::string m_table_name;
};

struct Join : public Operator {
  Join(Operator* left, 
       Operator* right);
  virtual ~Join();

  Operator* p_left;
  Operator* p_right;
};

struct Filter : public Operator {
  Filter(const std::string& tag, 
         Operator* child);
  virtual ~Filter();

  std::string m_tag;
  Operator* p_child;
};

struct Foreach : public Operator  {
  Foreach(const std::string& system_name, 
          Operator* child);
  virtual ~Foreach();

  std::string m_system_name;
  Operator*   p_child;
};


struct ExecutionPlan {
  ExecutionPlan() = default;
  virtual ~ExecutionPlan();
  std::vector<Operator*> m_queries;
};

/**
 * @brief Creates an execution plan from a set of systems
 *
 * @param systems A vector with the systems to create the execution plan from
 *
 * @return Returns a newly created execution plan
 */
ExecutionPlan* create_execution_plan( const std::vector<SystemExecInfo>& systems );

/**
 * @brief Destroys an execution plan
 *
 * @param exec_plan The execution plan to destroy
 */
void destroy_execution_plan( ExecutionPlan* exec_plan );

} /* execution_plan */ 

#endif /* ifndef SYMBOL */
