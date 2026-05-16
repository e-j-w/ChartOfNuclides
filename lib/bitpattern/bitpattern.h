#ifndef BPATTERN_H
#define BPATTERN_H

#include <stdlib.h>
#include <stdint.h> //allows uint8_t and similiar types
#include <string.h> //memset
//#include <stdio.h> //printf

// Bitpattern library, to handle bitpatterns > 64 bits.

typedef struct
{
  uint64_t bpElem[2];
}bp128; //128-bit bitpattern

void bp_set128(bp128 *bp, const uint8_t bit);
void bp_clear128(bp128 *bp, const uint8_t bit);
void bp_clearall128(bp128 *bp);
uint8_t bp_check128(const bp128 *bp, const uint8_t bit);
uint8_t bp_isonly128(const bp128 *bp, const uint8_t bit);
uint8_t bp_onlyoneormoreof128(const bp128 *bp, const uint8_t *bits, const uint8_t numBits);
//void bp_list128(const bp128 *bp);

#endif
