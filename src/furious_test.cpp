
#include "furious.h"

struct TestComponentA {
  float x; 
  float y;
  float z;
};

struct TestComponentB {
  float x; 
  float y;
  float z;
};

struct TestSystem {
  void run(TestComponentA* componentA, const TestComponentB*  componentB) {
    componentA->x = componentA->x + componentB->x;
    componentA->y = componentA->y + componentB->y;
    componentA->z = componentA->z + componentB->z;
  }
};

int main(int argc, char** argv) {

  furious::init();

  furious::register_component<TestComponentA>();
  furious::register_component<TestComponentB>();

  furious::register_system<TestSystem>();

  furious::Entity entity1 = furious::create_entity();
  entity1.add_component<TestComponentA>(0.0f, 0.0f, 0.0f);
  entity1.add_component<TestComponentB>(0.0f, 0.0f, 0.0f);

  furious::Entity entity2 = furious::create_entity();
  entity2.add_component<TestComponentA>(0.0f, 0.0f, 0.0f);
  entity2.add_component<TestComponentB>(0.0f, 0.0f, 0.0f);


  entity1.remove_component<TestComponentA>(); 
  entity1.remove_component<TestComponentB>(); 

  entity2.remove_component<TestComponentA>(); 
  entity2.remove_component<TestComponentB>(); 
  furious::release();

  return 0;
}
