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
void initializeTempState(const app_data *restrict dat, app_state *restrict state){
  //input
  state->mouseXPx = -1;
  state->mouseYPx = -1;
  state->mouseHoldStartPosXPx = -1;
  state->mouseHoldStartPosYPx = -1;
  state->lastAxisValX = 0;
  state->lastAxisValY = 0;
  state->activeAxis = 0; //the last used axis
  state->lastInputType = INPUT_TYPE_KEYBOARD; //default input type
	state->scrollSpeedMultiplier = 16.0f;
	state->inputFlags = 0;
  //app state
	state->chartSelectedNucl = MAXNUMNUCL;
  state->quitAppFlag = 0;
  //ui state
  changeUIState(dat,state,UISTATE_DEFAULT);
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //no selected UI element
  state->ds.shownElements = 0; //no UI elements being shown
	state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES);
  state->ds.uiAnimPlaying = 0; //no UI animations playing
  state->ds.useZoomAnimations = 1;
	state->ds.chartPosX = 62.0f;
	state->ds.chartPosY = 33.0f;
	state->ds.chartZoomScale = 0.5f;
	state->ds.chartZoomToScale = state->ds.chartZoomScale;
	state->ds.chartZoomStartScale = state->ds.chartZoomScale;
	state->ds.totalPanTime = CHART_KEY_PAN_TIME;
	state->ds.zoomFinished = 0;
	state->ds.zoomInProgress = 0;
	state->ds.dragFinished = 0;
	state->ds.dragInProgress = 0;
	state->ds.fcScrollFinished = 0;
	state->ds.fcScrollInProgress = 0;

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


