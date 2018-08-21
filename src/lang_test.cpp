
#include "lang_test.h"
#include "lang/lang.h"

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

/*furious::run<TestSystem1>(10).filter([](const ComponentA* componentA, 
                                        const ComponentB* componentB) 
                                     {
                                      return componentA->m_field > componentB->m_field;
                                     }
                                    ).with_tag("Affected").without_component<ComponentC>();
                                    */

furious::register_foreach<TestSystem1>(5, 1.0);

END_FURIOUS_SCRIPT

int main(int argc,
         char **argv)
{
  return 0;
}
