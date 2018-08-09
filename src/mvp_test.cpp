
#include<chrono>
#include<stdio.h>

#define NUM_ENTITIES 1000000
#define NUM_ITERS 1000

//#define REGULAR
#define NON_VIRTUAL 

struct Component1{
  float data[4];
};

struct Component2{
  float data[4];
};

struct Component3{
  float data[4];
  float data2;
};

class System {
public:
  virtual void run(void* component1,
                   void* component2) = 0;
};

struct SystemA : public System {

  virtual void run(void* component1, 
                   void* component2) {

    this->run((Component1*)component1, 
              (const Component2*)component2);
  }

  void run(Component1* component1, 
           const Component2* component2) {
    component1->data[0] *= component2->data[0]+1;
    component1->data[1] *= component2->data[1]+1;
    component1->data[2] *= component2->data[2]+1;
    component1->data[3] *= component2->data[3]+1;

  }
};

struct SystemB : public System {

  virtual void run(void* component1,
                   void* component2) {
    run((Component2*)component1,
        (Component3*)component2);

  }


  void run(const Component2* component2, 
           Component3* component3) {
    
    component3->data[0] *= component2->data[0]+1;
    component3->data[1] *= component2->data[1]+1;
    component3->data[2] *= component2->data[2]+1;
    component3->data[3] *= component2->data[3]+1;
  }
};

struct SystemC : public System {

  virtual void run(void* component1,
                   void* component2) {
    run((Component1*)component1,
        (Component2*)component2);
  }


  void run(Component1* component1, 
           Component2* component3) {

    component1->data[0] *= 2;
    component1->data[1] *= 2;
    component1->data[2] *= 2;
    component1->data[3] *= 2;
  }
};

int main(int argc, char** argv) {

  Component1* components1 = new Component1[NUM_ENTITIES];
  Component2* components2 = new Component2[NUM_ENTITIES];
  Component3* components3 = new Component3[NUM_ENTITIES];

  long accum = 0;
  for(int j = 0; j < NUM_ITERS; ++j) {
    auto start = std::chrono::high_resolution_clock::now();

#ifdef REGULAR

    SystemA systemA;
    SystemB systemB;

    System* system = &systemA;
    for(int i = 0; i < NUM_ENTITIES; ++i) {
      system->run((void*)components1,
                  (void*)components2);
    }

    system = &systemB;
    for(int i = 0; i < NUM_ENTITIES; ++i) {
      system->run((void*)components2,
                  (void*)components3);
    }


#endif

#ifdef NON_VIRTUAL

    SystemA systemA;
    SystemB systemB;
    SystemC systemC;

    for(int i = 0; i < NUM_ENTITIES; ++i) {

      systemA.run(components1,
                   components2);
      systemB.run(components2,
                   components3);

      systemC.run(components1,
                   components2);

    }

    /*for(int i = 0; i < NUM_ENTITIES; ++i) {

     
     
    }

    for(int i = 0; i < NUM_ENTITIES; ++i) {

     
    }*/

#endif 

    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    accum += std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
  }

  printf("Avearge iteration time: %lu\n us", accum / NUM_ITERS);

  delete [] components1;
  delete [] components2;
  delete [] components3;

}
