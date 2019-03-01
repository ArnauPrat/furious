
#include "lang_test.h"
#include "lang/lang.h"

struct ComponentC {
  int32_t m_field;
};

bool test1(const ComponentA* ca, 
           const ComponentC* cc,
          const ComponentB* cb)
{
  return true;
}

BEGIN_FURIOUS_SCRIPT

struct TestSystem1
{
  TestSystem1(int32_t val, float val2) : m_val{val} {}

  void run(furious::Context* context,
           uint32_t id,
           ComponentA *componentA,
           const ComponentC* componentC,
           const ComponentB *componentB)
  {
    componentA->m_field = componentB->m_field * m_val + componentC->m_field;
  };

  int32_t m_val;
};

furious::match<ComponentA,ComponentC,ComponentB>()
  .has_component<ComponentA>()
  .has_not_component<ComponentB>()
  .has_tag("Affected")
  .has_not_tag("NotAffected")
   .filter([](const ComponentA* ca, const ComponentC* cc, const ComponentB* cb)
        {
        return test1(ca,cc,cb);
        }
       )
  .filter(test1).foreach<TestSystem1>(10,0.2);


//furious::select<ComponentA,ComponentC,ComponentB>().foreach<TestSystem1>(10,0.2);

////////////////////////////////////////////////
////////////////////////////////////////////////
////////////////////////////////////////////////

struct TestSystem2
{
  TestSystem2(float val) : m_val{val} {}

  void run(furious::Context* context,
           uint32_t id,
           ComponentA* componentA,
           const ComponentB *componentB)
  {
    componentA->m_field = componentB->m_field * m_val;
  };

  float m_val;
};

furious::match<ComponentA, ComponentB>().expand<ComponentC>("edge name").foreach<TestSystem2>(1.0);


END_FURIOUS_SCRIPT

// This is just to not let the compiler and linker complain with warnings or
// errors
int main(int argc,
         char **argv)
{
  furious_script();
  return 0;
}
