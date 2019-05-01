

#include "../../common/string_builder.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "fcc_context.h"
#include "clang_tools.h"
#include "codegen_tools.h"
#include "codegen.h"

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
    if(foreach->m_columns[i].m_type != FccColumnType::E_GLOBAL)
    {
      all_globals = false;
      break;
    }
  }

  int param_index = 0;
  for(uint32_t i = 0; i < foreach->m_columns.size(); ++i) 
  {
    const FccColumn* column = &foreach->m_columns[i];
    if(column->m_type == FccColumnType::E_REFERENCE)
    {
      StringBuilder str_builder;
      str_builder.append("<ForEach> operator cannot be applied to reference column type: \"%s\"", 
                         column->m_ref_name.c_str());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }
    const std::string& type = get_qualified_type_name(column->m_q_type);

    if(column->m_type == FccColumnType::E_COMPONENT)
    {
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n", 
              type.c_str(), 
              param_index, 
              type.c_str(), 
              p_context->m_source.c_str(), 
              param_index);
    }
    else if (column->m_type == FccColumnType::E_GLOBAL)
    {
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n", 
              type.c_str(), 
              param_index, 
              type.c_str(), 
              p_context->m_source.c_str(), 
              param_index);
    }

    param_index++;
  }
  fprintf(p_context->p_fd, "\n");

  StringBuilder str_builder;
  uint32_t size = foreach->p_systems.size();
  for(uint32_t i = 0; i < size; ++i)
  {
    const FccSystem* info = foreach->p_systems[i];

    std::string system_name  = get_type_name(info->m_system_type);
    std::string wrapper_name = generate_system_wrapper_name(system_name, 
                                                            info->m_id);

    if(all_globals)
    {
      str_builder.append("%s->run(&context,\n0", 
                         wrapper_name.c_str());
    }
    else
    {
      str_builder.append("%s->run(&context,\n%s->m_start + i", 
                         wrapper_name.c_str(), 
                         p_context->m_source.c_str());
    }

    for(size_t i = 0; i <  foreach->m_columns.size(); ++i) 
    {
      const FccColumn* column = &foreach->m_columns[i];
      if(column->m_type == FccColumnType::E_COMPONENT)
      {
        str_builder.append(",\n&data_%d[i]",i);
      }
      else if (column->m_type == FccColumnType::E_GLOBAL)
      {
        str_builder.append(",\ndata_%d",i);
      }
    }
    str_builder.append(");\n"); 
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
            p_context->m_source.c_str());

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
            p_context->m_source.c_str());
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
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{

  std::string hashtable = generate_hashtable_name(join);
  if(p_context->p_caller == join->p_left.get()) 
  {
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable.c_str(), 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str()); 
  }
  else 
  {

      fprintf(p_context->p_fd,
              "BlockCluster* build = %s.get(%s->m_start);\n",
              hashtable.c_str(),
              p_context->m_source.c_str());
      fprintf(p_context->p_fd,
              "if(build != nullptr)\n{\n");
      std::string clustername = generate_cluster_name(join);
      fprintf(p_context->p_fd,
              "BlockCluster %s(*build);\n", 
              clustername.c_str());
      fprintf(p_context->p_fd,
              "%s.append(%s);\n", 
              clustername.c_str(), 
              p_context->m_source.c_str());
      fprintf(p_context->p_fd,
              "if(%s.p_enabled->num_set() != 0)\n{\n", 
              clustername.c_str());

      consume(p_context->p_fd,
              join->p_parent,
              "(&"+clustername+")",
              join);

      fprintf(p_context->p_fd,"}\n");
      fprintf(p_context->p_fd,"}\n");
  }
}

