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
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not find asset with name: %s\n",name);
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
          }else if(SDL_strcmp(tok,"bg_col") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.bgCol.r = (float)atof(tok);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.bgCol.g = (float)atof(tok);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.bgCol.b = (float)atof(tok);
                  dat->rules.themeRules.bgCol.a = 1.0f;
                }else{
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not white_bg_col color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not white_bg_col color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not white_bg_col color string in file: %s.\n",filePath);
              return -1;
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
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col color string in file: %s.\n",filePath);
              return -1;
            }
          }else if(SDL_strcmp(tok,"text_col_inactive") == 0){
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(str2,tok,255);
            tok = strtok(str2,",");
            if(tok!=NULL){
              dat->rules.themeRules.textColInactive.r = (Uint8)floor(atof(tok)*255.0);
              tok = strtok(NULL,",");
              if(tok!=NULL){
                dat->rules.themeRules.textColInactive.g = (Uint8)floor(atof(tok)*255.0);
                tok = strtok(NULL,",");
                if(tok!=NULL){
                  dat->rules.themeRules.textColInactive.b = (Uint8)floor(atof(tok)*255.0);
                  dat->rules.themeRules.textColInactive.a = 255;
                }else{
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col_inactive color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col_inactive color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse text_col_inactive color string in file: %s.\n",filePath);
              return -1;
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
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_normal color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_normal color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_normal color string in file: %s.\n",filePath);
              return -1;
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
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_mouseover color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_mouseover color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_mouseover color string in file: %s.\n",filePath);
              return -1;
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
                  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_selected color string in file: %s.\n",filePath);
                  return -1;
                }
              }else{
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_selected color string in file: %s.\n",filePath);
                return -1;
              }
            }else{
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not parse mod_col_selected color string in file: %s.\n",filePath);
              return -1;
            }
          }else{
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"unknown app rule (%s).\n",tok);
						return -1;
					}
        }
      }
    }
  }else{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"cannot open file %s\n",filePath);
    return -1;
  }
  fclose(inp);

  //setup string ids for mandatory strings
  dat->locStringIDs[LOCSTR_APPLY] = (uint16_t)nameToAssetID("apply",stringIDmap);
  dat->locStringIDs[LOCSTR_CANCEL] = (uint16_t)nameToAssetID("cancel",stringIDmap);
  dat->locStringIDs[LOCSTR_OK] = (uint16_t)nameToAssetID("ok",stringIDmap);
  dat->locStringIDs[LOCSTR_NODB] = (uint16_t)nameToAssetID("no_db",stringIDmap);
	dat->locStringIDs[LOCSTR_GM_STATE] = (uint16_t)nameToAssetID("gm_state",stringIDmap);
	dat->locStringIDs[LOCSTR_QALPHA] = (uint16_t)nameToAssetID("qalpha",stringIDmap);
	dat->locStringIDs[LOCSTR_QBETAMNUS] = (uint16_t)nameToAssetID("q_betaminus",stringIDmap);
	dat->locStringIDs[LOCSTR_SP] = (uint16_t)nameToAssetID("protonsep_energy",stringIDmap);
	dat->locStringIDs[LOCSTR_SN] = (uint16_t)nameToAssetID("neutronsep_energy",stringIDmap);
	dat->locStringIDs[LOCSTR_UNKNOWN] = (uint16_t)nameToAssetID("unknown",stringIDmap);
	dat->locStringIDs[LOCSTR_LEVELINFO_HEADER] = (uint16_t)nameToAssetID("level_info_header",stringIDmap);
	dat->locStringIDs[LOCSTR_ENERGY_KEV] = (uint16_t)nameToAssetID("energy_kev",stringIDmap);
	dat->locStringIDs[LOCSTR_JPI] = (uint16_t)nameToAssetID("jpi",stringIDmap);
	dat->locStringIDs[LOCSTR_HALFLIFE] = (uint16_t)nameToAssetID("halflife",stringIDmap);
	dat->locStringIDs[LOCSTR_LIFETIME] = (uint16_t)nameToAssetID("lifetime",stringIDmap);
	dat->locStringIDs[LOCSTR_DECAYMODE] = (uint16_t)nameToAssetID("decay_mode",stringIDmap);
	dat->locStringIDs[LOCSTR_ENERGY_GAMMA] = (uint16_t)nameToAssetID("energy_gamma",stringIDmap);
	dat->locStringIDs[LOCSTR_INTENSITY_GAMMA] = (uint16_t)nameToAssetID("intensity_gamma",stringIDmap);
	dat->locStringIDs[LOCSTR_MULTIPOLARITY_GAMMA] = (uint16_t)nameToAssetID("multipolarity_gamma",stringIDmap);
	dat->locStringIDs[LOCSTR_FINALLEVEL] = (uint16_t)nameToAssetID("final_level",stringIDmap);
	dat->locStringIDs[LOCSTR_PROTONSDESC] = (uint16_t)nameToAssetID("protons_desc",stringIDmap);
	dat->locStringIDs[LOCSTR_NEUTRONSDESC] = (uint16_t)nameToAssetID("neutrons_desc",stringIDmap);
	dat->locStringIDs[LOCSTR_NOTNATURAL] = (uint16_t)nameToAssetID("not_natural",stringIDmap);
	dat->locStringIDs[LOCSTR_ALLLEVELS] = (uint16_t)nameToAssetID("all_levels",stringIDmap);
	dat->locStringIDs[LOCSTR_BACKTOSUMMARY] = (uint16_t)nameToAssetID("back_to_summary",stringIDmap);
	dat->locStringIDs[LOCSTR_MENUITEM_PREFS] = (uint16_t)nameToAssetID("menuitem_preferences",stringIDmap);
	dat->locStringIDs[LOCSTR_MENUITEM_ABOUT] = (uint16_t)nameToAssetID("menuitem_about",stringIDmap);
	dat->locStringIDs[LOCSTR_ABOUTSTR_VERSION] = (uint16_t)nameToAssetID("about_string_version",stringIDmap);
	dat->locStringIDs[LOCSTR_ABOUTSTR_1] = (uint16_t)nameToAssetID("about_string_1",stringIDmap);
	dat->locStringIDs[LOCSTR_ABOUTSTR_2] = (uint16_t)nameToAssetID("about_string_2",stringIDmap);
	dat->locStringIDs[LOCSTR_ABOUTSTR_3] = (uint16_t)nameToAssetID("about_string_3",stringIDmap);
	dat->locStringIDs[LOCSTR_ABOUTSTR_4] = (uint16_t)nameToAssetID("about_string_4",stringIDmap);
	dat->locStringIDs[LOCSTR_SL_HOYLE] = (uint16_t)nameToAssetID("sl_hoyle",stringIDmap);
	dat->locStringIDs[LOCSTR_SL_NATURALLYOCCURINGISOMER] = (uint16_t)nameToAssetID("sl_naturallyoccuringisomer",stringIDmap);
	dat->locStringIDs[LOCSTR_SL_CLOCKISOMER] = (uint16_t)nameToAssetID("sl_clockisomer",stringIDmap);
	dat->locStringIDs[LOCSTR_PREF_SHELLCLOSURE] = (uint16_t)nameToAssetID("pref_shellclosure",stringIDmap);
	dat->locStringIDs[LOCSTR_PREF_LIFETIME] = (uint16_t)nameToAssetID("pref_lifetime",stringIDmap);
	dat->locStringIDs[LOCSTR_PREF_UIANIM] = (uint16_t)nameToAssetID("pref_ui_animations",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_MENUTITLE] = (uint16_t)nameToAssetID("chartview_menu_title",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME] = (uint16_t)nameToAssetID("chartview_lifetime",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE] = (uint16_t)nameToAssetID("chartview_halflife",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE] = (uint16_t)nameToAssetID("chartview_decaymode",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS] = (uint16_t)nameToAssetID("chartview_2plus",stringIDmap);
	dat->locStringIDs[LOCSTR_CHARTVIEW_R42] = (uint16_t)nameToAssetID("chartview_r42",stringIDmap);
	dat->locStringIDs[LOCSTR_SEARCH_PLACEHOLDER] = (uint16_t)nameToAssetID("search_placeholder",stringIDmap);
	dat->locStringIDs[LOCSTR_SEARCHRES_NUCLIDE] = (uint16_t)nameToAssetID("search_result_nuclide",stringIDmap);
	dat->locStringIDs[LOCSTR_SEARCHRES_EGAMMA] = (uint16_t)nameToAssetID("search_result_egamma",stringIDmap);
	dat->locStringIDs[LOCSTR_SEARCHRES_ELEVEL] = (uint16_t)nameToAssetID("search_result_elevel",stringIDmap);
	dat->locStringIDs[LOCSTR_PREF_UISCALE] = (uint16_t)nameToAssetID("interface_size",stringIDmap);
	dat->locStringIDs[LOCSTR_SMALL] = (uint16_t)nameToAssetID("small",stringIDmap);
	dat->locStringIDs[LOCSTR_LARGE] = (uint16_t)nameToAssetID("large",stringIDmap);
	dat->locStringIDs[LOCSTR_HUGE] = (uint16_t)nameToAssetID("huge",stringIDmap);
	dat->locStringIDs[LOCSTR_DEFAULT] = (uint16_t)nameToAssetID("default",stringIDmap);

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
    //SDL_Log("Opened file: %s\n",filePath);
    while(!(feof(inp))){ //go until the end of file is reached
      if(fgets(str,256,inp)!=NULL){
        str[strcspn(str,"\r\n")] = 0; //strips newline characters from the string read by fgets
        if(SDL_strcmp(str,"")!=0){
          tok = strtok(str,"|");
          if(tok!=NULL){
            if(dat->numStrings >= MAX_NUM_STRINGS){
              SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"maximum number of text strings reached, cannot parse file %s\n",filePath);
              return -1;
            }
            strncpy(stringIDmap->assetID[dat->numStrings],str,256);
            tok = strtok(NULL,""); //get the rest of the string
            strncpy(dat->strings[dat->numStrings],tok,255);
            dat->numStrings++;
          }else{
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"improperly formatted string in file %s\n",filePath);
            return -1;
          }
        }
      }
    }
  }else{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"cannot open file %s\n",filePath);
    return -1;
  }
  fclose(inp);
  stringIDmap->numAssets = dat->numStrings;

  return 0; //success
}


