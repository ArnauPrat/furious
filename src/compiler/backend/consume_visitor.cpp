

#include "../../common/str_builder.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "fcc_context.h"
#include "drivers/clang/clang_tools.h"
#include "backend/codegen.h"
#include "codegen_tools.h"
#include "../frontend/operator.h"

namespace furious 
{


ConsumeVisitor::ConsumeVisitor(CodeGenContext* context) :
p_context{context}
{
}


void 
ConsumeVisitor::visit(const Foreach* foreach)
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
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n", 
              tmp, 
              param_index, 
              tmp, 
              p_context->m_source, 
              param_index);
    }
    else if (column->m_type == fcc_column_type_t::E_GLOBAL)
    {
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n", 
              tmp, 
              param_index, 
              tmp, 
              p_context->m_source, 
              param_index);
    }

    param_index++;
  }
  fprintf(p_context->p_fd, "\n");

  str_builder_t str_builder;
  str_builder_init(&str_builder);
  uint32_t size = foreach->p_systems.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const fcc_system_t* info = foreach->p_systems[i];

    char system_name[MAX_TYPE_NAME];
    const uint32_t system_length = fcc_type_name(info->m_system_type, 
                                                 system_name, 
                                                 MAX_TYPE_NAME);

    FURIOUS_CHECK_STR_LENGTH(system_length, MAX_TYPE_NAME);

    char wrapper_name[MAX_SYSTEM_WRAPPER_VARNAME];
    const uint32_t wrapper_length = generate_system_wrapper_name(system_name, 
                                          info->m_id,
                                          wrapper_name, 
                                          MAX_SYSTEM_WRAPPER_VARNAME);

    FURIOUS_CHECK_STR_LENGTH(wrapper_length, MAX_SYSTEM_WRAPPER_VARNAME);


    if(all_globals)
    {
      str_builder_append(&str_builder, "%s->run(&context,\n0", 
                            wrapper_name);
    }
    else
    {
      str_builder_append(&str_builder, "%s->run(&context,\n%s->m_start + i", 
                         wrapper_name, 
                         p_context->m_source);
    }

    for(size_t i = 0; i <  foreach->m_columns.size(); ++i) 
    {
      const fcc_column_t* column = &foreach->m_columns[i];
      if(column->m_type == fcc_column_type_t::E_COMPONENT)
      {
        str_builder_append(&str_builder,",\n&data_%d[i]",i);
      }
      else if (column->m_type == fcc_column_type_t::E_GLOBAL)
      {
        str_builder_append(&str_builder,",\ndata_%d",i);
      }
    }
    str_builder_append(&str_builder,");\n"); 
  }

  if(all_globals)
  {
    fprintf(p_context->p_fd,
            "%s",
            str_builder.p_buffer);
  }
  else
  {
    fprintf(p_context->p_fd,
            "if(%s->p_enabled->num_set() == TABLE_BLOCK_SIZE)\n{\n", 
            p_context->m_source);

    fprintf(p_context->p_fd,
            "for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(p_context->p_fd,
            "%s",
            str_builder.p_buffer);
    fprintf(p_context->p_fd,
            "}\n");
    fprintf(p_context->p_fd,
            "}\n");
    fprintf(p_context->p_fd,
            "else\n{\n");
    fprintf(p_context->p_fd,
            "for (size_t i = 0; i < TABLE_BLOCK_SIZE; ++i)\n{\n");
    fprintf(p_context->p_fd,
            "if(%s->p_enabled->is_set(i))\n{\n", 
            p_context->m_source);
    fprintf(p_context->p_fd,
            "%s",
            str_builder.p_buffer);
    fprintf(p_context->p_fd,
            "}\n");

    fprintf(p_context->p_fd,
            "}\n");

    fprintf(p_context->p_fd,
            "}\n");
  }
  str_builder_release(&str_builder);
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(p_context->p_caller == join->p_left.get()) 
  {
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            p_context->m_source, 
            p_context->m_source); 
  }
  else 
  {

      fprintf(p_context->p_fd,
              "BlockCluster* build = %s.get(%s->m_start);\n",
              hashtable,
              p_context->m_source);
      fprintf(p_context->p_fd,
              "if(build != nullptr)\n{\n");
      char clustername[MAX_CLUSTER_VARNAME];
      const uint32_t length = generate_cluster_name(join,
                                                    clustername,
                                                    MAX_CLUSTER_VARNAME);
      FURIOUS_PERMA_ASSERT(length < MAX_CLUSTER_VARNAME && "Cluster varname exceeds maximum length");
      fprintf(p_context->p_fd,
              "BlockCluster %s(*build);\n", 
              clustername);
      fprintf(p_context->p_fd,
              "%s.append(%s);\n", 
              clustername, 
              p_context->m_source);
      fprintf(p_context->p_fd,
              "if(%s.p_enabled->num_set() != 0)\n{\n", 
              clustername);

      str_builder_t str_builder;
      str_builder_init(&str_builder);
      str_builder_append(&str_builder, "(&%s)", clustername);
      consume(p_context->p_fd,
              join->p_parent,
              str_builder.p_buffer,
              join);
      str_builder_release(&str_builder);

      fprintf(p_context->p_fd,"}\n");
      fprintf(p_context->p_fd,"}\n");
  }
}

