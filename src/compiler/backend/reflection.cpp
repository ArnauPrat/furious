

#include "../../common/string_builder.h"
#include "reflection.h"
#include "codegen_tools.h"
#include "../clang_tools.h"
#include "../../runtime/data/reflection.h"
#include "../../common/dyn_array.h"

#include <clang/AST/DeclCXX.h>
#include <clang/AST/RecursiveASTVisitor.h>

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


class RecordDeclVisitor : public RecursiveASTVisitor<RecordDeclVisitor>
{
public:

  RefCountPtr<ReflData>  p_refl_data;

  RecordDeclVisitor() : 
  RecursiveASTVisitor<RecordDeclVisitor>(),
  p_refl_data(new ReflData())
  {

  }

  virtual 
  bool VisitCXXRecordDecl(CXXRecordDecl* decl)
  {
    //decl->dump();
    if(isa<ClassTemplateSpecializationDecl>(decl))
    {
      const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(decl);
      p_refl_data.get()->m_type_name = get_type_name(tmplt_decl->getTypeForDecl()->getCanonicalTypeInternal());
    }
    else
    {
      p_refl_data.get()->m_type_name = decl->getName();
    }
    p_refl_data.get()->m_is_union = decl->getTypeForDecl()->isUnionType();

    for(auto child_decl = decl->decls_begin(); 
        child_decl != decl->decls_end();
        ++child_decl)
    {

      ReflField refl_field;
      const clang::Type* type = nullptr;
      QualType qtype;
      size_t num_bytes = 0;
      if(child_decl->getKind() == Decl::Field )
      {
        FieldDecl* field  = cast<FieldDecl>(*child_decl);
        refl_field.m_name = field->getName();
        refl_field.m_type = ReflType::E_UNKNOWN;
        refl_field.m_anonymous = field->isAnonymousStructOrUnion();
        qtype = field->getType();
        type = qtype.getTypePtr();
      } 
      /*else if(child_decl->getKind() == Decl::IndirectField)
      {
        IndirectFieldDecl* indirect_field  = cast<IndirectFieldDecl>(*child_decl);
        refl_field.m_name = indirect_field->getName();
        refl_field.m_type = ReflType::E_UNKNOWN;
        qtype = indirect_field->getType();
        type = qtype.getTypePtr();
      }*/
      else
      {
        continue;
      }

      num_bytes = decl->getASTContext().getTypeInfo(qtype).Width / 8;

      if(type->isBooleanType())
      {
        refl_field.m_type = ReflType::E_BOOL;
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isCharType())
      {
        refl_field.m_type = ReflType::E_CHAR;
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isSignedIntegerType())
      {
        switch(num_bytes)
        {
          case 1:
            refl_field.m_type = ReflType::E_INT8;
            break;
          case 2:
            refl_field.m_type = ReflType::E_INT16;
            break;
          case 4:
            refl_field.m_type = ReflType::E_INT32;
            break;
          case 8:
            refl_field.m_type = ReflType::E_INT64;
            break;
          default:
            refl_field.m_type = ReflType::E_UNKNOWN;
            break;
        };
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isUnsignedIntegerType())
      {
        switch(num_bytes)
        {
          case 1:
            refl_field.m_type = ReflType::E_UINT8;
            break;
          case 2:
            refl_field.m_type = ReflType::E_UINT16;
            break;
          case 4:
            refl_field.m_type = ReflType::E_UINT32;
            break;
          case 8:
            refl_field.m_type = ReflType::E_UINT64;
            break;
          default:
            refl_field.m_type = ReflType::E_UNKNOWN;
            break;
        };
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isFloatingType())
      {
        switch(num_bytes)
        {
          case 4:
            refl_field.m_type = ReflType::E_FLOAT;
            break;
          case 8:
            refl_field.m_type = ReflType::E_DOUBLE;
            break;
          default:
            refl_field.m_type = ReflType::E_UNKNOWN;
            break;
        };
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isPointerType())
      {
        // check char*
        refl_field.m_type = ReflType::E_CHAR_POINTER;
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

      if(type->isRecordType())
      {
        if(get_tagged_type_name(qtype) == "std::string")
        {
          refl_field.m_type = ReflType::E_STD_STRING;
        }
        else 
        {
          if(!type->isUnionType())
          {
            RecordDeclVisitor visitor;
            visitor.TraverseCXXRecordDecl(type->getAsCXXRecordDecl());
            refl_field.m_type = ReflType::E_STRUCT;
            refl_field.p_strct_type = visitor.p_refl_data;
          }
          else
          {
            RecordDeclVisitor visitor;
            visitor.TraverseCXXRecordDecl(type->getAsCXXRecordDecl());
            refl_field.m_type = ReflType::E_UNION;
            refl_field.p_strct_type = visitor.p_refl_data;
          }
        }
        p_refl_data.get()->m_fields.append(refl_field);
        continue;
      }

    }
    return false;
  }

};


std::string
generate_reflection_code(FILE* fd, ReflData* refl_data, const std::string& root, const std::string& path)
{
  std::string refl_data_varname = "ref_data_"+sanitize_name(refl_data->m_type_name);
  fprintf(fd, "RefCountPtr<ReflData> %s(new ReflData());\n", refl_data_varname.c_str());
  for(uint32_t i = 0; i < refl_data->m_fields.size(); ++i)
  {
    ReflField* field = &refl_data->m_fields[i];
    fprintf(fd,"{\n");
    fprintf(fd, "ReflField field;\n");
    fprintf(fd, "field.m_name = \"%s\";\n", field->m_name.c_str());
    fprintf(fd, "field.m_type = %s;\n", ReflType_str[(uint32_t)field->m_type]);
    fprintf(fd, "field.m_anonymous = %s;\n", field->m_anonymous ? "true" : "false");
    StringBuilder str_builder;
    if(field->m_anonymous)
    {
      fprintf(fd, "field.m_offset = 0;\n");
      if(path == "") // is first level
      {
        str_builder.append("%s", "");
      }
      else
      {
        str_builder.append("%s", path.c_str());
      }
    }
    else
    {
      if(path == "") // is first level
      {
        str_builder.append("%s", field->m_name.c_str());
        fprintf(fd, "field.m_offset = offsetof(%s, %s);\n", root.c_str(), str_builder.p_buffer);
      }
      else
      {
        str_builder.append("%s.%s", path.c_str(), field->m_name.c_str());

        fprintf(fd, "field.m_offset = offsetof(%s, %s) - offsetof(%s, %s);\n", 
                root.c_str(), str_builder.p_buffer,
                root.c_str(), path.c_str());
      }
    }

    if(field->m_type == ReflType::E_STRUCT || field->m_type == ReflType::E_UNION)
    {
      std::string var  = generate_reflection_code(fd, field->p_strct_type.get(), root, str_builder.p_buffer);
      fprintf(fd, "field.p_strct_type = %s;\n", var.c_str());
    }
    fprintf(fd, "%s.get()->m_fields.append(field);\n", refl_data_varname.c_str());
    fprintf(fd,"}\n");
  }
  return refl_data_varname;
}

void
generate_reflection_code(FILE* fd, CXXRecordDecl* decl)
{
  RecordDeclVisitor record_decl_visitor;
  record_decl_visitor.TraverseCXXRecordDecl(decl);
  fprintf(fd,"{\n");
  ReflData* refl_data = record_decl_visitor.p_refl_data.get();
  std::string var_name = generate_reflection_code(fd, refl_data, refl_data->m_type_name, "");
  fprintf(fd, "database->add_refl_data<%s>(%s);\n", refl_data->m_type_name.c_str(), var_name.c_str());
  fprintf(fd,"}\n");
}
  
} /* furious */ 