void
ConsumeVisitor::visit(const LeftFilterJoin* left_filter_join)
{
  std::string hashtable = generate_hashtable_name(left_filter_join);
  if(p_context->p_caller == left_filter_join->p_left.get()) 
  {
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable.c_str(), 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str()); 
  }
  else 
  {
    fprintf(p_context->p_fd,
            "BlockCluster* build = %s.get(%s->m_start);\n",
            hashtable.c_str(),
            p_context->m_source.c_str());
    fprintf(p_context->p_fd,
            "if(build != nullptr)\n{\n");
    std::string clustername = generate_cluster_name(left_filter_join);
    fprintf(p_context->p_fd,
            "BlockCluster %s(*build);\n", 
            clustername.c_str());
    fprintf(p_context->p_fd,
            "%s.filter(%s);\n", 
            clustername.c_str(), 
            p_context->m_source.c_str());
    fprintf(p_context->p_fd,
            "if(%s.p_enabled->num_set() != 0)\n{\n", 
            clustername.c_str());

    consume(p_context->p_fd,
            left_filter_join->p_parent,
            "(&"+clustername+")",
            left_filter_join);

    fprintf(p_context->p_fd,"}\n");
    fprintf(p_context->p_fd,"}\n");
  }
}

void
ConsumeVisitor::visit(const CrossJoin* join)
{

  if(p_context->p_caller == join->p_left.get()) 
  {

    std::string hashtable = "left_"+generate_hashtable_name(join);
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable.c_str(), 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str()); 
  }
  else 
  {
    std::string hashtable = "right_"+generate_hashtable_name(join);
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable.c_str(), 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str()); 
  }
}

void
ConsumeVisitor::visit(const Fetch* fetch)
{


}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  const std::string bittable_name = generate_bittable_name(tag_filter->m_tag);
  fprintf(p_context->p_fd,"\n");

  if(!tag_filter->m_on_column)
  {
    fprintf(p_context->p_fd,
            "const Bitmap* filter = %s->get_bitmap(%s->m_start);\n", 
            bittable_name.c_str(), 
            p_context->m_source.c_str());
    switch(tag_filter->m_op_type) 
    {
      case FccFilterOpType::E_HAS:
        {
          fprintf(p_context->p_fd,
                  "%s->p_enabled->set_and(filter);\n",
                  p_context->m_source.c_str());
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
                  p_context->m_source.c_str());
          fprintf(p_context->p_fd,"}\n");
          break;
        }
    }
  }
  else
  {
    if(tag_filter->m_columns[0].m_type != FccColumnType::E_REFERENCE)
    {
      StringBuilder str_builder;
      str_builder.append("Cannot apply filter tag \"%s\" on column on a non-reference column type", tag_filter->m_tag.c_str());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                          str_builder.p_buffer);
    }

    switch(tag_filter->m_op_type) 
    {
      case FccFilterOpType::E_HAS:
        {
          fprintf(p_context->p_fd,
                  "filter_bittable_exists(%s,%s,0);\n",
                  bittable_name.c_str(),
                  p_context->m_source.c_str());
          break;
        }
      case FccFilterOpType::E_HAS_NOT:
        {
          fprintf(p_context->p_fd,
                  "filter_bittable_not_exists(%s,%s,0);\n",
                  bittable_name.c_str(),
                  p_context->m_source.c_str());
          break;
        }
    }
  }

  fprintf(p_context->p_fd,
          "if(%s->p_enabled->num_set() != 0)\n{\n",
          p_context->m_source.c_str()); 
  consume(p_context->p_fd,
          tag_filter->p_parent,
          p_context->m_source,
          tag_filter);
  fprintf(p_context->p_fd,"}\n");
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
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
    const FccColumn* column = &predicate_filter->m_columns[i];
    if(column->m_type == FccColumnType::E_REFERENCE)
    {
      StringBuilder str_builder;
      str_builder.append("<PredicateFilter> operator cannot be applied to reference column type: \"%s\"", 
                         column->m_ref_name.c_str());
      p_fcc_context->report_compilation_error(FccCompilationErrorType::E_INVALID_COLUMN_TYPE,
                                                         str_builder.p_buffer);
    }
    if(column->m_type == FccColumnType::E_COMPONENT)
    {
      const std::string& type = get_qualified_type_name(column->m_q_type);
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_tblock(%d)->p_data);\n",
              type.c_str(),
              param_index,
              type.c_str(),
              p_context->m_source.c_str(),
              param_index);
    }
    else
    {
      const std::string& type = get_qualified_type_name(column->m_q_type);
      fprintf(p_context->p_fd,
              "%s* data_%d = (%s*)(%s->get_global(%d));\n",
              type.c_str(),
              param_index,
              type.c_str(),
              p_context->m_source.c_str(),
              param_index);
    }
    param_index++;
  }
  std::string func_name = "";
  if(!predicate_filter->p_func_decl->isCXXClassMember())
  {
    const FunctionDecl* func_decl = predicate_filter->p_func_decl;
    func_name = func_decl->getName();
  } 
  else
  {
    fprintf(p_context->p_fd,"auto predicate = [] (");
    const FunctionDecl* func_decl = predicate_filter->p_func_decl;
    auto array = func_decl->parameters();
    fprintf(p_context->p_fd, "%s%s", get_type_name(array[0]->getType()).c_str(), array[0]->getNameAsString().c_str());
    for(size_t i = 1; i < array.size(); ++i)
    {
      fprintf(p_context->p_fd ,",%s%s", get_type_name(array[i]->getType()).c_str(), array[i]->getNameAsString().c_str());
    }
    const ASTContext& context = predicate_filter->p_func_decl->getASTContext();
    const SourceManager& sm = context.getSourceManager();

    fprintf(p_context->p_fd, 
            "%s;\n",
            get_code(sm, predicate_filter->p_func_decl->getSourceRange()).c_str());
    func_name = "predicate";
  }

  fprintf(p_context->p_fd,
          "for(uint32_t i = 0; i < TABLE_BLOCK_SIZE && (%s->p_enabled->num_set() != 0); ++i) \n{\n",
          p_context->m_source.c_str());
  fprintf(p_context->p_fd,
          "%s->p_enabled->set_bit(i, %s->p_enabled->is_set(i) && %s(",
          p_context->m_source.c_str(),
          p_context->m_source.c_str(),
          func_name.c_str());

  const FccColumn* column = &predicate_filter->m_columns[0];
  if(column->m_type == FccColumnType::E_COMPONENT)
  {
    fprintf(p_context->p_fd, "&data_0[i]");
  }
  else
  {
    fprintf(p_context->p_fd, "data_0");
  }
  for(size_t i = 1; i <predicate_filter->m_columns.size(); ++i)
  {
    const FccColumn* column = &predicate_filter->m_columns[i];
    if(column->m_type == FccColumnType::E_COMPONENT)
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
          p_context->m_source.c_str()); 
  consume(p_context->p_fd,
          predicate_filter->p_parent,
          p_context->m_source,
          predicate_filter);
  fprintf(p_context->p_fd, "}\n");
}