//parse reaction strings
//returns 1 on success, 0 on failure
uint8_t parseRxn(reaction *rxn, char * rxnstring){

	if(strcmp(rxnstring,"COMMENTS")==0){
		return 0;
	}else if(strcmp(rxnstring,"REFERENCES")==0){
		return 0;
	}else if(strncmp(rxnstring,"COULOMB EXCITATION",18)==0){
		rxn->type = REACTIONTYPE_COULEX;
		return 1;
	}else if(strncmp(rxnstring,"INELASTIC SCATTERING",20)==0){
		rxn->type = REACTIONTYPE_SCATTERING;
		rxn->projectileNucl = 65535U;
		rxn->targetNucl = 65535U;
		rxn->ejectileNucl = 65535U;
		return 1;
	}else if((strncmp(rxnstring,"(HI,XNG)",8)==0)||(strncmp(rxnstring,"(HI,XNYPG)",10)==0)){
		//sometimes used for fusion-evaporation data
		//with various projectile/target combinations
		//(often multiple papers grouped together in a
		//single dataset)
		rxn->type = REACTIONTYPE_FUSEVAP;
		rxn->projectileNucl = 65535U;
		rxn->targetNucl = 65535U;
		rxn->ejectileNucl = 65535U;
		return 1;
	}else if(strcmp(rxnstring,"MUONIC ATOM")==0){
		rxn->type = REACTIONTYPE_MUONICATOM;
		rxn->projectileNucl = 65535U;
		rxn->targetNucl = 65535U;
		rxn->ejectileNucl = 65535U;
		return 1;
	}

	rxn->projectileNucl = 65535U;
	rxn->targetNucl = 65535U;
	rxn->ejectileNucl = 65535U;

	//SDL_Log("Parsing reaction string: %s\n",rxnstring);
	char rxnBuff[31], nucName[16], nameBuff[16];
	int16_t A, Z, N;
	strncpy(rxnBuff,rxnstring,30); //copy original string
	char *tok;
	tok = strtok(rxnBuff, " (");
	if(tok!=NULL){
		//get projectile
		A=-1; Z=-1; N=-1;
		strncpy(nucName,tok,15); //copy the nucleus name
		strcpy(nameBuff,nucName); //copy name to temporary buffer
		//get mass number
		tok=strtok(nameBuff,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		if(tok!=NULL){
			A=(int16_t)atoi(tok);
		}
		
		//get proton number
		strcpy(nameBuff,nucName); //copy name to temporary buffer
		tok=strtok(nameBuff,"0123456789");
		if(tok!=NULL){
			if(strcmp(tok,"NN")==0){
				Z=0;
				N=A;
			}else{
				for(int i=1;i<(int)(strlen(tok));i++){
					tok[i]=(char)SDL_tolower(tok[i]); //elemStrToZ() expects only first character to be uppercase
				}
				Z=elemStrToZ(tok);
				if(Z==255){
					Z=-1; //no element found
				}else{
					//get neutron number
					if(A>=0){
						N=A-Z;
					}
				}
			}
		}

		if((Z>=0)&&(N>=0)){
			//projectile found
			rxn->projectileNucl = 0;
			rxn->projectileNucl |= (uint16_t)(N & 255U);
			rxn->projectileNucl |= (uint16_t)((Z & 255U) << 7U);
		}else if(Z>=0){
			//element only string (eg. 'C')
			rxn->projectileNucl = 0;
			rxn->projectileNucl |= (uint16_t)(255U);
			rxn->projectileNucl |= (uint16_t)((Z & 255U) << 7U);
		}else{
			SDL_Log("WARNING: invalid projectile in reaction string: %s\n",rxnstring);
		}
	}else{
		SDL_Log("WARNING: couln't parse reaction string: %s\n",rxnstring);
		return 0;
	}

	
	return 1;
}


//parse half-life values for a given level
void parseHalfLife(level * lev, char * hlstring){

	char *tok;
  char hlAndUnitVal[11]; //both the half-life and its unit
  char hlVal[11], hlUnitVal[4];
  char hlErrVal[7];
  hlVal[0] = '\0';
  hlUnitVal[0] = '\0';
  hlErrVal[0] = '\0';
  memcpy(hlAndUnitVal,&hlstring[0],10);
  hlAndUnitVal[10] = '\0'; //terminate string
  tok = strtok(hlAndUnitVal, " ");
  if(tok!=NULL){
    //SDL_Log("%s\n",tok);
    strcpy(hlVal,tok);
    tok = strtok(NULL, "");
    if(tok!=NULL){
      //SDL_Log("%s\n",tok);
      strncpy(hlUnitVal,tok,3);
      for(uint8_t i=0;i<3;i++){
        if(isspace(hlUnitVal[i])){
          hlUnitVal[i] = '\0'; //terminate string at first space
        }
				if(i==2){
					hlUnitVal[3] = '\0'; //terminate string at end
				}
      }
    }
  }
  memcpy(hlErrVal,&hlstring[10],6);
  hlErrVal[6] = '\0'; //terminate string

	lev->halfLife.val = -1.0f;
	lev->halfLife.err = (uint8_t)atoi(hlErrVal);
  lev->halfLife.format = 0;
  lev->halfLife.unit=VALUE_UNIT_NOVAL;

  //SDL_Log("%s\n",hlstring);
  //SDL_Log("hlVal = %s, hlUnitVal = %s, hlErrVal = %s\n",hlVal,hlUnitVal,hlErrVal);

	if(strcmp(hlVal,"")==0){
    //SDL_Log("Couldn't parse half-life info from string: %s\n",hlstring);
    return;
  }else if(strcmp(hlVal,"?")==0){
		return; //no measured value
	}else if(strcmp(hlVal,"STABLE")==0){
		lev->halfLife.val = 1.0E20f;
		lev->halfLife.unit = VALUE_UNIT_STABLE;
	}else{
		lev->halfLife.val = (float)atof(hlVal);
    tok = strtok(hlVal,".");
    if(tok!=NULL){
      //SDL_Log("%s\n",tok);
      tok = strtok(NULL,"E+");
      if(tok!=NULL){
        //SDL_Log("%s\n",tok);
        lev->halfLife.format = (uint16_t)strlen(tok);
        if(lev->halfLife.format > 15U){
          lev->halfLife.format = 15U; //only 4 bits available for precision
        }
        tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
        if(tok!=NULL){
          //value was in exponent format
					lev->halfLife.exponent = (int8_t)atoi(tok);
					//SDL_Log("%s, parsed to %i\n",tok,lev->halfLife.exponent);
					lev->halfLife.val = lev->halfLife.val / powf(10.0f,(float)(lev->halfLife.exponent));
          lev->halfLife.format |= (uint16_t)(1U << 4); //exponent flag
        }
      }else{
        tok = strtok(hlVal,"E");
        if(tok!=NULL){
          tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
          if(tok!=NULL){
            //value was in exponent format
            lev->halfLife.exponent = (int8_t)atoi(tok);
						//SDL_Log("%s, parsed to %i\n",tok,lev->halfLife.exponent);
						lev->halfLife.val = lev->halfLife.val / powf(10.0f,(float)(lev->halfLife.exponent));
						lev->halfLife.format |= (uint16_t)(1U << 4); //exponent flag
          }
        }
      }

    }

    if(lev->halfLife.val<=0.0f){
      lev->halfLife.unit = VALUE_UNIT_STABLE;
    }else if(strcmp(hlUnitVal,"Y")==0){
      lev->halfLife.unit = VALUE_UNIT_YEARS;
    }else if(strcmp(hlUnitVal,"D")==0){
      lev->halfLife.unit = VALUE_UNIT_DAYS;
    }else if(strcmp(hlUnitVal,"H")==0){
      lev->halfLife.unit = VALUE_UNIT_HOURS;
    }else if(strcmp(hlUnitVal,"M")==0){
      lev->halfLife.unit = VALUE_UNIT_MINUTES;
    }else if(strcmp(hlUnitVal,"S")==0){
      lev->halfLife.unit = VALUE_UNIT_SECONDS;
    }else if(strcmp(hlUnitVal,"MS")==0){
      lev->halfLife.unit = VALUE_UNIT_MILLISECONDS;
    }else if(strcmp(hlUnitVal,"US")==0){
      lev->halfLife.unit = VALUE_UNIT_MICROSECONDS;
    }else if(strcmp(hlUnitVal,"NS")==0){
      lev->halfLife.unit = VALUE_UNIT_NANOSECONDS;
    }else if(strcmp(hlUnitVal,"PS")==0){
      lev->halfLife.unit = VALUE_UNIT_PICOSECONDS;
    }else if(strcmp(hlUnitVal,"FS")==0){
      lev->halfLife.unit = VALUE_UNIT_FEMTOSECONDS;
    }else if(strcmp(hlUnitVal,"AS")==0){
      lev->halfLife.unit = VALUE_UNIT_ATTOSECONDS;
    }else if(strcmp(hlUnitVal,"EV")==0){
      lev->halfLife.unit = VALUE_UNIT_EV;
    }else if(strcmp(hlUnitVal,"KEV")==0){
      lev->halfLife.unit = VALUE_UNIT_KEV;
    }else if(strcmp(hlUnitVal,"MEV")==0){
      lev->halfLife.unit = VALUE_UNIT_MEV;
    }else{
      SDL_Log("Unknown half-life unit: %s (full string: %s)\n",hlUnitVal,hlstring);
    }

    
		if(hlErrVal[0]=='+'){
			//asymmetric errors
			//SDL_Log("err: %s\n",hlErrVal);
			tok = strtok(hlErrVal, "-");
			if(tok != NULL){
				lev->halfLife.err = (uint8_t)atoi(tok); //positive error
				tok = strtok(NULL, ""); //get rest of the string
				if(tok!=NULL){
					uint16_t negErr = ((uint16_t)atoi(tok) & 127U); //negative error
					//SDL_Log("neg err: %u\n",negErr);
					lev->halfLife.format |= (uint16_t)(VALUETYPE_ASYMERROR << 5);
					lev->halfLife.format |= (uint16_t)(negErr << 9);
				}
			}
		}else{
			//check for special value type
			tok = strtok(hlErrVal, " ");
			if(tok!=NULL){
				if(strcmp(tok,"GT")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_GREATERTHAN << 5);
				}else if(strcmp(tok,"GT")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_GREATERTHAN << 5);
				}else if(strcmp(tok,"GE")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_GREATEROREQUALTHAN << 5);
				}else if(strcmp(tok,"LT")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_LESSTHAN << 5);
				}else if(strcmp(tok,"LE")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_LESSOREQUALTHAN << 5);
				}else if(strcmp(tok,"AP")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_APPROX << 5);
				}else if(strcmp(tok,"?")==0){
					lev->halfLife.format |= (uint16_t)(VALUETYPE_UNKNOWN << 5);
				}
			}
		}
		
    
	}

}

