
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define ARRAY_SIZE 4096 

int main(int argc, char** argv) {

  bool mask[ARRAY_SIZE];
  int vec_a[ARRAY_SIZE];
  int vec_b[ARRAY_SIZE];
  int vec_c[ARRAY_SIZE];

  memset(mask, 0, sizeof(bool)*ARRAY_SIZE);
  memset(vec_a, 0, sizeof(int)*ARRAY_SIZE);
  memset(vec_b, 0, sizeof(int)*ARRAY_SIZE);
  memset(vec_c, 0, sizeof(int)*ARRAY_SIZE);

  srand(0);
  for(int i = 0; i < ARRAY_SIZE; ++i) {
    vec_a[i] = rand();
    vec_b[i] = rand();
    vec_c[i] = rand();
    mask[i] = static_cast<bool>(rand() % 2);
  }

  for(int i = 0; i < ARRAY_SIZE; ++i  ) {
    if(mask[i]) {
      vec_c[i] = 2*(vec_a[i] + vec_b[i]);
    }
  }

  for(int i = 0; i < ARRAY_SIZE; ++i ) {
    if(mask[i]) {
      printf("%d\n", vec_c[i]);
    }
  }

}
