
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include <string>
#include <vector>
#include <clang/AST/RecursiveASTVisitor.h>



namespace furious {

enum class OperationType {
  E_UNKNOWN,
  E_FOREACH
};

struct FccExecInfo {
  OperationType                m_operation_type = OperationType::E_UNKNOWN;
  clang::QualType              m_system_type;
  std::vector<clang::QualType> m_basic_component_types;
  std::vector<const clang::FunctionDecl*> m_filter_func;
  std::vector<clang::QualType> m_with_components;
  std::vector<clang::QualType> m_without_components;
  std::vector<std::string>     m_with_tags;
  std::vector<std::string>     m_without_tags;
};

/**
 * @brief Used to store the Fcc compilation context information
 */
struct FccContext {
};


} /* furious */ 

#endif