//parse spin parity values for a given level
//varDat: contains spin variable data which is common to the entire nuclide
void parseSpinPar(level * lev, sp_var_data * varDat, char * spstring){

	char *tok;
	char str[256], tmpstr[256], tmpstr2[256], val[MAXNUMPARSERVALS][256];
	int numTok=0;
	uint8_t origLevFormat = lev->format;
	uint16_t lsBrakStart = 0;

	lev->numSpinParVals=0;
	memset(lev->spval,0,sizeof(lev->spval));
	//SDL_Log("--------\nstring: %s\n",spstring);

	//check for invalid strings
	strcpy(str,spstring);
	tok = strtok(str, " ");
	if(tok == NULL){
		//SDL_Log("energy %f, strings: %s,%s\n",lev->energy,spstring,tok);
		//SDL_Log("Not a valid spin-parity value.\n");
		//getc(stdin);
		return;
	}
	/*strcpy(str,spstring);
	tok = strtok (str, ".");
	if((tok == NULL)||(strcmp(tok,spstring)!=0)){
		//SDL_Log("%s\n",tok);
		//SDL_Log("Not a valid spin-parity value.\n");
		//getc(stdin);
		return;
	}*/

	strcpy(str,spstring);
	tok = strtok (str, " ,");
	strcpy(val[numTok],tok);
	while(tok != NULL){
		tok = strtok (NULL, " ,");
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

	

	/*SDL_Log("string: %s, number of tokens: %i\n",spstring,numTok);
	for(i=0;i<numTok;i++){
		SDL_Log("| %s ",val[i]);
	}
	SDL_Log("\n");*/

	uint8_t tentative = TENTATIVESP_NONE;
	uint8_t flipTentAfter = 0;

	if(numTok<=0){
		return;
	}else if((strcmp(val[0],"+")==0)&&(numTok==1)){
		lev->spval[lev->numSpinParVals].parVal = 1;
		lev->spval[lev->numSpinParVals].spinVal = 255;
		lev->spval[lev->numSpinParVals].format |= ((TENTATIVESP_NOSPIN & 7U) << 9U);
		lev->numSpinParVals=1;
		return;
	}else if((strcmp(val[0],"-")==0)&&(numTok==1)){
		lev->spval[lev->numSpinParVals].parVal = -1;
		lev->spval[lev->numSpinParVals].spinVal = 255;
		lev->spval[lev->numSpinParVals].format |= ((TENTATIVESP_NOSPIN & 7U) << 9U);
		lev->numSpinParVals=1;
		return;
	}else{
		for(int i=0;i<numTok;i++){
			if(i<MAXSPPERLEVEL){

				uint8_t lsBrak = 0; //temp var for bracket checking
				uint8_t rsBrak = 0; //temp var for bracket checking
				uint8_t spVarNum = 255; //temp var for spin variable assignment

				//special cases
				if(strcmp(val[i],"TO")==0){
					//specifies a range between the prior and next spin values
					//eg. '3/2 TO 7/2'
					lev->spval[lev->numSpinParVals].format |= ((TENTATIVESP_RANGE & 7U) << 9U);
					lev->spval[lev->numSpinParVals].spinVal = 255;
					lev->numSpinParVals++;
					continue;
				}else if(strcmp(val[i],"&")==0){
					//equivalent to a comma 
					continue;
				}else if(strcmp(val[i],"AND")==0){
					//equivalent to a comma 
					continue;
				}

				//check for J+number variable spin case
				uint8_t varSpin=0;
				for(int j=((int)strlen(val[i])-1); j>=0; j--){
					if(j<((int)strlen(val[i])-1)){
						if(j>0){
							if(val[i][j]=='+'){
								if(isdigit(val[i][j+1])){
									varSpin=1;
									break;
								}
							}
						}
					}
					if(isalpha(val[i][j])){
						varSpin=1;
					}
				}
				if(varSpin){
					//J or J+number variable spin value
					//SDL_Log("Variable spin string: %s\n",spstring);
					lev->format = 0;
					//SDL_Log("val: %s\n",val[i]);
					if(val[i][0] == '('){
						lsBrak = 1;
						lsBrakStart = (uint16_t)lev->numSpinParVals;
					}
					int len = (int)strlen(val[i]);
					if(val[i][len-1]==')'){
						rsBrak = 1;
					}
					lev->spval[lev->numSpinParVals].format = 1;
					strcpy(tmpstr,val[i]);
					tok = strtok(tmpstr,"(+");
					if(tok!=NULL){
						strcpy(tmpstr2,tok);
						tok = strtok(NULL,")");
						if(tok!=NULL){
							//SDL_Log("variable name (with offset): %s\n",tmpstr2);
							//check variable name
							for(int j=0;j<(varDat->numSpVars);j++){
								//SDL_Log("%i %s %s\n",j,tmpstr2,varDat->spVars[j]);
								if(strcmp(tmpstr2,varDat->spVars[j])==0){
									spVarNum = (uint8_t)j;
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_NUMBER << 1U);
									lev->spval[lev->numSpinParVals].format |= (uint16_t)(spVarNum << 5U); //set variable index
									//SDL_Log("variable index %u\n",spVarNum);
								}
							}
							if(spVarNum == 255){
								//SDL_Log("new variable found, index %u\n",varDat->numSpVars);
								//couldn't match to a previous spin variable, make a new one
								if(varDat->numSpVars < MAX_SPIN_VARS){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_NUMBER << 1U);
									lev->spval[lev->numSpinParVals].format |= (uint16_t)(varDat->numSpVars << 5U); //set variable index
									strcpy(varDat->spVars[varDat->numSpVars],tmpstr2);
									varDat->numSpVars++;
								}else{
									SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"parseSpinPar - too many spin variables could not parse value %s, need to increase MAX_SPIN_VARS!\n",val[i]);
									exit(-1);
								}
							}
							strcpy(val[i],tok); //for further parsing, only give spin value after + sign
						}else{
							//non-offset variable spin value
							//SDL_Log("non offset: %s\n",tmpstr2);
							//check for variable value types
							char varValType[3];
							uint8_t varValTypePos = 255;
							for(int j=2;j<=((int)strlen(tmpstr2));j++){
								memcpy(varValType, &tmpstr2[j-2], 2);
								tmpstr2[2] = '\0';
								if(strcmp(varValType,"GT")==0){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATERTHAN << 1U);
									varValTypePos = (uint8_t)(j-2);
									break;
								}else if(strcmp(varValType,"GE")==0){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATEROREQUALTHAN << 1U);
									varValTypePos = (uint8_t)(j-2);
									break;
								}else if(strcmp(varValType,"LT")==0){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSTHAN << 1U);
									varValTypePos = (uint8_t)(j-2);
									break;
								}else if(strcmp(varValType,"LE")==0){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSOREQUALTHAN << 1U);
									varValTypePos = (uint8_t)(j-2);
									break;
								}else if(strcmp(varValType,"AP")==0){
									lev->spval[lev->numSpinParVals].format |= (VALUETYPE_APPROX << 1U);
									varValTypePos = (uint8_t)(j-2);
									break;
								}
							}
							//SDL_Log("varValTypePos: %u\n",varValTypePos);
							if(varValTypePos == 255){
								//just a variable name
								//SDL_Log("variable name (no offset, default value type): %s\n",tmpstr2);
								lev->spval[lev->numSpinParVals].format |= (VALUETYPE_NUMBER << 1U);
								for(int j=0;j<(varDat->numSpVars);j++){
									if(strcmp(tmpstr2,varDat->spVars[j])==0){
										spVarNum = (uint8_t)j;
										lev->spval[lev->numSpinParVals].format |= (uint16_t)(spVarNum << 5U); //set variable index
										//SDL_Log("variable index %u\n",spVarNum);
									}
								}
								if(spVarNum == 255){
									//SDL_Log("new variable found, index %u\n",varDat->numSpVars);
									//couldn't match to a previous spin variable, make a new one
									if(varDat->numSpVars < MAX_SPIN_VARS){
										lev->spval[lev->numSpinParVals].format |= (uint16_t)(varDat->numSpVars << 5U); //set variable index
										strcpy(varDat->spVars[varDat->numSpVars],tmpstr2);
										varDat->numSpVars++;
									}else{
										SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"parseSpinPar - too many spin variables could not parse value %s, need to increase MAX_SPIN_VARS!\n",val[i]);
										exit(-1);
									}
								}
							}else if(varValTypePos >= 1){
								//variable name specified before value type
								if(varValTypePos<=16){
									//get substring corresponding to variable name
									char varName[16];
									memcpy(varName,&tmpstr2,varValTypePos);
									//SDL_Log("variable name (no offset, special value type): %s\n",varName);
									//check variable name
									for(int j=0;j<(varDat->numSpVars);j++){
										if(strcmp(varName,varDat->spVars[j])==0){
											spVarNum = (uint8_t)j;
											lev->spval[lev->numSpinParVals].format |= (uint16_t)(spVarNum << 5U); //set variable index
										}
									}
									if(spVarNum == 255){
										//SDL_Log("new variable found, index %u\n",varDat->numSpVars);
										//couldn't match to a previous spin variable, make a new one
										if(varDat->numSpVars < MAX_SPIN_VARS){
											lev->spval[lev->numSpinParVals].format |= (uint16_t)(varDat->numSpVars << 5U); //set variable index
											strcpy(varDat->spVars[varDat->numSpVars],varName);
											varDat->numSpVars++;
										}else{
											SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"parseSpinPar - too many spin variables could not parse value %s, need to increase MAX_SPIN_VARS!\n",val[i]);
											exit(-1);
										}
									}
								}
							}else{
								//just a variable value type followed by a spin value
								//so not a variable spin
								//SDL_Log("non-variable spin string: %s, tmpstr2: %s\n",spstring,tmpstr2);
								lev->spval[lev->numSpinParVals].format = (uint16_t)(lev->spval[lev->numSpinParVals].format & ~(1U)); //remove variable spin flag
								lev->format = origLevFormat; //restore original level format (including whether spins should be half-integer)
								if(strlen(tmpstr2) == 2){
									//only have the variable value type, the spin value was separated into the next value
									//i++;
								}
								//SDL_Log("val: %s\n",val[i]);
							}
						}
					}
					//SDL_Log("val: %s\n",val[i]);
				}else{
					//not a variable spin value
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_NUMBER << 1U);
				}

				//check for cases where the spin-parity value is separated by a space
				if(strcmp(val[i],"(LE")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSOREQUALTHAN << 1U);
					lsBrak = 1;
					lsBrakStart = (uint16_t)lev->numSpinParVals;
					continue;
				}else if(strcmp(val[i],"LE")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSOREQUALTHAN << 1U);
					continue;
				}else if(strcmp(val[i],"(LT")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSTHAN << 1U);
					lsBrak = 1;
					lsBrakStart = (uint16_t)lev->numSpinParVals;
					continue;
				}else if(strcmp(val[i],"LT")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSTHAN << 1U);
					continue;
				}else if(strcmp(val[i],"(GE")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATEROREQUALTHAN << 1U);
					lsBrak = 1;
					lsBrakStart = (uint16_t)lev->numSpinParVals;
					continue;
				}else if(strcmp(val[i],"GE")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATEROREQUALTHAN << 1U);
					continue;
				}else if(strcmp(val[i],"(GT")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATERTHAN << 1U);
					lsBrak = 1;
					lsBrakStart = (uint16_t)lev->numSpinParVals;
					continue;
				}else if(strcmp(val[i],"GT")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATERTHAN << 1U);
					continue;
				}else if(strcmp(val[i],"(AP")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_APPROX << 1U);
					lsBrak = 1;
					lsBrakStart = (uint16_t)lev->numSpinParVals;
					continue;
				}else if(strcmp(val[i],"AP")==0){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_APPROX << 1U);
					continue;
				}

				//check for ranges specified without spaces
				if(val[i][0]=='>'){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_GREATERTHAN << 1U);
				}else if(val[i][0]=='<'){
					lev->spval[lev->numSpinParVals].format |= (VALUETYPE_LESSTHAN << 1U);
				}

				//check for brackets
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"(");
				if(tok!=NULL){
					if(strcmp(tok,val[i])!=0){
						//bracket exists on left side
						lsBrak = 1;
						lsBrakStart = (uint16_t)lev->numSpinParVals;
					}
				}
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,")");
				if(tok!=NULL){
					if(strcmp(tok,val[i])!=0){
						//bracket exists on right side
						rsBrak = 1;
					}
				}

				if(flipTentAfter){
					//SDL_Log("setting tentative marker...\n");
					//tentative marker
					if(tentative == TENTATIVESP_NONE)
						tentative = TENTATIVESP_SPINANDPARITY;
					else if(tentative == TENTATIVESP_SPINANDPARITY)
						tentative = TENTATIVESP_NONE;
				}
				if(!lsBrak && rsBrak){
					//right bracket only, tentativeness is the same as the previous value
					//but the next value is outside of the brackets
					flipTentAfter = 1;
				}else if(lsBrak && !rsBrak){
					tentative = TENTATIVESP_SPINANDPARITY;
				}else if(lsBrak && rsBrak){
					tentative = TENTATIVESP_SPINANDPARITY;
				}

				//check for parity
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"+-");
				if(tok!=NULL){
					if(strcmp(tok,val[i])!=0){
						//SDL_Log("setting parity marker, tok: %s, val: %s\n",tok,val[i]);
						//contains parity info
						tok=strtok(val[i],"/(0123456789GELTAP, ");
						//SDL_Log("tok: %s\n",tok);
						if((strcmp(tok,"+")==0)||(strcmp(tok,"+)")==0)){
							lev->spval[lev->numSpinParVals].parVal = 1;
						}else if((strcmp(tok,"-")==0)||(strcmp(tok,"-)")==0)){
							lev->spval[lev->numSpinParVals].parVal = -1;
						}else if(strcmp(tok,"(+)")==0){
							lev->spval[lev->numSpinParVals].parVal = 1;
							lev->spval[lev->numSpinParVals].format |= ((TENTATIVESP_PARITYONLY & 7U) << 9U); //only parity is tentative
						}else if(strcmp(tok,"(-)")==0){
							lev->spval[lev->numSpinParVals].parVal = -1;
							lev->spval[lev->numSpinParVals].format |= ((TENTATIVESP_PARITYONLY & 7U) << 9U); //only parity is tentative
						}else if(strcmp(tok,")-")==0){
							//all spin values negative parity
							for(int j=lsBrakStart;j<lev->numSpinParVals;j++){
								lev->spval[j].parVal = -1;
								uint8_t prevTentative = (uint8_t)((uint16_t)(lev->spval[j].format >> 9U) & 7U);
								if(prevTentative != TENTATIVESP_RANGE){
									lev->spval[j].format = (uint16_t)(lev->spval[j].format & ~(7U << 9U)); //unset tentative type for previous spin-parity values
									lev->spval[j].format |= ((TENTATIVESP_SPINONLY & 7U) << 9U); //only spin is tentative
								}
							}
							lev->spval[lev->numSpinParVals].parVal = -1;
							tentative = TENTATIVESP_SPINONLY; //only spin is tentative
						}else if(strcmp(tok,")+")==0){
							//all spin values positive parity
							//SDL_Log("Setting all spin values to +ve parity.\n");
							for(int j=lsBrakStart;j<lev->numSpinParVals;j++){
								lev->spval[j].parVal = 1;
								uint8_t prevTentative = (uint8_t)((uint16_t)(lev->spval[j].format >> 9U) & 7U);
								if(prevTentative != TENTATIVESP_RANGE){
									lev->spval[j].format = (uint16_t)(lev->spval[j].format & ~(7U << 9U)); //unset tentative type for previous spin-parity values
									lev->spval[j].format |= ((TENTATIVESP_SPINONLY & 7U) << 9U); //only spin is tentative
								}
							}
							lev->spval[lev->numSpinParVals].parVal = 1;
							tentative = TENTATIVESP_SPINONLY; //only spin is tentative
						}
					}
				}

				//extract spin
				lev->spval[lev->numSpinParVals].spinVal = 255; //default to unknown spin
				strcpy(tmpstr,val[i]);
				tok=strtok(tmpstr,"()+-, GELTAP><");
				if(tok!=NULL){
					strcpy(tmpstr2,tok);
					tok=strtok(tok,"/");
					if(strcmp(tok,tmpstr2)!=0){
						//SDL_Log("Detected half-integer spin.\n");
						if(atoi(tok) >= 255){
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"high spin value: %i/2\n",atoi(tok));
						}
						lev->spval[lev->numSpinParVals].spinVal=(uint8_t)atoi(tok);
						//SDL_Log("Spin detected: %u/2\n",lev->spval[lev->numSpinParVals].spinVal);
					}else{
						if(atoi(tmpstr2) >= 255){
							SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"high spin value: %i\n",atoi(tmpstr2));
						}
						lev->spval[lev->numSpinParVals].spinVal=(uint8_t)atoi(tmpstr2);
						//SDL_Log("Spin detected: %u\n",lev->spval[lev->numSpinParVals].spinVal);
					}
				}

				lev->spval[lev->numSpinParVals].format |= (uint16_t)((tentative & 7U) << 9U);
				lev->numSpinParVals++;

				//SDL_Log("%f keV Entry %i: spin %i (half-int %i), parity %i, tentative %i\n",lev->energy,lev->numSpinParVals,lev->spval[lev->numSpinParVals-1].spinVal,lev->spval[lev->numSpinParVals-1].format,lev->spval[lev->numSpinParVals-1].parVal,lev->spval[lev->numSpinParVals-1].tentative);
			}
		}
		//SDL_Log("number of spin vals: %i\n",lev->numSpinParVals);
		
	}

	//getc(stdin);

}


