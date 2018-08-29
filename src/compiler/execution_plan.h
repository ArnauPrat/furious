
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/common.h"

#include <clang/AST/AST.h>
#include <vector>

using namespace clang;

namespace furious {

struct FccContext;
struct FccSystemInfo;

enum class FccOperatorType {
  E_JOIN,
  E_FILTER,
  E_SCAN,
  E_FOREACH,
};

/**
 * @brief Base structure for an operator 
 */
struct FccOperator {
  FccOperator(FccOperatorType type);
  virtual ~FccOperator() = default;

   FccOperatorType m_type;
};

/**
 * @brief Scan operator. Streams components from tables
 */
struct Scan : public FccOperator {
  Scan(QualType component);
  virtual ~Scan() = default;

  QualType  m_component;
};

/**
 * @brief Join operator. Joins two tables by entity id
 */
struct Join : public FccOperator {
  Join(FccOperator* left, 
       FccOperator* right);
  virtual ~Join();

  FccOperator* p_left;
  FccOperator* p_right;
};

/**
 * @brief Filter operator. Filter components by entity tags
 */
struct Filter : public FccOperator 
{
  Filter(FccOperator* child);
  virtual ~Filter();

  FccOperator* p_child;
};

struct PredicateFilter : public Filter 
{
  PredicateFilter(FccOperator* child,
                  const FunctionDecl* func_decl);
  virtual ~PredicateFilter() = default;

  const FunctionDecl*   p_func_decl;
};

enum class FccFilterOpType
{
  E_WITH,
  E_WITHOUT
};

struct TagFilter : public Filter 
{
  TagFilter(FccOperator* child,
            const std::string& tag,
            FccFilterOpType op_type);
  virtual ~TagFilter() = default;

  const std::string m_tag;
  FccFilterOpType   m_op_type;
};

struct ComponentFilter : public Filter
{
  ComponentFilter(FccOperator* child,
                  QualType component_type,
                  FccFilterOpType op_type);
  virtual ~ComponentFilter() = default;

  QualType          m_component_type;
  FccFilterOpType   m_op_type;
};


/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public FccOperator  {
  Foreach(FccOperator* child, 
          const std::vector<const FccSystemInfo*>& systems);
  virtual ~Foreach();

  std::vector<const FccSystemInfo*>  m_systems;
  FccOperator*                          p_child;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct ExecPlan 
{
  ExecPlan(FccContext* context);
  ~ExecPlan();

  FccContext*            p_context;
  std::vector<FccOperator*> m_roots; 
};

} /* execution_plan */ 

#endif /* ifndef SYMBOL */