void
ConsumeVisitor::visit(const LeftFilterJoin* left_filter_join)
{
  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(left_filter_join,
                                          hashtable,
                                          MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(p_context->p_caller == left_filter_join->p_left.get()) 
  {
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            p_context->m_source, 
            p_context->m_source); 
  }
  else 
  {
    fprintf(p_context->p_fd,
            "BlockCluster* build = %s.get(%s->m_start);\n",
            hashtable,
            p_context->m_source);
    fprintf(p_context->p_fd,
            "if(build != nullptr)\n{\n");
    char clustername[MAX_CLUSTER_VARNAME];
    const uint32_t length = generate_cluster_name(left_filter_join,
                                                  clustername,
                                                  MAX_CLUSTER_VARNAME);
    FURIOUS_PERMA_ASSERT(length < MAX_CLUSTER_VARNAME && "Cluster varname exceeds maximum length");

    fprintf(p_context->p_fd,
            "BlockCluster %s(*build);\n", 
            clustername);
    fprintf(p_context->p_fd,
            "%s.filter(%s);\n", 
            clustername, 
            p_context->m_source);
    fprintf(p_context->p_fd,
            "if(%s.p_enabled->num_set() != 0)\n{\n", 
            clustername);

    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder, "(&%s)", clustername);
    consume(p_context->p_fd,
            left_filter_join->p_parent,
            str_builder.p_buffer,
            left_filter_join);
    str_builder_release(&str_builder);

    fprintf(p_context->p_fd,"}\n");
    fprintf(p_context->p_fd,"}\n");
  }
}

void
ConsumeVisitor::visit(const CrossJoin* join)
{

  char hashtable[MAX_HASHTABLE_VARNAME];
  const uint32_t length = generate_hashtable_name(join,
                                                  hashtable,
                                                  MAX_HASHTABLE_VARNAME);

  FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable varname exceeds maximum length");

  if(p_context->p_caller == join->p_left.get()) 
  {
    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder,"left_%s", hashtable);
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            str_builder.p_buffer, 
            p_context->m_source, 
            p_context->m_source); 
    str_builder_release(&str_builder);
  }
  else 
  {
    str_builder_t str_builder;
    str_builder_init(&str_builder);
    str_builder_append(&str_builder,"right_%s", hashtable);
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            str_builder.p_buffer, 
            p_context->m_source, 
            p_context->m_source); 
    str_builder_release(&str_builder);
  }
}

