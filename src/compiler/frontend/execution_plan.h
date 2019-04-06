
#ifndef _FURIOUS_EXECUTION_PLAN_H_
#define _FURIOUS_EXECUTION_PLAN_H_

#include "../common/types.h"
#include "../common/dyn_array.h"
#include "../common/refcount_ptr.h"

#include "fcc_context.h"

#include <clang/AST/AST.h>

using namespace clang;

namespace furious 
{

struct FccSystem;
class FccExecPlanVisitor;

enum class FccOperatorType 
{
  E_SCAN,
  E_JOIN,
  E_LEFT_FILTER_JOIN,
  E_CROSS_JOIN,
  E_FETCH,
  E_FILTER,
  E_FOREACH,
  E_GATHER,
  E_CASCADING_GATHER,
};

enum class FccColumnType
{
  E_REFERENCE,
  E_COMPONENT,
  E_GLOBAL,
};

struct FccColumn
{
  FccColumnType m_type;
  QualType      m_q_type;
  std::string   m_ref_name;
  FccAccessMode m_access_mode;
};


class FccOperator
{
public:

  FccOperator(FccOperatorType type, 
              const std::string& name,
              FccContext* fcc_context);

  virtual
  ~FccOperator() = default;

  virtual void
  accept(FccExecPlanVisitor* visitor) const = 0;  

  FccOperatorType       m_type;
  std::string           m_name;
  FccContext*           p_fcc_context;
  const FccOperator*    p_parent;
  DynArray<FccColumn>   m_columns;
  uint32_t              m_id;
};

/**
 * @brief Base structure for an operator 
 */
template<typename T>
class FccOperatorTmplt : public FccOperator
{
public:
  FccOperatorTmplt(FccOperatorType type,
                   const std::string& name,
                   FccContext* fcc_context);

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
  explicit Scan(const std::string& ref_name,
                FccContext* fcc_context);
  explicit Scan(QualType component,
                FccAccessMode access_mode,
                FccContext* fcc_context);

  virtual 
  ~Scan() = default;

};

/**
 * @brief Join operator. Joins two tables by entity id
 */
struct Join : public FccOperatorTmplt<Join> 
{
  Join(RefCountPtr<FccOperator> left, 
       RefCountPtr<FccOperator> right,
       FccContext* fcc_context);
  virtual 
  ~Join();

  uint32_t m_split_point;
  RefCountPtr<FccOperator> p_left;
  RefCountPtr<FccOperator> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct LeftFilterJoin : public FccOperatorTmplt<LeftFilterJoin> 
{
  LeftFilterJoin(RefCountPtr<FccOperator> left, 
                 RefCountPtr<FccOperator> right,
                 FccContext* fcc_context);
  virtual 
  ~LeftFilterJoin();

  uint32_t m_split_point;
  RefCountPtr<FccOperator> p_left;
  RefCountPtr<FccOperator> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct CrossJoin : public FccOperatorTmplt<CrossJoin> 
{
  CrossJoin(RefCountPtr<FccOperator> left, 
            RefCountPtr<FccOperator> right,
            FccContext* fcc_context);
  virtual 
  ~CrossJoin();

  uint32_t m_split_point;
  RefCountPtr<FccOperator> p_left;
  RefCountPtr<FccOperator> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct Fetch : public FccOperatorTmplt<Fetch> 
{
  Fetch(QualType global_type,
        FccAccessMode access_mode,
        FccContext* fcc_context);

  virtual 
  ~Fetch();

  QualType m_global_type;
};

/**
 * @brief Filter operator. Filter components by entity tags
 */
template<typename T>
struct Filter : public FccOperatorTmplt<T> 
{
  Filter(RefCountPtr<FccOperator> child,
         const std::string& name,
         FccContext* fcc_context);

  virtual 
  ~Filter();

  RefCountPtr<FccOperator> p_child;

};

struct PredicateFilter : public Filter<PredicateFilter>
{
  PredicateFilter(RefCountPtr<FccOperator> child,
                  const FunctionDecl* func_decl,
                  FccContext* fcc_context);
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
  TagFilter(RefCountPtr<FccOperator> child,
            const std::string& tag,
            FccFilterOpType op_type,
            FccContext* fcc_context,
            bool on_column = false);

  virtual 
  ~TagFilter() = default;

  const std::string m_tag;
  bool              m_on_column;
  FccFilterOpType   m_op_type;
};

struct ComponentFilter : public Filter<ComponentFilter>
{
  ComponentFilter(RefCountPtr<FccOperator> child,
                  QualType component_type,
                  FccFilterOpType op_type,
                  FccContext* fcc_context,
                  bool on_column = false);

  virtual 
  ~ComponentFilter() = default;

  QualType          m_filter_type;
  bool              m_on_column;
  FccFilterOpType   m_op_type;
};


/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public FccOperatorTmplt<Foreach>  
{
  Foreach(RefCountPtr<FccOperator> child, 
          const DynArray<const FccSystem*>& systems,
          FccContext* fcc_context);

  virtual 
  ~Foreach();

  DynArray<const FccSystem*>      p_systems;
  RefCountPtr<FccOperator>        p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Gather : public FccOperatorTmplt<Gather>  
{
  Gather(RefCountPtr<FccOperator> ref_table,
         RefCountPtr<FccOperator> child,
         FccContext* fcc_context);

  virtual 
  ~Gather();

  RefCountPtr<FccOperator> p_ref_table;
  RefCountPtr<FccOperator> p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct CascadingGather : public FccOperatorTmplt<CascadingGather>  
{
  CascadingGather(RefCountPtr<FccOperator> ref_table,
                  RefCountPtr<FccOperator> child,
                  FccContext* fcc_context);

  virtual 
  ~CascadingGather();

  RefCountPtr<FccOperator> p_ref_table;
  RefCountPtr<FccOperator> p_child;
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct FccExecPlan 
{
  FccExecPlan(FccContext* context);
  ~FccExecPlan();

  FccCompilationErrorType 
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
  visit(const LeftFilterJoin* left_filter_join) = 0;

  virtual void
  visit(const CrossJoin* cross_join) = 0;

  virtual void
  visit(const Fetch* fetch) = 0;

  virtual void 
  visit(const TagFilter* tag_filter) = 0;

  virtual void
  visit(const ComponentFilter* component_filter) = 0;

  virtual void
  visit(const PredicateFilter* predicate_filter) = 0;

  virtual void
  visit(const Gather* gather) = 0;

  virtual void
  visit(const CascadingGather* casc_gather) = 0;
};

}  

#include "execution_plan.inl"

#endif /* ifndef SYMBOL */