void
ConsumeVisitor::visit(const Gather* gather)
{

  std::string groups = generate_ref_groups_name(gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                gather);
  if(p_context->p_caller == gather->p_ref_table.get()) 
  {
    // perform the group by of the references
    fprintf(p_context->p_fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups.c_str(),
            p_context->m_source.c_str());
  }
  else
  {
    // perform the gather using the grouped references
    fprintf(p_context->p_fd,"gather(&%s,%s",
            groups.c_str(),
            p_context->m_source.c_str());
    DynArray<FccColumn>& child_columns = gather->p_child.get()->m_columns;
    for(uint32_t i = 0; i < child_columns.size(); ++i)
    {
      FccColumn* column = &child_columns[i];
      std::string component_name = get_type_name(column->m_q_type); 
      std::string temp_table_name = generate_temp_table_name(component_name, gather);
      fprintf(p_context->p_fd,",&%s",
              temp_table_name.c_str());
    }
    fprintf(p_context->p_fd,");\n");
  }
}

void
ConsumeVisitor::visit(const CascadingGather* casc_gather)
{

  std::string groups = generate_ref_groups_name(casc_gather->p_ref_table.get()->m_columns[0].m_ref_name, 
                                                casc_gather);
  if(p_context->p_caller == casc_gather->p_ref_table.get()) 
  {
    // perform the group by of the references
    fprintf(p_context->p_fd,
            "group_references(&%s, %s->get_tblock(0));\n", 
            groups.c_str(),
            p_context->m_source.c_str());
  }
  else
  {
    std::string hashtable = generate_hashtable_name(casc_gather);
    fprintf(p_context->p_fd, 
            "%s.insert_copy(%s->m_start, %s);\n", 
            hashtable.c_str(), 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str()); 
  }
}

} /* furious */ 
