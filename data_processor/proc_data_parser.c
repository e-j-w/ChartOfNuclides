/* © J. Williams 2017-2024 */
/* Functions used to process raw app data files */

#include "proc_data_parser.h"

//Maps asset names to their IDs using a predefined map.
//Used for example to identify parseed textures based on their filePath. 
static unsigned int nameToAssetID(const char *name, const asset_mapping *restrict map){
  unsigned int i;
  for(i=0;i<map->numAssets;i++){
    if(SDL_strcmp(name,map->assetID[i])==0){
      return i;
    }
  }
  printf("ERROR: could not find asset with name: %s\n",name);
  exit(-1);
  return 0;
}

//parses app rules
static int parseAppRules(app_data *restrict dat, asset_mapping *restrict stringIDmap, const char *appBasePath){
  FILE *inp;
  char str[256], str2[256], filePath[528];
  char *tok;
  SDL_snprintf(filePath,528,"%sdata/app_rules.txt",appBasePath);
  inp = fopen(filePath, "r");
  if(inp!=NULL){
    while(!(feof(inp))){ //go until the end of file is reached
      if(fgets(str,256,inp)!=NULL){
        str[strcspn(str,"\r\n")] = 0; //strips newline characters from the string read by fgets
        if(SDL_strcmp(str,"")!=0){
          tok = strtok(str," ");
          if(SDL_strcmp(tok,"app_name") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(dat->rules.appName,tok,63);
          }else if(SDL_strcmp(tok,"white_bg_col") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.whiteBGCol.r = (float)atof(tok);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.whiteBGCol.g = (float)atof(tok);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.whiteBGCol.b = (float)atof(tok);
                  dat->rules.themeRules.whiteBGCol.a = 1.0f;
                }else{
                  printf("ERROR: could not white_bg_col color string in file: %s.\n",filePath);
                  exit(-1);
                }
              }else{
                printf("ERROR: could not white_bg_col color string in file: %s.\n",filePath);
                exit(-1);
              }
            }else{
              printf("ERROR: could not white_bg_col color string in file: %s.\n",filePath);
              exit(-1);
            }
          }else if(SDL_strcmp(tok,"text_col") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.textColNormal.r = (Uint8)floor(atof(tok)*255.0);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.textColNormal.g = (Uint8)floor(atof(tok)*255.0);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.textColNormal.b = (Uint8)floor(atof(tok)*255.0);
                  dat->rules.themeRules.textColNormal.a = 255;
                }else{
                  printf("ERROR: could not text_col color string in file: %s.\n",filePath);
                  exit(-1);
                }
              }else{
                printf("ERROR: could not text_col color string in file: %s.\n",filePath);
                exit(-1);
              }
            }else{
              printf("ERROR: could not text_col color string in file: %s.\n",filePath);
              exit(-1);
            }
          }else if(SDL_strcmp(tok,"mod_col_normal") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.modNormalCol.r = (float)atof(tok);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.modNormalCol.g = (float)atof(tok);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.modNormalCol.b = (float)atof(tok);
                  dat->rules.themeRules.modNormalCol.a = 1.0f;
                }else{
                  printf("ERROR: could not mod_col_normal color string in file: %s.\n",filePath);
                  exit(-1);
                }
              }else{
                printf("ERROR: could not mod_col_normal color string in file: %s.\n",filePath);
                exit(-1);
              }
            }else{
              printf("ERROR: could not mod_col_normal color string in file: %s.\n",filePath);
              exit(-1);
            }
          }else if(SDL_strcmp(tok,"mod_col_mouseover") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.modMouseOverCol.r = (float)atof(tok);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.modMouseOverCol.g = (float)atof(tok);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.modMouseOverCol.b = (float)atof(tok);
                  dat->rules.themeRules.modMouseOverCol.a = 1.0f;
                }else{
                  printf("ERROR: could not mod_col_mouseover color string in file: %s.\n",filePath);
                  exit(-1);
                }
              }else{
                printf("ERROR: could not mod_col_mouseover color string in file: %s.\n",filePath);
                exit(-1);
              }
            }else{
              printf("ERROR: could not mod_col_mouseover color string in file: %s.\n",filePath);
              exit(-1);
            }
          }else if(SDL_strcmp(tok,"mod_col_selected") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.modSelectedCol.r = (float)atof(tok);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.modSelectedCol.g = (float)atof(tok);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.modSelectedCol.b = (float)atof(tok);
                  dat->rules.themeRules.modSelectedCol.a = 1.0f;
                }else{
                  printf("ERROR: could not mod_col_selected color string in file: %s.\n",filePath);
                  exit(-1);
                }
              }else{
                printf("ERROR: could not mod_col_selected color string in file: %s.\n",filePath);
                exit(-1);
              }
            }else{
              printf("ERROR: could not mod_col_selected color string in file: %s.\n",filePath);
              exit(-1);
            }
          }
        }
      }
    }
  }else{
    printf("ERROR: cannot open file %s\n",filePath);
    return -1;
  }
  fclose(inp);

  //setup string ids for mandatory strings
  dat->locStringIDs[LOCSTR_APPLY] = (uint16_t)nameToAssetID("apply",stringIDmap);
  dat->locStringIDs[LOCSTR_CANCEL] = (uint16_t)nameToAssetID("cancel",stringIDmap);
  dat->locStringIDs[LOCSTR_OK] = (uint16_t)nameToAssetID("ok",stringIDmap);
  dat->locStringIDs[LOCSTR_OPENFILE] = (uint16_t)nameToAssetID("open_file",stringIDmap);
  dat->locStringIDs[LOCSTR_FILE_FILTER_NAME] = (uint16_t)nameToAssetID("file_filter_name",stringIDmap);
  dat->locStringIDs[LOCSTR_FILE_FILTER_PATTERN] = (uint16_t)nameToAssetID("file_filter_pattern",stringIDmap);
  dat->locStringIDs[LOCSTR_NODB] = (uint16_t)nameToAssetID("no_db",stringIDmap);

  return 0; //success

}


