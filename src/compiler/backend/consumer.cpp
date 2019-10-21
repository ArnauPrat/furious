

#include "../../common/str_builder.h"
#include "../frontend/operator.h"
#include "backend/codegen.h"
#include "codegen_tools.h"
#include "consumer.h"
#include "fcc_context.h"
#include "producer.h"

namespace furious 
{

void
consume(FILE* fd,
        const fcc_operator_t* op,
        const char* source,
        const fcc_operator_t* caller);

static void
consume_scan(FILE*fd,
             const fcc_operator_t* scan,
             const char* source,
             const fcc_operator_t* caller);

static void
consume_foreach(FILE*fd,
                const fcc_operator_t* foreach,
                const char* source,
                const fcc_operator_t* caller);

static void
consume_join(FILE*fd,
             const fcc_operator_t* join,
             const char* source,
             const fcc_operator_t* caller);

static void
consume_cross_join(FILE*fd,
                   const fcc_operator_t* cross_join,
                   const char* source,
                   const fcc_operator_t* caller);

static void
consume_leftfilter_join(FILE*fd,
                        const fcc_operator_t* left_filter_join,
                        const char* source,
                        const fcc_operator_t* caller);

static void
consume_gather(FILE*fd,
               const fcc_operator_t* gather,
               const char* source,
               const fcc_operator_t* caller);

static void
consume_cascading_gather(FILE*fd,
                         const fcc_operator_t* casc_gather,
                         const char* source,
                         const fcc_operator_t* caller);

static void
consume_fetch(FILE*fd,
        const fcc_operator_t* fetch,
        const char* source,
        const fcc_operator_t* caller);

static void
consume_tag_filter(FILE*fd,
                   const fcc_operator_t* tag_filter,
                   const char* source,
                   const fcc_operator_t* caller);

static void
consume_predicate_filter(FILE*fd,
        const fcc_operator_t* predicate_filter,
        const char* source,
        const fcc_operator_t* caller);

static void
consume_component_filter(FILE*fd,
                         const fcc_operator_t* component_filter,
                         const char* source,
                         const fcc_operator_t* caller);

void 
consume(FILE*fd,
        const fcc_operator_t* fcc_operator,
        const char* source,
        const fcc_operator_t* caller)
{
  switch(fcc_operator->m_type)
  {
    case fcc_operator_type_t::E_SCAN:
      consume_scan(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_FOREACH:
      consume_foreach(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_JOIN:
      consume_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_LEFT_FILTER_JOIN:
      consume_leftfilter_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_CROSS_JOIN:
      consume_cross_join(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_FETCH:
      consume_fetch(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_GATHER:
      consume_gather(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_CASCADING_GATHER:
      consume_cascading_gather(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_TAG_FILTER:
      consume_tag_filter(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_PREDICATE_FILTER:
      consume_predicate_filter(fd, fcc_operator, source, caller);
      break;
    case fcc_operator_type_t::E_COMPONENT_FILTER:
      consume_component_filter(fd, fcc_operator, source, caller);
      break;
  };
}

void 
consume_foreach(FILE*fd, 
        const fcc_operator_t* foreach,
        const char* source,
        const fcc_operator_t* caller)
{

  bool all_globals = true;
  for(uint32_t i = 0; i < foreach->m_columns.size(); ++i)
  {
    if(foreach->m_columns[i].m_type != fcc_column_type_t::E_GLOBAL)
    {
      all_globals = false;
      break;
    }
  }


  int param_index = 0;
  for(uint32_t i = 0; i < foreach->m_columns.size(); ++i) 
  {
    const fcc_column_t* column = &foreach->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_REFERENCE)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                            "<ForEach> operator cannot\
                            be applied to reference column type: \"%s\"", 
                            column->m_ref_name);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
      str_builder_release(&str_builder);
    }
    char tmp[MAX_TYPE_NAME+32];
    uint32_t length = fcc_type_qualified_name(column->m_component_type,
                                                      tmp,
                                                      MAX_TYPE_NAME+32);

    FURIOUS_PERMA_ASSERT( length < (MAX_TYPE_NAME +32) && "Qualified type name exceeds maximum length");

    if(column->m_type == fcc_column_type_t::E_COMPONENT)
    {
      fprintf(fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n", 
              tmp, 
              param_index, 
              tmp, 
              source, 
              param_index);
    }
    else if (column->m_type == fcc_column_type_t::E_GLOBAL)
    {
      fprintf(fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n", 
              tmp, 
              param_index, 
              tmp, 
              source, 
              param_index);
    }

    param_index++;
  }
  fprintf(fd, "\n");

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  //uint32_t size = foreach->p_systems.size();
  //for(uint32_t i = 0; i < size; ++i)
  {
    //const fcc_system_t* info = foreach->p_systems[i];
    const fcc_system_t* info = foreach->m_foreach.p_system;

    char system_name[MAX_TYPE_NAME];
    fcc_type_name(info->m_system_type, 
                  system_name, 
                  MAX_TYPE_NAME);


    char wrapper_name[MAX_SYSTEM_WRAPPER_VARNAME];
    generate_system_wrapper_name(system_name, 
                                 info->m_id,
                                 wrapper_name, 
                                 MAX_SYSTEM_WRAPPER_VARNAME);


    if(all_globals)
    {
      str_builder_append(&str_builder, "%s->run(&context,\n0", 
                            wrapper_name);
    }
    else
    {
      str_builder_append(&str_builder, "%s->run(&context,\n%s->m_start + i", 
                         wrapper_name, 
                         source);
    }

    for(size_t j = 0; j <  foreach->m_columns.size(); ++j) 
    {
      const fcc_column_t* column = &foreach->m_columns[j];
      if(column->m_type == fcc_column_type_t::E_COMPONENT)
      {
        str_builder_append(&str_builder,",\n&data_%d[i]",j);
      }
      else if (column->m_type == fcc_column_type_t::E_GLOBAL)
      {
        str_builder_append(&str_builder,",\ndata_%d",j);
      }
    }
    str_builder_append(&str_builder,");\n"); 
  }

  if(all_globals)
  {
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
  }
  else
  {
    fprintf(fd,
            "if(%s->p_enabled->num_set() == TABLE_BLOCK_SIZE)\n{\n", 
            source);

    fprintf(fd,
            "for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
    fprintf(fd,
            "}\n");
    fprintf(fd,
            "}\n");
    fprintf(fd,
            "else\n{\n");
    fprintf(fd,
            "for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(fd,
            "if(%s->p_enabled->is_set(i))\n{\n", 
            source);
    fprintf(fd,
            "%s",
            str_builder.p_buffer);
    fprintf(fd,
            "}\n");

    fprintf(fd,
            "}\n");

    fprintf(fd,
            "}\n");
  }
  str_builder_release(&str_builder);

}

void 
consume_scan(FILE*fd,
        const fcc_operator_t* scan,
        const char* source,
        const fcc_operator_t* caller)
{
}

void
consume_join(FILE*fd, 
        const fcc_operator_t* join,
        const char* source,
        const fcc_operator_t* caller)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(caller->m_id == join->m_join.m_left) 
  {
    fprintf(fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            source, 
            source); 
  }
  else 
  {

      fprintf(fd,
              "BlockCluster* build = %s.get(%s->m_start);\n",
              hashtable,
              source);
      fprintf(fd,
              "if(build != nullptr)\n{\n");
      char clustername[MAX_CLUSTER_VARNAME];
      const uint32_t length = generate_cluster_name(join,
                                                    clustername,
                                                    MAX_CLUSTER_VARNAME);
      FURIOUS_PERMA_ASSERT(length < MAX_CLUSTER_VARNAME && "Cluster varname exceeds maximum length");
      fprintf(fd,
              "BlockCluster %s(*build);\n", 
              clustername);
      fprintf(fd,
              "%s.append(%s);\n", 
              clustername, 
              source);
      fprintf(fd,
              "if(%s.p_enabled->num_set() != 0)\n{\n", 
              clustername);

      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, "(&%s)", clustername);
      fcc_subplan_t* subplan = join->p_subplan;
      consume(fd,
              &subplan->m_nodes[join->m_parent],
              str_builder.p_buffer,
              join);
      str_builder_release(&str_builder);

      fprintf(fd,"}\n");
      fprintf(fd,"}\n");
  }
}

void
consume_leftfilter_join(FILE*fd, 
        const fcc_operator_t* left_filter_join,
        const char* source,
        const fcc_operator_t* caller)
{
  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(left_filter_join,
                                          hashtable,
                                          MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(caller->m_id == left_filter_join->m_leftfilter_join.m_left) 
  {
    fprintf(fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            source, 
            source); 
  }
  else 
  {
    fprintf(fd,
            "BlockCluster* build = %s.get(%s->m_start);\n",
            hashtable,
            source);
    fprintf(fd,
            "if(build != nullptr)\n{\n");
    char clustername[MAX_CLUSTER_VARNAME];
    const uint32_t length = generate_cluster_name(left_filter_join,
                                                  clustername,
                                                  MAX_CLUSTER_VARNAME);
    FURIOUS_PERMA_ASSERT(length < MAX_CLUSTER_VARNAME && "Cluster varname exceeds maximum length");

    fprintf(fd,
            "BlockCluster %s(*build);\n", 
            clustername);
    fprintf(fd,
            "%s.filter(%s);\n", 
            clustername, 
            source);
    fprintf(fd,
            "if(%s.p_enabled->num_set() != 0)\n{\n", 
            clustername);

    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder, "(&%s)", clustername);
    fcc_subplan_t* subplan = left_filter_join->p_subplan;
    consume(fd,
            &subplan->m_nodes[left_filter_join->m_parent],
            str_builder.p_buffer,
            left_filter_join);
    str_builder_release(&str_builder);

    fprintf(fd,"}\n");
    fprintf(fd,"}\n");
  }
}

void
consume_cross_join(FILE*fd, 
        const fcc_operator_t* join,
        const char* source,
        const fcc_operator_t* caller)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(caller->m_id == join->m_cross_join.m_left) 
  {
    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder,"left_%s", hashtable);
    fprintf(fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            str_builder.p_buffer, 
            source, 
            source); 
    str_builder_release(&str_builder);
  }
  else 
  {
    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder,"right_%s", hashtable);
    fprintf(fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            str_builder.p_buffer, 
            source, 
            source); 
    str_builder_release(&str_builder);
  }
}

void
consume_fetch(FILE*fd, 
              const fcc_operator_t* fetch,
              const char* source,
              const fcc_operator_t* caller)
{


}

void 
consume_tag_filter(FILE*fd, 
                   const fcc_operator_t* tag_filter,
                   const char* source,
                   const fcc_operator_t* caller)
{

  char bittable_name[MAX_TAG_TABLE_VARNAME];
  const uint32_t length =  generate_bittable_name(tag_filter->m_tag_filter.m_tag,
                                                  bittable_name,
                                                  MAX_TAG_TABLE_VARNAME);
  FURIOUS_PERMA_ASSERT( length < MAX_TAG_TABLE_VARNAME && "Tag table varname exceeds maximum length");
  fprintf(fd,"\n");

  if(!tag_filter->m_tag_filter.m_on_column)
  {
    fprintf(fd,
            "const Bitmap* filter = %s->get_bitmap(%s->m_start);\n", 
            bittable_name, 
            source);
    switch(tag_filter->m_tag_filter.m_op_type) 
    {
      case fcc_filter_op_type_t::E_HAS:
        {
          fprintf(fd,
                  "%s->p_enabled->set_and(filter);\n",
                  source);
          break;
        }
      case fcc_filter_op_type_t::E_HAS_NOT:
        {
          fprintf(fd,"{\n");
          fprintf(fd, 
                  "Bitmap negate(TABLE_BLOCK_SIZE);\n");
          fprintf(fd, 
                  "negate.set_bitmap(filter);\n");
          fprintf(fd, 
                  "negate.set_negate();\n");
          fprintf(fd,
                  "%s->p_enabled->set_and(&negate);\n",
                  source);
          fprintf(fd,"}\n");
          break;
        }
    }
  }
  else
  {
    if(tag_filter->m_columns[0].m_type != fcc_column_type_t::E_REFERENCE)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder,"Cannot apply filter tag \"%s\" on column\
                         on a non-reference column type", 
                         tag_filter->m_tag_filter.m_tag);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                          str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    switch(tag_filter->m_tag_filter.m_op_type) 
    {
      case fcc_filter_op_type_t::E_HAS:
        {
          fprintf(fd,
                  "filter_bittable_exists(%s,%s,0);\n",
                  bittable_name,
                  source);
          break;
        }
      case fcc_filter_op_type_t::E_HAS_NOT:
        {
          fprintf(fd,
                  "filter_bittable_not_exists(%s,%s,0);\n",
                  bittable_name,
                  source);
          break;
        }
    }
  }

