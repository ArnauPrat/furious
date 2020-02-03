
#include "lang/lang.h"
#include "reflection_test_header.h"

BEGIN_FDB_SCRIPT

struct TestSystem 
{
  void run(fdb_context_t* context,
           uint32_t id,
           TestComponent* position)
  {


  }
};

match<TestComponent>().foreach<TestSystem>();

END_FDB_SCRIPT
