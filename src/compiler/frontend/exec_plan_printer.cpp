

#include "exec_plan_printer.h"
#include "fcc_context.h"
#include "drivers/clang/clang_tools.h"
#include "../common/str_builder.h"
#include "operator.h"

namespace furious
{

static constexpr uint32_t buffer_length = 2048;
static char buffer[buffer_length];

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          fcc_operator_t* fcc_operator);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Scan* scan);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Join* join);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const CrossJoin* crossjoin);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const LeftFilterJoin* left_filter_join);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Foreach* foreach);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Fetch* fetch);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const TagFilter* tag_filter);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const PredicateFilter* predicate_filter);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const ComponentFilter* component_filter);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Gather* gather);
static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const CascadingGather* gather);

static void
fcc_subplan_printer_incr_level(fcc_subplan_printer_t* printer,
                               bool sibling)
{
  if(sibling)
  {
    printer->m_offsets.append(' ');
    printer->m_offsets.append('|');
  } 
  else 
  {
    printer->m_offsets.append(' ');
    printer->m_offsets.append(' ');
  }
}

static void
fcc_subplan_printer_decr_level(fcc_subplan_printer_t* printer)
{
  printer->m_offsets.pop();
  printer->m_offsets.pop();
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const char* str)
{
  for(int32_t i = 0;
      i < (int32_t)printer->m_offsets.size();
      ++i) 
  {
    str_builder_append(&printer->m_str_builder,
                       "%c", 
                       printer->m_offsets[i]);
  }
  str_builder_append(&printer->m_str_builder, 
                     "- %s\n", 
                     str);
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

static void 
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Foreach* foreach) 
{
  const fcc_system_t* info = foreach->p_systems[0];
  snprintf(buffer, buffer_length, "foreach (%u) - \"%s\"",
           foreach->m_id, 
           to_string(info).c_str());

  fcc_subplan_printer_print(printer, 
                            buffer);

  fcc_subplan_printer_incr_level(printer,
                                 false);
  fcc_subplan_printer_print(printer, foreach->p_child.get());
  fcc_subplan_printer_decr_level(printer);
}

