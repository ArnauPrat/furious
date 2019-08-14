

#include "../common/types.h"
#include "../common/str_builder.h"
#include "../common/dyn_array.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "../frontend/operator.h"
#include "driver.h"

#include "codegen.h"
#include "codegen_tools.h"

#include <string.h>

namespace furious 
{

extern CodeGenRegistry* p_registry;

DependenciesExtr::~DependenciesExtr()
{
  const uint32_t num_includes = m_include_files.size();
  for (uint32_t i = 0; i < num_includes; ++i) 
  {
    delete [] m_include_files[i];
  }
}

void
DependenciesExtr::extract_dependencies(const DynArray<Dependency>& deps)
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
        const uint32_t num_decls = m_declarations.size();
        for (uint32_t j = 0; j < num_decls; ++j) 
        {
          if(fcc_decl_is_same(dep.m_decl, m_declarations[j]))
          {
            found = true;
            break;
          }
        }

        if(!found)
        {
          m_declarations.append(dep.m_decl);
        }
      }
    } 
    else 
    {
      bool found = false;
      const uint32_t num_includes = m_include_files.size();
      for (uint32_t j = 0; j < num_includes; ++j) 
      {
        if(strcmp(m_include_files[j], dep.m_include_file) == 0)
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
        m_include_files.append(buffer);
      }
    }
  }
}

void 
DependenciesExtr::visit(const Foreach* foreach)
{
  uint32_t size = foreach->p_systems.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const fcc_system_t* system = foreach->p_systems[i];
    extract_dependencies(fcc_type_dependencies(system->m_system_type));
  }
  foreach->p_child.get()->accept(this);
}

void 
DependenciesExtr::visit(const Scan* scan) 
{
  if(scan->m_columns[0].m_type == FccColumnType::E_COMPONENT)
  {
    extract_dependencies(fcc_type_dependencies(scan->m_columns[0].m_component_type));
  }
} 

void
DependenciesExtr::visit(const Join* join) 
{
  join->p_left.get()->accept(this);
  join->p_right.get()->accept(this);
}

void
DependenciesExtr::visit(const LeftFilterJoin* left_filter_join) 
{
  left_filter_join->p_left.get()->accept(this);
  left_filter_join->p_right.get()->accept(this);
}

void
DependenciesExtr::visit(const CrossJoin* cross_join) 
{
  cross_join->p_left.get()->accept(this);
  cross_join->p_right.get()->accept(this);
}

void
DependenciesExtr::visit(const Fetch* fetch) 
{
  if(fetch->m_columns[0].m_type == FccColumnType::E_GLOBAL)
  {
    extract_dependencies(fcc_type_dependencies(fetch->m_columns[0].m_component_type));
  }
}

void 
DependenciesExtr::visit(const TagFilter* tag_filter) 
{
  tag_filter->p_child.get()->accept(this);
}

void
DependenciesExtr::visit(const ComponentFilter* component_filter) 
{
  extract_dependencies(fcc_type_dependencies(component_filter->m_filter_type));
  component_filter->p_child.get()->accept(this);
}

void
DependenciesExtr::visit(const PredicateFilter* predicate_filter) 
{
  extract_dependencies(fcc_decl_dependencies(predicate_filter->m_func_decl));
  predicate_filter->p_child.get()->accept(this);
}

void
DependenciesExtr::visit(const Gather* gather) 
{
  gather->p_child.get()->accept(this);
}

