

#include "../common/types.h"
#include "../common/str_builder.h"
#include "../common/dyn_array.h"
#include "consumer.h"
#include "producer.h"
#include "../frontend/operator.h"
#include "driver.h"

#include "codegen.h"
#include "codegen_tools.h"

#include <string.h>

namespace furious 
{

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const DynArray<Dependency>& deps);

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
}

void
fcc_deps_extr_release(fcc_deps_extr_t* deps_extr)
{
  const uint32_t num_includes = deps_extr->m_include_files.size();
  for (uint32_t i = 0; i < num_includes; ++i) 
  {
    delete [] deps_extr->m_include_files[i];
  }
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const DynArray<Dependency>& deps)
{
  for(uint32_t i = 0; i < deps.size(); ++i) 
  {
    const Dependency& dep = deps[i];
    if(fcc_decl_is_valid(dep.m_decl))
    {
      bool is_var_or_struct = fcc_decl_is_variable_or_struct(dep.m_decl);
      bool is_function = fcc_decl_is_function(dep.m_decl);
      bool is_member = fcc_decl_is_member(dep.m_decl);
      if( is_var_or_struct || (is_function && !is_member))
      {
        bool found = false;
        const uint32_t num_decls = deps_extr->m_declarations.size();
        for (uint32_t j = 0; j < num_decls; ++j) 
        {
          if(fcc_decl_is_same(dep.m_decl, deps_extr->m_declarations[j]))
          {
            found = true;
            break;
          }
        }

        if(!found)
        {
          deps_extr->m_declarations.append(dep.m_decl);
        }
      }
    } 
    else 
    {
      bool found = false;
      const uint32_t num_includes = deps_extr->m_include_files.size();
      for (uint32_t j = 0; j < num_includes; ++j) 
      {
        if(strcmp(deps_extr->m_include_files[j], dep.m_include_file) == 0)
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        char* buffer = new char[MAX_INCLUDE_PATH_LENGTH];
        FURIOUS_COPY_AND_CHECK_STR(buffer, dep.m_include_file, MAX_INCLUDE_PATH_LENGTH);
        deps_extr->m_include_files.append(buffer);
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
  fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(system->m_system_type));
  fcc_deps_extr_extract(deps_extr, &foreach->p_subplan->m_nodes[foreach->m_foreach.m_child]);
}

void 
fcc_deps_extr_extract_scan(fcc_deps_extr_t* deps_extr, 
                           const fcc_operator_t* scan) 
{
  if(scan->m_columns[0].m_type == fcc_column_type_t::E_COMPONENT)
  {
    fcc_deps_extr_extract(deps_extr,
                          fcc_type_dependencies(scan->m_columns[0].m_component_type));
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
    fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(fetch->m_columns[0].m_component_type));
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
  fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(component_filter->m_component_filter.m_filter_type));
  fcc_deps_extr_extract(deps_extr, &subplan[component_filter->m_component_filter.m_child]);
}

void
fcc_deps_extr_extract_predicate_filter(fcc_deps_extr_t* deps_extr,
                                       const fcc_operator_t* predicate_filter) 
{
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  fcc_deps_extr_extract(deps_extr, fcc_decl_dependencies(predicate_filter->m_predicate_filter.m_func_decl));
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
}

void
fcc_vars_extr_release(fcc_vars_extr_t* vars_extr)
{
  const uint32_t num_components = vars_extr->m_components.size();
  for (uint32_t i = 0; i < num_components; ++i) 
  {
    delete [] vars_extr->m_components[i];
  }

  const uint32_t num_tags = vars_extr->m_tags.size();
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] vars_extr->m_tags[i];
  }

  const uint32_t num_refs = vars_extr->m_references.size();
  for (uint32_t i = 0; i < num_refs; ++i) 
  {
    delete [] vars_extr->m_references[i];
  }
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
    char tmp[MAX_TYPE_NAME];
    fcc_type_name(scan->m_columns[0].m_component_type, 
                  tmp, 
                  MAX_TYPE_NAME);

    const uint32_t num_components = vars_extr->m_components.size();
    bool found = false;
    for(uint32_t i = 0; i < num_components; ++i)
    {
      if(strcmp(tmp, vars_extr->m_components[i]) == 0)
      {
        found = true;
        break;
      }
    }
    if(!found)
    {
      char* buffer = new char [MAX_TYPE_NAME];
      strncpy(buffer, tmp, MAX_TYPE_NAME);
      vars_extr->m_components.append(buffer);
      fcc_decl_t decl;
      fcc_type_decl(scan->m_columns[0].m_component_type, &decl);
      bool found = false;
      const uint32_t num_decls = vars_extr->m_component_decls.size();
      for (uint32_t i = 0; i < num_decls; ++i) 
      {
        if(fcc_decl_is_same(vars_extr->m_component_decls[i], decl))
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        vars_extr->m_component_decls.append(decl);
      }
    }
  }
  else
  {
    bool found = false;
    const uint32_t num_references = vars_extr->m_references.size();
    for (uint32_t i = 0; i < num_references; ++i) 
    {
      if(strcmp(vars_extr->m_references[i], scan->m_columns[0].m_ref_name) == 0)
      {
        found = true;
        break;
      }
    }
    if (!found) 
    {
      char* buffer = new char[MAX_REF_NAME];
      strncpy(buffer, scan->m_columns[0].m_ref_name, MAX_REF_NAME);
      vars_extr->m_references.append(buffer);
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
  const uint32_t num_decls = vars_extr->m_component_decls.size();
  for (uint32_t i = 0; i < num_decls; ++i) 
  {
    if(fcc_decl_is_same(decl, vars_extr->m_component_decls[i]))
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    vars_extr->m_component_decls.append(decl);
  }
}

void 
fcc_vars_extr_extract_tag_filter(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* tag_filter) 
{
  char* buffer = new char[MAX_TAG_NAME];
  strncpy(buffer, tag_filter->m_tag_filter.m_tag, MAX_TAG_NAME);
  FURIOUS_COPY_AND_CHECK_STR(buffer, tag_filter->m_tag_filter.m_tag, MAX_TAG_NAME)
  vars_extr->m_tags.append(buffer);
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  fcc_vars_extr_extract(vars_extr, 
                       &subplan->m_nodes[tag_filter->m_tag_filter.m_child]);
}

void
fcc_vars_extr_extract_component_filter(fcc_vars_extr_t* vars_extr, 
                      const fcc_operator_t* component_filter) 
{
  char tmp[MAX_TYPE_NAME];
  fcc_type_name(component_filter->m_component_filter.m_filter_type, 
                tmp, 
                MAX_TYPE_NAME);

  bool found = false;
  const uint32_t num_components = vars_extr->m_components.size();
  for(uint32_t i = 0; i < num_components; ++i)
  {
    if(strcmp(vars_extr->m_components[i], tmp) == 0)
    {
      found = true;
    }
  }

  if(!found)
  {
    char* buffer = new char[MAX_TYPE_NAME];
    strncpy(buffer, tmp, MAX_TYPE_NAME);
    vars_extr->m_components.append(buffer);
    fcc_decl_t decl;
    fcc_type_decl(component_filter->m_component_filter.m_filter_type, &decl);
    vars_extr->m_component_decls.append(decl);
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
  char tmp[MAX_TYPE_NAME];
  FURIOUS_COPY_AND_CHECK_STR(tmp, type_name, MAX_TYPE_NAME);
  sanitize_name(tmp);

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "table_%s", tmp);

  if(op != nullptr)
  {
    str_builder_append(&str_builder,"_%u",op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_temp_table_name(const char* type_name, 
                         char * buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  char tmp[MAX_TABLE_VARNAME];
  generate_table_name(type_name, 
                      tmp,
                      MAX_TABLE_VARNAME,
                      op);

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "temp_%s", tmp);

  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_ref_table_name(const char* ref_name, 
                        char* buffer,
                        uint32_t buffer_length,
                        const fcc_operator_t* op)
{
  char tmp[MAX_TABLE_VARNAME];
  generate_table_name(ref_name, 
                      tmp,
                      MAX_TABLE_VARNAME,
                      op);

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "ref_%s", tmp);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const fcc_operator_t* op)
{

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "tagged_%s", tag_name);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "iter_%s", table_name);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const fcc_operator_t* op)
{
  char tmp[MAX_TYPE_NAME];
  FURIOUS_COPY_AND_CHECK_STR(tmp, type_name, MAX_TYPE_NAME);
  sanitize_name(tmp);

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "block_%s", tmp);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_cluster_name(const fcc_operator_t* op,
                      char* buffer,
                      uint32_t buffer_length)
{
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "cluster_%u", op->m_id);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  char tmp[MAX_REF_NAME];
  FURIOUS_COPY_AND_CHECK_STR(tmp, ref_name, MAX_REF_NAME);
  sanitize_name(tmp);
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "ref_%s_groups_%u", tmp, op->m_id);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_hashtable_name(const fcc_operator_t* op,
                        char* buffer,
                        uint32_t buffer_length)
{
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "hashtable_%u",op->m_id);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_system_wrapper_name(const char* system_name,
                             uint32_t system_id,
                             char* buffer,
                             uint32_t buffer_length,
                             const fcc_operator_t* op)
{
  char base_name[MAX_TYPE_NAME];
  FURIOUS_COPY_AND_CHECK_STR(base_name, system_name, MAX_TYPE_NAME);
  tolower(base_name);
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "%s_%d",base_name, system_id  );
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}

uint32_t
generate_global_name(const char* type_name, 
                     char* buffer,
                     uint32_t buffer_length,
                     const fcc_operator_t* op)
{
  char tmp[MAX_TYPE_NAME];
  strncpy(tmp, type_name, buffer_length);
  FURIOUS_COPY_AND_CHECK_STR(tmp, type_name, MAX_TYPE_NAME);
  sanitize_name(tmp);
  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "global_%s", tmp);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);
  return length;
}


} /* furious */ 
