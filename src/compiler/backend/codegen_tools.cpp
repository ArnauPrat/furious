

#include "../common/types.h"
#include "../common/dyn_array.h"
#include "consume_visitor.h"
#include "produce_visitor.h"
#include "../frontend/operator.h"
#include "driver.h"

#include "codegen.h"
#include "codegen_tools.h"

#include <string.h>
#include <algorithm>

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
        const char* source,
        const FccOperator* caller)
{
  CodeGenContext* context = p_registry->find_or_create(op,fd);
  FURIOUS_PERMA_ASSERT(strlen(source) <= MAX_SOURCE_LENGTH);
  strcpy(context->m_source,source);
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

uint32_t
sanitize_name(const char* name,
              char* buffer,
              uint32_t buffer_length)
{
    std::string base_name = name;
    std::transform(base_name.begin(), 
                   base_name.end(), 
                   base_name.begin(), ::tolower);

    char special_chars[] = {':','<','>',',','.',' '};

    for(uint32_t i = 0; i < sizeof(special_chars); ++i)
    {
      std::replace(base_name.begin(),
                   base_name.end(),
                   special_chars[i],
                   '_');
    }

    FURIOUS_CHECK_STR_LENGTH(base_name.size(), buffer_length);
    strcpy(buffer, base_name.c_str());

    return base_name.size();
}

uint32_t
generate_table_name(const char* type_name, 
                    char *buffer,
                    uint32_t buffer_length,
                    const FccOperator* op)
{
  char tmp[MAX_TYPE_NAME];
  sanitize_name(type_name,
                tmp,
                MAX_TYPE_NAME); 

  std::string table_varname = "table_"+std::string(tmp);

  if(op != nullptr)
  {
    table_varname = table_varname+"_"+std::to_string(op->m_id);
  }

  FURIOUS_CHECK_STR_LENGTH(table_varname.size(), buffer_length);

  strcpy(buffer, table_varname.c_str());

  return table_varname.size();
}

uint32_t
generate_temp_table_name(const char* type_name, 
                         char * buffer,
                         uint32_t buffer_length,
                         const FccOperator* op)
{
  char tmp[MAX_TABLE_VARNAME];
  generate_table_name(type_name, 
                      tmp,
                      MAX_TABLE_VARNAME,
                      op);
  std::string table_name = "temp_"+std::string(tmp);

  FURIOUS_CHECK_STR_LENGTH(table_name.size(), buffer_length);
  strcpy(buffer, table_name.c_str());
  return table_name.size();
}

uint32_t
generate_ref_table_name(const char* ref_name, 
                        char* buffer,
                        uint32_t buffer_length,
                        const FccOperator* op)
{
  char tmp[MAX_TABLE_VARNAME];
  generate_table_name(ref_name, 
                      tmp,
                      MAX_TABLE_VARNAME,
                      op);

  std::string table_name = "ref_"+std::string(tmp);
  FURIOUS_CHECK_STR_LENGTH(table_name.size(), buffer_length);
  strcpy(buffer, table_name.c_str());
  return table_name.size();
}

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const FccOperator* op)
{
  std::string table_name = "tagged_"+std::string(tag_name);
  if(op != nullptr)
  {
    table_name = table_name+"_"+std::to_string(op->m_id);
  }
  FURIOUS_CHECK_STR_LENGTH(table_name.size(), buffer_length);
  strcpy(buffer, table_name.c_str());
  return table_name.size();
}

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const FccOperator* op)
{
  std::string iter_name = "iter_"+std::string(table_name);
  if(op != nullptr)
  {
    iter_name = iter_name + "_"+std::to_string(op->m_id);
  }
  FURIOUS_CHECK_STR_LENGTH(iter_name.size(), buffer_length);
  strcpy(buffer, iter_name.c_str());
  return iter_name.size();
}

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const FccOperator* op)
{
  char tmp[MAX_TYPE_NAME];
  sanitize_name(type_name,
                tmp,
                MAX_TYPE_NAME);
  std::string block_name =  "block_"+std::string(tmp); 
  if(op != nullptr)
  {
    block_name = block_name + "_"+std::to_string(op->m_id);
  }
  FURIOUS_CHECK_STR_LENGTH(block_name.size(), buffer_length);
  strcpy(buffer, block_name.c_str());
  return block_name.size();
}

uint32_t
generate_cluster_name(const FccOperator* op,
                      char* buffer,
                      uint32_t buffer_length)
{
  std::string cluster_name =  "cluster_"+std::to_string(op->m_id);
  FURIOUS_CHECK_STR_LENGTH(cluster_name.size(), buffer_length);
  strcpy(buffer, cluster_name.c_str());
  return cluster_name.size();
}

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const FccOperator* op)
{
  char tmp[MAX_TYPE_NAME];
  sanitize_name(ref_name,
                tmp,
                MAX_TYPE_NAME);
  std::string groups_name ="ref_"+std::string(tmp)+"_groups_"+std::to_string(op->m_id);
  FURIOUS_CHECK_STR_LENGTH(groups_name.size(), buffer_length);
  strcpy(buffer, groups_name.c_str());
  return groups_name.size();
}

uint32_t
generate_hashtable_name(const FccOperator* op,
                        char* buffer,
                        uint32_t buffer_length)
{
  std::string hashtable =  "hashtable_"+std::to_string(op->m_id);
  FURIOUS_CHECK_STR_LENGTH(hashtable.size(), buffer_length);
  strcpy(buffer, hashtable.c_str());
  return hashtable.size();
}

uint32_t
generate_system_wrapper_name(const char* system_name,
                             uint32_t system_id,
                             char* buffer,
                             uint32_t buffer_length,
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
  FURIOUS_CHECK_STR_LENGTH(wrapper_name.size(), buffer_length);
  strcpy(buffer, wrapper_name.c_str());
  return wrapper_name.size();
}

uint32_t
generate_global_name(const char* type_name, 
                     char* buffer,
                     uint32_t buffer_length,
                     const FccOperator* op)
{
  char tmp[MAX_TYPE_NAME];
  sanitize_name(type_name,
                tmp,
                MAX_TYPE_NAME);

  std::string global_varname = "global_"+std::string(tmp);

  if(op != nullptr)
  {
    global_varname = global_varname+"_"+std::to_string(op->m_id);
  }

  FURIOUS_CHECK_STR_LENGTH(global_varname.size(), buffer_length);
  strcpy(buffer, global_varname.c_str());

  return global_varname.size();
}


} /* furious */ 
