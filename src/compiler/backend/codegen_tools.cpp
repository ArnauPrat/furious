

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
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Scan* scan);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Foreach* scan);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Join* join);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const CrossJoin* cross_join);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const LeftFilterJoin* left_filter_join);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Gather* gather);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const CascadingGather* casc_gather);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Fetch* fetch);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const TagFilter* tag_filter);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const PredicateFilter* predicate_filter);

static void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const ComponentFilter* component_filter);

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
        strncpy(buffer, dep.m_include_file, MAX_INCLUDE_PATH_LENGTH);
        FURIOUS_CHECK_STR_LENGTH(strlen(dep.m_include_file), MAX_INCLUDE_PATH_LENGTH);
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
      fcc_deps_extr_extract(deps_extr, (Scan*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_deps_extr_extract(deps_extr, (Foreach*)fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_deps_extr_extract(deps_extr, (Join*)fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_deps_extr_extract(deps_extr, (LeftFilterJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_deps_extr_extract(deps_extr, (CrossJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_deps_extr_extract(deps_extr, (Fetch*)fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_deps_extr_extract(deps_extr, (Gather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_deps_extr_extract(deps_extr, (CascadingGather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_deps_extr_extract(deps_extr, (TagFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_deps_extr_extract(deps_extr, (PredicateFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_deps_extr_extract(deps_extr, (ComponentFilter*)fcc_operator);
      break;
  };
}

void 
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Foreach* foreach)
{
  uint32_t size = foreach->p_systems.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const fcc_system_t* system = foreach->p_systems[i];
    fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(system->m_system_type));
  }
  fcc_deps_extr_extract(deps_extr, foreach->p_child.get());
}

void 
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr, 
                      const Scan* scan) 
{
  if(scan->m_columns[0].m_type == fcc_column_type_t::E_COMPONENT)
  {
    fcc_deps_extr_extract(deps_extr,
                          fcc_type_dependencies(scan->m_columns[0].m_component_type));
  }
} 

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr, 
                      const Join* join) 
{
  fcc_deps_extr_extract(deps_extr, join->p_left.get());
  fcc_deps_extr_extract(deps_extr, join->p_right.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const LeftFilterJoin* left_filter_join) 
{
  fcc_deps_extr_extract(deps_extr, left_filter_join->p_left.get());
  fcc_deps_extr_extract(deps_extr, left_filter_join->p_right.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const CrossJoin* cross_join) 
{
  fcc_deps_extr_extract(deps_extr,cross_join->p_left.get());
  fcc_deps_extr_extract(deps_extr,cross_join->p_right.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const Fetch* fetch) 
{
  if(fetch->m_columns[0].m_type == fcc_column_type_t::E_GLOBAL)
  {
    fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(fetch->m_columns[0].m_component_type));
  }
}

void 
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr, 
                      const TagFilter* tag_filter) 
{
  fcc_deps_extr_extract(deps_extr,tag_filter->p_child.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const ComponentFilter* component_filter) 
{
  fcc_deps_extr_extract(deps_extr, fcc_type_dependencies(component_filter->m_filter_type));
  fcc_deps_extr_extract(deps_extr, component_filter->p_child.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const PredicateFilter* predicate_filter) 
{
  fcc_deps_extr_extract(deps_extr, fcc_decl_dependencies(predicate_filter->m_func_decl));
  fcc_deps_extr_extract(deps_extr, predicate_filter->p_child.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr, 
                      const Gather* gather) 
{
  fcc_deps_extr_extract(deps_extr, gather->p_child.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr, 
                      const CascadingGather* casc_gather) 
{
  fcc_deps_extr_extract(deps_extr, casc_gather->p_child.get());
}

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const FccSubPlan* subplan)
{
  fcc_deps_extr_extract(deps_extr, subplan->p_root);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const fcc_operator_t* fcc_operator);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Scan* scan);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Foreach* scan);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Join* join);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const CrossJoin* cross_join);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const LeftFilterJoin* left_filter_join);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Gather* gather);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const CascadingGather* casc_gather);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Fetch* fetch);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const TagFilter* tag_filter);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const PredicateFilter* predicate_filter);

static void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const ComponentFilter* component_filter);

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
      fcc_vars_extr_extract(vars_extr, (Scan*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_vars_extr_extract(vars_extr, (Foreach*)fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_vars_extr_extract(vars_extr, (Join*)fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_vars_extr_extract(vars_extr, (LeftFilterJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_vars_extr_extract(vars_extr, (CrossJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_vars_extr_extract(vars_extr, (Fetch*)fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_vars_extr_extract(vars_extr, (Gather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_vars_extr_extract(vars_extr, (CascadingGather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_vars_extr_extract(vars_extr, (TagFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_vars_extr_extract(vars_extr, (PredicateFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_vars_extr_extract(vars_extr, (ComponentFilter*)fcc_operator);
      break;
  };
}

void 
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                     const Foreach* foreach)
{
  fcc_vars_extr_extract(vars_extr, foreach->p_child.get());
}

void 
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const Scan* scan) 
{
  if(scan->m_columns[0].m_type == fcc_column_type_t::E_COMPONENT)
  {
    char tmp[MAX_TYPE_NAME];
    const uint32_t length = fcc_type_name(scan->m_columns[0].m_component_type, 
                                          tmp, MAX_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

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
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const Join* join) 
{
  fcc_vars_extr_extract(vars_extr, join->p_left.get());
  fcc_vars_extr_extract(vars_extr, join->p_right.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const LeftFilterJoin* left_filter_join) 
{
  fcc_vars_extr_extract(vars_extr, left_filter_join->p_left.get());
  fcc_vars_extr_extract(vars_extr, left_filter_join->p_right.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const CrossJoin* cross_join) 
{
  fcc_vars_extr_extract(vars_extr, cross_join->p_left.get());
  fcc_vars_extr_extract(vars_extr, cross_join->p_right.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const Fetch* fetch)
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
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const TagFilter* tag_filter) 
{
  char* buffer = new char[MAX_TAG_NAME];
  strncpy(buffer, tag_filter->m_tag, MAX_TAG_NAME);
  vars_extr->m_tags.append(buffer);
  fcc_vars_extr_extract(vars_extr, 
                       tag_filter->p_child.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const ComponentFilter* component_filter) 
{
  char tmp[MAX_TYPE_NAME];
  const uint32_t length = fcc_type_name(component_filter->m_filter_type, tmp, MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

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
    fcc_type_decl(component_filter->m_filter_type, &decl);
    vars_extr->m_component_decls.append(decl);
    fcc_vars_extr_extract(vars_extr, component_filter->p_child.get());
  }
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr, 
                      const PredicateFilter* predicate_filter) 
{
  fcc_vars_extr_extract(vars_extr, 
                        predicate_filter->p_child.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const Gather* gather)
{
  fcc_vars_extr_extract(vars_extr, gather->p_ref_table.get());
  fcc_vars_extr_extract(vars_extr, gather->p_child.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const CascadingGather* casc_gather)
{
  fcc_vars_extr_extract(vars_extr,casc_gather->p_ref_table.get());
  fcc_vars_extr_extract(vars_extr,casc_gather->p_child.get());
}

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const FccSubPlan* subplan)
{
  fcc_vars_extr_extract(vars_extr, subplan->p_root);
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
  strncpy(tmp, type_name, MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(type_name), MAX_TYPE_NAME);
  sanitize_name(tmp);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "table_%s", tmp);

  if(op != nullptr)
  {
    str_builder_append(&str_builder,"_%u",op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  str_builder_release(&str_builder);
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

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "temp_%s", tmp);

  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
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

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "ref_%s", tmp);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const fcc_operator_t* op)
{

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "tagged_%s", tag_name);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "iter_%s", table_name);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const fcc_operator_t* op)
{
  char tmp[MAX_TYPE_NAME];
  strncpy(tmp, type_name, MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(type_name), MAX_TYPE_NAME);
  sanitize_name(tmp);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "block_%s", tmp);
  if(op != nullptr)
  {
  str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_cluster_name(const fcc_operator_t* op,
                      char* buffer,
                      uint32_t buffer_length)
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "cluster_%u", op->m_id);
  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op)
{
  char tmp[MAX_REF_NAME];
  strncpy(tmp, ref_name, MAX_REF_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(ref_name), MAX_REF_NAME);
  sanitize_name(tmp);
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "ref_%s_groups_%u", tmp, op->m_id);
  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
  return length;
}

uint32_t
generate_hashtable_name(const fcc_operator_t* op,
                        char* buffer,
                        uint32_t buffer_length)
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "hashtable_%u",op->m_id);
  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  str_builder_release(&str_builder);
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
  strncpy(base_name, system_name, MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(strlen(system_name), MAX_TYPE_NAME);

  tolower(base_name);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "%s_%d",base_name, system_id  );
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }
  const uint32_t length = str_builder.m_pos;
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  str_builder_release(&str_builder);
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
  FURIOUS_CHECK_STR_LENGTH(strlen(type_name),MAX_TYPE_NAME);
  sanitize_name(tmp);
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "global_%s", tmp);
  if(op != nullptr)
  {
    str_builder_append(&str_builder, "_%u", op->m_id);
  }

  const uint32_t length = str_builder.m_pos;
  strncpy(buffer, str_builder.p_buffer, buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  str_builder_release(&str_builder);
  return length;
}


} /* furious */ 
