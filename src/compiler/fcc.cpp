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

static llvm::cl::OptionCategory fccToolCategory("fcc options");

static furious::FccContext fcc_context;

int main(int argc, const char **argv)
{

  CommonOptionsParser op(argc, argv, fccToolCategory);
  ClangTool tool(op.getCompilations(), op.getSourcePathList());

  furious::FccContext_initialize(&fcc_context);
  int result = tool.run(newFrontendActionFactory<furious::FccFrontendAction>().get());
  if (result == 0)
  {
  }
  return result;
}
