
#ifndef _FURIOUS_COMPILER_STRUCTS_H_
#define _FURIOUS_COMPILER_STRUCTS_H_

#include <string>
#include <vector>
#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

namespace furious {

enum class OperationType {
  E_UNKNOWN,
  E_FOREACH
};


/**
 * @brief Structure used to store the information of the execution of a system,
 * including the type of the system, the components the systme needs, the tags
 * the entities must contain and the type of operation to execute.
 */
struct FccExecInfo {
  OperationType         m_operation_type = OperationType::E_UNKNOWN; // The type of operations
  QualType              m_system_type;              // The type of the system
  std::vector<QualType> m_basic_component_types;    // The types of the components of the system
  std::vector<QualType> m_with_components;          // The types of the "with" components
  std::vector<QualType> m_without_components;       // The types of the "without" components

  std::vector<const Expr*>  m_ctor_params;          // The expressions of the system's constructor parameters 
  std::vector<std::string>  m_with_tags;            // The "with" tags  
  std::vector<std::string>  m_without_tags;         // The "without" tags

  std::vector<const FunctionDecl*> m_filter_func;   // The filter function
};

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////


#define REPORT_ERROR(error,filename,line,column) \
  FccContext_report_error(fcc_get_context(), \
                          error, \
                          filename, \
                          line, \
                          column);

enum class FccErrorType
{
  E_UNKNOWN_ERROR,
  E_UNKNOWN_FURIOUS_OPERATION,
  E_UNSUPPORTED_STATEMENT
};

/**
 * @brief Used to store the Fcc compilation context information
 */
struct FccContext {
  void (*p_ecallback)(FccContext*, 
                      FccErrorType, 
                      const std::string&, 
                      int32_t, 
                      int32_t); // Pointer to function handling the error
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
                              void (*cback)(FccContext*, 
                                            FccErrorType, 
                                            const std::string&, 
                                            int32_t, 
                                            int32_t));

/**
 * @brief Reports an error using the callback set in FccContext
 *
 * @param context The context to use
 * @param error_type The type of error
 */
void 
FccContext_report_error(FccContext* context,
                        FccErrorType error_type,
                        const std::string& filename,
                        int32_t line,
                        int32_t column);

} /* furious */ 

#endif
