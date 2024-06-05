/* © J. Williams 2017-2024 */
/* Functions handling low-level operations on ENSDF database, or calculations using app data/state */

#include "juicer.h"
#include "data_ops.h"
#include "drawing.h"
#include "load_data.h"
#include "gui_constants.h"

//Initializes the temporary (unsaved) portion of the app state.
void initializeTempState(app_state *restrict state){
  //input
  state->mouseXPx = -1;
  state->mouseYPx = -1;
  state->mouseHoldStartPosXPx = -1;
  state->mouseHoldStartPosYPx = -1;
  state->lastAxisValX = 0;
  state->lastAxisValY = 0;
  state->activeAxis = 0; //the last used axis
  state->lastInputType = INPUT_TYPE_KEYBOARD; //default input type
	state->inputFlags = 0;
  //app state
  state->quitAppFlag = 0;
  //ui state
  changeUIState(state,UISTATE_DEFAULT);
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //no selected ui element
  state->ds.shownElements = 0; //no ui elements being shown
  state->ds.uiAnimPlaying = 0; //no ui animations playing
  state->ds.useZoomAnimations = 1;
	state->ds.chartPosX = 90.0f;
	state->ds.chartPosY = 55.0f;
	state->ds.chartZoomScale = 1.0f;
	state->ds.chartZoomToScale = state->ds.chartZoomScale;
	state->ds.chartZoomStartScale = state->ds.chartZoomScale;
	state->ds.zoomFinished = 0;
	state->ds.zoomInProgress = 0;
	state->ds.dragFinished = 0;
	state->ds.dragInProgress = 0;

  //check that constants are valid
  if(UIELEM_ENUM_LENGTH > /* DISABLES CODE */ (32)){
    printf("ERROR: ui_element_enum is too long, cannot be indexed by a uint32_t bit pattern (ds->shownElements, state->interactableElement)!\n");
    exit(-1);
  }
  if(UIANIM_ENUM_LENGTH > /* DISABLES CODE */ (32)){
    printf("ERROR: ui_animation_enum is too long, cannot be indexed by a uint32_t bit pattern (ds->uiAnimPlaying)!\n");
    exit(-1);
  }
}


