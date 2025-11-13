/*
Copyright (C) 2017-2025 J. Williams

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

#define MAX_CASCADE_GAMMAS 8 //if raised, need to change gammasMatched bit-pattern to a larger size type

int SDLCALL compareRelevance(const void *a, const void *b){
	search_result *resA = ((search_result*)(intptr_t)(a)); //get the search result (double cast to avoid warning)
	search_result *resB = ((search_result*)(intptr_t)(b)); //get the search result (double cast to avoid warning)
	float relA = resA->relevance;
	float relB = resB->relevance;
	//printf("A: val %u rel %f\n",resA->resultVal[0],(double)resA->relevance);
	//printf("B: val %u rel %f\n",resB->resultVal[0],(double)resB->relevance);
	if(relA > relB){
		return -1;
	}else if(relA < relB){
		return 1;
	}
	return 0;
}

void sortAndAppendResult(search_state *restrict ss, search_result *restrict res){

	SDL_WaitSemaphore(ss->canUpdateResults); //wait while any other threads update the results
	
	//check that the result isn't identical to an existing one
	for(uint8_t i=0; i<ss->numUpdatedResults;i++){
		if(res->resultType == ss->updatedResults[i].resultType){
			if(res->resultVal[0] == ss->updatedResults[i].resultVal[0]){
				if(res->resultVal[1] == ss->updatedResults[i].resultVal[1]){
					SDL_SignalSemaphore(ss->canUpdateResults); //signal that other threads can now update the results
					return; //don't append identical results
				}
			}
		}
	}

	//boost result relevance if neccessary
	if(res->resultType == ss->boostedResultType){
		res->relevance *= 100.0f;
	}
	if((res->resultType == SEARCHAGENT_EGAMMA)||(res->resultType == SEARCHAGENT_ELEVEL)||(res->resultType == SEARCHAGENT_GAMMACASCADE)||(res->resultType == SEARCHAGENT_HALFLIFE)||(res->resultType == SEARCHAGENT_ELEVELDIFF)){
		if((ss->searchInProgress != SEARCHSTATE_SEARCHING_SINGLENUCL)&&(res->resultVal[0] == ss->boostedNucl)){
			//gamma, level, or half-life matching a nuclide
			res->relevance *= 100.0f;
		}
	}

	/*if(res->relevance < 1.0f){
		return; //don't process low relevance results
	}*/

	//SDL_Log("Appending result with type %u, values [%u %u], relevance %0.3f.\n",res->resultType,res->resultVal[0],res->resultVal[1],(double)res->relevance);
	//add the result to the list, if possible
	if(ss->numUpdatedResults < MAX_SEARCH_RESULTS){
		//append the result
		memcpy(&ss->updatedResults[ss->numUpdatedResults],res,sizeof(search_result));
		ss->numUpdatedResults++;
	}else{
		//assuming results are already sorted...
		//overwrite the lowest ranked result with the new result
		memcpy(&ss->updatedResults[MAX_SEARCH_RESULTS-1],res,sizeof(search_result));
	}
	
	//sort the results by relevance
	//SDL_Log("Results: %u\n",ss->numUpdatedResults);
	/*for(uint8_t i=0;i<ss->numUpdatedResults;i++){
		SDL_Log("  Result %u - rel: %f\n",i,(double)ss->updatedResults[i].relevance);
	}*/
	SDL_qsort(&ss->updatedResults,ss->numUpdatedResults,sizeof(ss->updatedResults[0]),compareRelevance);

	SDL_SignalSemaphore(ss->canUpdateResults); //signal that other threads can now update the results
}

//breaks the search string down into smaller tokens
void tokenizeSearchStr(search_state *restrict ss){
	char searchStrCpy[SEARCH_STRING_MAX_SIZE], *tok;
	char *saveptr = NULL;
	uint8_t numTok = 0;
	memcpy(searchStrCpy,ss->searchString,sizeof(ss->searchString));
	tok = SDL_strtok_r(searchStrCpy," ,",&saveptr);
	while(tok!=NULL){
		if(SDL_strlen(tok) > 0){
			SDL_strlcpy(ss->searchTok[numTok],tok,15);
			numTok++;
		}
		if(numTok >= MAX_SEARCH_TOKENS){
			break;
		}
		tok=SDL_strtok_r(NULL," ,",&saveptr);
	}
	ss->numSearchTok = numTok;

	/*printf("%u search tokens:",ss->numSearchTok);
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		printf(" %s",ss->searchTok[i]);
	}
	printf("\n");*/

}

