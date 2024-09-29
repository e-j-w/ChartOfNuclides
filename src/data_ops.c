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

/* Functions handling low-level operations on ENSDF database, or calculations using app data/state */

#include "juicer.h"
#include "data_ops.h"
#include "drawing.h"
#include "load_data.h"
#include "gui_constants.h"

//Initializes the temporary (unsaved) portion of the app state.
//The default preferences are set here, these will later be overwritten during
//app startup by the values in con.ini
void initializeTempState(const app_data *restrict dat, app_state *restrict state){
  
	//input
  state->mouseXPx = -1;
  state->mouseYPx = -1;
  state->mouseHoldStartPosXPx = -1;
  state->mouseHoldStartPosYPx = -1;
  state->lastAxisValLX = 0;
  state->lastAxisValLY = 0;
	state->lastAxisValRX = 0;
  state->lastAxisValRY = 0;
  state->activeAxisX = 255;
	state->activeAxisY = 255;
  state->lastInputType = INPUT_TYPE_KEYBOARD; //default input type
	state->kbdModVal = KBD_MOD_NONE;
	state->mouseoverElement = UIELEM_ENUM_LENGTH;
	state->scrollSpeedMultiplier = 1.0f;
	state->inputFlags = 0;
  //app state
	state->chartSelectedNucl = MAXNUMNUCL;
	state->chartView = CHARTVIEW_HALFLIFE;
  state->quitAppFlag = 0;
	state->gamepadDeadzone = 16000;
  state->ds.windowXRes = 880;
  state->ds.windowYRes = 580;
  state->ds.drawPerformanceStats = 0;
  //ui state
	state->lastUIState = UISTATE_CHARTONLY;
  changeUIState(dat,state,UISTATE_CHARTONLY);
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //no selected UI element
	state->lastOpenedMenu = UIELEM_ENUM_LENGTH; //no previously selected menu
  state->ds.shownElements = 0; //no UI elements being shown
	state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES);
  state->ds.uiAnimPlaying = 0; //no UI animations playing
  state->ds.useUIAnimations = 1;
	state->ds.useLifetimes = 0;
	state->ds.drawShellClosures = 1;
	state->ds.chartPosX = 86.0f;
	state->ds.chartPosY = 52.0f;
	state->ds.chartZoomScale = 0.5f;
	state->ds.chartZoomToScale = state->ds.chartZoomScale;
	state->ds.chartZoomStartScale = state->ds.chartZoomScale;
	state->ds.infoBoxTableHeight = NUCL_INFOBOX_BIGLINE_HEIGHT;
	state->ds.infoBoxEColOffset = NUCL_INFOBOX_ENERGY_COL_MIN_OFFSET;
	state->ds.infoBoxJpiColOffset = NUCL_INFOBOX_JPI_COL_MIN_OFFSET;
	state->ds.infoBoxHlColOffset = NUCL_INFOBOX_HALFLIFE_COL_MIN_OFFSET;
	state->ds.infoBoxDcyModeColOffset = NUCL_INFOBOX_DECAYMODE_COL_MIN_OFFSET;
	state->ds.totalPanTime = CHART_KEY_PAN_TIME;
	state->ds.zoomFinished = 0;
	state->ds.zoomInProgress = 0;
	state->ds.dragFinished = 0;
	state->ds.dragInProgress = 0;
	state->ds.fcScrollFinished = 0;
	state->ds.fcScrollInProgress = 0;
	state->ds.fcNuclChangeInProgress = 0;
	state->ds.interfaceSizeInd = UISCALE_NORMAL;
	memset(state->ds.uiElemExtPlusX,0,sizeof(state->ds.uiElemExtPlusX));
	memset(state->ds.uiElemExtPlusY,0,sizeof(state->ds.uiElemExtPlusY));
	memset(state->ds.uiElemExtMinusX,0,sizeof(state->ds.uiElemExtMinusX));
	memset(state->ds.uiElemExtMinusY,0,sizeof(state->ds.uiElemExtMinusY));

  //check that constants are valid
  if(UIELEM_ENUM_LENGTH > /* DISABLES CODE */ (32)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ui_element_enum is too long, cannot be indexed by a uint32_t bit pattern (ds->shownElements, state->interactableElement)!\n");
    exit(-1);
  }
  if(UIANIM_ENUM_LENGTH > /* DISABLES CODE */ (32)){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ui_animation_enum is too long, cannot be indexed by a uint32_t bit pattern (ds->uiAnimPlaying)!\n");
    exit(-1);
  }
}


void startUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"startUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
	switch(uiAnim){
		case UIANIM_NUCLINFOBOX_TXTFADEOUT:
		case UIANIM_NUCLINFOBOX_EXPAND:
			//short animation
			state->ds.timeLeftInUIAnimation[uiAnim] = SHORT_UI_ANIM_LENGTH;
			break;
		default:
			state->ds.timeLeftInUIAnimation[uiAnim] = UI_ANIM_LENGTH;
			break;
	}
  
  state->ds.uiAnimPlaying |= (1U << uiAnim);

	//take action at the start of the animation
	switch(uiAnim){
		case UIANIM_PRIMARY_MENU_HIDE:
			changeUIState(dat,state,state->uiState); //make menu items uninteractable
			break;
		case UIANIM_NUCLINFOBOX_HIDE:
			changeUIState(dat,state,UISTATE_CHARTONLY); //make info box un-interactable
			break;
		default:
			break;
	}
}
void stopUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"stopUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
  state->ds.timeLeftInUIAnimation[uiAnim] = 0.0f;
  state->ds.uiAnimPlaying &= ~(1U << uiAnim);

  //take action at the end of the animation
  switch(uiAnim){
		case UIANIM_PRIMARY_MENU_HIDE:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_PRIMARY_MENU)); //close the menu
			if(!(state->ds.shownElements & (1U << UIELEM_PREFS_DIALOG))){
				if(!(state->ds.shownElements & (1U << UIELEM_ABOUT_BOX))){
					if(!(state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))){
						if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
							changeUIState(dat,state,UISTATE_FULLLEVELINFO);
						}else if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
							changeUIState(dat,state,UISTATE_INFOBOX);
						}else if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
							changeUIState(dat,state,UISTATE_FULLLEVELINFO);
						}else{
							changeUIState(dat,state,UISTATE_CHARTONLY);
						}
					}
				}
			}
			break;
		case UIANIM_CHARTVIEW_MENU_HIDE:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_CHARTVIEW_MENU)); //close the menu
			if(!(state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))){
				if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
					changeUIState(dat,state,UISTATE_INFOBOX);
				}else if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
					changeUIState(dat,state,UISTATE_FULLLEVELINFO);
				}else{
					changeUIState(dat,state,UISTATE_CHARTONLY);
				}
			}
			break;
		case UIANIM_UISCALE_MENU_HIDE:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_PREFS_UISCALE_MENU)); //close the menu
			changeUIState(dat,state,UISTATE_PREFS_DIALOG);
			break;
    case UIANIM_MODAL_BOX_HIDE:
      state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_MSG_BOX)); //close the message box
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_ABOUT_BOX)); //close the about box
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_PREFS_DIALOG)); //close the preferences dialog
			if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
				changeUIState(dat,state,UISTATE_FULLLEVELINFO);
			}else if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
				changeUIState(dat,state,UISTATE_INFOBOX);
			}else{
				changeUIState(dat,state,UISTATE_CHARTONLY);
			}
			//SDL_Log("UI state: %u\n",state->uiState);
      break;
		case UIANIM_NUCLINFOBOX_HIDE:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_INFOBOX)); //close the info box
			state->chartSelectedNucl = MAXNUMNUCL;
			break;
		case UIANIM_NUCLINFOBOX_EXPAND:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_INFOBOX)); //close the info box
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_CHARTOFNUCLIDES)); //don't show the chart
			state->ds.shownElements |= (1U << UIELEM_NUCL_FULLINFOBOX); //show the full info box
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_TXTFADEIN);
			changeUIState(dat,state,UISTATE_FULLLEVELINFO); //update UI state now that the full info box is visible
			break;
		case UIANIM_NUCLINFOBOX_TXTFADEOUT:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_FULLINFOBOX)); //close the full info box
			state->ds.shownElements |= (1U << UIELEM_NUCL_INFOBOX); //show the info box
			state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES); //show the chart
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_CONTRACT);
			changeUIState(dat,state,UISTATE_INFOBOX); //update UI state now that the regular info box is visible
			break;
    default:
      break;
  }
  state->ds.forceRedraw = 1;

  //SDL_Log("Stopped anim %u.\n",uiAnim);
}
void updateUIAnimationTimes(const app_data *restrict dat, app_state *restrict state, const float deltaTime){
  for(uint8_t i=0;i<UIANIM_ENUM_LENGTH;i++){
    if(state->ds.uiAnimPlaying & (uint32_t)(1U << i)){
      state->ds.timeLeftInUIAnimation[i] -= deltaTime;
      //SDL_Log("anim %u dt %.3f timeleft %.3f\n",i,(double)deltaTime,(double)state->ds.timeLeftInUIAnimation[i]);
      if((state->ds.timeLeftInUIAnimation[i] <= 0.0f)||(state->ds.useUIAnimations == 0)){
        state->ds.timeLeftInUIAnimation[i] = 0.0f;
        stopUIAnimation(dat,state,i);
      }
    }
  }
}

//called once per frame
void updateDrawingState(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float deltaTime){
	if(state->ds.zoomFinished){
		//we want the zooming flag to persist for 1 frame beyond the
		//end of the zoom, to force the UI to redraw
		//SDL_Log("Finished zoom.\n");
		state->ds.zoomInProgress = 0;
		state->ds.zoomFinished = 0; //reset flag
	}
	if(state->ds.dragFinished){
		state->ds.dragInProgress = 0;
		state->ds.dragFinished = 0; //reset flag
		SDL_SetCursor(rdat->defaultCursor); //set cursor back to default
	}
	if(state->ds.panFinished){
		state->ds.panInProgress = 0;
		state->ds.panFinished = 0; //reset flag
	}
	if(state->ds.fcScrollFinished){
		state->ds.fcScrollInProgress = 0;
		state->ds.fcScrollFinished = 0; //reset flag
	}
	if(state->ds.fcNuclChangeFinished){
		state->ds.fcNuclChangeInProgress = 0;
		state->ds.fcNuclChangeFinished = 0; //reset flag
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
		//SDL_Log("zoom scale: %0.4f\n",(double)state->ds.chartZoomScale);
	}
	if(state->ds.dragInProgress){
		if((state->lastInputType == INPUT_TYPE_MOUSE)&&((state->ds.chartPosX != state->ds.chartDragStartX)||(state->ds.chartPosY != state->ds.chartDragStartY))){
      SDL_SetCursor(rdat->dragCursor); //set mouse cursor
    }
		state->ds.chartPosX = state->ds.chartDragStartX + ((state->ds.chartDragStartMouseX - state->mouseXPx)/(DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale));
		state->ds.chartPosY = state->ds.chartDragStartY - ((state->ds.chartDragStartMouseY - state->mouseYPx)/(DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale));
	}
	if(state->ds.panInProgress){
		state->ds.timeSincePanStart += deltaTime;
		state->ds.chartPosX = state->ds.chartPanStartX + (state->ds.chartPanToX - state->ds.chartPanStartX)*juice_smoothStop2(state->ds.timeSincePanStart/state->ds.totalPanTime);
		state->ds.chartPosY = state->ds.chartPanStartY + (state->ds.chartPanToY - state->ds.chartPanStartY)*juice_smoothStop2(state->ds.timeSincePanStart/state->ds.totalPanTime);
		if(state->ds.timeSincePanStart >= state->ds.totalPanTime){
			state->ds.chartPosX = state->ds.chartPanToX;
			state->ds.chartPosY = state->ds.chartPanToY;
			state->ds.panFinished = 1;
		}
		//SDL_Log("pan t: %0.3f\n",(double)state->ds.timeSincePanStart);
	}
	if(state->ds.fcScrollInProgress){
		state->ds.timeSinceFCScollStart += deltaTime;
		state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollStartY + (state->ds.nuclFullInfoScrollToY - state->ds.nuclFullInfoScrollStartY)*juice_smoothStop2(state->ds.timeSinceFCScollStart/NUCL_FULLINFOBOX_SCROLL_TIME);
		if(state->ds.timeSinceFCScollStart >= NUCL_FULLINFOBOX_SCROLL_TIME){
			state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollToY;
			state->ds.fcScrollFinished = 1;
		}
		//SDL_Log("scroll t: %0.3f, pos: %f\n",(double)state->ds.timeSinceFCScollStart,(double)state->ds.nuclFullInfoScrollY);
	}
	if(state->ds.fcNuclChangeInProgress){
		state->ds.timeSinceFCNuclChangeStart += deltaTime;
		if(state->ds.timeSinceFCNuclChangeStart >= NUCL_FULLINFOBOX_SCROLL_TIME){
			state->ds.fcNuclChangeFinished = 1;
		}
		//SDL_Log("scroll t: %0.3f, pos: %f\n",(double)state->ds.timeSinceFCScollStart,(double)state->ds.nuclFullInfoScrollY);
	}
	//clamp chart display range
	if(state->ds.chartPosX < (-0.25f*getChartWidthN(&state->ds))){
		state->ds.chartPosX = (-0.25f*getChartWidthN(&state->ds));
	}else if(state->ds.chartPosX > (dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds)))){
		state->ds.chartPosX = (float)dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds));
	}
	if(state->ds.chartPosY < (-0.25f*getChartHeightZ(&state->ds))){
		state->ds.chartPosY = (-0.25f*getChartHeightZ(&state->ds));
	}else if(state->ds.chartPosY > (dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds)))){
		state->ds.chartPosY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds));
	}

	//dismiss info box if it selected nuclide is offscreen
	if(state->chartSelectedNucl != MAXNUMNUCL){
		if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
			if((state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]==0.0f)){
				if(((dat->ndat.nuclData[state->chartSelectedNucl].N+1) < getMinChartN(&state->ds))||(dat->ndat.nuclData[state->chartSelectedNucl].N > getMaxChartN(&state->ds))){
					//SDL_Log("hiding info box\n");
					startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
					startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
				}else if((dat->ndat.nuclData[state->chartSelectedNucl].Z < getMinChartZ(&state->ds))||((dat->ndat.nuclData[state->chartSelectedNucl].Z-1) > getMaxChartZ(&state->ds))){
					//SDL_Log("hiding info box\n");
					startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
					startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
				}
			}
		}
	}
}

