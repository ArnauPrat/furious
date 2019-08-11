
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include "../common/dyn_array.h"
#include "fcc_context.h"

#include <string>
#include <stdio.h>

namespace furious 
{

struct FccContext;
class FccOperator;
class ProduceVisitor;
class ConsumeVisitor;

/**
 * \brief This class represents the context of the code generation process at a
 * given code generation step. Since the code generation is a recursive
 * procedure implemented through the visitors, we need to store the "state" of
 * the generation process at a given step produce/consume step.
 */

constexpr uint32_t MAX_SOURCE_LENGTH = 512;


struct CodeGenContext
{
  CodeGenContext(FILE* fd);
  ~CodeGenContext();

  ProduceVisitor* p_producer;
  ConsumeVisitor* p_consumer;

  char                    m_source[MAX_SOURCE_LENGTH];
  FILE*                   p_fd;
  const FccOperator*      p_caller;
};

struct CodeGenRegistry
{
  struct Entry
  {
    const FccOperator*  p_operator;
    CodeGenContext*     p_context;
  };

  CodeGenContext* 
  find_or_create(const FccOperator*, FILE* fd);

  CodeGenRegistry();
  ~CodeGenRegistry();

  DynArray<Entry> m_contexts; 
};


void 
consume(FILE* fd,
        const FccOperator* op,
        const char* source,
        const FccOperator* caller);

void 
produce(FILE* fd,
        const FccOperator* op);

uint32_t
sanitize_name(const char* str,
              char* buffer,
              uint32_t buffer_length);

uint32_t
generate_table_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length, 
                    const FccOperator* op = nullptr);

uint32_t
generate_temp_table_name(const char* type_name,
                         char* buffer,
                         uint32_t buffer_length, 
                         const FccOperator* op = nullptr);

uint32_t
generate_ref_table_name(const char* ref_name,
                        char* buffer,
                        uint32_t buffer_length, 
                        const FccOperator* op = nullptr);

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const FccOperator* op = nullptr);

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const FccOperator* op = nullptr);

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const FccOperator* op = nullptr);

uint32_t
generate_cluster_name(const FccOperator* op,
                      char* buffer,
                      uint32_t buffer_length);

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length, 
                         const FccOperator* op);

uint32_t
generate_hashtable_name(const FccOperator* op,
                        char* buffer,
                        uint32_t buffer_length);

uint32_t
generate_system_wrapper_name(const char* system_name,
                             uint32_t system_id,
                             char* buffer,
                             uint32_t buffer_length,
                             const FccOperator* op = nullptr);

uint32_t
generate_global_name(const char* type_name,
                     char* buffer,
                     uint32_t buffer_length, 
                     const FccOperator* op = nullptr);


} /* furious */ 


#endif
