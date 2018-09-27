

#include "consume_visitor.h"
#include "produce_visitor.h"
#include "structs.h"
#include "clang_tools.h"

namespace furious 
{


ConsumeVisitor::ConsumeVisitor(std::stringstream& output_ss,
                               const std::string& source,
                               const std::vector<std::string>& types,
                               const FccOperator* caller)  :
m_source{source},
m_output_ss{output_ss},
m_types{types},
p_caller{caller}
{
}

void 
ConsumeVisitor::visit(const Foreach* foreach)
{
  std::stringstream ss;
  for(const FccSystemInfo& info : foreach->m_systems)
  {

    std::string system_name = get_type_name(info.m_system_type);
    std::string base_name = system_name;

    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);
    ss << base_name << "_" << info.m_id << ".run(&context, i ";
    for(size_t i = 0; i <  m_types.size(); ++i) 
    {
      ss << ",data_"<< i << "[i]";
    }
    ss << ");\n"; 
  }

  m_output_ss << "\n";
  int param_index = 0;
  for(const std::string& type : m_types) 
  {
      m_output_ss << type << "* data_"<< param_index << " = static_cast<" << type << "*>(" << m_source << ".m_blocks->m_data);\n";
      param_index++;
  }
  m_output_ss << "\n";

  m_output_ss << "if("<<m_source<<".m_enabled.all())\n{\n";
  m_output_ss << "for(int32_t i = 0; i < TABLE_BLOCK_SIZE; ++i) \n{\n";
  m_output_ss << ss.str();
  m_output_ss << "}\n";
  m_output_ss << "} else \n{\n";
  m_output_ss << "for(int32_t i = 0; i < TABLE_BLOCK_SIZE; ++i) \n{\n";
  m_output_ss << "if("<<m_source<<".m_enabled[i])\n{\n";
  m_output_ss << ss.str();
  m_output_ss << "}\n";
  m_output_ss << "}\n";
  m_output_ss << "}\n";
}

void 
ConsumeVisitor::visit(const Scan* scan)
{
}

void
ConsumeVisitor::visit(const Join* join)
{
  if(p_caller == join->p_left) 
  {
    m_output_ss << "hashtable[" << m_source << ".m_start] = " << m_source << ";\n"; 
  } else 
  {
    m_output_ss << "auto it = hashtable.find(" << m_source << ".m_start);\n";
    m_output_ss << "if(it != hashtable.end())\n{\n";
    m_output_ss << "auto& cluster = *it;\n";
    m_output_ss << "cluster.append(&" << m_source<< ");\n";
    ConsumeVisitor consume{m_output_ss, "cluster", m_types, join};
    join->p_parent->accept(&consume);
    m_output_ss << "}\n";
  }
}

void 
ConsumeVisitor::visit(const TagFilter* tag_filter)
{
  const std::string tag = tag_filter->m_tag;
  m_output_ss << "\n";
  m_output_ss << "for(int32_t i = 0; i < TABLE_BLOCK_SIZE && "<<m_source<<".m_enabled.any(); ++i) \n{\n";
  switch(tag_filter->m_op_type) 
  {
    case FccFilterOpType::E_HAS:
      {
        m_output_ss << m_source <<".m_enabled[i] &&= (tagged_"<<tag<<".find("<<m_source<<" != tagged_"<<tag<<".end()));\n";
        break;
      }
    case FccFilterOpType::E_HAS_NOT:
      {
        m_output_ss << m_source <<".m_enabled[i] &&= (tagged_"<<tag<<".find("<<m_source<<" == tagged_"<<tag<<".end()));\n";
        break;
      }
  }
  m_output_ss << "}\n";
  ConsumeVisitor consume{m_output_ss, m_source, m_types, tag_filter} ;
  tag_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const ComponentFilter* component_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss, m_source, m_types, component_filter};
    component_filter->p_parent->accept(&consume);
}

void
ConsumeVisitor::visit(const PredicateFilter* predicate_filter)
{
  // if ...
  ConsumeVisitor consume{m_output_ss, m_source, m_types, predicate_filter};
  predicate_filter->p_parent->accept(&consume);

}

} /* furious */ 