//For strings, we supply a map of all the files containing string data
//However, the strings themselves inside these files are mapped in this function
static int parseStrings(app_data *restrict dat, asset_mapping *restrict stringIDmap, const char *appBasePath){
  FILE *inp;
  char filePath[536], str[256];
  char *tok;
  
  SDL_snprintf(filePath,536,"%sdata/strings.txt",appBasePath); //open the string file
  inp = fopen(filePath, "r");
  if(inp!=NULL){
    //printf("Opened file: %s\n",filePath);
    while(!(feof(inp))){ //go until the end of file is reached
      if(fgets(str,256,inp)!=NULL){
        str[strcspn(str,"\r\n")] = 0; //strips newline characters from the string read by fgets
        if(SDL_strcmp(str,"")!=0){
          tok = strtok(str,"|");
          if(tok!=NULL){
            if(dat->numStrings >= MAX_NUM_STRINGS){
              printf("ERROR: maximum number of text strings reached, cannot parse file %s\n",filePath);
              return -1;
            }
            strncpy(stringIDmap->assetID[dat->numStrings],str,256);
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(dat->strings[dat->numStrings],tok,255);
            dat->numStrings++;
          }else{
            printf("ERROR: improperly formatted string in file %s\n",filePath);
            return -1;
          }
        }
      }
    }
  }else{
    printf("ERROR: cannot open file %s\n",filePath);
    return -1;
  }
  fclose(inp);
  stringIDmap->numAssets = dat->numStrings;

  return 0; //success
}

//returns true if two numbers are equal to within some fudge factor
int fudgeNumbers(float num1, float num2, float fudgeFactor){
  //printf("val: %f\n",fabs(num1-num2));
  if(fabsf(num1-num2)<=fudgeFactor)
    return 1;
  else
    return 0;
}

//if the specified gamma cascade contains the specified level energy,
//returns the step of the cascade containing that energy
int levelInCascade(gamma_cascade * c, float energy){
	int i;
	
	for(i=0;i<c->numLevels;i++)
		if(i<MAXCASCDELENGTH)
			if(energy==c->energies[i])
				return i;
				
	return -1;
}

//returns true if the specified gamma cascade contains the specified level energy as its highest level
int lastLevelInCascade(gamma_cascade * c, float energy){
	if(c->numLevels>0)
		if(energy==c->energies[(int)(c->numLevels)-1])
			return 1;

	return 0;
}