void searchELevel(const ndata *restrict ndat, const app_state *state, search_state *ss){
	for(uint8_t i=0; i<state->ss.numSearchTok; i++){

		//first, filter out any tokens with characters
		uint8_t isNum = 1;
		for(uint16_t j=0; j<SDL_strlen(ss->searchTok[i]); j++){
			if(isalpha(ss->searchTok[i][j])){
				isNum = 0;
				break;
			}
		}
		if(isNum == 0){
			continue; //check next search token
		}

		double eSearch = SDL_atof(ss->searchTok[i]);
		if(eSearch > 0.0){
			//valid energy
			for(int16_t j=0; j<ndat->numNucl; j++){

				if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(j!=ss->boostedNucl)){
					//if doing a single-nuclide search, skip all other nuclides
					continue;
				}

				float proximityFactor = 0.0f;
				if(state->ds.chartZoomScale > 5.0f){
					if(state->chartSelectedNucl != MAXNUMNUCL){
						//offset proximity center point based on presence of nuclide info box
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY - (16.0f/state->ds.chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}else{
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}
					if(proximityFactor < 2.0f){
						proximityFactor = 2.0f;
					}
					proximityFactor = 1.0f*state->ds.chartZoomScale/proximityFactor;
					if(proximityFactor > 100.0f){
						proximityFactor = 100.0f;
					}
				}

				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					
					//for single nuclide searches, if a specific reaction is selected,
					//do not search levels that are not populated in that reaction
					if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(state->ds.selectedRxn != 0)){
						if(isLvlDisplayed(ndat,state,(uint16_t)j,(uint16_t)(k -ndat->nuclData[j].firstLevel))==0){
							continue;
						}
					}
								
					if((((ndat->levels[k].energy.format >> 5U) & 15U)) == VALUETYPE_NUMBER){ //ignore variable energy
						double rawEVal = getRawValFromDB(&ndat->levels[k].energy);
						double rawErrVal = getRawErrFromDB(&ndat->levels[k].energy);
						if(rawEVal > 0.0){
							double errBound = 3.0*rawErrVal;
							if(errBound < rawEVal*0.005){
								errBound = rawEVal*0.005;
							}
							if(errBound < 3.0){
								errBound = 3.0;
							}
							if(ss->broadSearch == 1){
								errBound = errBound*5.0;
							}
							if(((rawEVal - errBound) <= eSearch)&&((rawEVal + errBound) >= eSearch)){
								//energy matches query
								search_result res;
								res.relevance = 0.6f; //base value
								res.relevance += proximityFactor;
								res.relevance -= (float)(rawErrVal/rawEVal); //weight by size of error bars
								res.relevance /= (1.0f + (float)fabs(0.1*(eSearch - rawEVal))); //weight by distance from value
								res.resultType = SEARCHAGENT_ELEVEL;
								res.resultVal[0] = (uint32_t)j; //nuclide index
								res.resultVal[1] = (uint32_t)k; //level index
								//SDL_Log("Found level %u\n",res.resultVal[1]);
								sortAndAppendResult(ss,&res);
							}
						}
					}
				}
			}
		}
	}
}

