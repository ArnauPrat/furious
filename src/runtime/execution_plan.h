
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/common.h"
#include "workload.h"

namespace furious {

class Workload;
class Database;
class Table;

enum class OperatorType {
  E_JOIN,
  E_FILTER,
  E_SCAN,
  E_FOREACH,
};

/**
 * @brief Base structure for an operator 
 */
struct Operator {
  Operator(OperatorType type);
  virtual ~Operator() = default;

  OperatorType m_type;
};

/**
 * @brief Scan operator. Streams components from tables
 */
struct Scan : public Operator {
  Scan(const std::string& table_name);
  virtual ~Scan() = default;
  
  std::string m_table_name;
};

/**
 * @brief Join operator. Joins two tables by entity id
 */
struct Join : public Operator {
  Join(Operator* left, 
       Operator* right);
  virtual ~Join();

  Operator* p_left;
  Operator* p_right;
};

/**
 * @brief Filter operator. Filter components by entity tags
 */
struct Filter : public Operator {
  Filter(const std::vector<std::string>& tags, 
         Operator* child);
  virtual ~Filter();

  std::vector<std::string> m_tags;
  Operator* p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public Operator  {
  Foreach(int32_t system_id, 
          Operator* child);
  virtual ~Foreach();

  int32_t 		m_system_id;
  Operator*   p_child;
};


/**
 * @brief Represents an execution plan
 */
struct ExecutionPlan {
  ExecutionPlan() = default;
  virtual ~ExecutionPlan();

  std::vector<Operator*> m_queries;
};

/**
 * @brief Creates an execution plan from a set of systems
 *
 * @param workload The workload to create the execution plan for 
 * @param database The database the workload is going to be executed on 
 *
 * @return Returns a newly created execution plan
 */
ExecutionPlan* create_execution_plan( Workload* workload, Database* database );

/**
 * @brief Destroys an execution plan
 *
 * @param exec_plan The execution plan to destroy
 */
void destroy_execution_plan( ExecutionPlan* exec_plan );

} /* execution_plan */ 

#endif /* ifndef SYMBOL */
