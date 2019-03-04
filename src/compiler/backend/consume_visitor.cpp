

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
  int param_index = 0;
  for(uint32_t i = 0; i < foreach->m_columns.size(); ++i) 
  {
    const FccColumn* column = &foreach->m_columns[i];
    if(column->m_type == FccColumnType::E_REFERENCE)
    {
      // TODO:report error
    }
    const std::string& type = get_qualified_type_name(column->m_q_type);

    fprintf(p_context->p_fd,
            "%s* data_%d = (%s*)(%s->m_blocks[%d]->p_data);\n", 
            type.c_str(), 
            param_index, 
            type.c_str(), 
            p_context->m_source.c_str(), 
            param_index);

    param_index++;
  }
  fprintf(p_context->p_fd, "\n");

  uint32_t size = foreach->p_systems.size();
  for(uint32_t i = 0; i < size; ++i)
  {

    const FccSystem* info = foreach->p_systems[i];
    std::string system_name = get_type_name(info->m_system_type);
    std::string base_name = system_name;

    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    fprintf(p_context->p_fd,
            "%s_%d->apply_block(&context,\n%s->m_start,\n%s->p_enabled", 
            base_name.c_str(), 
            info->m_id, 
            p_context->m_source.c_str(), 
            p_context->m_source.c_str());

    for(size_t i = 0; i <  foreach->m_columns.size(); ++i) 
    {
      fprintf(p_context->p_fd,",\ndata_%zu",i);
    }
    fprintf(p_context->p_fd, ");\n"); 
  }
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{

  std::string hashtable = "hashtable_"+std::to_string(join->m_id);
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
    std::string clustername = "cluster_"+std::to_string(join->m_id);
    fprintf(p_context->p_fd,
            "BlockCluster* %s = %s.get(%s->m_start);\n",
            clustername.c_str(),
            hashtable.c_str(),
            p_context->m_source.c_str());
    fprintf(p_context->p_fd,
            "if(%s != nullptr)\n{\n",
            clustername.c_str());
    fprintf(p_context->p_fd,
            "%s->append(%s);\n", 
            clustername.c_str(), 
            p_context->m_source.c_str());
    fprintf(p_context->p_fd,
            "if(%s->p_enabled->num_set() != 0)\n{\n", 
            clustername.c_str());

    consume(p_context->p_fd,
            join->p_parent,
            clustername,
            join);

    fprintf(p_context->p_fd,"}\n");
    fprintf(p_context->p_fd,"}\n");
  }
}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  const std::string tag = tag_filter->m_tag;
  fprintf(p_context->p_fd,"\n");
  fprintf(p_context->p_fd,
          "const Bitmap* filter = tagged_%s->get_bitmap(%s->m_start);\n", 
          tag.c_str(), 
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
        fprintf(p_context->p_fd, 
                "Bitmap* negate = new Bitmap(TABLE_BLOCK_SIZE);\n");
        fprintf(p_context->p_fd, 
                "negate->set_bitmap(filter);\n");
        fprintf(p_context->p_fd, 
                "negate->set_negate();\n");
        fprintf(p_context->p_fd,
                "%s->p_enabled->set_and(negate);\n",
                p_context->m_source.c_str());
        fprintf(p_context->p_fd, 
                "delete negate;\n");
        break;
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
      // TODO: report error
    }
    const std::string& type = get_qualified_type_name(column->m_q_type);
    fprintf(p_context->p_fd,
            "%s* data_%d = (%s*)(%s->m_blocks[%d]->p_data);\n",
            type.c_str(),
            param_index,
            type.c_str(),
            p_context->m_source.c_str(),
            param_index);
    param_index++;
  }
  std::string func_name = "";
  if(!predicate_filter->p_func_decl->isCXXClassMember())
  {
    const FunctionDecl* func_decl = predicate_filter->p_func_decl;
    func_name = func_decl->getName();
  } else
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
    SourceLocation start = predicate_filter->p_func_decl->getLocStart();
    SourceLocation end = predicate_filter->p_func_decl->getLocEnd();

    fprintf(p_context->p_fd, "%s;\n",get_code(sm, start, end).c_str());
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
  fprintf(p_context->p_fd, "&data_0[i]");
  for(size_t i = 1; i <predicate_filter->m_columns.size(); ++i)
  {
    fprintf(p_context->p_fd, ",&data_%zu[i]", i);
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
}

} /* furious */ 
