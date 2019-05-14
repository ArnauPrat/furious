
#include "lang/lang.h"
#include "reflection_test_header.h"

BEGIN_FURIOUS_SCRIPT

struct TestSystem 
{
  void run(furious::Context* context,
           uint32_t id,
           TestComponent* position)
  {


  }
};

furious::match<TestComponent>().foreach<TestSystem>();

END_FURIOUS_SCRIPT
