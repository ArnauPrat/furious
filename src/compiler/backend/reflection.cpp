

#include "../common/common.h"
#include "../../common/str_builder.h"
#include "reflection.h"
#include "codegen_tools.h"
#include "../../runtime/data/reflection.h"

#include <string.h>


namespace furious
{

const char* ReflType_str[(uint32_t)ReflType::E_NUM_TYPES] = {
  "ReflType::E_BOOL",
  "ReflType::E_CHAR",
  "ReflType::E_UINT8",
  "ReflType::E_UINT16",
  "ReflType::E_UINT32",
  "ReflType::E_UINT64",
  "ReflType::E_INT8",
  "ReflType::E_INT16",
  "ReflType::E_INT32",
  "ReflType::E_INT64",
  "ReflType::E_FLOAT",
  "ReflType::E_DOUBLE",
  "ReflType::E_CHAR_POINTER",
  "ReflType::E_STD_STRING",
  "ReflType::E_POINTER",
  "ReflType::E_STRUCT",
  "ReflType::E_UNION",
  "ReflType::E_UNKNOWN",
};


uint32_t
generate_reflection_code(FILE* fd, 
                         ReflData* refl_data, 
                         const char* root, 
                         const char* path,
                         char* buffer,
                         uint32_t buffer_length)
{
  char tmp[FCC_MAX_TYPE_NAME];
  FURIOUS_COPY_AND_CHECK_STR(tmp, refl_data->m_type_name, FCC_MAX_TYPE_NAME);
  sanitize_name(tmp);

  str_builder_t str_builder = str_builder_create();
  str_builder_append(&str_builder, "ref_data_");
  str_builder_append(&str_builder, tmp);
  const uint32_t ret_length = str_builder.m_pos;
  FURIOUS_COPY_AND_CHECK_STR(buffer, str_builder.p_buffer, buffer_length);
  str_builder_destroy(&str_builder);

  fprintf(fd, "RefCountPtr<ReflData> %s(new ReflData());\n", buffer);
  fprintf(fd, 
          "strcpy(%s.get()->m_type_name,\"%s\");\n", 
          buffer,
          refl_data->m_type_name);
  for(uint32_t i = 0; i < refl_data->m_fields.size(); ++i)
  {
    ReflField* field = &refl_data->m_fields[i];
    fprintf(fd,"{\n");
    fprintf(fd, "ReflField field;\n");
    fprintf(fd, "strcpy(field.m_name,\"%s\");\n", field->m_name);
    fprintf(fd, "field.m_type = %s;\n", ReflType_str[(uint32_t)field->m_type]);
    fprintf(fd, "field.m_anonymous = %s;\n", field->m_anonymous ? "true" : "false");
    str_builder_t str_builder = str_builder_create();
    if(field->m_anonymous)
    {
      fprintf(fd, "field.m_offset = 0;\n");
      if(strcmp(path,"")==0) // is first level
      {
        str_builder_append(&str_builder, "%s", "");
      }
      else
      {
        str_builder_append(&str_builder, "%s", path);
      }
    }
    else
    {
      if(strcmp(path,"")==0) // is first level
      {
        str_builder_append(&str_builder,  "%s", field->m_name);
        fprintf(fd, "field.m_offset = offsetof(%s, %s);\n", root, str_builder.p_buffer);
      }
      else
      {
        str_builder_append(&str_builder, "%s.%s", path, field->m_name);

        fprintf(fd, "field.m_offset = offsetof(%s, %s) - offsetof(%s, %s);\n", 
                root, 
                str_builder.p_buffer,
                root, 
                path);
      }
    }

    if(field->m_type == ReflType::E_STRUCT || field->m_type == ReflType::E_UNION)
    {
      char field_name[FCC_MAX_FIELD_NAME];
      generate_reflection_code(fd, 
                               field->p_strct_type.get(), 
                               root, 
                               str_builder.p_buffer,
                               field_name,
                               FCC_MAX_FIELD_NAME);
      fprintf(fd, "field.p_strct_type = %s;\n", field_name);
    }
    fprintf(fd, "%s.get()->m_fields.append(field);\n", buffer);
    fprintf(fd,"}\n");
    str_builder_destroy(&str_builder);
  }
  return ret_length;
}

void
generate_reflection_code(FILE* fd, fcc_decl_t decl)
{
  fprintf(fd,"{\n");
  ReflData refl_data = get_reflection_data(decl);
  char var_name[FCC_MAX_FIELD_NAME];
  generate_reflection_code(fd, 
                           &refl_data, 
                           refl_data.m_type_name, 
                           "", 
                           var_name, 
                           FCC_MAX_FIELD_NAME);
  fprintf(fd, "FURIOUS_ADD_REFL_DATA(database, %s, %s);\n", refl_data.m_type_name, var_name);
  fprintf(fd,"}\n");
}

} /* furious */ 