//sets everything needed to show a message box
void setupMessageBox(const app_data *restrict dat, app_state *restrict state, const char *headerTxt, const char *msgTxt){
  strncpy(state->msgBoxHeaderTxt,headerTxt,31);
  strncpy(state->msgBoxTxt,msgTxt,255);
  state->ds.shownElements |= (uint32_t)(1U << UIELEM_MSG_BOX);
  startUIAnimation(dat,state,UIANIM_MODAL_BOX_SHOW);
  changeUIState(dat,state,UISTATE_MSG_BOX);
}

//gets strings that denote 'special' levels in certain nuclides
const char* getSpecialLvlStr(const app_data *restrict dat, const uint8_t specialLvlInd){
	switch(specialLvlInd){
		case SPECIALLEVEL_HOYLE:
			return dat->strings[dat->locStringIDs[LOCSTR_SL_HOYLE]];
		case SPECIALLEVEL_NATURALLYOCCURINGISOMER:
			return dat->strings[dat->locStringIDs[LOCSTR_SL_NATURALLYOCCURINGISOMER]];
		case SPECIALLEVEL_CLOCKISOMER:
			return dat->strings[dat->locStringIDs[LOCSTR_SL_CLOCKISOMER]];
		default:
			return " ";
	}
}

//returns a string with element names corresponding to Z values
//N=255 causes special names to not be used
const char* getFullElemStr(const uint8_t Z, const uint8_t N){
	switch(Z){
		case 0:
			if(N != 255){
				//different names depending on N
				if(N==2){
					return "Dineutron";
				}else if(N==3){
					return "Trineutron";
				}else if(N==4){
					return "Tetraneutron";
				}else if(N==5){
					return "Pentaneutron";
				}else{
					return "Neutron";
				}
			}else{
				return "Neutron";
			}
		case 1:
			if(N != 255){
				//different names depending on N
				if(N==1){
					return "Deuterium";
				}else if(N==2){
					return "Tritium";
				}else{
					return "Hydrogen";
				}
			}else{
				return "Hydrogen";
			}
		case 2:
			return "Helium";
		case 3:
			return "Lithium";
		case 4:
			return "Beryllium";
		case 5:
			return "Boron";
		case 6:
			return "Carbon";
		case 7:
			return "Nitrogen";
		case 8:
			return "Oxygen";
		case 9:
			return "Fluorine";
		case 10:
			return "Neon";
		case 11:
			return "Sodium";
		case 12:
			return "Magnesium";
		case 13:
			return "Aluminium";
		case 14:
			return "Silicon";
		case 15:
			return "Phosphorus";
		case 16:
			return "Sulfur";
		case 17:
			return "Chlorine";
		case 18:
			return "Argon";
		case 19:
			return "Potassium";
		case 20:
			return "Calcium";
		case 21:
			return "Scandium";
		case 22:
			return "Titanium";
		case 23:
			return "Vanadium";
		case 24:
			return "Chromium";
		case 25:
			return "Manganese";
		case 26:
			return "Iron";
		case 27:
			return "Cobalt";
		case 28:
			return "Nickel";
		case 29:
			return "Copper";
		case 30:
			return "Zinc";
		case 31:
			return "Gallium";
		case 32:
			return "Germanium";
		case 33:
			return "Arsenic";
		case 34:
			return "Selenium";
		case 35:
			return "Bromine";
		case 36:
			return "Krypton";
		case 37:
			return "Rubidium";
		case 38:
			return "Strontium";
		case 39:
			return "Yttrium";
		case 40:
			return "Zirconium";
		case 41:
			return "Niobium";
		case 42:
			return "Molybdenum";
		case 43:
			return "Technetium";
		case 44:
			return "Ruthenium";
		case 45:
			return "Rhodium";
		case 46:
			return "Palladium";
		case 47:
			return "Silver";
		case 48:
			return "Cadmium";
		case 49:
			return "Indium";
		case 50:
			return "Tin";
		case 51:
			return "Antimony";
		case 52:
			return "Tellurium";
		case 53:
			return "Iodine";
		case 54:
			return "Xenon";
		case 55:
			return "Caesium";
		case 56:
			return "Barium";
		case 57:
			return "Lanthanum";
		case 58:
			return "Cerium";
		case 59:
			return "Praseodymium";
		case 60:
			return "Neodymium";
		case 61:
			return "Promethium";
		case 62:
			return "Samarium";
		case 63:
			return "Europium";
		case 64:
			return "Gadolinium";
		case 65:
			return "Terbium";
		case 66:
			return "Dysprosium";
		case 67:
			return "Holmium";
		case 68:
			return "Erbium";
		case 69:
			return "Thulium";
		case 70:
			return "Ytterbium";
		case 71:
			return "Lutetium";
		case 72:
			return "Hafnium";
		case 73:
			return "Tantalum";
		case 74:
			return "Tungsten";
		case 75:
			return "Rhenium";
		case 76:
			return "Osmium";
		case 77:
			return "Iridium";
		case 78:
			return "Platinum";
		case 79:
			return "Gold";
		case 80:
			return "Mercury";
		case 81:
			return "Thallium";
		case 82:
			return "Lead";
		case 83:
			return "Bismuth";
		case 84:
			return "Polonium";
		case 85:
			return "Astatine";
		case 86:
			return "Radon";
		case 87:
			return "Francium";
		case 88:
			return "Radium";
		case 89:
			return "Actinium";
		case 90:
			return "Thorium";
		case 91:
			return "Protactinium";
		case 92:
			return "Uranium";
		case 93:
			return "Neptunium";
		case 94:
			return "Plutonium";
		case 95:
			return "Americium";
		case 96:
			return "Curium";
		case 97:
			return "Berkelium";
		case 98:
			return "Californium";
		case 99:
			return "Einsteinium";
		case 100:
			return "Fermium";
		case 101:
			return "Mendelevium";
		case 102:
			return "Nobelium";
		case 103:
			return "Lawrencium";
		case 104:
			return "Rutherfordium";
		case 105:
			return "Dubnium";
		case 106:
			return "Seaborgium";
		case 107:
			return "Bohrium";
		case 108:
			return "Hassium";
		case 109:
			return "Meitnerium";
		case 110:
			return "Darmstadtium";
		case 111:
			return "Roentgenium"; //unununium!
		case 112:
			return "Copernicium";
		case 113:
			return "Nihonium";
		case 114:
			return "Flerovium";
		case 115:
			return "Moscovium"; //supposedly powers UFOs
		case 116:
			return "Livermorium";
		case 117:
			return "Tennessine";
		case 118:
			return "Oganesson";
		case 119:
			return "Ununennium";
		case 120:
			return "Unbinilium";
		case 121:
			return "Unbiunium";
		case 122:
			return "Unbibium";
		case 123:
			return "Unbitrium";
		case 124:
			return "Unbiquadium";
		default:
			return "Unknown";
	}
}

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

const char* getValueUnitShortStr(const uint8_t unit){
	switch(unit){
		case VALUE_UNIT_STABLE:
			return "STABLE"; //stable
		case VALUE_UNIT_YEARS:
			return "y";
		case VALUE_UNIT_DAYS:
			return "d";
		case VALUE_UNIT_HOURS:
			return "h";
		case VALUE_UNIT_MINUTES:
			return "m";
		case VALUE_UNIT_SECONDS:
			return "s";
		case VALUE_UNIT_MILLISECONDS:
			return "ms";
		case VALUE_UNIT_MICROSECONDS:
			return "µs";
		case VALUE_UNIT_NANOSECONDS:
			return "ns";
		case VALUE_UNIT_PICOSECONDS:
			return "ps";
		case VALUE_UNIT_FEMTOSECONDS:
			return "fs";
		case VALUE_UNIT_ATTOSECONDS:
			return "as";
		case VALUE_UNIT_EV:
			return "eV";
		case VALUE_UNIT_KEV:
			return "keV";
		case VALUE_UNIT_MEV:
			return "MeV";
		case VALUE_UNIT_NOVAL:
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
		case DECAYMODE_BETAMINUS_ALPHA:
			return "β-α";
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
		case DECAYMODE_BETAMINUS_SPONTANEOUSFISSION:
			return "β-SF";
		case DECAYMODE_2BETAMINUS:
			return "2β-";
		case DECAYMODE_2BETAPLUS:
			return "2β+";
		case DECAYMODE_2EC:
			return "2ε";
		case DECAYMODE_14C:
			return "14C";
		case DECAYMODE_20NE:
			return "20Ne";
		case DECAYMODE_25NE:
			return "25Ne";
		case DECAYMODE_28MG:
			return "28Mg";
		case DECAYMODE_34SI:
			return "34Si";
		default:
			return "";																						
	}
}

//for two multipolarity values, determine whether or not they
//can occur within the same transition (mix)
//if they can mix, return 1
//if not, return 0
uint8_t multsCanMix(const uint8_t mult1, const uint8_t mult2){
	uint8_t mOrder1 = (uint8_t)((mult1 >> 1U) & 15U);
	uint8_t mEM1 = (uint8_t)(mult1 & 1U);
	uint8_t mDQ1 = (uint8_t)((mult1 >> 7U) & 1U);
	uint8_t mOrder2 = (uint8_t)((mult2 >> 1U) & 15U);
	uint8_t mEM2 = (uint8_t)(mult2 & 1U);
	uint8_t mDQ2 = (uint8_t)((mult2 >> 7U) & 1U);

	if(mDQ1 != mDQ2){
		return 0;
	}else if(((mOrder1 + mOrder2) % 2) == 0){
		//both orders are even or both are odd, can only occur
		//if both multipolarities are M or both are E
		if(mEM1 == mEM2){
			return 1;
		}else{
			return 0;
		}
	}else{
		if(mEM1 == mEM2){
			return 0;
		}else{
			return 1;
		}
	}
}

void getQValStr(char strOut[32], const valWithErr qVal, const uint8_t showErr){
	uint8_t qPrecision = (uint8_t)(qVal.format & 15U);
	uint8_t qExponent = (uint8_t)((qVal.format >> 4U) & 1U);
	if((showErr == 0)||(qVal.err == 0)){
		if(qExponent == 0){
			SDL_snprintf(strOut,32,"%.*f",qPrecision,(double)(qVal.val));
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i",qPrecision,(double)(qVal.val),qVal.exponent);
		}
	}else{
		if(qExponent == 0){
			if(qVal.err == 255){
				//systematic
				SDL_snprintf(strOut,32,"%.*f(sys.)",qPrecision,(double)(qVal.val));
			}else{
				SDL_snprintf(strOut,32,"%.*f(%u)",qPrecision,(double)(qVal.val),qVal.err);
			}
		}else{
			if(qVal.err == 255){
				SDL_snprintf(strOut,32,"%.*f(sys.)E%i",qPrecision,(double)(qVal.val),qVal.exponent);
			}else{
				SDL_snprintf(strOut,32,"%.*f(%u)E%i",qPrecision,(double)(qVal.val),qVal.err,qVal.exponent);
			}
		}
	}
}

void getGammaEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr){

	uint8_t ePrecision = (uint8_t)(nd->tran[tranInd].energy.format & 15U);
	uint8_t eExponent = (uint8_t)((nd->tran[tranInd].energy.format >> 4U) & 1U);
	if((showErr == 0)||(nd->tran[tranInd].energy.err == 0)){
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f",ePrecision,(double)(nd->tran[tranInd].energy.val));
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
		}
	}else{
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f(%u)",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err);
		}else{
			SDL_snprintf(strOut,32,"%.*f(%u)E%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err,nd->tran[tranInd].energy.exponent);
		}
	}
	
}

void getGammaIntensityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr){

	uint8_t iPrecision = (uint8_t)(nd->tran[tranInd].intensity.format & 15U);
	uint8_t iExponent = (uint8_t)((nd->tran[tranInd].intensity.format >> 4U) & 1U);
	uint8_t iValueType = (uint8_t)((nd->tran[tranInd].intensity.format >> 5U) & 15U);
	if(nd->tran[tranInd].intensity.val <= 0.0f){
		SDL_snprintf(strOut,32," ");
	}else if((showErr == 0)||(nd->tran[tranInd].intensity.err == 0)){
		if(iExponent == 0){
			SDL_snprintf(strOut,32,"%s%.*f",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val));
		}else{
			SDL_snprintf(strOut,32,"%s%.*fE%i",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.exponent);
		}
	}else{
		if(iExponent == 0){
			SDL_snprintf(strOut,32,"%s%.*f(%u)",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.err);
		}else{
			SDL_snprintf(strOut,32,"%s%.*f(%u)E%i",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.err,nd->tran[tranInd].intensity.exponent);
		}
	}
	
}

void getGammaMultipolarityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd){

	strcpy(strOut,""); //clear the string
	uint8_t tentative = 0;
	uint8_t derived = 0;

	for(uint8_t i=0; i<nd->tran[tranInd].numMultipoles; i++){
		uint8_t mTentative = (uint8_t)((nd->tran[tranInd].multipole[i] >> 5U) & 3U);
		uint8_t mOrder = (uint8_t)((nd->tran[tranInd].multipole[i] >> 1U) & 15U);
		uint8_t mEM = (uint8_t)(nd->tran[tranInd].multipole[i] & 1U);
		uint8_t mDQ = (uint8_t)((nd->tran[tranInd].multipole[i] >> 7U) & 1U);
		if(mTentative == TENTATIVEMULT_YES){
			if(tentative == 0){
				tentative = 1;
				strcat(strOut,"(");
			}
		}else if(mTentative == TENTATIVEMULT_DERIVED){
			if(derived == 0){
				derived = 1;
				strcat(strOut,"[");
			}
		}else if(mTentative == TENTATIVESP_NONE){
			if(tentative == 1){
				tentative = 0;
				strcat(strOut,")");
			}
			if(derived == 1){
				derived = 0;
				strcat(strOut,"]");
			}
		}
		if(i>0){
			if(multsCanMix(nd->tran[tranInd].multipole[i],nd->tran[tranInd].multipole[i-1])){
				strcat(strOut,"+");
			}else{
				strcat(strOut,",");
			}
		}

		if(mDQ){
			//dipole/quadrupole assignment
			//check placeholder multipole values to determine which is which
			if((mEM == 0)&&(mOrder == 2)){
				strcat(strOut,"Q");
			}else if((mEM == 1)&&(mOrder == 1)){
				strcat(strOut,"D");
			}
		}else{
			if(mEM){
				strcat(strOut,"M");
			}else{
				strcat(strOut,"E");
			}
			char order[16];
		  SDL_snprintf(order,16,"%u",mOrder);
			strcat(strOut,order);
		}

		if(i==(nd->tran[tranInd].numMultipoles-1)){
			if(derived == 1){
				strcat(strOut,"]");
			}
			if(tentative == 1){
				strcat(strOut,")");
			}
		}
	}
}

void getLvlEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr){

	uint8_t ePrecision = (uint8_t)(nd->levels[lvlInd].energy.format & 15U);
	uint8_t eExponent = (uint8_t)((nd->levels[lvlInd].energy.format >> 4U) & 1U);
	uint8_t eValueType = (uint8_t)((nd->levels[lvlInd].energy.format >> 5U) & 15U);
	if(eValueType == VALUETYPE_X){
		uint8_t variable = (uint8_t)((nd->levels[lvlInd].energy.format >> 9U) & 127U);
		SDL_snprintf(strOut,32,"%c",variable);
	}else if(eValueType == VALUETYPE_PLUSX){
		uint8_t variable = (uint8_t)((nd->levels[lvlInd].energy.format >> 9U) & 127U);
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f+%c",ePrecision,(double)(nd->levels[lvlInd].energy.val),variable);
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i+%c",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.exponent,variable);
		}
	}else if((showErr == 0)||(nd->levels[lvlInd].energy.err == 0)){
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f",ePrecision,(double)(nd->levels[lvlInd].energy.val));
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.exponent);
		}
	}else{
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f(%u)",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.err);
		}else{
			SDL_snprintf(strOut,32,"%.*f(%u)E%i",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.err,nd->levels[lvlInd].energy.exponent);
		}
	}
	
}

void getHalfLifeStr(char strOut[32], const app_data *restrict dat, const uint32_t lvlInd, const uint8_t showErr, const uint8_t showUnknown, const uint8_t useLifetime){
	if(lvlInd < dat->ndat.numLvls){
		if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
			SDL_snprintf(strOut,32,"STABLE");
		}else if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_NOVAL){
			if(showUnknown){
				SDL_snprintf(strOut,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
			}else{
				SDL_snprintf(strOut,32," ");
			}
		}else if(dat->ndat.levels[lvlInd].halfLife.val > 0.0f){
			double hlVal = (double)(dat->ndat.levels[lvlInd].halfLife.val);
			if(useLifetime){
				hlVal *= 1.4427; //convert half-life to lifetime
			} 
			uint8_t hlPrecision = (uint8_t)(dat->ndat.levels[lvlInd].halfLife.format & 15U);
			uint8_t hlExponent = (uint8_t)((dat->ndat.levels[lvlInd].halfLife.format >> 4U) & 1U);
			uint8_t hlValueType = (uint8_t)((dat->ndat.levels[lvlInd].halfLife.format >> 5U) & 15U);
			uint8_t hlErr = dat->ndat.levels[lvlInd].halfLife.err;
			if(useLifetime){
				hlErr = (uint8_t)(SDL_ceil((double)hlErr * 1.4427)); //convert half-life error to lifetime error
			}
			if(hlValueType == VALUETYPE_ASYMERROR){
				uint8_t negErr = (uint8_t)((dat->ndat.levels[lvlInd].halfLife.format >> 9U) & 127U);
				if(useLifetime){
					negErr = (uint8_t)(SDL_ceil((double)negErr * 1.4427)); //convert half-life error to lifetime error
				}
				if(hlExponent == 0){
					SDL_snprintf(strOut,32,"%.*f(+%u-%u) %s",hlPrecision,hlVal,hlErr,negErr,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
				}else{
					SDL_snprintf(strOut,32,"%.*f(+%u-%u)E%i %s",hlPrecision,hlVal,hlErr,negErr,dat->ndat.levels[lvlInd].halfLife.exponent,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
				}
			}else{
				if((showErr == 0)||(hlErr == 0)){
					if(hlExponent == 0){
						SDL_snprintf(strOut,32,"%s%.*f %s",getValueTypeShortStr(hlValueType),hlPrecision,hlVal,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
					}else{
						SDL_snprintf(strOut,32,"%s%.*fE%i %s",getValueTypeShortStr(hlValueType),hlPrecision,hlVal,dat->ndat.levels[lvlInd].halfLife.exponent,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
					}
				}else{
					if(hlExponent == 0){
						SDL_snprintf(strOut,32,"%s%.*f(%u) %s",getValueTypeShortStr(hlValueType),hlPrecision,hlVal,hlErr,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
					}else{
						SDL_snprintf(strOut,32,"%s%.*f(%u)E%i %s",getValueTypeShortStr(hlValueType),hlPrecision,hlVal,hlErr,dat->ndat.levels[lvlInd].halfLife.exponent,getValueUnitShortStr(dat->ndat.levels[lvlInd].halfLife.unit));
					}
				}
			}
		}else{
			SDL_snprintf(strOut,32," ");
		}
	}else{
		if(showUnknown){
			SDL_snprintf(strOut,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
		}else{
			SDL_snprintf(strOut,32," ");
		}
	}
}
void getGSHalfLifeStr(char strOut[32], const app_data *restrict dat, const uint16_t nuclInd, const uint8_t useLifetime){
	if(dat->ndat.nuclData[nuclInd].numLevels > 0){
		getHalfLifeStr(strOut,dat,dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel,1,1,useLifetime);
	}else{
		SDL_snprintf(strOut,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
	}
}

void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd){
	if(dcyModeInd < nd->numDecModes){
		uint8_t decUnitType = nd->dcyMode[dcyModeInd].prob.unit;
		uint8_t decType = nd->dcyMode[dcyModeInd].type;
		uint8_t decPrecision = (uint8_t)(nd->dcyMode[dcyModeInd].prob.format & 15U);
		uint8_t decValueType = (uint8_t)((nd->dcyMode[dcyModeInd].prob.format >> 5U) & 15U);
		uint8_t decExponent = (uint8_t)((nd->dcyMode[dcyModeInd].prob.format >> 4U) & 1U);
		if(decUnitType == VALUETYPE_NUMBER){
			if(decValueType == VALUETYPE_ASYMERROR){
				uint8_t negErr = (uint8_t)((nd->dcyMode[dcyModeInd].prob.format >> 9U) & 127U);
				if(decExponent == 0){
					SDL_snprintf(strOut,32,"%s = %.*f(+%u-%u)%%%%",getDecayTypeShortStr(decType),decPrecision,(double)(nd->dcyMode[dcyModeInd].prob.val),nd->dcyMode[dcyModeInd].prob.err,negErr);
				}else{
					SDL_snprintf(strOut,32,"%s = %.*f(+%u-%u)E%i%%%%",getDecayTypeShortStr(decType),decPrecision,(double)(nd->dcyMode[dcyModeInd].prob.val),nd->dcyMode[dcyModeInd].prob.err,negErr,nd->dcyMode[dcyModeInd].prob.exponent);
				}
			}else{
				if(nd->dcyMode[dcyModeInd].prob.err > 0){
					if(decExponent == 0){
						SDL_snprintf(strOut,32,"%s = %.*f(%u)%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.err); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
					}else{
						SDL_snprintf(strOut,32,"%s = %.*f(%u)E%i%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.err,nd->dcyMode[dcyModeInd].prob.exponent); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
					}
				}else{
					if(decExponent == 0){
						SDL_snprintf(strOut,32,"%s = %.*f%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
					}else{
						SDL_snprintf(strOut,32,"%s = %.*fE%i%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.exponent); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
					}
				}
			}
		}else if(decUnitType == VALUETYPE_UNKNOWN){
			SDL_snprintf(strOut,32,"%s%s",getDecayTypeShortStr(decType),getValueTypeShortStr(decUnitType));
		}else{
			if(decExponent == 0){
				SDL_snprintf(strOut,32,"%s %s%.*f%%%%",getDecayTypeShortStr(decType),getValueTypeShortStr(decUnitType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
			}else{
				SDL_snprintf(strOut,32,"%s %s%.*fE%i%%%%",getDecayTypeShortStr(decType),getValueTypeShortStr(decUnitType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.exponent); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
			}
		}
	}else{
		SDL_snprintf(strOut,32," ");
	}
}

void getAbundanceStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd){
	if(nuclInd < nd->numNucl){
		if(nd->nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
			uint8_t abPrecision = (uint8_t)(nd->nuclData[nuclInd].abundance.format & 15U);
			SDL_snprintf(strOut,32,"%.*f%%%%",abPrecision,(double)nd->nuclData[nuclInd].abundance.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
		}else{
			SDL_snprintf(strOut,32," ");
		}
	}else{
		SDL_snprintf(strOut,32," ");
	}
}

void getSpinParStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd){

	char val[16];

	strcpy(strOut,""); //clear the string

	for(int i=0;i<nd->levels[lvlInd].numSpinParVals;i++){
		
		//SDL_Log("Spin: %i, parity: %i, tentative: %u\n\n",nd->levels[lvlInd].spval[i].spinVal,nd->levels[lvlInd].spval[i].parVal,tentative);
		
		uint8_t tentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i].format >> 9U) & 7U);
		uint8_t prevTentative = 0;
		if(i>0){
			prevTentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i-1].format >> 9U) & 7U);
		}
		uint8_t nextTentative = 0;
		if(i<nd->levels[lvlInd].numSpinParVals-1){
			nextTentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i+1].format >> 9U) & 7U);
		}

		if(tentative == TENTATIVESP_RANGE){
			strcat(strOut,"to");
		}else{

			uint8_t spinIsVar = (uint8_t)(nd->levels[lvlInd].spval[i].format & 1U);
			uint8_t spinVarInd = (uint8_t)((nd->levels[lvlInd].spval[i].format >> 5U) & 31U);
			uint8_t spinValType = (uint8_t)((nd->levels[lvlInd].spval[i].format >> 1U) & 15U);
			
			if((tentative == TENTATIVESP_SPINANDPARITY)||(tentative == TENTATIVESP_SPINONLY)){
				if((i==0)||((i>0)&&((prevTentative != TENTATIVESP_SPINANDPARITY)&&(prevTentative != TENTATIVESP_SPINONLY)))){
					if((i>0)&&(prevTentative == TENTATIVESP_RANGE)){
						//previous spin parity value specified a range
						strcat(strOut," ");
					}else{
						strcat(strOut,"(");
					}
				}
			}

			if(spinIsVar){
				if(spinValType != VALUETYPE_NUMBER){
					strcat(strOut,getValueTypeShortStr(spinValType));
				}
				if(nd->levels[lvlInd].spval[i].spinVal == 0){
					//variable only
					if(spinVarInd == 0){
						strcat(strOut,"J");
					}else{
						sprintf(val,"J%u",spinVarInd);
						strcat(strOut,val);
					}
				}else{
					//variable + offset
					if(spinVarInd == 0){
						strcat(strOut,"J+");
					}else{
						sprintf(val,"J%u+",spinVarInd);
						strcat(strOut,val);
					}
				}
			}

			if((!spinIsVar)||(nd->levels[lvlInd].spval[i].spinVal > 0)){
				if(spinValType != VALUETYPE_NUMBER){
					strcat(strOut,getValueTypeShortStr(spinValType));
				}
				if(nd->levels[lvlInd].spval[i].spinVal < 255){
					if((nd->levels[lvlInd].format & 1U) == 1){
						sprintf(val,"%i/2",nd->levels[lvlInd].spval[i].spinVal);
					}else{
						sprintf(val,"%i",nd->levels[lvlInd].spval[i].spinVal);
					}
					strcat(strOut,val);
				}
			}
			if((tentative != TENTATIVESP_SPINONLY)&&(tentative != TENTATIVESP_PARITYONLY)){
				if(nd->levels[lvlInd].spval[i].parVal == -1){
					strcat(strOut,"-");
				}else if(nd->levels[lvlInd].spval[i].parVal == 1){
					strcat(strOut,"+");
				}
			}
			if((tentative == TENTATIVESP_SPINANDPARITY)||(tentative == TENTATIVESP_SPINONLY)){
				if(i==nd->levels[lvlInd].numSpinParVals-1){
					strcat(strOut,")");
					if(tentative == TENTATIVESP_SPINONLY){
						if(nd->levels[lvlInd].spval[i].parVal == -1){
							strcat(strOut,"-");
						}else if(nd->levels[lvlInd].spval[i].parVal == 1){
							strcat(strOut,"+");
						}
					}
				}else if(i<nd->levels[lvlInd].numSpinParVals-1){
					if(nextTentative != TENTATIVESP_RANGE){
						if(nextTentative != TENTATIVESP_SPINANDPARITY){
							if(nextTentative != TENTATIVESP_SPINONLY){
								strcat(strOut,")");
								if(tentative == TENTATIVESP_SPINONLY){
									if(nd->levels[lvlInd].spval[i].parVal == -1){
										strcat(strOut,"-");
									}else if(nd->levels[lvlInd].spval[i].parVal == 1){
										strcat(strOut,"+");
									}
								}
							}
						}
					}
				}
			}else if(tentative == TENTATIVESP_PARITYONLY){
				if(nd->levels[lvlInd].spval[i].parVal == -1){
					strcat(strOut,"(-)");
				}else if(nd->levels[lvlInd].spval[i].parVal == 1){
					strcat(strOut,"(+)");
				}
			}
			if(i!=nd->levels[lvlInd].numSpinParVals-1){
				if(nextTentative == TENTATIVESP_RANGE){
					//next spin parity value specifies a range
					strcat(strOut," ");
				}else{
					strcat(strOut,",");
				}
			}
		}
		
	}
}

double getRawValFromDB(const valWithErr *restrict valStruct){
	double val = (double)(valStruct->val);
	if(((valStruct->format >> 4U) & 1U) != 0){
		//value in exponent form
		val = val * pow(10.0,(double)(valStruct->exponent));
	}
	return val;
}

double getRawErrFromDB(const valWithErr *restrict valStruct){
	if(valStruct->err == 0){
		return 0.0;
	}
	uint8_t numSigFigs = (uint8_t)(valStruct->format & 15U);
	double err = (double)(valStruct->err);
	if(numSigFigs > 0){
		err = err/(1.0*numSigFigs);
	}
	if(((valStruct->format >> 4U) & 1U) != 0){
		//value in exponent form
		err = err * pow(10.0,(double)(valStruct->exponent));
	}
	return err;
}

double getLevelEnergykeV(const ndata *restrict nd, const uint32_t levelInd){
	if(levelInd < nd->numLvls){
		double levelE = getRawValFromDB(&nd->levels[levelInd].energy);
		if(levelE < 0.0){
			//unknown level energy
			return -2.0;
		}
		uint8_t eUnit = nd->levels[levelInd].energy.unit;
		switch(eUnit){
			case VALUE_UNIT_EV:
				return levelE/1000.0;
			case VALUE_UNIT_KEV:
				return levelE;
			case VALUE_UNIT_MEV:
				return levelE*1000;
			default:
				return -2.0; //couldn't find level energy
		}
	}else{
		return -2.0; //couldn't find level energy
	}
}

uint32_t get4PlusLvlInd(const ndata *restrict nd, const uint16_t nuclInd){
	if((nd->nuclData[nuclInd].N + nd->nuclData[nuclInd].Z) > 0){
		if((nd->nuclData[nuclInd].N % 2)==0){
			if((nd->nuclData[nuclInd].Z % 2)==0){
				for(uint16_t i=0; i<nd->nuclData[nuclInd].numLevels; i++){
					if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].numSpinParVals == 1){
						if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].spval[0].spinVal == 4){
							if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].spval[0].parVal == 1){
								//one of the spin-parity values is 4+
								uint8_t eValueType = (uint8_t)((nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].energy.format >> 5U) & 15U);
								if(eValueType == VALUETYPE_NUMBER){
									//not some weird offset or variable energy, use this level
									return (uint32_t)(nd->nuclData[nuclInd].firstLevel + (uint32_t)i);
								}
							}
						}
					}
				}
			}
		}
	}
	return MAXNUMLVLS;
}

