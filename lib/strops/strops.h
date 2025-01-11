#ifndef STROPS_H
#define STROPS_H

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// String ops library
// Various functions to manipulate C strings.

int strInsert(char *dest, size_t destsize, const char *ins, size_t position);
int strDelChar(char *dest, size_t destsize, size_t position);
char* findReplaceUTF8(const char *findstr, const char *replacestr, const char *origstr);
char* findReplaceAllUTF8(const char *findstr, const char *replacestr, const char *origstr);
int charIsAscii(const char c);

#endif
