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
	for(uint8_t i=0; i<64; i++){
		if(resA->corrRes & (uint64_t)(1U << i)){
			relA += 1.0f;
			relA *= 10.0f;
		}
		if(resB->corrRes & (uint64_t)(1U << i)){
			relB += 1.0f;
			relB *= 10.0f;
		}
	}
	if(relA >= relB){
		return -1;
	}else{
		return 1;
	}
}

void sortAndAppendResult(search_state *restrict ss, const search_result *restrict res){

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
	//SDL_Log("Appending result with type %u, values [%u %u], relevance %0.3f.\n",res->resultType,res->resultVal[0],res->resultVal[1],(double)res->relevance);
	//add the result to the list, if possible
	if(ss->numUpdatedResults < MAX_SEARCH_RESULTS){
		//append the result
		memcpy(&ss->updatedResults[ss->numUpdatedResults],res,sizeof(search_result));
		ss->numUpdatedResults++;
	}else{
		//assuming results are already sorted...
		//overwrite that lowest ranked result with the new result
		memcpy(&ss->updatedResults[MAX_SEARCH_RESULTS-1],res,sizeof(search_result));
	}

	//check for correlations between results
	for(uint8_t i=0; i<ss->numUpdatedResults; i++){
		ss->updatedResults[i].corrRes = 0;
		for(uint8_t j=0; j<ss->numUpdatedResults; j++){
			if(i != j){
				if((ss->updatedResults[i].resultType == SEARCHAGENT_EGAMMA)||(ss->updatedResults[i].resultType == SEARCHAGENT_ELEVEL)||(ss->updatedResults[i].resultType == SEARCHAGENT_GAMMACASCADE)||(ss->updatedResults[i].resultType == SEARCHAGENT_HALFLIFE)){
					if(ss->updatedResults[j].resultType == SEARCHAGENT_NUCLIDE){
						if(ss->updatedResults[i].resultVal[0] == ss->updatedResults[j].resultVal[0]){
							//gamma or half-life matching a nuclide
							//SDL_Log("Correlated result %u with result %u\n",i,j);
							ss->updatedResults[i].corrRes |= (uint64_t)(1U << j);
						}
					}
				}
			}
		}
	}
	
	//sort the results by relevance
	//SDL_Log("Results: %u\n",ss->numUpdatedResults);
	/*for(uint8_t i=0;i<ss->numUpdatedResults;i++){
		SDL_Log("  Result %u - rel: %f\n",i,(double)ss->updatedResults[i].relevance);
	}*/
	qsort(&ss->updatedResults,ss->numUpdatedResults,sizeof(ss->updatedResults[0]),compareRelevance); //SDL_qsort appears to have a bug right now that causes it to sort out of bounds in the array...

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

void searchELevel(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss){
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
				float proximityFactor = sqrtf(fabsf((float)ndat->nuclData[j].Z - ds->chartPosY - (16.0f/ds->chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - ds->chartPosX) + 0.1f);
				proximityFactor = 0.6f*ds->chartZoomScale/proximityFactor;
				if(proximityFactor > 1.0f){
					proximityFactor = 1.0f;
				}
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					if((((ndat->levels[k].energy.format >> 5U) & 15U)) == VALUETYPE_NUMBER){ //ignore variable energy
						double rawEVal = getRawValFromDB(&ndat->levels[k].energy);
						double rawErrVal = getRawErrFromDB(&ndat->levels[k].energy);
						if(rawEVal > 0.0){
							double errBound = 3.0*rawErrVal;
							if(errBound < 5.0){
								errBound = 5.0;
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
								res.corrRes = 0;
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

void searchEGamma(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss){
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
				float proximityFactor = sqrtf(fabsf((float)ndat->nuclData[j].Z - ds->chartPosY - (16.0f/ds->chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - ds->chartPosX) + 0.1f);
				proximityFactor = 0.5f*ds->chartZoomScale/proximityFactor;
				if(proximityFactor > 1.0f){
					proximityFactor = 1.0f;
				}
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					for(uint32_t l=ndat->levels[k].firstTran; l<(ndat->levels[k].firstTran + (uint32_t)ndat->levels[k].numTran); l++){
						if((((ndat->tran[l].energy.format >> 5U) & 15U)) != VALUETYPE_X){ //ignore variable energy
							double rawEVal = getRawValFromDB(&ndat->tran[l].energy);
							double rawErrVal = getRawErrFromDB(&ndat->tran[l].energy);
							if(rawEVal > 0.0){
								double errBound = 3.0*rawErrVal;
								if(errBound < 5.0){
									errBound = 5.0;
								}
								if(((rawEVal - errBound) <= eSearch)&&((rawEVal + errBound) >= eSearch)){
									//energy matches query
									search_result res;
									res.relevance = 0.5f; //base value
									res.relevance += proximityFactor;
									res.relevance -= (float)(rawErrVal/rawEVal); //weight by size of error bars
									res.relevance /= (1.0f + (float)fabs(0.1*(eSearch - rawEVal))); //weight by distance from value
									uint8_t intensityType = (uint8_t)((ndat->tran[l].energy.format >> 5U) & 15U);
									switch(intensityType){
										case VALUETYPE_NUMBER:
										case VALUETYPE_GREATERTHAN:
										case VALUETYPE_GREATEROREQUALTHAN:
										case VALUETYPE_APPROX:
											; //get around gcc warning
											float intensityFactor = (float)getRawValFromDB(&ndat->tran[l].intensity)/100.0f;
											if(intensityFactor > 1.0f){
												intensityFactor = 1.0f;
											}
											res.relevance *= intensityFactor;
											break;
										default:
											res.relevance *= 0.01f;
											break;
									}
									res.resultType = SEARCHAGENT_EGAMMA;
									res.resultVal[0] = (uint32_t)j; //nuclide index
									res.resultVal[1] = (uint32_t)l; //transition index
									res.corrRes = 0;
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

void searchGammaCascade(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss){
	
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
			if(ndat->nuclData[i].numLevels > 1){
				float proximityFactor = sqrtf(fabsf((float)ndat->nuclData[i].Z - ds->chartPosY - (16.0f/ds->chartZoomScale)) + fabsf((float)ndat->nuclData[i].N - ds->chartPosX) + 0.1f);
				proximityFactor = 1.0f*ds->chartZoomScale/proximityFactor;
				if(proximityFactor > 1.0f){
					proximityFactor = 1.0f;
				}
				for(uint32_t j=(ndat->nuclData[i].firstLevel + (uint32_t)(ndat->nuclData[i].numLevels - 1)); j>=ndat->nuclData[i].firstLevel; j--){
					if(j==0){
						break; //safety valve
					}
					for(uint32_t k=ndat->levels[j].firstTran; k<(ndat->levels[j].firstTran + (uint32_t)ndat->levels[j].numTran); k++){
						if((((ndat->tran[k].energy.format >> 5U) & 15U)) != VALUETYPE_X){ //ignore variable energy
							double rawEVal = getRawValFromDB(&ndat->tran[k].energy);
							double rawErrVal = getRawErrFromDB(&ndat->tran[k].energy);
							if(rawEVal > 0.0){
								double errBound = 3.0*rawErrVal;
								if(errBound < 5.0){
									errBound = 5.0;
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
											res.corrRes = 0;
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

void searchHalfLife(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss){
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
			if(ds->useLifetimes){
				hlSearch /= 1.4427; //convert lifetime to half-life
			}
			for(int16_t j=0; j<ndat->numNucl; j++){
				float proximityFactor = sqrtf(fabsf((float)ndat->nuclData[j].Z - ds->chartPosY - (16.0f/ds->chartZoomScale)) + fabsf((float)ndat->nuclData[j].N - ds->chartPosX) + 0.1f);
				proximityFactor = 0.5f*ds->chartZoomScale/proximityFactor;
				if(proximityFactor > 1.0f){
					proximityFactor = 1.0f;
				}
				//SDL_Log("proximityFactor: %f\n",(double)proximityFactor);
				for(uint32_t k=ndat->nuclData[j].firstLevel; k<(ndat->nuclData[j].firstLevel + (uint32_t)ndat->nuclData[j].numLevels); k++){
					uint8_t hlValueType = (uint8_t)((ndat->levels[k].halfLife.format >> 5U) & 15U);
					if((hlValueType == VALUETYPE_NUMBER)||(hlValueType == VALUETYPE_ASYMERROR)){
						double rawHlVal = getRawValFromDB(&ndat->levels[k].halfLife);
						if(rawHlVal > 0.0){
							double rawErrVal = getRawErrFromDB(&ndat->levels[k].halfLife);
							double errBound = 3.0*rawErrVal;
							if(errBound < 5.0){
								errBound = 5.0;
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
								res.corrRes = 0;
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
							res.relevance = 2.0f; //exact match
							res.resultType = SEARCHAGENT_NUCLIDE;
							res.resultVal[0] = (uint32_t)j;
							res.resultVal[1] = 0;
							res.corrRes = 0;
							//SDL_Log("Found nuclide %u\n",res.resultVal[0]);
							sortAndAppendResult(ss,&res);
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
							res.relevance = 1.8f; //uppercase match
							res.resultType = SEARCHAGENT_NUCLIDE;
							res.resultVal[0] = (uint32_t)j;
							res.resultVal[1] = 0;
							res.corrRes = 0;
							//SDL_Log("Found nuclide %u\n",res.resultVal[0]);
							sortAndAppendResult(ss,&res);
						}
					}
				}
			}
		}
	}

	//SDL_Log("Number of search results: %u\n",ss->numUpdatedResults);
}
