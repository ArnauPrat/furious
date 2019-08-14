
#ifndef _FURIOUS_OPERATOR_H_
#define _FURIOUS_OPERATOR_H_ 

#include "../../common/common.h"
#include "../../common/refcount_ptr.h"

#include "fcc_context.h"

namespace furious
{

#define MAX_OPERATOR_NAME 256

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
  FccColumnType     m_type;
  fcc_type_t        m_component_type;
  char              m_ref_name[MAX_REF_NAME];
  fcc_access_mode_t m_access_mode;
};


class FccOperator
{
public:

  FccOperator(FccOperatorType type, 
              const char* name);

  virtual
  ~FccOperator() = default;

  virtual void
  accept(FccSubPlanVisitor* visitor) const = 0;  

  FccOperatorType       m_type;
  char                  m_name[MAX_OPERATOR_NAME];
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
                   const char* name);

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
  explicit Scan(const char* ref_name);

  explicit Scan(fcc_type_t component,
                fcc_access_mode_t access_mode);

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
  Fetch(fcc_type_t global_type,
        fcc_access_mode_t access_mode);

  virtual 
  ~Fetch();

  fcc_type_t m_global_type;
};

/**
 * @brief Filter operator. Filter components by entity tags
 */
template<typename T>
struct Filter : public FccOperatorTmplt<T> 
{
  Filter(RefCountPtr<FccOperator> child,
         const char* name);

  virtual 
  ~Filter();

  RefCountPtr<FccOperator> p_child;

};

struct PredicateFilter : public Filter<PredicateFilter>
{
  PredicateFilter(RefCountPtr<FccOperator> child,
                  fcc_decl_t func_decl);
  virtual 
  ~PredicateFilter() = default;

  fcc_decl_t m_func_decl;
};

enum class FccFilterOpType
{
  E_HAS,
  E_HAS_NOT
};

struct TagFilter : public Filter<TagFilter> 
{
  TagFilter(RefCountPtr<FccOperator> child,
            const char* tag,
            FccFilterOpType op_type,
            bool on_column = false);

  virtual 
  ~TagFilter() = default;

  char              m_tag[MAX_TYPE_NAME];
  bool              m_on_column;
  FccFilterOpType   m_op_type;
};

struct ComponentFilter : public Filter<ComponentFilter>
{
  ComponentFilter(RefCountPtr<FccOperator> child,
                  fcc_type_t component_type,
                  FccFilterOpType op_type,
                  bool on_column = false);

  virtual 
  ~ComponentFilter() = default;

  fcc_type_t          m_filter_type;
  bool                m_on_column;
  FccFilterOpType     m_op_type;
};


/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Foreach : public FccOperatorTmplt<Foreach>  
{
  Foreach(RefCountPtr<FccOperator> child, 
          const DynArray<const fcc_system_t*>& systems);

  virtual 
  ~Foreach();

  DynArray<const fcc_system_t*>      p_systems;
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
