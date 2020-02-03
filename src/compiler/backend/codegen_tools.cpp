

#include "../common/types.h"
#include "../common/str_builder.h"
#include "consumer.h"
#include "producer.h"
#include "../frontend/operator.h"
#include "driver.h"

#include "codegen.h"
#include "codegen_tools.h"

#include <string.h>

// autogen includes
#include "autogen//fcc_depcy_array.h"


static void 
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      fcc_depcy_array_t* deps);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const fcc_operator_t* fcc_operator);

static void
fcc_deps_extr_extract_scan(fcc_deps_extr_t* deps_extr,
                           const fcc_operator_t* scan);

static void
fcc_deps_extr_extract_foreach(fcc_deps_extr_t* deps_extr,
                              const fcc_operator_t* foreach);

static void
fcc_deps_extr_extract_join(fcc_deps_extr_t* deps_extr,
                           const fcc_operator_t* join);

static void
fcc_deps_extr_extract_cross_join(fcc_deps_extr_t* deps_extr,
                                 const fcc_operator_t* cross_join);

static void
fcc_deps_extr_extract_leftfilter_join(fcc_deps_extr_t* deps_extr,
                                      const fcc_operator_t* left_filter_join);

static void
fcc_deps_extr_extract_gather(fcc_deps_extr_t* deps_extr,
                             const fcc_operator_t* gather);

static void
fcc_deps_extr_extract_cascading_gather(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* casc_gather);

static void
fcc_deps_extr_extract_fetch(fcc_deps_extr_t* deps_extr,
                            const fcc_operator_t* fetch);

static void
fcc_deps_extr_extract_tag_filter(fcc_deps_extr_t* deps_extr,
                                 const fcc_operator_t* tag_filter);

static void
fcc_deps_extr_extract_predicate_filter(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* predicate_filter);

static void
fcc_deps_extr_extract_component_filter(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* component_filter);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
fcc_deps_extr_init(fcc_deps_extr_t* deps_extr)
{
  *deps_extr = {};
  deps_extr->m_incs = cchar_ptr_array_init();
  deps_extr->m_decls = fcc_decl_array_init();
}

void
fcc_deps_extr_release(fcc_deps_extr_t* deps_extr)
{
  const uint32_t num_includes = deps_extr->m_incs.m_count;
  const char** incs = deps_extr->m_incs.m_data;
  for (uint32_t i = 0; i < num_includes; ++i) 
  {
    delete [] incs[i];
  }
  fcc_decl_array_release(&deps_extr->m_decls);
  cchar_ptr_array_release(&deps_extr->m_incs);
}
void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      fcc_depcy_array_t* deps_array)
{
  uint32_t ndeps = deps_array->m_count;
  fcc_depcy_t* deps = deps_array->m_data;
  for(uint32_t i = 0; i < ndeps; ++i) 
  {
    fcc_depcy_t* dep = &deps[i];
    if(fcc_decl_is_valid(dep->m_decl))
    {
      bool is_var_or_struct = fcc_decl_is_variable_or_struct(dep->m_decl);
      bool is_function = fcc_decl_is_function(dep->m_decl);
      bool is_member = fcc_decl_is_member(dep->m_decl);
      if( is_var_or_struct || (is_function && !is_member))
      {
        bool found = false;
        const uint32_t num_decls = deps_extr->m_decls.m_count;
        fcc_decl_t* decls = deps_extr->m_decls.m_data;
        for (uint32_t j = 0; j < num_decls; ++j) 
        {
          if(fcc_decl_is_same(dep->m_decl, decls[j]))
          {
            found = true;
            break;
          }
        }

        if(!found)
        {
          fcc_decl_array_append(&deps_extr->m_decls, &dep->m_decl);
        }
      }
    } 
    else 
    {
      bool found = false;
      const uint32_t num_includes = deps_extr->m_incs.m_count;
      const char** incs = deps_extr->m_incs.m_data;
      for (uint32_t j = 0; j < num_includes; ++j) 
      {
        if(strcmp(incs[j], dep->m_include_file) == 0)
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        char* buffer = new char[FCC_MAX_INCLUDE_PATH_LENGTH];
        FDB_COPY_AND_CHECK_STR(buffer, dep->m_include_file, FCC_MAX_INCLUDE_PATH_LENGTH);
        const char* ptr = buffer;
        cchar_ptr_array_append(&deps_extr->m_incs, &ptr);
      }
    }
  }
}