void startUIAnimation(drawing_state *ds, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    printf("WARNING: startUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
  ds->timeLeftInUIAnimation[uiAnim] = UI_ANIM_LENGTH;
  ds->uiAnimPlaying |= (1U << uiAnim);
}
void stopUIAnimation(drawing_state *ds, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    printf("WARNING: stopUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
  ds->timeLeftInUIAnimation[uiAnim] = 0.0f;
  ds->uiAnimPlaying &= ~(1U << uiAnim);

  //take action at the end of the animation
  switch(uiAnim){
    case UIANIM_MSG_BOX_HIDE:
      ds->shownElements &= (uint32_t)(~(1U << UIELEM_MSG_BOX)); //close the message box
      break;
    default:
      break;
  }
  ds->forceRedraw = 1;

  //printf("Stopped anim %u.\n",uiAnim);
}
void updateUIAnimationTimes(drawing_state *ds, const float deltaTime){
  for(uint8_t i=0;i<UIANIM_ENUM_LENGTH;i++){
    if(ds->uiAnimPlaying & (uint32_t)(1U << i)){
      ds->timeLeftInUIAnimation[i] -= deltaTime;
      //printf("anim %u dt %.3f timeleft %.3f\n",i,(double)deltaTime,(double)ds->timeLeftInUIAnimation[i]);
      if(ds->timeLeftInUIAnimation[i] <= 0.0f){
        ds->timeLeftInUIAnimation[i] = 0.0f;
        stopUIAnimation(ds,i);
      }
    }
  }
}

//called once per frame
void updateDrawingState(const app_data *restrict dat, app_state *restrict state, const float deltaTime){
	if(state->ds.zoomFinished){
		//we want the zooming flag to persist for 1 frame beyond the
		//end of the zoom, to force the UI to redraw
		//printf("Finished zoom.\n");
		state->ds.zoomInProgress = 0;
		state->ds.zoomFinished = 0; //reset flag
	}
	if(state->ds.dragFinished){
		state->ds.dragInProgress = 0;
		state->ds.dragFinished = 0; //reset flag
	}
	if(state->ds.panFinished){
		state->ds.panInProgress = 0;
		state->ds.panFinished = 0; //reset flag
	}
	if(state->ds.zoomInProgress){
		state->ds.timeSinceZoomStart += deltaTime;
		state->ds.chartZoomScale = state->ds.chartZoomStartScale + ((state->ds.chartZoomToScale - state->ds.chartZoomStartScale)*juice_smoothStop2(state->ds.timeSinceZoomStart/CHART_ZOOM_TIME));
		//compute new x,y range on each axis at the new zoom scale
		float xRange = getChartWidthN(&state->ds);
		float yRange = getChartHeightZ(&state->ds);
		//set the center focus such that the cursor is at the same fractional value of the screen axes
		state->ds.chartPosX = state->ds.chartZoomStartMouseX + (0.5f - state->ds.chartZoomStartMouseXFrac)*xRange;
		state->ds.chartPosY = state->ds.chartZoomStartMouseY + (0.5f - state->ds.chartZoomStartMouseYFrac)*yRange;
		if(state->ds.timeSinceZoomStart >= CHART_ZOOM_TIME){
			state->ds.chartZoomScale = state->ds.chartZoomToScale;
			state->ds.chartPosX = state->ds.chartZoomToX;
			state->ds.chartPosY = state->ds.chartZoomToY;
			state->ds.zoomFinished = 1;
		}
		//printf("zoom scale: %0.4f\n",(double)state->ds.chartZoomScale);
	}
	if(state->ds.dragInProgress){
		state->ds.chartPosX = state->ds.chartDragStartX + ((state->ds.chartDragStartMouseX - state->mouseXPx)/(DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale));
		state->ds.chartPosY = state->ds.chartDragStartY - ((state->ds.chartDragStartMouseY - state->mouseYPx)/(DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale));
	}
	if(state->ds.panInProgress){
		state->ds.timeSincePanStart += deltaTime;
		state->ds.chartPosX = state->ds.chartPanStartX + (state->ds.chartPanToX - state->ds.chartPanStartX)*juice_smoothStop2(state->ds.timeSincePanStart/CHART_PAN_TIME);
		state->ds.chartPosY = state->ds.chartPanStartY + (state->ds.chartPanToY - state->ds.chartPanStartY)*juice_smoothStop2(state->ds.timeSincePanStart/CHART_PAN_TIME);
		if(state->ds.timeSincePanStart >= CHART_PAN_TIME){
			state->ds.chartPosX = state->ds.chartPanToX;
			state->ds.chartPosY = state->ds.chartPanToY;
			state->ds.panFinished = 1;
		}
	}
	//clamp chart display range
	if(state->ds.chartPosX < 0.0f){
		state->ds.chartPosX = 0.0f;
	}else if(state->ds.chartPosX > (dat->ndat.maxN+1)){
		state->ds.chartPosX = (float)dat->ndat.maxN+1.0f;
	}
	if(state->ds.chartPosY < 0.0f){
		state->ds.chartPosY = 0.0f;
	}else if(state->ds.chartPosY > (dat->ndat.maxZ+1)){
		state->ds.chartPosY = (float)dat->ndat.maxZ+1.0f;
	}
}

//sets everything needed to show a message box
void setupMessageBox(app_state *restrict state, const char *headerTxt, const char *msgTxt){
  strncpy(state->msgBoxHeaderTxt,headerTxt,31);
  strncpy(state->msgBoxTxt,msgTxt,255);
  state->ds.shownElements |= (uint32_t)(1U << UIELEM_MSG_BOX);
  startUIAnimation(&state->ds,UIANIM_MSG_BOX_SHOW);
  changeUIState(state,UISTATE_MSG_BOX);
}

//returns a string with element names corresponding to Z values
const char* getElemStr(const uint8_t Z){
	switch(Z){
		case 0:
			return "n";
		case 1:
			return "H";
		case 2:
			return "He";
		case 3:
			return "Li";
		case 4:
			return "Be";
		case 5:
			return "B";
		case 6:
			return "C";
		case 7:
			return "N";
		case 8:
			return "O";
		case 9:
			return "F";
		case 10:
			return "Ne";
		case 11:
			return "Na";
		case 12:
			return "Mg";
		case 13:
			return "Al";
		case 14:
			return "Si";
		case 15:
			return "P";
		case 16:
			return "S";
		case 17:
			return "Cl";
		case 18:
			return "Ar";
		case 19:
			return "K";
		case 20:
			return "Ca";
		case 21:
			return "Sc";
		case 22:
			return "Ti";
		case 23:
			return "V";
		case 24:
			return "Cr";
		case 25:
			return "Mn";
		case 26:
			return "Fe";
		case 27:
			return "Co";
		case 28:
			return "Ni";
		case 29:
			return "Cu";
		case 30:
			return "Zn";
		case 31:
			return "Ga";
		case 32:
			return "Ge";
		case 33:
			return "As";
		case 34:
			return "Se";
		case 35:
			return "Br";
		case 36:
			return "Kr";
		case 37:
			return "Rb";
		case 38:
			return "Sr";
		case 39:
			return "Y";
		case 40:
			return "Zr";
		case 41:
			return "Nb";
		case 42:
			return "Mo";
		case 43:
			return "Tc";
		case 44:
			return "Ru";
		case 45:
			return "Rh";
		case 46:
			return "Pd";
		case 47:
			return "Ag";
		case 48:
			return "Cd";
		case 49:
			return "In";
		case 50:
			return "Sn";
		case 51:
			return "Sb";
		case 52:
			return "Te";
		case 53:
			return "I";
		case 54:
			return "Xe";
		case 55:
			return "Cs";
		case 56:
			return "Ba";
		case 57:
			return "La";
		case 58:
			return "Ce";
		case 59:
			return "Pr";
		case 60:
			return "Nd";
		case 61:
			return "Pm";
		case 62:
			return "Sm";
		case 63:
			return "Eu";
		case 64:
			return "Gd";
		case 65:
			return "Tb";
		case 66:
			return "Dy";
		case 67:
			return "Ho";
		case 68:
			return "Er";
		case 69:
			return "Tm";
		case 70:
			return "Yb";
		case 71:
			return "Lu";
		case 72:
			return "Hf";
		case 73:
			return "Ta";
		case 74:
			return "W";
		case 75:
			return "Re";
		case 76:
			return "Os";
		case 77:
			return "Ir";
		case 78:
			return "Pt";
		case 79:
			return "Au";
		case 80:
			return "Hg";
		case 81:
			return "Tl";
		case 82:
			return "Pb";
		case 83:
			return "Bi";
		case 84:
			return "Po";
		case 85:
			return "At";
		case 86:
			return "Rn";
		case 87:
			return "Fr";
		case 88:
			return "Ra";
		case 89:
			return "Ac"; 
		case 90:
			return "Th";
		case 91:
			return "Pa";
		case 92:
			return "U";
		case 93:
			return "Np";
		case 94:
			return "Pu";
		case 95:
			return "Am";
		case 96:
			return "Cm";
		case 97:
			return "Bk";
		case 98:
			return "Cf";
		case 99:
			return "Es";
		case 100:
			return "Fm";
		case 101:
			return "Md";
		case 102:
			return "No";
		case 103:
			return "Lr";
		case 104:
			return "Rf";
		case 105:
			return "Db";
		case 106:
			return "Sg";
		case 107:
			return "Bh";
		case 108:
			return "Hs";
		case 109:
			return "Mt";
		case 110:
			return "Ds";
		case 111:
			return "Rg"; //unununium!
		case 112:
			return "Cn";
		case 113:
			return "Nh";
		case 114:
			return "Fl";
		case 115:
			return "Mc"; //supposedly powers UFOs
		case 116:
			return "Lv";
		case 117:
			return "Ts";
		case 118:
			return "Og";
		case 119:
			return "Uue";
		case 120:
			return "Ubn";
		case 121:
			return "Ubu";
		case 122:
			return "Ubb";
		case 123:
			return "Ubt";
		case 124:
			return "Ubq";
		default:
			return "Unknown";
	}
}

const char* getHalfLifeUnitShortStr(const uint8_t unit){
	switch(unit){
		case HALFLIFE_UNIT_STABLE:
			return "STABLE"; //stable
		case HALFLIFE_UNIT_YEARS:
			return "y";
		case HALFLIFE_UNIT_DAYS:
			return "d";
		case HALFLIFE_UNIT_HOURS:
			return "h";
		case HALFLIFE_UNIT_MINUTES:
			return "m";
		case HALFLIFE_UNIT_SECONDS:
			return "s";
		case HALFLIFE_UNIT_MILLISECONDS:
			return "ms";
		case HALFLIFE_UNIT_MICROSECONDS:
			return "µs";
		case HALFLIFE_UNIT_NANOSECONDS:
			return "ns";
		case HALFLIFE_UNIT_PICOSECONDS:
			return "ps";
		case HALFLIFE_UNIT_FEMTOSECONDS:
			return "fs";
		case HALFLIFE_UNIT_ATTOSECONDS:
			return "as";
		case HALFLIFE_UNIT_EV:
			return "eV";
		case HALFLIFE_UNIT_KEV:
			return "keV";
		case HALFLIFE_UNIT_MEV:
			return "MeV";
		case HALFLIFE_UNIT_NOVAL:
		default:
			return "";																						
	}
}

const char* getValueTypeShortStr(const uint8_t type){
	switch(type){
		case VALUETYPE_APPROX:
			return "≈ ";
		case VALUETYPE_GREATERTHAN:
			return "> ";
		case VALUETYPE_GREATEROREQUALTHAN:
			return "≥ ";
		case VALUETYPE_LESSTHAN:
			return "< ";
		case VALUETYPE_LESSOREQUALTHAN:
			return "≤ ";
		case VALUETYPE_UNKNOWN:
			return " ?";
		case VALUETYPE_NUMBER:
		default:
			return "";
	}
}

const char* getDecayTypeShortStr(const uint8_t type){
	switch(type){
		case DECAYMODE_BETAMINUS:
			return "β-";
		case DECAYMODE_BETAPLUS:
			return "β+";
		case DECAYMODE_ALPHA:
			return "α";
		case DECAYMODE_BETAMINUS_NEUTRON:
			return "β-n";
		case DECAYMODE_BETAPLUS_PROTON:
			return "β+p";
		case DECAYMODE_BETAPLUS_TWOPROTON:
			return "β+2p";
		case DECAYMODE_EC:
			return "ε";
		case DECAYMODE_EC_PROTON:
			return "εp";
		case DECAYMODE_ECANDBETAPLUS:
			return "ε/β+";
		case DECAYMODE_IT:
			return "IT";
		case DECAYMODE_3HE:
			return "3He";
		case DECAYMODE_DEUTERON:
			return "d";
		case DECAYMODE_NEUTRON:
			return "n";
		case DECAYMODE_TWONEUTRON:
			return "2n";
		case DECAYMODE_PROTON:
			return "p";
		case DECAYMODE_TWOPROTON:
			return "2p";
		case DECAYMODE_SPONTANEOUSFISSION:
			return "SF";
		default:
			return "";																						
	}
}

void getHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint32_t lev){
	//are the snprintf lines slow?
	if(lev < nd->numLvls){
		if(nd->levels[lev].halfLifeUnit == HALFLIFE_UNIT_STABLE){
			snprintf(strOut,32,"STABLE");
		}else if(nd->levels[lev].halfLifeUnit == HALFLIFE_UNIT_NOVAL){
			snprintf(strOut,32,"Unknown");
		}else if(nd->levels[lev].halfLife > 0.0f){
			uint8_t hlPrecision = (uint8_t)(nd->levels[lev].halfLifeFormat & 15U);
			uint8_t hlExponent = (uint8_t)((nd->levels[lev].halfLifeFormat >> 4U) & 1U);
			uint8_t hlValueType = (uint8_t)((nd->levels[lev].halfLifeFormat >> 5U) & 7U);
			if(hlPrecision > 0){
				if(hlExponent == 0){
					snprintf(strOut,32,"%s%.*f %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lev].halfLife),getHalfLifeUnitShortStr(nd->levels[lev].halfLifeUnit));
				}else{
					snprintf(strOut,32,"%s%.*e %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lev].halfLife),getHalfLifeUnitShortStr(nd->levels[lev].halfLifeUnit));
				}
			}else{
				if(hlExponent == 0){
					snprintf(strOut,32,"%s%.0f %s",getValueTypeShortStr(hlValueType),(double)(nd->levels[lev].halfLife),getHalfLifeUnitShortStr(nd->levels[lev].halfLifeUnit));
				}else{
					snprintf(strOut,32,"%s%.0e %s",getValueTypeShortStr(hlValueType),(double)(nd->levels[lev].halfLife),getHalfLifeUnitShortStr(nd->levels[lev].halfLifeUnit));
				}
			}
		}else{
			snprintf(strOut,32," ");
		}
	}else{
		snprintf(strOut,32,"Unknown");
	}
}
void getGSHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd){
	if(nd->nuclData[nuclInd].numLevels > 0){
		getHalfLifeStr(strOut,nd,nd->nuclData[nuclInd].firstLevel + nd->nuclData[nuclInd].gsLevel);
	}else{
		snprintf(strOut,32,"Unknown");
	}
}

void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd){
	if(dcyModeInd < nd->numDecModes){
		uint8_t decValueType = nd->dcyMode[dcyModeInd].probType;
		uint8_t decType = nd->dcyMode[dcyModeInd].type;
		if(decValueType == VALUETYPE_NUMBER){
			snprintf(strOut,32,"%s=%.0f%%%%",getDecayTypeShortStr(decType),(double)nd->dcyMode[dcyModeInd].prob); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
		}else if(decValueType == VALUETYPE_UNKNOWN){
			snprintf(strOut,32,"%s%s",getDecayTypeShortStr(decType),getValueTypeShortStr(decValueType));
		}else{
			snprintf(strOut,32,"%s %s%.0f%%%%",getDecayTypeShortStr(decType),getValueTypeShortStr(decValueType),(double)nd->dcyMode[dcyModeInd].prob); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
		}
	}else{
		snprintf(strOut,32," ");
	}
}

double getLevelHalfLifeSeconds(const ndata *restrict nd, const uint32_t levelInd){
	if(levelInd < nd->numLvls){
		double hl = (double)(nd->levels[levelInd].halfLife);
		if(hl < 0.0){
			//unknown half-life
			return -2.0;
		}
		uint8_t hlUnit = nd->levels[levelInd].halfLifeUnit;
		switch(hlUnit){
			case HALFLIFE_UNIT_STABLE:
				return 1.0E30; //stable
			case HALFLIFE_UNIT_YEARS:
				return hl*365.25*24*3600;
			case HALFLIFE_UNIT_DAYS:
				return hl*24*3600;
			case HALFLIFE_UNIT_HOURS:
				return hl*3600;
			case HALFLIFE_UNIT_MINUTES:
				return hl*60;
			case HALFLIFE_UNIT_SECONDS:
				return hl;
			case HALFLIFE_UNIT_MILLISECONDS:
				return hl*0.001;
			case HALFLIFE_UNIT_MICROSECONDS:
				return hl*0.000001;
			case HALFLIFE_UNIT_NANOSECONDS:
				return hl*0.000000001;
			case HALFLIFE_UNIT_PICOSECONDS:
				return hl*0.000000000001;
			case HALFLIFE_UNIT_FEMTOSECONDS:
				return hl*0.000000000000001;
			case HALFLIFE_UNIT_ATTOSECONDS:
				return hl*0.000000000000000001;
			default:
				return -2.0; //couldn't find half-life
		}
	}else{
		return -2.0; //couldn't find half-life
	}
}

double getNuclLevelHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel){
	if((nuclInd < nd->numNucl)&&(nuclLevel < nd->nuclData[nuclInd].numLevels)){
		return getLevelHalfLifeSeconds(nd,(uint32_t)(nd->nuclData[nuclInd].firstLevel + (uint32_t)nuclLevel));
	}else{
		return -2.0; //couldn't find half-life
	}
}

double getNuclGSHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd){
	return getNuclLevelHalfLifeSeconds(nd,nuclInd,nd->nuclData[nuclInd].gsLevel);
}


float mouseXPxToN(const drawing_state *restrict ds, const float mouseX){
	return ds->chartPosX + ((mouseX - ds->windowXRes/2.0f)/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float mouseYPxToZ(const drawing_state *restrict ds, const float mouseY){
	return ds->chartPosY - ((mouseY - ds->windowYRes/2.0f)/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float getMinChartN(const drawing_state *restrict ds){
	return ds->chartPosX - (ds->windowXRes)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMaxChartN(const drawing_state *restrict ds){
	return ds->chartPosX + (ds->windowXRes)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMinChartZ(const drawing_state *restrict ds){
	return ds->chartPosY - (ds->windowYRes)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMaxChartZ(const drawing_state *restrict ds){
	return ds->chartPosY + (ds->windowYRes)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getChartWidthN(const drawing_state *restrict ds){
	return (ds->windowXRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float getChartHeightZ(const drawing_state *restrict ds){
	return (ds->windowYRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float getChartWidthNAfterZoom(const drawing_state *restrict ds){
	return (ds->windowXRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomToScale));
}
float getChartHeightZAfterZoom(const drawing_state *restrict ds){
	return (ds->windowYRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomToScale));
}

//change the modal state of the UI, and update which UI elements are interactable
void changeUIState(app_state *restrict state, const uint8_t newState){
  
  state->interactableElement = 0;
  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' any buttons
  state->uiState = newState;
  
  switch(state->uiState){
    case UISTATE_UNINTERACTABLE:
      //no UI elements are interactable
      break;
    case UISTATE_MSG_BOX:
      state->interactableElement |= (uint32_t)(1U << UIELEM_MSG_BOX_OK_BUTTON);
      break;
    case UISTATE_DEFAULT:
    default:
      state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
      break;
  }
}

//take action after clicking a button or other UI element
void uiElemClickAction(app_state *restrict state, const uint8_t uiElemID){
  state->clickedUIElem = uiElemID;
  switch(uiElemID){
    case UIELEM_MENU_BUTTON:
      if(state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU)){
        state->ds.shownElements &= (uint32_t)(~(1 << UIELEM_PRIMARY_MENU)); //close the menu
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else{
        state->ds.shownElements |= (1U << UIELEM_PRIMARY_MENU);
      }
      break;
    case UIELEM_MSG_BOX_OK_BUTTON:
      changeUIState(state,UISTATE_DEFAULT);
      startUIAnimation(&state->ds,UIANIM_MSG_BOX_HIDE); //message box will be closed after animation finishes
      break;
    default:
      if(state->uiState == UISTATE_DEFAULT){
        state->ds.shownElements = 0; //close any menu being shown
      }
      break;
  }
}

//updates the UI element (buttons etc.) positions, based on the screen resolution
//positioning constants are defined in gui_constants.h
void updateUIElemPositions(drawing_state *restrict ds){
  for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){
    switch(i){
      case UIELEM_MENU_BUTTON:
        ds->uiElemPosX[i] = (uint16_t)(ds->windowXRes-MENU_BUTTON_WIDTH-MENU_BUTTON_POS_XR);
        ds->uiElemPosY[i] = MENU_BUTTON_POS_Y;
        ds->uiElemWidth[i] = MENU_BUTTON_WIDTH;
        ds->uiElemHeight[i] = UI_TILE_SIZE;
        break;
      case UIELEM_PRIMARY_MENU:
        ds->uiElemPosX[i] = (uint16_t)(ds->windowXRes-PRIMARY_MENU_WIDTH-PRIMARY_MENU_POS_XR);
        ds->uiElemPosY[i] = PRIMARY_MENU_POS_Y;
        ds->uiElemWidth[i] = PRIMARY_MENU_WIDTH;
        ds->uiElemHeight[i] = PRIMARY_MENU_HEIGHT;
        break;
      case UIELEM_MSG_BOX:
        ds->uiElemPosX[i] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_WIDTH)/2);
        ds->uiElemPosY[i] = (uint16_t)((ds->windowYRes - MESSAGE_BOX_HEIGHT)/2);
        ds->uiElemWidth[i] = MESSAGE_BOX_WIDTH;
        ds->uiElemHeight[i] = MESSAGE_BOX_HEIGHT;
        break;
      case UIELEM_MSG_BOX_OK_BUTTON:
        ds->uiElemPosX[i] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_OK_BUTTON_WIDTH)/2);
        ds->uiElemPosY[i] = (uint16_t)((ds->windowYRes + MESSAGE_BOX_HEIGHT)/2 - MESSAGE_BOX_OK_BUTTON_YB - UI_TILE_SIZE);
        ds->uiElemWidth[i] = MESSAGE_BOX_OK_BUTTON_WIDTH;
        ds->uiElemHeight[i] = UI_TILE_SIZE;
        break;
      default:
        break;
    }
  }
}


void updateWindowRes(app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat){
  int wwidth, wheight;
  int rwidth, rheight;
  SDL_GetWindowSize(rdat->window, &wwidth, &wheight);
  SDL_GetWindowSizeInPixels(rdat->window, &rwidth, &rheight);
  if((rwidth != ds->windowXRenderRes)||(rheight != ds->windowYRenderRes)){
    ds->forceRedraw = 1;
  }
	float newScale = (float)rwidth/((float)wwidth);
	if(fabsf(rdat->uiScale - newScale) > 0.001f){
		printf("Re-scaling UI from %0.9f to %0.9f.\n",(double)rdat->uiScale,(double)newScale);
		rdat->uiScale = newScale; //set UI scale properly for HI-DPI
		if(rdat->font){
			//rescale fonts as well, this requires loading them from the app data file
			regenerateFontCache(dat,rdat); //load_data.c
		}
	}
  ds->windowXRes = (uint16_t)wwidth;
  ds->windowYRes = (uint16_t)wheight;
  ds->windowXRenderRes = (uint16_t)rwidth;
  ds->windowYRenderRes = (uint16_t)rheight;

  //update things that depend on the window res
  updateUIElemPositions(ds); //UI element positions
}

void handleScreenGraphicsMode(app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat){

  //handle vsync and frame cap
  SDL_SetRenderVSync(rdat->renderer,1); //vsync always enabled

  if(ds->windowFullscreenMode){
    if(SDL_SetWindowFullscreen(rdat->window,SDL_TRUE) != 0){
      printf("WARNING: cannot set fullscreen mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,ds,rdat);
    //printf("Full screen display mode.  Window resolution: %u x %u.\n",ds->windowXRes,ds->windowYRes);
  }else{
    if(SDL_SetWindowFullscreen(rdat->window,0) != 0){
      printf("WARNING: cannot set windowed mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,ds,rdat);
    //printf("Windowed display mode.  Window resolution: %u x %u.\n",ds->windowXRes,ds->windowYRes);
  }
}