uint32_t get2PlusLvlInd(const ndata *restrict nd, const uint16_t nuclInd){
	if((nd->nuclData[nuclInd].N + nd->nuclData[nuclInd].Z) > 0){
		if((nd->nuclData[nuclInd].N % 2)==0){
			if((nd->nuclData[nuclInd].Z % 2)==0){
				for(uint16_t i=0; i<nd->nuclData[nuclInd].numLevels; i++){
					if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].numSpinParVals == 1){
						if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].spval[0].spinVal == 2){
							if(nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].spval[0].parVal == 1){
								//one of the spin-parity values is 2+
								uint8_t eValueType = (uint8_t)((nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)i].energy.format >> 5U) & 15U);
								if(eValueType == VALUETYPE_NUMBER){
									//not some weird offset or variable energy, use this level
									return (uint32_t)(nd->nuclData[nuclInd].firstLevel + (uint32_t)i);
								}
							}
						}
					}
				}
			}
		}
	}
	return MAXNUMLVLS;
}

//get the energy ratio of the first 4+ state to the first 2+ state, for even-even nuclei
double getR42(const ndata *restrict nd, const uint16_t nuclInd){
	uint32_t lvlInd2 = get2PlusLvlInd(nd,nuclInd);
	uint32_t lvlInd4 = get4PlusLvlInd(nd,nuclInd);
	if((lvlInd2 != MAXNUMLVLS)&&(lvlInd4 != MAXNUMLVLS)){
		return (getLevelEnergykeV(nd,lvlInd4)/getLevelEnergykeV(nd,lvlInd2));
	}
	return -1.0; //no 2+ state, or not even-even
}

//get the energy of the first 2+ state, for even-even nuclei
double get2PlusEnergy(const ndata *restrict nd, const uint16_t nuclInd){
	uint32_t lvlInd = get2PlusLvlInd(nd,nuclInd);
	if(lvlInd != MAXNUMLVLS){
		return getLevelEnergykeV(nd,lvlInd);
	}
	return -1.0; //no 2+ state, or not even-even
}