void getNuclNZ(nucl *nuc, const char *nucName){

	char str[256];
	int16_t Z = -1;
	int16_t N = -1;
	char *tok;
	
	//get mass number
	strcpy(str,nucName); //copy the nucleus name
	tok=strtok(str,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	int16_t A=(int16_t)atoi(tok);
	
	//get proton number
	strcpy(str,nucName); //copy the nucleus name
	tok=strtok(str,"0123456789");
	if(tok!=NULL){
		if(strcmp(tok,"NN")==0){
			Z=0;
			N=A;
		}else{
			for(int i=1;i<(int)(strlen(tok));i++){
				tok[i]=(char)SDL_tolower(tok[i]); //elemStrToZ() expects only first character to be uppercase
			}
			Z=elemStrToZ(tok);
			if(Z==255){
				Z=-1; //no element found
			}else{
      	N=A-Z; //get neutron number
			}
		}
	}
	
	nuc->N=N;
	nuc->Z=Z;
	
}

uint8_t getDcyModeFromENSDFSubstr(const char *substr){
	if(strcmp(substr,"%IT")==0){
		return DECAYMODE_IT;
	}else if(strcmp(substr,"%B-")==0){
		return DECAYMODE_BETAMINUS;
	}else if(strcmp(substr,"%B+")==0){
		return DECAYMODE_BETAPLUS;
	}else if(strcmp(substr,"%EC")==0){
		return DECAYMODE_EC;
	}else if(strcmp(substr,"%EC+%B+")==0){
		return DECAYMODE_ECANDBETAPLUS;
	}else if(strcmp(substr,"%B-N")==0){
		return DECAYMODE_BETAMINUS_NEUTRON;
	}else if(strcmp(substr,"%B+P")==0){
		return DECAYMODE_BETAPLUS_PROTON;
	}else if(strcmp(substr,"%B+2P")==0){
		return DECAYMODE_BETAPLUS_TWOPROTON;
	}else if(strcmp(substr,"%ECP")==0){
		return DECAYMODE_EC_PROTON;
	}else if(strcmp(substr,"%P")==0){
		return DECAYMODE_PROTON;
	}else if(strcmp(substr,"%2P")==0){
		return DECAYMODE_TWOPROTON;
	}else if(strcmp(substr,"%N")==0){
		return DECAYMODE_NEUTRON;
	}else if(strcmp(substr,"%2N")==0){
		return DECAYMODE_TWONEUTRON;
	}else if(strcmp(substr,"%D")==0){
		return DECAYMODE_DEUTERON;
	}else if(strcmp(substr,"%3HE")==0){
		return DECAYMODE_3HE;
	}else if(strcmp(substr,"%A")==0){
		return DECAYMODE_ALPHA;
	}else if(strcmp(substr,"%B-A")==0){
		return DECAYMODE_BETAMINUS_ALPHA;
	}else if(strcmp(substr,"%SF")==0){
		return DECAYMODE_SPONTANEOUSFISSION;
	}else if(strcmp(substr,"%B-SF")==0){
		return DECAYMODE_BETAMINUS_SPONTANEOUSFISSION;
	}else if(strcmp(substr,"%2B-")==0){
		return DECAYMODE_2BETAMINUS;
	}else if(strcmp(substr,"%2B+")==0){
		return DECAYMODE_2BETAPLUS;
	}else if(strcmp(substr,"%2EC")==0){
		return DECAYMODE_2EC;
	}else if(strcmp(substr,"%14C")==0){
		return DECAYMODE_14C;
	}else if(strcmp(substr,"%{+20}Ne")==0){
		return DECAYMODE_20NE;
	}else if(strcmp(substr,"%{+25}Ne")==0){
		return DECAYMODE_25NE;
	}else if(strcmp(substr,"%{+28}Mg")==0){
		return DECAYMODE_28MG;
	}else if(strcmp(substr,"%34SI")==0){
		return DECAYMODE_34SI;
	}else if(strcmp(substr,"%|b{+-}")==0){
		return DECAYMODE_BETAMINUS;
	}else if(strcmp(substr,"%|b{++}")==0){
		return DECAYMODE_BETAPLUS;
	}else if(strcmp(substr,"%|b{++}")==0){
		return DECAYMODE_BETAPLUS;
	}else if(strcmp(substr,"%|e+%|b{++}")==0){
		return DECAYMODE_ECANDBETAPLUS;
	}else if(strcmp(substr,"%|e")==0){
		return DECAYMODE_EC;
	}else if(strcmp(substr,"%|a")==0){
		return DECAYMODE_ALPHA;
	}else{
		return DECAYMODE_ENUM_LENGTH;
	}
}

//parses a substring containing info on a single decay mode
//eg. '%EC+%B+>99.87'
//returns 1 if successful
uint8_t parseDcyModeSubstr(ndata *nd, const uint16_t dcyModeInd, const char *substr){
	
	char *tok, *tok2;
	char substrCpy[128], valBuff[16], errBuff[16];

	//SDL_Log("Parsing decay mode substring: %s\n",substr);
	strcpy(substrCpy,substr);
	tok = strtok(substrCpy," =><");
	if(tok!=NULL){
		uint8_t dcyMode = getDcyModeFromENSDFSubstr(tok);
		if(dcyMode != DECAYMODE_ENUM_LENGTH){
			nd->dcyMode[dcyModeInd].type = dcyMode;
			nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_NUMBER; //default
		}else{
			return 0;
		}
		//check for decay modes using '>' or '<'
		strcpy(substrCpy,substr);
		tok = strtok(substrCpy,">");
		tok = strtok(NULL,"$ ,");
		if(tok!=NULL){
			//SDL_Log("Parsing decay mode with '>'.\n");
			nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_GREATERTHAN;
		}else{
			strcpy(substrCpy,substr);
			tok = strtok(substrCpy,"<");
			tok = strtok(NULL,"$ ,");
			if(tok!=NULL){
				//SDL_Log("Parsing decay mode with '<'.\n");
				nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_LESSTHAN;
			}
		}
		strcpy(substrCpy,substr);
		tok = strtok(substrCpy," =><");
		tok = strtok(NULL,"$ ,");
		if(tok!=NULL){
			//SDL_Log("value tok: %s\n",tok);
			strncpy(valBuff,tok,15); //value
			tok = strtok(NULL,""); //get the rest of the string
			if(tok!=NULL){
				//SDL_Log("err tok: %s\n",tok);
				strncpy(errBuff,tok,15); //error on value
			}
			tok2 = strtok(valBuff," ");
			if(tok2 != NULL){
				//SDL_Log("tok2: %s\n",tok2);
				if(strcmp(tok2,"GT")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_GREATERTHAN;
					tok2 = strtok(NULL," ");
				}else if(strcmp(tok2,"GE")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_GREATEROREQUALTHAN;
					tok2 = strtok(NULL," ");
				}else if(strcmp(tok2,"LT")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_LESSTHAN;
					tok2 = strtok(NULL," ");
				}else if(strcmp(tok2,"LE")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_LESSOREQUALTHAN;
					tok2 = strtok(NULL," ");
				}else if(strcmp(tok2,"AP")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_APPROX;
					tok2 = strtok(NULL," ");
				}else if(strcmp(tok2,"?")==0){
					nd->dcyMode[dcyModeInd].prob.unit = VALUETYPE_UNKNOWN;
					tok2 = strtok(NULL," ");
				}
				if(tok2!=NULL){
					nd->dcyMode[dcyModeInd].prob.format = 0;
					char valueCpy[16];
					strncpy(valueCpy,tok2,15);
					strcpy(valBuff,valueCpy);
					//SDL_Log("%s\n",tok2);
					float probVal = (float)atof(tok2);

					if(probVal >= 0.0f){
						//get the number of sig figs in the decay probability
						//SDL_Log("valBuff: %s\n",valBuff);
						tok2 = strtok(valBuff,".");
						if(tok2!=NULL){
							//SDL_Log("%s\n",tok2);
							tok2 = strtok(NULL,"E+"); //some decay probabilites are specified with exponents
							if(tok2!=NULL){
								
									//SDL_Log("%s\n",tok2);
									uint16_t len = (uint16_t)strlen(tok2);
									//check for trailing empty spaces
									for(uint16_t i=0;i<len;i++){
										if(isspace(tok2[i])){
											len = i;
											break;
										}
									}
									nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(len & 15U);
									//SDL_Log("format: %u\n",nd->dcyMode[dcyModeInd].prob.format);
									if(((nd->dcyMode[dcyModeInd].prob.format >> 5U) & 15U) != VALUETYPE_PLUSX){
										tok2 = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
										if(tok2!=NULL){
											//SDL_Log("decay probability in exponent form: %s\n",valueCpy);
											//value was in exponent format
											nd->dcyMode[dcyModeInd].prob.exponent = (int8_t)atoi(tok2);
											probVal = probVal / powf(10.0f,(float)(nd->dcyMode[dcyModeInd].prob.exponent));
											nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
							}else{
								//potentially an exponent form value with no decimal place
								strcpy(valBuff,valueCpy);
								tok2 = strtok(valBuff,"E");
								if(tok2!=NULL){
									tok2 = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
									if(tok2!=NULL){
										//SDL_Log("%s\n",tok2);
										//value was in exponent format
										nd->dcyMode[dcyModeInd].prob.exponent = (int8_t)atoi(tok2);
										probVal = probVal / powf(10.0f,(float)(nd->dcyMode[dcyModeInd].prob.exponent));
										nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}
							}
						}
						
					}

					nd->dcyMode[dcyModeInd].prob.val = probVal;
					//SDL_Log("prob val: %f\n",(double)nd->dcyMode[dcyModeInd].prob.val);
					
					//quick consistency check - if the probability is exactly zero,
					//then there's probably something wrong with the parsing
					//(likely an improperly formatted string)
					if((nd->dcyMode[dcyModeInd].prob.val == 0.0f)&&(nd->dcyMode[dcyModeInd].prob.unit == VALUETYPE_NUMBER)){
						return 0;
					}
					
					//handle errors
					if(tok != NULL){ //error string was valid, so errBuff was defined earlier
						if(errBuff[0]=='+'){
							//asymmetric errors
							//SDL_Log("err: %s\n",errBuff);
							tok2 = strtok(errBuff, "-");
							if(tok2 != NULL){
								//SDL_Log("pos err: %s\n",tok2);
								nd->dcyMode[dcyModeInd].prob.err = (uint8_t)atoi(tok2); //positive error
								tok2 = strtok(NULL, ""); //get rest of the string
								if(tok2 != NULL){
									uint16_t negErr = ((uint16_t)atoi(tok2) & 127U); //negative error
									//SDL_Log("neg err: %u\n",negErr);
									nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(VALUETYPE_ASYMERROR << 5);
									nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(negErr << 9);
								}
							}
						}else{
							nd->dcyMode[dcyModeInd].prob.err = (uint8_t)atoi(errBuff);
							//SDL_Log("err: %u\n",nd->dcyMode[dcyModeInd].prob.err);
						}
					}
				}else if(tok != NULL){
					//error string was valid, so errBuff was defined earlier
					//in this case, we have a string like '%SF LE 12.5',
					//where '12.5' was put into errBuff
					char valueCpy[16];
					strcpy(valueCpy,errBuff);
					float probVal = (float)atof(errBuff);
					
					//count sig figs
					tok2 = strtok(errBuff,".");
					if(tok2!=NULL){
						//SDL_Log("%s\n",tok2);
						tok2 = strtok(NULL,"E+"); //some decay probabilites are specified with exponents
						if(tok2!=NULL){
							
								//SDL_Log("%s\n",tok2);
								uint16_t len = (uint16_t)strlen(tok2);
								//check for trailing empty spaces
								for(uint16_t i=0;i<len;i++){
									if(isspace(tok2[i])){
										len = i;
										break;
									}
								}
								nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(len & 15U);
								//SDL_Log("format: %u\n",nd->dcyMode[dcyModeInd].prob.format);
								if(((nd->dcyMode[dcyModeInd].prob.format >> 5U) & 15U) != VALUETYPE_PLUSX){
									tok2 = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
									if(tok2!=NULL){
										//SDL_Log("decay probability in exponent form: %s\n",valueCpy);
										//value was in exponent format
										nd->dcyMode[dcyModeInd].prob.exponent = (int8_t)atoi(tok2);
										probVal = probVal / powf(10.0f,(float)(nd->dcyMode[dcyModeInd].prob.exponent));
										nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}
						}else{
							//potentially an exponent form value with no decimal place
							strcpy(errBuff,valueCpy);
							tok2 = strtok(errBuff,"E");
							if(tok2!=NULL){
								tok2 = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
								if(tok2!=NULL){
									//SDL_Log("%s\n",tok2);
									//value was in exponent format
									nd->dcyMode[dcyModeInd].prob.exponent = (int8_t)atoi(tok2);
									probVal = probVal / powf(10.0f,(float)(nd->dcyMode[dcyModeInd].prob.exponent));
									nd->dcyMode[dcyModeInd].prob.format |= (uint16_t)(1U << 4); //exponent flag
								}
							}
						}
					}

					nd->dcyMode[dcyModeInd].prob.val = probVal;
				}
			}
			
			//SDL_Log("Found decay with type %u and probability: %f %u (type %u)\n",nd->dcyMode[dcyModeInd].type,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.err,nd->dcyMode[dcyModeInd].prob.unit);
			return 1;
		}
	}
	return 0;
}

//set initial databae values prior to importing data
void initialize_database(ndata *nd){
	
	memset(nd,0,sizeof(ndata));
	
	nd->numNucl = -1;
	nd->numLvls = 0;
	nd->numTran = 0;
	nd->numDecModes = 0;
	nd->numRxns = 0;
	for(uint32_t i=0;i<MAXNUMNUCL;i++){
		nd->nuclData[i].numLevels = 0;
	}
	for(uint32_t i=0;i<MAXNUMLVLS;i++){
		nd->levels[i].numDecModes = -1; //default if no decay mode info in data
	}
	for(uint32_t i=0; i<MAX_PROTON_NUM; i++){
		nd->minNforZ[i] = MAX_NEUTRON_NUM+1;
	}
	for(uint32_t i=0; i<MAX_NEUTRON_NUM; i++){
		nd->minZforN[i] = MAX_PROTON_NUM+1;
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
int parseENSDFFile(const char * filePath, ndata * nd){

  FILE *efile;
  char *tok;
  char str[256];//string to be read from file (will be tokenized)
  char nuclNameStr[10];
  char line[256],val[MAXNUMPARSERVALS][256];
  int tokPos;//position when tokenizing
  int firstQLine = 1; //flag to specify whether Q values have been read in for a specific nucleus
	uint8_t qValDecModeFlag = 0; //flag specifying whether a Q-value was parsed as a decay mode
	uint8_t decModeLineParsed = 0; //flag specifying whether a decay mode line has already been parsed
	uint8_t qValDecModeType = DECAYMODE_ENUM_LENGTH; //the specific decay mode specified by Q-value
	double longestIsomerHl = 0.0; //longest isomeric state half-life for a given nucleus
	uint8_t isomerMValInNucl = 0;
	sp_var_data varDat;
  
  //subsection of the entry for a particular nucleus that the parser is at
  //each nucleus has multiple entries, including adopted gammas, and gammas 
  //associated with a particlular reaction mechanism
  int subSec=0;
	uint8_t startedParsingSec=0;
	uint8_t numRxnsParsed=0;
  
  //open the file and read all parameters
  if((efile=fopen(filePath,"r"))==NULL){
    //file doesn't exist, and will be omitted from the database
		//SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Cannot open the ENSDF file %s\n",filePath);
		return 0;
	}

	SDL_Log("Reading file: %s\n",filePath);

  while(!(feof(efile))){ //go until the end of file is reached

		if(fgets(str,256,efile)!=NULL){ //get an entire line

			strncpy(line,str,256); //store the entire line
			//SDL_Log("%s\n",line);
			if(isEmpty(str)){
				subSec++; //empty line, increment which subsection we're on
				startedParsingSec=0;
				firstQLine = 1;
				continue;
			}else{
				tok=strtok(str," ");
				tokPos=0;
				strcpy(val[tokPos],tok);
				while(tok != NULL){
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
				//new nuclide

				//first handle any business arising from the previous nuclide
				if(qValDecModeFlag){
					if(nd->levels[nd->nuclData[nd->numNucl].firstLevel].numDecModes == 0){
						nd->levels[nd->nuclData[nd->numNucl].firstLevel].numDecModes = 1;
						nd->levels[nd->nuclData[nd->numNucl].firstLevel].firstDecMode = nd->numDecModes;
						nd->dcyMode[nd->numDecModes].type = qValDecModeType;
						nd->dcyMode[nd->numDecModes].prob.val = 100.0f;
						nd->dcyMode[nd->numDecModes].prob.err = 0;
						nd->dcyMode[nd->numDecModes].prob.format = 0;
						//SDL_Log("Assigned decay mode %u\n",nd->dcyMode[nd->numDecModes].type);
						nd->numDecModes++;
						if(nd->numDecModes > MAXNUMDECAYMODES){
							SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Maximum number of decay modes (%i) exceeded!\n",MAXNUMDECAYMODES);
							return -1;
						}
					}
				}

				if(nd->numNucl<MAXNUMNUCL){
					nd->numNucl++;
					subSec=0; //we're at the beginning of the entry for this nuclide
					startedParsingSec=1;
					numRxnsParsed=0; //no reactions parsed yet for this nuclide
					longestIsomerHl = 0.0;
					isomerMValInNucl = 0;
					qValDecModeFlag = 0;
					decModeLineParsed = 0;
					qValDecModeType = DECAYMODE_ENUM_LENGTH;
					nd->nuclData[nd->numNucl].numIsomerMVals = 0;
					nd->nuclData[nd->numNucl].longestIsomerLevel = MAXNUMLVLS;
					nd->nuclData[nd->numNucl].abundance.unit = VALUE_UNIT_NOVAL; //default
					nd->nuclData[nd->numNucl].firstRxn = nd->numRxns;
					memset(&varDat,0,sizeof(sp_var_data));
					//SDL_Log("Adding gamma data for nucleus %s\n",val[0]);
					memcpy(nuclNameStr,val[0],10);
					nuclNameStr[9] = '\0'; //terminate string
					getNuclNZ(&nd->nuclData[nd->numNucl],nuclNameStr); //get N and Z
					if((nd->nuclData[nd->numNucl].N > MAX_NEUTRON_NUM)||(nd->nuclData[nd->numNucl].Z > MAX_PROTON_NUM)){
						SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"parseENSDFFile - invalid proton (%i) or neutron (%i) number.\n",nd->nuclData[nd->numNucl].Z,nd->nuclData[nd->numNucl].N);
						return -1;
					}
					//check for unobserved/inferred/tentative nuclei
					char obsbuff[8];
					memcpy(obsbuff, &line[23], 7);
					obsbuff[7] = '\0';
					if(strcmp(obsbuff,":NOT OB")==0){
						nd->nuclData[nd->numNucl].flags = OBSFLAG_UNOBSERVED;
					}else if(strcmp(obsbuff,":UNOBSE")==0){
						nd->nuclData[nd->numNucl].flags = OBSFLAG_UNOBSERVED;
					}else if(strcmp(obsbuff,":INFERR")==0){
						nd->nuclData[nd->numNucl].flags = OBSFLAG_INFERRED;
					}else if(strcmp(obsbuff,":TENTAT")==0){
						nd->nuclData[nd->numNucl].flags = OBSFLAG_TENTATIVE;
					}else{
						nd->nuclData[nd->numNucl].flags = OBSFLAG_OBSERVED;
						if(nd->nuclData[nd->numNucl].N > nd->maxNforZ[nd->nuclData[nd->numNucl].Z]){
							nd->maxNforZ[nd->nuclData[nd->numNucl].Z] = (uint16_t)nd->nuclData[nd->numNucl].N;
						}
						if(nd->nuclData[nd->numNucl].N < nd->minNforZ[nd->nuclData[nd->numNucl].Z]){
							if(nd->nuclData[nd->numNucl].N >= 0){
								nd->minNforZ[nd->nuclData[nd->numNucl].Z] = (uint16_t)nd->nuclData[nd->numNucl].N;
							}
						}
						if(nd->nuclData[nd->numNucl].Z > nd->maxZforN[nd->nuclData[nd->numNucl].N]){
							nd->maxZforN[nd->nuclData[nd->numNucl].N] = (uint16_t)nd->nuclData[nd->numNucl].Z;
						}
						if(nd->nuclData[nd->numNucl].Z < nd->minZforN[nd->nuclData[nd->numNucl].N]){
							if(nd->nuclData[nd->numNucl].Z >= 0){
								nd->minZforN[nd->nuclData[nd->numNucl].N] = (uint16_t)nd->nuclData[nd->numNucl].Z;
							}
						}
						if(nd->nuclData[nd->numNucl].N > nd->maxN){
							nd->maxN = (uint16_t)nd->nuclData[nd->numNucl].N;
						}
						if(nd->nuclData[nd->numNucl].Z > nd->maxZ){
							nd->maxZ = (uint16_t)nd->nuclData[nd->numNucl].Z;
						}
					}
				}
			}
			/*//parse the nucleus name
			char nbuff[7];
			memcpy(nbuff, &line[0], 6);
			nbuff[6] = '\0';*/

			//parse the line type
			char typebuff[4];
			memcpy(typebuff, &line[5], 3);
			typebuff[3] = '\0';

			//parse the energy
			char ebuff[11];
			memcpy(ebuff, &line[9], 10);
			ebuff[10] = '\0';

			//add levels
			if(nd->numNucl>=0){ //check that indices are valid
				if(subSec==0){ //adopted levels subsection

					if(nd->numLvls<MAXNUMLVLS){
						if(strcmp(typebuff,"  L")==0){

							float levelE = -1.0f;

							//get length without trailing spaces
							uint8_t levEStrLen = 10;
							for(int i=9;i>=0;i--){
								if(isspace(ebuff[i])){
									levEStrLen=(uint8_t)i;
								}else{
									break;
								}
							}
							//get the position of the first non-space character
							uint8_t levEStartPos = 10;
							for(uint8_t i=0;i<10;i++){
								if(!(isspace(ebuff[i]))){
									levEStartPos = i;
									break;
								}
							}

							//check for variables in level energy
							if(isalpha(ebuff[levEStrLen-1])&&((levEStrLen==1) || ebuff[levEStrLen-2]==' ')){
								//SDL_Log("X ebuff: %s\n",ebuff);
								nd->nuclData[nd->numNucl].numLevels++;
								nd->numLvls++;
								
								nd->levels[nd->numLvls-1].energy.val=0;
								nd->levels[nd->numLvls-1].energy.err=0;
								nd->levels[nd->numLvls-1].energy.unit=VALUE_UNIT_NOVAL;
								nd->levels[nd->numLvls-1].energy.format = 0; //default
								nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(VALUETYPE_X << 5);
								//record variable index (stored value = variable ASCII code)
								nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(ebuff[levEStrLen-1] << 9);
								
							}else if((levEStartPos < 10)&&(isalpha(ebuff[levEStartPos]))&&(ebuff[levEStartPos+1]=='+')){
								//level energy in X+number format
								//SDL_Log("X+number ebuff: %s\n",ebuff);
								tok = strtok(ebuff,"+");
								if(tok != NULL){
									tok = strtok(NULL,""); //get the rest of the string
									if(tok != NULL){
										levelE = (float)atof(tok);
										nd->nuclData[nd->numNucl].numLevels++;
										nd->numLvls++;
										nd->levels[nd->numLvls-1].energy.format = 0; //default
										nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(VALUETYPE_PLUSX << 5);
									}
								}
								memcpy(ebuff, &line[9], 10); //re-constitute original buffer
								ebuff[10] = '\0';
								nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(ebuff[levEStartPos] << 9);
							}else if((levEStrLen > 1)&&(ebuff[levEStrLen-2]=='+')&&(isalpha(ebuff[levEStrLen-1]))){
								//level energy in number+X format
								//SDL_Log("number+X ebuff: %s\n",ebuff);
								tok = strtok(ebuff,"+");
								if(tok != NULL){
									levelE = (float)atof(tok);
									nd->nuclData[nd->numNucl].numLevels++;
									nd->numLvls++;
									tok = strtok(NULL,""); //get the rest of the string
									if(tok != NULL){
										nd->levels[nd->numLvls-1].energy.format = 0; //default
										nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(VALUETYPE_PLUSX << 5);
										nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(tok[0] << 9);
										//SDL_Log("variable: %c\n",tok[0]);
									}
								}
								memcpy(ebuff, &line[9], 10); //re-constitute original buffer
								ebuff[10] = '\0';
							}else{
								//normal level energy
								//SDL_Log("normal ebuff: %s (length %u)\n",ebuff,levEStrLen);
								levelE = (float)atof(ebuff);
								nd->nuclData[nd->numNucl].numLevels++;
								nd->numLvls++;
								nd->levels[nd->numLvls-1].energy.format = 0; //default
								//SDL_Log("Found level at %f keV from string: %s\n",(double)levelE,ebuff);
							}

							if(nd->nuclData[nd->numNucl].numLevels == 1){
								nd->nuclData[nd->numNucl].firstLevel = nd->numLvls-1;
							}

							if(levelE >= 0.0f){
								//get the number of sig figs in the level energy
								//SDL_Log("ebuff: %s\n",ebuff);
								tok = strtok(ebuff,".");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									tok = strtok(NULL,"E+"); //some level energies are specified with exponents, or relative to a variable (eg. 73.0+X)
									if(tok!=NULL){
										
										//SDL_Log("%s\n",tok);
										uint16_t len = (uint16_t)strlen(tok);
										//check for trailing empty spaces
										for(uint16_t i=0;i<len;i++){
											if(isspace(tok[i])){
												len = i;
												break;
											}
										}
										nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(len & 15U);
										//SDL_Log("format: %u\n",nd->levels[nd->numLvls-1].energy.format);
										if(((nd->levels[nd->numLvls-1].energy.format >> 5U) & 15U) != VALUETYPE_PLUSX){
											tok = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
											if(tok!=NULL){
												//SDL_Log("energy in exponent form: %s\n",ebuff);
												//value was in exponent format
												nd->levels[nd->numLvls-1].energy.exponent = (int8_t)atoi(tok);
												levelE = levelE / powf(10.0f,(float)(nd->levels[nd->numLvls-1].energy.exponent));
												nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(1U << 4); //exponent flag
											}
										}
									}else{
										//potentially an exponent form value with no decimal place
										memcpy(ebuff, &line[9], 10); //re-copy buffer
										ebuff[10] = '\0';
										tok = strtok(ebuff,"E");
										//SDL_Log("ebuff: %s\n",ebuff);
										if(tok!=NULL){
											tok = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
											if(tok!=NULL){
												//SDL_Log("%s\n",tok);
												//value was in exponent format
												nd->levels[nd->numLvls-1].energy.exponent = (int8_t)atoi(tok);
												levelE = levelE / powf(10.0f,(float)(nd->levels[nd->numLvls-1].energy.exponent));
												nd->levels[nd->numLvls-1].energy.format |= (uint16_t)(1U << 4); //exponent flag
											}
										}
									}
								}
								if((levelE==0.0f)&&((nd->levels[nd->numLvls-1].energy.format & 15U) == 0)){
									nd->levels[nd->numLvls-1].energy.format |= 1U; //always include at least one decimal place for ground states, for aesthetic purposes
								}

								//parse the energy error
								char eeBuff[3];
								memcpy(eeBuff, &line[19], 2);
								eeBuff[2] = '\0';
								uint8_t levelEerr = (uint8_t)atoi(eeBuff);

								//assign level energy values
								nd->levels[nd->numLvls-1].energy.val=levelE;
								nd->levels[nd->numLvls-1].energy.err=levelEerr;
								nd->levels[nd->numLvls-1].energy.unit=VALUE_UNIT_KEV;
								
							}

							//parse and handle level properties not related to energy
							
							uint8_t halfInt = 0;
							if(((nd->nuclData[nd->numNucl].N + nd->nuclData[nd->numNucl].Z) % 2) != 0){
								halfInt = 1; //odd mass nucleus, half-integer spins
							}

							nd->levels[nd->numLvls-1].format |= halfInt;
							nd->levels[nd->numLvls-1].numDecModes = 0;
							nd->levels[nd->numLvls-1].firstDecMode = nd->numDecModes;
							//parse the level spin and parity
							char spbuff[16];
							memcpy(spbuff, &line[21], 15);
							spbuff[15] = '\0';
							//SDL_Log("%s\n",spbuff);
							parseSpinPar(&nd->levels[nd->numLvls-1],&varDat,spbuff);
							//parse the half-life information
							char hlBuff[18];
							memcpy(hlBuff, &line[39], 17);
							hlBuff[17] = '\0';
							//SDL_Log("%s\n",hlBuff);
							parseHalfLife(&nd->levels[nd->numLvls-1],hlBuff);
							//check isomerism
							uint8_t eValueType = (uint8_t)((nd->levels[nd->numLvls-1].energy.format >> 5U) & 15U);
							double en = getLevelEnergykeV(nd,nd->numLvls-1);
							if((en > 0.0)||((nd->nuclData[nd->numNucl].numLevels > 1) && (eValueType == VALUETYPE_PLUSX))){
								uint8_t hlValueType = (uint8_t)((nd->levels[nd->numLvls-1].halfLife.format >> 5U) & 15U);
								if(!((hlValueType == VALUETYPE_LESSTHAN)||(hlValueType == VALUETYPE_LESSOREQUALTHAN))){
									double hl = getLevelHalfLifeSeconds(nd,nd->numLvls-1);
									if(hl >= 10.0E-9){
										//SDL_Log("E: %f, hl: %f\n",en,hl);
										if(hl >= 1E-3){
											isomerMValInNucl++; //m-values are somewhat informal but are generally only assigned for longer-lived isomers
											nd->nuclData[nd->numNucl].numIsomerMVals++;
										}
										if(hl > longestIsomerHl){
											//SDL_Log("Longest lived isomer found with m-val %u\n",isomerMValInNucl);
											longestIsomerHl = hl;
											nd->nuclData[nd->numNucl].longestIsomerLevel = nd->numLvls-1;
											nd->nuclData[nd->numNucl].longestIsomerMVal = isomerMValInNucl;
											
										}
									}else if((en < 0.02)&&(eValueType != VALUETYPE_PLUSX)&&(eValueType != VALUETYPE_X)){
										//low energy levels are generally isomers (even if their half-life is unknown)
										//229Th is a famous case
										//only include these if no other long-lived isomers are found
										isomerMValInNucl++;
										nd->nuclData[nd->numNucl].numIsomerMVals++;
										if(!(longestIsomerHl > 0.0)&&(hl <= longestIsomerHl)){
											longestIsomerHl = hl;
											nd->nuclData[nd->numNucl].longestIsomerLevel = nd->numLvls-1;
											nd->nuclData[nd->numNucl].longestIsomerMVal = isomerMValInNucl;
										}
									}
								}
							}

							//check whether the level is 'special'
							if((nd->nuclData[nd->numNucl].Z == 6)&&(nd->nuclData[nd->numNucl].N == 6)){
								//12C
								if((en > 7650.0)&&(en < 7660.0)){
									//Hoyle state
									//SDL_Log("Found Hoyle state.\n");
									nd->levels[nd->numLvls-1].format |= (uint8_t)(SPECIALLEVEL_HOYLE << 1U);
								}
							}else if((nd->nuclData[nd->numNucl].Z == 73)&&(nd->nuclData[nd->numNucl].N == 107)){
								//180Ta
								if((en > 77.0)&&(en < 78.0)){
									//Naturally occuring isomer in 180Ta
									nd->levels[nd->numLvls-1].format |= (uint8_t)(SPECIALLEVEL_NATURALLYOCCURINGISOMER << 1U);
								}
							}else if((nd->nuclData[nd->numNucl].Z == 90)&&(nd->nuclData[nd->numNucl].N == 139)){
								//229Th
								if((en > 0.000005)&&(en < 1.0)){
									//Nuclear clock isomer in 229Th
									nd->levels[nd->numLvls-1].format |= (uint8_t)(SPECIALLEVEL_CLOCKISOMER << 1U);
								}
							}
							
							
						}
					}

					//add decay modes
					if(nd->nuclData[nd->numNucl].numLevels>0){ //check that indices are valid
						//can parse multiple 'L' lines, but only one 'cL' (comment) line
						//logic here is that if there are many known decay modes, they will be
						//listed across several 'L' lines, but if decay modes are tentative, 
						//they are usually listed per-measurement in 'cL' lines (so they can 
						//repeat if there are multiple measurements), with the best values
						//being at the top
						if((strcmp(typebuff+1," L")==0)||((strcmp(typebuff+1,"cL")==0)&&(decModeLineParsed == 0))){
							//parse decay mode info
							//search for first decay string
							//SDL_Log("%s\n",line);
							uint8_t decStrStart = 9;
							for(uint8_t i=9;i<70;i++){
								if(line[i]=='%'){
									decStrStart = i;
									break;
								}
							}
							//SDL_Log("decStrStart: %u\n",decStrStart);
							if(line[decStrStart]=='%'){
								//line contains decay mode info
								//SDL_Log("dec mode found: %s\n",line);
								char dmBuffOrig[128], dmBuff[128];
								memcpy(dmBuffOrig, &line[decStrStart], 127-decStrStart);
								dmBuffOrig[127-decStrStart] = '\0';
								//SDL_Log("Original decay mode buffer: %s\n",dmBuffOrig);
								tok = strtok(dmBuffOrig,"$,;");
								while(tok!=NULL){
									//SDL_Log("tok: %s\n",tok);
									strcpy(dmBuff,tok);
									if(parseDcyModeSubstr(nd,nd->numDecModes,dmBuff)==1){
										decModeLineParsed = 1;
										nd->levels[nd->numLvls-1].numDecModes++;
										nd->numDecModes++;
										if(nd->numDecModes > MAXNUMDECAYMODES){
											SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Maximum number of decay modes (%i) exceeded!\n",MAXNUMDECAYMODES);
											return -1;
										}
										//reconstitute buffer
										memcpy(dmBuffOrig, &line[decStrStart], 127-decStrStart);
										dmBuffOrig[127-decStrStart] = '\0';
										//SDL_Log("Decay mode buffer after: %s\n",dmBuffOrig);
										tok = strtok(dmBuffOrig,"$,;");
										if(tok!=NULL){
											for(uint8_t i=0;i<nd->levels[nd->numLvls-1].numDecModes;i++){
												if(tok!=NULL){
													tok = strtok(NULL,"$,;");
												}else{
													break;
												}
											}
										}
										//SDL_Log("tok after: %s\n",tok);
										if((tok==NULL)||(isEmpty(tok))){
											break; //no need to process empty substring at end
										}
									}else{
										break;
									}
								}
								//getc(stdin);
							}
						}
					}

					//SDL_Log("line: %s\n",line);
					if(strcmp(typebuff+1,"cQ")==0){
						//some GS decays are only specified as Q-values
						//before any of the other level info
						//if there are no other decay modes specified,
						//but a Q-value is present, assume 100% branching
						//to that decay mode
						if(nd->nuclData[nd->numNucl].numLevels == 0){
							//SDL_Log("line: %s\n",line);
							char dmBuff[128];
							memcpy(dmBuff, &line[9], 118);
							dmBuff[118] = '\0';
							//SDL_Log("dmBuff: %s\n",dmBuff);
							tok = strtok(dmBuff," =");
							if(tok!=NULL){
								//SDL_Log("tok: %s\n",tok);
								if(strcmp(tok,"$Q(2|b{+-})")==0){
									tok = strtok(NULL," ;");
									if(tok!=NULL){
										if(atof(tok) > 0.0){
											//positive Q-value
											qValDecModeFlag = 1;
											qValDecModeType = DECAYMODE_2BETAMINUS;
										}
									}
								}else if(strcmp(tok,"$Q(2|b{++})")==0){
									tok = strtok(NULL," ;");
									if(tok!=NULL){
										if(atof(tok) > 0.0){
											//positive Q-value
											qValDecModeFlag = 1;
											qValDecModeType = DECAYMODE_2BETAPLUS;
										}
									}
								}
							}
						}
					}

					//add gamma rays
					if(nd->nuclData[nd->numNucl].numLevels>0){ //check that indices are valid
						if(nd->levels[nd->numLvls-1].numTran<MAXGAMMASPERLEVEL){
							if(strcmp(typebuff,"  G")==0){
								//SDL_Log("%s\n",line);
								if(nd->levels[nd->numLvls-1].numTran == 0){
									nd->levels[nd->numLvls-1].firstTran = nd->numTran;
								}

								//parse the gamma intensity
								char iBuff[9];
								memcpy(iBuff, &line[21], 8);
								iBuff[8] = '\0';
								//parse the gamma intensity error
								char ieBuff[3];
								memcpy(ieBuff, &line[29], 2);
								ieBuff[2] = '\0';
								//parse the gamma multipolarity
								char mBuff[11];
								memcpy(mBuff, &line[31], 10);
								mBuff[10] = '\0';
								//parse the energy error
								char eeBuff[3];
								memcpy(eeBuff, &line[19], 2);
								eeBuff[2] = '\0';
								uint8_t gammaEerr = (uint8_t)atoi(eeBuff);
								
								uint32_t tranInd = nd->levels[nd->numLvls-1].firstTran + (uint32_t)(nd->levels[nd->numLvls-1].numTran);

								//process gamma energy

								//get length without trailing spaces
								uint8_t gamEStrLen = 10;
								for(int i=9;i>=0;i--){
									if(isspace(ebuff[i])){
										gamEStrLen=(uint8_t)i;
									}else{
										break;
									}
								}
								//get the position of the first non-space character
								uint8_t gamEStartPos = 10;
								for(uint8_t i=0;i<10;i++){
									if(!(isspace(ebuff[i]))){
										gamEStartPos = i;
										break;
									}
								}

								//check for variables in gamma energy
								float gammaE = 0.0f;
								if(isalpha(ebuff[gamEStrLen-1])&&((gamEStrLen==1) || ebuff[gamEStrLen-2]==' ')){
									//SDL_Log("X ebuff: %s\n",ebuff);
									nd->tran[tranInd].energy.val=0;
									nd->tran[tranInd].energy.err=0;
									nd->tran[tranInd].energy.unit=VALUE_UNIT_NOVAL;
									nd->tran[tranInd].energy.format = 0; //default
									nd->tran[tranInd].energy.format |= (uint16_t)(VALUETYPE_X << 5);
									//record variable index (stored value = variable ASCII code)
									nd->tran[tranInd].energy.format |= (uint16_t)(ebuff[gamEStrLen-1] << 9);
								}else if((gamEStartPos < 10)&&(isalpha(ebuff[gamEStartPos]))&&(ebuff[gamEStartPos+1]=='+')){
									//gamma energy in X+number format
									//SDL_Log("X+number ebuff: %s\n",ebuff);
									tok = strtok(ebuff,"+");
									if(tok != NULL){
										tok = strtok(NULL,""); //get the rest of the string
										if(tok != NULL){
											gammaE = (float)atof(tok);
											nd->tran[tranInd].energy.format = 0; //default
											nd->tran[tranInd].energy.format |= (uint16_t)(VALUETYPE_PLUSX << 5);
										}
									}
									memcpy(ebuff, &line[9], 10); //re-constitute original buffer
									ebuff[10] = '\0';
									nd->tran[tranInd].energy.format |= (uint16_t)(ebuff[gamEStartPos] << 9);
								}else if((gamEStrLen > 1)&&(ebuff[gamEStrLen-2]=='+')&&(isalpha(ebuff[gamEStrLen-1]))){
									//gamma energy in number+X format
									//SDL_Log("number+X ebuff: %s\n",ebuff);
									tok = strtok(ebuff,"+");
									if(tok != NULL){
										gammaE = (float)atof(tok);
										tok = strtok(NULL,""); //get the rest of the string
										if(tok != NULL){
											nd->tran[tranInd].energy.format = 0; //default
											nd->tran[tranInd].energy.format |= (uint16_t)(VALUETYPE_PLUSX << 5);
											nd->tran[tranInd].energy.format |= (uint16_t)(tok[0] << 9);
											//SDL_Log("variable: %c\n",tok[0]);
										}
									}
									memcpy(ebuff, &line[9], 10); //re-constitute original buffer
									ebuff[10] = '\0';
								}else{
									//normal gamma energy
									//SDL_Log("normal ebuff: %s (length %u)\n",ebuff,gamEStrLen);
									gammaE = (float)atof(ebuff);
									nd->tran[tranInd].energy.format = 0; //default
									//SDL_Log("Found gamma at %f keV from string: %s\n",(double)gammaE,ebuff);
								}


								//get the number of sig figs
								//SDL_Log("ebuff: %s\n",ebuff);
								tok = strtok(ebuff,".");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									tok = strtok(NULL,"E"); //some gamma energies are specified with exponents
									if(tok!=NULL){
										//SDL_Log("%s\n",tok);
										uint16_t len = (uint16_t)strlen(tok);
										//check for trailing empty spaces
										for(uint16_t i=0;i<len;i++){
											if(isspace(tok[i])){
												len = i;
												break;
											}
										}
										nd->tran[tranInd].energy.format |= (uint16_t)(len & 15U);
										//SDL_Log("format: %u\n",nd->tran[tranInd].energy.format);
										if(((nd->tran[tranInd].energy.format >> 5U) & 15U) != VALUETYPE_PLUSX){
											tok = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
											if(tok!=NULL){
												//SDL_Log("energy in exponent form: %s\n",ebuff);
												//value was in exponent format
												nd->tran[tranInd].energy.exponent = (int8_t)atoi(tok);
												gammaE = gammaE / powf(10.0f,(float)(nd->tran[tranInd].energy.exponent));
												nd->tran[tranInd].energy.format |= (uint16_t)(1U << 4); //exponent flag
											}
										}
									}
								}else{
									//potentially an exponent form value with no decimal place
									memcpy(ebuff, &line[9], 10); //re-copy buffer
									ebuff[10] = '\0';
									tok = strtok(ebuff,"E");
									//SDL_Log("ebuff: %s\n",ebuff);
									if(tok!=NULL){
										tok = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
										if(tok!=NULL){
											//SDL_Log("%s\n",tok);
											//value was in exponent format
											nd->tran[tranInd].energy.exponent = (int8_t)atoi(tok);
											gammaE = gammaE / powf(10.0f,(float)(nd->tran[tranInd].energy.exponent));
											nd->tran[tranInd].energy.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}

								nd->tran[tranInd].energy.val=gammaE;
								nd->tran[tranInd].energy.err=gammaEerr;
								if(nd->tran[tranInd].energy.unit != VALUE_UNIT_NOVAL){
									nd->tran[tranInd].energy.unit=VALUE_UNIT_KEV;
								}
								
								//check for final level of transition
								double minEDiff = 1000.0;
								nd->tran[tranInd].finalLvlOffset = 0;
								uint8_t lvlValType = ((nd->levels[nd->numLvls-1].energy.format >> 5U) & 15U);
								uint8_t gammaValType = ((nd->tran[tranInd].energy.format >> 5U) & 15U);
								//SDL_Log("lvl type: %u, gamma type: %u\n",lvlValType,gammaValType);
								//SDL_Log("Lvl E: %f, gamma E: %f\n",getRawValFromDB(&nd->levels[nd->numLvls-1].energy),getRawValFromDB(&nd->tran[tranInd].energy));
								if(nd->numLvls >= 2){
									for(uint32_t lvlInd = (nd->numLvls-2); lvlInd >= nd->nuclData[nd->numNucl].firstLevel; lvlInd--){
										
										//SDL_Log("Final lvl E: %f\n",getRawValFromDB(&nd->levels[lvlInd].energy));

										if((gammaValType == VALUETYPE_X)&&(lvlValType == VALUETYPE_PLUSX)){
											//handle special case where gamma energy is variable and defines a level
											//offset from a previous level
											double eDiff = fabs(getRawValFromDB(&nd->levels[lvlInd].energy) - getRawValFromDB(&nd->levels[nd->numLvls-1].energy));
											if(eDiff < minEDiff){
												minEDiff = eDiff;
												nd->tran[tranInd].finalLvlOffset = (uint8_t)((nd->numLvls-1) - lvlInd);
											}
											//SDL_Log("finalLvlOffset: %u\n",nd->tran[tranInd].finalLvlOffset);
											continue; //don't evaluate other conditions that don't correspond to this special case
										}
										
										//handle variable level energies (ie. number+X, Y+number...)
										//transitions cannot link between levels defined by different variables
										uint8_t prevLvlValType = ((nd->levels[lvlInd].energy.format >> 5U) & 15U);
										if((lvlValType == VALUETYPE_X)||(lvlValType == VALUETYPE_PLUSX)){
											if((prevLvlValType != VALUETYPE_X)&&(prevLvlValType != VALUETYPE_PLUSX)){
												continue; //skip
											}else{
												//check that the variables are the same
												uint8_t var = (uint8_t)((nd->levels[nd->numLvls-1].energy.format >> 9U) & 127U);
												uint8_t prevVar = (uint8_t)((nd->levels[lvlInd].energy.format >> 9U) & 127U);
												if(var != prevVar){
													continue; //variables don't match, skip
												}
											}
										}else{
											if((prevLvlValType == VALUETYPE_X)||(prevLvlValType == VALUETYPE_PLUSX)){
												continue; //skip
											}
										}
										
									
										double fudgeFactor = SDL_sqrt(pow(getRawErrFromDB(&nd->tran[tranInd].energy),2.0) + pow(getRawErrFromDB(&nd->levels[lvlInd].energy),2.0) + pow(getRawErrFromDB(&nd->levels[nd->numLvls-1].energy),2.0));
										fudgeFactor += pow(getRawValFromDB(&nd->tran[tranInd].energy),2.0)/(2.0E6*(nd->nuclData[nd->numNucl].Z + nd->nuclData[nd->numNucl].N)); //nuclear recoil energy approximation
										if(fudgeFactor < 0.01){
											fudgeFactor = 1.0; //default assumed energy resolution, when no error is reported 
										}
										//SDL_Log("Fudge factor: %f\n",fudgeFactor);
										double eDiff = fabs(getRawValFromDB(&nd->levels[lvlInd].energy) - (getRawValFromDB(&nd->levels[nd->numLvls-1].energy) - getRawValFromDB(&nd->tran[tranInd].energy)));
										if(eDiff <= fudgeFactor){
											if(eDiff < minEDiff){
												minEDiff = eDiff;
												nd->tran[tranInd].finalLvlOffset = (uint8_t)((nd->numLvls-1) - lvlInd);
											}
											//SDL_Log("finalLvlOffset: %u\n",nd->tran[tranInd].finalLvlOffset);
										}
										
										if(lvlInd == 0){
											break; //handle rare integer overflow case
										}
									}

									//if a final level wasn't found with the usual method, 
									//try to simply look for a final level within a few keV
									//without taking the error bars into account	
									if(nd->tran[tranInd].finalLvlOffset == 0){
										//no final level was found yet
										if((lvlValType == VALUETYPE_NUMBER)&&(gammaValType == VALUETYPE_NUMBER)){
											minEDiff = 1.5;
											for(uint32_t lvlInd = (nd->numLvls-2); lvlInd >= nd->nuclData[nd->numNucl].firstLevel; lvlInd--){
												double eDiff = fabs(getRawValFromDB(&nd->levels[lvlInd].energy) - (getRawValFromDB(&nd->levels[nd->numLvls-1].energy) - getRawValFromDB(&nd->tran[tranInd].energy)));
												if(eDiff < minEDiff){
													minEDiff = eDiff;
													nd->tran[tranInd].finalLvlOffset = (uint8_t)((nd->numLvls-1) - lvlInd);
												}
												//SDL_Log("finalLvlOffset: %u\n",nd->tran[tranInd].finalLvlOffset);
											}
										}
									}
								}
								
								//gamma intensity
								float gammaI = (float)atof(iBuff);
								//get the number of sig figs
								nd->tran[tranInd].intensity.format = 0; //default
								//SDL_Log("ebuff: %s\n",ebuff);
								tok = strtok(iBuff,".");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									tok = strtok(NULL,"E"); //some gamma energies are specified with exponents
									if(tok!=NULL){
										//SDL_Log("%s\n",tok);
										nd->tran[tranInd].intensity.format = (uint16_t)strlen(tok);
										//check for trailing empty spaces
										for(uint8_t i=0;i<nd->tran[tranInd].intensity.format;i++){
											if(isspace(tok[i])){
												nd->tran[tranInd].intensity.format = i;
												break;
											}
										}
										if(nd->tran[tranInd].intensity.format > 15U){
											nd->tran[tranInd].intensity.format = 15U; //only 4 bits available for precision
										}
										//SDL_Log("format: %u\n",nd->tran[tranInd].intensity.format);
										tok = strtok(NULL,""); //get the remaining part of the string (only get past here if the value was expressed in exponent form)
										if(tok!=NULL){
											//SDL_Log("energy in exponent form: %s\n",ebuff);
											//value was in exponent format
											nd->tran[tranInd].intensity.exponent = (int8_t)atoi(tok);
											gammaI = gammaI / powf(10.0f,(float)(nd->tran[tranInd].intensity.exponent));
											nd->tran[tranInd].intensity.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}

								//gamma intensity: check for special value type
								nd->tran[tranInd].intensity.err=0;
								tok = strtok(ieBuff, " ");
								if(tok!=NULL){
									if(strcmp(tok,"GT")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_GREATERTHAN << 5);
									}else if(strcmp(tok,"GT")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_GREATERTHAN << 5);
									}else if(strcmp(tok,"GE")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_GREATEROREQUALTHAN << 5);
									}else if(strcmp(tok,"LT")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_LESSTHAN << 5);
									}else if(strcmp(tok,"LE")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_LESSOREQUALTHAN << 5);
									}else if(strcmp(tok,"AP")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_APPROX << 5);
									}else if(strcmp(tok,"?")==0){
										nd->tran[tranInd].intensity.format |= (uint16_t)(VALUETYPE_UNKNOWN << 5);
									}else{
										nd->tran[tranInd].intensity.err=(uint8_t)atoi(ieBuff);
									}
								}

								//gamma multipolarity
								nd->tran[tranInd].numMultipoles = 0;
								tok = strtok(mBuff," ");
								if(tok != NULL){
									uint8_t tentative = 0;
									uint8_t derived = 0;
									//SDL_Log("%s\n",tok);
									nd->tran[tranInd].multipole[0] = 0; //default
									for(int i=0;i<(int)strlen(tok);i++){
										if(nd->tran[tranInd].numMultipoles >= MAXMULTPERLEVEL){
											SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"parseENSDFFile - too many multipoles in string: %s\n",tok);
											return -1;
										}
										if(tok[i]=='('){
											tentative = 1;
											if(i == 0){
												nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(TENTATIVEMULT_YES << 5);
											}
										}else if(tok[i]==')'){
											tentative = 0;
										}else if(tok[i]=='['){
											derived = 1;
											if(i == 0){
												nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(TENTATIVEMULT_DERIVED << 5U);
											}
										}else if(tok[i]==']'){
											derived = 0;
										}else if((tok[i]=='+')||(tok[i]==',')){
											nd->tran[tranInd].numMultipoles++;
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] = 0; //default
											if(tentative){
												nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(TENTATIVEMULT_YES << 5);
											}else if(derived){
												nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(TENTATIVEMULT_DERIVED << 5);
											}else{
												nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(TENTATIVEMULT_NONE << 5);
											}
										}else if(tok[i]=='E'){
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] &= (uint8_t)(~(uint8_t)(1U)); //unset
										}else if(tok[i]=='M'){
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(1U);
										}else if(tok[i]=='D'){
											//set M1 as placeholder value
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(1U);
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)((uint8_t)(1U) << 1);
											//specify that assignment is 'D'
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(1U << 7);
										}else if(tok[i]=='Q'){
											//set E2 as placeholder value
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] &= (uint8_t)(~(uint8_t)(1U)); //unset
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)((uint8_t)(2U) << 1);
											//specify that assignment is 'Q'
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(1U << 7);
										}else if(isdigit(tok[i])){
											//set multipole order
											nd->tran[tranInd].multipole[nd->tran[tranInd].numMultipoles] |= (uint8_t)(((uint8_t)(tok[i] - '0') & 15U) << 1);
										}
									}
									nd->tran[tranInd].numMultipoles++;
									//SDL_Log("Number of multipoles: %u\n",nd->tran[tranInd].numMultipoles);
								}else{
									//recopy buffer
									memcpy(mBuff, &line[31], 10);
									mBuff[10] = '\0';
									if(!isspace(mBuff[0])){
										//string with no preceding or trailing spaces
										SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Unhandled multipolarity string: %s\n",mBuff);
									}
								}

								nd->tran[tranInd].intensity.val=gammaI;
								nd->levels[nd->numLvls-1].numTran++;
								nd->numTran++;
									
							}
						}
					}

					//add Q-values and separation energies
					if(strcmp(typebuff,"  Q")==0){
						if(firstQLine==1){
							//parse the beta Q-value
							char qbBuff[11];
							memcpy(qbBuff, &line[9], 10);
							for(uint8_t i=1;i<10;i++){
								if((isspace(qbBuff[i])) && (!(isspace(qbBuff[i-1])))){
									qbBuff[i] = '\0'; //terminate string at first trailing space
								}else if(i==9){
									qbBuff[10] = '\0'; //terminate string at end
								}
							}
							char qbErrBuff[3];
							memcpy(qbErrBuff, &line[19], 2);
							qbErrBuff[2] = '\0';

							nd->nuclData[nd->numNucl].qbeta.val = (float)atof(qbBuff);
							if(strcmp(qbErrBuff,"SY")==0){
								nd->nuclData[nd->numNucl].qbeta.err = 255; //systematic
							}else{
								nd->nuclData[nd->numNucl].qbeta.err = (uint8_t)atoi(qbErrBuff);
							}
							nd->nuclData[nd->numNucl].qbeta.unit = VALUE_UNIT_KEV;

							//handle expoenents
							tok = strtok(qbBuff,".");
							if(tok!=NULL){
								//SDL_Log("%s\n",tok);
								tok = strtok(NULL,"E+");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									nd->nuclData[nd->numNucl].qbeta.format = (uint16_t)strlen(tok);
									if(nd->nuclData[nd->numNucl].qbeta.format > 15U){
										nd->nuclData[nd->numNucl].qbeta.format = 15U; //only 4 bits available for precision
									}
									tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
									if(tok!=NULL){
										//value was in exponent format
										nd->nuclData[nd->numNucl].qbeta.exponent = (int8_t)atoi(tok);
										//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].qbeta.exponent);
										nd->nuclData[nd->numNucl].qbeta.val = nd->nuclData[nd->numNucl].qbeta.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].qbeta.exponent));
										nd->nuclData[nd->numNucl].qbeta.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}else{
									tok = strtok(qbBuff,"E");
									if(tok!=NULL){
										tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
										if(tok!=NULL){
											//value was in exponent format
											nd->nuclData[nd->numNucl].qbeta.exponent = (int8_t)atoi(tok);
											//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].qbeta.exponent);
											nd->nuclData[nd->numNucl].qbeta.val = nd->nuclData[nd->numNucl].qbeta.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].qbeta.exponent));
											nd->nuclData[nd->numNucl].qbeta.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}
							}

							//parse the neutron sep energy
							char nsBuff[9];
							memcpy(nsBuff, &line[21], 8);
							for(uint8_t i=1;i<8;i++){
								if((isspace(nsBuff[i])) && (!(isspace(nsBuff[i-1])))){
									nsBuff[i] = '\0'; //terminate string at first trailing space
								}else if(i==7){
									nsBuff[8] = '\0'; //terminate string at end
								}
							}
							char nsErrBuff[3];
							memcpy(nsErrBuff, &line[29], 2);
							nsErrBuff[2] = '\0';

							nd->nuclData[nd->numNucl].sn.val = (float)atof(nsBuff);
							if(strcmp(nsErrBuff,"SY")==0){
								nd->nuclData[nd->numNucl].sn.err = 255; //systematic
							}else{
								nd->nuclData[nd->numNucl].sn.err = (uint8_t)atoi(nsErrBuff);
							}
							nd->nuclData[nd->numNucl].sn.unit = VALUE_UNIT_KEV;

							//handle expoenents
							tok = strtok(nsBuff,".");
							if(tok!=NULL){
								//SDL_Log("%s\n",tok);
								tok = strtok(NULL,"E+");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									nd->nuclData[nd->numNucl].sn.format = (uint16_t)strlen(tok);
									if(nd->nuclData[nd->numNucl].sn.format > 15U){
										nd->nuclData[nd->numNucl].sn.format = 15U; //only 4 bits available for precision
									}
									tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
									if(tok!=NULL){
										//value was in exponent format
										nd->nuclData[nd->numNucl].sn.exponent = (int8_t)atoi(tok);
										//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].sn.exponent);
										nd->nuclData[nd->numNucl].sn.val = nd->nuclData[nd->numNucl].sn.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].sn.exponent));
										nd->nuclData[nd->numNucl].sn.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}else{
									tok = strtok(nsBuff,"E");
									if(tok!=NULL){
										tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
										if(tok!=NULL){
											//value was in exponent format
											nd->nuclData[nd->numNucl].sn.exponent = (int8_t)atoi(tok);
											//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].sn.exponent);
											nd->nuclData[nd->numNucl].sn.val = nd->nuclData[nd->numNucl].sn.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].sn.exponent));
											nd->nuclData[nd->numNucl].sn.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}
							}

							//parse the proton sep energy
							char psBuff[9];
							memcpy(psBuff, &line[31], 8);
							for(uint8_t i=1;i<8;i++){
								if((isspace(psBuff[i])) && (!(isspace(psBuff[i-1])))){
									psBuff[i] = '\0'; //terminate string at first trailing space
								}else if(i==7){
									psBuff[8] = '\0'; //terminate string at end
								}
							}
							char psErrBuff[3];
							memcpy(psErrBuff, &line[39], 2);
							psErrBuff[2] = '\0';

							nd->nuclData[nd->numNucl].sp.val = (float)atof(psBuff);
							if(strcmp(psErrBuff,"SY")==0){
								nd->nuclData[nd->numNucl].sp.err = 255; //systematic
							}else{
								nd->nuclData[nd->numNucl].sp.err = (uint8_t)atoi(psErrBuff);
							}
							nd->nuclData[nd->numNucl].sp.unit = VALUE_UNIT_KEV;

							//handle expoenents
							tok = strtok(psBuff,".");
							if(tok!=NULL){
								//SDL_Log("%s\n",tok);
								tok = strtok(NULL,"E+");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									nd->nuclData[nd->numNucl].sp.format = (uint16_t)strlen(tok);
									if(nd->nuclData[nd->numNucl].sp.format > 15U){
										nd->nuclData[nd->numNucl].sp.format = 15U; //only 4 bits available for precision
									}
									tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
									if(tok!=NULL){
										//value was in exponent format
										nd->nuclData[nd->numNucl].sp.exponent = (int8_t)atoi(tok);
										//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].sp.exponent);
										nd->nuclData[nd->numNucl].sp.val = nd->nuclData[nd->numNucl].sp.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].sp.exponent));
										nd->nuclData[nd->numNucl].sp.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}else{
									tok = strtok(psBuff,"E");
									if(tok!=NULL){
										tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
										if(tok!=NULL){
											//value was in exponent format
											nd->nuclData[nd->numNucl].sp.exponent = (int8_t)atoi(tok);
											//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].sp.exponent);
											nd->nuclData[nd->numNucl].sp.val = nd->nuclData[nd->numNucl].sp.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].sp.exponent));
											nd->nuclData[nd->numNucl].sp.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}
							}

							//parse the alpha Q-value
							char qaBuff[9];
							memcpy(qaBuff, &line[41], 8);
							for(uint8_t i=1;i<8;i++){
								if((isspace(qaBuff[i])) && (!(isspace(qaBuff[i-1])))){
									qaBuff[i] = '\0'; //terminate string at first trailing space
								}else if(i==7){
									qaBuff[8] = '\0'; //terminate string at end
								}
							}
							char qaErrBuff[9];
							memcpy(qaErrBuff, &line[49], 2);
							qaErrBuff[2] = '\0';
							
							nd->nuclData[nd->numNucl].qalpha.val = (float)atof(qaBuff);
							if(strcmp(qaErrBuff,"SY")==0){
								nd->nuclData[nd->numNucl].qalpha.err = 255; //systematic
							}else{
								nd->nuclData[nd->numNucl].qalpha.err = (uint8_t)atoi(qaErrBuff);
							}
							nd->nuclData[nd->numNucl].qalpha.unit = VALUE_UNIT_KEV;

							//handle expoenents
							tok = strtok(qaBuff,".");
							if(tok!=NULL){
								//SDL_Log("%s\n",tok);
								tok = strtok(NULL,"E+");
								if(tok!=NULL){
									//SDL_Log("%s\n",tok);
									nd->nuclData[nd->numNucl].qalpha.format = (uint16_t)strlen(tok);
									if(nd->nuclData[nd->numNucl].qalpha.format > 15U){
										nd->nuclData[nd->numNucl].qalpha.format = 15U; //only 4 bits available for precision
									}
									tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
									if(tok!=NULL){
										//value was in exponent format
										nd->nuclData[nd->numNucl].qalpha.exponent = (int8_t)atoi(tok);
										//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].qalpha.exponent);
										nd->nuclData[nd->numNucl].qalpha.val = nd->nuclData[nd->numNucl].qalpha.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].qalpha.exponent));
										nd->nuclData[nd->numNucl].qalpha.format |= (uint16_t)(1U << 4); //exponent flag
									}
								}else{
									tok = strtok(qaBuff,"E");
									if(tok!=NULL){
										tok = strtok(NULL,""); //get the rest of the string (the part after the exponent, if it exists)
										if(tok!=NULL){
											//value was in exponent format
											nd->nuclData[nd->numNucl].qalpha.exponent = (int8_t)atoi(tok);
											//SDL_Log("%s, parsed to %i\n",tok,nd->nuclData[nd->numNucl].qalpha.exponent);
											nd->nuclData[nd->numNucl].qalpha.val = nd->nuclData[nd->numNucl].qalpha.val / powf(10.0f,(float)(nd->nuclData[nd->numNucl].qalpha.exponent));
											nd->nuclData[nd->numNucl].qalpha.format |= (uint16_t)(1U << 4); //exponent flag
										}
									}
								}
							}

						}
						firstQLine = 0;
					}
				}else if(subSec > 0){
					//reaction subsection
					//reaction hasn't been parsed yet
					if(startedParsingSec == 0){
						//SDL_Log("numRxnsParsed: %u, subSec: %u\n",numRxnsParsed,subSec);
						startedParsingSec=1;
						char rxnBuff[31];
						memcpy(rxnBuff, &line[9], 30);
						for(uint8_t i=29; 1; i--){
							if(!(isspace(rxnBuff[i]))){
								rxnBuff[i+1] = '\0'; //terminate string at end, without trailing whitespace
								break;
							}
						}
						if(parseRxn(&nd->rxn[nd->numRxns],rxnBuff)==1){
							numRxnsParsed++;
							nd->numRxns++;
						}
					}
					
				}
			}

		}
	}
	//handle any business arising from the last nuclide
	if(qValDecModeFlag){
		if(nd->levels[nd->nuclData[nd->numNucl].firstLevel].numDecModes == 0){
			nd->levels[nd->nuclData[nd->numNucl].firstLevel].numDecModes = 1;
			nd->levels[nd->nuclData[nd->numNucl].firstLevel].firstDecMode = nd->numDecModes;
			nd->dcyMode[nd->numDecModes].type = qValDecModeType;
			nd->dcyMode[nd->numDecModes].prob.val = 100.0f;
			nd->dcyMode[nd->numDecModes].prob.err = 0;
			nd->dcyMode[nd->numDecModes].prob.format = 0;
		}
	}
	fclose(efile);
	
	if(nd->numNucl>=MAXNUMNUCL){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Attempted to import data for too many nuclei.  Increase the value of MAXNUMNUCL in levelup.h\n");
		return -1;
	}
	
  return 0;
}