  fprintf(fd,
          "if(%s->p_enabled->num_set() != 0)\n{\n",
          source); 
  fcc_subplan_t* subplan = tag_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[tag_filter->m_parent],
          source,
          tag_filter);
  fprintf(fd,"}\n");
}

void
consume_component_filter(FILE*fd, 
        const fcc_operator_t* component_filter,
        const char* source,
        const fcc_operator_t* caller)
{
  fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                     "Component filter not yet implemented");
  // if ...
  fcc_subplan_t* subplan = component_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[component_filter->m_parent],
          "cluster",
          component_filter);
}

void
consume_predicate_filter(FILE*fd, 
                         const fcc_operator_t* predicate_filter,
                         const char* source,
                         const fcc_operator_t* caller)
{
  // if ...
  fprintf(fd,"\n");
  int param_index = 0;
  for(uint32_t i = 0; i < predicate_filter->m_columns.size(); ++i)  
  {
    const fcc_column_t* column = &predicate_filter->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_REFERENCE)
    {
      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, 
                            "<PredicateFilter> operator cannot be applied to \
                            reference column type: \"%s\"", 
                            column->m_ref_name);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    char tmp[MAX_QUALIFIED_TYPE_NAME];
    fcc_type_qualified_name(column->m_component_type,
                            tmp,
                            MAX_QUALIFIED_TYPE_NAME);

    if(column->m_type == fcc_column_type_t::E_COMPONENT)
    {
      fprintf(fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n",
              tmp,
              param_index,
              tmp,
              source,
              param_index);
    }
    else
    {
      fprintf(fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n",
              tmp,
              param_index,
              tmp,
              source,
              param_index);
    }
    param_index++;
  }

  if(fcc_decl_is_function(predicate_filter->m_predicate_filter.m_func_decl) && 
     fcc_decl_is_member(predicate_filter->m_predicate_filter.m_func_decl))
  {
    uint32_t code_length = 4096;
    char* code = new char[code_length];
    uint32_t length = 0;
    while((length = fcc_decl_code(predicate_filter->m_predicate_filter.m_func_decl, code, code_length)) >= code_length)
    {
      delete [] code;
      code_length *= 2;
      code = new char[code_length];
    }

    fprintf(fd, "%s", code);
    delete [] code;
  }

  char func_name[2048];
  fcc_decl_function_name(predicate_filter->m_predicate_filter.m_func_decl, 
                         func_name, 
                         2048);


  fprintf(fd,
          "for(uint32_t i = 0; i < TABLE_BLOCK_SIZE && (%s->p_enabled->num_set() != 0); ++i) \n{\n",
          source);
  fprintf(fd,
          "%s->p_enabled->set_bit(i, %s->p_enabled->is_set(i) && %s(",
          source,
          source,
          func_name);

  const fcc_column_t* column = &predicate_filter->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    fprintf(fd, "&data_0[i]");
  }
  else
  {
    fprintf(fd, "data_0");
  }
  for(size_t i = 1; i <predicate_filter->m_columns.size(); ++i)
  {
    const fcc_column_t* column = &predicate_filter->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_COMPONENT)
    {
      fprintf(fd, ",&data_%zu[i]", i);
    }
    else
    {
      fprintf(fd, ",data_%zu", i);
    }
  }
  fprintf(fd, "));\n");
  fprintf(fd, "}\n");
  fprintf(fd,
          "if(%s->p_enabled->num_set() != 0)\n{\n",
          source); 
  fcc_subplan_t* subplan = predicate_filter->p_subplan;
  consume(fd,
          &subplan->m_nodes[predicate_filter->m_parent],
          source,
          predicate_filter);
  fprintf(fd, "}\n");
}