double getLevelHalfLifeSeconds(const ndata *restrict nd, const uint32_t levelInd){
	if(levelInd < nd->numLvls){
		double hl = getRawValFromDB(&nd->levels[levelInd].halfLife);
		if(hl < 0.0){
			//unknown half-life
			return -2.0;
		}
		uint8_t hlUnit = nd->levels[levelInd].halfLife.unit;
		switch(hlUnit){
			case VALUE_UNIT_STABLE:
				return 1.0E30; //stable
			case VALUE_UNIT_YEARS:
				return hl*365.25*24*3600;
			case VALUE_UNIT_DAYS:
				return hl*24*3600;
			case VALUE_UNIT_HOURS:
				return hl*3600;
			case VALUE_UNIT_MINUTES:
				return hl*60;
			case VALUE_UNIT_SECONDS:
				return hl;
			case VALUE_UNIT_MILLISECONDS:
				return hl*0.001;
			case VALUE_UNIT_MICROSECONDS:
				return hl*0.000001;
			case VALUE_UNIT_NANOSECONDS:
				return hl*0.000000001;
			case VALUE_UNIT_PICOSECONDS:
				return hl*0.000000000001;
			case VALUE_UNIT_FEMTOSECONDS:
				return hl*0.000000000000001;
			case VALUE_UNIT_ATTOSECONDS:
				return hl*0.000000000000000001;
			case VALUE_UNIT_NOVAL:
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

uint8_t getNuclLevelMostProbableDcyMode(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel){
	
	uint32_t lvlInd = nd->nuclData[nuclInd].firstLevel + (uint32_t)nuclLevel;
	uint8_t hlUnit = nd->levels[lvlInd].halfLife.unit;
	if(hlUnit == VALUE_UNIT_STABLE){
		return (DECAYMODE_ENUM_LENGTH+1); //stable
	}else if((nd->levels[lvlInd].numDecModes == 0)&&(getLevelHalfLifeSeconds(nd,lvlInd)>1.0E15)){
		return (DECAYMODE_ENUM_LENGTH+1); //roughly stable
	}

	double maxProb = -1.0;
	int8_t maxProbInd = -1;
	for(int8_t i=0; i<nd->levels[lvlInd].numDecModes; i++){
		uint32_t dcyModeInd = nd->levels[lvlInd].firstDecMode + (uint32_t)i;
		uint8_t decUnitType = nd->dcyMode[dcyModeInd].prob.unit;
		if(decUnitType < VALUETYPE_ENUM_LENGTH){
			double prob = getRawValFromDB(&nd->dcyMode[dcyModeInd].prob);
			if(prob > maxProb){
				maxProb = prob;
				maxProbInd = i;
			}
		}		
	}

	if(maxProbInd >= 0){
		return nd->dcyMode[nd->levels[nd->nuclData[nuclInd].firstLevel + (uint32_t)nuclLevel].firstDecMode + (uint32_t)maxProbInd].type;
	}

	return DECAYMODE_ENUM_LENGTH; //no decay mode found
}

uint8_t getNuclGSMostProbableDcyMode(const ndata *restrict nd, const uint16_t nuclInd){
	uint8_t dcyMode = getNuclLevelMostProbableDcyMode(nd,nuclInd,nd->nuclData[nuclInd].gsLevel);
	if((dcyMode != DECAYMODE_ENUM_LENGTH)&&(dcyMode != DECAYMODE_IT)){
		return dcyMode;
	}else{
		//check for decay modes in the first few levels instead
		for(uint16_t i=0; i<10; i++){
			if(i<nd->nuclData[nuclInd].numLevels){
				dcyMode = getNuclLevelMostProbableDcyMode(nd,nuclInd,i);
				if((dcyMode != DECAYMODE_ENUM_LENGTH)&&(dcyMode != DECAYMODE_IT)){
					return dcyMode;
				}
			}else{
				break;
			}
		}
	}

	//if no decay mode is listed, maybe we can infer one from the GS Q-values
	if((nd->nuclData[nuclInd].qbeta.val > 0.0f)&&(nd->nuclData[nuclInd].qalpha.val < 0.0f)){
		return DECAYMODE_BETAMINUS;
	}else if((nd->nuclData[nuclInd].qbeta.val < 0.0f)&&(nd->nuclData[nuclInd].qalpha.val < 0.0f)){
		return DECAYMODE_ECANDBETAPLUS;
	}

	return DECAYMODE_ENUM_LENGTH;
	
}


uint32_t getFinalLvlInd(const ndata *restrict nd, const uint32_t initialLevel, const uint32_t tran){
	return (uint32_t)(initialLevel - nd->tran[tran].finalLvlOffset);
}

uint16_t getNuclInd(const ndata *restrict nd, const int16_t N, const int16_t Z){
	for(uint16_t i=0; i<nd->numNucl;i++){
		if(nd->nuclData[i].Z == Z){
			if(nd->nuclData[i].N == N){
				if((nd->nuclData[i].flags & 3U) == OBSFLAG_OBSERVED){
					return i;
				}
			}
		}
	}
	//SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"getNuclInd - couldn't find nucleus with N,Z = [%i %i].\n",N,Z);
	return MAXNUMNUCL;
}

uint16_t getNumScreenLvlDispLines(const drawing_state *restrict ds){
	return (uint16_t)(SDL_floorf((ds->windowYRes - NUCL_FULLINFOBOX_LEVELLIST_POS_Y)/(NUCL_INFOBOX_SMALLLINE_HEIGHT*ds->uiUserScale) - 2*ds->uiUserScale)); //somewhat hacky, to make sure all levels are visible on all UI scales
}
uint16_t getNumTotalLvlDispLines(const ndata *restrict nd, const app_state *restrict state){
	uint16_t numLines = 0;
	for(uint32_t lvlInd = nd->nuclData[state->chartSelectedNucl].firstLevel; lvlInd<(nd->nuclData[state->chartSelectedNucl].firstLevel + nd->nuclData[state->chartSelectedNucl].numLevels); lvlInd++){
		numLines += getNumDispLinesForLvl(nd,lvlInd);
	}
	return numLines;
}

uint16_t getNumDispLinesForLvl(const ndata *restrict nd, const uint32_t lvlInd){
  uint16_t levelNumLines = 1;
  if(nd->levels[lvlInd].numDecModes > 0){
    levelNumLines += (uint16_t)(nd->levels[lvlInd].numDecModes);
  }
  if(nd->levels[lvlInd].numTran > levelNumLines){
    levelNumLines = (uint16_t)(nd->levels[lvlInd].numTran);
  }
	if(levelNumLines < 2){
		uint8_t slInd = (uint8_t)((nd->levels[lvlInd].format >> 1U) & 127U);
		if(slInd > 0){
			levelNumLines++;
		}
	}
  return levelNumLines;
}

uint16_t getMaxNumLvlDispLines(const ndata *restrict nd, const app_state *restrict state){
	//find total number of lines displayable
	uint16_t numLines = getNumTotalLvlDispLines(nd,state);
	const uint16_t numScreenLines = getNumScreenLvlDispLines(&state->ds);
	if(numLines > numScreenLines){
		numLines -= numScreenLines;
	}else{
		numLines = 0;
	}
	//SDL_Log("max line: %u, screen lines: %u\n",numLines,numScreenLines);
	return numLines;
}

float mouseXPxToN(const drawing_state *restrict ds, const float mouseX){
	return ds->chartPosX + ((mouseX - ds->windowXRes/(2.0f))/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale));
}
float mouseYPxToZ(const drawing_state *restrict ds, const float mouseY){
	return ds->chartPosY - ((mouseY - ds->windowYRes/(2.0f))/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale));
}
float chartNtoXPx(const drawing_state *restrict ds, const float N){
	return (ds->windowXRes/(2.0f)) + (N - ds->chartPosX)*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale;
}
float chartZtoYPx(const drawing_state *restrict ds, const float Z){
	return (ds->windowYRes/(2.0f)) + (ds->chartPosY - Z)*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale;
}
float getMinChartN(const drawing_state *restrict ds){
	return ds->chartPosX - (ds->windowXRes/ds->uiUserScale)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMaxChartN(const drawing_state *restrict ds){
	return ds->chartPosX + (ds->windowXRes/ds->uiUserScale)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMinChartZ(const drawing_state *restrict ds){
	return ds->chartPosY - (ds->windowYRes/ds->uiUserScale)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getMaxChartZ(const drawing_state *restrict ds){
	return ds->chartPosY + (ds->windowYRes/ds->uiUserScale)/(2.0f*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale);
}
float getChartWidthN(const drawing_state *restrict ds){
	return (ds->windowXRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale));
}
float getChartHeightZ(const drawing_state *restrict ds){
	return (ds->windowYRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale*ds->uiUserScale));
}
float getChartWidthNAfterZoom(const drawing_state *restrict ds){
	return (ds->windowXRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomToScale*ds->uiUserScale));
}
float getChartHeightZAfterZoom(const drawing_state *restrict ds){
	return (ds->windowYRes/(DEFAULT_NUCLBOX_DIM*ds->chartZoomToScale*ds->uiUserScale));
}

//change the modal state of the UI, and update which UI elements are interactable
void changeUIState(const app_data *restrict dat, app_state *restrict state, const uint8_t newState){
  
	const uint8_t prevMouseover = state->mouseoverElement;
  state->interactableElement = 0;
  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' any buttons
	if(newState != state->uiState){
		//^only update the last UI state if the UI state is actually being changed
		state->lastUIState = state->uiState; //useful to remember old UI states, for modal dialogs
  	state->uiState = newState;
	}
  
  switch(state->uiState){
    case UISTATE_MSG_BOX:
      state->interactableElement |= (uint32_t)(1U << UIELEM_MSG_BOX_OK_BUTTON);
      break;
		case UISTATE_ABOUT_BOX:
			state->interactableElement |= (uint32_t)(1U << UIELEM_ABOUT_BOX_OK_BUTTON);
			break;
		case UISTATE_PREFS_DIALOG:
			state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_DIALOG_CLOSEBUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
			if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
				state->interactableElement |= (uint32_t)(1U << UIELEM_UISM_SMALL_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_UISM_DEFAULT_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_UISM_LARGE_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_UISM_HUGE_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_PREFS_UISCALE_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)(UIELEM_PREFS_UISCALE_MENU - UISCALE_ENUM_LENGTH); //select the first menu item
				}
			}else{
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					if((prevMouseover < UIELEM_PREFS_UISCALE_MENU)&&(prevMouseover >= (UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH))){
						//was just in the UI scale dropdown
						state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
					}else{
						state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG - PREFS_DIALOG_NUM_UIELEMENTS); //select the first menu item
					}
				}
			}
			break;
		case UISTATE_FULLLEVELINFO:
		case UISTATE_FULLLEVELINFOWITHMENU:
			state->ds.nuclFullInfoMaxScrollY = getMaxNumLvlDispLines(&dat->ndat,state); //find total number of lines displayable
			state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_FULLINFOBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_FULLINFOBOX_BACKBUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_FULLINFOBOX_SCROLLBAR);
			if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				state->interactableElement |= (uint32_t)(1U << UIELEM_PM_PREFS_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_PM_ABOUT_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_PRIMARY_MENU);
			}
			break;
		case UISTATE_INFOBOX:
			state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_CHARTVIEW_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			break;
		case UISTATE_CHARTWITHMENU:
			state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_CHARTVIEW_BUTTON);
			if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				state->interactableElement |= (uint32_t)(1U << UIELEM_PM_PREFS_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_PM_ABOUT_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_PRIMARY_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU - PRIMARY_MENU_NUM_UIELEMENTS); //select the first menu item
				}
			}
			if((state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
				state->interactableElement |= (uint32_t)(1U << UIELEM_CVM_HALFLIFE_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_CVM_DECAYMODE_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_CVM_2PLUS_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_CVM_R42_BUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_CHARTVIEW_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)(UIELEM_CHARTVIEW_MENU - CHARTVIEW_ENUM_LENGTH); //select the first menu item
				}
			}
			break;
    case UISTATE_CHARTONLY:
    default:
      state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_CHARTVIEW_BUTTON);
      break;
  }
	//SDL_Log("Mouseover element: %u\n",state->mouseoverElement);
}

void panChartToPos(const app_data *restrict dat, drawing_state *restrict ds, const uint16_t posN, const uint16_t posZ, float panTime){
	ds->chartPanStartX = ds->chartPosX;
	ds->chartPanStartY = ds->chartPosY;
	ds->chartPanToX = posN*1.0f + 0.5f;
	ds->chartPanToY = posZ*1.0f - 0.5f - (16.0f/ds->chartZoomScale);
	//SDL_Log("pos: %u %u, panning to: %f %f\n",posN,posZ,(double)ds->chartPanToX,(double)ds->chartPanToY);
	//clamp chart display range
	if(ds->chartPanToX < (-0.25f*getChartWidthN(ds))){
		ds->chartPanToX = (-0.25f*getChartWidthN(ds));
	}else if(ds->chartPanToX > (dat->ndat.maxN+(0.25f*getChartWidthN(ds)))){
		ds->chartPanToX = (float)dat->ndat.maxN+(0.25f*getChartWidthN(ds));
	}
	if(ds->chartPanToY < (-0.25f*getChartHeightZ(ds))){
		ds->chartPanToY = (-0.25f*getChartHeightZ(ds));
	}else if(ds->chartPanToY > (dat->ndat.maxZ+(0.25f*getChartHeightZ(ds)))){
		ds->chartPanToY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZ(ds));
	}
	//SDL_Log("panning to: %f %f\n",(double)ds->chartPanToX,(double)ds->chartPanToY);
	ds->timeSincePanStart = 0.0f;
	ds->totalPanTime = panTime;
	ds->panInProgress = 1;
	ds->panFinished = 0;
}


//finds the nearest nuclide to the coordinates N,Z (values can be negative)
//and returns its index
uint16_t getNearestNuclInd(const app_data *restrict dat, const int16_t N, const int16_t Z){
	int16_t selectedN = N;
  int16_t selectedZ = Z;
	uint8_t loopCtr = 0;
	uint16_t nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
	while(nuclInd == MAXNUMNUCL){
		//try to guess closest
		if(loopCtr > 10){
			return MAXNUMNUCL; //safety valve
		}
		if((N <= 0)&&(Z <= 0)){
			if(N < Z){
				selectedN = 0;
				selectedZ = 1;
			}else{
				selectedN = 0;
				selectedZ = 1;
			}
		}else if((selectedN >= dat->ndat.maxN)&&(selectedZ >= dat->ndat.maxZ)){
			if((selectedN > dat->ndat.maxN)||(selectedZ > dat->ndat.maxZ)){
				if(selectedN > dat->ndat.maxN){
					selectedN = (int16_t)(dat->ndat.maxN);
				}
				if(selectedZ > dat->ndat.maxZ){
					selectedZ = (int16_t)(dat->ndat.maxZ);
				}
			}else{
				selectedN -= 1;
				selectedZ -= 1;
			}
		}else if(selectedN > selectedZ){
			//go towards N=Z from neutron rich side
			for(uint8_t i=0; i<100; i++){
				selectedN -= 1;
				selectedZ += 1;
				nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
				if(nuclInd != MAXNUMNUCL){
					return nuclInd;
				}
			}
			//if that didn't work, try the opposite direction
			selectedN += 100;
			selectedZ -= 100;
			for(uint8_t i=0; i<100; i++){
				selectedN += 1;
				selectedZ -= 1;
				nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
				if(nuclInd != MAXNUMNUCL){
					return nuclInd;
				}
			}
			break; //don't do any more loops
		}else{
			//go towards N=Z from neutron deficient side
			for(uint8_t i=0; i<100; i++){
				selectedN += 1;
				selectedZ -= 1;
				nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
				if(nuclInd != MAXNUMNUCL){
					return nuclInd;
				}
			}
			//if that didn't work, try the opposite direction
			selectedN -= 100;
			selectedZ += 100;
			for(uint8_t i=0; i<100; i++){
				selectedN -= 1;
				selectedZ += 1;
				nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
				if(nuclInd != MAXNUMNUCL){
					return nuclInd;
				}
			}
			break; //don't do any more loops
		}

		nuclInd = getNuclInd(&dat->ndat,selectedN,selectedZ);
		loopCtr++;
	}
	return nuclInd;
}

