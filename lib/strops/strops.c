#include "strops.h"

//insert a string into another string at the specified position 
//return value: 0 if output is not truncated, 1 if it is truncated,-1 if there is no change
int strInsert(char *dest, size_t destsize, const char *ins, size_t position){

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
int strDelChar(char *dest, size_t destsize, size_t position){

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


//find/replace functions for UTF-8 strings
char* findReplaceUTF8(const char *findstr, const char *replacestr, const char *origstr){
  size_t len_replace = strlen(replacestr);
  size_t len_findstr = strlen(findstr);
  size_t len = strlen(origstr);
  
  int diff = (int)(len_replace - len_findstr); //can be negative!
  
  size_t new_string_len = (size_t)((int)(len) + diff + 1);
  if(new_string_len < (len+1)){
    new_string_len = len+1; //because the original string might be copied back here...
  }

  char *new_string = (char*)calloc(new_string_len, sizeof(char));
  
  char *pos = strstr(origstr, findstr);
  
  if(pos == NULL){
    strcpy(new_string, origstr);
    return new_string;
  }
  
  size_t num_shifts = (size_t)(pos - origstr);
  
  //Add begining of the string
  memcpy(new_string, origstr, num_shifts);
  
  //Copy the replacement in place of the findstr
  memcpy(new_string + num_shifts, replacestr, len_replace);
  
  //Copy the remainder of the initial string
  memcpy(new_string + num_shifts + len_replace, pos + len_findstr, len - num_shifts - len_findstr);
  
  return new_string;
}

char* findReplaceAllUTF8(const char *findstr, const char *replacestr, const char *origstr){
  char *new_string = findReplaceUTF8(findstr, replacestr, origstr);
  char *old_new_string = NULL;
  
  while(strstr(new_string, findstr) != NULL){
    old_new_string = new_string;
    new_string = findReplaceUTF8(findstr, replacestr, new_string);
    free(old_new_string);
  }
  
  return new_string;
}

size_t UTF8Strlen(const char *utf8str){
  size_t i = 0, len = 0;
  while(utf8str[i]){ //ends at NULL terminator
    if(!((utf8str[i] & 0xc0) == 0x80)){
      len++;
    } 
    i++;
  }
  return len;
}

//in case the isacii() function is unavailable...
int charIsAscii(const char c){
  return (!((c & 0x80) == 0x80));
}
