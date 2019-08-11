

#include "exec_plan_printer.h"
#include "fcc_context.h"
#include "drivers/clang/clang_tools.h"
#include "../common/str_builder.h"
#include "operator.h"

namespace furious
{

ExecPlanPrinter::ExecPlanPrinter(bool add_comments)
{
  if(add_comments)
  {
    m_offsets.append('/');
    m_offsets.append('/');
  }
  str_builder_init(&m_str_builder);
}

ExecPlanPrinter::~ExecPlanPrinter()
{
  str_builder_release(&m_str_builder);
}

void
ExecPlanPrinter::traverse(const FccSubPlan* plan) 
{
  plan->p_root->accept(this);
}

static std::string
to_string(const fcc_system_t* info)
{
  constexpr uint32_t buffer_length = 2048;
  char buffer[buffer_length];
  const uint32_t length = fcc_type_name(info->m_system_type, 
                                        buffer,
                                        buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, 
                     "%s (", 
                     buffer);

  const DynArray<fcc_expr_t>& ctor_params = info->m_ctor_params;
  if(ctor_params.size() > 0)
  {
    const uint32_t length = fcc_expr_code(info->m_ctor_params[0],
                                          buffer,
                                          buffer_length);
    FURIOUS_CHECK_STR_LENGTH(length, buffer_length);

    str_builder_append(&str_builder, 
                         "%s", 
                         buffer);

    for (int32_t i = 1; i < (int32_t)ctor_params.size(); ++i) 
    {

      const uint32_t length = fcc_expr_code(info->m_ctor_params[i],
                                            buffer,
                                            buffer_length);
      FURIOUS_CHECK_STR_LENGTH(length, buffer_length);

      str_builder_append(&str_builder,
                         ", %s", 
                         buffer); 
    }

  }
  str_builder_append(&str_builder,")");
  std::string ret (str_builder.p_buffer);
  str_builder_release(&str_builder);
  return ret;
}

void 
ExecPlanPrinter::visit(const Foreach* foreach) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  const fcc_system_t* info = foreach->p_systems[0];
  str_builder_append(&str_builder, "foreach (%u) - \"%s\"",
                        foreach->m_id, 
                        to_string(info).c_str());
  print(str_builder.p_buffer);
  incr_level(false);
  foreach->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void 
ExecPlanPrinter::visit(const Scan* scan) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  const FccColumn* column = &scan->m_columns[0];
  if(column->m_type == FccColumnType::E_COMPONENT)
  {
    char ctype[MAX_TYPE_NAME];
    const uint32_t length = fcc_type_name(column->m_component_type,
                                          ctype,
                                          MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    str_builder_append(&str_builder, "scan (%u) - \"%s\"",
                          scan->m_id, 
                          ctype);

  }
  else
  {
    str_builder_append(&str_builder,"scan (%u) - #REFERENCE \"%s\"",
                          scan->m_id, 
                          column->m_ref_name);
  }

  print(str_builder.p_buffer);
  incr_level(false);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const Join* join) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "join(%u)", join->m_id);
  print(str_builder.p_buffer);
  incr_level(true);
  join->p_left.get()->accept(this);
  join->p_right.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const LeftFilterJoin* left_filter_join) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "left_filter_join(%u)", left_filter_join->m_id);
  print(str_builder.p_buffer);
  incr_level(true);
  left_filter_join->p_left.get()->accept(this);
  left_filter_join->p_right.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const CrossJoin* cross_join) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder,"cross_join(%u)", cross_join->m_id);
  print(str_builder.p_buffer);
  incr_level(true);
  cross_join->p_left.get()->accept(this);
  cross_join->p_right.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const Fetch* fetch) 
{

  char ctype[MAX_TYPE_NAME];
  const uint32_t length = fcc_type_name(fetch->m_columns[0].m_component_type,
                                        ctype,
                                        MAX_TYPE_NAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder,
                        "fetch(%u) - GLOBAL %s", 
                        fetch->m_id, ctype);
  print(str_builder.p_buffer);
  str_builder_release(&str_builder);
}