void setFullLevelInfoDimensions(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t selNucl){
	
	char tmpStr[32];
	float tmpWidth = 0.0f;
	state->ds.fullInfoElevelColWidth = NUCL_FULLINFOBOX_ENERGY_COL_MIN_WIDTH;
	state->ds.fullInfoJpiColWidth = NUCL_FULLINFOBOX_JPI_COL_MIN_WIDTH;
	state->ds.fullInfoHlColWidth = NUCL_FULLINFOBOX_HALFLIFE_COL_MIN_WIDTH;
	state->ds.fullInfoEgammaColWidth = NUCL_FULLINFOBOX_EGAMMA_COL_MIN_WIDTH;
	state->ds.fullInfoIgammaColWidth = NUCL_FULLINFOBOX_IGAMMA_COL_MIN_WIDTH;
	state->ds.fullInfoMgammaColWidth = NUCL_FULLINFOBOX_MGAMMA_COL_MIN_WIDTH;
	state->ds.fullInfoFinalElevelColWidth = NUCL_FULLINFOBOX_FINALLEVEL_E_COL_MIN_WIDTH;
	state->ds.fullInfoFinalJpiColWidth = NUCL_FULLINFOBOX_FINALLEVEL_JPI_COL_MIN_WIDTH;

	for(uint32_t lvlInd = dat->ndat.nuclData[selNucl].firstLevel; lvlInd<(dat->ndat.nuclData[selNucl].firstLevel+dat->ndat.nuclData[selNucl].numLevels); lvlInd++){
		getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
		tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
		if(tmpWidth > state->ds.fullInfoElevelColWidth){
			state->ds.fullInfoElevelColWidth = tmpWidth;
		}
		uint8_t slInd = (uint8_t)((dat->ndat.levels[lvlInd].format >> 1U) & 127U);
		if(slInd > 0){
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,getSpecialLvlStr(dat,slInd)) + 6*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoElevelColWidth){
				state->ds.fullInfoElevelColWidth = tmpWidth;
			}
		}
		getSpinParStr(tmpStr,&dat->ndat,lvlInd);
		tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
		if(tmpWidth > state->ds.fullInfoJpiColWidth){
			state->ds.fullInfoJpiColWidth = tmpWidth;
		}
		getHalfLifeStr(tmpStr,dat,lvlInd,1,0,state->ds.useLifetimes);
		tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
		if(tmpWidth > state->ds.fullInfoHlColWidth){
			state->ds.fullInfoHlColWidth = tmpWidth;
		}
		if(dat->ndat.levels[lvlInd].numDecModes > 0){
			for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
				getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
				tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 5*UI_PADDING_SIZE;
				if(tmpWidth > state->ds.fullInfoHlColWidth){
					state->ds.fullInfoHlColWidth = tmpWidth;
				}
			}
		}
		for(uint16_t i=0; i<dat->ndat.levels[lvlInd].numTran; i++){
      getGammaEnergyStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoEgammaColWidth){
				state->ds.fullInfoEgammaColWidth = tmpWidth;
			}
			getGammaIntensityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoIgammaColWidth){
				state->ds.fullInfoIgammaColWidth = tmpWidth;
			}
			getGammaMultipolarityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoMgammaColWidth){
				state->ds.fullInfoMgammaColWidth = tmpWidth;
			}
			uint32_t finalLvlInd = getFinalLvlInd(&dat->ndat,lvlInd,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
      getLvlEnergyStr(tmpStr,&dat->ndat,finalLvlInd,0);
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoFinalElevelColWidth){
				state->ds.fullInfoFinalElevelColWidth = tmpWidth;
			}
			getSpinParStr(tmpStr,&dat->ndat,finalLvlInd);
			tmpWidth = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*UI_PADDING_SIZE;
			if(tmpWidth > state->ds.fullInfoFinalJpiColWidth){
				state->ds.fullInfoFinalJpiColWidth = tmpWidth;
			}
		}
	}
	
	state->ds.fullInfoAllColWidth = state->ds.fullInfoElevelColWidth + state->ds.fullInfoJpiColWidth + state->ds.fullInfoHlColWidth + state->ds.fullInfoEgammaColWidth + state->ds.fullInfoIgammaColWidth + state->ds.fullInfoMgammaColWidth + state->ds.fullInfoFinalElevelColWidth + state->ds.fullInfoFinalJpiColWidth;
	state->ds.fullInfoAllColWidthExclM = state->ds.fullInfoElevelColWidth + state->ds.fullInfoJpiColWidth + state->ds.fullInfoHlColWidth + state->ds.fullInfoEgammaColWidth + state->ds.fullInfoIgammaColWidth + state->ds.fullInfoFinalElevelColWidth + state->ds.fullInfoFinalJpiColWidth;
	state->ds.fullInfoAllColWidthExcluMFinalJpi = state->ds.fullInfoElevelColWidth + state->ds.fullInfoJpiColWidth + state->ds.fullInfoHlColWidth + state->ds.fullInfoEgammaColWidth + state->ds.fullInfoIgammaColWidth + state->ds.fullInfoFinalElevelColWidth;

}

void setInfoBoxDimensions(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t selNucl){
	//calculate the number of unscaled pixels needed to show the ground and isomeric state info
	state->ds.infoBoxTableHeight = NUCL_INFOBOX_BIGLINE_HEIGHT;
	if(dat->ndat.levels[dat->ndat.nuclData[selNucl].firstLevel + dat->ndat.nuclData[selNucl].gsLevel].numDecModes > 1){
		state->ds.infoBoxTableHeight += NUCL_INFOBOX_SMALLLINE_HEIGHT*(dat->ndat.levels[dat->ndat.nuclData[selNucl].firstLevel + dat->ndat.nuclData[selNucl].gsLevel].numDecModes - 1);
	}
	if(dat->ndat.nuclData[selNucl].longestIsomerLevel != MAXNUMLVLS){
		if(dat->ndat.nuclData[selNucl].longestIsomerLevel != (dat->ndat.nuclData[selNucl].firstLevel + dat->ndat.nuclData[selNucl].gsLevel)){
			state->ds.infoBoxTableHeight += NUCL_INFOBOX_BIGLINE_HEIGHT;
			if(dat->ndat.levels[dat->ndat.nuclData[selNucl].longestIsomerLevel].numDecModes > 1){
				state->ds.infoBoxTableHeight += NUCL_INFOBOX_SMALLLINE_HEIGHT*(dat->ndat.levels[dat->ndat.nuclData[selNucl].longestIsomerLevel].numDecModes - 1);
			}
		}
	}

	if( dat->ndat.nuclData[selNucl].abundance.val > 0.0f){
		state->ds.infoBoxTableHeight += NUCL_INFOBOX_ABUNDANCE_LINE_HEIGHT;
	}

	//calculate the widths of each column and therefore the info box itself
	uint8_t useIsomer = 0;
	float calcOffset = 0.0f;
	uint32_t gsLvlInd = dat->ndat.nuclData[selNucl].firstLevel + dat->ndat.nuclData[selNucl].gsLevel;
	uint32_t isomerLvlInd = dat->ndat.nuclData[selNucl].longestIsomerLevel;
	if((isomerLvlInd != MAXNUMLVLS)&&(isomerLvlInd != gsLvlInd)){
		useIsomer = 1;
	}
	char tmpStr[32];
	state->ds.infoBoxEColOffset = NUCL_INFOBOX_ENERGY_COL_MIN_OFFSET;
  //width of level energy text
	getLvlEnergyStr(tmpStr,&dat->ndat,gsLvlInd,0);
	state->ds.infoBoxJpiColOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxEColOffset;
  if(useIsomer){
    getLvlEnergyStr(tmpStr,&dat->ndat,isomerLvlInd,1);
		calcOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxEColOffset;
		if(calcOffset > state->ds.infoBoxJpiColOffset){
			state->ds.infoBoxJpiColOffset = calcOffset;
		}
	}
	if(state->ds.infoBoxJpiColOffset < NUCL_INFOBOX_JPI_COL_MIN_OFFSET){
		state->ds.infoBoxJpiColOffset = NUCL_INFOBOX_JPI_COL_MIN_OFFSET;
	}
	//width of Jpi text
	getSpinParStr(tmpStr,&dat->ndat,gsLvlInd);
	state->ds.infoBoxHlColOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxJpiColOffset;
	if(useIsomer){
    getSpinParStr(tmpStr,&dat->ndat,isomerLvlInd);
		calcOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxJpiColOffset;
		if(calcOffset > state->ds.infoBoxHlColOffset){
			state->ds.infoBoxHlColOffset = calcOffset;
		}
	}
	if(state->ds.infoBoxHlColOffset < NUCL_INFOBOX_HALFLIFE_COL_MIN_OFFSET){
		state->ds.infoBoxHlColOffset = NUCL_INFOBOX_HALFLIFE_COL_MIN_OFFSET;
	}
	//width of t1/2 text
	getHalfLifeStr(tmpStr,dat,gsLvlInd,1,1,state->ds.useLifetimes);
	state->ds.infoBoxDcyModeColOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxHlColOffset;
	if(useIsomer){
    getHalfLifeStr(tmpStr,dat,isomerLvlInd,1,1,state->ds.useLifetimes);
		calcOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + UI_PADDING_SIZE + state->ds.infoBoxHlColOffset;
		if(calcOffset > state->ds.infoBoxDcyModeColOffset){
			state->ds.infoBoxDcyModeColOffset = calcOffset;
		}
	}
	if(state->ds.infoBoxDcyModeColOffset < NUCL_INFOBOX_DECAYMODE_COL_MIN_OFFSET){
		state->ds.infoBoxDcyModeColOffset = NUCL_INFOBOX_DECAYMODE_COL_MIN_OFFSET;
	}
	//width of decay mode text
	state->ds.infoBoxWidth = state->ds.infoBoxDcyModeColOffset;
	for(int8_t i=0; i<dat->ndat.levels[gsLvlInd].numDecModes; i++){
		getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLvlInd].firstDecMode + (uint32_t)i);
		calcOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*state->ds.infoBoxEColOffset + state->ds.infoBoxDcyModeColOffset;
		if(calcOffset > state->ds.infoBoxWidth){
			state->ds.infoBoxWidth = calcOffset;
		}
	}
	if(useIsomer){
		for(int8_t i=0; i<dat->ndat.levels[isomerLvlInd].numDecModes; i++){
			getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[isomerLvlInd].firstDecMode + (uint32_t)i);
			calcOffset = getTextWidthScaleIndependent(rdat,FONTSIZE_NORMAL,tmpStr) + 2*state->ds.infoBoxEColOffset + state->ds.infoBoxDcyModeColOffset;
			if(calcOffset > state->ds.infoBoxWidth){
				state->ds.infoBoxWidth = calcOffset;
			}
		}
	}
	if(state->ds.infoBoxWidth < NUCL_INFOBOX_MIN_WIDTH){
		state->ds.infoBoxWidth = NUCL_INFOBOX_MIN_WIDTH;
	}

	updateSingleUIElemPosition(dat,&state->ds,rdat,UIELEM_NUCL_INFOBOX);
	updateSingleUIElemPosition(dat,&state->ds,rdat,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
}

void setSelectedNuclOnLevelList(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t N, const uint16_t Z){
	uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)N,(int16_t)Z);
	//SDL_Log("Selected nucleus: %u\n",state->chartSelectedNucl);
	if((selNucl < MAXNUMNUCL)&&(selNucl != state->chartSelectedNucl)){
		state->chartSelectedNucl = selNucl;
		setFullLevelInfoDimensions(dat,state,rdat,selNucl);
		setInfoBoxDimensions(dat,state,rdat,selNucl);
		state->ds.nuclFullInfoScrollY = 0.0f;
		state->ds.nuclFullInfoMaxScrollY = getMaxNumLvlDispLines(&dat->ndat,state);
		state->ds.timeSinceFCScollStart = 0.0f;
		state->ds.fcScrollInProgress = 0;
		state->ds.fcScrollFinished = 1;
		state->ds.timeSinceFCNuclChangeStart = 0.0f;
		state->ds.fcNuclChangeInProgress = 1;
		state->ds.fcNuclChangeFinished = 0;
		startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_TXTFADEIN);

		//also pan the chart 'behind the scenes'
		state->ds.chartPosX = N*1.0f + 0.5f;
		state->ds.chartPosY = Z*1.0f - 0.5f - (16.0f/state->ds.chartZoomScale);
	}
}



//handles everything needed to select a new nucleus on the main chart view
//forcePan: 0=don't pan chart (except to dodge UI elements)
//          1=pan chart from mouse (double-click)
//          2=pan chart from keyboard (fast)
void setSelectedNuclOnChart(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t N, const uint16_t Z, const uint8_t forcePan){
	uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)N,(int16_t)Z);
	//SDL_Log("Selected nucleus: %u\n",state->chartSelectedNucl);
	if((selNucl < MAXNUMNUCL)&&(selNucl != state->chartSelectedNucl)){
		state->chartSelectedNucl = selNucl;
		setInfoBoxDimensions(dat,state,rdat,selNucl);
		if(!(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))){
			state->ds.shownElements |= (1U << UIELEM_NUCL_INFOBOX);
			changeUIState(dat,state,UISTATE_INFOBOX); //make info box interactable
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_SHOW);
		}
		startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_SHOW);
		if(forcePan==1){
			panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
		}else if(forcePan==2){
			panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME);
		}else{
			//check occlusion by info box
			float xOcclLeft = chartNtoXPx(&state->ds,(float)(N+1));
			float xOcclRight = chartNtoXPx(&state->ds,(float)N);
			float yOcclTop = chartZtoYPx(&state->ds,(float)(Z-1));
			if((xOcclLeft >= state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX])&&(xOcclRight <= (state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]))){
				if(yOcclTop >= state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX]){
					//always pan chart to dodge occlusion
					if(forcePan==2){
						panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME);
					}else{
						panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
					}
				}
			}
		}	
	}else{
		if((forcePan==1) && (state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f) && (selNucl < MAXNUMNUCL)){
			//pan chart to nuclide that is clicked
			//SDL_Log("starting pan to: %f %f\n",(double)mouseX,(double)mouseY);
			panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
		}else if(forcePan == 0){
			if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
				startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
			}else{
				state->chartSelectedNucl = selNucl;
			}
		}
	}
	
}
void setSelectedNuclOnChartDirect(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t selNucl, const uint8_t forcePan){
	if(selNucl < MAXNUMNUCL){
		setSelectedNuclOnChart(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[selNucl].N),(uint16_t)(dat->ndat.nuclData[selNucl].Z),forcePan);
	}
}

