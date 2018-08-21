
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/common.h"

#include <vector>

namespace furious {

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
  Scan(int32_t table_id);
  virtual ~Scan() = default;
  
  int32_t m_table_id;
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
  Filter(Operator* child, 
         int32_t func_id);
  virtual ~Filter();

  int32_t   m_func_id;
  Operator* p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public Operator  {
  Foreach(Operator* child, 
          const std::vector<int32_t>& funcs);
  virtual ~Foreach();

  std::vector<int32_t> 		m_funcs;
  Operator*               p_child;
};

} /* execution_plan */ 

#endif /* ifndef SYMBOL */
