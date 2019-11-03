

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
                          const fcc_operator_t* fcc_operator);
static void
fcc_subplan_printer_print_scan(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* scan);
static void
fcc_subplan_printer_print_join(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* join);
static void
fcc_subplan_printer_print_cross_join(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* crossjoin);
static void
fcc_subplan_printer_print_left_filter_join(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* left_filter_join);
static void
fcc_subplan_printer_print_foreach(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* foreach);
static void
fcc_subplan_printer_print_fetch(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* fetch);
static void
fcc_subplan_printer_print_tag_filter(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* tag_filter);
static void
fcc_subplan_printer_print_predicate_filter(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* predicate_filter);
static void
fcc_subplan_printer_print_component_filter(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* component_filter);
static void
fcc_subplan_printer_print_gather(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* gather);
static void
fcc_subplan_printer_print_cascading_gather(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* casc_gather);

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
  if(printer->m_c_src_string)
  {
  str_builder_append(&printer->m_str_builder, 
                     "- %s\\n\\\n", 
                     str);
  }
  else
  {
  str_builder_append(&printer->m_str_builder, 
                     "- %s\n", 
                     str);
  }
}

static std::string
to_string(const fcc_system_t* info)
{
  constexpr uint32_t buffer_length = 2048;
  char buffer[buffer_length];
  fcc_type_name(info->m_system_type, 
                buffer,
                buffer_length);

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  str_builder_append(&str_builder, 
                     "%s (", 
                     buffer);

  const DynArray<fcc_expr_t>& ctor_params = info->m_ctor_params;
  if(ctor_params.size() > 0)
  {
    fcc_expr_code(info->m_ctor_params[0],
                  buffer,
                  buffer_length);

    str_builder_append(&str_builder, 
                         "%s", 
                         buffer);

    for (int32_t i = 1; i < (int32_t)ctor_params.size(); ++i) 
    {

      fcc_expr_code(info->m_ctor_params[i],
                    buffer,
                    buffer_length);

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
fcc_subplan_printer_print_foreach(fcc_subplan_printer_t* printer,
                                  const fcc_operator_t* foreach) 
{
  const fcc_system_t* info = foreach->m_foreach.p_system;
  snprintf(buffer, buffer_length, "foreach (%u) - %s",
           foreach->m_id, 
           to_string(info).c_str());

  fcc_subplan_printer_print(printer, 
                            buffer);

  fcc_subplan_printer_incr_level(printer,
                                 false);
  fcc_subplan_t* subplan = foreach->p_subplan;
  fcc_subplan_printer_print(printer, &subplan->m_nodes[foreach->m_foreach.m_child]);
  fcc_subplan_printer_decr_level(printer);
}

static void 
fcc_subplan_printer_print_scan(fcc_subplan_printer_t* printer, 
                          const fcc_operator_t* scan) 
{
  const fcc_column_t* column = &scan->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    char ctype[MAX_TYPE_NAME];
    fcc_type_name(column->m_component_type,
                  ctype,
                  MAX_TYPE_NAME);

    snprintf(buffer, 
             buffer_length,
             "scan (%u) - %s",
             scan->m_id, 
             ctype);

  }
  else
  {
    snprintf(buffer,
             buffer_length,
             "scan (%u) - #REFERENCE %s",
             scan->m_id, 
             column->m_ref_name);
  }

  fcc_subplan_printer_print(printer,
                            buffer);

}

static void
fcc_subplan_printer_print_join(fcc_subplan_printer_t* printer, 
                               const fcc_operator_t* join) 
{
  snprintf(buffer, 
           buffer_length,
           "join(%u)", join->m_id);

  fcc_subplan_printer_print(printer, 
                            buffer);
  fcc_subplan_printer_incr_level(printer, 
                                 true);
  fcc_subplan_t* subplan = join->p_subplan;
  // print left child
  fcc_subplan_printer_print(printer, 
                            &subplan->m_nodes[join->m_join.m_left]);

  // print right child
  fcc_subplan_printer_print(printer,
                            &subplan->m_nodes[join->m_join.m_right]);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_left_filter_join(fcc_subplan_printer_t* printer, 
                                           const fcc_operator_t* left_filter_join) 
{
  snprintf(buffer,
           buffer_length,
           "left_filter_join(%u)", left_filter_join->m_id);
  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer, 
                                 true);
  fcc_subplan_t* subplan = left_filter_join->p_subplan;
  // print left child
  fcc_subplan_printer_print(printer, 
                            &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_left]);

  // print right child
  fcc_subplan_printer_print(printer,
                            &subplan->m_nodes[left_filter_join->m_leftfilter_join.m_right]);
  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_cross_join(fcc_subplan_printer_t* printer, 
                                     const fcc_operator_t* cross_join) 
{
  snprintf(buffer, 
           buffer_length,
           "cross_join(%u)", 
           cross_join->m_id);

  fcc_subplan_printer_print(printer,
                            buffer);
  fcc_subplan_printer_incr_level(printer,
                                 true);
  fcc_subplan_t* subplan = cross_join->p_subplan;
  // print left hicld
  fcc_subplan_printer_print(printer,
                            &subplan->m_nodes[cross_join->m_cross_join.m_left]);

  // print right child
  fcc_subplan_printer_print(printer,
                            &subplan->m_nodes[cross_join->m_cross_join.m_right]);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_fetch(fcc_subplan_printer_t* printer,
                                const fcc_operator_t* fetch) 
{

  char ctype[MAX_TYPE_NAME];
  fcc_type_name(fetch->m_columns[0].m_component_type,
                ctype,
                MAX_TYPE_NAME);

  snprintf(buffer, 
           buffer_length, 
           "fetch(%u) - GLOBAL %s", 
           fetch->m_id, ctype);

  fcc_subplan_printer_print(printer,
                            buffer);
}

static void 
fcc_subplan_printer_print_tag_filter(fcc_subplan_printer_t* printer, 
                                     const fcc_operator_t* tag_filter) 
{
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(tag_filter->m_tag_filter.m_op_type == fcc_filter_op_type_t::E_HAS) 
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
  if(tag_filter->m_tag_filter.m_on_column)
  {
    column_type = on_ref_column;
  }
  else
  {
    column_type = on_key;
  }

  snprintf(buffer,
           buffer_length,
           "tag_filter (%u) - %s - %s - %s", 
           tag_filter->m_id, 
           type, 
           tag_filter->m_tag_filter.m_tag,
           column_type); 

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer, 
                                 false);

  fcc_subplan_t* subplan = tag_filter->p_subplan;
  fcc_subplan_printer_print(printer,
                            &subplan->m_nodes[tag_filter->m_tag_filter.m_child]);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_component_filter(fcc_subplan_printer_t* printer,
                                           const fcc_operator_t* component_filter) 
{
  char* type;
  char has_type[] ="has";
  char has_not_type[] ="has not";
  if(component_filter->m_component_filter.m_op_type == fcc_filter_op_type_t::E_HAS) 
  {
    type = has_type;
  }
  else
  {
    type = has_not_type;
  }

  char ctype[MAX_TYPE_NAME];
  fcc_type_name(component_filter->m_component_filter.m_filter_type,
                ctype,
                MAX_TYPE_NAME);


  snprintf(buffer,
           buffer_length,
           "component_filter (%u) - %s - %s", 
           component_filter->m_id, 
           type, 
           ctype);

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer,false);

  fcc_subplan_t* subplan = component_filter->p_subplan;
  fcc_subplan_printer_print(printer, 
                            &subplan->m_nodes[component_filter->m_component_filter.m_child]);

  fcc_subplan_printer_decr_level(printer);
}

std::string
to_string(fcc_decl_t decl)
{
  constexpr uint32_t buffer_length = 2048;
  char buffer[buffer_length];
  fcc_decl_function_name(decl, 
                         buffer,
                         buffer_length);
  return buffer;
}

static void
fcc_subplan_printer_print_predicate_filter(fcc_subplan_printer_t* printer, 
                          const fcc_operator_t* predicate_filter) 
{
  uint32_t function_name_length = 2048;
  char function_name[function_name_length];
  fcc_decl_function_name(predicate_filter->m_predicate_filter.m_func_decl,
                         function_name,
                         function_name_length);
  snprintf(buffer, buffer_length, "predicate_filter (%u) - %s", 
           predicate_filter->m_id, 
           function_name);

  fcc_subplan_printer_print(printer,
                            buffer);

  fcc_subplan_printer_incr_level(printer,
                                 false);
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  fcc_subplan_printer_print(printer, 
                            &subplan->m_nodes[predicate_filter->m_predicate_filter.m_child]);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_gather(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* gather) 
{
  fcc_subplan_t* subplan = gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[gather->m_gather.m_ref_table];
  fcc_column_t* ref_column = &ref_table->m_columns[0];
  if(ref_column->m_type != fcc_column_type_t::E_ID)
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
                            ref_table);

  fcc_operator_t* child = &subplan->m_nodes[gather->m_gather.m_child];
  fcc_subplan_printer_print(printer, 
                            child);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print_cascading_gather(fcc_subplan_printer_t* printer,
                                      const fcc_operator_t* casc_gather) 
{
  fcc_subplan_t* subplan = casc_gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[casc_gather->m_cascading_gather.m_ref_table];
  fcc_column_t* ref_column = &ref_table->m_columns[0];
  if(ref_column->m_type != fcc_column_type_t::E_ID)
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
                            ref_table);

  fcc_operator_t* child = &subplan->m_nodes[casc_gather->m_cascading_gather.m_child];
  fcc_subplan_printer_print(printer,
                            child);

  fcc_subplan_printer_decr_level(printer);
}

static void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const fcc_operator_t* fcc_operator)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      fcc_subplan_printer_print_scan(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_FOREACH:
      fcc_subplan_printer_print_foreach(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_JOIN:
      fcc_subplan_printer_print_join(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      fcc_subplan_printer_print_left_filter_join(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      fcc_subplan_printer_print_cross_join(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_FETCH:
      fcc_subplan_printer_print_fetch(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_GATHER:
      fcc_subplan_printer_print_gather(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      fcc_subplan_printer_print_cascading_gather(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      fcc_subplan_printer_print_tag_filter(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      fcc_subplan_printer_print_predicate_filter(printer, fcc_operator);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      fcc_subplan_printer_print_component_filter(printer, fcc_operator);
      break;
  };
}

void
fcc_subplan_printer_init(fcc_subplan_printer_t* printer, 
                         bool c_src_string, 
                         bool add_comments)
{
  printer->m_indents = 0;
  if(add_comments)
  {
    printer->m_offsets.append('/');
    printer->m_offsets.append('/');
  }
  printer->m_c_src_string = c_src_string;
  str_builder_init(&printer->m_str_builder);
}

void
fcc_subplan_printer_release(fcc_subplan_printer_t* printer)
{
  str_builder_release(&printer->m_str_builder);
}

void
fcc_subplan_printer_print(fcc_subplan_printer_t* printer,
                          const fcc_subplan_t* plan) 
{
  fcc_subplan_printer_print(printer, &plan->m_nodes[plan->m_root]);
}


} 
