
#ifndef _FURIOUS_CODEGEN_TOOLS_H_
#define _FURIOUS_CODEGEN_TOOLS_H_

#include "../../common/platform.h"
#include "../common/dyn_array.h"
#include "../driver.h"
#include "../fcc_context.h"

#include <stdio.h>
#include <string.h>

namespace furious 
{

struct FccSubPlan;
struct fcc_operator_t;

/**
 * @brief Tool used to extract the dependencies of an execution plan, which
 * include structures and headers including dependent structures
 */
struct fcc_deps_extr_t  
{
  DynArray<char*>         m_include_files;
  DynArray<fcc_decl_t>    m_declarations;
};

void
fcc_deps_extr_init(fcc_deps_extr_t* deps_extr);

void
fcc_deps_extr_release(fcc_deps_extr_t* deps_extr);

void
fcc_deps_extr_extract(fcc_deps_extr_t* deps_extr,
                      const FccSubPlan* subplan);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


/**
 * \brief Tool to extracts the components and tags used in an execution plan, which will
 * be used to declare the required variables in the generated code.
 */
struct fcc_vars_extr_t  
{
  DynArray<fcc_decl_t>    m_component_decls;
  DynArray<char*>         m_tags;
  DynArray<char*>         m_references;
  DynArray<char*>         m_components;
};

void
fcc_vars_extr_init(fcc_vars_extr_t* vars_extr);

void
fcc_vars_extr_release(fcc_vars_extr_t* vars_extr);

void
fcc_vars_extr_extract(fcc_vars_extr_t* vars_extr,
                      const FccSubPlan* subplan);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

void
sanitize_name(char* str);

uint32_t
generate_table_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length, 
                    const fcc_operator_t* op = nullptr);

uint32_t
generate_temp_table_name(const char* type_name,
                         char* buffer,
                         uint32_t buffer_length, 
                         const fcc_operator_t* op = nullptr);

uint32_t
generate_ref_table_name(const char* ref_name,
                        char* buffer,
                        uint32_t buffer_length, 
                        const fcc_operator_t* op = nullptr);

uint32_t
generate_bittable_name(const char* tag_name,
                       char* buffer,
                       uint32_t buffer_length,
                       const fcc_operator_t* op = nullptr);

uint32_t
generate_table_iter_name(const char* table_name,
                         char* buffer,
                         uint32_t buffer_length,
                         const fcc_operator_t* op = nullptr);

uint32_t
generate_block_name(const char* type_name,
                    char* buffer,
                    uint32_t buffer_length,
                    const fcc_operator_t* op = nullptr);

uint32_t
generate_cluster_name(const fcc_operator_t* op,
                      char* buffer,
                      uint32_t buffer_length);

uint32_t
generate_ref_groups_name(const char* ref_name,
                         char* buffer,
                         uint32_t buffer_length, 
                         const fcc_operator_t* op);

uint32_t
generate_hashtable_name(const fcc_operator_t* op,
                        char* buffer,
                        uint32_t buffer_length);

uint32_t
generate_system_wrapper_name(const char* system_name,
                             uint32_t system_id,
                             char* buffer,
                             uint32_t buffer_length,
                             const fcc_operator_t* op = nullptr);

uint32_t
generate_global_name(const char* type_name,
                     char* buffer,
                     uint32_t buffer_length, 
                     const fcc_operator_t* op = nullptr);


} /* furious */ 


#endif
