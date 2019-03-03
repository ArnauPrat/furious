
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/types.h"
#include "../common/dyn_array.h"

#include <clang/AST/AST.h>

using namespace clang;

namespace furious 
{

struct FccContext;
struct FccSystem;
class FccExecPlanVisitor;

enum class FccOperatorType 
{
  E_JOIN,
  E_FILTER,
  E_SCAN,
  E_FOREACH,
};

class FccOperator
{
public:

  FccOperator(FccOperatorType type);

  virtual
  ~FccOperator() = default;

  virtual void
  accept(FccExecPlanVisitor* visitor) const = 0;  

  FccOperatorType m_type;
  const FccOperator*  p_parent;
};

/**
 * @brief Base structure for an operator 
 */
template<typename T>
class FccOperatorTmplt : public FccOperator
{
public:
  FccOperatorTmplt(const FccOperatorType type);

  virtual 
  ~FccOperatorTmplt() = default;

  virtual void
  accept(FccExecPlanVisitor* visitor) const override; 
};

/**
 * @brief Scan operator. Streams components from tables
 */
struct Scan : public FccOperatorTmplt<Scan> 
{
  Scan(QualType component);

  virtual 
  ~Scan() = default;

  QualType  m_component;
};

/**
 * @brief Join operator. Joins two tables by entity id
 */
struct Join : public FccOperatorTmplt<Join> 
{
  Join(FccOperator* left, 
       FccOperator* right);
  virtual 
  ~Join();

  FccOperator * p_left;
  FccOperator * p_right;
};

/**
 * @brief Filter operator. Filter components by entity tags
 */
template<typename T>
struct Filter : public FccOperatorTmplt<T> 
{
  Filter(FccOperator* child);

  virtual 
  ~Filter();

  FccOperator* p_child;
};

struct PredicateFilter : public Filter<PredicateFilter>
{
  PredicateFilter(FccOperator* child,
                  const FunctionDecl* func_decl);
  virtual 
  ~PredicateFilter() = default;

  const FunctionDecl* p_func_decl;
};

enum class FccFilterOpType
{
  E_HAS,
  E_HAS_NOT
};

struct TagFilter : public Filter<TagFilter> 
{
  TagFilter(FccOperator* child,
            const std::string& tag,
            FccFilterOpType op_type);

  virtual 
  ~TagFilter() = default;

  const std::string m_tag;
  FccFilterOpType   m_op_type;
};

struct ComponentFilter : public Filter<ComponentFilter>
{
  ComponentFilter(FccOperator* child,
                  QualType component_type,
                  FccFilterOpType op_type);

  virtual 
  ~ComponentFilter() = default;

  QualType          m_component_type;
  FccFilterOpType   m_op_type;
};


/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public FccOperatorTmplt<Foreach>  
{
  Foreach(FccOperator* child, 
          const DynArray<const FccSystem*>& systems);

  virtual 
  ~Foreach();

  DynArray<const FccSystem*>      p_systems;
  FccOperator*                    p_child;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct FccExecPlan 
{
  FccExecPlan(FccContext* context);
  ~FccExecPlan();

  bool
  bootstrap();

  /**
   * \brief Inserts a new root to the execution plan
   *
   * \param ast_context The ast context of the root operator 
   * \param FccOperator The root operator
   */
  void
  insert_root(ASTContext* ast_context, 
              FccOperator*);

  FccContext*               p_context;
  DynArray<ASTContext*>     p_asts;
  DynArray<FccOperator*>    p_roots; 
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


class FccExecPlanVisitor
{
public:
  void
  traverse(const FccExecPlan* plan);

  virtual void 
  visit(const Foreach* foreach) = 0;

  virtual void 
  visit(const Scan* scan) = 0;

  virtual void
  visit(const Join* join) = 0;

  virtual void 
  visit(const TagFilter* tag_filter) = 0;

  virtual void
  visit(const ComponentFilter* component_filter) = 0;

  virtual void
  visit(const PredicateFilter* predicate_filter) = 0;
};

}  

#include "execution_plan.inl"

#endif /* ifndef SYMBOL */
