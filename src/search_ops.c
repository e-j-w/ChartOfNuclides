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
	for(uint8_t i=0; i<ss->numUpdatedResults;i++){
		if(res->resultType == ss->updatedResults[i].resultType){
			if(res->resultVal == ss->updatedResults[i].resultVal){
				return; //don't append identical results
			}
		}
	}
	
	//add the result to the list, if possible
	if(ss->numUpdatedResults < MAX_SEARCH_RESULTS){
		//append the result
		memcpy(&ss->updatedResults[ss->numUpdatedResults],res,sizeof(search_result));
		ss->numUpdatedResults++;
	}else{
		//assuming results are already sorted...
		//if the new result has higher relevance than the lowest rank result,
		//overwrite that lowest ranked result
		if(res->relevance > ss->updatedResults[MAX_SEARCH_RESULTS-1].relevance){
			memcpy(&ss->updatedResults[MAX_SEARCH_RESULTS-1],res,sizeof(search_result));
		}else{
			return; //no result appended
		}
	}
	//sort the results by relevance
	SDL_qsort_r(&ss->updatedResults[0],ss->numUpdatedResults,sizeof(search_result),compareRelevance,NULL);
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

	/*printf("%u search tokens:",ss->numSearchTok);
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		printf(" %s",ss->searchTok[i]);
	}
	printf("\n");*/

}

void searchNuclides(const ndata *restrict ndat, search_state *restrict ss){
	char nuclAStr[8], nuclElemName[8];
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		uint8_t foundNucl = 0;
		uint8_t len = (uint8_t)strlen(ss->searchTok[i]);
		if(isdigit(ss->searchTok[i][0])){
			//look for nuclide names starting with a digit (eg. 32Si)
			for(uint8_t j=1; j<len; j++){
				if(j>=7){
					break;
				}
				if(!(isdigit(ss->searchTok[i][j]))){
					memcpy(&nuclAStr,&ss->searchTok[i],(size_t)j);
					nuclAStr[j] = '\0'; //terminate string at end
					if((len > j)&&((len-j)<8)){
						memcpy(&nuclElemName,&(ss->searchTok[i][j]),(size_t)(len-j));
						nuclElemName[len-j] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}
			}
		}else if(isalpha(ss->searchTok[i][0])){
			//look for nuclide names starting with a letter (eg. si32)
			for(uint8_t j=1; j<len; j++){
				if(j>=7){
					break;
				}
				if(isdigit(ss->searchTok[i][j])){
					memcpy(&nuclElemName,&ss->searchTok[i],(size_t)j);
					nuclElemName[j] = '\0'; //terminate string at end
					if((len > j)&&((len-j)<8)){
						memcpy(&nuclAStr,&(ss->searchTok[i][j]),(size_t)(len-j));
						nuclAStr[len-j] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}
			}
		}
		if(foundNucl){
			int16_t nuclA = (int16_t)SDL_atoi(nuclAStr);
			int16_t nuclZ = (int16_t)elemStrToZ(nuclElemName);
			nuclElemName[0] = (char)SDL_toupper(nuclElemName[0]); //convert to uppercase
			int16_t nuclZu = (int16_t)elemStrToZ(nuclElemName);
			//SDL_Log("A: %i, Z: %i, elem name: %s, len: %u\n",nuclA,nuclZ,nuclElemName,len);
			for(int16_t j=0; j<ndat->numNucl; j++){
				if(ndat->nuclData[j].Z == nuclZ){
					if(((ndat->nuclData[j].N + ndat->nuclData[j].Z)) == nuclA){
						//identified nuclide (exact match)
						search_result res;
						res.relevance = 1.0f; //exact match
						res.resultType = SEARCHAGENT_NUCLIDE;
						res.resultVal = (uint32_t)j;
						//SDL_Log("Found nuclide %u\n",res.resultVal);
						sortAndAppendResult(ss,&res);
					}
				}
				if((nuclZu != nuclZ)&&(ndat->nuclData[j].Z == nuclZu)){
					if(((ndat->nuclData[j].N + ndat->nuclData[j].Z)) == nuclA){
						//identified nuclide (uppercase match)
						search_result res;
						res.relevance = 0.8f; //uppercase match
						res.resultType = SEARCHAGENT_NUCLIDE;
						res.resultVal = (uint32_t)j;
						//SDL_Log("Found nuclide %u\n",res.resultVal);
						sortAndAppendResult(ss,&res);
					}
				}
			}
		}
	}

	//SDL_Log("Number of search results: %u\n",ss->numUpdatedResults);
}