//function to parse isotopic abundance data file
//assumes ENSDF data has already been parsed
int parseAbundanceData(const char * filePath, ndata * nd){

  FILE *afile;
  char *tok;
  char str[256];//string to be read from file (will be tokenized)
  char line[256],val[MAXNUMPARSERVALS][256],tmpVal[256];
  int tokPos;//position when tokenizing
  
  int16_t Z = 0;
	int16_t A = 0;
	int16_t N = 0;
  
  //open the file and read all parameters
  if((afile=fopen(filePath,"r"))==NULL){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Cannot open the abundance data file %s\n",filePath);
		return -1;
	}
  while(!(feof(afile))){ //go until the end of file is reached

		if(fgets(str,256,afile)!=NULL){ //get an entire line

			strcpy(line,str); //store the entire line
			//SDL_Log("%s\n",line);

			//tokenize
			tok=strtok(str,"=");
			tokPos=0;
			strcpy(val[tokPos],tok);
			while(tok != NULL){
				tok = strtok(NULL, "=");
				if(tok!=NULL){
					tokPos++;
					if(tokPos<MAXNUMPARSERVALS)
						strcpy(val[tokPos],tok);
					else
						break;
				}
			}

			if(tokPos == 1){
				if(strcmp(val[0],"Atomic Number ")==0){
					Z = (int16_t)atoi(val[1]);
				}else if(strcmp(val[0],"Mass Number ")==0){
					A = (int16_t)atoi(val[1]);
					N = (int16_t)(A - Z);
				}else if(strcmp(val[0],"Isotopic Composition ")==0){
					uint16_t nuclInd = getNuclInd(nd,N,Z);
					if(nuclInd < nd->numNucl){
						strcpy(tmpVal,val[1]);
						tok=strtok(tmpVal,"(");
						if(tok!=NULL){
							nd->nuclData[nuclInd].abundance.val = (float)(atof(tok)*100.0);
							nd->nuclData[nuclInd].abundance.format = (uint16_t)(strlen(tok)-5);
							if(nd->nuclData[nuclInd].abundance.format > 15U){
								nd->nuclData[nuclInd].abundance.format = 15U; //only 4 bits available for precision
							}
							tok=strtok(NULL,")");
							if(tok!=NULL){
								nd->nuclData[nuclInd].abundance.err = (uint8_t)atoi(tok);
								nd->nuclData[nuclInd].abundance.unit = VALUE_UNIT_PERCENT;
								//SDL_Log("Abundance for N,Z = [%i %i]: %.*f %u\n",N,Z,nd->nuclData[nuclInd].abundance.format,(double)nd->nuclData[nuclInd].abundance.val,nd->nuclData[nuclInd].abundance.err);
							}else{
								//check special cases
								if(atoi(val[1])==1){
									//100% abundance case
									nd->nuclData[nuclInd].abundance.val = 100.0f;
									nd->nuclData[nuclInd].abundance.format = 0;
									nd->nuclData[nuclInd].abundance.err = 0;
									nd->nuclData[nuclInd].abundance.unit = VALUE_UNIT_PERCENT;
								}
							}
						}
					}
				}
			}

		}
	}
	fclose(afile);
	
	SDL_Log("Finished reading abundance data file: %s\n",filePath);
  return 0;
}