static void 
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const Scan* scan) 
{
  const fcc_column_t* column = &scan->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    char ctype[MAX_TYPE_NAME];
    const uint32_t length = fcc_type_name(column->m_component_type,
                                          ctype,
                                          MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

    snprintf(buffer, 
             buffer_length,
             "scan (%u) - \"%s\"",
             scan->m_id, 
             ctype);

  }
  else
  {
    snprintf(buffer,
             buffer_length,
             "scan (%u) - #REFERENCE \"%s\"",
             scan->m_id, 
             column->m_ref_name);
  }

  fcc_subplan_printer_print(printer,
                            buffer);

}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const Join* join) 
{
  snprintf(buffer, 
           buffer_length,
           "join(%u)", join->m_id);

  fcc_subplan_printer_print(printer, 
                            buffer);
  fcc_subplan_printer_incr_level(printer, 
                                 true);
  // print left child
  fcc_subplan_printer_print(printer, 
                            join->p_left.get());

  // print right child
  fcc_subplan_printer_print(printer,
                            join->p_right.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const LeftFilterJoin* left_filter_join) 
{
  snprintf(buffer,
           buffer_length,
           "left_filter_join(%u)", left_filter_join->m_id);
  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer, 
                                 true);
  // print left child
  fcc_subplan_printer_print(printer, 
                            left_filter_join->p_left.get());

  // print right child
  fcc_subplan_printer_print(printer,
                            left_filter_join->p_right.get());
  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const CrossJoin* cross_join) 
{
  snprintf(buffer, 
           buffer_length,
           "cross_join(%u)", 
           cross_join->m_id);

  fcc_subplan_printer_print(printer,
                            buffer);
  fcc_subplan_printer_incr_level(printer,
                                 true);
  // print left hicld
  fcc_subplan_printer_print(printer,
                            cross_join->p_left.get());

  // print right child
  fcc_subplan_printer_print(printer,
                            cross_join->p_right.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Fetch* fetch) 
{

  char ctype[MAX_TYPE_NAME];
  const uint32_t length = fcc_type_name(fetch->m_columns[0].m_component_type,
                                        ctype,
                                        MAX_TYPE_NAME);

  FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

  snprintf(buffer, 
           buffer_length, 
           "fetch(%u) - GLOBAL %s", 
           fetch->m_id, ctype);

  fcc_subplan_printer_print(printer,
                            buffer);
}

static void 
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const TagFilter* tag_filter) 
{
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

  snprintf(buffer,
           buffer_length,
           "tag_filter (%u) - %s - \"%s\" - %s", 
           tag_filter->m_id, 
           type, 
           tag_filter->m_tag,
           column_type); 

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer, 
                                 false);

  fcc_subplan_printer_print(printer,
                            tag_filter->p_child.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const ComponentFilter* component_filter) 
{
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


  snprintf(buffer,
           buffer_length,
           "component_filter (%u) - %s - \"%s\"", 
           component_filter->m_id, 
           type, 
           ctype);

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer,false);

  fcc_subplan_printer_print(printer, 
                            component_filter->p_child.get());

  fcc_subplan_printer_decr_level(printer);
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

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer, 
                          const PredicateFilter* predicate_filter) 
{
  uint32_t function_name_length = 2048;
  char function_name[function_name_length];
  uint32_t length = fcc_decl_function_name(predicate_filter->m_func_decl,
                                           function_name,
                                           function_name_length);
  FURIOUS_CHECK_STR_LENGTH(length, function_name_length);
  snprintf(buffer, buffer_length, "predicate_filter (%u) - %s", 
           predicate_filter->m_id, 
           function_name);

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer,
                                 false);
  fcc_subplan_printer_print(printer, 
                            predicate_filter->p_child.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const Gather* gather) 
{
  fcc_column_t* ref_column = &gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != fcc_column_type_t::E_REFERENCE)
  {
    //TODO: handle error
  }
  snprintf(buffer, 
           buffer_length,
           "gather (%u)", 
           gather->m_id);

  fcc_subplan_printer_print(printer,
                            buffer);
  fcc_subplan_printer_incr_level(printer,
                                 false);
  fcc_subplan_printer_print(printer, 
                            gather->p_ref_table.get());

  fcc_subplan_printer_print(printer, 
                            gather->p_child.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const CascadingGather* casc_gather) 
{
  fcc_column_t* ref_column = &casc_gather->p_ref_table.p_ptr->m_columns[0];
  if(ref_column->m_type != fcc_column_type_t::E_REFERENCE)
  {
    //TODO: handle error
  }
  snprintf(buffer, buffer_length,"cascading_gather (%u)", 
           casc_gather->m_id);

  fcc_subplan_printer_print(printer, 
                            buffer);

  fcc_subplan_printer_incr_level(printer, 
                                 false);
  fcc_subplan_printer_print(printer, 
                            casc_gather->p_ref_table.get());

  fcc_subplan_printer_print(printer,
                            casc_gather->p_child.get());

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          fcc_operator_t* fcc_operator)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      fcc_subplan_printer_print(printer, (Scan*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_subplan_printer_print(printer, (Foreach*)fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_subplan_printer_print(printer, (Join*)fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_subplan_printer_print(printer, (LeftFilterJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_subplan_printer_print(printer, (CrossJoin*)fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_subplan_printer_print(printer, (Fetch*)fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_subplan_printer_print(printer, (Gather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_subplan_printer_print(printer, (CascadingGather*)fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_subplan_printer_print(printer, (TagFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_subplan_printer_print(printer, (PredicateFilter*)fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_subplan_printer_print(printer, (ComponentFilter*)fcc_operator);
      break;
  };
}

void
fcc_subplan_printer_init(fcc_subplan_printer_t* printer, 
                         bool add_comments)
{
  printer->m_indents = 0;
  if(add_comments)
  {
    printer->m_offsets.append('/');
    printer->m_offsets.append('/');
  }
  str_builder_init(&printer->m_str_builder);
}

void
fcc_subplan_printer_release(fcc_subplan_printer_t* printer)
{
  str_builder_release(&printer->m_str_builder);
}

void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const FccSubPlan* plan) 
{
  fcc_subplan_printer_print(printer, plan->p_root);
}


} 
