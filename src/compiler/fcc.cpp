#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "fccASTVisitor.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

static llvm::cl::OptionCategory fccToolCategory("fcc options");

static furious::FccContext fcc_context;

namespace furious
{

class FccASTConsumer : public ASTConsumer
{
private:
  FccASTVisitor *visitor; // doesn't have to be private

public:
  // override the constructor in order to pass CI
  explicit FccASTConsumer(CompilerInstance *cI,
                          FccContext *fcc_context)
      : visitor(new FccASTVisitor(cI, fcc_context)) // initialize the visitor
  {
  }

  // override this to call our ExampleVisitor on the entire source file
  virtual void
  HandleTranslationUnit(ASTContext &context)
  {

    // context.getTranslationUnitDecl()->dump(llvm::errs());
    /* we can use ASTContext to get the TranslationUnitDecl, which is
         a single Decl that collectively represents the entire source file */
    visitor->TraverseDecl(context.getTranslationUnitDecl());
  }
};

class FccFrontendAction : public ASTFrontendAction
{
public:
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &cI, StringRef file)
  {
    return std::make_unique<FccASTConsumer>(&cI, &fcc_context); // pass CI pointer to ASTConsumer
  }
};

} // namespace furious

int main(int argc, const char **argv)
{

  CommonOptionsParser op(argc, argv, fccToolCategory);

  for (auto file : op.getCompilations().getAllFiles())
  {
    llvm::errs() << file << "\n";
  }
  ClangTool tool(op.getCompilations(), op.getSourcePathList());

  int result = tool.run(newFrontendActionFactory<furious::FccFrontendAction>().get());
  if (result == 0)
  {
  }
  return result;
}