void searchELevelDiff(const ndata *restrict ndat, const app_state *state, search_state *ss){

	//only search this type if asked to
	if(ss->boostedResultType != SEARCHAGENT_ELEVELDIFF){
		return;
	}

	for(uint8_t i=0; i<ss->numSearchTok; i++){

		//first, filter out any tokens with characters
		uint8_t isNum = 1;
		for(uint16_t j=0; j<SDL_strlen(ss->searchTok[i]); j++){
			if(isalpha(ss->searchTok[i][j])){
				isNum = 0;
				break;
			}
		}
		if(isNum == 0){
			continue; //check next search token
		}

		double eSearch = SDL_atof(ss->searchTok[i]);
		if(eSearch > 0.0){
			//valid energy
			for(uint16_t j=0; j<ndat->numNucl; j++){

				if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(j!=ss->boostedNucl)){
					//if doing a single-nuclide search, skip all other nuclides
					continue;
				}

				float proximityFactor = 0.0f;
				if(state->ds.chartZoomScale > 5.0f){
					if(state->chartSelectedNucl != MAXNUMNUCL){
						//offset proximity center point based on presence of nuclide info box
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY - (16.0f/state->ds.chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}else{
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}
					if(proximityFactor < 2.0f){
						proximityFactor = 2.0f;
					}
					proximityFactor = 1.0f*state->ds.chartZoomScale/proximityFactor;
					if(proximityFactor > 100.0f){
						proximityFactor = 100.0f;
					}
				}
				
				uint32_t lastLvlInd = (ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels);
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<lastLvlInd; k++){

					//for single nuclide searches, if a specific reaction is selected,
					//do not search levels that are not populated in that reaction
					if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(state->ds.selectedRxn != 0)){
						if(isLvlDisplayed(ndat,state,(uint16_t)j,(uint16_t)(k -ndat->nuclData[j].firstLevel))==0){
							continue;
						}
					}

					if((((ndat->levels[k].energy.format >> 5U) & 15U)) == VALUETYPE_NUMBER){ //ignore variable energy
						for(uint32_t l=k+1; l<lastLvlInd; l++){
							if((((ndat->levels[l].energy.format >> 5U) & 15U)) == VALUETYPE_NUMBER){ //ignore variable energy

								double diffVal = SDL_fabs(getRawValFromDB(&ndat->levels[l].energy) - getRawValFromDB(&ndat->levels[k].energy));
								double errBound = 3.0*(getRawErrFromDB(&ndat->levels[l].energy) + getRawErrFromDB(&ndat->levels[k].energy));
								if(errBound < diffVal*0.005){
									errBound = diffVal*0.005;
								}
								if(ss->broadSearch == 1){
									errBound = errBound*5.0;
								}

								if((diffVal - errBound) > eSearch){
									//level energy difference is too high
									//assume levels are ordered by energy
									break;
								}else if(diffVal > 0.0){
									if((diffVal + errBound) >= eSearch){
										//energy matches query
										search_result res;
										res.relevance = 1.0f; //base value
										res.relevance += proximityFactor;
										res.relevance -= (float)(errBound/diffVal); //weight by size of error bars
										res.relevance -= (float)(getRawValFromDB(&ndat->levels[l].energy)/1000000.0); //weight by level energy (prefer lower levels)
										res.relevance /= (1.0f + (float)fabs(0.1*(eSearch - diffVal))); //weight by distance from value
										//SDL_Log("relevance: %f\n",(double)res.relevance);
										if(res.relevance > 0.9f){
											res.resultType = SEARCHAGENT_ELEVELDIFF;
											res.resultVal[0] = (uint32_t)j; //nuclide index
											res.resultVal[1] = (uint32_t)k; //level index
											res.resultVal[2] = (uint32_t)l; //level index
											//SDL_Log("Found level %u\n",res.resultVal[1]);
											sortAndAppendResult(ss,&res);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void searchEGamma(const ndata *restrict ndat, const app_state *state, search_state *ss){
	for(uint8_t i=0; i<ss->numSearchTok; i++){

		//first, filter out any tokens with characters
		uint8_t isNum = 1;
		for(uint16_t j=0; j<SDL_strlen(ss->searchTok[i]); j++){
			if(isalpha(ss->searchTok[i][j])){
				isNum = 0;
				break;
			}
		}
		if(isNum == 0){
			continue; //check next search token
		}

		double eSearch = SDL_atof(ss->searchTok[i]);
		if(eSearch > 0.0){
			//valid energy
			for(int16_t j=0; j<ndat->numNucl; j++){

				if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(j!=ss->boostedNucl)){
					//if doing a single-nuclide search, skip all other nuclides
					continue;
				}

				float proximityFactor = 0.0f;
				if(state->ds.chartZoomScale > 5.0f){
					if(state->chartSelectedNucl != MAXNUMNUCL){
						//offset proximity center point based on presence of nuclide info box
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY - (16.0f/state->ds.chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}else{
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}
					if(proximityFactor < 2.0f){
						proximityFactor = 2.0f;
					}
					proximityFactor = 1.0f*state->ds.chartZoomScale/proximityFactor;
					if(proximityFactor > 100.0f){
						proximityFactor = 100.0f;
					}
				}
				
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					
					//for single nuclide searches, if a specific reaction is selected,
					//do not search levels that are not populated in that reaction
					if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(state->ds.selectedRxn != 0)){
						if(isLvlDisplayed(ndat,state,(uint16_t)j,(uint16_t)(k -ndat->nuclData[j].firstLevel))==0){
							continue;
						}
					}
					
					for(uint32_t l=ndat->levels[k].firstTran; l<(ndat->levels[k].firstTran + (uint32_t)ndat->levels[k].numTran); l++){
						if((((ndat->tran[l].energy.format >> 5U) & 15U)) != VALUETYPE_X){ //ignore variable energy
							double rawEVal = getRawValFromDB(&ndat->tran[l].energy);
							double rawErrVal = getRawErrFromDB(&ndat->tran[l].energy);
							if(rawEVal > 0.0){
								double errBound = 3.0*rawErrVal;
								if(errBound < rawEVal*0.005){
									errBound = rawEVal*0.005;
								}
								if(errBound < 3.0){
									errBound = 3.0;
								}
								if(ss->broadSearch == 1){
									errBound = errBound*5.0;
								}
								if(((rawEVal - errBound) <= eSearch)&&((rawEVal + errBound) >= eSearch)){
									//energy matches query
									search_result res;
									res.relevance = 0.5f; //base value
									res.relevance += proximityFactor;
									res.relevance -= (float)(rawErrVal/rawEVal); //weight by size of error bars
									res.relevance /= (1.0f + (float)fabs(0.1*(eSearch - rawEVal))); //weight by distance from value
									//SDL_Log("proximity factor: %f\n",(double)proximityFactor);
									uint8_t intensityType = (uint8_t)((ndat->tran[l].energy.format >> 5U) & 15U);
									switch(intensityType){
										case VALUETYPE_NUMBER:
										case VALUETYPE_GREATERTHAN:
										case VALUETYPE_GREATEROREQUALTHAN:
										case VALUETYPE_APPROX:
											{//prevent -Wjump-misses-init
												float intensityFactor = (float)getRawValFromDB(&ndat->tran[l].intensity)/100.0f;
												if(intensityFactor > 1.0f){
													intensityFactor = 1.0f;
												}
												res.relevance *= intensityFactor;
											}
											break;
										default:
											res.relevance *= 0.01f;
											break;
									}
									res.resultType = SEARCHAGENT_EGAMMA;
									res.resultVal[0] = (uint32_t)j; //nuclide index
									res.resultVal[1] = (uint32_t)l; //transition index
									res.resultVal[2] = (uint32_t)k; //level index
									res.resultVal[3] = 0; //normal gamma energy result
									//SDL_Log("Found transition %u\n",res.resultVal[1]);
									sortAndAppendResult(ss,&res);
								}else if((rawEVal > 1022.0)&&((((rawEVal - 511.0) - errBound) <= eSearch)&&(((rawEVal - 511.0) + errBound) >= eSearch))){
									//energy matches query
									search_result res;
									res.relevance = 0.4f; //base value
									res.relevance += proximityFactor;
									res.relevance -= (float)(rawErrVal/rawEVal); //weight by size of error bars
									res.relevance /= (3.0f + (float)fabs(0.1*(eSearch - (rawEVal - 511.0)))); //weight by distance from value
									//SDL_Log("proximity factor: %f\n",(double)proximityFactor);
									uint8_t intensityType = (uint8_t)((ndat->tran[l].energy.format >> 5U) & 15U);
									switch(intensityType){
										case VALUETYPE_NUMBER:
										case VALUETYPE_GREATERTHAN:
										case VALUETYPE_GREATEROREQUALTHAN:
										case VALUETYPE_APPROX:
											{//prevent -Wjump-misses-init
												float intensityFactor = (float)getRawValFromDB(&ndat->tran[l].intensity)/100.0f;
												if(intensityFactor > 1.0f){
													intensityFactor = 1.0f;
												}
												res.relevance *= intensityFactor;
											}
											break;
										default:
											res.relevance *= 0.01f;
											break;
									}
									res.resultType = SEARCHAGENT_EGAMMA;
									res.resultVal[0] = (uint32_t)j; //nuclide index
									res.resultVal[1] = (uint32_t)l; //transition index
									res.resultVal[2] = (uint32_t)k; //level index
									res.resultVal[3] = 511; //flag as a single escape peak
									//SDL_Log("Found transition %u\n",res.resultVal[1]);
									sortAndAppendResult(ss,&res);
								}else if((rawEVal > 1022.0)&&((((rawEVal - 1022.0) - errBound) <= eSearch)&&(((rawEVal - 1022.0) + errBound) >= eSearch))){
									//energy matches query
									search_result res;
									res.relevance = 0.4f; //base value
									res.relevance += proximityFactor;
									res.relevance -= (float)(rawErrVal/rawEVal); //weight by size of error bars
									res.relevance /= (4.0f + (float)fabs(0.1*(eSearch - (rawEVal - 1022.0)))); //weight by distance from value
									//SDL_Log("proximity factor: %f\n",(double)proximityFactor);
									uint8_t intensityType = (uint8_t)((ndat->tran[l].energy.format >> 5U) & 15U);
									switch(intensityType){
										case VALUETYPE_NUMBER:
										case VALUETYPE_GREATERTHAN:
										case VALUETYPE_GREATEROREQUALTHAN:
										case VALUETYPE_APPROX:
											{//prevent -Wjump-misses-init
												float intensityFactor = (float)getRawValFromDB(&ndat->tran[l].intensity)/100.0f;
												if(intensityFactor > 1.0f){
													intensityFactor = 1.0f;
												}
												res.relevance *= intensityFactor;
											}
											break;
										default:
											res.relevance *= 0.01f;
											break;
									}
									res.resultType = SEARCHAGENT_EGAMMA;
									res.resultVal[0] = (uint32_t)j; //nuclide index
									res.resultVal[1] = (uint32_t)l; //transition index
									res.resultVal[2] = (uint32_t)k; //level index
									res.resultVal[3] = 1022; //flag as a double escape peak
									//SDL_Log("Found transition %u\n",res.resultVal[1]);
									sortAndAppendResult(ss,&res);
								}
							}
						}
					}
				}
			}
		}
	}
}

void searchGammaCascade(const ndata *restrict ndat, const app_state *state, search_state *ss){
	
	uint8_t numCascadeGammas = 0;
	double cascadeGammas[MAX_CASCADE_GAMMAS];
	uint32_t matchedTran[MAX_CASCADE_GAMMAS];

	for(uint8_t i=0; i<ss->numSearchTok; i++){

		//first, filter out any tokens with characters
		uint8_t isNum = 1;
		for(uint16_t j=0; j<SDL_strlen(ss->searchTok[i]); j++){
			if(isalpha(ss->searchTok[i][j])){
				isNum = 0;
				break;
			}
		}

		if(isNum == 0){
			continue; //check next search token
		}else{
			cascadeGammas[numCascadeGammas] = SDL_atof(ss->searchTok[i]);
			if(cascadeGammas[numCascadeGammas] > 0.0){
				numCascadeGammas++;
				if(numCascadeGammas >= MAX_CASCADE_GAMMAS){
					break; //array is full
				}
			}
		}

	}

	if(numCascadeGammas > 1){
		//search for nuclides containing all of the cascade's gammas in coincidenc
		for(int16_t i=0; i<ndat->numNucl; i++){

			if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(i!=ss->boostedNucl)){
				//if doing a single-nuclide search, skip all other nuclides
				continue;
			}

			if(ndat->nuclData[i].numLevels > 1){

				float proximityFactor = 0.0f;
				if(state->ds.chartZoomScale > 5.0f){
					if(state->chartSelectedNucl != MAXNUMNUCL){
						//offset proximity center point based on presence of nuclide info box
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[i].Z - state->ds.chartPosY - (16.0f/state->ds.chartZoomScale)) + fabsf((float)ndat->nuclData[i].N - state->ds.chartPosX) + 0.1f);
					}else{
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[i].Z - state->ds.chartPosY) + fabsf((float)ndat->nuclData[i].N - state->ds.chartPosX) + 0.1f);
					}
					if(proximityFactor < 2.0f){
						proximityFactor = 2.0f;
					}
					proximityFactor = 1.0f*state->ds.chartZoomScale/proximityFactor;
					if(proximityFactor > 100.0f){
						proximityFactor = 100.0f;
					}
				}
				
				for(uint32_t j=(ndat->nuclData[i].firstLevel + (uint32_t)(ndat->nuclData[i].numLevels - 1)); j>=ndat->nuclData[i].firstLevel; j--){
					if(j==0){
						break; //safety valve
					}
					
					//for single nuclide searches, if a specific reaction is selected,
					//do not search levels that are not populated in that reaction
					if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(state->ds.selectedRxn != 0)){
						if(isLvlDisplayed(ndat,state,(uint16_t)i,(uint16_t)(j -ndat->nuclData[i].firstLevel))==0){
							continue;
						}
					}
					
					for(uint32_t k=ndat->levels[j].firstTran; k<(ndat->levels[j].firstTran + (uint32_t)ndat->levels[j].numTran); k++){
						if((((ndat->tran[k].energy.format >> 5U) & 15U)) != VALUETYPE_X){ //ignore variable energy
							double rawEVal = getRawValFromDB(&ndat->tran[k].energy);
							double rawErrVal = getRawErrFromDB(&ndat->tran[k].energy);
							if(rawEVal > 0.0){
								double errBound = 3.0*rawErrVal;
								if(errBound < rawEVal*0.005){
									errBound = rawEVal*0.005;
								}
								if(ss->broadSearch == 1){
									errBound = errBound*5.0;
								}
								
								for(uint8_t l=0; l<numCascadeGammas; l++){
									if(((rawEVal - errBound) <= cascadeGammas[l])&&((rawEVal + errBound) >= cascadeGammas[l])){
										//energy matches query
										matchedTran[0] = k;
										uint8_t numGammasMatched = 1;
										uint8_t gammasMatched = 0; //bit-pattern of matched gammas
										gammasMatched |= (uint8_t)(1U << l);
										double intensityFactor = getRawValFromDB(&ndat->tran[k].intensity);
										double energyFactor = (1.0 + fabs(0.1*(cascadeGammas[l] - rawEVal)));
										double rawLvlE = getRawValFromDB(&ndat->levels[j].energy) - rawEVal;
										//search the lower levels for other cascade members
										for(uint32_t m=(uint32_t)(j-1); m>=ndat->nuclData[i].firstLevel; m--){
											if(numGammasMatched < numCascadeGammas){
												if(SDL_fabs(getRawValFromDB(&ndat->levels[m].energy) - rawLvlE) < errBound){
													uint8_t nextCascMemberFound = 0;
													for(uint32_t n=ndat->levels[m].firstTran; n<(ndat->levels[m].firstTran + (uint32_t)ndat->levels[m].numTran); n++){
														if((((ndat->tran[n].energy.format >> 5U) & 15U)) != VALUETYPE_X){ //ignore variable energy
															rawEVal = getRawValFromDB(&ndat->tran[n].energy);
															rawErrVal = getRawErrFromDB(&ndat->tran[n].energy);
															if(rawEVal > 0.0){
																errBound = 3.0*rawErrVal;
																if(errBound < 5.0){
																	errBound = 5.0;
																}
															}
															
															for(uint8_t p=0; p<numCascadeGammas; p++){
																if(!(gammasMatched & (1U << p))){
																	//matches energy of cascade member not previously found
																	if(((rawEVal - errBound) <= cascadeGammas[p])&&((rawEVal + errBound) >= cascadeGammas[p])){
																		//energy matches query
																		nextCascMemberFound = 1;
																		matchedTran[numGammasMatched] = n;
																		numGammasMatched++;
																		gammasMatched |= (uint8_t)(1U << p);
																		intensityFactor += getRawValFromDB(&ndat->tran[n].intensity);
																		energyFactor *= (1.0 + fabs(0.1*(cascadeGammas[p] - rawEVal)));
																		rawLvlE = getRawValFromDB(&ndat->levels[m].energy) - rawEVal;
																		break;
																	}
																}
															}
														}
														if(nextCascMemberFound){
															break;
														}
													}
												}
											}
										}
										if(numGammasMatched == numCascadeGammas){
											//full cascade identified
											search_result res;
											res.relevance = 1.0f; //base value
											res.relevance += (float)intensityFactor/100.0f; //weight by intensity of gammas (and implicitly by multiplicity of cascade)
											res.relevance /= (float)(energyFactor); //weight by distance of energies from search values
											res.relevance += proximityFactor;
											res.resultType = SEARCHAGENT_GAMMACASCADE;
											res.resultVal[0] = (uint32_t)i; //nuclide index
											for(uint8_t ind=1; ind<=numGammasMatched; ind++){
												if((ind < SEARCH_RESULT_DATASIZE)&&(ind <= MAX_CASCADE_GAMMAS)){
													res.resultVal[ind] = matchedTran[ind-1]; //transition index (first cascade member)
												}
											}
											if((numGammasMatched+1) < SEARCH_RESULT_DATASIZE){
												res.resultVal[numGammasMatched+1] = UNUSED_SEARCH_RESULT; //truncate results
											}
											//SDL_Log("Found cascade in nuclide %u starting with transition %u\n",res.resultVal[0],res.resultVal[1]);
											sortAndAppendResult(ss,&res);
										}
									}
								}

							}
						}
					}
				}
			}
			
		}

		//SDL_Log("Gamma cascade search finished.\n");
	}

}

void searchHalfLife(const ndata *restrict ndat, const app_state *state, search_state *ss){
	for(uint8_t i=0; i<ss->numSearchTok; i++){

		//first, filter out any tokens with characters
		uint8_t isNum = 1;
		for(uint16_t j=0; j<SDL_strlen(ss->searchTok[i]); j++){
			if(isalpha(ss->searchTok[i][j])){
				isNum = 0;
				break;
			}
		}
		if(isNum == 0){
			continue; //check next search token
		}

		double hlSearch = SDL_atof(ss->searchTok[i]);
		if(hlSearch > 0.0){
			//valid energy
			if(state->ds.useLifetimes){
				hlSearch /= 1.4427; //convert lifetime to half-life
			}
			for(int16_t j=0; j<ndat->numNucl; j++){

				if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(j!=ss->boostedNucl)){
					//if doing a single-nuclide search, skip all other nuclides
					continue;
				}

				float proximityFactor = 0.0f;
				if(state->ds.chartZoomScale > 5.0f){
					if(state->chartSelectedNucl != MAXNUMNUCL){
						//offset proximity center point based on presence of nuclide info box
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY - (16.0f/state->ds.chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}else{
						proximityFactor = SDL_sqrtf(fabsf((float)ndat->nuclData[j].Z - state->ds.chartPosY) + fabsf((float)ndat->nuclData[j].N - state->ds.chartPosX) + 0.1f);
					}
					if(proximityFactor < 2.0f){
						proximityFactor = 2.0f;
					}
					proximityFactor = 1.0f*state->ds.chartZoomScale/proximityFactor;
					if(proximityFactor > 100.0f){
						proximityFactor = 100.0f;
					}
				}
				
				//SDL_Log("proximityFactor: %f\n",(double)proximityFactor);
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					
					//for single nuclide searches, if a specific reaction is selected,
					//do not search levels that are not populated in that reaction
					if((ss->searchInProgress == SEARCHSTATE_SEARCHING_SINGLENUCL)&&(state->ds.selectedRxn != 0)){
						if(isLvlDisplayed(ndat,state,(uint16_t)j,(uint16_t)(k -ndat->nuclData[j].firstLevel))==0){
							continue;
						}
					}
					
					uint8_t hlValueType = (uint8_t)((ndat->levels[k].halfLife.format >> 5U) & 15U);
					if((hlValueType == VALUETYPE_NUMBER)||(hlValueType == VALUETYPE_ASYMERROR)){
						double rawHlVal = getRawValFromDB(&ndat->levels[k].halfLife);
						if(rawHlVal > 0.0){
							double rawErrVal = getRawErrFromDB(&ndat->levels[k].halfLife);
							double errBound = 3.0*rawErrVal;
							if(errBound < rawHlVal*0.005){
								errBound = rawHlVal*0.005;
							}
							if(ss->broadSearch == 1){
								errBound = errBound*5.0;
							}
							if(((rawHlVal - errBound) <= hlSearch)&&((rawHlVal + errBound) >= hlSearch)){
								//energy matches query
								search_result res;
								//set base value differently for different types of states
								if(k != (ndat->nuclData[j].firstLevel + ndat->nuclData[j].gsLevel)){
									//de-prioritize short lived excited states
									if(getLevelHalfLifeSeconds(ndat,k)<1.0E-6){
										res.relevance = 0.07f; 
									}else{
										res.relevance = 0.7f; 
									}
								}else{
									res.relevance = 0.7f;
								}
								res.relevance += proximityFactor;
								res.relevance -= (float)(rawErrVal/rawHlVal); //weight by size of error bars
								res.relevance /= (1.0f + (float)fabs(0.1*(hlSearch - rawHlVal))); //weight by distance from value
								res.resultType = SEARCHAGENT_HALFLIFE;
								res.resultVal[0] = (uint32_t)j; //nuclide index
								res.resultVal[1] = (uint32_t)k; //level index
								//SDL_Log("Found transition %u\n",res.resultVal[1]);
								sortAndAppendResult(ss,&res);
							}
						}
					}
				}
			}
		}
	}
}