void generateCascadeData(ndata *nd){

	int i,j,k,l,m,n;// loop indices (!)
	int append=0;
	int ind;
	
  //generate gamma cascade lists
	if(nd->numNucl>=0) //check that indices are valid
	  for(i=0;i<nd->numNucl;i++){
			nd->nuclData[i].numCascades=0;//initialize properly
			for(j=0;j<nd->nuclData[i].numLevels;j++){
				for(k=0;k<nd->levels[(int)(nd->nuclData[i].firstLevel) + j].numTran;k++){
					//check whether the level decays to another level
					float trialLevelE = nd->levels[(int)(nd->nuclData[i].firstLevel) + j].energy - nd->tran[(int)(nd->levels[(int)(nd->nuclData[i].firstLevel) + j].firstTran) + k].energy;
					for(l=0;l<j;l++)
						if(fudgeNumbers(trialLevelE, nd->levels[(int)(nd->nuclData[i].firstLevel) + l].energy, 2.)==1){
								
							//printf("-->%s: Cascade detected between levels at energies %f and %f keV.\n",nd->nuclData[i].nuclName,nd->levels[nd->nuclData[i].firstLevel + j].energy,nd->levels[nd->nuclData[i].firstLevel + l].energy);
							//printf("Number of cascades: %i\n",nd->nuclData[i].numCascades);
								
							append=0;
							//see if we can add the level to existing cascade(s)
							for(m=0;m<nd->nuclData[i].numCascades;m++)
								if(m<MAXCASCDESPERNUCL)
									if(nd->nuclData[i].cascades[m].numLevels<MAXGAMMASPERLEVEL)
										if(lastLevelInCascade(&nd->nuclData[i].cascades[m], nd->levels[(int)(nd->nuclData[i].firstLevel) + l].energy))
											if(nd->nuclData[i].cascades[m].numLevels+1<=MAXCASCDELENGTH){
												//printf("Adding to cascade %i.\n",m+1);
												//add level to existing cascade
												nd->nuclData[i].cascades[m].energies[nd->nuclData[i].cascades[m].numLevels] = nd->levels[(int)(nd->nuclData[i].firstLevel) + j].energy;
												nd->nuclData[i].cascades[m].numLevels++;
												append=1;
											}
							if(append==0){
								//see if we can make a new cascade by copying part of an older one
								if((nd->nuclData[i].numCascades+1)<=MAXCASCDESPERNUCL)//verify that there is room for a new cascade
									for(m=0;m<nd->nuclData[i].numCascades;m++)
										{
											ind=levelInCascade(&nd->nuclData[i].cascades[m], nd->levels[(int)(nd->nuclData[i].firstLevel) + l].energy);
											if(ind>=0)
												{
													//printf("Copying existing data from cascade %i.  ind=%i\n",m+1,ind);
													append=1;
													if((ind+2)<MAXCASCDELENGTH)//verify that there is room for a new level
														{
															//copy the cascade
															nd->nuclData[i].cascades[nd->nuclData[i].numCascades].numLevels=(int16_t)(ind+2);
															for(n=0;n<=ind;n++)
																nd->nuclData[i].cascades[nd->nuclData[i].numCascades].energies[n]=nd->nuclData[i].cascades[m].energies[n];
															//add the new level
															nd->nuclData[i].cascades[nd->nuclData[i].numCascades].energies[ind+1]=nd->levels[(int)(nd->nuclData[i].firstLevel) + j].energy;
															//increment cascade counter
															nd->nuclData[i].numCascades++;
															break;
														}
												}
										}
							}
							if(append==0){
								//make a brand new cascade
								if((nd->nuclData[i].numCascades+1)<=MAXCASCDESPERNUCL)//verify that there is room for a new cascade
									{
										//printf("Creating new cascade.\n");
										nd->nuclData[i].cascades[nd->nuclData[i].numCascades].numLevels=2;
										nd->nuclData[i].cascades[nd->nuclData[i].numCascades].energies[0]=nd->levels[(int)(nd->nuclData[i].firstLevel) + l].energy;
										nd->nuclData[i].cascades[nd->nuclData[i].numCascades].energies[1]=nd->levels[(int)(nd->nuclData[i].firstLevel) + j].energy;
										nd->nuclData[i].numCascades++;
									}
							}
								
						}
				}
			}  
			//dump cascade data
			/*if(nd->nuclData[i].numCascades>0)
				{
					printf("Cascades generated for nucleus: %s\n",nd->nuclData[i].nuclName);
					for(m=0;m<nd->nuclData[i].numCascades;m++)
						{
							printf("CASCADE %i:\nStep   Level Energy (keV)   Gamma energy (keV)\n",m+1);
							for(n=0;n<nd->nuclData[i].cascades[m].numLevels;n++)
								printf("%i      %f\n",n+1,nd->nuclData[i].cascades[m].energies[n]);
						}
					//if(strcmp(nd->nuclData[i].nuclName,"68SE")==0)
						//getc(stdin);
				}*/
		}
	
	
	//generate cascade gamma ray energies
	if(nd->numNucl>=0) //check that indices are valid
	  for(i=0;i<nd->numNucl;i++)
	  	for(j=0;j<nd->nuclData[i].numCascades;j++)
	  		for(k=0;k<nd->nuclData[i].cascades[j].numLevels;k++){
					if(k==0)
						nd->nuclData[i].cascades[j].gammaEnergies[k]=0.;
					else
						nd->nuclData[i].cascades[j].gammaEnergies[k]=nd->nuclData[i].cascades[j].energies[k] - nd->nuclData[i].cascades[j].energies[k-1];
				}
	
	
}


//parse half-life values for a given level
void parseHalfLife(level * lev, char * ltstring){

	char *tok;
	char str[256], val[MAXNUMPARSERVALS][256];
	int numTok=0;

	lev->halfLife = -1.0f;
	lev->halfLifeErr = -1.0f;
  lev->halfLifeUnit=HALFLIFE_UNIT_NOVAL;

	strcpy(str,ltstring);
	tok = strtok (str, " ?");
	if(tok == NULL){
		return;
	}
	strcpy(val[numTok],tok);
	while(tok != NULL){
		tok = strtok(NULL, " ?");
		if(tok!=NULL){
      numTok++;
      if(numTok<MAXNUMPARSERVALS){
        strcpy(val[numTok],tok);
      }else{
        numTok--;
        break;
      }
    }
	}
	numTok++;

	if(strcmp(val[1],"STABLE")==0){
		lev->halfLife = 1.0E20f;
		lev->halfLifeUnit = HALFLIFE_UNIT_STABLE;
	}else{
		lev->halfLife = (float)atof(val[0]);
		if(lev->halfLife<=0.0f){
			lev->halfLifeUnit = HALFLIFE_UNIT_STABLE;
		}else if(strcmp(val[1],"Y")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_YEARS;
		}else if(strcmp(val[1],"D")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_DAYS;
		}else if(strcmp(val[1],"H")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_HOURS;
		}else if(strcmp(val[1],"M")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_MINUTES;
		}else if(strcmp(val[1],"S")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_SECONDS;
		}else if(strcmp(val[1],"MS")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_MILLISECONDS;
		}else if(strcmp(val[1],"US")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_MICROSECONDS;
		}else if(strcmp(val[1],"NS")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_NANOSECONDS;
		}else if(strcmp(val[1],"PS")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_PICOSECONDS;
		}else if(strcmp(val[1],"FS")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_FEMTOSECONDS;
		}else if(strcmp(val[1],"AS")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_ATTOSECONDS;
		}else if(strcmp(val[1],"EV")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_EV;
		}else if(strcmp(val[1],"KEV")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_KEV;
		}else if(strcmp(val[1],"MEV")==0){
			lev->halfLifeUnit = HALFLIFE_UNIT_MEV;
		}else{
			printf("Unknown half-life unit: %s (full string: %s)\n",val[1],ltstring);
		}
	}

}