int buildDatabase(const char *appBasePath, ndata *nd){

	char filePath[256],str[8];
	
	initialize_database(nd);
	
	//parse ENSDF data files
	for(uint16_t i=1;i<350;i++){
		strcpy(filePath,"");
		strcat(filePath,appBasePath);
    strcat(filePath,"data/ensdf/");
		if(i<10)
			strcat(filePath,"ensdf.00");
		else if(i<100)
			strcat(filePath,"ensdf.0");
		else
			strcat(filePath,"ensdf.");
		sprintf(str,"%u",i);
		strcat(filePath,str);
		if(parseENSDFFile(filePath,nd) == -1){ //grab data from the ENSDF file
      return -1;
    }
	}
	SDL_Log("Data imported for %i nuclei, containing:\n  %u levels (max %u)\n  %u transitions (max %u)\n  %u decay branches (max %u)\n  %u reactions (max %u)\n",nd->numNucl,nd->numLvls,MAXNUMLVLS,nd->numTran,MAXNUMTRAN,nd->numDecModes,MAXNUMDECAYMODES,nd->numRxns,MAXNUMREACTIONS);
	
	//parse abundance data file
	strcpy(filePath,"");
	strcat(filePath,appBasePath);
	strcat(filePath,"data/abundances.txt");
	if(parseAbundanceData(filePath,nd) == -1){
		return -1;
	}

  //post-process the data
  //find ground state level
  for(uint16_t i=0;i<nd->numNucl;i++){
		uint8_t firstLvlWithHl = 255;
		nd->nuclData[i].gsLevel = 0;
    for(uint16_t j=0; j<nd->nuclData[i].numLevels; j++){
      double hl = getNuclLevelHalfLifeSeconds(nd,i,j);
      if(hl >= -1.0){
				if((firstLvlWithHl==255)&&(j<255)){
					firstLvlWithHl=(uint8_t)j;
				}
				uint8_t eValueType = (uint8_t)((nd->levels[nd->nuclData[i].firstLevel + (uint32_t)j].energy.format >> 5U) & 15U);
				uint8_t isVariableE = ((eValueType == VALUETYPE_X)||(eValueType == VALUETYPE_PLUSX));
				if(!isVariableE){
					//if(j!=0) SDL_Log("GS ind for nucleus %u: %u\n",i,j);
					if(j>=255){
						SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"GS level index for nuclide %u is too high (%u).\n",i,j);
						nd->nuclData[i].gsLevel = 0;
					}else{
						nd->nuclData[i].gsLevel = (uint8_t)j;
					}
					break;
				}
      }
    }
		if((nd->nuclData[i].gsLevel == 0)&&(firstLvlWithHl != 255)){
			//could have searched through all the levels but didn't find a ground state
			//maybe it had variable energy or was excluded for some other reason
			nd->nuclData[i].gsLevel = firstLvlWithHl;
		}
  }

	//write the database to disk
	if(nd->numNucl<=0){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"no valid ENSDF data was found.\nPlease check that ENSDF files exist in the data/ensdf directory.\n");
    return -1;
  }
	SDL_Log("Database build finished.\n");
	return 0;
}