void startUIAnimation(drawing_state *restrict ds, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    printf("WARNING: startUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
  ds->timeLeftInUIAnimation[uiAnim] = UI_ANIM_LENGTH;
  ds->uiAnimPlaying |= (1U << uiAnim);
}
void stopUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim){
  if(uiAnim >= UIANIM_ENUM_LENGTH){
    printf("WARNING: stopUIAnimation - invalid animation ID (%u, max %u).\n",uiAnim,UIANIM_ENUM_LENGTH-1);
    return;
  }
  state->ds.timeLeftInUIAnimation[uiAnim] = 0.0f;
  state->ds.uiAnimPlaying &= ~(1U << uiAnim);

  //take action at the end of the animation
  switch(uiAnim){
    case UIANIM_MSG_BOX_HIDE:
      state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_MSG_BOX)); //close the message box
      break;
		case UIANIM_NUCLINFOBOX_HIDE:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_INFOBOX)); //close the info box
			changeUIState(dat,state,UISTATE_DEFAULT); //make info box un-interactable
			state->chartSelectedNucl = MAXNUMNUCL;
			break;
		case UIANIM_NUCLINFOBOX_EXPAND:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_INFOBOX)); //close the info box
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_CHARTOFNUCLIDES)); //don't show the chart
			state->ds.shownElements |= (1U << UIELEM_NUCL_FULLINFOBOX); //show the full info box
			changeUIState(dat,state,UISTATE_FULLLEVELINFO); //update UI state now that the full info box is visible
			break;
    default:
      break;
  }
  state->ds.forceRedraw = 1;

  //printf("Stopped anim %u.\n",uiAnim);
}
void updateUIAnimationTimes(const app_data *restrict dat, app_state *restrict state, const float deltaTime){
  for(uint8_t i=0;i<UIANIM_ENUM_LENGTH;i++){
    if(state->ds.uiAnimPlaying & (uint32_t)(1U << i)){
      state->ds.timeLeftInUIAnimation[i] -= deltaTime;
      //printf("anim %u dt %.3f timeleft %.3f\n",i,(double)deltaTime,(double)state->ds.timeLeftInUIAnimation[i]);
      if(state->ds.timeLeftInUIAnimation[i] <= 0.0f){
        state->ds.timeLeftInUIAnimation[i] = 0.0f;
        stopUIAnimation(dat,state,i);
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
	if(state->ds.fcScrollFinished){
		state->ds.fcScrollInProgress = 0;
		state->ds.fcScrollFinished = 0; //reset flag
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
		state->ds.chartPosX = state->ds.chartPanStartX + (state->ds.chartPanToX - state->ds.chartPanStartX)*juice_smoothStop2(state->ds.timeSincePanStart/state->ds.totalPanTime);
		state->ds.chartPosY = state->ds.chartPanStartY + (state->ds.chartPanToY - state->ds.chartPanStartY)*juice_smoothStop2(state->ds.timeSincePanStart/state->ds.totalPanTime);
		if(state->ds.timeSincePanStart >= state->ds.totalPanTime){
			state->ds.chartPosX = state->ds.chartPanToX;
			state->ds.chartPosY = state->ds.chartPanToY;
			state->ds.panFinished = 1;
		}
		//printf("pan t: %0.3f\n",(double)state->ds.timeSincePanStart);
	}
	if(state->ds.fcScrollInProgress){
		state->ds.timeSinceFCScollStart += deltaTime;
		state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollStartY + (state->ds.nuclFullInfoScrollToY - state->ds.nuclFullInfoScrollStartY)*juice_smoothStop2(state->ds.timeSinceFCScollStart/NUCL_FULLINFOBOX_SCROLL_TIME);
		if(state->ds.timeSinceFCScollStart >= NUCL_FULLINFOBOX_SCROLL_TIME){
			state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollToY;
			state->ds.fcScrollFinished = 1;
		}
		//printf("scroll t: %0.3f, pos: %f\n",(double)state->ds.timeSinceFCScollStart,(double)state->ds.nuclFullInfoScrollY);
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
void setupMessageBox(const app_data *restrict dat, app_state *restrict state, const char *headerTxt, const char *msgTxt){
  strncpy(state->msgBoxHeaderTxt,headerTxt,31);
  strncpy(state->msgBoxTxt,msgTxt,255);
  state->ds.shownElements |= (uint32_t)(1U << UIELEM_MSG_BOX);
  startUIAnimation(&state->ds,UIANIM_MSG_BOX_SHOW);
  changeUIState(dat,state,UISTATE_MSG_BOX);
}

//returns a string with element names corresponding to Z values
const char* getFullElemStr(const uint8_t Z, const uint8_t N){
	switch(Z){
		case 0:
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
		case 1:
			//different names depending on N
			if(N==1){
				return "Deuterium";
			}else if(N==2){
				return "Tritium";
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

const char* getHalfLifeUnitShortStr(const uint8_t unit){
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

void getGammaEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr){

	uint8_t ePrecision = (uint8_t)(nd->tran[tranInd].energy.format & 15U);
	uint8_t eExponent = (uint8_t)((nd->tran[tranInd].energy.format >> 4U) & 1U);
	if((showErr == 0)||(nd->tran[tranInd].energy.err == 0)){
		if(eExponent == 0){
			snprintf(strOut,32,"%.*f",ePrecision,(double)(nd->tran[tranInd].energy.val));
		}else{
			snprintf(strOut,32,"%.*fE%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
		}
	}else{
		if(eExponent == 0){
			snprintf(strOut,32,"%.*f(%u)",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err);
		}else{
			snprintf(strOut,32,"%.*f(%u)E%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent,nd->tran[tranInd].energy.err);
		}
	}
	
}

void getGammaIntensityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr){

	uint8_t iPrecision = (uint8_t)(nd->tran[tranInd].intensity.format & 15U);
	uint8_t iExponent = (uint8_t)((nd->tran[tranInd].intensity.format >> 4U) & 1U);
	uint8_t iValueType = (uint8_t)((nd->tran[tranInd].intensity.format >> 5U) & 15U);
	if(nd->tran[tranInd].intensity.val <= 0.0f){
		snprintf(strOut,32," ");
	}else if((showErr == 0)||(nd->tran[tranInd].intensity.err == 0)){
		if(iExponent == 0){
			snprintf(strOut,32,"%s%.*f",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val));
		}else{
			snprintf(strOut,32,"%s%.*fE%i",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.exponent);
		}
	}else{
		if(iExponent == 0){
			snprintf(strOut,32,"%s%.*f(%u)",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.err);
		}else{
			snprintf(strOut,32,"%s%.*f(%u)E%i",getValueTypeShortStr(iValueType),iPrecision,(double)(nd->tran[tranInd].intensity.val),nd->tran[tranInd].intensity.err,nd->tran[tranInd].intensity.exponent);
		}
	}
	
}

void getLvlEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr){

	uint8_t ePrecision = (uint8_t)(nd->levels[lvlInd].energy.format & 15U);
	uint8_t eExponent = (uint8_t)((nd->levels[lvlInd].energy.format >> 4U) & 1U);
	uint8_t eValueType = (uint8_t)((nd->levels[lvlInd].energy.format >> 5U) & 15U);
	if(eValueType == VALUETYPE_X){
		uint8_t variable = (uint8_t)((nd->levels[lvlInd].energy.format >> 9U) & 127U);
		snprintf(strOut,32,"%c",variable);
	}else if(eValueType == VALUETYPE_PLUSX){
		uint8_t variable = (uint8_t)((nd->levels[lvlInd].energy.format >> 9U) & 127U);
		if(eExponent == 0){
			snprintf(strOut,32,"%.*f+%c",ePrecision,(double)(nd->levels[lvlInd].energy.val),variable);
		}else{
			snprintf(strOut,32,"%.*fE%i+%c",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.exponent,variable);
		}
	}else if((showErr == 0)||(nd->levels[lvlInd].energy.err == 0)){
		if(eExponent == 0){
			snprintf(strOut,32,"%.*f",ePrecision,(double)(nd->levels[lvlInd].energy.val));
		}else{
			snprintf(strOut,32,"%.*fE%i",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.exponent);
		}
	}else{
		if(eExponent == 0){
			snprintf(strOut,32,"%.*f(%u)",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.err);
		}else{
			snprintf(strOut,32,"%.*f(%u)E%i",ePrecision,(double)(nd->levels[lvlInd].energy.val),nd->levels[lvlInd].energy.err,nd->levels[lvlInd].energy.exponent);
		}
	}
	
}

void getHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr, const uint8_t showUnknown){
	if(lvlInd < nd->numLvls){
		if(nd->levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
			snprintf(strOut,32,"STABLE");
		}else if(nd->levels[lvlInd].halfLife.unit == VALUE_UNIT_NOVAL){
			if(showUnknown){
				snprintf(strOut,32,"Unknown");
			}else{
				snprintf(strOut,32," ");
			}
		}else if(nd->levels[lvlInd].halfLife.val > 0.0f){
			uint8_t hlPrecision = (uint8_t)(nd->levels[lvlInd].halfLife.format & 15U);
			uint8_t hlExponent = (uint8_t)((nd->levels[lvlInd].halfLife.format >> 4U) & 1U);
			uint8_t hlValueType = (uint8_t)((nd->levels[lvlInd].halfLife.format >> 5U) & 15U);
			if(hlValueType == VALUETYPE_ASYMERROR){
				uint8_t negErr = (uint8_t)((nd->levels[lvlInd].halfLife.format >> 9U) & 127U);
				if(hlExponent == 0){
					snprintf(strOut,32,"%.*f(+%u-%u) %s",hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),nd->levels[lvlInd].halfLife.err,negErr,getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
				}else{
					snprintf(strOut,32,"%.*f(+%u-%u)E%i %s",hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),nd->levels[lvlInd].halfLife.err,negErr,nd->levels[lvlInd].halfLife.exponent,getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
				}
			}else{
				if((showErr == 0)||(nd->levels[lvlInd].halfLife.err == 0)){
					if(hlExponent == 0){
						snprintf(strOut,32,"%s%.*f %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
					}else{
						snprintf(strOut,32,"%s%.*fE%i %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),nd->levels[lvlInd].halfLife.exponent,getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
					}
				}else{
					if(hlExponent == 0){
						snprintf(strOut,32,"%s%.*f(%u) %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),nd->levels[lvlInd].halfLife.err,getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
					}else{
						snprintf(strOut,32,"%s%.*f(%u)E%i %s",getValueTypeShortStr(hlValueType),hlPrecision,(double)(nd->levels[lvlInd].halfLife.val),nd->levels[lvlInd].halfLife.err,nd->levels[lvlInd].halfLife.exponent,getHalfLifeUnitShortStr(nd->levels[lvlInd].halfLife.unit));
					}
				}
			}
		}else{
			snprintf(strOut,32," ");
		}
	}else{
		if(showUnknown){
			snprintf(strOut,32,"Unknown");
		}else{
			snprintf(strOut,32," ");
		}
	}
}
void getGSHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd){
	if(nd->nuclData[nuclInd].numLevels > 0){
		getHalfLifeStr(strOut,nd,nd->nuclData[nuclInd].firstLevel + nd->nuclData[nuclInd].gsLevel,1,1);
	}else{
		snprintf(strOut,32,"Unknown");
	}
}

void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd){
	if(dcyModeInd < nd->numDecModes){
		uint8_t decValueType = nd->dcyMode[dcyModeInd].prob.unit;
		uint8_t decType = nd->dcyMode[dcyModeInd].type;
		uint8_t decPrecision = (uint8_t)(nd->dcyMode[dcyModeInd].prob.format & 15U);
		if(decValueType == VALUETYPE_NUMBER){
			if(nd->dcyMode[dcyModeInd].prob.err > 0){
				snprintf(strOut,32,"%s = %.*f(%u)%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val,nd->dcyMode[dcyModeInd].prob.err); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
			}else{
				snprintf(strOut,32,"%s = %.*f%%%%",getDecayTypeShortStr(decType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
			}
		}else if(decValueType == VALUETYPE_UNKNOWN){
			snprintf(strOut,32,"%s%s",getDecayTypeShortStr(decType),getValueTypeShortStr(decValueType));
		}else{
			snprintf(strOut,32,"%s %s%.*f%%%%",getDecayTypeShortStr(decType),getValueTypeShortStr(decValueType),decPrecision,(double)nd->dcyMode[dcyModeInd].prob.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
		}
	}else{
		snprintf(strOut,32," ");
	}
}

void getAbundanceStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd){
	if(nuclInd < nd->numNucl){
		if(nd->nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
			uint8_t abPrecision = (uint8_t)(nd->nuclData[nuclInd].abundance.format & 15U);
			snprintf(strOut,32,"%.*f%%%%",abPrecision,(double)nd->nuclData[nuclInd].abundance.val); //%%%% will be parsed to "%%" in tmpStr, which will then be parsed as a format string by SDL_FontCacahe, leaving "%"
		}else{
			snprintf(strOut,32," ");
		}
	}else{
		snprintf(strOut,32," ");
	}
}

void getSpinParStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd){

	char val[16];

	strcpy(strOut,""); //clear the string

	for(int i=0;i<nd->levels[lvlInd].numSpinParVals;i++){
		
		//printf("Spin: %i, parity: %i, tentative: %u\n\n",nd->levels[lvlInd].spval[i].spinVal,nd->levels[lvlInd].spval[i].parVal,nd->levels[lvlInd].spval[i].tentative);
		
		if(nd->levels[lvlInd].spval[i].tentative == TENTATIVE_RANGE){
			strcat(strOut,"to");
		}else{
			if((nd->levels[lvlInd].spval[i].tentative == TENTATIVE_SPINANDPARITY)||(nd->levels[lvlInd].spval[i].tentative == TENTATIVE_SPINONLY)){
				if((i==0)||((i>0)&&((nd->levels[lvlInd].spval[i-1].tentative != TENTATIVE_SPINANDPARITY)&&(nd->levels[lvlInd].spval[i-1].tentative != TENTATIVE_SPINONLY)))){
					if((i>0)&&(nd->levels[lvlInd].spval[i-1].tentative == TENTATIVE_RANGE)){
						//previous spin parity value specified a range
						strcat(strOut," ");
					}else{
						strcat(strOut,"(");
					}
					
				}
			}
			if(nd->levels[lvlInd].spval[i].spinVal >= 0){
				if(nd->levels[lvlInd].halfInt == 1){
					sprintf(val,"%i/2",nd->levels[lvlInd].spval[i].spinVal);
				}else{
					sprintf(val,"%i",nd->levels[lvlInd].spval[i].spinVal);
				}
				strcat(strOut,val);
			}
			if(nd->levels[lvlInd].spval[i].parVal == -1){
				strcat(strOut,"-");
			}else if(nd->levels[lvlInd].spval[i].parVal == 1){
				strcat(strOut,"+");
			}
			if((nd->levels[lvlInd].spval[i].tentative == TENTATIVE_SPINANDPARITY)||(nd->levels[lvlInd].spval[i].tentative == TENTATIVE_SPINONLY)){
				if(i==nd->levels[lvlInd].numSpinParVals-1){
					strcat(strOut,")");
				}else if(i<nd->levels[lvlInd].numSpinParVals-1){
					if(nd->levels[lvlInd].spval[i+1].tentative != TENTATIVE_RANGE){
						if(nd->levels[lvlInd].spval[i+1].tentative != TENTATIVE_SPINANDPARITY){
							if(nd->levels[lvlInd].spval[i+1].tentative != TENTATIVE_SPINONLY){
								strcat(strOut,")");
							}
						}
					}
				}
			}
			if(i!=nd->levels[lvlInd].numSpinParVals-1){
				if(nd->levels[lvlInd].spval[i+1].tentative == TENTATIVE_RANGE){
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
	//printf("WARNING: getNuclInd - couldn't find nucleus with N,Z = [%i %i].\n",N,Z);
	return MAXNUMNUCL;
}

uint16_t getNumDispLinesForLvl(const ndata *restrict nd, const uint32_t lvlInd){
  uint16_t levelNumLines = 1;
  if(nd->levels[lvlInd].numDecModes > 0){
    levelNumLines += (uint16_t)(nd->levels[lvlInd].numDecModes);
  }
  if(nd->levels[lvlInd].numTran > levelNumLines){
    levelNumLines = (uint16_t)(nd->levels[lvlInd].numTran);
  }
  return levelNumLines;
}

uint16_t getMaxNumLvlDispLines(const ndata *restrict nd, const app_state *restrict state){
	//find total number of lines displayable
	uint16_t numLines = 0;
	for(uint32_t lvlInd = nd->nuclData[state->chartSelectedNucl].firstLevel; lvlInd<(nd->nuclData[state->chartSelectedNucl].firstLevel + nd->nuclData[state->chartSelectedNucl].numLevels); lvlInd++){
		numLines += getNumDispLinesForLvl(nd,lvlInd);
	}
	uint16_t numScreenLines = (uint16_t)(floorf((state->ds.windowYRes - NUCL_FULLINFOBOX_LEVELLIST_POS_Y)/NUCL_INFOBOX_SMALLLINE_HEIGHT));
	if(numLines > numScreenLines){
		numLines -= numScreenLines;
	}else{
		numLines = 0;
	}
	return numLines;
}

float mouseXPxToN(const drawing_state *restrict ds, const float mouseX){
	return ds->chartPosX + ((mouseX - ds->windowXRes/2.0f)/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float mouseYPxToZ(const drawing_state *restrict ds, const float mouseY){
	return ds->chartPosY - ((mouseY - ds->windowYRes/2.0f)/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float chartNtoXPx(const drawing_state *restrict ds, const float N){
	return (ds->windowXRes/2.0f) + (N - ds->chartPosX)*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale;
}
float chartZtoYPx(const drawing_state *restrict ds, const float Z){
	return (ds->windowYRes/2.0f) + (ds->chartPosY - Z)*DEFAULT_NUCLBOX_DIM*ds->chartZoomScale;
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
void changeUIState(const app_data *restrict dat, app_state *restrict state, const uint8_t newState){
  
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
		case UISTATE_FULLLEVELINFO:
			state->ds.nuclFullInfoMaxScrollY = getMaxNumLvlDispLines(&dat->ndat,state); //find total number of lines displayable
			state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_FULLINFOBOX);
			state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_FULLINFOBOX_BACKBUTTON);
			break;
    case UISTATE_DEFAULT:
    default:
      state->interactableElement |= (uint32_t)(1U << UIELEM_MENU_BUTTON);
			if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
				state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX);
				state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
				state->interactableElement |= (uint32_t)(1U << UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			}
      break;
  }
}

void panChartToPos(const app_data *restrict dat, drawing_state *restrict ds, const uint16_t posN, const uint16_t posZ){
	ds->chartPanStartX = ds->chartPosX;
	ds->chartPanStartY = ds->chartPosY;
	ds->chartPanToX = posN*1.0f + 0.5f;
	ds->chartPanToY = posZ*1.0f + 0.5f - (16.0f/ds->chartZoomScale);
	//printf("pos: %u %u, panning to: %f %f\n",posN,posZ,(double)ds->chartPanToX,(double)ds->chartPanToY);
	//clamp chart display range
	if(ds->chartPanToX < 0.0f){
		ds->chartPanToX = 0.0f;
	}else if(ds->chartPanToX > (dat->ndat.maxN+1)){
		ds->chartPanToX = (float)dat->ndat.maxN+1.0f;
	}
	if(ds->chartPanToY < 0.0f){
		ds->chartPanToY = 0.0f;
	}else if(ds->chartPanToY > (dat->ndat.maxZ+1)){
		ds->chartPanToY = (float)dat->ndat.maxZ+1.0f;
	}
	//printf("panning to: %f %f\n",(double)ds->chartPanToX,(double)ds->chartPanToY);
	ds->timeSincePanStart = 0.0f;
	ds->totalPanTime = CHART_DOUBLECLICK_PAN_TIME;
	ds->panInProgress = 1;
	ds->panFinished = 0;
}

//take action after clicking a button or other UI element
void uiElemClickAction(const app_data *restrict dat, app_state *restrict state, const uint8_t doubleClick, const uint8_t uiElemID){

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
      changeUIState(dat,state,UISTATE_DEFAULT);
      startUIAnimation(&state->ds,UIANIM_MSG_BOX_HIDE); //message box will be closed after animation finishes
      break;
		case UIELEM_NUCL_INFOBOX:
			break;
		case UIELEM_NUCL_INFOBOX_CLOSEBUTTON:
			//close the info box
			if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
				startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
				startUIAnimation(&state->ds,UIANIM_NUCLHIGHLIGHT_HIDE);
			}
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
				state->ds.nuclFullInfoScrollY = 0.0f;
				startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_EXPAND);
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			state->ds.shownElements &= (uint32_t)(~(1U << UIELEM_NUCL_FULLINFOBOX)); //close the full info box
			state->ds.shownElements |= (1U << UIELEM_NUCL_INFOBOX); //show the info box
			state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES); //show the chart
			startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_CONTRACT);
			changeUIState(dat,state,UISTATE_DEFAULT); //update UI state now that the regular info box is visible
			break;
		case UIELEM_ENUM_LENGTH:
    default:
			//clicked outside of a button or UI element
      if(state->uiState == UISTATE_DEFAULT){
				if(((state->ds.shownElements) == (uint32_t)(1U << UIELEM_CHARTOFNUCLIDES))||((state->ds.shownElements >> (UIELEM_CHARTOFNUCLIDES+1)) == (1U << (UIELEM_NUCL_INFOBOX-1)))){
					//only the chart of nuclides and/or info box are open
					//clicked on the chart view
					float mouseX = mouseXPxToN(&state->ds,state->mouseXPx);
    			float mouseY = mouseYPxToZ(&state->ds,state->mouseYPx);
					//select nucleus
					uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)floorf(mouseX),(int16_t)floorf(mouseY + 1.0f));
					//printf("Selected nucleus: %u\n",state->chartSelectedNucl);
					if((selNucl < MAXNUMNUCL)&&(selNucl != state->chartSelectedNucl)){
						state->chartSelectedNucl = selNucl;
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
						updateSingleUIElemPosition(&state->ds,UIELEM_NUCL_INFOBOX);
						updateSingleUIElemPosition(&state->ds,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
						if(!(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))){
							state->ds.shownElements |= (1U << UIELEM_NUCL_INFOBOX);
							changeUIState(dat,state,UISTATE_DEFAULT); //make info box interactable
							startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_SHOW);
						}
						startUIAnimation(&state->ds,UIANIM_NUCLHIGHLIGHT_SHOW);
						//check occlusion by info box
						float xOcclLeft = chartNtoXPx(&state->ds,floorf(mouseX + 1.0f));
						float xOcclRight = chartNtoXPx(&state->ds,floorf(mouseX));
						float yOcclTop = chartZtoYPx(&state->ds,floorf(mouseY));
						if((xOcclLeft >= state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX])&&(xOcclRight <= (state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]))){
							if(yOcclTop >= state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX]){
								//pan chart to dodge occlusion
								panChartToPos(dat,&state->ds,(uint16_t)floorf(fabsf(mouseX)),(uint16_t)floorf(fabsf(mouseY)));
							}
						}

					}else{
						if(doubleClick && (state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
							//pan chart to nuclide that is clicked
							//printf("starting pan to: %f %f\n",(double)mouseX,(double)mouseY);
							panChartToPos(dat,&state->ds,(uint16_t)floorf(fabsf(mouseX)),(uint16_t)floorf(fabsf(mouseY)));
						}else if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
							startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
							startUIAnimation(&state->ds,UIANIM_NUCLHIGHLIGHT_HIDE);
						}else{
							state->chartSelectedNucl = selNucl;
						}
					}
				}else if(state->ds.shownElements & (1U << UIELEM_NUCL_FULLINFOBOX)){
					//full info box, do nothing for now
				}else{
					//clicked out of a menu
					state->ds.shownElements = 0; //close any menu being shown
					state->ds.shownElements |= (1U << UIELEM_CHARTOFNUCLIDES);
				}
      }
      break;
  }
}

//updates UI element (buttons etc.) positions, based on the screen resolution and other factors
//positioning constants are defined in gui_constants.h
void updateSingleUIElemPosition(drawing_state *restrict ds, const uint8_t uiElemInd){
	switch(uiElemInd){
		case UIELEM_MENU_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-MENU_BUTTON_WIDTH-MENU_BUTTON_POS_XR);
			ds->uiElemPosY[uiElemInd] = MENU_BUTTON_POS_Y;
			ds->uiElemWidth[uiElemInd] = MENU_BUTTON_WIDTH;
			ds->uiElemHeight[uiElemInd] = UI_TILE_SIZE;
			break;
		case UIELEM_PRIMARY_MENU:
			ds->uiElemPosX[uiElemInd] = (uint16_t)(ds->windowXRes-PRIMARY_MENU_WIDTH-PRIMARY_MENU_POS_XR);
			ds->uiElemPosY[uiElemInd] = PRIMARY_MENU_POS_Y;
			ds->uiElemWidth[uiElemInd] = PRIMARY_MENU_WIDTH;
			ds->uiElemHeight[uiElemInd] = PRIMARY_MENU_HEIGHT;
			break;
		case UIELEM_MSG_BOX:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_WIDTH)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes - MESSAGE_BOX_HEIGHT)/2);
			ds->uiElemWidth[uiElemInd] = MESSAGE_BOX_WIDTH;
			ds->uiElemHeight[uiElemInd] = MESSAGE_BOX_HEIGHT;
			break;
		case UIELEM_MSG_BOX_OK_BUTTON:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - MESSAGE_BOX_OK_BUTTON_WIDTH)/2);
			ds->uiElemPosY[uiElemInd] = (uint16_t)((ds->windowYRes + MESSAGE_BOX_HEIGHT)/2 - MESSAGE_BOX_OK_BUTTON_YB - UI_TILE_SIZE);
			ds->uiElemWidth[uiElemInd] = MESSAGE_BOX_OK_BUTTON_WIDTH;
			ds->uiElemHeight[uiElemInd] = UI_TILE_SIZE;
			break;
		case UIELEM_NUCL_INFOBOX:
			ds->uiElemPosX[uiElemInd] = (uint16_t)((ds->windowXRes - NUCL_INFOBOX_WIDTH)/2);
			uint16_t freeXSpace = (uint16_t)(ds->windowXRes - NUCL_INFOBOX_WIDTH);
			if(freeXSpace < 4*NUCL_INFOBOX_X_PADDING){
				ds->uiElemPosX[uiElemInd] += (uint16_t)(NUCL_INFOBOX_X_PADDING - freeXSpace/4); //make sure info box doesn't bump up against y-axis
			}
			ds->uiElemHeight[uiElemInd] = (uint16_t)((float)NUCL_INFOBOX_MIN_HEIGHT + ds->infoBoxTableHeight);
			ds->uiElemPosY[uiElemInd] = (uint16_t)(ds->windowYRes - ds->uiElemHeight[uiElemInd] - UI_PADDING_SIZE - (int32_t)CHART_AXIS_DEPTH);
			ds->uiElemWidth[uiElemInd] = (uint16_t)(NUCL_INFOBOX_WIDTH);
			//update child/dependant UI elements
			updateSingleUIElemPosition(ds,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			updateSingleUIElemPosition(ds,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			break;
		case UIELEM_NUCL_INFOBOX_CLOSEBUTTON:
			ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = UI_TILE_SIZE;
			ds->uiElemHeight[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON];
			ds->uiElemPosX[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - 4*UI_PADDING_SIZE);
			ds->uiElemPosY[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + 4*UI_PADDING_SIZE;
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
			ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = NUCL_INFOBOX_ALLLEVELS_BUTTON_WIDTH;
			ds->uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = UI_TILE_SIZE;
			if(ds->uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
				float animFrac = juice_smoothStop3(1.0f - ds->timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/UI_ANIM_LENGTH);
				uint16_t defaultPosX = (uint16_t)((ds->windowXRes - NUCL_INFOBOX_WIDTH)/2 + NUCL_INFOBOX_WIDTH - UI_TILE_SIZE - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - 7*UI_PADDING_SIZE);
				uint16_t defaultPosY = (uint16_t)(ds->windowYRes - (uint16_t)((float)NUCL_INFOBOX_MIN_HEIGHT + ds->infoBoxTableHeight) - UI_PADDING_SIZE - (int32_t)CHART_AXIS_DEPTH + 4*UI_PADDING_SIZE);
				uint16_t fullPosX = (uint16_t)(ds->windowXRes-NUCL_FULLINFOBOX_BACKBUTTON_WIDTH-NUCL_FULLINFOBOX_BACKBUTTON_POS_XR);
				uint16_t fullPosY = NUCL_FULLINFOBOX_BACKBUTTON_POS_Y;
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(defaultPosX + animFrac*(fullPosX - defaultPosX));
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(defaultPosY + animFrac*(fullPosY - defaultPosY));
			}else if(ds->uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
				float animFrac = juice_smoothStart3(1.0f - ds->timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH);
				uint16_t defaultPosX = (uint16_t)((ds->windowXRes - NUCL_INFOBOX_WIDTH)/2 + NUCL_INFOBOX_WIDTH - UI_TILE_SIZE - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - 7*UI_PADDING_SIZE);
				uint16_t defaultPosY = (uint16_t)(ds->windowYRes - (uint16_t)((float)NUCL_INFOBOX_MIN_HEIGHT + ds->infoBoxTableHeight) - UI_PADDING_SIZE - (int32_t)CHART_AXIS_DEPTH + 4*UI_PADDING_SIZE);
				uint16_t fullPosX = (uint16_t)(ds->windowXRes-NUCL_FULLINFOBOX_BACKBUTTON_WIDTH-NUCL_FULLINFOBOX_BACKBUTTON_POS_XR);
				uint16_t fullPosY = NUCL_FULLINFOBOX_BACKBUTTON_POS_Y;
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(fullPosX + animFrac*(defaultPosX - fullPosX));
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(fullPosY + animFrac*(defaultPosY - fullPosY));
			}else{
				ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (uint16_t)(ds->uiElemPosX[UIELEM_NUCL_INFOBOX] + ds->uiElemWidth[UIELEM_NUCL_INFOBOX] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - 7*UI_PADDING_SIZE);
				ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = ds->uiElemPosY[UIELEM_NUCL_INFOBOX] + 4*UI_PADDING_SIZE;
			}
			//printf("x: %u, y: %u, w: %u, h: %u\n",ds->uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],ds->uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON]);
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			ds->uiElemPosX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)(ds->windowXRes-NUCL_FULLINFOBOX_BACKBUTTON_WIDTH-NUCL_FULLINFOBOX_BACKBUTTON_POS_XR);
			ds->uiElemPosY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = NUCL_FULLINFOBOX_BACKBUTTON_POS_Y;
			ds->uiElemWidth[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = NUCL_FULLINFOBOX_BACKBUTTON_WIDTH;
			ds->uiElemHeight[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = UI_TILE_SIZE;
			break;
		default:
			break;
	}
}
void updateUIElemPositions(drawing_state *restrict ds){
  for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){
    updateSingleUIElemPosition(ds,i);
  }
}

float getUIthemeScale(const float uiScale){
	if(fabsf(uiScale - 1.0f) < 0.001f){
		return uiScale;
	}else{
		return roundf(2.0f*uiScale); //super-sample UI elements, hack to prevent scaling artifacts
	}
}

void updateWindowRes(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  int wwidth, wheight;
  int rwidth, rheight;
  SDL_GetWindowSize(rdat->window, &wwidth, &wheight);
  SDL_GetWindowSizeInPixels(rdat->window, &rwidth, &rheight);
  if((rwidth != state->ds.windowXRenderRes)||(rheight != state->ds.windowYRenderRes)){
    state->ds.forceRedraw = 1;
  }
	float newScale = (float)rwidth/((float)wwidth);
	//float newScale = 1.0f; //for testing UI scales
	if(fabsf(rdat->uiScale - newScale) > 0.001f){
		printf("Re-scaling UI from %0.9f to %0.9f.\n",(double)rdat->uiScale,(double)newScale);
		rdat->uiScale = newScale; //set UI scale properly for HI-DPI
		rdat->uiThemeScale = getUIthemeScale(rdat->uiScale);
		if(rdat->font){
			//rescale font and UI theme as well, this requires loading them from the app data file
			regenerateThemeAndFontCache(dat,rdat); //load_data.c
		}
	}
  state->ds.windowXRes = (uint16_t)wwidth;
  state->ds.windowYRes = (uint16_t)wheight;
  state->ds.windowXRenderRes = (uint16_t)rwidth;
  state->ds.windowYRenderRes = (uint16_t)rheight;

  //update things that depend on the window res
  updateUIElemPositions(&state->ds); //UI element positions
	changeUIState(dat,state,state->uiState);
}

void handleScreenGraphicsMode(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  //handle vsync and frame cap
  SDL_SetRenderVSync(rdat->renderer,1); //vsync always enabled

  if(state->ds.windowFullscreenMode){
    if(SDL_SetWindowFullscreen(rdat->window,SDL_TRUE) != 0){
      printf("WARNING: cannot set fullscreen mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,state,rdat);
    //printf("Full screen display mode.  Window resolution: %u x %u.\n",state->ds.windowXRes,state->ds.windowYRes);
  }else{
    if(SDL_SetWindowFullscreen(rdat->window,0) != 0){
      printf("WARNING: cannot set windowed mode - %s\n",SDL_GetError());
    }
    updateWindowRes(dat,state,rdat);
    //printf("Windowed display mode.  Window resolution: %u x %u.\n",state->ds.windowXRes,state->ds.windowYRes);
  }
}
