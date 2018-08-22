
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

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


enum class FccErrorType
{
  E_UNKNOWN_ERROR,
};

/**
 * @brief Used to store the Fcc compilation context information
 */
struct FccContext {
  void (*p_ecallback)(FccContext*, FccErrorType); // Pointer to function handling the error
};

/**
 * @brief Gets an initialized global fcc context
 *
 * @return Returns an initialized Fcc context
 */
FccContext*
fcc_get_context();

/**
 * @brief Frees The global Fcc context
 */
void
fcc_release_context();

/**
 * @brief Initializes the given FccContext
 *
 * @param context The context to initialize
 */
void 
FccContext_initialize(FccContext* context);

/**
 * @brief Initializes the given FccContext
 *
 * @param context The context to initialize
 */
void 
FccContext_release(FccContext* context);

/**
 * @brief Sets the error callback function
 *
 * @param context The FccContext to set the function for
 * @param callback The callback function to set
 */
void 
FccContext_set_error_callback(FccContext* context,
                              void (*cback)(FccContext*, FccErrorType));

/**
 * @brief Reports an error using the callback in FccContext
 *
 * @param context The context to use
 * @param error_type The type of error
 */
void 
FccContext_report_error(FccContext* context,
                        FccErrorType error_type);

} /* furious */ 

#endif