void
ConsumeVisitor::visit(const Fetch* fetch)
{


}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{

  char bittable_name[MAX_TAG_TABLE_VARNAME];
  const uint32_t length =  generate_bittable_name(tag_filter->m_tag,
                                                  bittable_name,
                                                  MAX_TAG_TABLE_VARNAME);
  FURIOUS_PERMA_ASSERT( length < MAX_TAG_TABLE_VARNAME && "Tag table varname exceeds maximum length");
  fprintf(p_context->p_fd,"\n");

  if(!tag_filter->m_on_column)
  {
    fprintf(p_context->p_fd,
            "const Bitmap* filter = %s->get_bitmap(%s->m_start);\n", 
            bittable_name, 
            p_context->m_source);
    switch(tag_filter->m_op_type) 
    {
      case FccFilterOpType::E_HAS:
        {
          fprintf(p_context->p_fd,
                  "%s->p_enabled->set_and(filter);\n",
                  p_context->m_source);
          break;
        }
      case FccFilterOpType::E_HAS_NOT:
        {
          fprintf(p_context->p_fd,"{\n");
          fprintf(p_context->p_fd, 
                  "Bitmap negate(TABLE_BLOCK_SIZE);\n");
          fprintf(p_context->p_fd, 
                  "negate.set_bitmap(filter);\n");
          fprintf(p_context->p_fd, 
                  "negate.set_negate();\n");
          fprintf(p_context->p_fd,
                  "%s->p_enabled->set_and(&negate);\n",
                  p_context->m_source);
          fprintf(p_context->p_fd,"}\n");
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
                         tag_filter->m_tag);
      fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                          str_builder.p_buffer);
      str_builder_release(&str_builder);
    }

    switch(tag_filter->m_op_type) 
    {
      case FccFilterOpType::E_HAS:
        {
          fprintf(p_context->p_fd,
                  "filter_bittable_exists(%s,%s,0);\n",
                  bittable_name,
                  p_context->m_source);
          break;
        }
      case FccFilterOpType::E_HAS_NOT:
        {
          fprintf(p_context->p_fd,
                  "filter_bittable_not_exists(%s,%s,0);\n",
                  bittable_name,
                  p_context->m_source);
          break;
        }
    }
  }

  fprintf(p_context->p_fd,
          "if(%s->p_enabled->num_set() != 0)\n{\n",
          p_context->m_source); 
  consume(p_context->p_fd,
          tag_filter->p_parent,
          p_context->m_source,
          tag_filter);
  fprintf(p_context->p_fd,"}\n");
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  fcc_context_report_compilation_error(fcc_compilation_error_type_t::E_INVALID_COLUMN_TYPE,
                                                     "Component filter not yet implemented");
  // if ...
  consume(p_context->p_fd,
          component_filter->p_parent,
          "cluster",
          component_filter);
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
  // if ...
  fprintf(p_context->p_fd,"\n");
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
    const uint32_t length = fcc_type_qualified_name(column->m_component_type,
                                                    tmp,
                                                    MAX_QUALIFIED_TYPE_NAME);
    FURIOUS_CHECK_STR_LENGTH(length, MAX_QUALIFIED_TYPE_NAME);

    if(column->m_type == fcc_column_type_t::E_COMPONENT)
    {
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n",
              tmp,
              param_index,
              tmp,
              p_context->m_source,
              param_index);
    }
    else
    {
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n",
              tmp,
              param_index,
              tmp,
              p_context->m_source,
              param_index);
    }
    param_index++;
  }

  if(fcc_decl_is_function(predicate_filter->m_func_decl) && 
     fcc_decl_is_member(predicate_filter->m_func_decl))
  {
    uint32_t code_length = 4096;
    char* code = new char[code_length];
    uint32_t length = 0;
    while((length = fcc_decl_code(predicate_filter->m_func_decl, code, code_length)) >= code_length)
    {
      delete [] code;
      code_length *= 2;
      code = new char[code_length];
    }

    fprintf(p_context->p_fd, "%s", code);
    delete [] code;
  }

  char func_name[2048];
  uint32_t length = fcc_decl_function_name(predicate_filter->m_func_decl, func_name, 2048);
  FURIOUS_CHECK_STR_LENGTH(length, 2048);


  fprintf(p_context->p_fd,
          "for(uint32_t i = 0; i < TABLE_BLOCK_SIZE && (%s->p_enabled->num_set() != 0); ++i) \n{\n",
          p_context->m_source);
  fprintf(p_context->p_fd,
          "%s->p_enabled->set_bit(i, %s->p_enabled->is_set(i) && %s(",
          p_context->m_source,
          p_context->m_source,
          func_name);

  const fcc_column_t* column = &predicate_filter->m_columns[0];
  if(column->m_type == fcc_column_type_t::E_COMPONENT)
  {
    fprintf(p_context->p_fd, "&data_0[i]");
  }
  else
  {
    fprintf(p_context->p_fd, "data_0");
  }
  for(size_t i = 1; i <predicate_filter->m_columns.size(); ++i)
  {
    const fcc_column_t* column = &predicate_filter->m_columns[i];
    if(column->m_type == fcc_column_type_t::E_COMPONENT)
    {
      fprintf(p_context->p_fd, ",&data_%zu[i]", i);
    }
    else
    {
      fprintf(p_context->p_fd, ",data_%zu", i);
    }
  }
  fprintf(p_context->p_fd, "));\n");
  fprintf(p_context->p_fd, "}\n");
  fprintf(p_context->p_fd,
          "if(%s->p_enabled->num_set() != 0)\n{\n",
          p_context->m_source); 
  consume(p_context->p_fd,
          predicate_filter->p_parent,
          p_context->m_source,
          predicate_filter);
  fprintf(p_context->p_fd, "}\n");
}

