
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

furious::register_foreach<TestSystem1>(10,0.2)
  .with_component<ComponentA>()
  .without_component<ComponentB>()
  .with_tag("Affected")
  .without_tag("Affected")
   .filter([](const ComponentA* ca, const ComponentB* cb)
        {
        return test1(ca,cb);
        }
       )
  .filter(test1);
//furious::register_foreach<TestSystem1>(5, 1.0).with_tag("Affected").without_tag("NotAffected");

END_FURIOUS_SCRIPT

int main(int argc,
         char **argv)
{
  return 0;
}
