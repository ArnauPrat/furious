

#include <sstream>

#include "structs.h"

namespace furious 
{

void 
handle_error(FccContext* context,
             FccErrorType type,
             const std::string& filename, 
             int line,
             int column
             )
{
  std::stringstream ss;
  switch(type) 
  {
    case FccErrorType::E_UNKNOWN_ERROR:
      ss << "Unknown error";
      break;
    case FccErrorType::E_UNKNOWN_FURIOUS_OPERATION:
      ss << "Unknown furious operation"; 
      break;
    case FccErrorType::E_UNSUPPORTED_STATEMENT:
      ss << "Non furious staetment ";
  }
  ss << " found in " << filename << ":" << line << ":" << column << "\n"; 
  llvm::errs() << ss.str();
}

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

FccContext*
fcc_get_context()
{
  static FccContext* global_fcc_context = new FccContext();
  FccContext_initialize(global_fcc_context);
  FccContext_set_error_callback(global_fcc_context, 
                                handle_error);
  return global_fcc_context;
}

void
fcc_release_context()
{
  FccContext_release(fcc_get_context());
  delete fcc_get_context();
}

void 
FccContext_initialize(FccContext* context)
{
  context->p_ecallback = nullptr;
}

void 
FccContext_release(FccContext* context)
{
  context->p_ecallback = nullptr;
}

void 
FccContext_set_error_callback(FccContext* context,
                              void (*callback)(FccContext*, 
                                               FccErrorType,
                                               const std::string&,
                                               int32_t,
                                               int32_t))
{
  context->p_ecallback = callback;
}

void 
FccContext_report_error(FccContext* context,
                        FccErrorType error_type,
                        const std::string& filename,
                        int32_t line,
                        int32_t column)
{
  if(context->p_ecallback) 
  {
    context->p_ecallback(context, 
                         error_type,
                         filename,
                         line,
                         column);
  }
}


} /* furious */ 
