#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "frontend/fccASTVisitor.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

using namespace furious;

static cl::OptionCategory fccToolCategory("fcc options");
static cl::opt<std::string> output_file("o", cl::cat(fccToolCategory));

int main(int argc, const char **argv)
{
  output_file.setInitialValue("furious_generated.cpp");
  CommonOptionsParser op(argc, argv, fccToolCategory);
  FccContext* fcc_context = Fcc_create_context();
  int result = Fcc_run(fcc_context, op, output_file.getValue());
  Fcc_release_context(fcc_context);
  return result;
}
