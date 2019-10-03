
#ifndef _FURIOUS_OPERATOR_H_
#define _FURIOUS_OPERATOR_H_ 

#include "../../common/common.h"
#include "fcc_context.h"

namespace furious
{

#define _FURIOUS_COMPILER_INVALID_ID 0xffffffff
#define _FURIOUS_COMPILER_MAX_OPERATOR_NAME 256

struct fcc_subplan_t;

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

enum class fcc_filter_op_type_t
{
  E_HAS,
  E_HAS_NOT
};

struct fcc_column_t 
{
  fcc_column_type_t     m_type;
  fcc_type_t            m_component_type;
  char                  m_ref_name[MAX_REF_NAME];
  fcc_access_mode_t     m_access_mode;
};

struct fcc_scan_t
{
};

struct fcc_join_t
{
  uint32_t m_left;
  uint32_t m_right;
};

struct fcc_cross_join_t
{
  uint32_t m_left;
  uint32_t m_right;
};

struct fcc_leftfilter_join_t
{
  uint32_t m_left;
  uint32_t m_right;
};

struct fcc_fetch_t
{
  fcc_type_t m_global_type;
};

struct fcc_tag_filter_t
{
  uint32_t              m_child;
  char                  m_tag[MAX_TYPE_NAME];
  bool                  m_on_column;
  fcc_filter_op_type_t  m_op_type;
};

struct fcc_predicate_filter_t
{
  uint32_t m_child;
  fcc_decl_t m_func_decl;
};

struct fcc_component_filter_t
{
  uint32_t m_child;
  fcc_type_t                m_filter_type;
  bool                      m_on_column;
  fcc_filter_op_type_t      m_op_type;
};

struct fcc_foreach_t
{
  const fcc_system_t*   p_system;
  uint32_t              m_child;
};

struct fcc_gather_t
{
  uint32_t          m_ref_table;
  uint32_t          m_child;
};

struct fcc_cascading_gather_t
{
  uint32_t          m_ref_table;
  uint32_t          m_child;
};

struct fcc_operator_t 
{
  fcc_operator_type_t       m_type;
  fcc_subplan_t*            p_subplan;
  uint32_t                  m_parent;
  uint32_t                  m_id;
  char                      m_name[_FURIOUS_COMPILER_MAX_OPERATOR_NAME];
  DynArray<fcc_column_t>    m_columns;
  union 
  {
    fcc_scan_t              m_scan;
    fcc_fetch_t             m_fetch;
    fcc_join_t              m_join;
    fcc_leftfilter_join_t   m_leftfilter_join;
    fcc_cross_join_t        m_cross_join;
    fcc_tag_filter_t        m_tag_filter;
    fcc_predicate_filter_t  m_predicate_filter;
    fcc_component_filter_t  m_component_filter;
    fcc_foreach_t           m_foreach;
    fcc_gather_t            m_gather;
    fcc_cascading_gather_t  m_cascading_gather;
  };
};

uint32_t 
create_scan(fcc_subplan_t* subplan, 
            fcc_type_t component,
            fcc_access_mode_t access_mode);

uint32_t 
create_scan(fcc_subplan_t* subplan, 
            const char* ref_name);

uint32_t 
create_fetch(fcc_subplan_t* subplan, 
             fcc_type_t global_type,
             fcc_access_mode_t access_mode);

uint32_t 
create_join(fcc_subplan_t* subplan,
            uint32_t left,
            uint32_t right);

uint32_t 
create_leftfilter_join(fcc_subplan_t* subplan,
                       uint32_t left,
                       uint32_t right);
uint32_t 
create_cross_join(fcc_subplan_t* subplan,
                       uint32_t left,
                       uint32_t right);

uint32_t 
create_predicate_filter(fcc_subplan_t* subplan,
                        uint32_t child,
                        fcc_decl_t filter_func);

uint32_t 
create_tag_filter(fcc_subplan_t* subplan,
                  uint32_t child,
                  const char* tag,
                  fcc_filter_op_type_t op_type,
                  bool on_column = false);

uint32_t 
create_component_filter(fcc_subplan_t* subplan,
                        uint32_t child,
                        fcc_type_t component,
                        fcc_filter_op_type_t op_type,
                        bool on_column = false);

uint32_t
create_foreach(fcc_subplan_t* subplan,
               uint32_t child,
               const fcc_system_t* system);

uint32_t
create_gather(fcc_subplan_t* subplan,
              uint32_t child,
              uint32_t ref_table);

uint32_t
create_cascading_gather(fcc_subplan_t* subplan,
                        uint32_t child,
                        uint32_t ref_table);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct fcc_subplan_t 
{
  uint32_t                  m_root;
  DynArray<fcc_operator_t>  m_nodes;
  bool                      m_requires_sync;
};


/**
 * @brief Bootstraps an initial plan from an execution info
 *
 * @param exec_info The Exec info to bootstrap the plan from
 * @param subplan   The Subplan to initialize 
 *
 */
void 
init_subplan(const fcc_stmt_t* stmt, 
             fcc_subplan_t* subplan);

/**
 * \brief Destroys the given subplan
 *
 * \param subplan The subplan to release
 */
void 
release_subplan(fcc_subplan_t* subplan);

/**
 * @brief Base structure for an operator 
 */
//template<typename T>
//struct fcc_operator_tmplt_t : public fcc_operator_t 
//{
//  fcc_operator_tmplt_t(fcc_operator_type_t type,
//                       const char* name);
//
//  virtual 
//  ~fcc_operator_tmplt_t() = default;
//};

/**
 * @brief Scan operator. Streams components from tables
 */
//struct Scan : public fcc_operator_tmplt_t<Scan> 
//{
//  explicit Scan(const char* ref_name);
//
//  explicit Scan(fcc_type_t component,
//                fcc_access_mode_t access_mode);
//
//  virtual 
//  ~Scan() = default;
//
//};

/**
 * @brief Join operator. Joins two tables by entity id
 */
//struct Join : public fcc_operator_tmplt_t<Join> 
//{
//  Join(RefCountPtr<fcc_operator_t> left, 
//       RefCountPtr<fcc_operator_t> right);
//  virtual 
//  ~Join();
//
//  RefCountPtr<fcc_operator_t> p_left;
//  RefCountPtr<fcc_operator_t> p_right;
//};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
//struct LeftFilterJoin : public fcc_operator_tmplt_t<LeftFilterJoin> 
//{
//  LeftFilterJoin(RefCountPtr<fcc_operator_t> left, 
//                 RefCountPtr<fcc_operator_t> right);
//  virtual 
//  ~LeftFilterJoin();
//
//  RefCountPtr<fcc_operator_t> p_left;
//  RefCountPtr<fcc_operator_t> p_right;
//};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
//struct CrossJoin : public fcc_operator_tmplt_t<CrossJoin> 
//{
//  CrossJoin(RefCountPtr<fcc_operator_t> left, 
//            RefCountPtr<fcc_operator_t> right);
//  virtual 
//  ~CrossJoin();
//
//  RefCountPtr<fcc_operator_t> p_left;
//  RefCountPtr<fcc_operator_t> p_right;
//};

/**
 * \brief LeftFilterJoin. Joins two tables but only returns those rows from the
 * left table. Do not confuse with Left Joins.
 */
//struct Fetch : public fcc_operator_tmplt_t<Fetch> 
//{
//  Fetch(fcc_type_t global_type,
//        fcc_access_mode_t access_mode);
//
//  virtual 
//  ~Fetch();
//
//  fcc_type_t m_global_type;
//};

/**
 * @brief Filter operator. Filter components by entity tags
 */
//template<typename T>
//struct Filter : public fcc_operator_tmplt_t<T> 
//{
//  Filter(RefCountPtr<fcc_operator_t> child,
//         fcc_operator_type_t type,
//         const char* name);
//
//  virtual 
//  ~Filter();
//
//  RefCountPtr<fcc_operator_t> p_child;
//
//};

//struct PredicateFilter : public Filter<PredicateFilter>
//{
//  PredicateFilter(RefCountPtr<fcc_operator_t> child,
//                  fcc_decl_t func_decl);
//  virtual 
//  ~PredicateFilter() = default;
//
//  fcc_decl_t m_func_decl;
//};

//struct TagFilter : public Filter<TagFilter> 
//{
//  TagFilter(RefCountPtr<fcc_operator_t> child,
//            const char* tag,
//            FccFilterOpType op_type,
//            bool on_column = false);
//
//  virtual 
//  ~TagFilter() = default;
//
//  char              m_tag[MAX_TYPE_NAME];
//  bool              m_on_column;
//  FccFilterOpType   m_op_type;
//};

//struct ComponentFilter : public Filter<ComponentFilter>
//{
//  ComponentFilter(RefCountPtr<fcc_operator_t> child,
//                  fcc_type_t component_type,
//                  FccFilterOpType op_type,
//                  bool on_column = false);
//
//  virtual 
//  ~ComponentFilter() = default;
//
//  fcc_type_t          m_filter_type;
//  bool                m_on_column;
//  FccFilterOpType     m_op_type;
//};


/**
 * @brief Foreach operator. Applies a system for each table row
 */
//struct Foreach : public fcc_operator_tmplt_t<Foreach>  
//{
//  Foreach(RefCountPtr<fcc_operator_t> child, 
//          const DynArray<const fcc_system_t*>& systems);
//
//  virtual 
//  ~Foreach();
//
//  DynArray<const fcc_system_t*>      p_systems;
//  RefCountPtr<fcc_operator_t>        p_child;
//};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
//struct Gather : public fcc_operator_tmplt_t<Gather>  
//{
//  Gather(RefCountPtr<fcc_operator_t> ref_table,
//         RefCountPtr<fcc_operator_t> child);
//
//  virtual 
//  ~Gather();
//
//  RefCountPtr<fcc_operator_t> p_ref_table;
//  RefCountPtr<fcc_operator_t> p_child;
//};

/**
 * @brief Foreach operator. Applies a system for each table row
 */
//struct CascadingGather : public fcc_operator_tmplt_t<CascadingGather>  
//{
//  CascadingGather(RefCountPtr<fcc_operator_t> ref_table,
//                  RefCountPtr<fcc_operator_t> child);
//
//  virtual 
//  ~CascadingGather();
//
//  RefCountPtr<fcc_operator_t> p_ref_table;
//  RefCountPtr<fcc_operator_t> p_child;
//};
  
} /* furious */ 

//#include "operator.inl"


#endif /* ifndef _FURIOUS_OPERATOR_H_ */