void 
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const fcc_operator_t* fcc_operator)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      fcc_deps_extr_extract_scan(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_deps_extr_extract_foreach(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_deps_extr_extract_join(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_deps_extr_extract_leftfilter_join(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_deps_extr_extract_cross_join(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_deps_extr_extract_fetch(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_deps_extr_extract_gather(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_deps_extr_extract_cascading_gather(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_deps_extr_extract_tag_filter(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_deps_extr_extract_predicate_filter(deps_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_deps_extr_extract_component_filter(deps_extr, fcc_operator);
      break;
  };
}

void 
fcc_deps_extr_extract_foreach(fcc_deps_extr_t* deps_extr,
                              const fcc_operator_t* foreach)
{
  const fcc_system_t* system = foreach->m_foreach.p_system;
  fcc_depcy_array_t array = fcc_depcy_array_init();
  fcc_type_dependencies(system->m_system_type, &array);
  fcc_deps_extr_extract(deps_extr,
                        &array);
  fcc_depcy_array_release(&array);
  const uint32_t child_idx = foreach->m_foreach.m_child;
  fcc_operator_t* op =&foreach->p_subplan->m_nodes[child_idx];
  fcc_deps_extr_extract(deps_extr, op);
}

void 
fcc_deps_extr_extract_scan(fcc_deps_extr_t* deps_extr, 
                           const fcc_operator_t* scan) 
{
  if(scan->m_columns[0].m_type == fcc_column_type_t::E_COMPONENT)
  {
    fcc_depcy_array_t array = fcc_depcy_array_init();
    fcc_type_dependencies(scan->m_columns[0].m_component_type, &array);
    fcc_deps_extr_extract(deps_extr,
                          &array);
    fcc_depcy_array_release(&array);
  }
} 

void
fcc_deps_extr_extract_join(fcc_deps_extr_t* deps_extr, 
                           const fcc_operator_t* join) 
{
  fcc_subplan_t* subplan = join->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[join->m_join.m_left]);
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[join->m_join.m_right]);
}

void
fcc_deps_extr_extract_leftfilter_join(fcc_deps_extr_t* deps_extr,
                                      const fcc_operator_t* left_filter_join) 
{
  fcc_subplan_t* subplan = left_filter_join->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_left]);
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_right]);
}

void
fcc_deps_extr_extract_cross_join(fcc_deps_extr_t* deps_extr,
                                 const fcc_operator_t* cross_join) 
{
  fcc_subplan_t* subplan = cross_join->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[cross_join->m_cross_join.m_left]);
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[cross_join->m_cross_join.m_right]);
}

void
fcc_deps_extr_extract_fetch(fcc_deps_extr_t* deps_extr,
                            const fcc_operator_t* fetch) 
{
  if(fetch->m_columns[0].m_type == fcc_column_type_t::E_GLOBAL)
  {
    fcc_depcy_array_t array = fcc_depcy_array_init();
    fcc_type_dependencies(fetch->m_columns[0].m_component_type, &array);
    fcc_deps_extr_extract(deps_extr,
                          &array);
    fcc_depcy_array_release(&array);
  }
}

void 
fcc_deps_extr_extract_tag_filter(fcc_deps_extr_t* deps_extr, 
                                 const fcc_operator_t* tag_filter) 
{
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[tag_filter->m_tag_filter.m_child]);
}

