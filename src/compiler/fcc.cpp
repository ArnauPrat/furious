#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "fccASTVisitor.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

using namespace furious;

static llvm::cl::OptionCategory fccToolCategory("fcc options");

int main(int argc, const char **argv)
{
  CommonOptionsParser op(argc, argv, fccToolCategory);
  FccContext* fcc_context = FccContext_create_and_init();
  int result = FccContext_run(fcc_context, op);
  FccContext_release(fcc_context);
  return result;
}
