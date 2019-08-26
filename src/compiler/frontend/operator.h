
#ifndef _FURIOUS_OPERATOR_H_
#define _FURIOUS_OPERATOR_H_ 

#include "../../common/common.h"
#include "../../common/refcount_ptr.h"

#include "fcc_context.h"

namespace furious
{

#define MAX_OPERATOR_NAME 256

class FccSubPlanVisitor;

enum class fcc_operator_type_t 
{
  E_SCAN,
  E_JOIN,
  E_LEFT_FILTER_JOIN,
  E_CROSS_JOIN,
  E_FETCH,
  E_TAG_FILTER,
  E_PREDICATE_FILTER,
  E_COMPONENT_FILTER,
  E_FOREACH,
  E_GATHER,
  E_CASCADING_GATHER,
};

enum class fcc_column_type_t 
{
  E_REFERENCE,
  E_COMPONENT,
  E_GLOBAL,
};

struct fcc_column_t 
{
  fcc_column_type_t     m_type;
  fcc_type_t            m_component_type;
  char                  m_ref_name[MAX_REF_NAME];
  fcc_access_mode_t     m_access_mode;
};


struct fcc_operator_t 
{
  fcc_operator_t(fcc_operator_type_t type, 
                 const char* name);

  virtual
  ~fcc_operator_t() = default;

  fcc_operator_type_t       m_type;
  char                      m_name[MAX_OPERATOR_NAME];
  const fcc_operator_t*     p_parent;
  DynArray<fcc_column_t>    m_columns;
  uint32_t                  m_id;
};

/**
 * @brief Base structure for an operator 
 */
template<typename T>
struct fcc_operator_tmplt_t : public fcc_operator_t 
{
  fcc_operator_tmplt_t(fcc_operator_type_t type,
                       const char* name);

  virtual 
  ~fcc_operator_tmplt_t() = default;
};

/**
 * @brief Scan operator. Streams components from tables
 */
struct Scan : public fcc_operator_tmplt_t<Scan> 
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
struct Join : public fcc_operator_tmplt_t<Join> 
{
  Join(RefCountPtr<fcc_operator_t> left, 
       RefCountPtr<fcc_operator_t> right);
  virtual 
  ~Join();

  RefCountPtr<fcc_operator_t> p_left;
  RefCountPtr<fcc_operator_t> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct LeftFilterJoin : public fcc_operator_tmplt_t<LeftFilterJoin> 
{
  LeftFilterJoin(RefCountPtr<fcc_operator_t> left, 
                 RefCountPtr<fcc_operator_t> right);
  virtual 
  ~LeftFilterJoin();

  RefCountPtr<fcc_operator_t> p_left;
  RefCountPtr<fcc_operator_t> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct CrossJoin : public fcc_operator_tmplt_t<CrossJoin> 
{
  CrossJoin(RefCountPtr<fcc_operator_t> left, 
            RefCountPtr<fcc_operator_t> right);
  virtual 
  ~CrossJoin();

  RefCountPtr<fcc_operator_t> p_left;
  RefCountPtr<fcc_operator_t> p_right;
};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
struct Fetch : public fcc_operator_tmplt_t<Fetch> 
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
struct Filter : public fcc_operator_tmplt_t<T> 
{
  Filter(RefCountPtr<fcc_operator_t> child,
         fcc_operator_type_t type,
         const char* name);

  virtual 
  ~Filter();

  RefCountPtr<fcc_operator_t> p_child;

};

struct PredicateFilter : public Filter<PredicateFilter>
{
  PredicateFilter(RefCountPtr<fcc_operator_t> child,
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
  TagFilter(RefCountPtr<fcc_operator_t> child,
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
  ComponentFilter(RefCountPtr<fcc_operator_t> child,
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
struct Foreach : public fcc_operator_tmplt_t<Foreach>  
{
  Foreach(RefCountPtr<fcc_operator_t> child, 
          const DynArray<const fcc_system_t*>& systems);

  virtual 
  ~Foreach();

  DynArray<const fcc_system_t*>      p_systems;
  RefCountPtr<fcc_operator_t>        p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct Gather : public fcc_operator_tmplt_t<Gather>  
{
  Gather(RefCountPtr<fcc_operator_t> ref_table,
         RefCountPtr<fcc_operator_t> child);

  virtual 
  ~Gather();

  RefCountPtr<fcc_operator_t> p_ref_table;
  RefCountPtr<fcc_operator_t> p_child;
};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
struct CascadingGather : public fcc_operator_tmplt_t<CascadingGather>  
{
  CascadingGather(RefCountPtr<fcc_operator_t> ref_table,
                  RefCountPtr<fcc_operator_t> child);

  virtual 
  ~CascadingGather();

  RefCountPtr<fcc_operator_t> p_ref_table;
  RefCountPtr<fcc_operator_t> p_child;
};
  
} /* furious */ 

#include "operator.inl"


#endif /* ifndef _FURIOUS_OPERATOR_H_ */
