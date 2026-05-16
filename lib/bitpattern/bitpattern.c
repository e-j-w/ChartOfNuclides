#include "bitpattern.h"

//set a bit in a 128-bit bit-pattern
void bp_set128(bp128 *bp, const uint8_t bit){
  if(bit > 127){
    //out of bit-pattern range
    return;
  }
  const int32_t bpInd = bit/64;
  const uint8_t offset = (uint8_t)(bit - (bpInd*64));
  bp->bpElem[bpInd] |= (uint64_t)((uint64_t)(1) << offset);
}

//clear a bit in a 128-bit bit-pattern
void bp_clear128(bp128 *bp, const uint8_t bit){
  if(bit > 127){
    //out of bit-pattern range
    return;
  }
  const int32_t bpInd = bit/64;
  const uint8_t offset = (uint8_t)(bit - (bpInd*64));
  bp->bpElem[bpInd] &= ~((uint64_t)((uint64_t)(1) << offset));
}

//clear all bits in a 128-bit bit-pattern
void bp_clearall128(bp128 *bp){
  memset(bp->bpElem,0,sizeof(bp->bpElem));
}

//check whether a bit is set in a 128-bit bit-pattern
uint8_t bp_check128(const bp128 *bp, const uint8_t bit){
  if(bit > 127){
    //out of bit-pattern range
    return 0;
  }
  const int32_t bpInd = bit/64;
  const uint8_t offset = (uint8_t)(bit - (bpInd*64));
  if(bp->bpElem[bpInd] & ((uint64_t)(1) << offset)){
    return 1;
  }
  return 0;
}

//check whether ONLY the specified bit is set in a 128-bit bit-pattern
uint8_t bp_isonly128(const bp128 *bp, const uint8_t bit){
  if(bit > 127){
    //out of bit-pattern range
    return 0;
  }
  const int32_t bpInd = bit/64;
  const uint8_t offset = (uint8_t)(bit - (bpInd*64));
  for(uint8_t i=0; i<2; i++){
    if(i==bpInd){
      if(bp->bpElem[i] != ((uint64_t)(1) << offset)){
        return 0;
      }
    }else if(bp->bpElem[i] != 0){
      return 0;
    }
  }
  return 1;
}

//check whether ONLY one or more of the specified bits is set in a 128-bit bit-pattern
uint8_t bp_onlyoneormoreof128(const bp128 *bp, const uint8_t *bits, const uint8_t numBits){
  
  bp128 tmp;
  memcpy(&tmp,bp,sizeof(bp128));

  for(int bitNum=0; bitNum<numBits; bitNum++){
    bp_clear128(&tmp,bits[bitNum]);
  }
  for(uint8_t i=0; i<2; i++){
    if(tmp.bpElem[i] != 0){
      return 0;
    }
  }
  return 1;
}

//list bit that are set in a 128-bit bit-pattern
/*void bp_list128(const bp128 *bp){
  for(int bitNum=0; bitNum<127; bitNum++){
    const int32_t bpInd = bitNum/64;
    const uint8_t offset = (uint8_t)(bitNum - (bpInd*64));
    if(bp->bpElem[bpInd] & ((uint64_t)(1) << offset)){
      printf("Bit %i set\n",bitNum);
    }
  }
}*/