void
ConsumeVisitor::visit(const Gather* gather)
{
  char groups_varname[MAX_REF_TABLE_VARNAME];
  const uint32_t length = generate_ref_groups_name(gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                   groups_varname,
                                                   MAX_REF_TABLE_VARNAME,
                                                   gather);

  FURIOUS_PERMA_ASSERT(length < MAX_REF_TABLE_VARNAME && "Ref table varname exceeded maximum length");


  if(p_context->p_caller == gather->p_ref_table.get()) 
  {
    // perform the group by of the references
    fprintf(p_context->p_fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups_varname,
            p_context->m_source);
  }
  else
  {
    // perform the gather using the grouped references
    fprintf(p_context->p_fd,"gather(&%s,%s",
            groups_varname,
            p_context->m_source);
    DynArray<fcc_column_t>& child_columns = gather->p_child.get()->m_columns;
    for(uint32_t i = 0; i < child_columns.size(); ++i)
    {
      fcc_column_t* column = &child_columns[i];
      char tmp[MAX_TYPE_NAME];
      uint32_t length = fcc_type_name(column->m_component_type,
                                                 tmp,
                                                 MAX_TYPE_NAME); 
      FURIOUS_CHECK_STR_LENGTH(length, MAX_TYPE_NAME);

      char tablename[MAX_TABLE_VARNAME];
      length = generate_temp_table_name(tmp,
                                        tablename,
                                        MAX_TABLE_VARNAME,
                                        gather);

      FURIOUS_CHECK_STR_LENGTH(length, MAX_TABLE_VARNAME);

      fprintf(p_context->p_fd,",&%s",
              tablename);
    }
    fprintf(p_context->p_fd,");\n");
  }
}

void
ConsumeVisitor::visit(const CascadingGather* casc_gather)
{

  char groups_varname[MAX_REF_TABLE_VARNAME];
  const uint32_t length = generate_ref_groups_name(casc_gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                   groups_varname,
                                                   MAX_REF_TABLE_VARNAME,
                                                   casc_gather);

  FURIOUS_PERMA_ASSERT(length < MAX_REF_TABLE_VARNAME && "Ref table varname exceeded maximum length");
  if(p_context->p_caller == casc_gather->p_ref_table.get()) 
  {
    // perform the group by of the references
    fprintf(p_context->p_fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups_varname,
            p_context->m_source);
  }
  else
  {
    char hashtable[MAX_HASHTABLE_VARNAME];
    const uint32_t length = generate_hashtable_name(casc_gather,
                                                    hashtable,
                                                    MAX_HASHTABLE_VARNAME);
    FURIOUS_PERMA_ASSERT(length < MAX_HASHTABLE_VARNAME && "Hashtable name exceeds maximum length");

    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable, 
            p_context->m_source, 
            p_context->m_source); 
  }
}

} /* furious */ 
