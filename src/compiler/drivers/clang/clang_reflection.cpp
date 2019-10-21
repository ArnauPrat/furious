
#include "../../../runtime/data/reflection.h"
#include "../../driver.h"
#include "clang_tools.h"

#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;


namespace furious
{

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
      get_type_name(tmplt_decl->getTypeForDecl()->getCanonicalTypeInternal(),
                    p_refl_data.get()->m_type_name,
                    MAX_TYPE_NAME);

    }
    else
    {
      std::string str = decl->getName();
      FURIOUS_COPY_AND_CHECK_STR(p_refl_data.get()->m_type_name, str.c_str(), MAX_TYPE_NAME);
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
        std::string str = field->getName();
        FURIOUS_COPY_AND_CHECK_STR(refl_field.m_name, str.c_str(), MAX_FIELD_NAME);

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
        char tmp[MAX_TYPE_NAME];
        get_tagged_type_name(qtype, tmp, MAX_TYPE_NAME);  
        if(strncmp(tmp, "std::string", MAX_TYPE_NAME) == 0)
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

ReflData
get_reflection_data(fcc_decl_t decl)
{
  RecordDeclVisitor record_decl_visitor;
  record_decl_visitor.TraverseCXXRecordDecl(cast<CXXRecordDecl>((Decl*)decl));
  return *record_decl_visitor.p_refl_data.get();
}

  
} /* furious */ 