void uiElemHoldAction(const app_data *restrict dat, app_state *restrict state, const uint8_t uiElemID){
	switch(uiElemID){
		case UIELEM_NUCL_FULLINFOBOX_SCROLLBAR:
			; //empty statement to suppress -Wpedantic warning
			const float screenNumLines = (float)(getNumScreenLvlDispLines(&state->ds));
			state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoMaxScrollY*(state->mouseYPx - state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR])/((float)state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]*(1.0f - screenNumLines/(float)(getNumTotalLvlDispLines(&dat->ndat,state))));
			state->ds.nuclFullInfoScrollY -=  screenNumLines*0.5f; //center scrollbar on mouse 9moving the scrollbar by half its length scrolls the view by half the number of lines visible on screen)
			if(state->ds.nuclFullInfoScrollY < 0.0f){
				state->ds.nuclFullInfoScrollY = 0.0f;
			}else if(state->ds.nuclFullInfoScrollY > state->ds.nuclFullInfoMaxScrollY){
				state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoMaxScrollY;
			}
			break;
		default:
			break;
	}
}

//take action after clicking a button or other UI element
void uiElemClickAction(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t doubleClick, const uint8_t uiElemID){

	//SDL_Log("Clicked UI element %u\n",uiElemID);
  state->clickedUIElem = uiElemID;

	//handle extraneous opened menus
	if((uiElemID != UIELEM_MENU_BUTTON)&&(uiElemID != UIELEM_PRIMARY_MENU)&&(uiElemID != UIELEM_PM_PREFS_BUTTON)&&(uiElemID != UIELEM_PM_ABOUT_BUTTON)){
		if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_CHARTVIEW_BUTTON)&&(uiElemID != UIELEM_CHARTVIEW_MENU)&&(uiElemID != UIELEM_CVM_HALFLIFE_BUTTON)&&(uiElemID != UIELEM_CVM_DECAYMODE_BUTTON)&&(uiElemID != UIELEM_CVM_2PLUS_BUTTON)&&(uiElemID != UIELEM_CVM_R42_BUTTON)){
		if((state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN)&&(uiElemID != UIELEM_PREFS_UISCALE_MENU)&&(uiElemID != UIELEM_UISM_SMALL_BUTTON)&&(uiElemID != UIELEM_UISM_DEFAULT_BUTTON)&&(uiElemID != UIELEM_UISM_LARGE_BUTTON)&&(uiElemID != UIELEM_UISM_HUGE_BUTTON)){
		if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}

	//take action from click
  switch(uiElemID){
    case UIELEM_MENU_BUTTON:
      if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f){
				state->ds.shownElements |= (1U << UIELEM_PRIMARY_MENU);
				state->lastOpenedMenu = UIELEM_PRIMARY_MENU;
				startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_SHOW);
				if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
					changeUIState(dat,state,UISTATE_FULLLEVELINFOWITHMENU);
				}else{
					changeUIState(dat,state,UISTATE_CHARTWITHMENU);
				}
				state->clickedUIElem = UIELEM_MENU_BUTTON;
      }
      break;
		case UIELEM_CHARTVIEW_BUTTON:
			if((state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]==0.0f){
				state->ds.shownElements |= (1U << UIELEM_CHARTVIEW_MENU);
				state->lastOpenedMenu = UIELEM_CHARTVIEW_MENU;
				startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_SHOW);
				changeUIState(dat,state,UISTATE_CHARTWITHMENU);
				state->clickedUIElem = UIELEM_CHARTVIEW_BUTTON;
      }
			break;
		case UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN:
			if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]==0.0f){
				state->ds.shownElements |= (1U << UIELEM_PREFS_UISCALE_MENU);
				startUIAnimation(dat,state,UIANIM_UISCALE_MENU_SHOW);
				changeUIState(dat,state,state->uiState);
				state->clickedUIElem = UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN;
      }
			break;
    case UIELEM_MSG_BOX_OK_BUTTON:
      changeUIState(dat,state,state->lastUIState); //restore previous interactable elements
      startUIAnimation(dat,state,UIANIM_MODAL_BOX_HIDE); //message box will be closed after animation finishes
      break;
		case UIELEM_ABOUT_BOX_OK_BUTTON:
      changeUIState(dat,state,state->lastUIState); //restore previous interactable elements
      startUIAnimation(dat,state,UIANIM_MODAL_BOX_HIDE); //about box will be closed after animation finishes
      break;
		case UIELEM_PREFS_DIALOG_CLOSEBUTTON:
      changeUIState(dat,state,state->lastUIState); //restore previous interactable elements
      startUIAnimation(dat,state,UIANIM_MODAL_BOX_HIDE); //about box will be closed after animation finishes
      break;
		case UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX:
			state->ds.drawShellClosures = !(state->ds.drawShellClosures);
			state->ds.forceRedraw = 1;
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the button
			break;
		case UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX:
			state->ds.useLifetimes = !(state->ds.useLifetimes);
			state->ds.forceRedraw = 1;
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the button
			break;
		case UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX:
			state->ds.useUIAnimations = !(state->ds.useUIAnimations);
			state->ds.forceRedraw = 1;
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the button
			break;
		case UIELEM_NUCL_INFOBOX:
			break;
		case UIELEM_NUCL_INFOBOX_CLOSEBUTTON:
			//close the info box
			if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
				startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
			}
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
				state->ds.nuclFullInfoScrollY = 0.0f;
				startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_EXPAND);
				setFullLevelInfoDimensions(dat,state,rdat,state->chartSelectedNucl);
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_TXTFADEOUT); //menu will be closed after animation finishes
			break;
		case UIELEM_NUCL_FULLINFOBOX_SCROLLBAR:
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the scrollbar when the mouse is released
			break;
		case UIELEM_PM_PREFS_BUTTON:
			//SDL_Log("Clicked prefs button.\n");
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
      state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.shownElements |= (uint32_t)(1U << UIELEM_PREFS_DIALOG);
			startUIAnimation(dat,state,UIANIM_MODAL_BOX_SHOW);
			changeUIState(dat,state,UISTATE_PREFS_DIALOG);
			break;
		case UIELEM_PM_ABOUT_BUTTON:
			//SDL_Log("Clicked about button.\n");
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
      state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.shownElements |= (uint32_t)(1U << UIELEM_ABOUT_BOX);
			startUIAnimation(dat,state,UIANIM_MODAL_BOX_SHOW);
			changeUIState(dat,state,UISTATE_ABOUT_BOX);
			break;
		case UIELEM_CVM_HALFLIFE_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_HALFLIFE;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_DECAYMODE_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_DECAYMODE;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_2PLUS_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_2PLUS;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_R42_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_R42;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_PRIMARY_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_MENU_BUTTON;
			break;
		case UIELEM_CHARTVIEW_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_CHARTVIEW_BUTTON;
			break;
		case UIELEM_PREFS_UISCALE_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN;
			break;
		case UIELEM_UISM_SMALL_BUTTON:
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.interfaceSizeInd = UISCALE_SMALL;
			updateUIScale(dat,state,rdat);
			break;
		case UIELEM_UISM_DEFAULT_BUTTON:
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.interfaceSizeInd = UISCALE_NORMAL;
			updateUIScale(dat,state,rdat);
			break;
		case UIELEM_UISM_LARGE_BUTTON:
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.interfaceSizeInd = UISCALE_LARGE;
			updateUIScale(dat,state,rdat);
			break;
		case UIELEM_UISM_HUGE_BUTTON:
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.interfaceSizeInd = UISCALE_HUGE;
			updateUIScale(dat,state,rdat);
			break;
		case UIELEM_ENUM_LENGTH:
    default:
			//clicked outside of a button or UI element
      if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
				if(((state->ds.shownElements) == (uint32_t)(1U << UIELEM_CHARTOFNUCLIDES))||((state->ds.shownElements >> (UIELEM_CHARTOFNUCLIDES+1)) == (1U << (UIELEM_NUCL_INFOBOX-1)))){
					//only the chart of nuclides and/or info box are open
					//clicked on the chart view
					uint16_t mouseX = (uint16_t)mouseXPxToN(&state->ds,state->mouseXPx);
    			uint16_t mouseY = (uint16_t)SDL_ceilf(mouseYPxToZ(&state->ds,state->mouseYPx));
					//select nucleus
					setSelectedNuclOnChart(dat,state,rdat,mouseX,mouseY,doubleClick? 1 : 0);
				}else{
					//clicked out of a menu
					//handle individual menu closing animations
					if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
						startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
					}
					if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX)){
						state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES);
					}
				}
      }
      break;
  }
}


