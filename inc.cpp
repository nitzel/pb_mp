#include "inc.hpp"

struct vec2 screen, view, mouseR, mouseV;

//////////////////////////////
//// random number generation
/////////////////////////////

uint32_t xorshf962(void) {          //period 2^96-1
// Marsaglia's xorshf generator
//http://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
  static unsigned long x=123456789, y=362436069, z=521288629;
  unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

   t = x;
   x = y;
   y = z;
   z = t ^ x ^ y;

  return z;
}

uint32_t rand(uint32_t max){
  return xorshf962()%max;
}
int32_t rand(int32_t min, int32_t max){
  return ((int32_t)rand(max-min)+min);
}
float randf(){ // within -1 and 1
  return ((signed int)rand(-1000000000 ,1000000000))/(float)(2000000000);
}
