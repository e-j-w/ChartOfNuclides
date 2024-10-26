/*
Copyright (C) 2017-2024 J. Williams

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/* Functions handling search query manipulation */

#include "search_ops.h"

//breaks the search string down into smaller tokens
void tokenizeSearchStr(search_state *restrict ss){
	char searchStrCpy[SEARCH_STRING_MAX_SIZE], *tok;
	uint8_t numTok = 0;
	memcpy(searchStrCpy,ss->searchString,sizeof(ss->searchString));
	tok = strtok(searchStrCpy," ,");
	while(tok!=NULL){
		if(strlen(tok) > 0){
			strncpy(ss->searchTok[numTok],tok,15);
			numTok++;
		}
		if(numTok >= MAX_SEARCH_TOKENS){
			break;
		}
		tok=strtok(NULL," ,");
	}

	printf("%u search tokens:",numTok);
	for(uint8_t i=0; i<numTok; i++){
		printf(" %s",ss->searchTok[i]);
	}
	printf("\n");

}