//parse spin parity values for a given level
void parseSpinPar(level * lev, char * spstring){

	char *tok;
	char str[256], tmpstr[256], tmpstr2[256], val[MAXNUMPARSERVALS][256];
	int i,j;
	int numTok=0;

	lev->numSpinParVals=0;
	//printf("--------\nstring: %s\n",spstring);

	//check for invalid strings
	strcpy(str,spstring);
	tok = strtok (str, " ");
	if(tok == NULL){
		//printf("energy %f, strings: %s,%s\n",lev->energy,spstring,tok);
		//printf("Not a valid spin-parity value.\n");
		//getc(stdin);
		return;
	}
	/*strcpy(str,spstring);
	tok = strtok (str, ".");
	if((tok == NULL)||(strcmp(tok,spstring)!=0)){
		//printf("%s\n",tok);
		//printf("Not a valid spin-parity value.\n");
		//getc(stdin);
		return;
	}*/

	strcpy(str,spstring);
	tok = strtok (str, " ,");
	strcpy(val[numTok],tok);
	while (tok != NULL)
	{
		tok = strtok (NULL, " ,");
		if(tok!=NULL)
			{
				numTok++;
				if(numTok<MAXNUMPARSERVALS){
					strcpy(val[numTok],tok);
				}else{
					numTok--;
					break;
				}
					
			}
	}
	numTok++;

	/*printf("string: %s, number of tokens: %i\n",spstring,numTok);
	for(i=0;i<numTok;i++){
		printf("| %s ",val[i]);
	}
	printf("\n");*/

	uint8_t tentative=0;

	if(numTok<=0){
		return;
	}else if(strcmp(val[0],"GE")==0){
		lev->spval[lev->numSpinParVals].tentative = 3;
		lev->spval[lev->numSpinParVals].spinVal = (int16_t)atoi(val[1]);
		lev->numSpinParVals=1;
		return;
	}else if((strcmp(val[0],"+")==0)&&(numTok==1)){
		lev->spval[lev->numSpinParVals].parVal = 1;
		lev->spval[lev->numSpinParVals].spinVal = -1;
		lev->numSpinParVals=1;
		return;
	}else if((strcmp(val[0],"-")==0)&&(numTok==1)){
		lev->spval[lev->numSpinParVals].parVal = -1;
		lev->spval[lev->numSpinParVals].spinVal = -1;
		lev->numSpinParVals=1;
		return;
	}else{
		for(i=0;i<numTok;i++){
			if(i<MAXSPPERLEVEL){

				//check for brackets
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"()");
				if(tok!=NULL){
					if(strcmp(tok,val[i])!=0){
						//printf("setting tentative marker...\n");
						//tentative marker
						if(tentative == 0)
							tentative = 1;
						else if(tentative == 1)
							tentative = 0;
					}
				}

				//check for parity
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"+-");
				if(tok!=NULL){
					if(strcmp(tok,val[i])!=0){
						//printf("setting parity marker...\n");
						//contains parity info
						tok=strtok(val[i],"/(0123456789, ");
						if((strcmp(tok,"+")==0)||(strcmp(tok,"+)")==0)){
							lev->spval[lev->numSpinParVals].parVal = 1;
						}else if((strcmp(tok,"-")==0)||(strcmp(tok,"-)")==0)){
							lev->spval[lev->numSpinParVals].parVal = -1;
						}else if(strcmp(tok,")-")==0){
							//all spin values negative parity
							for(j=0;j<=lev->numSpinParVals;j++){
								lev->spval[j].parVal = -1;
								tentative = 2;
							}
						}else if(strcmp(tok,")+")==0){
							//all spin values positive parity
							for(j=0;j<=lev->numSpinParVals;j++){
								lev->spval[j].parVal = 1;
								tentative = 2;
							}
						}
					}
				}

				//extract spin
				lev->spval[lev->numSpinParVals].spinVal = -1; //default to unknown spin
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"()+-, ");
				if(tok!=NULL){
					strcpy(tmpstr2,tok);
					tok=strtok(tok,"/");
					if(strcmp(tok,tmpstr2)!=0){
						//printf("Detected half-integer spin.\n");
						lev->spval[lev->numSpinParVals].halfInt = 1;
						lev->spval[lev->numSpinParVals].spinVal=(int16_t)atoi(tok);
					}else{
						lev->spval[lev->numSpinParVals].spinVal=(int16_t)atoi(tmpstr2);
					}
				}

				lev->spval[lev->numSpinParVals].tentative = tentative;
				lev->numSpinParVals++;

				//printf("%f keV Entry %i: spin %i (half-int %i), parity %i, tentative %i\n",lev->energy,lev->numSpinParVals,lev->spval[lev->numSpinParVals-1].spinVal,lev->spval[lev->numSpinParVals-1].halfInt,lev->spval[lev->numSpinParVals-1].parVal,lev->spval[lev->numSpinParVals-1].tentative);
			}
		}
		//printf("number of spin vals: %i\n",lev->numSpinParVals);
		
	}

	//getc(stdin);

}