void 
ExecPlanPrinter::visit(const TagFilter* tag_filter) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(tag_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    type = has_type;
  }
  else
  {
    type = has_not_type;
  }

  char* column_type;
  char on_key[] = "on_key";
  char on_ref_column[] = "on_ref_column";
  if(tag_filter->m_on_column)
  {
    column_type = on_ref_column;
  }
  else
  {
    column_type = on_key;
  }

  str_builder_append(&str_builder, "tag_filter (%u) - %s - \"%s\" - %s", 
                        tag_filter->m_id, 
                        type, 
                        tag_filter->m_tag,
                        column_type); 

  print(str_builder.p_buffer);
  incr_level(false);
  tag_filter->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const ComponentFilter* component_filter) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(component_filter->m_op_type == FccFilterOpType::E_HAS) 
  {
    type = has_type;
  }
  else
  {
    type = has_not_type;
  }

  char ctype[MAX_TYPE_NAME];
  const uint32_t length = fcc_type_name(component_filter->m_filter_type,
                                        ctype,
                                        MAX_TYPE_NAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);


  str_builder_append(&str_builder,"component_filter (%u) - %s - \"%s\"", 
                        component_filter->m_id, 
                        type, 
                        ctype);
  print(str_builder.p_buffer);
  incr_level(false);
  component_filter->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

std::string
to_string(fcc_decl_t decl)
{
  constexpr uint32_t buffer_length = 2048;
  char buffer[buffer_length];
  const uint32_t length = fcc_decl_function_name(decl, 
                                                 buffer,
                                                 buffer_length);
  FURIOUS_CHECK_STR_LENGTH(length, buffer_length);
  return buffer;
}

void
ExecPlanPrinter::visit(const PredicateFilter* predicate_filter) 
{
  uint32_t function_name_length = 2048;
  char function_name[function_name_length];
  uint32_t length = fcc_decl_function_name(predicate_filter->m_func_decl,
                                           function_name,
                                           function_name_length);
  FURIOUS_CHECK_STR_LENGTH(length, function_name_length);
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, "predicate_filter (%u) - %s", 
                        predicate_filter->m_id, 
                        function_name);
  print(str_builder.p_buffer);
  incr_level(false);
  predicate_filter->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const Gather* gather) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  FccColumn* ref_column = &gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != FccColumnType::E_REFERENCE)
  {
    //TODO: handle error
  }
  str_builder_append(&str_builder, "gather (%u)", 
                        gather->m_id);
  print(str_builder.p_buffer);
  incr_level(false);
  gather->p_ref_table.get()->accept(this);
  gather->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::visit(const CascadingGather* casc_gather) 
{
  str_builder_t str_builder;
  str_builder_init(&str_builder);
  FccColumn* ref_column = &casc_gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != FccColumnType::E_REFERENCE)
  {
    //TODO: handle error
  }
  str_builder_append(&str_builder,"cascading_gather (%u)", 
                        casc_gather->m_id);
  print(str_builder.p_buffer);
  incr_level(false);
  casc_gather->p_ref_table.get()->accept(this);
  casc_gather->p_child.get()->accept(this);
  decr_level();
  str_builder_release(&str_builder);
}

void
ExecPlanPrinter::incr_level(bool sibling)
{
  if(sibling)
  {
    m_offsets.append(' ');
    m_offsets.append('|');
  } 
  else 
  {
    m_offsets.append(' ');
    m_offsets.append(' ');
  }
}

void
ExecPlanPrinter::decr_level()
{
  m_offsets.pop();
  m_offsets.pop();
}

void
ExecPlanPrinter::print(const std::string& str)
{
  for(int32_t i = 0;
      i < (int32_t)m_offsets.size();
      ++i) 
  {
    str_builder_append(&m_str_builder,"%c", m_offsets[i]);
  }
  str_builder_append(&m_str_builder, "- %s\n", str.c_str());
}

} /* furious
*/ 
