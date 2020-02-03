

#include "../common/common.h"
#include "../../common/str_builder.h"
#include "reflection.h"
#include "codegen_tools.h"
#include "../../runtime/data/reflection.h"

#include <string.h>



const char* fdb_mtype_str[(uint32_t)fdb_mtype_t::E_NUM_TYPES] = {
  "fdb_mtype_t::E_BOOL",
  "fdb_mtype_t::E_CHAR",
  "fdb_mtype_t::E_UINT8",
  "fdb_mtype_t::E_UINT16",
  "fdb_mtype_t::E_UINT32",
  "fdb_mtype_t::E_UINT64",
  "fdb_mtype_t::E_INT8",
  "fdb_mtype_t::E_INT16",
  "fdb_mtype_t::E_INT32",
  "fdb_mtype_t::E_INT64",
  "fdb_mtype_t::E_FLOAT",
  "fdb_mtype_t::E_DOUBLE",
  "fdb_mtype_t::E_CHAR_POINTER",
  "fdb_mtype_t::E_STD_STRING",
  "fdb_mtype_t::E_POINTER",
  "fdb_mtype_t::E_STRUCT",
  "fdb_mtype_t::E_UNION",
  "fdb_mtype_t::E_UNKNOWN",
};


void
generate_reflection_code(FILE* fd, 
                         fdb_mstruct_t* refl_data, 
                         const char* root, 
                         const char* path,
                         char* var_name)
{
  for(uint32_t i = 0; i < refl_data->m_nfields; ++i)
  {
    fdb_mfield_t* field = refl_data->p_fields[i];
    fprintf(fd,"{\n");
    fprintf(fd, "char field_name[FCC_MAX_FIELD_NAME];\n");
    fprintf(fd, "strcpy(field_name,\"%s\");\n", field->m_name);
    fprintf(fd, "fdb_mtype_t type = %s;\n", fdb_mtype_str[(uint32_t)field->m_type]);
    fprintf(fd, "bool is_anon = %s;\n", field->m_anonymous ? "true" : "false");
    fdb_str_builder_t str_builder;
    fdb_str_builder_init(&str_builder);
    if(field->m_anonymous)
    {
      fprintf(fd, "uint32_t offset = 0;\n");
      if(strcmp(path,"")==0) // is first level
      {
        fdb_str_builder_append(&str_builder, "%s", "");
      }
      else
      {
        fdb_str_builder_append(&str_builder, "%s", path);
      }
    }
    else
    {
      if(strcmp(path,"")==0) // is first level
      {
        fdb_str_builder_append(&str_builder,  "%s", field->m_name);
        fprintf(fd, "uint32_t offset = offsetof(%s, %s);\n", root, str_builder.p_buffer);
      }
      else
      {
        fdb_str_builder_append(&str_builder, "%s.%s", path, field->m_name);

        fprintf(fd, "uint32_t offset = offsetof(%s, %s) - offsetof(%s, %s);\n", 
                root, 
                str_builder.p_buffer,
                root, 
                path);
      }
    }

    if(field->m_type == fdb_mtype_t::E_STRUCT || field->m_type == fdb_mtype_t::E_UNION)
    {
      char tmp[FCC_MAX_TYPE_NAME];
      FDB_COPY_AND_CHECK_STR(tmp, field->m_name, FCC_MAX_TYPE_NAME);
      sanitize_name(tmp);
      fdb_str_builder_t field_builder;
      fdb_str_builder_init(&field_builder);
      fdb_str_builder_append(&field_builder, "ref_data_");
      fdb_str_builder_append(&field_builder, tmp);
      fprintf(fd, "fdb_mstruct_t* %s = fdb_mregistry_init_mfield(reg, %s, field_name, type, offset, is_anon);\n", field_builder.p_buffer, var_name);
      fprintf(fd,"{\n");
      generate_reflection_code(fd, 
                               field->p_strct_type, 
                               root, 
                               str_builder.p_buffer,
                               field_builder.p_buffer);
      fdb_str_builder_release(&field_builder);
      fprintf(fd,"}\n");
    }
    else
    {
      fprintf(fd, "fdb_mregistry_init_mfield(reg, %s, field_name, type, offset, is_anon);\n", var_name);
    }
    fprintf(fd,"}\n");
    fdb_str_builder_release(&str_builder);
  }
}

void
generate_reflection_code(FILE* fd, fcc_decl_t decl)
{
  fprintf(fd,"{\n");
  fprintf(fd, "fdb_mregistry_t* reg = fdb_database_get_mregistry(database);\n");
  fdb_mregistry_t reg;
  fdb_mregistry_init(&reg, NULL);
  fdb_mstruct_t* mstruct = get_reflection_data(decl, &reg);

  char tmp[FCC_MAX_TYPE_NAME];
  FDB_COPY_AND_CHECK_STR(tmp, mstruct->m_type_name, FCC_MAX_TYPE_NAME);
  sanitize_name(tmp);
  fdb_str_builder_t str_builder;
  fdb_str_builder_init(&str_builder);
  fdb_str_builder_append(&str_builder, "ref_data_");
  fdb_str_builder_append(&str_builder, tmp);
  fprintf(fd, "fdb_mstruct_t* %s = fdb_mregistry_init_mstruct(reg, \"%s\", %d );\n", 
          str_builder.p_buffer,
          mstruct->m_type_name, 
          mstruct->m_is_union);
  generate_reflection_code(fd, 
                           mstruct, 
                           mstruct->m_type_name, 
                           "", 
                           str_builder.p_buffer);
  fdb_str_builder_release(&str_builder);
  fprintf(fd,"}\n");
  fdb_mregistry_release(&reg);
}

