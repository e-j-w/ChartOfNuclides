#include "strops.h"

//insert a string into another string at the specified position 
//return value: 0 if output is not truncated, 1 if it is truncated,-1 if there is no change
int strinsert(char *dest, size_t destsize, const char *ins, size_t position){

  if(!ins){
    return -1;  //invalid parameter
  }

  const size_t origsize = strlen(dest);
  size_t inssize = strlen(ins);

  if(inssize == 0){
    return -1; //invalid parameter
  }

  size_t resize = origsize + inssize + 1; //1 for the null terminator

  if(position > origsize){
    //printf("position: %lu, origsize: %lu\n",position,origsize);
    return -1; //invalid position, out of original string
  }

  //truncate inserted string if necessary
  if(destsize < resize){
    inssize = destsize - origsize - 1; //truncate
    if(inssize == 0){
      return -1; //truncated to nothing
    }
    resize = destsize;
  }

  //move string to make room for insertion
  memmove(&dest[position+inssize], &dest[position], origsize - position);
  dest[origsize + inssize] = '\0'; //null terminate string

  //insert string
  memcpy(&dest[position], ins, inssize);

  if(resize == destsize){
    return 1;
  }
  return 0;
}

//delete a character from the string at the specified position
//position is 1-indexed, so position==1 means the first character
//in the string gets deleted (equivalent to cursor position when
//editing text)
//return value: 0 on success, -1 on failure (unchanged output string)
int strdelchar(char *dest, size_t destsize, size_t position){

  if(position == 0){
    return -1; //cursor prior to any character in the string
  }

  if((!dest) || (destsize == 0)){
    return -1;  //invalid parameter
  }

  const size_t origsize = strlen(dest);
  if(position > origsize){
    return -1; //invalid position
  }
  if(origsize == 0){
    return -1; //input string empty, cannot truncate
  }

  //move string
  memmove(&dest[position-1], &dest[position], origsize - position);
  dest[origsize - 1] = '\0'; //null terminate string
  return 0;
}