void
DependenciesExtr::visit(const CascadingGather* casc_gather) 
{
  casc_gather->p_child.get()->accept(this);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

VarsExtr::~VarsExtr()
{
  const uint32_t num_components = m_components.size();
  for (uint32_t i = 0; i < num_components; ++i) 
  {
    delete [] m_components[i];
  }

  const uint32_t num_tags = m_tags.size();
  for (uint32_t i = 0; i < num_tags; ++i) 
  {
    delete [] m_tags[i];
  }

  const uint32_t num_refs = m_references.size();
  for (uint32_t i = 0; i < num_refs; ++i) 
  {
    delete [] m_references[i];
  }
}


void 
VarsExtr::visit(const Foreach* foreach)
{
  foreach->p_child.get()->accept(this);
}

void 
VarsExtr::visit(const Scan* scan) 
{
  if(scan->m_columns[0].m_type == FccColumnType::E_COMPONENT)
  {
    char tmp[MAX_TYPE_NAME];
    const uint32_t length = fcc_type_name(scan->m_columns[0].m_component_type, 
                                          tmp, MAX_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    const uint32_t num_components = m_components.size();
    bool found = false;
    for(uint32_t i = 0; i < num_components; ++i)
    {
      if(strcmp(tmp, m_components[i]) == 0)
      {
        found = true;
        break;
      }
    }
    if(!found)
    {
      char* buffer = new char [MAX_TYPE_NAME];
      strncpy(buffer, tmp, MAX_TYPE_NAME);
      m_components.append(buffer);
      fcc_decl_t decl;
      fcc_type_decl(scan->m_columns[0].m_component_type, &decl);
      bool found = false;
      const uint32_t num_decls = m_component_decls.size();
      for (uint32_t i = 0; i < num_decls; ++i) 
      {
        if(fcc_decl_is_same(m_component_decls[i], decl))
        {
          found = true;
          break;
        }
      }
      if(!found)
      {
        m_component_decls.append(decl);
      }
    }
  }
  else
  {
    bool found = false;
    const uint32_t num_references = m_references.size();
    for (uint32_t i = 0; i < num_references; ++i) 
    {
      if(strcmp(m_references[i], scan->m_columns[0].m_ref_name) == 0)
      {
        found = true;
        break;
      }
    }
    if (!found) 
    {
      char* buffer = new char[MAX_REF_NAME];
      strncpy(buffer, scan->m_columns[0].m_ref_name, MAX_REF_NAME);
      m_references.append(buffer);
    }
  }
}

void
VarsExtr::visit(const Join* join) 
{
  join->p_left.get()->accept(this);
  join->p_right.get()->accept(this);
}

void
VarsExtr::visit(const LeftFilterJoin* left_filter_join) 
{
  left_filter_join->p_left.get()->accept(this);
  left_filter_join->p_right.get()->accept(this);
}

void
VarsExtr::visit(const CrossJoin* cross_join) 
{
  cross_join->p_left.get()->accept(this);
  cross_join->p_right.get()->accept(this);
}

void
VarsExtr::visit(const Fetch* fetch)
{
  fcc_decl_t decl;
  fcc_type_decl(fetch->m_columns[0].m_component_type, &decl);
  bool found = false;
  const uint32_t num_decls = m_component_decls.size();
  for (uint32_t i = 0; i < num_decls; ++i) 
  {
    if(fcc_decl_is_same(decl, m_component_decls[i]))
    {
      found = true;
      break;
    }
  }

  if (!found)
  {
    m_component_decls.append(decl);
  }
}

void 
VarsExtr::visit(const TagFilter* tag_filter) 
{
  char* buffer = new char[MAX_TAG_NAME];
  strncpy(buffer, tag_filter->m_tag, MAX_TAG_NAME);
  m_tags.append(buffer);
  tag_filter->p_child.get()->accept(this);
}

void
VarsExtr::visit(const ComponentFilter* component_filter) 
{
  char tmp[MAX_TYPE_NAME];
  const uint32_t length = fcc_type_name(component_filter->m_filter_type, tmp, MAX_TYPE_NAME);
  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

  bool found = false;
  const uint32_t num_components = m_components.size();
  for(uint32_t i = 0; i < num_components; ++i)
  {
    if(strcmp(m_components[i], tmp) == 0)
    {
      found = true;
    }
  }

  if(!found)
  {
    char* buffer = new char[MAX_TYPE_NAME];
    strncpy(buffer, tmp, MAX_TYPE_NAME);
    m_components.append(buffer);
    fcc_decl_t decl;
    fcc_type_decl(component_filter->m_filter_type, &decl);
    m_component_decls.append(decl);
    component_filter->p_child.get()->accept(this);
  }
}

void
VarsExtr::visit(const PredicateFilter* predicate_filter) 
{
  predicate_filter->p_child.get()->accept(this);
}

void
VarsExtr::visit(const Gather* gather)
{
  gather->p_ref_table.get()->accept(this);
  gather->p_child.get()->accept(this);
}

void
VarsExtr::visit(const CascadingGather* casc_gather)
{
  casc_gather->p_ref_table.get()->accept(this);
  casc_gather->p_child.get()->accept(this);
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

CodeGenContext::CodeGenContext(FILE* fd) :
p_fd(fd)
{
  p_consumer = new ConsumeVisitor(this);
  p_producer = new ProduceVisitor(this);
}

CodeGenContext::~CodeGenContext()
{
  delete p_consumer;
  delete p_producer;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


CodeGenRegistry::CodeGenRegistry()
{
}

CodeGenRegistry::~CodeGenRegistry()
{
  for(uint32_t i = 0; i < m_contexts.size(); ++i)
  {
    delete m_contexts[i].p_context;
  }
}

CodeGenContext* 
CodeGenRegistry::find_or_create(const FccOperator* op, FILE* fd)
{
  for(uint32_t i = 0; i < m_contexts.size(); ++i)
  {
    if(m_contexts[i].p_operator == op)
    {
      return m_contexts[i].p_context;
    }
  }

  Entry entry = {op,new CodeGenContext(fd)};
  m_contexts.append(entry);
  return entry.p_context;
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////



void 
consume(FILE* fd,
        const FccOperator* op,
        const char* source,
        const FccOperator* caller)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  FURIOUS_PERMA_ASSERT(strlen(source) <= MAX_SOURCE_LENGTH);
  strcpy(context->m_source,source);
  context->p_caller = caller;
  op->accept(context->p_consumer);
}

void 
produce(FILE* fd,
        const FccOperator* op)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  op->accept(context->p_producer);
}

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
                    const FccOperator* op)
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
                         const FccOperator* op)
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
                        const FccOperator* op)
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
                       const FccOperator* op)
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
                         const FccOperator* op)
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
                    const FccOperator* op)
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
generate_cluster_name(const FccOperator* op,
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
                         const FccOperator* op)
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
generate_hashtable_name(const FccOperator* op,
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
                             const FccOperator* op)
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
                     const FccOperator* op)
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
