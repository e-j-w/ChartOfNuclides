#ifndef STROPS_H
#define STROPS_H

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String ops library
// Various functions to manipulate C strings.

int strinsert(char *dest, size_t destsize, const char *ins, size_t position);
int strdelchar(char *dest, size_t destsize, size_t position);

#endif