void
fcc_deps_extr_extract_component_filter(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* component_filter) 
{
  fcc_subplan_t* subplan = component_filter->p_subplan;
  fcc_depcy_array_t array = fcc_depcy_array_init();
  fcc_type_dependencies(component_filter->m_component_filter.m_filter_type, &array);
  fcc_deps_extr_extract(deps_extr,
                        &array);
  fcc_depcy_array_release(&array);
  fcc_deps_extr_extract(deps_extr, &subplan[component_filter->m_component_filter.m_child]);
}

void
fcc_deps_extr_extract_predicate_filter(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* predicate_filter) 
{
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  fcc_depcy_array_t array = fcc_depcy_array_init();\
  fcc_decl_dependencies(predicate_filter->m_predicate_filter.m_func_decl, &array);\
  fcc_deps_extr_extract(deps_extr,\
                        &array);\
  fcc_depcy_array_release(&array);
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[predicate_filter->m_predicate_filter.m_child]);
}

void
fcc_deps_extr_extract_gather(fcc_deps_extr_t* deps_extr, 
                             const fcc_operator_t* gather) 
{
  fcc_subplan_t* subplan = gather->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[gather->m_gather.m_child]);
}

void
fcc_deps_extr_extract_cascading_gather(fcc_deps_extr_t* deps_extr, 
                                       const fcc_operator_t* casc_gather) 
{
  fcc_subplan_t* subplan = casc_gather->p_subplan;
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[casc_gather->m_cascading_gather.m_child]);
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const fcc_subplan_t* subplan)
{
  fcc_deps_extr_extract(deps_extr, &subplan->m_nodes[subplan->m_root]);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const fcc_operator_t* fcc_operator);

static void
fcc_vars_extr_extract_scan(fcc_vars_extr_t* vars_extr,
                           const fcc_operator_t* scan);

static void
fcc_vars_extr_extract_foreach(fcc_vars_extr_t* vars_extr,
                              const fcc_operator_t* scan);

static void
fcc_vars_extr_extract_join(fcc_vars_extr_t* vars_extr,
                           const fcc_operator_t* join);

static void
fcc_vars_extr_extract_cross_join(fcc_vars_extr_t* vars_extr,
                                 const fcc_operator_t* cross_join);

static void
fcc_vars_extr_extract_leftfilter_join(fcc_vars_extr_t* vars_extr,
                                      const fcc_operator_t* left_filter_join);

static void
fcc_vars_extr_extract_gather(fcc_vars_extr_t* vars_extr,
                             const fcc_operator_t* gather);

static void
fcc_vars_extr_extract_cascading_gather(fcc_vars_extr_t* vars_extr,
                                       const fcc_operator_t* casc_gather);

static void
fcc_vars_extr_extract_fetch(fcc_vars_extr_t* vars_extr,
                            const fcc_operator_t* fetch);

static void
fcc_vars_extr_extract_tag_filter(fcc_vars_extr_t* vars_extr,
                                 const fcc_operator_t* tag_filter);

static void
fcc_vars_extr_extract_predicate_filter(fcc_vars_extr_t* vars_extr,
                                       const fcc_operator_t* predicate_filter);

static void
fcc_vars_extr_extract_component_filter(fcc_vars_extr_t* vars_extr,
                                       const fcc_operator_t* component_filter);

void
fcc_vars_extr_init(fcc_vars_extr_t* vars_extr)
{
  memset(vars_extr, 0, sizeof(fcc_vars_extr_t));
  vars_extr->m_comp_decls = fcc_decl_array_init();
  vars_extr->m_tags = cchar_ptr_array_init();
  vars_extr->m_refs = cchar_ptr_array_init();
  vars_extr->m_comps = cchar_ptr_array_init();
}

