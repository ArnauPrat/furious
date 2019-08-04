
#ifndef _FURIOUS_OPERATOR_H_
#define _FURIOUS_OPERATOR_H_ 

#include "../../common/common.h"
#include "../../common/refcount_ptr.h"

#include "fcc_context.h"

#include <clang/AST/AST.h>
#include <string>

namespace furious
{

class FccSubPlanVisitor;

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
              const std::string& name);

  virtual
  ~FccOperator() = default;

  virtual void
  accept(FccSubPlanVisitor* visitor) const = 0;  

  FccOperatorType       m_type;
  std::string           m_name;
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
                   const std::string& name);

  virtual 
  ~FccOperatorTmplt() = default;

  virtual void
  accept(FccSubPlanVisitor* visitor) const override; 
};

/**
 * @brief Scan operator. Streams components from tables
 */
struct Scan : public FccOperatorTmplt<Scan> 
{
  explicit Scan(const std::string& ref_name);

  explicit Scan(QualType component,
                FccAccessMode access_mode);

  virtual 
  ~Scan() = default;

};

/**
 * @brief Join operator. Joins two tables by entity id
 */
struct Join : public FccOperatorTmplt<Join> 
{
  Join(RefCountPtr<FccOperator> left, 
       RefCountPtr<FccOperator> right);
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
                 RefCountPtr<FccOperator> right);
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
            RefCountPtr<FccOperator> right);
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
        FccAccessMode access_mode);

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
         const std::string& name);

  virtual 
  ~Filter();

  RefCountPtr<FccOperator> p_child;

};

struct PredicateFilter : public Filter<PredicateFilter>
{
  PredicateFilter(RefCountPtr<FccOperator> child,
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
  TagFilter(RefCountPtr<FccOperator> child,
            const std::string& tag,
            FccFilterOpType op_type,
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
          const DynArray<const FccSystem*>& systems);

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
         RefCountPtr<FccOperator> child);

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
                  RefCountPtr<FccOperator> child);

  virtual 
  ~CascadingGather();

  RefCountPtr<FccOperator> p_ref_table;
  RefCountPtr<FccOperator> p_child;
};
  
} /* furious */ 

#include "operator.inl"


#endif /* ifndef _FURIOUS_OPERATOR_H_ */
