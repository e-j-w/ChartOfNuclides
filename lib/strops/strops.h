#ifndef STROPS_H
#define STROPS_H

//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// String ops library
// Various functions to manipulate C strings.

int32_t strInsert(char *dest, size_t destsize, const char *ins, size_t position);
int32_t strDelChar(char *dest, size_t destsize, size_t position);
char* findReplaceUTF8(const char *findstr, const char *replacestr, const char *origstr);
char* findReplaceAllUTF8(const char *findstr, const char *replacestr, const char *origstr);
size_t UTF8Strlen(const char *utf8str);
int32_t charIsAscii(const char c);
int32_t strStartsWithValidUTF8OrASCIIChar(const char *c);

#endif