void
fcc_vars_extr_release(fcc_vars_extr_t* vars_extr)
{
  const uint32_t ncomps = vars_extr->m_comps.m_count;
  const char** comps = vars_extr->m_comps.m_data;
  for (uint32_t i = 0; i < ncomps; ++i) 
  {
    delete [] comps[i];
  }

  const uint32_t ntags = vars_extr->m_tags.m_count;
  const char** tags = vars_extr->m_tags.m_data;
  for (uint32_t i = 0; i < ntags; ++i) 
  {
    delete [] tags[i];
  }

  const uint32_t nrefs = vars_extr->m_refs.m_count;
  const char** refs = vars_extr->m_refs.m_data;
  for (uint32_t i = 0; i < nrefs; ++i) 
  {
    delete [] refs[i];
  }

  cchar_ptr_array_release(&vars_extr->m_tags);
  cchar_ptr_array_release(&vars_extr->m_refs);
  cchar_ptr_array_release(&vars_extr->m_comps);
}

void 
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const fcc_operator_t* fcc_operator)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      fcc_vars_extr_extract_scan(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_vars_extr_extract_foreach(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_vars_extr_extract_join(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_vars_extr_extract_leftfilter_join(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_vars_extr_extract_cross_join(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_vars_extr_extract_fetch(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_vars_extr_extract_gather(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_vars_extr_extract_cascading_gather(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_vars_extr_extract_tag_filter(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_vars_extr_extract_predicate_filter(vars_extr, fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_vars_extr_extract_component_filter(vars_extr, fcc_operator);
      break;
  };
}

void 
fcc_vars_extr_extract_foreach(fcc_vars_extr_t* vars_extr, 
                     const fcc_operator_t* foreach)
{
  fcc_subplan_t* subplan = foreach->p_subplan;
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[foreach->m_foreach.m_child]);
}

void 
fcc_vars_extr_extract_scan(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* scan) 
{
  if(scan->m_columns[0].m_type == fcc_column_type_t::E_COMPONENT)
  {
    char tmp[FCC_MAX_TYPE_NAME];
    fcc_type_name(scan->m_columns[0].m_component_type, 
                  tmp, 
                  FCC_MAX_TYPE_NAME);

    const uint32_t ncomps = vars_extr->m_comps.m_count;
    const char** comps = vars_extr->m_comps.m_data;
    bool found = false;
    for(uint32_t i = 0; i < ncomps; ++i)
    {
      if(strcmp(tmp, comps[i]) == 0)
      {
        found = true;
        break;
      }
    }
    if(!found)
    {
      char* buffer = new char [FCC_MAX_TYPE_NAME];
      FDB_COPY_AND_CHECK_STR(buffer, tmp, FCC_MAX_TYPE_NAME);
      const char* ptr = buffer;
      cchar_ptr_array_append(&vars_extr->m_comps, &ptr);
      fcc_decl_t decl;
      fcc_type_decl(scan->m_columns[0].m_component_type, &decl);
      bool found = false;
      const uint32_t ndecls = vars_extr->m_comp_decls.m_count;
      fcc_decl_t* cdecls = vars_extr->m_comp_decls.m_data;
      for (uint32_t i = 0; i < ndecls; ++i) 
      {
        if(fcc_decl_is_same(cdecls[i], decl))
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        fcc_decl_array_append(&vars_extr->m_comp_decls, &decl);
      }
    }
  }
  else
  {
    bool found = false;
    const uint32_t nrefs = vars_extr->m_refs.m_count;
    const char**   refs = vars_extr->m_refs.m_data;
    for (uint32_t i = 0; i < nrefs; ++i) 
    {
      if(strcmp(refs[i], scan->m_columns[0].m_ref_name) == 0)
      {
        found = true;
        break;
      }
    }
    if (!found) 
    {
      char* buffer = new char[FCC_MAX_REF_NAME];
      FDB_COPY_AND_CHECK_STR(buffer, scan->m_columns[0].m_ref_name, FCC_MAX_REF_NAME);
      const char* ptr = buffer;
      cchar_ptr_array_append(&vars_extr->m_refs, &ptr);
    }
  }
}

void
fcc_vars_extr_extract_join(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* join) 
{
  fcc_subplan_t* subplan = join->p_subplan;
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[join->m_join.m_left]);
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[join->m_join.m_right]);
}

void
fcc_vars_extr_extract_leftfilter_join(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* left_filter_join) 
{
  fcc_subplan_t* subplan = left_filter_join->p_subplan;
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_left]);
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_right]);
}

void
fcc_vars_extr_extract_cross_join(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* cross_join) 
{

  fcc_subplan_t* subplan = cross_join->p_subplan;
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[cross_join->m_cross_join.m_left]);
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[cross_join->m_cross_join.m_right]);
}

