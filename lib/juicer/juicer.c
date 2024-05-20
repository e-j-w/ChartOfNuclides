#include "juicer.h"


float juice_smoothStart2(const float inp){
  return inp*inp;
}
float juice_smoothStart2R(const float inp, const float range){
  return juice_smoothStart2(inp/range)*range;
}
float juice_smoothStart2MM(const float inp, const float min, const float max){
  return juice_smoothStart2((inp-min)/(max-min))*(max-min);
}
float juice_smoothStart2Int1(const float inp){
  return 0.66f + juice_smoothStart2(inp);
}
float juice_smoothStart3(const float inp){
  return inp*inp*inp;
}
float juice_smoothStart3R(const float inp, const float range){
  return juice_smoothStart3(inp/range)*range;
}
float juice_smoothStart3MM(const float inp, const float min, const float max){
  return juice_smoothStart3((inp-min)/(max-min))*(max-min);
}
float juice_smoothStart4(const float inp){
  return inp*inp*inp*inp;
}
float juice_smoothStart4R(const float inp, const float range){
  return juice_smoothStart4(inp/range)*range;
}
float juice_smoothStart4MM(const float inp, const float min, const float max){
  return juice_smoothStart4((inp-min)/(max-min))*(max-min);
}

float juice_smoothStop2(const float inp){
  return 1.0f - (1.0f - inp)*(1.0f - inp);
}
float juice_smoothStop2R(const float inp, const float range){
  return juice_smoothStop2(inp/range)*range;
}
float juice_smoothStop2MM(const float inp, const float min, const float max){
  return juice_smoothStop2((inp-min)/(max-min))*(max-min);
}
float juice_smoothStop2Int1(const float inp){
  return 0.33f + juice_smoothStop2(inp);
}
float juice_smoothStop3(const float inp){
  return 1.0f - (1.0f - inp)*(1.0f - inp)*(1.0f - inp);
}
float juice_smoothStop3R(const float inp, const float range){
  return juice_smoothStop3(inp/range)*range;
}
float juice_smoothStop3MM(const float inp, const float min, const float max){
  return juice_smoothStop3((inp-min)/(max-min))*(max-min);
}
float juice_smoothStop4(const float inp){
  return 1.0f - (1.0f - inp)*(1.0f - inp)*(1.0f - inp)*(1.0f - inp);
}
float juice_smoothStop4R(const float inp, const float range){
  return juice_smoothStop4(inp/range)*range;
}
float juice_smoothStop4MM(const float inp, const float min, const float max){
  return juice_smoothStop4((inp-min)/(max-min))*(max-min);
}

float juice_arch2(const float inp){
  return inp*(1.0f - inp);
}
float juice_arch2R(const float inp, const float range){
  return juice_arch2(inp/range)*range;
}
float juice_arch2MM(const float inp, const float min, const float max){
  return juice_arch2((inp-min)/(max-min))*(max-min);
}
float juice_arch2Int1(const float inp){
  //a version where the area under the curve is 1
  //useful for mapping continuous movement over a period of time
  return 0.833f + juice_arch2(inp);
}

float juice_smoothStopArch3(const float inp){
  return inp*(1.0f - inp)*(1.0f - inp);
}
float juice_smoothStopArch3R(const float inp, const float range){
  return juice_smoothStopArch3(inp/range)*range;
}
float juice_smoothStopArch3MM(const float inp, const float min, const float max){
  return juice_smoothStopArch3((inp-min)/(max-min))*(max-min);
}
float juice_smoothStopArch3Int1(const float inp){
  //a version where the area under the curve is 1
  //useful for mapping continuous movement over a period of time
  return 0.9167f + juice_smoothStopArch3(inp);
}

//output goes past 1.0 then 'bounces back'
float juice_bounceBack(const float inp){
  return 2.0f*inp*(inp*(1.0f - inp)) + inp;
}
float juice_bounceBackR(const float inp, const float range){
  return juice_bounceBack(inp/range)*range;
}
float juice_bounceBackMM(const float inp, const float min, const float max){
  return juice_bounceBack((inp-min)/(max-min))*(max-min);
}

float juice_bounceBack2(const float inp){
  return (4.0f - 3.0f*inp)*(inp*(1.0f - inp)) + inp;
}
float juice_bounceBack2R(const float inp, const float range){
  return juice_bounceBack2(inp/range)*range;
}
float juice_bounceBack2MM(const float inp, const float min, const float max){
  return juice_bounceBack2((inp-min)/(max-min))*(max-min);
}

//'bounces back' and oscillates
float juice_bounceBackOsc(const float inp){
  return (6.0f - 7.0f*inp)*(inp*(1.0f - inp)) + inp;
}
float juice_bounceBackOscR(const float inp, const float range){
  return juice_bounceBackOsc(inp/range)*range;
}
float juice_bounceBackOscMM(const float inp, const float min, const float max){
  return juice_bounceBackOsc((inp-min)/(max-min))*(max-min);
}