//updates UI element (buttons etc.) positions, based on the screen resolution and other factors
//positioning constants are defined in gui_constants.h
void updateSingleUIElemPosition(const app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat, const uint8_t uiElemInd){
	switch(uiElemInd){
		case UIELEM_MENU_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((MENU_BUTTON_WIDTH+MENU_BUTTON_POS_XR)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)(MENU_BUTTON_POS_Y*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(MENU_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemExtPlusX[uiElemInd] = (uint16_t)(MENU_BUTTON_POS_XR*ds->uiUserScale); //prevent clicking chart 'in between' buttons
			ds->uiElemExtMinusY[uiElemInd] = (uint16_t)(MENU_BUTTON_POS_Y*ds->uiUserScale); //prevent clicking chart 'in between' buttons
			//SDL_Log("res: [%u %u]\nx: %u, y: %u, w: %u, h: %u\n",ds->windowXRes,ds->windowYRes,ds->uiElemPosX[uiElemInd],ds->uiElemPosY[uiElemInd],ds->uiElemWidth[uiElemInd],ds->uiElemHeight[uiElemInd]);
			break;
		case UIELEM_CHARTVIEW_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_BUTTON_WIDTH+CHARTVIEW_BUTTON_POS_XR+MENU_BUTTON_WIDTH)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)(CHARTVIEW_BUTTON_POS_Y*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(CHARTVIEW_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemExtPlusX[uiElemInd] = (uint16_t)(CHARTVIEW_BUTTON_POS_XR*ds->uiUserScale); //prevent clicking chart 'in between' buttons
			ds->uiElemExtMinusY[uiElemInd] = (uint16_t)(CHARTVIEW_BUTTON_POS_Y*ds->uiUserScale); //prevent clicking chart 'in between' buttons
			break;
		case UIELEM_PRIMARY_MENU:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)(PRIMARY_MENU_POS_Y*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(PRIMARY_MENU_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(PRIMARY_MENU_HEIGHT*ds->uiUserScale);
			break;
		case UIELEM_PM_PREFS_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((PRIMARY_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((PRIMARY_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((PRIMARY_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_PM_ABOUT_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((PRIMARY_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + PRIMARY_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((PRIMARY_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((PRIMARY_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_CHARTVIEW_MENU:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)(CHARTVIEW_MENU_POS_Y*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(CHARTVIEW_MENU_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(CHARTVIEW_MENU_HEIGHT*ds->uiUserScale);
			break;
		case UIELEM_CVM_HALFLIFE_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + CHARTVIEW_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_CVM_DECAYMODE_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 2*CHARTVIEW_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_CVM_2PLUS_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 3*CHARTVIEW_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_CVM_R42_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*ds->uiUserScale));
			ds->uiElemPosY[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 4*CHARTVIEW_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_MSG_BOX:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_WIDTH*ds->uiUserScale)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes - MESSAGE_BOX_HEIGHT*ds->uiUserScale)/2);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(MESSAGE_BOX_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(MESSAGE_BOX_HEIGHT*ds->uiUserScale);
			break;
		case UIELEM_MSG_BOX_OK_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_OK_BUTTON_WIDTH*ds->uiUserScale)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes + MESSAGE_BOX_HEIGHT*ds->uiUserScale)/2 - ((MESSAGE_BOX_OK_BUTTON_YB + UI_TILE_SIZE)*ds->uiUserScale));
			ds->uiElemWidth[uiElemInd] = (uint16_t)(MESSAGE_BOX_OK_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			break;
		case UIELEM_ABOUT_BOX:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - ABOUT_BOX_WIDTH*ds->uiUserScale)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes - ABOUT_BOX_HEIGHT*ds->uiUserScale)/2);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(ABOUT_BOX_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(ABOUT_BOX_HEIGHT*ds->uiUserScale);
			break;
		case UIELEM_ABOUT_BOX_OK_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - ABOUT_BOX_OK_BUTTON_WIDTH*ds->uiUserScale)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes + ABOUT_BOX_HEIGHT*ds->uiUserScale)/2 - ((ABOUT_BOX_OK_BUTTON_YB + UI_TILE_SIZE)*ds->uiUserScale));
			ds->uiElemWidth[uiElemInd] = (uint16_t)(ABOUT_BOX_OK_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			break;
		case UIELEM_PREFS_DIALOG:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - PREFS_DIALOG_WIDTH*ds->uiUserScale)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes - PREFS_DIALOG_HEIGHT*ds->uiUserScale)/2);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(PREFS_DIALOG_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[uiElemInd] = (uint16_t)(PREFS_DIALOG_HEIGHT*ds->uiUserScale);
			//update child/dependant UI elements
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_DIALOG_CLOSEBUTTON);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_PREFS_UISCALE_MENU);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_UISM_SMALL_BUTTON);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_UISM_DEFAULT_BUTTON);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_UISM_LARGE_BUTTON);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_UISM_HUGE_BUTTON);
			break;
		case UIELEM_PREFS_DIALOG_CLOSEBUTTON:
			ds->uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = ds->uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON];
			ds->uiElemPosX[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = (uint16_t)((float)(ds->uiElemPosX[UIELEM_PREFS_DIALOG] + ds->uiElemWidth[UIELEM_PREFS_DIALOG] - ds->uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON]) - 4*UI_PADDING_SIZE*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = ds->uiElemPosY[UIELEM_PREFS_DIALOG] + (uint16_t)(4*UI_PADDING_SIZE*ds->uiUserScale);
			break;
		case UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX:
			ds->uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = ds->uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX];
			ds->uiElemPosX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = ds->uiElemPosX[UIELEM_PREFS_DIALOG] + (uint16_t)(PREFS_DIALOG_PREFCOL1_X*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = ds->uiElemPosY[UIELEM_PREFS_DIALOG] + (uint16_t)(PREFS_DIALOG_PREFCOL1_Y*ds->uiUserScale);
			ds->uiElemExtPlusX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*ds->uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_SHELLCLOSURE]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX:
			ds->uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = ds->uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX];
			ds->uiElemPosX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = ds->uiElemPosX[UIELEM_PREFS_DIALOG] + (uint16_t)(PREFS_DIALOG_PREFCOL1_X*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = ds->uiElemPosY[UIELEM_PREFS_DIALOG] + (uint16_t)((PREFS_DIALOG_PREFCOL1_Y + PREFS_DIALOG_PREF_Y_SPACING)*ds->uiUserScale);
			ds->uiElemExtPlusX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*ds->uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_LIFETIME]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX:
			ds->uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = ds->uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX];
			ds->uiElemPosX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = ds->uiElemPosX[UIELEM_PREFS_DIALOG] + (uint16_t)(PREFS_DIALOG_PREFCOL1_X*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = ds->uiElemPosY[UIELEM_PREFS_DIALOG] + (uint16_t)((PREFS_DIALOG_PREFCOL1_Y + 2*PREFS_DIALOG_PREF_Y_SPACING)*ds->uiUserScale);
			ds->uiElemExtPlusX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*ds->uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_UIANIM]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN:
			ds->uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (uint16_t)(PREFS_DIALOG_UISCALE_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (uint16_t)(ds->uiElemPosX[UIELEM_PREFS_DIALOG] + (uint16_t)((PREFS_DIALOG_PREFCOL1_X+3*UI_PADDING_SIZE)*ds->uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_UISCALE]])/rdat->uiDPIScale));
			ds->uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = ds->uiElemPosY[UIELEM_PREFS_DIALOG] + (uint16_t)((PREFS_DIALOG_PREFCOL1_Y + 3*PREFS_DIALOG_PREF_Y_SPACING + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_PREFS_UISCALE_MENU:
			ds->uiElemPosX[UIELEM_PREFS_UISCALE_MENU] = ds->uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN];
			ds->uiElemPosY[UIELEM_PREFS_UISCALE_MENU] = (uint16_t)(ds->uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] + ds->uiElemHeight[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]);
			ds->uiElemWidth[UIELEM_PREFS_UISCALE_MENU] = (uint16_t)(PREFS_DIALOG_UISCALE_MENU_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_PREFS_UISCALE_MENU] = (uint16_t)(((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING + UI_PADDING_SIZE)*UISCALE_ENUM_LENGTH + 2*PANEL_EDGE_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_UISM_SMALL_BUTTON:
			ds->uiElemPosX[UIELEM_UISM_SMALL_BUTTON] = ds->uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_UISM_SMALL_BUTTON] =  ds->uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_UISM_SMALL_BUTTON] = ds->uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (uint16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_UISM_SMALL_BUTTON] = (uint16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_UISM_DEFAULT_BUTTON:
			ds->uiElemPosX[UIELEM_UISM_DEFAULT_BUTTON] = ds->uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_UISM_DEFAULT_BUTTON] =  ds->uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_UISM_DEFAULT_BUTTON] = ds->uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (uint16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_UISM_DEFAULT_BUTTON] = (uint16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_UISM_LARGE_BUTTON:
			ds->uiElemPosX[UIELEM_UISM_LARGE_BUTTON] = ds->uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_UISM_LARGE_BUTTON] =  ds->uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 2*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_UISM_LARGE_BUTTON] = ds->uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (uint16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_UISM_LARGE_BUTTON] = (uint16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_UISM_HUGE_BUTTON:
			ds->uiElemPosX[UIELEM_UISM_HUGE_BUTTON] = ds->uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_UISM_HUGE_BUTTON] =  ds->uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (uint16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 3*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_UISM_HUGE_BUTTON] = ds->uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (uint16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_UISM_HUGE_BUTTON] = (uint16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*ds->uiUserScale);
			break;
		case UIELEM_NUCL_INFOBOX:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - ds->infoBoxWidth*ds->uiUserScale)/2);
			uint16_t freeXSpace = (uint16_t)(ds->windowXRes - ds->infoBoxWidth*ds->uiUserScale);
			if(freeXSpace < 4*NUCL_INFOBOX_X_PADDING*ds->uiUserScale){
				ds->uiElemPosX[uiElemInd] += (uint16_t)(NUCL_INFOBOX_X_PADDING*ds->uiUserScale - freeXSpace/4); //make sure info box doesn't bump up against y-axis
			}
			ds->uiElemHeight[uiElemInd] = (uint16_t)(((float)NUCL_INFOBOX_MIN_HEIGHT + ds->infoBoxTableHeight + 2*PANEL_EDGE_SIZE)*ds->uiUserScale);
			ds->uiElemPosY[uiElemInd] = (uint16_t)(ds->windowYRes - ds->uiElemHeight[uiElemInd] - (uint16_t)((UI_PADDING_SIZE + CHART_AXIS_DEPTH)*ds->uiUserScale));
			ds->uiElemWidth[uiElemInd] = (uint16_t)(ds->infoBoxWidth*ds->uiUserScale);
			//update child/dependant UI elements
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			updateSingleUIElemPosition(dat,ds,rdat,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			break;
		case UIELEM_NUCL_INFOBOX_CLOSEBUTTON:
			ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON];
			ds->uiElemPosX[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - (int32_t)(4*UI_PADDING_SIZE*ds->uiUserScale));
			ds->uiElemPosY[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + (uint16_t)(4*UI_PADDING_SIZE*ds->uiUserScale);
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
			ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(NUCL_INFOBOX_ALLLEVELS_BUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			if(ds->uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
				float animFrac = juice_smoothStop3(1.0f - ds->timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH);
				uint16_t defaultPosX = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*ds->uiUserScale));
				uint16_t defaultPosY = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + (uint16_t)(4*UI_PADDING_SIZE*ds->uiUserScale);
				uint16_t fullPosX = (uint16_t)(ds->windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*ds->uiUserScale);
				uint16_t fullPosY = (uint16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*ds->uiUserScale);
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(defaultPosX + animFrac*(fullPosX - defaultPosX));
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(defaultPosY + animFrac*(fullPosY - defaultPosY));
			}else if(ds->uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
				float animFrac = juice_smoothStop3(1.0f - ds->timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH);
				uint16_t defaultPosX = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*ds->uiUserScale));
				uint16_t defaultPosY = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + (uint16_t)(4*UI_PADDING_SIZE*ds->uiUserScale);
				uint16_t fullPosX = (uint16_t)(ds->windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*ds->uiUserScale);
				uint16_t fullPosY = (uint16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*ds->uiUserScale);
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(fullPosX + animFrac*(defaultPosX - fullPosX));
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(fullPosY + animFrac*(defaultPosY - fullPosY));
			}else{
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*ds->uiUserScale));
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + (uint16_t)(4*UI_PADDING_SIZE*ds->uiUserScale);
			}
			//SDL_Log("x: %u, y: %u, w: %u, h: %u\n",ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON]);
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			ds->uiElemPosX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)(ds->windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)(UI_TILE_SIZE*ds->uiUserScale);
			break;
		case UIELEM_NUCL_FULLINFOBOX_SCROLLBAR:
			ds->uiElemPosX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(ds->windowXRes - NUCL_FULLINFOBOX_SCROLLBAR_POS_XR*ds->uiUserScale);
			ds->uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)((NUCL_FULLINFOBOX_LEVELLIST_POS_Y + UI_PADDING_SIZE)*ds->uiUserScale);
			ds->uiElemWidth[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(0.5f*UI_TILE_SIZE*ds->uiUserScale);
			ds->uiElemHeight[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(ds->windowYRes - ds->uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] - 2*UI_PADDING_SIZE*ds->uiUserScale);
		default:
			break;
	}
}
void updateUIElemPositions(const app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat){
  for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){
    updateSingleUIElemPosition(dat,ds,rdat,i);
  }
}

float getUIthemeScale(const float uiScale){
	//return roundf(2.0f*uiScale); //super-samples the UI texture, but results in odd spacing of text on panels
	return roundf(uiScale); //seems to work, don't question it
}

//updates the UI scaling, which requires the theme and 
//font data to be reloaded from disk and re-scaled
void updateUIScale(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
	state->ds.uiUserScale = uiScales[state->ds.interfaceSizeInd];
	rdat->uiScale = rdat->uiDPIScale * state->ds.uiUserScale;
	rdat->uiThemeScale = getUIthemeScale(rdat->uiScale);
	if(rdat->font[0]){
		//rescale font and UI theme as well, this requires loading them from the app data file
		regenerateThemeAndFontCache(dat,rdat); //load_data.c
	}
	state->ds.nuclFullInfoMaxScrollY = getMaxNumLvlDispLines(&dat->ndat,state);
	setFullLevelInfoDimensions(dat,state,rdat,state->chartSelectedNucl);
	setInfoBoxDimensions(dat,state,rdat,state->chartSelectedNucl);
	updateUIElemPositions(dat,&state->ds,rdat); //UI element positions
	SDL_SetWindowMinimumSize(rdat->window,(int)(MIN_RENDER_WIDTH*state->ds.uiUserScale),(int)(MIN_RENDER_HEIGHT*state->ds.uiUserScale));
	state->ds.forceRedraw = 1;
}

void updateWindowRes(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  int wwidth, wheight;
  int rwidth, rheight;
  SDL_GetWindowSize(rdat->window, &wwidth, &wheight);
  SDL_GetWindowSizeInPixels(rdat->window, &rwidth, &rheight);
  if((rwidth != state->ds.windowXRenderRes)||(rheight != state->ds.windowYRenderRes)){
    state->ds.forceRedraw = 1;
  }
	float newDPIScale = (float)rwidth/((float)wwidth);
	//float newScale = 1.0f; //for testing UI scales
	if(fabsf(rdat->uiDPIScale - newDPIScale) > 0.001f){
		SDL_Log("Changing UI DPI scale from %0.9f to %0.9f.\n",(double)rdat->uiDPIScale,(double)newDPIScale);
		rdat->uiDPIScale = newDPIScale; //set UI DPI scale properly for HI-DPI
		updateUIScale(dat,state,rdat);
	}
  state->ds.windowXRes = (uint16_t)wwidth;
  state->ds.windowYRes = (uint16_t)wheight;
  state->ds.windowXRenderRes = (uint16_t)rwidth;
  state->ds.windowYRenderRes = (uint16_t)rheight;

  //update things that depend on the window res
  updateUIElemPositions(dat,&state->ds,rdat); //UI element positions
	changeUIState(dat,state,state->uiState);
}

void handleScreenGraphicsMode(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  //handle vsync and frame cap
  SDL_SetRenderVSync(rdat->renderer,1); //vsync always enabled

  if(state->ds.windowFullscreenMode){
    if(SDL_SetWindowFullscreen(rdat->window,1) == 0){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"cannot set fullscreen mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,state,rdat);
    //SDL_Log("Full screen display mode.  Window resolution: %u x %u.\n",state->ds.windowXRes,state->ds.windowYRes);
  }else{
    if(SDL_SetWindowFullscreen(rdat->window,0) == 0){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"cannot set windowed mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,state,rdat);
    //SDL_Log("Windowed display mode.  Window resolution: %u x %u.\n",state->ds.windowXRes,state->ds.windowYRes);
  }
}
