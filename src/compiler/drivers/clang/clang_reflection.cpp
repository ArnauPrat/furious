
#include "../../../runtime/data/reflection.h"
#include "../../driver.h"
#include "clang_tools.h"

#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;



class RecordDeclVisitor : public RecursiveASTVisitor<RecordDeclVisitor>
{
public:

  fdb_mregistry_t*  p_reg;
  fdb_mstruct_t*      p_mstruct;

  RecordDeclVisitor(fdb_mregistry_t* reg, fdb_mstruct_t* data) : 
  RecursiveASTVisitor<RecordDeclVisitor>(),
  p_reg(reg),
  p_mstruct(data)
  {
  }

  virtual
  ~RecordDeclVisitor()
  {
  }

  virtual 
  bool VisitCXXRecordDecl(CXXRecordDecl* decl)
  {

    if(isa<ClassTemplateSpecializationDecl>(decl))
    {
      const ClassTemplateSpecializationDecl* tmplt_decl = cast<ClassTemplateSpecializationDecl>(decl);
      get_type_name(tmplt_decl->getTypeForDecl()->getCanonicalTypeInternal(),
                    p_mstruct->m_type_name,
                    FCC_MAX_TYPE_NAME);
    }
    else
    {
      std::string str = decl->getName();
      FDB_COPY_AND_CHECK_STR(p_mstruct->m_type_name, str.c_str(), FDB_MAX_TABLE_NAME);
    }

    p_mstruct->m_is_union = decl->getTypeForDecl()->isUnionType();

    for(auto child_decl = decl->decls_begin(); 
        child_decl != decl->decls_end();
        ++child_decl)
    {

      if(child_decl->getKind() != Decl::Field )
      {
        continue;
      }

      fdb_mtype_t mtype = fdb_mtype_t::E_UNKNOWN;
      bool        is_anon = false;
      const clang::Type* type = nullptr;
      char        name[FCC_MAX_FIELD_NAME];
      QualType qtype;
      size_t num_bytes = 0;
      if(child_decl->getKind() == Decl::Field )
      {
        FieldDecl* field  = cast<FieldDecl>(*child_decl);
        std::string str = field->getName();
        FDB_COPY_AND_CHECK_STR(name, str.c_str(), FCC_MAX_FIELD_NAME);
        mtype = fdb_mtype_t::E_UNKNOWN;
        is_anon = field->isAnonymousStructOrUnion();
        qtype = field->getType();
        type = qtype.getTypePtr();
      } 
      /*else if(child_decl->getKind() == Decl::IndirectField)
      {
        IndirectFieldDecl* indirect_field  = cast<IndirectFieldDecl>(*child_decl);
        refl_field.m_name = indirect_field->getName();
        refl_field.m_type = fdb_mtype_t::E_UNKNOWN;
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
        mtype = fdb_mtype_t::E_BOOL;
      } 
      else if(type->isCharType())
      {
        mtype = fdb_mtype_t::E_CHAR;
      }
      else if(type->isSignedIntegerType())
      {
        switch(num_bytes)
        {
          case 1:
            mtype = fdb_mtype_t::E_INT8;
            break;
          case 2:
            mtype = fdb_mtype_t::E_INT16;
            break;
          case 4:
            mtype = fdb_mtype_t::E_INT32;
            break;
          case 8:
            mtype = fdb_mtype_t::E_INT64;
            break;
          default:
            mtype = fdb_mtype_t::E_UNKNOWN;
            break;
        };
      }
      else if(type->isUnsignedIntegerType())
      {
        switch(num_bytes)
        {
          case 1:
            mtype = fdb_mtype_t::E_UINT8;
            break;
          case 2:
            mtype = fdb_mtype_t::E_UINT16;
            break;
          case 4:
            mtype = fdb_mtype_t::E_UINT32;
            break;
          case 8:
            mtype = fdb_mtype_t::E_UINT64;
            break;
          default:
            mtype = fdb_mtype_t::E_UNKNOWN;
            break;
        };
      }
      else if(type->isFloatingType())
      {
        switch(num_bytes)
        {
          case 4:
            mtype = fdb_mtype_t::E_FLOAT;
            break;
          case 8:
            mtype = fdb_mtype_t::E_DOUBLE;
            break;
          default:
            mtype = fdb_mtype_t::E_UNKNOWN;
            break;
        };
      }
      else if(type->isPointerType())
      {
        // check char*
        mtype = fdb_mtype_t::E_CHAR_POINTER;
      }
      else if(type->isRecordType())
      {
        char tmp[FCC_MAX_TYPE_NAME];
        get_tagged_type_name(qtype, tmp, FCC_MAX_TYPE_NAME);  
        if(strncmp(tmp, "std::string", FCC_MAX_TYPE_NAME) == 0)
        {
          mtype = fdb_mtype_t::E_STD_STRING;
        }
        else 
        {
          if(!type->isUnionType())
          {
            mtype = fdb_mtype_t::E_UNION;
          }
          else
          {
            mtype = fdb_mtype_t::E_STRUCT;
          }
        }
      }
      fdb_mstruct_t* field = fdb_mregistry_init_mfield(p_reg, 
                                                       p_mstruct, 
                                                       name, 
                                                       mtype, 
                                                       -1, 
                                                       is_anon);
      if(field != nullptr) // Is UNION or STRICT
      {
        RecordDeclVisitor visitor(p_reg, field);
        visitor.TraverseCXXRecordDecl(type->getAsCXXRecordDecl());
      }

    }
    return false;
  }

};

fdb_mstruct_t* 
get_reflection_data(fcc_decl_t decl, fdb_mregistry_t* reg)
{
  fdb_mstruct_t* mstruct = fdb_mregistry_init_mstruct(reg, "", false); // NOTE: "" and false will be overwitten by the visitor
  RecordDeclVisitor record_decl_visitor(reg, mstruct);
  record_decl_visitor.TraverseCXXRecordDecl(cast<CXXRecordDecl>((Decl*)decl));
  return mstruct;
}