void
consume_gather(FILE*fd, 
        const fcc_operator_t* gather,
        const char* source,
        const fcc_operator_t* caller)
{
  char groups_varname[MAX_REF_TABLE_VARNAME];
  fcc_subplan_t* subplan = gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[gather->m_gather.m_ref_table];
  const uint32_t length = generate_ref_groups_name(ref_table->m_columns[0].m_ref_name, 
                                                   groups_varname,
                                                   MAX_REF_TABLE_VARNAME,
                                                   gather);

  FURIOUS_PERMA_ASSERT(length < MAX_REF_TABLE_VARNAME && "Ref table varname exceeded maximum length");


  if(caller->m_id == gather->m_gather.m_ref_table) 
  {
    // perform the group by of the references
    fprintf(fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups_varname,
            source);
  }
  else
  {
    // perform the gather using the grouped references
    fprintf(fd,"gather(&%s,%s",
            groups_varname,
            source);
    fcc_operator_t* child = &subplan->m_nodes[gather->m_gather.m_child];
    DynArray<fcc_column_t>& child_columns = child->m_columns;
    for(uint32_t i = 0; i < child_columns.size(); ++i)
    {
      fcc_column_t* column = &child_columns[i];
      char tmp[MAX_TYPE_NAME];
      fcc_type_name(column->m_component_type,
                    tmp,
                    MAX_TYPE_NAME); 

      char tablename[MAX_TABLE_VARNAME];
      generate_temp_table_name(tmp,
                               tablename,
                               MAX_TABLE_VARNAME,
                               gather);

      fprintf(fd,",&%s",
              tablename);
    }
    fprintf(fd,");\n");
  }
}

