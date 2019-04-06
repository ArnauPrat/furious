

#include "../common/types.h"
#include "../common/dyn_array.h"
#include "consume_visitor.h"
#include "produce_visitor.h"

#include "codegen.h"
#include "codegen_tools.h"

namespace furious 
{

extern CodeGenRegistry* p_registry;

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
        const std::string& source,
        const FccOperator* caller)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  context->m_source = source;
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

std::string
generate_table_name(const std::string& type_name, 
                    const FccOperator* op)
{
    std::string base_name = type_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    std::replace(base_name.begin(),
                 base_name.end(),
                 ':',
                 '_');

    std::string table_varname = "table_"+base_name;

    if(op != nullptr)
    {
      table_varname = table_varname+"_"+std::to_string(op->m_id);
    }

    return table_varname;
}

std::string
generate_temp_table_name(const std::string& type_name, 
                         const FccOperator* op)
{
  std::string table_name = generate_table_name(type_name, op);
  table_name = "temp_"+table_name;
  return table_name;
}

std::string
generate_ref_table_name(const std::string& ref_name, 
                              const FccOperator* op)
{
  std::string table_name = generate_table_name(ref_name, op);
  table_name = "ref_"+table_name;
  return table_name;
}

std::string
generate_bittable_name(const std::string& tag_name,
                       const FccOperator* op)
{
  std::string table_name = "tagged_"+tag_name;
  if(op != nullptr)
  {
    table_name = table_name+"_"+std::to_string(op->m_id);
  }
  return table_name;
}

std::string
generate_table_iter_name(const std::string& table_name,
                         const FccOperator* op)
{
  std::string iter_name = "iter_"+table_name;
  if(op != nullptr)
  {
    iter_name = iter_name + "_"+std::to_string(op->m_id);
  }
  return iter_name;
}

std::string
generate_block_name(const std::string& type_name,
                    const FccOperator* op)
{
    std::string base_name = type_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    std::replace(base_name.begin(),
                 base_name.end(),
                 ':',
                 '_');
    std::string block_name =  "block_"+base_name; 
    if(op != nullptr)
    {
      block_name = block_name + "_"+std::to_string(op->m_id);
    }
    return block_name;
}

std::string
generate_cluster_name(const FccOperator* op)
{
  return "cluster_"+std::to_string(op->m_id);
}

std::string
generate_ref_groups_name(const std::string& ref_name,
                         const FccOperator* op)
{
    std::string base_name = ref_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    std::replace(base_name.begin(),
                 base_name.end(),
                 ':',
                 '_');
  return "ref_"+base_name+"_groups_"+std::to_string(op->m_id);
}

std::string
generate_hashtable_name(const FccOperator* op)
{
  return "hashtable_"+std::to_string(op->m_id);
}

std::string
generate_system_wrapper_name(const std::string& system_name,
                             uint32_t system_id,
                             const FccOperator* op)
{
  std::string base_name = system_name;
  std::transform(base_name.begin(), 
                 base_name.end(), 
                 base_name.begin(), ::tolower);
  std::string wrapper_name = base_name+"_"+std::to_string(system_id);
  if(op != nullptr)
  {
    wrapper_name =  wrapper_name +"_"+std::to_string(op->m_id);
  }
  return wrapper_name;
}

std::string
generate_global_name(const std::string& type_name, 
                    const FccOperator* op)
{
    std::string base_name = type_name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    std::replace(base_name.begin(),
                 base_name.end(),
                 ':',
                 '_');

    std::string global_varname = "global_"+base_name;

    if(op != nullptr)
    {
      global_varname = global_varname+"_"+std::to_string(op->m_id);
    }

    return global_varname;
}

} /* furious */ 
