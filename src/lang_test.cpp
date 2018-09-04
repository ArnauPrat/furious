
#include "lang_test.h"
#include "lang/lang.h"

bool test1(const ComponentA* ca, 
          const ComponentB* cb)
{
  return true;
}

BEGIN_FURIOUS_SCRIPT

struct TestSystem1
{
  TestSystem1(int32_t val, float val2) : m_val{val} {}

  void run(furious::Context *context,
           int32_t id,
           ComponentA *componentA,
           const ComponentB *componentB)
  {
    componentA->m_field = componentB->m_field * m_val;
  };

  int32_t m_val;
};

furious::select<ComponentA,ComponentB>()
  .has_component<ComponentA>()
  .has_not_component<ComponentB>()
  .has_tag("Affected")
  .has_not_tag("NotAffected")
   .filter([](const ComponentA* ca, const ComponentB* cb)
        {
        return test1(ca,cb);
        }
       )
  .filter(test1).foreach<TestSystem1>(10,0.2);

END_FURIOUS_SCRIPT

int main(int argc,
         char **argv)
{
  return 0;
}
