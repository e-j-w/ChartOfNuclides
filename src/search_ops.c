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
#include "data_ops.h"

int SDLCALL compareRelevance(void *userdata, const void *a, const void *b){
	(void)userdata; //unused
	search_result *resA = ((search_result*)(intptr_t)(a)); //get the search result (double cast to avoid warning)
	search_result *resB = ((search_result*)(intptr_t)(b)); //get the search result (double cast to avoid warning)
	if(resA->relevance >= resB->relevance){
		return -1;
	}else{
		return 1;
	}
}

void sortAndAppendResult(search_state *restrict ss, const search_result *restrict res){
	
	//check that the result isn't identical to an existing one
	for(uint8_t i=0; i<ss->numResults;i++){
		if(res->resultType == ss->results[i].resultType){
			if(res->resultVal == ss->results[i].resultVal){
				return; //don't append identical results
			}
		}
	}
	
	//add the result to the list, if possible
	if(ss->numResults < MAX_SEARCH_RESULTS){
		//append the result
		memcpy(&ss->results[ss->numResults],res,sizeof(search_result));
		ss->numResults++;
	}else{
		//assuming results are already sorted...
		//if the new result has higher relevance than the lowest rank result,
		//overwrite that lowest ranked result
		if(res->relevance > ss->results[MAX_SEARCH_RESULTS-1].relevance){
			memcpy(&ss->results[MAX_SEARCH_RESULTS-1],res,sizeof(search_result));
		}else{
			return; //no result appended
		}
	}
	//sort the results by relevance
	SDL_qsort_r(&ss->results[0],ss->numResults,sizeof(search_result),compareRelevance,NULL);
}

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
	ss->numSearchTok = numTok;

	printf("%u search tokens:",ss->numSearchTok);
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		printf(" %s",ss->searchTok[i]);
	}
	printf("\n");

}

void searchNuclides(const ndata *restrict ndat, search_state *restrict ss){
	char nuclAStr[8], nuclElemName[8];
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		uint8_t len = (uint8_t)strlen(ss->searchTok[i]);
		if(isdigit(ss->searchTok[i][0])){
			uint8_t foundNucl = 0;
			for(uint8_t j=1; j<len; j++){
				if(j>=7){
					break;
				}
				if(!(isdigit(ss->searchTok[i][j]))){
					memcpy(&nuclAStr,&ss->searchTok[i],(size_t)j);
					nuclAStr[j] = '\0'; //terminate string at end
					if(len > j){
						memcpy(&nuclElemName,&(ss->searchTok[i][j]),(size_t)(len-j));
						nuclElemName[len-j] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}
			}
			if(foundNucl){
				int16_t nuclA = (int16_t)SDL_atoi(nuclAStr);
				int16_t nuclZ = (int16_t)elemStrToZ(nuclElemName);
				SDL_Log("A: %i, Z: %i, elem name: %s, len: %u\n",nuclA,nuclZ,nuclElemName,len);
				for(int16_t j=0; j<ndat->numNucl; j++){
					if(ndat->nuclData[j].Z == nuclZ){
						if(((ndat->nuclData[j].N + ndat->nuclData[j].Z)) == nuclA){
							//identified nuclide
							search_result res;
							res.relevance = 1.0f;
							res.resultType = SEARCHAGENT_NUCLIDE;
							res.resultVal = (uint32_t)j;
							SDL_Log("Found nuclide %u\n",res.resultVal);
							sortAndAppendResult(ss,&res);
						}
					}
				}
			}
		}
	}

	SDL_Log("Number of search results: %u\n",ss->numResults);
}