void getNuclNZ(nucl *nuc, const char *nucName){

	char str[256];
	int16_t A,Z,N;
	char *tok;
	
	//get mass number
	strcpy(str,nucName); //copy the nucleus name
	tok=strtok(str,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	A=(int16_t)atoi(tok);
	
	//get proton number
	strcpy(str,nucName); //copy the nucleus name
	tok=strtok(str,"0123456789");
	if(tok!=NULL){
		if(strcmp(tok,"H")==0)
			Z=1;
		else if(strcmp(tok,"HE")==0)
			Z=2;
		else if(strcmp(tok,"LI")==0)
			Z=3;
		else if(strcmp(tok,"BE")==0)
			Z=4;
		else if(strcmp(tok,"B")==0)
			Z=5;
		else if(strcmp(tok,"C")==0)
			Z=6;
		else if(strcmp(tok,"N")==0)
			Z=7;
		else if(strcmp(tok,"O")==0)
			Z=8;
		else if(strcmp(tok,"F")==0)
			Z=9;
		else if(strcmp(tok,"NE")==0)
			Z=10;
		else if(strcmp(tok,"NA")==0)
			Z=11;
		else if(strcmp(tok,"MG")==0)
			Z=12;
		else if(strcmp(tok,"AL")==0)
			Z=13;
		else if(strcmp(tok,"SI")==0)
			Z=14;
		else if(strcmp(tok,"P")==0)
			Z=15;
		else if(strcmp(tok,"S")==0)
			Z=16;
		else if(strcmp(tok,"CL")==0)
			Z=17;
		else if(strcmp(tok,"AR")==0)
			Z=18;
		else if(strcmp(tok,"K")==0)
			Z=19;
		else if(strcmp(tok,"CA")==0)
			Z=20;
		else if(strcmp(tok,"SC")==0)
			Z=21;
		else if(strcmp(tok,"TI")==0)
			Z=22;
		else if(strcmp(tok,"V")==0)
			Z=23;
		else if(strcmp(tok,"CR")==0)
			Z=24;
		else if(strcmp(tok,"MN")==0)
			Z=25;
		else if(strcmp(tok,"FE")==0)
			Z=26;
		else if(strcmp(tok,"CO")==0)
			Z=27;
		else if(strcmp(tok,"NI")==0)
			Z=28;
		else if(strcmp(tok,"CU")==0)
			Z=29;
		else if(strcmp(tok,"ZN")==0)
			Z=30;
		else if(strcmp(tok,"GA")==0)
			Z=31;
		else if(strcmp(tok,"GE")==0)
			Z=32;
		else if(strcmp(tok,"AS")==0)
			Z=33;
		else if(strcmp(tok,"SE")==0)
			Z=34;
		else if(strcmp(tok,"BR")==0)
			Z=35;
		else if(strcmp(tok,"KR")==0)
			Z=36;
		else if(strcmp(tok,"RB")==0)
			Z=37;
		else if(strcmp(tok,"SR")==0)
			Z=38;
		else if(strcmp(tok,"Y")==0)
			Z=39;
		else if(strcmp(tok,"ZR")==0)
			Z=40;
		else if(strcmp(tok,"NB")==0)
			Z=41;
		else if(strcmp(tok,"MO")==0)
			Z=42;
		else if(strcmp(tok,"TC")==0)
			Z=43;
		else if(strcmp(tok,"RU")==0)
			Z=44;
		else if(strcmp(tok,"RH")==0)
			Z=45;
		else if(strcmp(tok,"PD")==0)
			Z=46;
		else if(strcmp(tok,"AG")==0)
			Z=47;
		else if(strcmp(tok,"CD")==0)
			Z=48;
		else if(strcmp(tok,"IN")==0)
			Z=49;
		else if(strcmp(tok,"SN")==0)
			Z=50;
		else if(strcmp(tok,"SB")==0)
			Z=51;
		else if(strcmp(tok,"TE")==0)
			Z=52;
		else if(strcmp(tok,"I")==0)
			Z=53;
		else if(strcmp(tok,"XE")==0)
			Z=54;
		else if(strcmp(tok,"CS")==0)
			Z=55;
		else if(strcmp(tok,"BA")==0)
			Z=56;
		else if(strcmp(tok,"LA")==0)
			Z=57;
		else if(strcmp(tok,"CE")==0)
			Z=58;
		else if(strcmp(tok,"PR")==0)
			Z=59;
		else if(strcmp(tok,"ND")==0)
			Z=60;
		else if(strcmp(tok,"PM")==0)
			Z=61;
		else if(strcmp(tok,"SM")==0)
			Z=62;
		else if(strcmp(tok,"EU")==0)
			Z=63;
		else if(strcmp(tok,"GD")==0)
			Z=64;
		else if(strcmp(tok,"TB")==0)
			Z=65;
		else if(strcmp(tok,"DY")==0)
			Z=66;
		else if(strcmp(tok,"HO")==0)
			Z=67;
		else if(strcmp(tok,"ER")==0)
			Z=68;
		else if(strcmp(tok,"TM")==0)
			Z=69;
		else if(strcmp(tok,"YB")==0)
			Z=70;
		else if(strcmp(tok,"LU")==0)
			Z=71;
		else if(strcmp(tok,"HF")==0)
			Z=72;
		else if(strcmp(tok,"TA")==0)
			Z=73;
		else if(strcmp(tok,"W")==0)
			Z=74;
		else if(strcmp(tok,"RE")==0)
			Z=75;
		else if(strcmp(tok,"OS")==0)
			Z=76;
		else if(strcmp(tok,"IR")==0)
			Z=77;
		else if(strcmp(tok,"PT")==0)
			Z=78;
		else if(strcmp(tok,"AU")==0)
			Z=79;
		else if(strcmp(tok,"HG")==0)
			Z=80;
		else if(strcmp(tok,"TL")==0)
			Z=81;
		else if(strcmp(tok,"PB")==0)
			Z=82;
		else if(strcmp(tok,"BI")==0)
			Z=83;
		else if(strcmp(tok,"PO")==0)
			Z=84;
		else if(strcmp(tok,"AT")==0)
			Z=85;
		else if(strcmp(tok,"RN")==0)
			Z=86;
		else if(strcmp(tok,"FR")==0)
			Z=87;
		else if(strcmp(tok,"RA")==0)
			Z=88;
		else if(strcmp(tok,"AC")==0)
			Z=89;
		else if(strcmp(tok,"TH")==0)
			Z=90;
		else if(strcmp(tok,"PA")==0)
			Z=91;
		else if(strcmp(tok,"U")==0)
			Z=92;
		else if(strcmp(tok,"NP")==0)
			Z=93;
		else if(strcmp(tok,"PU")==0)
			Z=94;
		else if(strcmp(tok,"AM")==0)
			Z=95;
		else if(strcmp(tok,"CM")==0)
			Z=96;
		else if(strcmp(tok,"BK")==0)
			Z=97;
		else if(strcmp(tok,"CF")==0)
			Z=98;
		else if(strcmp(tok,"ES")==0)
			Z=99;
		else if(strcmp(tok,"FM")==0)
			Z=100;
		else if(strcmp(tok,"MD")==0)
			Z=101;
		else if(strcmp(tok,"NO")==0)
			Z=102;
		else if(strcmp(tok,"LR")==0)
			Z=103;
		else if(strcmp(tok,"RF")==0)
			Z=104;
		else if(strcmp(tok,"DB")==0)
			Z=105;
		else if(strcmp(tok,"SG")==0)
			Z=106;
		else if(strcmp(tok,"BH")==0)
			Z=107;
		else if(strcmp(tok,"HS")==0)
			Z=108;
		else if(strcmp(tok,"MT")==0)
			Z=109;
		else if(strcmp(tok,"DS")==0)
			Z=110;
		else if(strcmp(tok,"RG")==0)
			Z=111;
    else if(strcmp(tok,"CN")==0)
			Z=112;
    else if(strcmp(tok,"NH")==0)
			Z=113;
    else if(strcmp(tok,"FL")==0)
			Z=114;
    else if(strcmp(tok,"MC")==0)
			Z=115;
    else if(strcmp(tok,"LV")==0)
			Z=116;
    else if(strcmp(tok,"TS")==0)
			Z=117;
    else if(strcmp(tok,"OG")==0)
			Z=118;
    else if(strcmp(tok,"NN")==0)
      Z=0;
		else
			Z=-1;

		//get neutron number
    N=A-Z;
	}else{
		Z=-1;
		N=-1;
	}
	
	nuc->N=N;
	nuc->Z=Z;
	
}

//set initial databae values prior to importing data
void initialize_database(ndata * nd){
	
	memset(nd,0,sizeof(ndata));
	
	nd->numNucl = -1;
	nd->numLvls = 0;
	nd->numTran = 0;
	for(uint32_t i=0;i<MAXNUMNUCL;i++){
		nd->nuclData[i].numLevels = 0;
	}
	for(uint32_t i=0;i<MAXNUMLVLS;i++){
		nd->levels[i].numDecModes = -1; //default if no decay mode info in data
	}
	
}

//checks whether a string is all whitespace and returns 1 if true
int isEmpty(const char *str){
  while(*str != '\0'){
    if(!isspace((unsigned char)*str)){
			return 0;
		}
    str++;
  }
  return 1;
}

//function to parse ENSDF data files
int readENSDFFile(const char * filePath, ndata * nd){

  FILE *config;
  char *tok, *tok2;
  char str[256];//string to be read from file (will be tokenized)
  char nuclNameStr[10];
  char line[256],val[MAXNUMPARSERVALS][256];
  int tokPos;//position when tokenizing
  int firstQLine = 1; //flag to specify whether Q values have been read in for a specific nucleus
  
  //subsection of the entry for a particular nucleus that the parser is at
  //each nucleus has multiple entries, including adopted gammas, and gammas 
  //associated with a particlular reaction mechanism
  int subSec=0;
  
  //open the file and read all parameters
  if((config=fopen(filePath,"r"))==NULL){
    //file doesn't exist, and will be omitted from the database
		//printf("WARNING: Cannot open the ENSDF file %s\n",filePath);
		return 0;
	}
  while(!(feof(config))){ //go until the end of file is reached

		if(fgets(str,256,config)!=NULL){ //get an entire line

			strcpy(line,str); //store the entire line
			//printf("%s\n",line);
			if(isEmpty(str)){
				subSec++; //empty line, increment which subsection we're on
				firstQLine = 1;
			}else{
				tok=strtok(str," ");
				tokPos=0;
				strcpy(val[tokPos],tok);
				while (tok != NULL){
					tok = strtok(NULL, " ");
					if(tok!=NULL){
						tokPos++;
						if(tokPos<MAXNUMPARSERVALS)
							strcpy(val[tokPos],tok);
						else
							break;
					}
				}
			}
			
			//increment the nucleus if a new nucleus is found
			char hbuff[15];
			memcpy(hbuff, &line[9], 14);
			hbuff[14] = '\0';
			if(strcmp(hbuff,"ADOPTED LEVELS")==0){
				if(nd->numNucl<MAXNUMNUCL){
					nd->numNucl++;
					subSec=0; //we're at the beginning of the entry for this nucleus 
					//printf("Adding gamma data for nucleus %s\n",val[0]);
					memcpy(nuclNameStr,val[0],10);
					nuclNameStr[9] = '\0'; //terminate string
					getNuclNZ(&nd->nuclData[nd->numNucl],nuclNameStr); //get N and Z
				}
			}
			/*//parse the nucleus name
			char nbuff[7];
			memcpy(nbuff, &line[0], 6);
			nbuff[6] = '\0';*/

			//parse the line type
			char typebuff[3];
			memcpy(typebuff, &line[6], 2);
			typebuff[2] = '\0';

			//parse the energy
			char ebuff[10];
			memcpy(ebuff, &line[9], 9);
			ebuff[9] = '\0';

			//add levels
			if(nd->numNucl>=0){ //check that indices are valid
				if(strncmp(val[0],nuclNameStr,10)==0){
					if(subSec==0){ //adopted levels subsection
						if(nd->numLvls<MAXNUMLVLS){
							if(strcmp(typebuff," L")==0){

								float levelE = (float)atof(ebuff);
                //printf("%s\n",line);
								if(line[9]=='%'){
									//line contains decay mode info
									uint8_t decModCtr = 0;
									nd->levels[nd->numLvls].numDecModes = 0;
									nd->levels[nd->numLvls].firstDecMode = nd->numDecModes;
									char dmBuff[128], valBuff[16];
									memcpy(dmBuff, &line[9], 127);
									dmBuff[127] = '\0';
									tok = strtok(dmBuff," =");
									while(tok!=NULL){
										//printf("tok at start: %s\n",tok);
										if(strcmp(tok,"%IT")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_IT;
										}else if(strcmp(tok,"%B-")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_BETAMINUS;
										}else if(strcmp(tok,"%B+")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_BETAPLUS;
										}else if(strcmp(tok,"%EC")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_EC;
										}else if(strcmp(tok,"%EC+%B+")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_ECANDBETAPLUS;
										}else if(strcmp(tok,"%B-N")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_BETAMINUS_NEUTRON;
										}else if(strcmp(tok,"%ECP")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_EC_PROTON;
										}else if(strcmp(tok,"%P")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_PROTON;
										}else if(strcmp(tok,"%2P")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_TWOPROTON;
										}else if(strcmp(tok,"%N")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_NEUTRON;
										}else if(strcmp(tok,"%D")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_DEUTERON;
										}else if(strcmp(tok,"%3HE")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_3HE;
										}else if(strcmp(tok,"%A")==0){
											nd->dcyMode[nd->numDecModes].type = DECAYMODE_ALPHA;
										}else{
											break;
										}
										tok = strtok(NULL,"$");
										if(tok!=NULL){
											//printf("%s\n",tok);
											strncpy(valBuff,tok,15);
											tok2 = strtok(valBuff," ");
											if(tok2 != NULL){
												//printf("%s\n",tok2);
												if(strcmp(tok2,"GT")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_GREATERTHAN;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"GT")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_GREATERTHAN;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"GE")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_GREATEROREQUALTHAN;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"LT")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_LESSTHAN;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"LE")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_LESSOREQUALTHAN;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"AP")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_APPROX;
													tok2 = strtok(NULL," ");
												}else if(strcmp(tok2,"?")==0){
													nd->dcyMode[nd->numDecModes].probType = VALUETYPE_UNKNOWN;
													tok2 = strtok(NULL," ");
												}
												if(tok2!=NULL){
													//printf("%s\n",tok2);
													nd->dcyMode[nd->numDecModes].prob = (float)atof(tok2);
													tok2 = strtok(NULL,""); //get the rest of the string
													if(tok2 != NULL){
														//printf("%s\n",tok2);
														nd->dcyMode[nd->numDecModes].probErr = (uint8_t)atoi(tok2);
													}
												}
												
											}
											
											//printf("Found decay with type %u and probability: %f %u (type %u)\n",nd->dcyMode[nd->numDecModes].type,(double)nd->dcyMode[nd->numDecModes].prob,nd->dcyMode[nd->numDecModes].probErr,nd->dcyMode[nd->numDecModes].probType);
											nd->levels[nd->numLvls].numDecModes++;
											nd->numDecModes++;
											decModCtr++;
											//go to the next decay mode
											memcpy(dmBuff, &line[9], 127);
											dmBuff[127] = '\0';
											tok = strtok(dmBuff,"$");
											for(uint8_t i=0;i<(decModCtr-1);i++){
												if(tok!=NULL){
													//printf("tok %u: %s\n",i,tok);
													tok = strtok(NULL,"$");
												}
											}
											if(tok!=NULL){
												//printf("tok: %s\n",tok);
												tok = strtok(NULL," =");
											}
										}
									}
									//getc(stdin);
								}
								

                //printf("Found level at %f keV.\n",atof(ebuff));
								//parse the energy error
								char eeBuff[3];
								memcpy(eeBuff, &line[19], 2);
								eeBuff[2] = '\0';
								uint8_t levelEerr = (uint8_t)atoi(eeBuff);
								if((nd->nuclData[nd->numNucl].numLevels==0)||(levelE>(nd->levels[nd->numLvls-1].energy))){
									//the level energy represents a new level
									if(nd->nuclData[nd->numNucl].numLevels == 0){
										nd->nuclData[nd->numNucl].firstLevel = nd->numLvls;
									}
									nd->levels[nd->numLvls].energy=levelE;
									nd->levels[nd->numLvls].energyErr=levelEerr;
									//parse the level spin and parity
									char spbuff[16];
									memcpy(spbuff, &line[21], 15);
									spbuff[15] = '\0';
                  //printf("%s\n",spbuff);
									parseSpinPar(&nd->levels[nd->numLvls],spbuff);
									//parse the half-life imformation
									char hlBuff[11];
									memcpy(hlBuff, &line[39], 10);
									hlBuff[10] = '\0';
                  //printf("%s\n",hlBuff);
									parseHalfLife(&nd->levels[nd->numLvls],hlBuff);
									nd->nuclData[nd->numNucl].numLevels++;
									nd->numLvls++;
								}
							}
						}
					}
				}
			}
			//add gamma rays
			if(nd->numNucl>=0){ //check that indices are valid
				if(nd->nuclData[nd->numNucl].numLevels>0){ //check that indices are valid
					if(strcmp(val[0],nuclNameStr)==0){
						if(subSec==0){ //adopted levels subsection
							if(nd->levels[nd->numLvls-1].numTran<MAXGAMMASPERLEVEL){
								if(strcmp(typebuff," G")==0){
									//parse the gamma intensity
									char iBuff[8];
									memcpy(iBuff, &line[21], 7);
									iBuff[7] = '\0';
									if(nd->levels[nd->numLvls-1].numTran == 0){
										nd->levels[nd->numLvls-1].firstTran = nd->numTran;
									}
									nd->tran[(int)(nd->levels[nd->numLvls-1].firstTran) + nd->levels[nd->numLvls-1].numTran].energy=(float)atof(ebuff);
									//printf("-> Found gamma ray with energy %f keV.\n",atof(ebuff));
									nd->tran[(int)(nd->levels[nd->numLvls-1].firstTran) + nd->levels[nd->numLvls-1].numTran].intensity=(float)atof(iBuff);
									nd->levels[nd->numLvls-1].numTran++;
									nd->numTran++;
										
								}
							}
						}
					}
				}
			}
			//add Q-values and separation energies
			if(nd->numNucl>=0){ //check that indices are valid
				if(strcmp(val[0],nuclNameStr)==0)
					if(subSec==0)//adopted levels subsection
						if(strcmp(typebuff," Q")==0){
							if(firstQLine==1){
								//parse the beta Q-value
								char qbBuff[11];
								memcpy(qbBuff, &line[9], 10);
								qbBuff[10] = '\0';
								
								nd->nuclData[nd->numNucl].qbeta = (float)atof(qbBuff);

								//parse the neutron sep energy
								char nsBuff[9];
								memcpy(nsBuff, &line[21], 8);
								nsBuff[8] = '\0';

								nd->nuclData[nd->numNucl].sn = (float)atof(nsBuff);

								//parse the proton sep energy
								char psBuff[9];
								memcpy(psBuff, &line[31], 8);
								psBuff[8] = '\0';

								nd->nuclData[nd->numNucl].sp = (float)atof(psBuff);

								//parse the alpha Q-value
								char qaBuff[9];
								memcpy(qaBuff, &line[41], 8);
								qaBuff[8] = '\0';
								
								nd->nuclData[nd->numNucl].qalpha = (float)atof(qaBuff);
							}
							firstQLine = 0;
						}
			}

		}
	}
	fclose(config);
	
	if(nd->numNucl>=MAXNUMNUCL){
		printf("ERROR: Attempted to import data for too many nuclei.  Increase the value of MAXNUMNUCL in levelup.h\n");
		return -1;
	}
	
	printf("Finished reading ENSDF file: %s\n",filePath);
  return 0;
}