void
fcc_vars_extr_extract_fetch(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* fetch)
{
  fcc_decl_t decl;
  fcc_type_decl(fetch->m_columns[0].m_component_type, &decl);
  bool found = false;
  const uint32_t ndecls = vars_extr->m_comp_decls.m_count;
  fcc_decl_t* decls = vars_extr->m_comp_decls.m_data;
  for (uint32_t i = 0; i < ndecls; ++i) 
  {
    if(fcc_decl_is_same(decl, decls[i]))
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    fcc_decl_array_append(&vars_extr->m_comp_decls, &decl);
  }
}

void 
fcc_vars_extr_extract_tag_filter(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* tag_filter) 
{
  char* buffer = new char[FCC_MAX_TAG_NAME];
  FDB_COPY_AND_CHECK_STR(buffer, tag_filter->m_tag_filter.m_tag, FCC_MAX_TAG_NAME)
  const char* ptr = buffer;
  cchar_ptr_array_append(&vars_extr->m_tags, &ptr);
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  fcc_vars_extr_extract(vars_extr, 
                       &subplan->m_nodes[tag_filter->m_tag_filter.m_child]);
}

void
fcc_vars_extr_extract_component_filter(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* component_filter) 
{
  char tmp[FCC_MAX_TYPE_NAME];
  fcc_type_name(component_filter->m_component_filter.m_filter_type, 
                tmp, 
                FCC_MAX_TYPE_NAME);

  bool found = false;
  const uint32_t ncomps = vars_extr->m_comps.m_count;
  const char** comps = vars_extr->m_comps.m_data;
  for(uint32_t i = 0; i < ncomps; ++i)
  {
    if(strcmp(comps[i], tmp) == 0)
    {
      found = true;
    }
  }

  if(!found)
  {
    char* buffer = new char[FCC_MAX_TYPE_NAME];
    FDB_COPY_AND_CHECK_STR(buffer, tmp, FCC_MAX_TYPE_NAME);
    const char* ptr = buffer;
    cchar_ptr_array_append(&vars_extr->m_comps, &ptr);
    fcc_decl_t decl;
    fcc_type_decl(component_filter->m_component_filter.m_filter_type, &decl);
    fcc_decl_array_append(&vars_extr->m_comp_decls, &decl);
    fcc_subplan_t* subplan = component_filter->p_subplan;
    fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[component_filter->m_component_filter.m_child]);
  }
}

void
fcc_vars_extr_extract_predicate_filter(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* predicate_filter) 
{
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  fcc_vars_extr_extract(vars_extr, 
                        &subplan->m_nodes[predicate_filter->m_predicate_filter.m_child]);
}

void
fcc_vars_extr_extract_gather(fcc_vars_extr_t* vars_extr,
                      const fcc_operator_t* gather)
{
  fcc_subplan_t* subplan = gather->p_subplan;
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[gather->m_gather.m_ref_table]);
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[gather->m_gather.m_child]);
}