void searchNuclides(const ndata *restrict ndat, search_state *restrict ss){
	char nuclAStr[8], nuclElemName[32];
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		uint8_t foundNucl = 0;
		uint8_t len = (uint8_t)SDL_strlen(ss->searchTok[i]);
		if(isdigit(ss->searchTok[i][0])){
			//look for nuclide names starting with a digit (eg. 32Si)
			for(uint8_t j=1; j<len; j++){
				if(j>=7){
					break;
				}
				if(ss->searchTok[i][j] == '-'){
					memcpy(&nuclAStr,&ss->searchTok[i],(size_t)j);
					nuclAStr[j] = '\0'; //terminate string at end
					if((len > (j+1))&&((len-j+1)<32)){
						memcpy(&nuclElemName,&(ss->searchTok[i][j+1]),(size_t)(len-j+1));
						nuclElemName[len-j+1] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}else if(!(isdigit(ss->searchTok[i][j]))){
					memcpy(&nuclAStr,&ss->searchTok[i],(size_t)j);
					nuclAStr[j] = '\0'; //terminate string at end
					if((len > j)&&((len-j)<32)){
						memcpy(&nuclElemName,&(ss->searchTok[i][j]),(size_t)(len-j));
						nuclElemName[len-j] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}
			}
			if(foundNucl == 0){
				//try looking at the full string, for special cases
				SDL_strlcpy(nuclElemName,ss->searchTok[i],31);
				SDL_strlcpy(nuclAStr,"-1",7); //mark special case
				//SDL_Log("Parsing special case: %s\n",ss->searchTok[i]);
			}
		}else if(isalpha(ss->searchTok[i][0])){
			//look for nuclide names starting with a letter (eg. si32)
			for(uint8_t j=1; j<len; j++){
				if(j>=32){
					break;
				}
				if(ss->searchTok[i][j] == '-'){
					memcpy(&nuclElemName,&ss->searchTok[i],(size_t)j);
					nuclElemName[j] = '\0'; //terminate string at end
					if((len > (j+1))&&((len-j+1)<8)){
						memcpy(&nuclAStr,&(ss->searchTok[i][j+1]),(size_t)(len-j+1));
						nuclAStr[len-j+1] = '\0'; //terminate string at end
						foundNucl = 1;
						break;
					}
				}else if(isdigit(ss->searchTok[i][j])){
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
			if(foundNucl == 0){
				//try looking at the full string, for special cases
				SDL_strlcpy(nuclElemName,ss->searchTok[i],31);
				SDL_strlcpy(nuclAStr,"-1",7); //mark special case
				//SDL_Log("Parsing special case: %s\n",ss->searchTok[i]);
			}
		}
		int16_t nuclZ = -1;
		int16_t nuclZu = -1;
		int16_t nuclA = (int16_t)SDL_atoi(nuclAStr);
		//SDL_Log("A: %i, Z: %i, elem name: %s, len: %u\n",nuclA,nuclZ,nuclElemName,len);
		if(nuclA < 0){
			//check for special cases
			nuclElemName[0] = (char)SDL_toupper(nuclElemName[0]); //convert to uppercase
			if(strcmp(nuclElemName,"Neutron")==0){
				nuclZ = 0;
				nuclA = 1;
			}else if(strcmp(nuclElemName,"Tetraneutron")==0){
				nuclZ = 0;
				nuclA = 4;
			}else if((strcmp(nuclElemName,"Proton")==0)||(strcmp(nuclElemName,"Hydrogen")==0)){
				nuclZ = 1;
				nuclA = 1;
			}else if(strcmp(nuclElemName,"Deuterium")==0){
				nuclZ = 1;
				nuclA = 2;
			}else if(strcmp(nuclElemName,"Tritium")==0){
				nuclZ = 1;
				nuclA = 3;
			}else{
				continue; //go to the next token
			}
		}else{
			nuclZ = (int16_t)elemStrToZ(nuclElemName);
			nuclElemName[0] = (char)SDL_toupper(nuclElemName[0]); //convert to uppercase
			nuclZu = (int16_t)elemStrToZ(nuclElemName);
		}
		for(int16_t j=0; j<ndat->numNucl; j++){
			if(nuclZ >= 0){
				if(ndat->nuclData[j].Z == nuclZ){
					if(((ndat->nuclData[j].N + ndat->nuclData[j].Z)) == nuclA){
						if((ndat->nuclData[j].flags & 3U) == OBSFLAG_OBSERVED){
							//identified nuclide (exact match)
							search_result res;
							res.relevance = 1.0f; //base value
							res.resultType = SEARCHAGENT_NUCLIDE;
							res.resultVal[0] = (uint32_t)j; //nuclide index
							sortAndAppendResult(ss,&res);
							ss->boostedNucl = (uint16_t)j;
						}
					}
				}
			}
			if(nuclZu >= 0){
				if((nuclZu != nuclZ)&&(ndat->nuclData[j].Z == nuclZu)){
					if(((ndat->nuclData[j].N + ndat->nuclData[j].Z)) == nuclA){
						if((ndat->nuclData[j].flags & 3U) == OBSFLAG_OBSERVED){
							//identified nuclide (uppercase match)
							search_result res;
							res.relevance = 1.0f; //base value
							res.resultType = SEARCHAGENT_NUCLIDE;
							res.resultVal[0] = (uint32_t)j; //nuclide index
							sortAndAppendResult(ss,&res);
							ss->boostedNucl = (uint16_t)j;
						}
					}
				}
			}
		}
	}

	//SDL_Log("Number of search results: %u\n",ss->numUpdatedResults);
}

//search for special strings (eg. 'level') which can be used to modify the
//priority of search results
void searchSpecialStrings(search_state *restrict ss){
	
	
	char tmpStr[64];
	for(uint8_t i=0; i<ss->numSearchTok; i++){
		
		SDL_strlcpy(tmpStr,ss->searchTok[i],64);
		uint8_t len = (uint8_t)SDL_strlen(tmpStr);
		for(uint8_t j=0; j<len; j++){
			tmpStr[j] = (char)SDL_tolower(tmpStr[j]); //convert entire string to lowercase
		}
		if((SDL_strncmp(tmpStr,"lev",3)==0)||(SDL_strcmp(tmpStr,"lvl")==0)||(SDL_strcmp(tmpStr,"state")==0)){
			ss->boostedResultType = SEARCHAGENT_ELEVEL;
			return;
		}else if((SDL_strcmp(tmpStr,"gam")==0)||(SDL_strncmp(tmpStr,"tran",4)==0)){
			ss->boostedResultType = SEARCHAGENT_EGAMMA;
			return;
		}else if(SDL_strncmp(tmpStr,"casc",4)==0){
			ss->boostedResultType = SEARCHAGENT_GAMMACASCADE;
			return;
		}else if((SDL_strcmp(tmpStr,"hl")==0)||(SDL_strcmp(tmpStr,"halflife")==0)||(SDL_strcmp(tmpStr,"half life")==0)||(SDL_strcmp(tmpStr,"lt")==0)||(SDL_strcmp(tmpStr,"lifetime")==0)){
			ss->boostedResultType = SEARCHAGENT_HALFLIFE;
			return;
		}else if(SDL_strncmp(tmpStr,"diff",4)==0){
			ss->boostedResultType = SEARCHAGENT_ELEVELDIFF;
			return;
		}else if((SDL_strcmp(tmpStr,"broad")==0)||(SDL_strcmp(tmpStr,"wide")==0)){
			ss->broadSearch = 1;
			return;
		}
		
	}

	//SDL_Log("Number of search results: %u\n",ss->numUpdatedResults);
}