int buildENSDFDatabase(const char *appBasePath, ndata *nd){

	int i;
	char filePath[256],str[8];
	
	initialize_database(nd);
	
	for(i=1;i<350;i++){
		strcpy(filePath,"");
		strcat(filePath,appBasePath);
    strcat(filePath,"data/ensdf/");
		if(i<10)
			strcat(filePath,"ensdf.00");
		else if(i<100)
			strcat(filePath,"ensdf.0");
		else
			strcat(filePath,"ensdf.");
		sprintf(str,"%i",i);
		strcat(filePath,str);
		if(readENSDFFile(filePath,nd) == -1) return -1; //grab data from the ENSDF file (see parse_ENSDF.c)
	}
	printf("Data imported for %i nuclei, containing %u levels, %u transitions, and %u decay branches.\n",nd->numNucl,nd->numLvls,nd->numTran,nd->numDecModes);
	printf("Generating cascade data.\n");
	generateCascadeData(nd);
	
	//write the database to disk
	if(nd->numNucl<=0)
		{
			printf("ERROR: no valid ENSDF data was found.\nPlease check that ENSDF files exist in the directory under the ENDSF environment variable.\n");
			return -1;
		}
	printf("ENSDF database build finished.\n");
	return 0;
}

//parse all app data
int parseAppData(app_data *restrict dat, const char *appBasePath){

  asset_mapping *restrict stringIDmap=(asset_mapping*)SDL_calloc(1,sizeof(asset_mapping)); //allocated on heap to not overflow the stack

  //set default values
  dat->numStrings=0;
  memset(dat->locStringIDs,0,sizeof(dat->locStringIDs));

  //start parsing data
  if(parseStrings(dat,stringIDmap,appBasePath)==-1) return -1;
  if(parseAppRules(dat,stringIDmap,appBasePath)==-1) return -1;

  if(buildENSDFDatabase(appBasePath,&dat->ndat)==-1) return -1;

  //summarize
  printf("Data parsing complete.\n");
  printf("  Number of text strings parsed:               %4i (%4i max)\n",dat->numStrings,MAX_NUM_STRINGS);

  return 0; //success
  
}