void
consume_cascading_gather(FILE*fd, 
                         const fcc_operator_t* casc_gather,
                         const char* source,
                         const fcc_operator_t* caller)
{

  char groups_varname[MAX_REF_TABLE_VARNAME];
  fcc_subplan_t* subplan = casc_gather->p_subplan;
  fcc_operator_t* ref_table = &subplan->m_nodes[casc_gather->m_cascading_gather.m_ref_table];
  const uint32_t length = generate_ref_groups_name(ref_table->m_columns[0].m_ref_name, 
                                                   groups_varname,
                                                   MAX_REF_TABLE_VARNAME,
                                                   casc_gather);

  FURIOUS_PERMA_ASSERT(length < MAX_REF_TABLE_VARNAME && "Ref table varname exceeded maximum length");
  if(caller->m_id == casc_gather->m_cascading_gather.m_ref_table) 
  {
    // perform the group by of the references
    fprintf(fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups_varname,
            source);
  }
  else
  {
    char hashtable[MAX_HASHTABLE_VARNAME];
    const uint32_t length = generate_hashtable_name(casc_gather,
                                                    hashtable,
                                                    MAX_HASHTABLE_VARNAME);
    FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable name exceeds maximum length");

    fprintf(fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            source, 
            source); 
  }
}

} /* furious */ 
