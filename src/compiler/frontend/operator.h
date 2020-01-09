
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
  E_ID,
  E_COMPONENT,
  E_REFERENCE,
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
  char                  m_ref_name[FCC_MAX_REF_NAME];
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
  char                  m_tag[FCC_MAX_TYPE_NAME];
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
  uint32_t                  m_id;
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
subplan_init(const fcc_stmt_t* stmt, 
             fcc_subplan_t* subplan);

/**
 * \brief Destroys the given subplan
 *
 * \param subplan The subplan to release
 */
void 
subplan_release(fcc_subplan_t* subplan);

} /* furious */ 

#endif /* ifndef _FURIOUS_OPERATOR_H_ */