void
fcc_vars_extr_extract_cascading_gather(fcc_vars_extr_t* vars_extr,
                      const fcc_operator_t* casc_gather)
{
  fcc_subplan_t* subplan = casc_gather->p_subplan;
  fcc_vars_extr_extract(vars_extr,&subplan->m_nodes[casc_gather->m_cascading_gather.m_ref_table]);
  fcc_vars_extr_extract(vars_extr,&subplan->m_nodes[casc_gather->m_cascading_gather.m_child]);
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const fcc_subplan_t* subplan)
{
  fcc_vars_extr_extract(vars_extr, &subplan->m_nodes[subplan->m_root]);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static
void tolower(char* str)
{
  uint32_t i = 0; 
  while (str[i] != '\0') 
  {
    if (str[i] >= 'A' && str[i] <= 'Z') 
    {
      str[i] = str[i] + 32;
    }
    i++;
  }
}

static
void replace_special(char* str)
{
  char special_chars[] = {':','<','>',',','.',' '};

  for(uint32_t i = 0; i < sizeof(special_chars); ++i)
  {
    uint32_t j = 0;
    while(str[j] != '\0')
    {
      if(str[j] == special_chars[i])
      {
        str[j] = '_';
      }
      ++j;
    }
  }
}

void
sanitize_name(char* str)
{
  tolower(str);
  replace_special(str);
}

uint32_t
generate_table_name(const char* type_name, 
                    char *buffer,
                    uint32_t buffer_length,
                    const fcc_operator_t* op)
{
  char tmp[FCC_MAX_TYPE_NAME];
  FDB_COPY_AND_CHECK_STR(tmp, type_name, FCC_MAX_TYPE_NAME);
  sanitize_name(tmp);

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "table_%s", tmp);

  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder,"_%u",op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_temp_table_name(const char* type_name, 
                         char * buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  char tmp[FCC_MAX_TABLE_VARNAME];
  generate_table_name(type_name, 
                      tmp,
                      FCC_MAX_TABLE_VARNAME,
                      op);

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "temp_%s", tmp);

  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_ref_table_name(const char* ref_name, 
                        char* buffer,
                        uint32_t buffer_length,
                        const fcc_operator_t* op)
{
  char tmp[FCC_MAX_TABLE_VARNAME];
  generate_table_name(ref_name, 
                      tmp,
                      FCC_MAX_TABLE_VARNAME,
                      op);

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "ref_%s", tmp);
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const fcc_operator_t* op)
{

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "tagged_%s", tag_name);
  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "iter_%s", table_name);
  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const fcc_operator_t* op)
{
  char tmp[FCC_MAX_TYPE_NAME];
  FDB_COPY_AND_CHECK_STR(tmp, type_name, FCC_MAX_TYPE_NAME);
  sanitize_name(tmp);

  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "block_%s", tmp);
  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_cluster_name(const fcc_operator_t* op,
                      char* buffer,
                      uint32_t buffer_length)
{
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "cluster_%u", op->m_id);
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  char tmp[FCC_MAX_REF_NAME];
  FDB_COPY_AND_CHECK_STR(tmp, ref_name, FCC_MAX_REF_NAME);
  sanitize_name(tmp);
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "ref_%s_groups_%u", tmp, op->m_id);
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_hashtable_name(const fcc_operator_t* op,
                        char* buffer,
                        uint32_t buffer_length)
{
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "hashtable_%u",op->m_id);
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_system_wrapper_name(const char* system_name,
                             uint32_t system_id,
                             char* buffer,
                             uint32_t buffer_length,
                             const fcc_operator_t* op)
{
  char base_name[FCC_MAX_TYPE_NAME];
  FDB_COPY_AND_CHECK_STR(base_name, system_name, FCC_MAX_TYPE_NAME);
  tolower(base_name);
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "%s_%d",base_name, system_id  );
  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_global_name(const char* type_name, 
                     char* buffer,
                     uint32_t buffer_length,
                     const fcc_operator_t* op)
{
  char tmp[FCC_MAX_TYPE_NAME];
  strncpy(tmp, type_name, buffer_length);
  FDB_COPY_AND_CHECK_STR(tmp, type_name, FCC_MAX_TYPE_NAME);
  sanitize_name(tmp);
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "global_%s", tmp);
  if(op != nullptr)
  {
    fdb_str_builder_append(&str_builder, "_%u", op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FDB_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  fdb_str_builder_release(&str_builder);
  return length;
}

