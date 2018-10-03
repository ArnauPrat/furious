

#include "consume_visitor.h"
#include "produce_visitor.h"
#include "structs.h"
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
  for(const std::string& type : p_context->m_types) 
  {
      p_context->m_output_ss << type << "* data_"<< param_index << " = reinterpret_cast<" << type << "*>(" << p_context->m_source << ".m_blocks["<<param_index<<"]->p_data);\n";
      param_index++;
  }
  p_context->m_output_ss << "\n";

  for(const FccSystemInfo& info : foreach->m_systems)
  {

    std::string system_name = get_type_name(info.m_system_type);
    std::string base_name = system_name;

    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    p_context->m_output_ss << base_name << "_" << info.m_id << "->apply_block(&context,\n"<< p_context->m_source<<".m_start,\n"<<p_context->m_source<<".m_enabled";
    for(size_t i = 0; i <  p_context->m_types.size(); ++i) 
    {
      p_context->m_output_ss << ",\ndata_"<< i;
    }
    p_context->m_output_ss << ");\n"; 
  }
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{

  std::string hashtable = "hashtable_"+p_context->m_join_id;
  if(p_context->p_caller == join->p_left) 
  {
    p_context->m_output_ss << hashtable << "[" << p_context->m_source << ".m_start] = " << p_context->m_source << ";\n"; 
    p_context->m_left_types.insert(p_context->m_left_types.end(), 
                                   p_context->m_types.begin(),
                                   p_context->m_types.end());
  } else 
  {
    std::string clustername = "cluster_"+p_context->m_join_id;
    p_context->m_output_ss << "auto it = "<< hashtable << ".find(" << p_context->m_source << ".m_start);\n";
    p_context->m_output_ss << "if(it != "<< hashtable <<".end())\n{\n";
    p_context->m_output_ss << "BlockCluster& "<< clustername <<" = it->second;\n";
    p_context->m_output_ss << clustername << ".append(&" << p_context->m_source<< ");\n";
    p_context->m_output_ss << "if("<< clustername << ".m_enabled.any())\n{\n";
    std::vector<std::string> joined_types{p_context->m_left_types};
    joined_types.insert(joined_types.end(),
                        p_context->m_types.begin(),
                        p_context->m_types.end());
    consume(p_context->m_output_ss,
            join->p_parent,
            clustername,
            joined_types,
            join);
    p_context->m_output_ss << "}\n";
    p_context->m_output_ss << "}\n";
  }
}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  const std::string tag = tag_filter->m_tag;
  p_context->m_output_ss << "\n";
  p_context->m_output_ss << "const std::bitset<TABLE_BLOCK_SIZE>& filter = tagged_"<< tag << "->get_bitset("<< p_context->m_source <<".m_start);\n";
  switch(tag_filter->m_op_type) 
  {
    case FccFilterOpType::E_HAS:
      {
        p_context->m_output_ss << p_context->m_source <<".m_enabled &= filter;\n";
        break;
      }
    case FccFilterOpType::E_HAS_NOT:
      {
        p_context->m_output_ss << p_context->m_source <<".m_enabled &= ~filter;\n";
        break;
      }
  }

  p_context->m_output_ss << "if("<<p_context->m_source<<".m_enabled.any())\n{\n"; 
  consume(p_context->m_output_ss,
          tag_filter->p_parent,
          p_context->m_source,
          p_context->m_types,
          tag_filter);
  p_context->m_output_ss << "}\n";
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  // if ...
  consume(p_context->m_output_ss,
          component_filter->p_parent,
          "cluster",
          p_context->m_types,
          component_filter);
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
  // if ...
  p_context->m_output_ss << "\n";
  int param_index = 0;
  for(const std::string& type : p_context->m_types) 
  {
      p_context->m_output_ss << type << "* data_"<< param_index << " = reinterpret_cast<" << type << "*>(" << p_context->m_source << ".m_blocks["<<param_index<<"]->p_data);\n";
      param_index++;
  }
  std::string func_name = "";
  if(!predicate_filter->p_func_decl->isCXXClassMember())
  {
    const FunctionDecl* func_decl = predicate_filter->p_func_decl;
    func_name = func_decl->getName();
  } else
  {
    p_context->m_output_ss << "auto predicate = [] (";
    const FunctionDecl* func_decl = predicate_filter->p_func_decl;
    auto array = func_decl->parameters();
    p_context->m_output_ss << get_type_name(array[0]->getType()) << array[0]->getNameAsString();
    for(size_t i = 1; i < array.size(); ++i)
    {
      p_context->m_output_ss << "," << get_type_name(array[i]->getType()) << array[i]->getNameAsString();
    }
    const ASTContext& context = predicate_filter->p_func_decl->getASTContext();
    const SourceManager& sm = context.getSourceManager();
    SourceLocation start = predicate_filter->p_func_decl->getLocStart();
    SourceLocation end = predicate_filter->p_func_decl->getLocEnd();

    p_context->m_output_ss << get_code(sm, start, end) << ";\n";
    func_name = "predicate";
  }

  p_context->m_output_ss << "for(int32_t i = 0; i < TABLE_BLOCK_SIZE && "<< p_context->m_source <<".m_enabled.any(); ++i) \n{\n";
  p_context->m_output_ss << p_context->m_source <<".m_enabled[i] = "<< p_context->m_source <<".m_enabled[i] && "<<func_name<<"(";
  p_context->m_output_ss << "&data_0[i]";
  for(size_t i = 1; i <p_context->m_types.size(); ++i)
  {
    p_context->m_output_ss << ",&data_"<<i<<"[i]";
  }
  p_context->m_output_ss << ");\n";

  p_context->m_output_ss << "}\n";
  p_context->m_output_ss << "if("<<p_context->m_source<<".m_enabled.any())\n{\n"; 
  consume(p_context->m_output_ss,
          predicate_filter->p_parent,
          p_context->m_source,
          p_context->m_types,
          predicate_filter);
  p_context->m_output_ss << "}\n";
}

} /* furious */ 