//parse all app data
int parseAppData(app_data *restrict dat, const char *appBasePath){

  //check validity of data format
  if(VALUETYPE_ENUM_LENGTH > /* DISABLES CODE */ (16)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"VALUETYPE_ENUM_LENGTH is too long, can't store as 4 bits in a bit pattern (eg. level->halfLife.format).\n");
    return -1;
  }else if(MAX_SPIN_VARS > /* DISABLES CODE */ (32)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"MAX_SPIN_VARS is too large, can't store as 5 bits in a bit pattern (eg. spinparval->format).\n");
    return -1;
  }else if(TENTATIVESP_ENUM_LENGTH > /* DISABLES CODE */ (8)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TENTATIVESP_ENUM_LENGTH is too large, can't store as 3 bits in a bit pattern (eg. spinparval->format).\n");
    return -1;
  }else if(TENTATIVEMULT_ENUM_LENGTH > /* DISABLES CODE */ (4)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"TENTATIVEMULT_ENUM_LENGTH is too large, can't store as 2 bits in a bit pattern (eg. transition->multipole).\n");
    return -1;
  }else if(EVAPTYPE_ENUM_LENGTH > /* DISABLES CODE */ (4)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"EVAPTYPE_ENUM_LENGTH is too large, can't store as 2 bits in a bit pattern (eg. reaction->ejectileNucl).\n");
    return -1;
  }

  asset_mapping *restrict stringIDmap=(asset_mapping*)SDL_calloc(1,sizeof(asset_mapping)); //allocated on heap to not overflow the stack

  //set default values
  dat->numStrings=0;
  memset(dat->locStringIDs,0,sizeof(dat->locStringIDs));

  //start parsing data
  if(parseStrings(dat,stringIDmap,appBasePath)==-1) return -1;
  if(parseAppRules(dat,stringIDmap,appBasePath)==-1) return -1;

  if(buildDatabase(appBasePath,&dat->ndat)==-1) return -1;

  //summarize
  SDL_Log("Data parsing complete.\n");
  SDL_Log("  Number of text strings parsed:               %4i (%4i max)\n",dat->numStrings,MAX_NUM_STRINGS);

  return 0; //success
  
}
