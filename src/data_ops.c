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

/* Functions handling low-level operations on ENSDF database, or calculations using app data/state */

#include "juicer.h"
#include "strops.h"
#include "data_ops.h"
#include "drawing.h"
#include "load_data.h"
#include "gui_constants.h"

//Initializes the temporary (unsaved) portion of the app state.
//The default preferences are set here, these will later be overwritten during
//app startup by the values in con.ini
void initializeTempState(const app_data *restrict dat, app_state *restrict state, thread_manager_state *restrict tms){
  
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
	state->inputFlags = 0;
	state->interactableElement = 0;
	//app state
	state->chartSelectedNucl = MAXNUMNUCL;
	state->chartView = CHARTVIEW_HALFLIFE;
	state->quitAppFlag = 0;
	state->gamepadDeadzone = 16000;
	state->gamepadTriggerDeadzone = 4000;
	state->ds.windowXRes = 880;
	state->ds.windowYRes = 580;
	state->ds.drawPerformanceStats = 0;
	memset(state->ss.searchString,0,sizeof(state->ss.searchString));
	state->searchStrUpdated = 0;
	//ui state
	state->lastUIState = UISTATE_CHARTONLY;
	changeUIState(dat,state,UISTATE_CHARTONLY);
	state->clickedUIElem = UIELEM_ENUM_LENGTH; //no selected UI element
	state->lastOpenedMenu = UIELEM_ENUM_LENGTH; //no previously selected menu
	state->ds.shownElements = 0; //no UI elements being shown
	state->ds.shownElements |= ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES);
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
	state->ds.mouseOverRxn = 255;
	state->ds.mouseHoldRxn = 255;
	state->ds.selectedRxn = 0;
	state->ds.totalPanTime = CHART_KEY_PAN_TIME;
	state->ds.zoomFinished = 0;
	state->ds.zoomInProgress = 0;
	state->ds.chartDragFinished = 0;
	state->ds.textDragFinished = 0;
	state->ds.chartDragInProgress = 0;
	state->ds.textDragInProgress = 0;
	state->ds.fcScrollFinished = 0;
	state->ds.fcScrollInProgress = 0;
	state->ds.fcNuclChangeInProgress = 0;
	state->ds.searchEntryDispStartChar = 0;
	state->ds.searchEntryDispNumChars = 65535U; //default value specifying no text has been input yet
	state->ds.interfaceSizeInd = UISCALE_NORMAL;
	state->cms.numContextMenuItems = 0;
	state->ss.numResults = 0;
	state->ss.canUpdateResults = SDL_CreateSemaphore(1);
	clearSelectionStrs(&state->tss,0);
	memset(state->ds.uiElemExtPlusX,0,sizeof(state->ds.uiElemExtPlusX));
	memset(state->ds.uiElemExtPlusY,0,sizeof(state->ds.uiElemExtPlusY));
	memset(state->ds.uiElemExtMinusX,0,sizeof(state->ds.uiElemExtMinusX));
	memset(state->ds.uiElemExtMinusY,0,sizeof(state->ds.uiElemExtMinusY));
	//threads
	for(uint8_t i=0;i<MAX_NUM_THREADS;i++){
		tms->threadData[i].threadState = THREADSTATE_DEAD;
	}

	//check that constants are valid
	if(UIELEM_ENUM_LENGTH > /* DISABLES CODE */ (64)){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ui_element_enum is too long, cannot be indexed by a uint64_t bit pattern (ds->shownElements, state->interactableElement)!\n");
		exit(-1);
	}
	if(UIANIM_ENUM_LENGTH > /* DISABLES CODE */ (32)){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"ui_animation_enum is too long, cannot be indexed by a uint32_t bit pattern (ds->uiAnimPlaying)!\n");
		exit(-1);
	}
	if(SEARCHAGENT_ENUM_LENGTH > /* DISABLES CODE */ (32)){
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"search_agent_enum is too long, cannot be indexed by a uint32_t bit pattern (ss->finishedSearchAgents)!\n");
		exit(-1);
	}

}

//callback for saving screenshots
//userdata points to the application resource_data
void saveScreenshotCallback(void *userdata, const char * const *filelist, int filter){
	(void)filter; //unused for now

	resource_data *rdat = ((resource_data*)(intptr_t)userdata);
	
	if(rdat->ssdat.screenshot){
		if(filelist){
			//loop through all files
			while(*filelist){
				const char *fileName;
				//handle string format returned by file dialog
				if(strncmp(*filelist,"file://",7)==0){
					fileName = (*filelist)+7;
				}else{
					fileName = (*filelist);
				}
				if(IMG_SavePNG(rdat->ssdat.screenshot,fileName) == false){
					SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"saveScreenshotCallback - error saving PNG file: %s",SDL_GetError());
					break;
				}
				break; //only one filename can be provided
			}
		}else{
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"saveScreenshotCallback - file selection error: %s",SDL_GetError());
		}
		SDL_DestroySurface(rdat->ssdat.screenshot);
	}

	rdat->ssdat.takingScreenshot = 0; //no longer taking a screenshot
  //successfully saved file
  //changeUIState(((app_state*)userdata),UISTATE_DRAWAREAONLY); //make buttons interactable again
}

void takeScreenshot(resource_data *restrict rdat){
	rdat->ssdat.takingScreenshot = 2; //currently taking a screenshot
	rdat->ssdat.screenshot = SDL_RenderReadPixels(rdat->renderer,NULL);
	const SDL_DialogFileFilter filters[2] = {
		{"Image files (.png)","png"},
		{NULL,NULL}
	};
	SDL_ShowSaveFileDialog(saveScreenshotCallback,(void *)(intptr_t)(rdat),rdat->window,filters,1,NULL);
}

//resets the text selection state, should be called on frames where selectable text changes position
void clearSelectionStrs(text_selection_state *restrict tss, const uint8_t modifiableAfter){
	tss->selStartPos = 0;
	tss->selEndPos = 0;
	tss->selectedStr = 65535; //no string selected
	tss->numSelStrs = 0; //no selectable strings defined
	tss->selStrsModifiable = modifiableAfter;
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
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_PRIMARY_MENU)); //close the menu
			if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_DIALOG))){
				if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_ABOUT_BOX))){
					if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))){
						if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU))){
							if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_RXN_MENU))){
								if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
									if((state->mouseoverElement != UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)||(state->lastOpenedMenu != UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)||(state->lastInputType != INPUT_TYPE_KEYBOARD)){
										changeUIState(dat,state,UISTATE_FULLLEVELINFO);
									}
								}else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
									changeUIState(dat,state,UISTATE_INFOBOX);
								}else{
									changeUIState(dat,state,UISTATE_CHARTONLY);
								}
							}
						}
					}
				}
			}
			break;
		case UIANIM_CHARTVIEW_MENU_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_CHARTVIEW_MENU)); //close the menu
			if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))){
				if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU))){
					if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
						changeUIState(dat,state,UISTATE_INFOBOX);
					}else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
						changeUIState(dat,state,UISTATE_FULLLEVELINFO);
					}else{
						changeUIState(dat,state,UISTATE_CHARTONLY);
					}
				}
			}
			break;
		case UIANIM_SEARCH_MENU_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_SEARCH_MENU)); //close the menu
			if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))){
				if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))){
					if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
						changeUIState(dat,state,UISTATE_INFOBOX);
					}else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
						changeUIState(dat,state,UISTATE_FULLLEVELINFO);
					}else{
						changeUIState(dat,state,UISTATE_CHARTONLY);
					}
				}
			}
			break;
		case UIANIM_UISCALE_MENU_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU)); //close the menu
			changeUIState(dat,state,UISTATE_PREFS_DIALOG);
			break;
		case UIANIM_RXN_MENU_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_RXN_MENU)); //close the menu
			if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))){
				if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES))){ //in case the menu was hidden by going back to the main chart
					if((state->mouseoverElement != UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)||(state->lastOpenedMenu != UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)||(state->lastInputType != INPUT_TYPE_KEYBOARD)){
						changeUIState(dat,state,UISTATE_FULLLEVELINFO);
					}
				}
			}
			break;
    case UIANIM_MODAL_BOX_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_ABOUT_BOX)); //close the about box
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_PREFS_DIALOG)); //close the preferences dialog
			if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
				changeUIState(dat,state,UISTATE_FULLLEVELINFO);
			}else if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
				changeUIState(dat,state,UISTATE_INFOBOX);
			}else{
				changeUIState(dat,state,UISTATE_CHARTONLY);
			}
			//SDL_Log("UI state: %u\n",state->uiState);
      break;
		case UIANIM_NUCLINFOBOX_SHOW:
			clearSelectionStrs(&state->tss,1); //allow strings on the info box to be selectable
			break;
		case UIANIM_NUCLINFOBOX_HIDE:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_NUCL_INFOBOX)); //close the info box
			state->chartSelectedNucl = MAXNUMNUCL;
			break;
		case UIANIM_NUCLINFOBOX_EXPAND:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_NUCL_INFOBOX)); //close the info box
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES)); //don't show the chart
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX); //show the full info box
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_TXTFADEIN);
			changeUIState(dat,state,UISTATE_FULLLEVELINFO); //update UI state now that the full info box is visible
			break;
		case UIANIM_NUCLINFOBOX_CONTRACT:
			clearSelectionStrs(&state->tss,1); //allow strings on the info box to be selectable
			break;
		case UIANIM_NUCLINFOBOX_TXTFADEIN:
			clearSelectionStrs(&state->tss,1); //allow strings on the full info box to be selectable
			break;
		case UIANIM_NUCLINFOBOX_TXTFADEOUT:
			state->ds.shownElements &= (~((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)); //close the full info box
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_NUCL_INFOBOX); //show the info box
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES); //show the chart
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_CONTRACT);
			changeUIState(dat,state,UISTATE_INFOBOX); //update UI state now that the regular info box is visible
			clearSelectionStrs(&state->tss,0); //strings on full level list are no longer selectable
			break;
		case UIANIM_CONTEXT_MENU_HIDE:
			state->cms.numContextMenuItems = 0; //prevent further interactions with the context menu
			state->ds.uiElemPosX[UIELEM_CONTEXT_MENU] = -1;
			state->ds.uiElemPosY[UIELEM_CONTEXT_MENU] = -1;
			state->ds.uiElemWidth[UIELEM_CONTEXT_MENU] = 0;
			state->ds.uiElemWidth[UIELEM_CONTEXT_MENU] = 0;
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
		changeUIState(dat,state,state->uiState); //update interactable elements (specifically zoom buttons)
	}
	if(state->ds.chartDragFinished){
		state->ds.chartDragInProgress = 0;
		state->ds.chartDragFinished = 0; //reset flag
		SDL_SetCursor(rdat->defaultCursor); //set cursor back to default
	}
	if(state->ds.textDragFinished){
		state->ds.textDragInProgress = 0;
		state->ds.textDragFinished = 0; //reset flag
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
			state->mouseholdElement = UIELEM_ENUM_LENGTH; //remove highlight from zoom buttons
			clearSelectionStrs(&state->tss,1); //allow strings on the chart to be selectable
		}
		//SDL_Log("zoom scale: %0.4f\n",(double)state->ds.chartZoomScale);
	}
	if(state->ds.textDragInProgress == 1){
		if(state->lastInputType == INPUT_TYPE_MOUSE){
      SDL_SetCursor(rdat->textEntryCursor); //set mouse cursor
    }
	}
	if(state->ds.chartDragInProgress){
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
			clearSelectionStrs(&state->tss,1); //allow strings on the chart to be selectable
		}
		//SDL_Log("pan t: %0.3f\n",(double)state->ds.timeSincePanStart);
	}
	if(state->ds.fcScrollInProgress){
		state->ds.timeSinceFCScollStart += deltaTime;
		state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollStartY + (state->ds.nuclFullInfoScrollToY - state->ds.nuclFullInfoScrollStartY)*juice_smoothStop2(state->ds.timeSinceFCScollStart/NUCL_FULLINFOBOX_SCROLL_TIME);
		if(state->ds.timeSinceFCScollStart >= NUCL_FULLINFOBOX_SCROLL_TIME){
			state->ds.nuclFullInfoScrollY = state->ds.nuclFullInfoScrollToY;
			state->ds.fcScrollFinished = 1;
			clearSelectionStrs(&state->tss,1); //now that scroll is finished, allow selection strings to be regenerated
		}
		//SDL_Log("scroll t: %0.3f, pos: %f\n",(double)state->ds.timeSinceFCScollStart,(double)state->ds.nuclFullInfoScrollY);
	}
	if(state->ds.fcNuclChangeInProgress){
		state->ds.timeSinceFCNuclChangeStart += deltaTime;
		if(state->kbdModVal == KBD_MOD_SHIFT){
			if(state->ds.timeSinceFCNuclChangeStart >= NUCL_FULLINFOBOX_SCROLL_TIME*PAN_SPRINT_MULTIPLIER){
				state->ds.fcNuclChangeFinished = 1;
			}
		}else{
			if(state->ds.timeSinceFCNuclChangeStart >= NUCL_FULLINFOBOX_SCROLL_TIME){
				state->ds.fcNuclChangeFinished = 1;
			}
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
		if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX)){
			if(state->ds.panInProgress == 0){ //don't hide while panning (eg. when panning to a search result)
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

}

void setupNuclideContextMenu(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){
  char nuclStr[32];
  getNuclNameStr(nuclStr,&dat->ndat.nuclData[nuclInd],255);
	SDL_strlcpy(state->cms.headerText,nuclStr,32);
	state->cms.selectionInd = nuclInd;
	state->cms.useHeaderText = 1;
	state->cms.numContextMenuItems = 3;
	state->cms.contextMenuItems[0] = CONTEXTITEM_NUCLNAME;
	state->cms.contextMenuItems[1] = CONTEXTITEM_NUCLINFO;
	state->cms.contextMenuItems[2] = CONTEXTITEM_CHART_PROPERTY;
	state->cms.mouseOverContextItem = 255;
	state->cms.clickedContextItem = 255;
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_CONTEXT_MENU);
	//const SDL_FRect conMenuRect = {(float)state->ds.uiElemPosX[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemPosY[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemWidth[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemHeight[UIELEM_CONTEXT_MENU]};
	//removeSelectableStringsInRect(&state->tss, conMenuRect);
	startUIAnimation(dat,state,UIANIM_CONTEXT_MENU_SHOW);
}

void setupCopyContextMenu(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  state->cms.useHeaderText = 0;
	state->cms.numContextMenuItems = 1;
	state->cms.contextMenuItems[0] = CONTEXTITEM_COPY;
	state->cms.mouseOverContextItem = 255;
	state->cms.clickedContextItem = 255;
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_CONTEXT_MENU);
	//const SDL_FRect conMenuRect = {(float)state->ds.uiElemPosX[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemPosY[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemWidth[UIELEM_CONTEXT_MENU], (float)state->ds.uiElemHeight[UIELEM_CONTEXT_MENU]};
	//removeSelectableStringsInRect(&state->tss, conMenuRect);
	startUIAnimation(dat,state,UIANIM_CONTEXT_MENU_SHOW);
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

//isomerMVal = 255 for non-isomers, 0 for isomers where the m-index is not drawn
void getNuclNameStr(char strOut[32], const nucl *restrict nuclide, const uint8_t isomerMVal){
	char *nameStrCpy;
	if(isomerMVal == 255){
		SDL_snprintf(strOut,32,"%u%s",nuclide->Z + nuclide->N,getElemStr((uint8_t)(nuclide->Z)));
	}else if(isomerMVal == 0){
		SDL_snprintf(strOut,32,"%uᵐ%s",nuclide->Z + nuclide->N,getElemStr((uint8_t)(nuclide->Z)));
	}else{
		SDL_snprintf(strOut,32,"%uᵐ%u%s",nuclide->Z + nuclide->N,isomerMVal,getElemStr((uint8_t)(nuclide->Z)));
	}
	//convert mass number to superscript
	nameStrCpy = findReplaceAllUTF8("0","⁰",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("1","¹",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("2","²",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("3","³",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("4","⁴",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("5","⁵",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("6","⁶",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("7","⁷",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("8","⁸",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
	nameStrCpy = findReplaceAllUTF8("9","⁹",strOut);
	SDL_strlcpy(strOut,nameStrCpy,32);
	free(nameStrCpy);
}

uint8_t elemStrToZ(const char *elemStr){
	if(strcmp(elemStr,"n")==0){
		return 0;
	}else if(strcmp(elemStr,"H")==0){
		return 1;
	}else if(strcmp(elemStr,"He")==0){
		return 2;
	}else if(strcmp(elemStr,"Li")==0){
		return 3;
	}else if(strcmp(elemStr,"Be")==0){
		return 4;
	}else if(strcmp(elemStr,"B")==0){
		return 5;
	}else if(strcmp(elemStr,"C")==0){
		return 6;
	}else if(strcmp(elemStr,"N")==0){
		return 7;
	}else if(strcmp(elemStr,"O")==0){
		return 8;
	}else if(strcmp(elemStr,"F")==0){
		return 9;
	}else if(strcmp(elemStr,"Ne")==0){
		return 10;
	}else if(strcmp(elemStr,"Na")==0){
		return 11;
	}else if(strcmp(elemStr,"Mg")==0){
		return 12;
	}else if(strcmp(elemStr,"Al")==0){
		return 13;
	}else if(strcmp(elemStr,"Si")==0){
		return 14;
	}else if(strcmp(elemStr,"P")==0){
		return 15;
	}else if(strcmp(elemStr,"S")==0){
		return 16;
	}else if(strcmp(elemStr,"Cl")==0){
		return 17;
	}else if(strcmp(elemStr,"Ar")==0){
		return 18;
	}else if(strcmp(elemStr,"K")==0){
		return 19;
	}else if(strcmp(elemStr,"Ca")==0){
		return 20;
	}else if(strcmp(elemStr,"Sc")==0){
		return 21;
	}else if(strcmp(elemStr,"Ti")==0){
		return 22;
	}else if(strcmp(elemStr,"V")==0){
		return 23;
	}else if(strcmp(elemStr,"Cr")==0){
		return 24;
	}else if(strcmp(elemStr,"Mn")==0){
		return 25;
	}else if(strcmp(elemStr,"Fe")==0){
		return 26;
	}else if(strcmp(elemStr,"Co")==0){
		return 27;
	}else if(strcmp(elemStr,"Ni")==0){
		return 28;
	}else if(strcmp(elemStr,"Cu")==0){
		return 29;
	}else if(strcmp(elemStr,"Zn")==0){
		return 30;
	}else if(strcmp(elemStr,"Ga")==0){
		return 31;
	}else if(strcmp(elemStr,"Ge")==0){
		return 32;
	}else if(strcmp(elemStr,"As")==0){
		return 33;
	}else if(strcmp(elemStr,"Se")==0){
		return 34;
	}else if(strcmp(elemStr,"Br")==0){
		return 35;
	}else if(strcmp(elemStr,"Kr")==0){
		return 36;
	}else if(strcmp(elemStr,"Rb")==0){
		return 37;
	}else if(strcmp(elemStr,"Sr")==0){
		return 38;
	}else if(strcmp(elemStr,"Y")==0){
		return 39;
	}else if(strcmp(elemStr,"Zr")==0){
		return 40;
	}else if(strcmp(elemStr,"Nb")==0){
		return 41;
	}else if(strcmp(elemStr,"Mo")==0){
		return 42;
	}else if(strcmp(elemStr,"Tc")==0){
		return 43;
	}else if(strcmp(elemStr,"Ru")==0){
		return 44;
	}else if(strcmp(elemStr,"Rh")==0){
		return 45;
	}else if(strcmp(elemStr,"Pd")==0){
		return 46;
	}else if(strcmp(elemStr,"Ag")==0){
		return 47;
	}else if(strcmp(elemStr,"Cd")==0){
		return 48;
	}else if(strcmp(elemStr,"In")==0){
		return 49;
	}else if(strcmp(elemStr,"Sn")==0){
		return 50;
	}else if(strcmp(elemStr,"Sb")==0){
		return 51;
	}else if(strcmp(elemStr,"Te")==0){
		return 52;
	}else if(strcmp(elemStr,"I")==0){
		return 53;
	}else if(strcmp(elemStr,"Xe")==0){
		return 54;
	}else if(strcmp(elemStr,"Cs")==0){
		return 55;
	}else if(strcmp(elemStr,"Ba")==0){
		return 56;
	}else if(strcmp(elemStr,"La")==0){
		return 57;
	}else if(strcmp(elemStr,"Ce")==0){
		return 58;
	}else if(strcmp(elemStr,"Pr")==0){
		return 59;
	}else if(strcmp(elemStr,"Nd")==0){
		return 60;
	}else if(strcmp(elemStr,"Pm")==0){
		return 61;
	}else if(strcmp(elemStr,"Sm")==0){
		return 62;
	}else if(strcmp(elemStr,"Eu")==0){
		return 63;
	}else if(strcmp(elemStr,"Gd")==0){
		return 64;
	}else if(strcmp(elemStr,"Tb")==0){
		return 65;
	}else if(strcmp(elemStr,"Dy")==0){
		return 66;
	}else if(strcmp(elemStr,"Ho")==0){
		return 67;
	}else if(strcmp(elemStr,"Er")==0){
		return 68;
	}else if(strcmp(elemStr,"Tm")==0){
		return 69;
	}else if(strcmp(elemStr,"Yb")==0){
		return 70;
	}else if(strcmp(elemStr,"Lu")==0){
		return 71;
	}else if(strcmp(elemStr,"Hf")==0){
		return 72;
	}else if(strcmp(elemStr,"Ta")==0){
		return 73;
	}else if(strcmp(elemStr,"W")==0){
		return 74;
	}else if(strcmp(elemStr,"Re")==0){
		return 75;
	}else if(strcmp(elemStr,"Os")==0){
		return 76;
	}else if(strcmp(elemStr,"Ir")==0){
		return 77;
	}else if(strcmp(elemStr,"Pt")==0){
		return 78;
	}else if(strcmp(elemStr,"Au")==0){
		return 79;
	}else if(strcmp(elemStr,"Hg")==0){
		return 80;
	}else if(strcmp(elemStr,"Tl")==0){
		return 81;
	}else if(strcmp(elemStr,"Pb")==0){
		return 82;
	}else if(strcmp(elemStr,"Bi")==0){
		return 83;
	}else if(strcmp(elemStr,"Po")==0){
		return 84;
	}else if(strcmp(elemStr,"At")==0){
		return 85;
	}else if(strcmp(elemStr,"Rn")==0){
		return 86;
	}else if(strcmp(elemStr,"Fr")==0){
		return 87;
	}else if(strcmp(elemStr,"Ra")==0){
		return 88;
	}else if(strcmp(elemStr,"Ac")==0){
		return 89;
	}else if(strcmp(elemStr,"Th")==0){
		return 90;
	}else if(strcmp(elemStr,"Pa")==0){
		return 91;
	}else if(strcmp(elemStr,"U")==0){
		return 92;
	}else if(strcmp(elemStr,"Np")==0){
		return 93;
	}else if(strcmp(elemStr,"Pu")==0){
		return 94;
	}else if(strcmp(elemStr,"Am")==0){
		return 95;
	}else if(strcmp(elemStr,"Cm")==0){
		return 96;
	}else if(strcmp(elemStr,"Bk")==0){
		return 97;
	}else if(strcmp(elemStr,"Cf")==0){
		return 98;
	}else if(strcmp(elemStr,"Es")==0){
		return 99;
	}else if(strcmp(elemStr,"Fm")==0){
		return 100;
	}else if(strcmp(elemStr,"Md")==0){
		return 101;
	}else if(strcmp(elemStr,"No")==0){
		return 102;
	}else if(strcmp(elemStr,"Lr")==0){
		return 103;
	}else if(strcmp(elemStr,"Rf")==0){
		return 104;
	}else if(strcmp(elemStr,"Db")==0){
		return 105;
	}else if(strcmp(elemStr,"Sg")==0){
		return 106;
	}else if(strcmp(elemStr,"Bh")==0){
		return 107;
	}else if(strcmp(elemStr,"Hs")==0){
		return 108;
	}else if(strcmp(elemStr,"Mt")==0){
		return 109;
	}else if(strcmp(elemStr,"Ds")==0){
		return 110;
	}else if(strcmp(elemStr,"Rg")==0){
		return 111;
	}else if(strcmp(elemStr,"Cn")==0){
		return 112;
	}else if(strcmp(elemStr,"Nh")==0){
		return 113;
	}else if(strcmp(elemStr,"Fl")==0){
		return 114;
	}else if(strcmp(elemStr,"Mc")==0){
		return 115;
	}else if(strcmp(elemStr,"Lv")==0){
		return 116;
	}else if(strcmp(elemStr,"Ts")==0){
		return 117;
	}else if(strcmp(elemStr,"Og")==0){
		return 118;
	}else if(strcmp(elemStr,"Neutron")==0){
		return 0;
	}else if((strcmp(elemStr,"Hydrogen")==0)||(strcmp(elemStr,"Proton")==0)){
		return 1;
	}else if(strcmp(elemStr,"Helium")==0){
		return 2;
	}else if(strcmp(elemStr,"Lithium")==0){
		return 3;
	}else if(strcmp(elemStr,"Beryllium")==0){
		return 4;
	}else if(strcmp(elemStr,"Boron")==0){
		return 5;
	}else if(strcmp(elemStr,"Carbon")==0){
		return 6;
	}else if(strcmp(elemStr,"Nitrogen")==0){
		return 7;
	}else if(strcmp(elemStr,"Oxygen")==0){
		return 8;
	}else if(strcmp(elemStr,"Fluorine")==0){
		return 9;
	}else if(strcmp(elemStr,"Neon")==0){
		return 10;
	}else if(strcmp(elemStr,"Sodium")==0){
		return 11;
	}else if(strcmp(elemStr,"Magnesium")==0){
		return 12;
	}else if((strcmp(elemStr,"Aluminium")==0)||(strcmp(elemStr,"Aluminum")==0)){
		return 13;
	}else if(strcmp(elemStr,"Silicon")==0){
		return 14;
	}else if(strcmp(elemStr,"Phosphorus")==0){
		return 15;
	}else if((strcmp(elemStr,"Sulfur")==0)||(strcmp(elemStr,"Sulphur")==0)){
		return 16;
	}else if(strcmp(elemStr,"Chlorine")==0){
		return 17;
	}else if(strcmp(elemStr,"Argon")==0){
		return 18;
	}else if(strcmp(elemStr,"Potassium")==0){
		return 19;
	}else if(strcmp(elemStr,"Calcium")==0){
		return 20;
	}else if(strcmp(elemStr,"Scandium")==0){
		return 21;
	}else if(strcmp(elemStr,"Titanium")==0){
		return 22;
	}else if(strcmp(elemStr,"Vanadium")==0){
		return 23;
	}else if(strcmp(elemStr,"Chromium")==0){
		return 24;
	}else if(strcmp(elemStr,"Manganese")==0){
		return 25;
	}else if(strcmp(elemStr,"Iron")==0){
		return 26;
	}else if(strcmp(elemStr,"Cobalt")==0){
		return 27;
	}else if(strcmp(elemStr,"Nickel")==0){
		return 28;
	}else if(strcmp(elemStr,"Copper")==0){
		return 29;
	}else if(strcmp(elemStr,"Zinc")==0){
		return 30;
	}else if(strcmp(elemStr,"Gallium")==0){
		return 31;
	}else if(strcmp(elemStr,"Germanium")==0){
		return 32;
	}else if(strcmp(elemStr,"Arsenic")==0){
		return 33;
	}else if(strcmp(elemStr,"Selenium")==0){
		return 34;
	}else if(strcmp(elemStr,"Bromine")==0){
		return 35;
	}else if(strcmp(elemStr,"Krypton")==0){
		return 36;
	}else if(strcmp(elemStr,"Rubidium")==0){
		return 37;
	}else if(strcmp(elemStr,"Strontium")==0){
		return 38;
	}else if(strcmp(elemStr,"Yttrium")==0){
		return 39;
	}else if(strcmp(elemStr,"Zirconium")==0){
		return 40;
	}else if((strcmp(elemStr,"Niobium")==0)||(strcmp(elemStr,"Columbium")==0)){
		return 41;
	}else if(strcmp(elemStr,"Molybdenum")==0){
		return 42;
	}else if(strcmp(elemStr,"Technetium")==0){
		return 43;
	}else if(strcmp(elemStr,"Ruthenium")==0){
		return 44;
	}else if(strcmp(elemStr,"Rhodium")==0){
		return 45;
	}else if(strcmp(elemStr,"Palladium")==0){
		return 46;
	}else if(strcmp(elemStr,"Silver")==0){
		return 47;
	}else if(strcmp(elemStr,"Cadmium")==0){
		return 48;
	}else if(strcmp(elemStr,"Indium")==0){
		return 49;
	}else if(strcmp(elemStr,"Tin")==0){
		return 50;
	}else if(strcmp(elemStr,"Antimony")==0){
		return 51;
	}else if(strcmp(elemStr,"Tellurium")==0){
		return 52;
	}else if(strcmp(elemStr,"Iodine")==0){
		return 53;
	}else if(strcmp(elemStr,"Xenon")==0){
		return 54;
	}else if((strcmp(elemStr,"Caesium")==0)||(strcmp(elemStr,"Cesium")==0)){
		return 55;
	}else if(strcmp(elemStr,"Barium")==0){
		return 56;
	}else if(strcmp(elemStr,"Lanthanum")==0){
		return 57;
	}else if(strcmp(elemStr,"Cerium")==0){
		return 58;
	}else if(strcmp(elemStr,"Praseodymium")==0){
		return 59;
	}else if(strcmp(elemStr,"Neodymium")==0){
		return 60;
	}else if(strcmp(elemStr,"Promethium")==0){
		return 61;
	}else if(strcmp(elemStr,"Samarium")==0){
		return 62;
	}else if(strcmp(elemStr,"Europium")==0){
		return 63;
	}else if(strcmp(elemStr,"Gadolinium")==0){
		return 64;
	}else if(strcmp(elemStr,"Terbium")==0){
		return 65;
	}else if(strcmp(elemStr,"Dysprosium")==0){
		return 66;
	}else if(strcmp(elemStr,"Holmium")==0){
		return 67;
	}else if(strcmp(elemStr,"Erbium")==0){
		return 68;
	}else if(strcmp(elemStr,"Thulium")==0){
		return 69;
	}else if(strcmp(elemStr,"Ytterbium")==0){
		return 70;
	}else if(strcmp(elemStr,"Lutetium")==0){
		return 71;
	}else if(strcmp(elemStr,"Hafnium")==0){
		return 72;
	}else if(strcmp(elemStr,"Tantalum")==0){
		return 73;
	}else if((strcmp(elemStr,"Tungsten")==0)||(strcmp(elemStr,"Wolfram")==0)){
		return 74;
	}else if(strcmp(elemStr,"Rhenium")==0){
		return 75;
	}else if(strcmp(elemStr,"Osmium")==0){
		return 76;
	}else if(strcmp(elemStr,"Iridium")==0){
		return 77;
	}else if(strcmp(elemStr,"Platinum")==0){
		return 78;
	}else if(strcmp(elemStr,"Gold")==0){
		return 79;
	}else if(strcmp(elemStr,"Mercury")==0){
		return 80;
	}else if(strcmp(elemStr,"Thallium")==0){
		return 81;
	}else if(strcmp(elemStr,"Lead")==0){
		return 82;
	}else if(strcmp(elemStr,"Bismuth")==0){
		return 83;
	}else if(strcmp(elemStr,"Polonium")==0){
		return 84;
	}else if(strcmp(elemStr,"Astatine")==0){
		return 85;
	}else if(strcmp(elemStr,"Radon")==0){
		return 86;
	}else if(strcmp(elemStr,"Francium")==0){
		return 87;
	}else if(strcmp(elemStr,"Radium")==0){
		return 88;
	}else if(strcmp(elemStr,"Actinium")==0){
		return 89;
	}else if(strcmp(elemStr,"Thorium")==0){
		return 90;
	}else if(strcmp(elemStr,"Protactinium")==0){
		return 91;
	}else if(strcmp(elemStr,"Uranium")==0){
		return 92;
	}else if(strcmp(elemStr,"Neptunium")==0){
		return 93;
	}else if(strcmp(elemStr,"Plutonium")==0){
		return 94;
	}else if(strcmp(elemStr,"Americium")==0){
		return 95;
	}else if(strcmp(elemStr,"Curium")==0){
		return 96;
	}else if(strcmp(elemStr,"Berkelium")==0){
		return 97;
	}else if(strcmp(elemStr,"Californium")==0){
		return 98;
	}else if(strcmp(elemStr,"Einsteinium")==0){
		return 99;
	}else if(strcmp(elemStr,"Fermium")==0){
		return 100;
	}else if(strcmp(elemStr,"Mendelevium")==0){
		return 101;
	}else if(strcmp(elemStr,"Nobelium")==0){
		return 102;
	}else if(strcmp(elemStr,"Lawrencium")==0){
		return 103;
	}else if(strcmp(elemStr,"Rutherfordium")==0){
		return 104;
	}else if(strcmp(elemStr,"Dubnium")==0){
		return 105;
	}else if(strcmp(elemStr,"Seaborgium")==0){
		return 106;
	}else if(strcmp(elemStr,"Bohrium")==0){
		return 107;
	}else if(strcmp(elemStr,"Hassium")==0){
		return 108;
	}else if(strcmp(elemStr,"Meitnerium")==0){
		return 109;
	}else if(strcmp(elemStr,"Darmstadtium")==0){
		return 110;
	}else if((strcmp(elemStr,"Roentgenium")==0)||(strcmp(elemStr,"Unununium")==0)){
		return 111;
	}else if(strcmp(elemStr,"Copernicium")==0){
		return 112;
	}else if(strcmp(elemStr,"Nihonium")==0){
		return 113;
	}else if(strcmp(elemStr,"Flerovium")==0){
		return 114;
	}else if(strcmp(elemStr,"Moscovium")==0){
		return 115;
	}else if(strcmp(elemStr,"Livermorium")==0){
		return 116;
	}else if(strcmp(elemStr,"Tennessine")==0){
		return 117;
	}else if(strcmp(elemStr,"Oganesson")==0){
		return 118;
	}

	return 255; //no matching element found
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
		case VALUE_UNIT_AMU:
			return "u";
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
		case DECAYMODE_BETAMINUS_TWONEUTRON:
			return "β-2n";
		case DECAYMODE_BETAMINUS_PROTON:
			return "β-p";
		case DECAYMODE_BETAPLUS_PROTON:
			return "β+p";
		case DECAYMODE_BETAPLUS_TWOPROTON:
			return "β+2p";
		case DECAYMODE_BETAPLUS_THREEPROTON:
			return "β+3p";
		case DECAYMODE_BETAPLUS_ALPHA:
			return "β+α";
		case DECAYMODE_EC:
			return "ε";
		case DECAYMODE_EC_PROTON:
			return "εp";
		case DECAYMODE_EC_TWOPROTON:
			return "ε2p";
		case DECAYMODE_EC_THREEPROTON:
			return "ε3p";
		case DECAYMODE_EC_ALPHA:
			return "εα";
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

const char* getElementFamilyStr(const app_data *restrict dat, const int16_t Z){
	if(Z == 0){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_NONELEMENT]];
	}else if((Z==1)||(Z>=5 && Z <= 8)||(Z>=14 && Z<=16)||(Z>=33 && Z<=34)||(Z==52)){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_NONMETAL]];
	}else if(Z==3 || Z==11 || Z==19 || Z==37 || Z==55 || Z==87){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_ALKALI]];
	}else if(Z==4 || Z==12 || Z==20 || Z==38 || Z==56 || Z==88){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_ALKALIEARTH]];
	}else if((Z>=21 && Z<=29)||(Z>=39 && Z<=47)||(Z>=72 && Z<=79)){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_TRANSITIONMETAL]];
	}else if((Z==13)||(Z>=30 && Z<=32)||(Z>=48 && Z<=51)||(Z>=80 && Z<= 84)){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_METAL]];
	}else if(Z==9 || Z==17 || Z==35 || Z==53 || Z==85){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_HALOGEN]];
	}else if(Z==2 || Z==10 || Z==18 || Z==36 || Z==54 || Z==86){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_NOBLEGAS]];
	}else if(Z>=57 && Z<=71){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_LANTHANIDE]];
	}else if(Z>=89 && Z<=103){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_ACTINIDE]];
	}else if(Z>=104){
		return dat->strings[dat->locStringIDs[LOCSTR_ELEMTYPE_SUPERHEAVY]];
	}else{
		return "Unknown element";
	}
}

void getMassValStr(char strOut[32], const dblValWithErr mVal, const uint8_t showErr){
	uint8_t mPrecision = (uint8_t)(mVal.format & 15U);
	uint8_t mExponent = (uint8_t)((mVal.format >> 4U) & 1U);
	uint8_t mValueType = (uint8_t)((mVal.format >> 5U) & 15U);
	if((showErr == 0)||(mVal.err == 0)){
		if(mExponent == 0){
			SDL_snprintf(strOut,32,"%.*f",mPrecision,mVal.val);
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i",mPrecision,mVal.val,mVal.exponent);
		}
	}else{
		if(mExponent == 0){
			if(mValueType == VALUETYPE_UNKNOWN){
				//estimated
				SDL_snprintf(strOut,32,"%.*f(%u) (est.)",mPrecision,mVal.val,mVal.err);
			}else{
				SDL_snprintf(strOut,32,"%.*f(%u)",mPrecision,mVal.val,mVal.err);
			}
		}else{
			if(mValueType == VALUETYPE_UNKNOWN){
				SDL_snprintf(strOut,32,"%.*f(%u)E%i (est.)",mPrecision,mVal.val,mVal.err,mVal.exponent);
			}else{
				SDL_snprintf(strOut,32,"%.*f(%u)E%i",mPrecision,mVal.val,mVal.err,mVal.exponent);
			}
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
	uint8_t eValueType = (uint8_t)((nd->tran[tranInd].energy.format >> 5U) & 15U);
	if(eValueType == VALUETYPE_X){
		uint8_t variable = (uint8_t)((nd->tran[tranInd].energy.format >> 9U) & 127U);
		SDL_snprintf(strOut,32,"%c",variable);
	}else if(eValueType == VALUETYPE_PLUSX){
		uint8_t variable = (uint8_t)((nd->tran[tranInd].energy.format >> 9U) & 127U);
		if(eExponent == 0){
			SDL_snprintf(strOut,32,"%.*f+%c",ePrecision,(double)(nd->tran[tranInd].energy.val),variable);
		}else{
			SDL_snprintf(strOut,32,"%.*fE%i+%c",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent,variable);
		}
	}else{
		uint8_t eTentative = (uint8_t)((nd->tran[tranInd].energy.format >> 9U) & 1U);
		if((showErr == 0)||(nd->tran[tranInd].energy.err == 0)){
			if(eValueType == VALUETYPE_NUMBER){
				if(eExponent == 0){
					if(eTentative == 0){
						SDL_snprintf(strOut,32,"%.*f",ePrecision,(double)(nd->tran[tranInd].energy.val));
					}else{
						SDL_snprintf(strOut,32,"(%.*f)",ePrecision,(double)(nd->tran[tranInd].energy.val));
					}
				}else{
					if(eTentative == 0){
						SDL_snprintf(strOut,32,"%.*fE%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
					}else{
						SDL_snprintf(strOut,32,"(%.*fE%i)",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
					}
				}
			}else{
				if(eExponent == 0){
					if(eTentative == 0){
						SDL_snprintf(strOut,32,"%s%.*f",getValueTypeShortStr(eValueType),ePrecision,(double)(nd->tran[tranInd].energy.val));
					}else{
						SDL_snprintf(strOut,32,"(%s%.*f)",getValueTypeShortStr(eValueType),ePrecision,(double)(nd->tran[tranInd].energy.val));
					}
				}else{
					if(eTentative == 0){
						SDL_snprintf(strOut,32,"%s%.*fE%i",getValueTypeShortStr(eValueType),ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
					}else{
						SDL_snprintf(strOut,32,"(%s%.*fE%i)",getValueTypeShortStr(eValueType),ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.exponent);
					}
				}
			}
		}else{
			if(eExponent == 0){
				if(eTentative == 0){
					SDL_snprintf(strOut,32,"%.*f(%u)",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err);
				}else{
					SDL_snprintf(strOut,32,"(%.*f(%u))",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err);
				}
			}else{
				if(eTentative == 0){
					SDL_snprintf(strOut,32,"%.*f(%u)E%i",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err,nd->tran[tranInd].energy.exponent);
				}else{
					SDL_snprintf(strOut,32,"(%.*f(%u)E%i)",ePrecision,(double)(nd->tran[tranInd].energy.val),nd->tran[tranInd].energy.err,nd->tran[tranInd].energy.exponent);
				}
			}
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

	SDL_strlcpy(strOut,"",32); //clear the string
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
				SDL_strlcat(strOut,"(",32);
			}
		}else if(mTentative == TENTATIVEMULT_DERIVED){
			if(derived == 0){
				derived = 1;
				SDL_strlcat(strOut,"[",32);
			}
		}else if(mTentative == TENTATIVESP_NONE){
			if(tentative == 1){
				tentative = 0;
				SDL_strlcat(strOut,")",32);
			}
			if(derived == 1){
				derived = 0;
				SDL_strlcat(strOut,"]",32);
			}
		}
		if(i>0){
			if(multsCanMix(nd->tran[tranInd].multipole[i],nd->tran[tranInd].multipole[i-1])){
				SDL_strlcat(strOut,"+",32);
			}else{
				SDL_strlcat(strOut,",",32);
			}
		}

		if(mDQ){
			//dipole/quadrupole assignment
			//check placeholder multipole values to determine which is which
			if((mEM == 0)&&(mOrder == 2)){
				SDL_strlcat(strOut,"Q",32);
			}else if((mEM == 1)&&(mOrder == 1)){
				SDL_strlcat(strOut,"D",32);
			}
		}else{
			if(mEM){
				SDL_strlcat(strOut,"M",32);
			}else{
				SDL_strlcat(strOut,"E",32);
			}
			char order[16];
		  SDL_snprintf(order,16,"%u",mOrder);
			SDL_strlcat(strOut,order,32);
		}

		if(i==(nd->tran[tranInd].numMultipoles-1)){
			if(derived == 1){
				SDL_strlcat(strOut,"]",32);
			}
			if(tentative == 1){
				SDL_strlcat(strOut,")",32);
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
			SDL_snprintf(strOut,32,"Stable");
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

void getMostProbableDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd){

	uint32_t probDcyModeInd = MAXNUMDECAYMODES;

	uint8_t hlUnit = nd->levels[lvlInd].halfLife.unit;
	if(hlUnit == VALUE_UNIT_STABLE){
		SDL_snprintf(strOut,32," "); //stable
		return;
	}else if((nd->levels[lvlInd].numDecModes == 0)&&(getLevelHalfLifeSeconds(nd,lvlInd)>1.0E15)){
		SDL_snprintf(strOut,32," "); //roughly stable
		return;
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
		probDcyModeInd = nd->levels[lvlInd].firstDecMode + (uint32_t)maxProbInd;
		getDecayModeStr(strOut,nd,probDcyModeInd);
	}else if(nd->levels[lvlInd].numTran >0){
		SDL_snprintf(strOut,32,"IT > 0%%%%");
	}else{
		SDL_snprintf(strOut,32," "); //couldn't find probable decay mode
		return;
	}
}

void getRxnStr(char strOut[32], const ndata *restrict nd, const uint32_t rxnInd){
	SDL_snprintf(strOut,nd->rxn[rxnInd].rxnStrLen,"%s",&nd->ensdfStrBuf[nd->rxn[rxnInd].rxnStrBufStartPos]);
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

	SDL_strlcpy(strOut,"",32); //clear the string

	for(int i=0;i<nd->levels[lvlInd].numSpinParVals;i++){
		
		//SDL_Log("Spin: %i, parity: %i, tentative: %u\n\n",nd->levels[lvlInd].spval[i].spinVal,nd->levels[lvlInd].spval[i].parVal,tentative);
		
		const uint8_t tentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i].format >> 10U) & 15U);
		uint8_t prevTentative = 0;
		if(i>0){
			prevTentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i-1].format >> 10U) & 15U);
		}
		uint8_t nextTentative = 0;
		if(i<nd->levels[lvlInd].numSpinParVals-1){
			nextTentative = (uint8_t)((uint16_t)(nd->levels[lvlInd].spval[i+1].format >> 10U) & 15U);
		}

		if(tentative == TENTATIVESP_RANGE){
			SDL_strlcat(strOut,"to",32);
		}else if(tentative == TENTATIVESP_HIGHJ){
			SDL_strlcat(strOut,"High J",32);
		}else if(tentative == TENTATIVESP_LOWJ){
			SDL_strlcat(strOut,"Low J",32);
		}else{

			const uint8_t spinIsVar = (uint8_t)(nd->levels[lvlInd].spval[i].format & 1U);
			const uint8_t spinVarInd = (uint8_t)((nd->levels[lvlInd].spval[i].format >> 5U) & 15U);
			const uint8_t spinValType = (uint8_t)((nd->levels[lvlInd].spval[i].format >> 1U) & 15U);
			
			if((tentative == TENTATIVESP_SPINANDPARITY)||(tentative == TENTATIVESP_SPINONLY)){
				if((i==0)||((i>0)&&((prevTentative != TENTATIVESP_SPINANDPARITY)&&(prevTentative != TENTATIVESP_SPINONLY)))){
					if((i>0)&&(prevTentative == TENTATIVESP_RANGE)){
						//previous spin parity value specified a range
						SDL_strlcat(strOut," ",32);
					}else{
						SDL_strlcat(strOut,"(",32);
					}
				}
			}else if((tentative == TENTATIVESP_ASSUMED)||(tentative == TENTATIVESP_ASSUMEDSPINONLY)){
				if((i==0)||((i>0)&&((prevTentative != TENTATIVESP_ASSUMED)&&(prevTentative != TENTATIVESP_ASSUMEDSPINONLY)))){
					if((i>0)&&(prevTentative == TENTATIVESP_RANGE)){
						//previous spin parity value specified a range
						SDL_strlcat(strOut," ",32);
					}else{
						SDL_strlcat(strOut,"[",32);
					}
				}
			}

			if(spinIsVar){
				if(spinValType != VALUETYPE_NUMBER){
					SDL_strlcat(strOut,getValueTypeShortStr(spinValType),32);
				}
				if(nd->levels[lvlInd].spval[i].spinVal == 0){
					//variable only
					if(spinVarInd == 0){
						SDL_strlcat(strOut,"J",32);
					}else{
						sprintf(val,"J%u",spinVarInd);
						SDL_strlcat(strOut,val,32);
					}
				}else{
					//variable + offset
					//SDL_Log("spin variable index: %u\n",spinVarInd);
					if(spinVarInd == 0){
						SDL_strlcat(strOut,"J+",32);
					}else{
						sprintf(val,"J%u+",spinVarInd);
						SDL_strlcat(strOut,val,32);
					}
				}
			}

			if((!spinIsVar)||(nd->levels[lvlInd].spval[i].spinVal > 0)){
				if(spinValType != VALUETYPE_NUMBER){
					SDL_strlcat(strOut,getValueTypeShortStr(spinValType),32);
				}
				if(nd->levels[lvlInd].spval[i].spinVal < 255){
					if((nd->levels[lvlInd].format & 1U) == 1){
						sprintf(val,"%i/2",nd->levels[lvlInd].spval[i].spinVal);
					}else{
						sprintf(val,"%i",nd->levels[lvlInd].spval[i].spinVal);
					}
					SDL_strlcat(strOut,val,32);
				}
			}
			if((tentative != TENTATIVESP_SPINONLY)&&(tentative != TENTATIVESP_PARITYONLY)){
				if(nd->levels[lvlInd].spval[i].parVal == -1){
					SDL_strlcat(strOut,"-",32);
				}else if(nd->levels[lvlInd].spval[i].parVal == 1){
					SDL_strlcat(strOut,"+",32);
				}
			}
			if((tentative == TENTATIVESP_SPINANDPARITY)||(tentative == TENTATIVESP_SPINONLY)){
				if(i==nd->levels[lvlInd].numSpinParVals-1){
					SDL_strlcat(strOut,")",32);
					if(tentative == TENTATIVESP_SPINONLY){
						if(nd->levels[lvlInd].spval[i].parVal == -1){
							SDL_strlcat(strOut,"-",32);
						}else if(nd->levels[lvlInd].spval[i].parVal == 1){
							SDL_strlcat(strOut,"+",32);
						}
					}
				}else if(i<nd->levels[lvlInd].numSpinParVals-1){
					if(nextTentative != TENTATIVESP_RANGE){
						if(nextTentative != TENTATIVESP_SPINANDPARITY){
							if(nextTentative != TENTATIVESP_SPINONLY){
								SDL_strlcat(strOut,")",32);
								if(tentative == TENTATIVESP_SPINONLY){
									if(nd->levels[lvlInd].spval[i].parVal == -1){
										SDL_strlcat(strOut,"-",32);
									}else if(nd->levels[lvlInd].spval[i].parVal == 1){
										SDL_strlcat(strOut,"+",32);
									}
								}
							}
						}
					}
				}
			}else if(tentative == TENTATIVESP_PARITYONLY){
				if(nd->levels[lvlInd].spval[i].parVal == -1){
					SDL_strlcat(strOut,"(-)",32);
				}else if(nd->levels[lvlInd].spval[i].parVal == 1){
					SDL_strlcat(strOut,"(+)",32);
				}
			}else if((tentative == TENTATIVESP_ASSUMED)||(tentative == TENTATIVESP_ASSUMEDSPINONLY)){
				if(i==nd->levels[lvlInd].numSpinParVals-1){
					SDL_strlcat(strOut,"]",32);
					if(tentative == TENTATIVESP_ASSUMEDSPINONLY){
						if(nd->levels[lvlInd].spval[i].parVal == -1){
							SDL_strlcat(strOut,"-",32);
						}else if(nd->levels[lvlInd].spval[i].parVal == 1){
							SDL_strlcat(strOut,"+",32);
						}
					}
				}else if(i<nd->levels[lvlInd].numSpinParVals-1){
					if(nextTentative != TENTATIVESP_RANGE){
						if(nextTentative != TENTATIVESP_ASSUMED){
							if(nextTentative != TENTATIVESP_ASSUMEDSPINONLY){
								SDL_strlcat(strOut,"]",32);
								if(tentative == TENTATIVESP_ASSUMEDSPINONLY){
									if(nd->levels[lvlInd].spval[i].parVal == -1){
										SDL_strlcat(strOut,"-",32);
									}else if(nd->levels[lvlInd].spval[i].parVal == 1){
										SDL_strlcat(strOut,"+",32);
									}
								}
							}
						}
					}
				}
			}
			if(i!=nd->levels[lvlInd].numSpinParVals-1){
				if(nextTentative == TENTATIVESP_RANGE){
					//next spin parity value specifies a range
					SDL_strlcat(strOut," ",32);
				}else{
					SDL_strlcat(strOut,",",32);
				}
			}
		}
		
	}
}

double getRawDblValFromDB(const dblValWithErr *restrict valStruct){
	double val = valStruct->val;
	if(((valStruct->format >> 4U) & 1U) != 0){
		//value in exponent form
		val = val * pow(10.0,(double)(valStruct->exponent));
	}
	return val;
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
		err = err/(pow(10.0,numSigFigs));
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

int8_t getMostProbableParity(const ndata *restrict nd, const uint32_t lvlInd){
	if(nd->levels[lvlInd].numSpinParVals == 1){
		return nd->levels[lvlInd].spval[0].parVal;
	}else if(nd->levels[lvlInd].numSpinParVals > 0){
		int8_t par = nd->levels[lvlInd].spval[0].parVal;
		for(uint8_t i=1;i<nd->levels[lvlInd].numSpinParVals;i++){
			if(nd->levels[lvlInd].spval[i].parVal != par){
				//unknown parity
				return 0;
			}
		}
		return par;
	}
	return 0; //unknown parity
}

double getMostProbableSpin(const ndata *restrict nd, const uint32_t lvlInd){
	if(nd->levels[lvlInd].numSpinParVals == 1){
		if(nd->levels[lvlInd].spval[0].spinVal == 255){
			//unknown spin
			return 255.0;
		}
		if(nd->levels[lvlInd].format & 1U){
			//half-integer spin
			return 0.5*(nd->levels[lvlInd].spval[0].spinVal);
		}else{
			return 1.0*(nd->levels[lvlInd].spval[0].spinVal);
		}
	}else if(nd->levels[lvlInd].numSpinParVals > 0){
		//return average of multiple spins
		double avgSpin = 0.0;
		for(uint8_t i=0;i<nd->levels[lvlInd].numSpinParVals;i++){
			if(nd->levels[lvlInd].spval[i].spinVal == 255){
				//unknown spin
				return 255.0;
			}
			if(nd->levels[lvlInd].format & 1U){
				//half-integer spin
				avgSpin += 0.5*(nd->levels[lvlInd].spval[i].spinVal);
			}else{
				avgSpin += 1.0*(nd->levels[lvlInd].spval[i].spinVal);
			}
		}
		return avgSpin/(1.0*nd->levels[lvlInd].numSpinParVals);
	}
	return 255.0; //unknown spin
}

//gets the number of levels which don't have an assigned level energy
uint16_t getNumUnknownLvls(const ndata *restrict nd, const uint16_t nuclInd){
	uint16_t numUnknowns = 0;
	for(uint32_t i=nd->nuclData[nuclInd].firstLevel; i<(uint32_t)(nd->nuclData[nuclInd].firstLevel + nd->nuclData[nuclInd].numLevels); i++){
		uint8_t eValueType = (uint8_t)((nd->levels[i].energy.format >> 5U) & 15U);
		if(eValueType == VALUETYPE_X){
			numUnknowns++;
		}else if(eValueType == VALUETYPE_PLUSX){
			if(nd->levels[i].energy.val <= 0.0f){
				//energy of 0+X
				numUnknowns++;
			}
		}
	}
	return numUnknowns;
}

//get the binding energy per nucleon
double getBEA(const ndata *restrict nd, const uint16_t nuclInd){
	return getRawDblValFromDB(&nd->nuclData[nuclInd].beA);
}

//get the quadrupole deformation parameter (without uncertainty), for even-even nuclei
//need to correct for internal conversion
//can compare values against Pritychenko et al. 2016
double getBeta2(const ndata *restrict nd, const uint16_t nuclInd){
	uint32_t lvlInd = get2PlusLvlInd(nd,nuclInd);
	if(lvlInd != MAXNUMLVLS){
		uint32_t e2TranInd = MAXNUMTRAN;
		for(uint32_t i=nd->levels[lvlInd].firstTran; i<(nd->levels[lvlInd].firstTran + nd->levels[lvlInd].numTran); i++){
			if(fabs(getRawValFromDB(&nd->tran[i].energy) - getRawValFromDB(&nd->levels[lvlInd].energy)) < (0.01*getRawValFromDB(&nd->levels[lvlInd].energy))){
				e2TranInd = i;
				break;
			}
		}
		if(e2TranInd < MAXNUMTRAN){
			double lts = getLevelHalfLifeSeconds(nd,lvlInd)*(1.0 + getRawValFromDB(&nd->tran[e2TranInd].icc))*1.4427; //convert half-life to lifetime
			if(lts > 0.0){
				double levelEMeV = getRawValFromDB(&nd->tran[e2TranInd].energy)/1000.0;
				double fac = 24.0*3.14159/(2*(6.58212E-22)*225.0*0.693);
				fac = fac * (pow((levelEMeV/197.327),2.0*2 + 1.0));
				double b=1.0/(fac*lts); /* lifetime to reduced transition probability (E2: e^2 fm^4) */
				double beta2 = sqrt(5*b)*4.0*3.14159/(3*(nd->nuclData[nuclInd].Z)*1.2*1.2*pow(1.0*(nd->nuclData[nuclInd].N + nd->nuclData[nuclInd].Z),2.0/3.0));
				//SDL_Log("energy: %f, lt: %e, beta2: %f",levelEMeV,lts,beta2);
				return beta2;
			}
		}
		
	}
	return -1.0; //no B(E2), or not even-even
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

uint8_t getLevelMostProbableDcyMode(const ndata *restrict nd, const uint32_t lvlInd){
	
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
		return nd->dcyMode[nd->levels[lvlInd].firstDecMode + (uint32_t)maxProbInd].type;
	}else if(nd->levels[lvlInd].numTran > 0){
		//no decay mode is specified, but there are gammas depopulating the level
		return DECAYMODE_IT;
	}

	return DECAYMODE_ENUM_LENGTH; //no decay mode found
}

uint8_t getNuclLevelMostProbableDcyMode(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel){
	
	uint32_t lvlInd = nd->nuclData[nuclInd].firstLevel + (uint32_t)nuclLevel;
	return getLevelMostProbableDcyMode(nd,lvlInd);
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
		if(state->ds.selectedRxn == 0){
			numLines += getNumDispLinesForLvl(nd,lvlInd);
		}else if(nd->levels[lvlInd].populatingRxns & ((uint64_t)(1) << (state->ds.selectedRxn-1))){
			numLines += getNumDispLinesForLvl(nd,lvlInd);
		}
	}
	return numLines;
}
uint16_t getNumDispLinesUpToLvl(const ndata *restrict nd, const app_state *restrict state, const uint16_t nuclLevel){
	uint16_t numLines = 0;
	for(uint32_t i = nd->nuclData[state->chartSelectedNucl].firstLevel; i<(nd->nuclData[state->chartSelectedNucl].firstLevel + (uint32_t)nuclLevel); i++){
		numLines += getNumDispLinesForLvl(nd,i);
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
  
	//SDL_Log("Changing UI state to %u\n",newState);
	const uint8_t prevMouseover = state->mouseoverElement;
  state->interactableElement = 0;
  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' any buttons
	if(newState != state->uiState){
		//^only update the last UI state if the UI state is actually being changed
		state->lastUIState = state->uiState; //useful to remember old UI states, for modal dialogs
  	state->uiState = newState;
	}
  
	state->interactableElement |= ((uint64_t)(1) << UIELEM_CONTEXT_MENU);
  switch(state->uiState){
		case UISTATE_ABOUT_BOX:
			state->interactableElement |= ((uint64_t)(1) << UIELEM_ABOUT_BOX_CLOSEBUTTON);
			break;
		case UISTATE_PREFS_DIALOG:
			state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG_CLOSEBUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_UISM_SMALL_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_UISM_DEFAULT_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_UISM_LARGE_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_UISM_HUGE_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)((int16_t)UIELEM_PREFS_UISCALE_MENU - (int16_t)UISCALE_ENUM_LENGTH); //select the first menu item
				}
			}else{
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					if((prevMouseover < UIELEM_PREFS_UISCALE_MENU)&&(prevMouseover >= (uint8_t)((int16_t)UIELEM_PREFS_UISCALE_MENU-(int16_t)UISCALE_ENUM_LENGTH))){
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
			state->interactableElement |= ((uint64_t)(1) << UIELEM_MENU_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX_BACKBUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX_RXNBUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX_SCROLLBAR);
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_PREFS_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_SCREENSHOT_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_ABOUT_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PRIMARY_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU - PRIMARY_MENU_NUM_UIELEMENTS); //select the first menu item
				}
			}
			if(state->uiState == UISTATE_FULLLEVELINFOWITHMENU){
				clearSelectionStrs(&state->tss,0); //strings on full level list are no longer selectable
			}else{
				clearSelectionStrs(&state->tss,1); //allow strings on the full info box to be selectable
			}
			break;
		case UISTATE_INFOBOX:
			state->interactableElement |= ((uint64_t)(1) << UIELEM_MENU_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_CHARTVIEW_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_SEARCH_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_INFOBOX);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			if(state->ds.chartZoomToScale > MIN_CHART_ZOOM_SCALE){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_ZOOMOUT_BUTTON);
			}
			if(state->ds.chartZoomToScale < MAX_CHART_ZOOM_SCALE){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_ZOOMIN_BUTTON);
			}
			break;
		case UISTATE_CHARTWITHMENU:
			state->interactableElement |= ((uint64_t)(1) << UIELEM_MENU_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_CHARTVIEW_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_SEARCH_BUTTON);
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_PREFS_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_SCREENSHOT_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PM_ABOUT_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_PRIMARY_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU - PRIMARY_MENU_NUM_UIELEMENTS); //select the first menu item
				}
			}
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_HALFLIFE_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_DECAYMODE_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_2PLUS_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_R42_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_BETA2_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_SPIN_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_PARITY_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_BEA_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_NUMLVLS);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CVM_UNKNOWN_ENERGY_BUTTON);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU);
				if(state->lastInputType != INPUT_TYPE_MOUSE){
					//keyboard/gamepad navigation of the menu
					state->mouseoverElement = (uint8_t)((int16_t)UIELEM_CHARTVIEW_MENU - (int16_t)CHARTVIEW_ENUM_LENGTH); //select the first menu item
				}
			}
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]==0.0f)){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_SEARCH_ENTRYBOX);
				state->interactableElement |= ((uint64_t)(1) << UIELEM_SEARCH_MENU);
			}
			break;
    case UISTATE_CHARTONLY:
    default:
      state->interactableElement |= ((uint64_t)(1) << UIELEM_MENU_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_CHARTVIEW_BUTTON);
			state->interactableElement |= ((uint64_t)(1) << UIELEM_SEARCH_BUTTON);
			if(state->ds.chartZoomToScale > MIN_CHART_ZOOM_SCALE){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_ZOOMOUT_BUTTON);
			}
			if(state->ds.chartZoomToScale < MAX_CHART_ZOOM_SCALE){
				state->interactableElement |= ((uint64_t)(1) << UIELEM_ZOOMIN_BUTTON);
			}
			clearSelectionStrs(&state->tss,0); //strings on the info box are no longer selectable
      break;
  }
	//SDL_Log("Mouseover element: %u\n",state->mouseoverElement);
}

void panChartToPos(const app_data *restrict dat, drawing_state *restrict ds, const uint16_t posN, const uint16_t posZ, const float panTime){
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
	
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON); //width/position depends on selected reaction
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_RXN_MENU); //menu position depends on above button position

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

	updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX);
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON);
	updateSingleUIElemPosition(dat,state,rdat,UIELEM_RXN_MENU);
}

void setSelectedNuclOnLevelList(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t N, const uint16_t Z, const uint8_t updateRxn){
	clearSelectionStrs(&state->tss,0); //strings on previous full level list are no longer selectable
	uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)N,(int16_t)Z);
	//SDL_Log("Selected nucleus: %u\n",state->chartSelectedNucl);
	if(((selNucl < MAXNUMNUCL)&&(selNucl != state->chartSelectedNucl)) || updateRxn){
		state->chartSelectedNucl = selNucl;
		if(updateRxn){
			state->ds.selectedRxn = state->ds.selectedRxn;
		}else{
			state->ds.selectedRxn = 0;
		}
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
		state->ds.mouseOverRxn = 255;
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
		clearSelectionStrs(&state->tss,1); //allow strings on the info box to be selectable
		if(!(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX))){
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_NUCL_INFOBOX);
			changeUIState(dat,state,UISTATE_INFOBOX); //make info box interactable
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_SHOW);
		}
		startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_SHOW);
		if(forcePan==1){
			panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
		}else if(forcePan==2){
			if(state->kbdModVal == KBD_MOD_SHIFT){
				panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME*PAN_SPRINT_MULTIPLIER);
			}else{
				panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME);
			}
		}else{
			//check occlusion by info box
			float xOcclLeft = chartNtoXPx(&state->ds,(float)(N+1));
			float xOcclRight = chartNtoXPx(&state->ds,(float)N);
			float yOcclTop = chartZtoYPx(&state->ds,(float)(Z-1));
			if((xOcclLeft >= state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX])&&(xOcclRight <= (state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]))){
				if(yOcclTop >= state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX]){
					//always pan chart to dodge occlusion
					if(forcePan==2){
						if(state->kbdModVal == KBD_MOD_SHIFT){
							panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME*PAN_SPRINT_MULTIPLIER);
							return;
						}else{
							panChartToPos(dat,&state->ds,N,Z,CHART_KEY_PAN_TIME);
							return;
						}
					}else{
						panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
						return;
					}
				}
			}
			//check if nuclide is at screen edge
			float yOcclBottom = chartZtoYPx(&state->ds,(float)(Z));
			if((xOcclRight < (0.0f + CHART_AXIS_DEPTH))||(xOcclLeft > state->ds.windowXRes)||(yOcclBottom < 0.0f)||(yOcclTop > (state->ds.windowYRes - CHART_AXIS_DEPTH))){
				panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
			}
		}	
	}else{
		if((forcePan==1) && (state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f) && (selNucl < MAXNUMNUCL)){
			//pan chart to nuclide that is clicked
			//SDL_Log("starting pan to: %f %f\n",(double)mouseX,(double)mouseY);
			panChartToPos(dat,&state->ds,N,Z,CHART_DOUBLECLICK_PAN_TIME);
		}else if(forcePan == 0){
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
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
			clearSelectionStrs(&state->tss,1); //selection string positions are changed on scroll
			break;
		default:
			break;
	}
}

void uiElemMouseoverAction(const app_state *restrict state, resource_data *restrict rdat){
	//SDL_Log("Mouseover element %u\n",state->mouseoverElement);
	switch(state->mouseoverElement){
		case UIELEM_SEARCH_ENTRYBOX:
			SDL_SetCursor(rdat->textEntryCursor);
			break;
		default:
			if(state->ds.textDragInProgress == 0){
				SDL_SetCursor(rdat->defaultCursor);
			}else{
				SDL_SetCursor(rdat->textEntryCursor);
			}
			break;
	}
}

//updates the search result UI as results come in
void updateSearchUIState(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  updateSingleUIElemPosition(dat,state,rdat,UIELEM_SEARCH_MENU);
  for(uint8_t i=0; i<state->ss.numResults; i++){
    if(i<MAX_DISP_SEARCH_RESULTS){
      state->interactableElement |= ((uint64_t)(1) << (UIELEM_SEARCH_RESULT+i));
			//SDL_Log("interactable: %u\n",i);
    }
  }
  for(uint8_t i=state->ss.numResults; i<MAX_DISP_SEARCH_RESULTS; i++){
    state->interactableElement = (uint64_t)(state->interactableElement & ~((uint64_t)(1) << (UIELEM_SEARCH_RESULT+i))); //unset
		//SDL_Log("uninteractable: %u\n",i);
  }
}

void contextMenuClickAction(app_data *restrict dat, app_state *restrict state, const uint8_t menuItemInd){
	
	if(menuItemInd >= state->cms.numContextMenuItems){
		SDL_Log("WARNING: conextMenuClickAction - attempt to click on invalid menu item (%u).\n",menuItemInd);
		return;
	}

	startUIAnimation(dat,state,UIANIM_CONTEXT_MENU_HIDE); //menu will be closed after animation finishes
	
	switch(state->cms.contextMenuItems[menuItemInd]){
		case CONTEXTITEM_NUCLNAME:
			; //suppress warning
			getNuclNameStr(state->copiedTxt,&dat->ndat.nuclData[state->cms.selectionInd],255);
			SDL_SetClipboardText(state->copiedTxt);
			//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
			break;
		case CONTEXTITEM_NUCLINFO:
			; //suppress warning
			char tmpStr[32];
			getNuclNameStr(tmpStr,&dat->ndat.nuclData[state->cms.selectionInd],255);
			SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s, %s: %i, %s: %i, %s: ",tmpStr, dat->strings[dat->locStringIDs[LOCSTR_PROTONSDESC]],dat->ndat.nuclData[state->cms.selectionInd].Z,dat->strings[dat->locStringIDs[LOCSTR_NEUTRONSDESC]],dat->ndat.nuclData[state->cms.selectionInd].N,dat->strings[dat->locStringIDs[LOCSTR_JPI]]);
			getSpinParStr(tmpStr,&dat->ndat,dat->ndat.nuclData[state->cms.selectionInd].firstLevel + dat->ndat.nuclData[state->cms.selectionInd].gsLevel);
			if(SDL_strlen(tmpStr) > 0){
				SDL_strlcat(state->copiedTxt,tmpStr,MAX_SELECTABLE_STR_LEN);
			}else{
				SDL_strlcat(state->copiedTxt,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],MAX_SELECTABLE_STR_LEN);
			}
			SDL_SetClipboardText(state->copiedTxt);
			//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
			break;
		case CONTEXTITEM_CHART_PROPERTY:
			; //suppress warning
			const uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[state->cms.selectionInd].firstLevel + dat->ndat.nuclData[state->cms.selectionInd].gsLevel);
			switch(state->chartView){
				case CHARTVIEW_HALFLIFE:
					; //suppress warning
					char tmpHlStr[32];
					getGSHalfLifeStr(tmpHlStr,dat,state->cms.selectionInd,state->ds.useLifetimes);
					if(state->ds.useLifetimes){
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_LIFETIME]],tmpHlStr);
					}else{
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_HALFLIFE]],tmpHlStr);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_DECAYMODE:
					if(dat->ndat.levels[gsLevInd].halfLife.unit == VALUE_UNIT_STABLE){
						//if the nuclide is stable, show the 'STABLE' label
						getGSHalfLifeStr(state->copiedTxt,dat,state->cms.selectionInd,state->ds.useLifetimes);
					}else{
						char tmpDecStr[32];
						state->copiedTxt[0] = '\0'; //empty string to be copied
						for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
							getDecayModeStr(tmpDecStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
							SDL_strlcat(state->copiedTxt,tmpDecStr,MAX_SELECTABLE_STR_LEN);
							if(i<(dat->ndat.levels[gsLevInd].numDecModes - 1)){
								SDL_strlcat(state->copiedTxt,", ",MAX_SELECTABLE_STR_LEN);
							}
						}
					}
					char *cpyStrCpy = findReplaceAllUTF8("%%","%",state->copiedTxt); //you copii?
					SDL_strlcpy(state->copiedTxt,cpyStrCpy,MAX_SELECTABLE_STR_LEN);
					free(cpyStrCpy);
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_2PLUS:
					; //suppress warning
					uint32_t plus2Lvl = get2PlusLvlInd(&dat->ndat,state->cms.selectionInd);
					if(plus2Lvl != MAXNUMLVLS){
						char tmpEnStr[32];
						getLvlEnergyStr(tmpEnStr,&dat->ndat,plus2Lvl,1);
						SDL_strlcat(tmpEnStr," keV",32);
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]],tmpEnStr);
					}else{
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]],dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_R42:
					; //suppress warning
					double r42 = getR42(&dat->ndat,state->cms.selectionInd);
					if(r42 > 0.0){
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"R₄₂: %0.2f",r42);
					}else{
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"R₄₂: %s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_BETA2:
					; //suppress warning
					double beta2 = getBeta2(&dat->ndat,state->cms.selectionInd);
					if(beta2 > 0.0){
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %0.2f",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BETA2]], beta2);
					}else{
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BETA2]], dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_SPIN:
				case CHARTVIEW_PARITY:
					if(getMostProbableSpin(&dat->ndat,gsLevInd) >= 255.0){
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_JPI]],dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
					}else{
						char tmpJPiStr[32];
						getSpinParStr(tmpJPiStr,&dat->ndat,gsLevInd);
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_JPI]],tmpJPiStr);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_BEA:
					; //suppress warning
					double beA = getBEA(&dat->ndat,state->cms.selectionInd);
					if(beA > 0.0){
						char tmpBEAStr[32];
						getMassValStr(tmpBEAStr,dat->ndat.nuclData[state->cms.selectionInd].beA,1);
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s keV",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BEA]],tmpBEAStr);
					}else{
						SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %s",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_BEA]],dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
					}
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_NUMLVLS:
					SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %u",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_NUMLVLS]],dat->ndat.nuclData[state->cms.selectionInd].numLevels);
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				case CHARTVIEW_UNKNOWN_ENERGY:
					; //suppress warning
					const uint16_t numUnknowns = getNumUnknownLvls(&dat->ndat,state->cms.selectionInd);
					SDL_snprintf(state->copiedTxt,MAX_SELECTABLE_STR_LEN,"%s: %u",dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_UNKNOWN_ENERGY]],numUnknowns);
					SDL_SetClipboardText(state->copiedTxt);
					//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
					break;
				default:
					break;
			}
			break;
		case CONTEXTITEM_COPY:
			//copy text
			if(state->tss.selectedStr < 65535){
				//copy selected text to clipboard
				if(state->tss.selStartPos != state->tss.selEndPos){
					if(state->tss.selStartPos > state->tss.selEndPos){
						//swap start and end for the purposes of drawing
						uint8_t tmp = state->tss.selEndPos;
						state->tss.selEndPos = state->tss.selStartPos;
						state->tss.selStartPos = tmp;
					}
					size_t selLen = (size_t)(state->tss.selEndPos - state->tss.selStartPos);
					if(selLen < MAX_SELECTABLE_STR_LEN){
						SDL_strlcpy(state->copiedTxt,&state->tss.selectableStrTxt[state->tss.selectedStr][state->tss.selStartPos],selLen+1);
						char *selSubStrCpy = findReplaceAllUTF8("%%","%",state->copiedTxt);
						SDL_strlcpy(state->copiedTxt,selSubStrCpy,selLen+1);
						free(selSubStrCpy);
					}
					SDL_SetClipboardText(state->copiedTxt);
				}
				//SDL_Log("Copied text to clipboard: %s\n",SDL_GetClipboardText());
			}
			break;
		case CONTEXTITEM_ENUM_LENGTH:
		default:
			break;
	}
}

void searchResultClickAction(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t resultInd){
	
	if(resultInd >= MAX_DISP_SEARCH_RESULTS){
		SDL_Log("WARNING: searchResultClickAction - attempt to click on invalid search result (%u).\n",resultInd);
		return;
	}

	startUIAnimation(dat,state,UIANIM_SEARCH_MENU_HIDE); //menu will be closed after animation finishes
	state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
	changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
	
	uint16_t nuclLevel = 65535U;
	switch(state->ss.results[resultInd].resultType){
		case SEARCHAGENT_NUCLIDE:
			setSelectedNuclOnChartDirect(dat,state,rdat,(uint16_t)(state->ss.results[resultInd].resultVal[0]),1);
			break;
		case SEARCHAGENT_GAMMACASCADE:
		case SEARCHAGENT_EGAMMA:
			setSelectedNuclOnChartDirect(dat,state,rdat,(uint16_t)(state->ss.results[resultInd].resultVal[0]),1);
			uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			//get the level corresponding to the transition
			for(uint16_t i=0; i<dat->ndat.nuclData[state->ss.results[resultInd].resultVal[0]].numLevels; i++){
				uint32_t firstTran = dat->ndat.levels[dat->ndat.nuclData[state->ss.results[resultInd].resultVal[0]].firstLevel + i].firstTran;
				for(uint32_t j=firstTran; j<(firstTran + dat->ndat.levels[dat->ndat.nuclData[state->ss.results[resultInd].resultVal[0]].firstLevel + i].numTran); j++){
					if(j==state->ss.results[resultInd].resultVal[1]){
						nuclLevel = i;
						break;
					}
				}
				if(nuclLevel != 65535U){
					break;
				}
			}
			state->ds.nuclFullInfoScrollY = getNumDispLinesUpToLvl(&dat->ndat,state,nuclLevel); //scroll to the level of interest
			break;
		case SEARCHAGENT_ELEVEL:
			setSelectedNuclOnChartDirect(dat,state,rdat,(uint16_t)(state->ss.results[resultInd].resultVal[0]),1);
			uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			nuclLevel = (uint16_t)(state->ss.results[resultInd].resultVal[1] - dat->ndat.nuclData[state->ss.results[resultInd].resultVal[0]].firstLevel);
			state->ds.nuclFullInfoScrollY = getNumDispLinesUpToLvl(&dat->ndat,state,nuclLevel); //scroll to the level of interest
			break;
		default:
			break;
	}
}

//take action after clicking a button or other UI element
//doubleClick: 0=left click only, 1=double click (left), 2=right click
void uiElemClickAction(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t doubleClick, const uint8_t uiElemID){

	//SDL_Log("Clicked UI element %u\n",uiElemID);
  state->clickedUIElem = uiElemID;

	//handle extraneous opened menus
	if((uiElemID != UIELEM_CONTEXT_MENU) && (state->cms.numContextMenuItems > 0)){
		startUIAnimation(dat,state,UIANIM_CONTEXT_MENU_HIDE); //remove any opened context menus
	}
	if((uiElemID != UIELEM_MENU_BUTTON)&&(uiElemID != UIELEM_PRIMARY_MENU)&&(uiElemID != UIELEM_PM_PREFS_BUTTON)&&(uiElemID != UIELEM_PM_SCREENSHOT_BUTTON)&&(uiElemID != UIELEM_PM_ABOUT_BUTTON)){
		if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_CHARTVIEW_BUTTON)&&(uiElemID != UIELEM_CHARTVIEW_MENU)&&(uiElemID != UIELEM_CVM_HALFLIFE_BUTTON)&&(uiElemID != UIELEM_CVM_DECAYMODE_BUTTON)&&(uiElemID != UIELEM_CVM_2PLUS_BUTTON)&&(uiElemID != UIELEM_CVM_R42_BUTTON)&&(uiElemID != UIELEM_CVM_BETA2_BUTTON)&&(uiElemID != UIELEM_CVM_SPIN_BUTTON)&&(uiElemID != UIELEM_CVM_PARITY_BUTTON)&&(uiElemID != UIELEM_CVM_BEA_BUTTON)&&(uiElemID != UIELEM_CVM_NUMLVLS)&&(uiElemID != UIELEM_CVM_UNKNOWN_ENERGY_BUTTON)){
		if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_SEARCH_BUTTON)&&(uiElemID != UIELEM_SEARCH_MENU)&&(uiElemID != UIELEM_SEARCH_ENTRYBOX)){
		if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_SEARCH_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			//stop search input
			if(SDL_TextInputActive(rdat->window)){
				memset(state->ss.searchString,0,sizeof(state->ss.searchString));
				state->searchCursorPos = 0;
				state->searchSelectionLen = 0;
				state->ds.searchEntryDispStartChar = 0;
				state->ds.searchEntryDispNumChars = 65535U; //default value specifying no text has been input yet
				SDL_StopTextInput(rdat->window);
			}
		}
	}
	if((uiElemID != UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN)&&(uiElemID != UIELEM_PREFS_UISCALE_MENU)&&(uiElemID != UIELEM_UISM_SMALL_BUTTON)&&(uiElemID != UIELEM_UISM_DEFAULT_BUTTON)&&(uiElemID != UIELEM_UISM_LARGE_BUTTON)&&(uiElemID != UIELEM_UISM_HUGE_BUTTON)){
		if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_NUCL_FULLINFOBOX_RXNBUTTON)&&(uiElemID != UIELEM_RXN_MENU)){
		if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_RXN_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_HIDE]==0.0f)){
			startUIAnimation(dat,state,UIANIM_RXN_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
		}
	}
	if((uiElemID != UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)){
		if((state->lastOpenedMenu == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)&&(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
			state->uiState = UISTATE_FULLLEVELINFO; //exit menu control
			state->mouseoverElement = UIELEM_ENUM_LENGTH; //'unclick' all buttons
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' all buttons
		}
	}

	//take action from click
  switch(uiElemID){
    case UIELEM_MENU_BUTTON:
      if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f){
				state->ds.shownElements |= ((uint64_t)(1) << UIELEM_PRIMARY_MENU);
				state->lastOpenedMenu = UIELEM_PRIMARY_MENU;
				startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_SHOW);
				if(state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_FULLINFOBOX)){
					changeUIState(dat,state,UISTATE_FULLLEVELINFOWITHMENU);
				}else{
					changeUIState(dat,state,UISTATE_CHARTWITHMENU);
				}
				state->clickedUIElem = UIELEM_MENU_BUTTON;
      }
      break;
		case UIELEM_CHARTVIEW_BUTTON:
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]==0.0f){
				state->ds.shownElements |= ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU);
				state->lastOpenedMenu = UIELEM_CHARTVIEW_MENU;
				startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_SHOW);
				changeUIState(dat,state,UISTATE_CHARTWITHMENU);
				state->clickedUIElem = UIELEM_CHARTVIEW_BUTTON;
      }
			break;
		case UIELEM_SEARCH_BUTTON:
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_SEARCH_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
				//stop search input
				if(SDL_TextInputActive(rdat->window)){
					memset(state->ss.searchString,0,sizeof(state->ss.searchString));
					state->searchCursorPos = 0;
					state->searchSelectionLen = 0;
					state->ds.searchEntryDispStartChar = 0;
					state->ds.searchEntryDispNumChars = 65535U; //default value specifying no text has been input yet
					SDL_StopTextInput(rdat->window);
				}
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]==0.0f){
				state->ds.shownElements |= ((uint64_t)(1) << UIELEM_SEARCH_MENU);
				state->lastOpenedMenu = UIELEM_SEARCH_MENU;
				startUIAnimation(dat,state,UIANIM_SEARCH_MENU_SHOW);
				changeUIState(dat,state,UISTATE_CHARTWITHMENU);
				state->clickedUIElem = UIELEM_SEARCH_BUTTON;
				state->mouseholdElement = UIELEM_ENUM_LENGTH; //remove any previous selection highlight (from previous searches)
				//start search input
				memset(state->ss.searchString,0,sizeof(state->ss.searchString));
				state->ss.numResults = 0;
				state->searchCursorPos = 0;
				state->searchSelectionLen = 0;
				state->ds.searchEntryDispStartChar = 0;
				state->ds.searchEntryDispNumChars = 65535U; //default value specifying no text has been input yet
				SDL_StartTextInput(rdat->window);
      }
			break;
		case UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN:
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]==0.0f){
				state->ds.shownElements |= ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU);
				startUIAnimation(dat,state,UIANIM_UISCALE_MENU_SHOW);
				changeUIState(dat,state,state->uiState);
				state->clickedUIElem = UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN;
      }
			break;
		case UIELEM_ABOUT_BOX_CLOSEBUTTON:
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
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
				startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
			}
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
			//SDL_Log("selected: %u\n",state->chartSelectedNucl);
			if(state->chartSelectedNucl < MAXNUMNUCL){
				state->ds.nuclFullInfoScrollY = 0.0f;
				state->ds.selectedRxn = 0;
				state->lastOpenedMenu = UIELEM_PRIMARY_MENU;
				startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_EXPAND);
				setFullLevelInfoDimensions(dat,state,rdat,state->chartSelectedNucl);
			}
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_TXTFADEOUT); //menu will be closed after animation finishes
			break;
		case UIELEM_NUCL_FULLINFOBOX_SCROLLBAR:
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the scrollbar when the mouse is released
			break;
		case UIELEM_NUCL_FULLINFOBOX_RXNBUTTON:
			if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_RXN_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_HIDE]==0.0f)){
				startUIAnimation(dat,state,UIANIM_RXN_MENU_HIDE); //menu will be closed after animation finishes
        state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
      }else if(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_SHOW]==0.0f){
				state->ds.shownElements |= ((uint64_t)(1) << UIELEM_RXN_MENU);
				state->lastOpenedMenu = UIELEM_RXN_MENU;
				state->ds.mouseOverRxn = 255;
				state->ds.mouseHoldRxn = 255;
				startUIAnimation(dat,state,UIANIM_RXN_MENU_SHOW);
				changeUIState(dat,state,UISTATE_FULLLEVELINFOWITHMENU);
				state->clickedUIElem = UIELEM_NUCL_FULLINFOBOX_RXNBUTTON;
      }
			break;
		case UIELEM_PM_PREFS_BUTTON:
			//SDL_Log("Clicked prefs button.\n");
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
      state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_PREFS_DIALOG);
			startUIAnimation(dat,state,UIANIM_MODAL_BOX_SHOW);
			changeUIState(dat,state,UISTATE_PREFS_DIALOG);
			break;
		case UIELEM_PM_SCREENSHOT_BUTTON:
			//SDL_Log("Clicked screenshot button.\n");
			//flag that a screenshot is to be taken at the end of the frame
			//this will be used by the GUI drawing code to omit UI elements from the screenshot
			rdat->ssdat.takingScreenshot = 1;
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
      state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			break;
		case UIELEM_PM_ABOUT_BUTTON:
			//SDL_Log("Clicked about button.\n");
			startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
      state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->ds.shownElements |= ((uint64_t)(1) << UIELEM_ABOUT_BOX);
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
		case UIELEM_CVM_BETA2_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_BETA2;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_SPIN_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_SPIN;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_PARITY_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_PARITY;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_BEA_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_BEA;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_NUMLVLS:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_NUMLVLS;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_CVM_UNKNOWN_ENERGY_BUTTON:
			startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
			state->chartView = CHARTVIEW_UNKNOWN_ENERGY;
			changeUIState(dat,state,UISTATE_CHARTONLY); //prevents mouseover from still highlighting buttons while the menu closes
			break;
		case UIELEM_SEARCH_ENTRYBOX:
			if((state->ds.searchEntryDispNumChars > 0)&&(state->ds.searchEntryDispNumChars < 65535U)){
				//reposition the cursor using the mouse
				float relXPos = ((state->mouseXPx-state->ds.uiElemPosX[UIELEM_SEARCH_ENTRYBOX])/state->ds.uiUserScale)-33.0f;
				if(relXPos > 0.0f){
					uint16_t mousePosChar = getNumTextCharsUnderWidth(rdat,(uint16_t)relXPos,state->ss.searchString,state->ds.searchEntryDispStartChar,FONTSIZE_NORMAL);
					state->searchCursorPos = state->ds.searchEntryDispStartChar + mousePosChar;
					//SDL_Log("Repositioned cursor to position %u (rel pos %f).\n",state->searchCursorPos,(double)relXPos);
				}
			}else{
				state->ds.searchEntryDispNumChars = 0;
				state->searchCursorPos = 0;
			}
			state->clickedUIElem = UIELEM_SEARCH_BUTTON; //keep the menu button selected
			break;
		case UIELEM_SEARCH_RESULT:
			searchResultClickAction(dat,state,rdat,0);
			break;
		case UIELEM_SEARCH_RESULT_2:
			searchResultClickAction(dat,state,rdat,1);
			break;
		case UIELEM_SEARCH_RESULT_3:
			searchResultClickAction(dat,state,rdat,2);
			break;
		case UIELEM_SEARCH_RESULT_4:
			searchResultClickAction(dat,state,rdat,3);
			break;
		case UIELEM_PRIMARY_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_MENU_BUTTON;
			break;
		case UIELEM_CHARTVIEW_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_CHARTVIEW_BUTTON;
			break;
		case UIELEM_SEARCH_MENU:
			//clicked on menu background, do nothing except keep the menu button selected
			state->clickedUIElem = UIELEM_SEARCH_BUTTON;
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
		case UIELEM_ZOOMIN_BUTTON:
			//zoom in
			state->ds.chartZoomStartScale = state->ds.chartZoomScale;
			if(state->ds.chartZoomToScale <= 0.55f){
				state->ds.chartZoomToScale = 2.0f; //UX: zoom in more than usual, so that the initial interaction is more responsive
			}else{
				state->ds.chartZoomToScale += state->ds.chartZoomToScale*1.0f;
			}
			if(state->ds.chartZoomToScale > MAX_CHART_ZOOM_SCALE){
				state->ds.chartZoomToScale = MAX_CHART_ZOOM_SCALE;
			}
			if(state->ds.zoomInProgress == 0){
				//zoom to center of screen
				state->ds.chartZoomStartMouseX = state->ds.chartPosX;
				if(state->chartSelectedNucl != MAXNUMNUCL){
					state->ds.chartZoomStartMouseY = state->ds.chartPosY + (16.0f/state->ds.chartZoomScale); //corrected for position of selected nuclide
				}else{
					state->ds.chartZoomStartMouseY = state->ds.chartPosY; //centred on screen
				}
				if(state->ds.chartZoomStartMouseX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
					state->ds.chartZoomStartMouseX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
				}else if(state->ds.chartZoomStartMouseX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
					state->ds.chartZoomStartMouseX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
				}
				if(state->ds.chartZoomStartMouseY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
					state->ds.chartZoomStartMouseY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
				}else if(state->ds.chartZoomStartMouseY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
					state->ds.chartZoomStartMouseY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
				}
			}
			state->ds.chartZoomStartMouseXFrac = (state->ds.chartZoomStartMouseX - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
			state->ds.chartZoomToX = state->ds.chartZoomStartMouseX - getChartWidthNAfterZoom(&state->ds)*state->ds.chartZoomStartMouseXFrac + getChartWidthNAfterZoom(&state->ds)*0.5f;
			state->ds.chartZoomStartMouseYFrac = (state->ds.chartZoomStartMouseY - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
			state->ds.chartZoomToY = state->ds.chartZoomStartMouseY - getChartHeightZAfterZoom(&state->ds)*state->ds.chartZoomStartMouseYFrac + getChartHeightZAfterZoom(&state->ds)*0.5f;
			if(state->ds.chartZoomToX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
				state->ds.chartZoomToX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
			}else if(state->ds.chartZoomToX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
				state->ds.chartZoomToX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
			}
			if(state->ds.chartZoomToY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
				state->ds.chartZoomToY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
			}else if(state->ds.chartZoomToY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
				state->ds.chartZoomToY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
			}
			state->ds.timeSinceZoomStart = 0.0f;
			state->ds.zoomInProgress = 1;
			state->ds.zoomFinished = 0;
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the zoom button
			break;
		case UIELEM_ZOOMOUT_BUTTON:
			//zoom out
			state->ds.chartZoomStartScale = state->ds.chartZoomScale;
			if(state->ds.chartZoomToScale <= 2.05f){
				state->ds.chartZoomToScale = 0.5f;
			}else{
				state->ds.chartZoomToScale -= state->ds.chartZoomToScale*0.5f;
			}
			if(state->ds.chartZoomToScale < MIN_CHART_ZOOM_SCALE){
				state->ds.chartZoomToScale = MIN_CHART_ZOOM_SCALE;
			}
			if(state->ds.zoomInProgress == 0){
				//zoom to center of screen
				state->ds.chartZoomStartMouseX = state->ds.chartPosX;
				if(state->chartSelectedNucl != MAXNUMNUCL){
					state->ds.chartZoomStartMouseY = state->ds.chartPosY + (16.0f/state->ds.chartZoomScale); //corrected for position of selected nuclide
				}else{
					state->ds.chartZoomStartMouseY = state->ds.chartPosY; //centred on screen
				}
				if(state->ds.chartZoomStartMouseX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
					state->ds.chartZoomStartMouseX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
				}else if(state->ds.chartZoomStartMouseX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
					state->ds.chartZoomStartMouseX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
				}
				if(state->ds.chartZoomStartMouseY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
					state->ds.chartZoomStartMouseY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
				}else if(state->ds.chartZoomStartMouseY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
					state->ds.chartZoomStartMouseY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
				}
			}
			state->ds.chartZoomStartMouseXFrac = (state->ds.chartZoomStartMouseX - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
			state->ds.chartZoomToX = state->ds.chartZoomStartMouseX - getChartWidthNAfterZoom(&state->ds)*state->ds.chartZoomStartMouseXFrac + getChartWidthNAfterZoom(&state->ds)*0.5f;
			state->ds.chartZoomStartMouseYFrac = (state->ds.chartZoomStartMouseY - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
			state->ds.chartZoomToY = state->ds.chartZoomStartMouseY - getChartHeightZAfterZoom(&state->ds)*state->ds.chartZoomStartMouseYFrac + getChartHeightZAfterZoom(&state->ds)*0.5f;
			if(state->ds.chartZoomToX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
				state->ds.chartZoomToX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
			}else if(state->ds.chartZoomToX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
				state->ds.chartZoomToX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
			}
			if(state->ds.chartZoomToY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
				state->ds.chartZoomToY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
			}else if(state->ds.chartZoomToY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
				state->ds.chartZoomToY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
			}
			state->ds.timeSinceZoomStart = 0.0f;
			state->ds.zoomInProgress = 1;
			state->ds.zoomFinished = 0;
			state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the zoom button
			break;
		case UIELEM_CONTEXT_MENU:
			break; //do nothing
		case UIELEM_ENUM_LENGTH:
    default:
			//clicked outside of a button or UI element
      if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
				if(((state->ds.shownElements) == ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES))||((state->ds.shownElements >> (UIELEM_CHARTOFNUCLIDES+1)) == ((uint64_t)(1) << (UIELEM_NUCL_INFOBOX-1)))){
					//only the chart of nuclides and/or info box are open
					//clicked on the chart view
					if((state->mouseXPx > state->ds.uiElemPosX[UIELEM_ZOOMOUT_BUTTON])&&(state->mouseYPx > state->ds.uiElemPosY[UIELEM_ZOOMOUT_BUTTON])){
						//clicked on zoom buttons, don't select the chart behind them
						return;
					}else if(state->mouseXPx < CHART_AXIS_DEPTH*state->ds.uiUserScale){
						//clicked on the y axis, don't select the chart behind it
						return;
					}else if(state->mouseYPx > (state->ds.windowYRes-CHART_AXIS_DEPTH*state->ds.uiUserScale)){
						//clicked on the x axis, don't select the chart behind it
						return;
					}
					uint16_t mouseX = (uint16_t)mouseXPxToN(&state->ds,state->mouseXPx);
    			uint16_t mouseY = (uint16_t)SDL_ceilf(mouseYPxToZ(&state->ds,state->mouseYPx));
					if(doubleClick <= 1){
						//select nucleus
						setSelectedNuclOnChart(dat,state,rdat,mouseX,mouseY,(doubleClick == 1)? 1 : 0);
					}else if(doubleClick == 2){
						//right click on chart
						uint16_t rightClickNucl = getNuclInd(&dat->ndat,(int16_t)mouseX,(int16_t)mouseY);
						if(rightClickNucl < MAXNUMNUCL){
							setupNuclideContextMenu(dat,state,rdat,rightClickNucl);
						}
					}
				}else{
					//clicked out of a menu
					//handle individual menu closing animations
					if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_PRIMARY_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_CHARTVIEW_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_UISCALE_MENU_HIDE); //menu will be closed after animation finishes
						state->clickedUIElem = UIELEM_ENUM_LENGTH; //'unclick' the menu button
					}
					if((state->ds.shownElements & ((uint64_t)(1) << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
						startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
						startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
					}
					if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX)){
						state->ds.shownElements |= ((uint64_t)(1) << UIELEM_CHARTOFNUCLIDES);
					}
				}
      }
      break;
  }
}


//get the number of characters in a string which can be drawn below a certain width,
//starting at txtStartChar
uint16_t getNumTextCharsUnderWidth(resource_data *restrict rdat, const uint16_t widthPx, const char *text, const uint16_t txtStartChar, const uint8_t fontSizeInd){
	
	int textLen = (int)SDL_strlen(text);
	if((textLen < 1)||(txtStartChar > textLen)){
		return 0;
	}

	uint16_t utf8StartChar = txtStartChar;
	while((charIsAscii(text[utf8StartChar])) == 0){
		if(utf8StartChar > 0){
			utf8StartChar--;
		}else{
			break;
		}
	}

	//deal with '%%' sequences in some snprintf'd strings where the '% character needs to be displayed
	/*if((utf8StartChar > 0)&&(text[utf8StartChar] == '%')&&(text[utf8StartChar-1] == '%')){
		utf8StartChar--;
	}*/

	uint16_t txtDrawLen = 0;
	char tmpTxt[256];
	float lastTxtWidth = -1.0f;
	uint16_t lastutf8Len = 0;
	for(uint16_t i=(uint16_t)textLen; i>0; i--){
		memcpy(tmpTxt,text+utf8StartChar,sizeof(char)*(i-utf8StartChar));

		//figure out where to terminate the string, without lopping off partial UTF-8 characters
		uint16_t utf8Len = (uint16_t)(i-utf8StartChar);
		if((utf8Len > 0) && (!(charIsAscii(tmpTxt[utf8Len-1])))){
			while(utf8Len > 0){
				if(charIsAscii(tmpTxt[utf8Len-1])){
					break;
				}
				utf8Len--;
			}
		}
		tmpTxt[utf8Len] = '\0'; //null terminate string
		
		float currentTxtWidth = getTextWidthScaleIndependent(rdat,fontSizeInd,tmpTxt);
		if(lastTxtWidth > 0.0f){
			//make the selection boundaries for characters halfway across the characters
			//helps with less precise mouse selections
			if((currentTxtWidth + 0.5f*(lastTxtWidth-currentTxtWidth)) <= widthPx){
				txtDrawLen = (uint16_t)(lastutf8Len);
				break;
			}else if(currentTxtWidth <= widthPx){
				txtDrawLen = (uint16_t)(utf8Len);
				break;
			}
		}else if(currentTxtWidth <= widthPx){
			txtDrawLen = (uint16_t)(utf8Len);
			break;
		}
		lastTxtWidth = currentTxtWidth;
		lastutf8Len = utf8Len;
	}
	return txtDrawLen;
}

uint8_t getRxnMenuNumRxnsPerColumn(const app_data *restrict dat, const app_state *restrict state){
	return (uint8_t)(SDL_ceilf((dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1.0f)/(state->ds.rxnMenuColumns*1.0f)));
}

SDL_FRect getRxnMenuButtonRect(const drawing_state *restrict ds, const uint8_t numRxnPerCol, const uint8_t menuItem){
	SDL_FRect rect;
	rect.x = (float)ds->uiElemPosX[UIELEM_RXN_MENU] + (float)(2.0f*PANEL_EDGE_SIZE + RXN_MENU_COLUMN_WIDTH*((menuItem)/numRxnPerCol))*ds->uiUserScale;
	rect.y = (float)ds->uiElemPosY[UIELEM_RXN_MENU] + (PANEL_EDGE_SIZE + 0.5f*UI_PADDING_SIZE + ((float)((menuItem)%numRxnPerCol) + 0.1f)*RXN_MENU_ITEM_SPACING)*ds->uiUserScale;
	rect.w = RXN_MENU_COLUMN_WIDTH*ds->uiUserScale;
	rect.h = RXN_MENU_ITEM_SPACING*ds->uiUserScale;
	return rect;
}

SDL_FRect getContextMenuButtonRect(const app_state *restrict state, const uint8_t menuItem){
	SDL_FRect rect;
	rect.x = (float)state->ds.uiElemPosX[UIELEM_CONTEXT_MENU] + (2.0f*PANEL_EDGE_SIZE*state->ds.uiUserScale);
	rect.y = (float)state->ds.uiElemPosY[UIELEM_CONTEXT_MENU] + (2.0f*PANEL_EDGE_SIZE + menuItem*CONTEXT_MENU_ITEM_SPACING)*state->ds.uiUserScale;
	rect.w = (CONTEXT_MENU_WIDTH - (4.0f*PANEL_EDGE_SIZE))*state->ds.uiUserScale;
	rect.h = CONTEXT_MENU_ITEM_SPACING*state->ds.uiUserScale;
	if(state->cms.useHeaderText){
		rect.y += (float)(CONTEXT_MENU_HEADER_HEIGHT*state->ds.uiUserScale);
	}
	return rect;
}

SDL_FRect getTextSelRect(const text_selection_state *restrict tss, resource_data *restrict rdat){
	SDL_FRect rect = {-10.0f, -10.0f, 0.0f, 0.0f};
	if(tss->selectedStr < tss->numSelStrs){

		uint8_t charIndStart = tss->selStartPos;
		uint8_t charIndEnd = tss->selEndPos;
		if(charIndStart == charIndEnd){
			return rect; //width of selection is 0
		}else if(charIndStart > charIndEnd){
			//swap start and end for the purposes of drawing
			uint8_t tmp = charIndEnd;
			charIndEnd = charIndStart;
			charIndStart = tmp;
		}
		uint8_t selLen = (uint8_t)(charIndEnd - charIndStart);
		char selSubStr[MAX_SELECTABLE_STR_LEN], selPreStr[MAX_SELECTABLE_STR_LEN];
		SDL_strlcpy(selSubStr,&tss->selectableStrTxt[tss->selectedStr][charIndStart],selLen+1);
		//deal with '%%' sequences in some snprintf'd strings where the '% character needs to be displayed
		if((charIndStart > 0)&&(selSubStr[0] == '%')&&(selSubStr[1] != '%')){
			SDL_strlcpy(selSubStr,&tss->selectableStrTxt[tss->selectedStr][charIndStart-1],selLen+2);
		}
		SDL_strlcpy(selPreStr,tss->selectableStrTxt[tss->selectedStr],charIndStart+1);

		rect = tss->selectableStrRect[tss->selectedStr];
		rect.x = rect.x + getTextWidth(rdat,tss->selectableStrFontSize[tss->selectedStr],selPreStr)/rdat->uiDPIScale;
		rect.w = getTextWidth(rdat,tss->selectableStrFontSize[tss->selectedStr],selSubStr)/rdat->uiDPIScale;
	}
	return rect;
}


//updates UI element (buttons etc.) positions, based on the screen resolution and other factors
//positioning constants are defined in gui_constants.h
void updateSingleUIElemPosition(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t uiElemInd){
	switch(uiElemInd){
		case UIELEM_MENU_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((MENU_BUTTON_WIDTH+MENU_BUTTON_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(MENU_BUTTON_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(MENU_BUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[uiElemInd] = (uint16_t)((MENU_BUTTON_POS_XR+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtMinusY[uiElemInd] = (uint16_t)((MENU_BUTTON_POS_Y+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			//SDL_Log("res: [%u %u]\nx: %u, y: %u, w: %u, h: %u\n",state->ds.windowXRes,state->ds.windowYRes,state->ds.uiElemPosX[uiElemInd],state->ds.uiElemPosY[uiElemInd],state->ds.uiElemWidth[uiElemInd],state->ds.uiElemHeight[uiElemInd]);
			break;
		case UIELEM_CHARTVIEW_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_BUTTON_WIDTH+CHARTVIEW_BUTTON_POS_XR+MENU_BUTTON_WIDTH+MENU_BUTTON_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(CHARTVIEW_BUTTON_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(CHARTVIEW_BUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[uiElemInd] = (uint16_t)((CHARTVIEW_BUTTON_POS_XR+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtMinusY[uiElemInd] = (uint16_t)((CHARTVIEW_BUTTON_POS_Y+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			break;
		case UIELEM_SEARCH_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_BUTTON_WIDTH+SEARCH_BUTTON_POS_XR+CHARTVIEW_BUTTON_WIDTH+CHARTVIEW_BUTTON_POS_XR+MENU_BUTTON_WIDTH+MENU_BUTTON_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(SEARCH_BUTTON_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_BUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[uiElemInd] = (uint16_t)((SEARCH_BUTTON_POS_XR+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtMinusY[uiElemInd] = (uint16_t)((SEARCH_BUTTON_POS_Y+1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			//SDL_Log("res: [%u %u]\nx: %u, y: %u, w: %u, h: %u\n",state->ds.windowXRes,state->ds.windowYRes,state->ds.uiElemPosX[uiElemInd],state->ds.uiElemPosY[uiElemInd],state->ds.uiElemWidth[uiElemInd],state->ds.uiElemHeight[uiElemInd]);
			break;
		case UIELEM_CONTEXT_MENU:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->mouseRightClickPosXPx);
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->mouseRightClickPosYPx);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(CONTEXT_MENU_WIDTH*state->ds.uiUserScale);
			if((state->ds.uiElemPosX[uiElemInd] + state->ds.uiElemWidth[uiElemInd]) > state->ds.windowXRes){
				state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.uiElemPosX[uiElemInd] - state->ds.uiElemWidth[uiElemInd]);
			}
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((state->cms.numContextMenuItems*CONTEXT_MENU_ITEM_SPACING + 2.0f*PANEL_EDGE_SIZE + 3.0f*UI_PADDING_SIZE)*state->ds.uiUserScale);
			if((state->ds.uiElemPosY[uiElemInd] + state->ds.uiElemHeight[uiElemInd]) > state->ds.windowYRes){
				state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->ds.uiElemPosY[uiElemInd] - state->ds.uiElemHeight[uiElemInd]);
			}
			if(state->cms.useHeaderText){
				state->ds.uiElemHeight[uiElemInd] += (int16_t)(CONTEXT_MENU_HEADER_HEIGHT*state->ds.uiUserScale);
			}
			break;
		case UIELEM_PRIMARY_MENU:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(PRIMARY_MENU_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(PRIMARY_MENU_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(PRIMARY_MENU_HEIGHT*state->ds.uiUserScale);
			break;
		case UIELEM_PM_PREFS_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((PRIMARY_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((PRIMARY_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((PRIMARY_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_PM_SCREENSHOT_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((PRIMARY_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + PRIMARY_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((PRIMARY_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((PRIMARY_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_PM_ABOUT_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((PRIMARY_MENU_WIDTH+PRIMARY_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((PRIMARY_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 2*PRIMARY_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((PRIMARY_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((PRIMARY_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CHARTVIEW_MENU:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(CHARTVIEW_MENU_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(CHARTVIEW_MENU_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(CHARTVIEW_MENU_HEIGHT*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_HALFLIFE_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_DECAYMODE_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 2*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_2PLUS_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 3*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_R42_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 4*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_BETA2_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 5*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_SPIN_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 6*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_PARITY_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 7*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_BEA_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 8*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_NUMLVLS:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 9*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_CVM_UNKNOWN_ENERGY_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((CHARTVIEW_MENU_WIDTH+CHARTVIEW_MENU_POS_XR - PANEL_EDGE_SIZE - 2*UI_PADDING_SIZE)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((CHARTVIEW_MENU_POS_Y + PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 10*CHARTVIEW_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)((CHARTVIEW_MENU_WIDTH - 2*PANEL_EDGE_SIZE - 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((CHARTVIEW_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_SEARCH_MENU:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(SEARCH_MENU_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_WIDTH*state->ds.uiUserScale);
			if(state->ss.numResults < MAX_DISP_SEARCH_RESULTS){
				state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_HEADER_HEIGHT + (float)state->ss.numResults*SEARCH_MENU_RESULT_HEIGHT)*state->ds.uiUserScale);
			}else{
				state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_HEADER_HEIGHT + MAX_DISP_SEARCH_RESULTS*SEARCH_MENU_RESULT_HEIGHT)*state->ds.uiUserScale);
			}
			break;
		case UIELEM_SEARCH_ENTRYBOX:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR-SEARCH_MENU_ENTRYBOX_POS_X)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((SEARCH_MENU_POS_Y+SEARCH_MENU_ENTRYBOX_POS_Y)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_ENTRYBOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			break;
		case UIELEM_SEARCH_RESULT:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR-SEARCH_MENU_ENTRYBOX_POS_X)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((SEARCH_MENU_POS_Y+SEARCH_MENU_ENTRYBOX_POS_Y+UI_TILE_SIZE+UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_ENTRYBOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_RESULT_HEIGHT-UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_SEARCH_RESULT_2:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR-SEARCH_MENU_ENTRYBOX_POS_X)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((SEARCH_MENU_POS_Y+SEARCH_MENU_ENTRYBOX_POS_Y+UI_TILE_SIZE+UI_PADDING_SIZE+SEARCH_MENU_RESULT_HEIGHT)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_ENTRYBOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_RESULT_HEIGHT-UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_SEARCH_RESULT_3:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR-SEARCH_MENU_ENTRYBOX_POS_X)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((SEARCH_MENU_POS_Y+SEARCH_MENU_ENTRYBOX_POS_Y+UI_TILE_SIZE+UI_PADDING_SIZE+2*SEARCH_MENU_RESULT_HEIGHT)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_ENTRYBOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_RESULT_HEIGHT-UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_SEARCH_RESULT_4:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((SEARCH_MENU_WIDTH+SEARCH_MENU_POS_XR-SEARCH_MENU_ENTRYBOX_POS_X)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((SEARCH_MENU_POS_Y+SEARCH_MENU_ENTRYBOX_POS_Y+UI_TILE_SIZE+UI_PADDING_SIZE+3*SEARCH_MENU_RESULT_HEIGHT)*state->ds.uiUserScale);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(SEARCH_MENU_ENTRYBOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((SEARCH_MENU_RESULT_HEIGHT-UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_ABOUT_BOX:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)((state->ds.windowXRes - ABOUT_BOX_WIDTH*state->ds.uiUserScale)/2);
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((state->ds.windowYRes - ABOUT_BOX_HEIGHT*state->ds.uiUserScale)/2);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(ABOUT_BOX_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(ABOUT_BOX_HEIGHT*state->ds.uiUserScale);
			//update child/dependant UI elements
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_ABOUT_BOX_CLOSEBUTTON);
			break;
		case UIELEM_ABOUT_BOX_CLOSEBUTTON:
			state->ds.uiElemWidth[UIELEM_ABOUT_BOX_CLOSEBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_ABOUT_BOX_CLOSEBUTTON] = state->ds.uiElemWidth[UIELEM_ABOUT_BOX_CLOSEBUTTON];
			state->ds.uiElemPosX[UIELEM_ABOUT_BOX_CLOSEBUTTON] = (int16_t)((float)(state->ds.uiElemPosX[UIELEM_ABOUT_BOX] + state->ds.uiElemWidth[UIELEM_ABOUT_BOX] - state->ds.uiElemWidth[UIELEM_ABOUT_BOX_CLOSEBUTTON]) - 4*UI_PADDING_SIZE*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_ABOUT_BOX_CLOSEBUTTON] = state->ds.uiElemPosY[UIELEM_ABOUT_BOX] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
			break;
		case UIELEM_PREFS_DIALOG:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)((state->ds.windowXRes - PREFS_DIALOG_WIDTH*state->ds.uiUserScale)/2);
			state->ds.uiElemPosY[uiElemInd] = (int16_t)((state->ds.windowYRes - PREFS_DIALOG_HEIGHT*state->ds.uiUserScale)/2);
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(PREFS_DIALOG_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(PREFS_DIALOG_HEIGHT*state->ds.uiUserScale);
			//update child/dependant UI elements
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_DIALOG_CLOSEBUTTON);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_PREFS_UISCALE_MENU);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_UISM_SMALL_BUTTON);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_UISM_DEFAULT_BUTTON);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_UISM_LARGE_BUTTON);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_UISM_HUGE_BUTTON);
			break;
		case UIELEM_PREFS_DIALOG_CLOSEBUTTON:
			state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON];
			state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = (int16_t)((float)(state->ds.uiElemPosX[UIELEM_PREFS_DIALOG] + state->ds.uiElemWidth[UIELEM_PREFS_DIALOG] - state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON]) - 4*UI_PADDING_SIZE*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_CLOSEBUTTON] = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
			break;
		case UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX:
			state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX];
			state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = state->ds.uiElemPosX[UIELEM_PREFS_DIALOG] + (int16_t)(PREFS_DIALOG_PREFCOL1_X*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + (int16_t)((PREFS_DIALOG_PREFCOL1_Y + PREFS_DIALOG_PREF_Y_SPACING + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*state->ds.uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_SHELLCLOSURE]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX:
			state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX];
			state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = state->ds.uiElemPosX[UIELEM_PREFS_DIALOG] + (int16_t)(PREFS_DIALOG_PREFCOL1_X*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + (int16_t)((PREFS_DIALOG_PREFCOL1_Y + 2*PREFS_DIALOG_PREF_Y_SPACING + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*state->ds.uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_LIFETIME]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX:
			state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX];
			state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = state->ds.uiElemPosX[UIELEM_PREFS_DIALOG] + (int16_t)(PREFS_DIALOG_PREFCOL1_X*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + (int16_t)((PREFS_DIALOG_PREFCOL1_Y + 3*PREFS_DIALOG_PREF_Y_SPACING + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX] = (uint16_t)(2*UI_PADDING_SIZE*state->ds.uiUserScale) + (uint16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_UIANIM]])/rdat->uiDPIScale); //so that checkbox can be toggled by clicking on adjacent text
			break;
		case UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN:
			state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (int16_t)(PREFS_DIALOG_UISCALE_BUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = (int16_t)(state->ds.uiElemPosX[UIELEM_PREFS_DIALOG] + (int16_t)((PREFS_DIALOG_PREFCOL1_X+3*UI_PADDING_SIZE)*state->ds.uiUserScale) + (int16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,dat->strings[dat->locStringIDs[LOCSTR_PREF_UISCALE]])/rdat->uiDPIScale));
			state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + (int16_t)((PREFS_DIALOG_PREFCOL1_Y)*state->ds.uiUserScale);
			break;
		case UIELEM_PREFS_UISCALE_MENU:
			state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU] = (int16_t)(PREFS_DIALOG_UISCALE_MENU_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU] = (int16_t)(((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*UISCALE_ENUM_LENGTH + 2*PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] = (int16_t)(state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] + state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]/2 - state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU]/2);
			state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] = (int16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN] + state->ds.uiElemHeight[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]);
			break;
		case UIELEM_UISM_SMALL_BUTTON:
			state->ds.uiElemPosX[UIELEM_UISM_SMALL_BUTTON] = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_UISM_SMALL_BUTTON] =  state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_UISM_SMALL_BUTTON] = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (int16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_UISM_SMALL_BUTTON] = (int16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_UISM_DEFAULT_BUTTON:
			state->ds.uiElemPosX[UIELEM_UISM_DEFAULT_BUTTON] = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_UISM_DEFAULT_BUTTON] =  state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_UISM_DEFAULT_BUTTON] = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (int16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_UISM_DEFAULT_BUTTON] = (int16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_UISM_LARGE_BUTTON:
			state->ds.uiElemPosX[UIELEM_UISM_LARGE_BUTTON] = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_UISM_LARGE_BUTTON] =  state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 2*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_UISM_LARGE_BUTTON] = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (int16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_UISM_LARGE_BUTTON] = (int16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_UISM_HUGE_BUTTON:
			state->ds.uiElemPosX[UIELEM_UISM_HUGE_BUTTON] = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_UISM_HUGE_BUTTON] =  state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + (int16_t)((PANEL_EDGE_SIZE + 2*UI_PADDING_SIZE + 3*PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING)*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_UISM_HUGE_BUTTON] = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU] - (int16_t)((2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_UISM_HUGE_BUTTON] = (int16_t)((PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING - UI_PADDING_SIZE)*state->ds.uiUserScale);
			break;
		case UIELEM_NUCL_INFOBOX:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)((state->ds.windowXRes - state->ds.infoBoxWidth*state->ds.uiUserScale)/2);
			int16_t freeXSpace = (int16_t)(state->ds.windowXRes - state->ds.infoBoxWidth*state->ds.uiUserScale);
			if(freeXSpace < 4*NUCL_INFOBOX_X_PADDING*state->ds.uiUserScale){
				state->ds.uiElemPosX[uiElemInd] += (int16_t)(NUCL_INFOBOX_X_PADDING*state->ds.uiUserScale - freeXSpace/4); //make sure info box doesn't bump up against y-axis
			}
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(((float)NUCL_INFOBOX_MIN_HEIGHT + state->ds.infoBoxTableHeight + 2*PANEL_EDGE_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->ds.windowYRes - state->ds.uiElemHeight[uiElemInd] - (int16_t)((UI_PADDING_SIZE + CHART_AXIS_DEPTH)*state->ds.uiUserScale));
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(state->ds.infoBoxWidth*state->ds.uiUserScale);
			//update child/dependant UI elements
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX_CLOSEBUTTON);
			updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
			break;
		case UIELEM_NUCL_INFOBOX_CLOSEBUTTON:
			state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON];
			state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - (int32_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale));
			state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
			break;
		case UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON:
			state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(NUCL_INFOBOX_ALLLEVELS_BUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
				float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH);
				int16_t defaultPosX = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*state->ds.uiUserScale));
				int16_t defaultPosY = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
				int16_t fullPosX = (int16_t)(state->ds.windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*state->ds.uiUserScale);
				int16_t fullPosY = (int16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*state->ds.uiUserScale);
				state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(defaultPosX + animFrac*(fullPosX - defaultPosX));
				state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(defaultPosY + animFrac*(fullPosY - defaultPosY));
			}else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
				float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH);
				int16_t defaultPosX = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*state->ds.uiUserScale));
				int16_t defaultPosY = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
				int16_t fullPosX = (int16_t)(state->ds.windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*state->ds.uiUserScale);
				int16_t fullPosY = (int16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*state->ds.uiUserScale);
				state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(fullPosX + animFrac*(defaultPosX - fullPosX));
				state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(fullPosY + animFrac*(defaultPosY - fullPosY));
			}else{
				state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX] + state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] - (int32_t)(7*UI_PADDING_SIZE*state->ds.uiUserScale));
				state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + (int16_t)(4*UI_PADDING_SIZE*state->ds.uiUserScale);
			}
			//SDL_Log("x: %u, y: %u, w: %u, h: %u\n",state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON]);
			break;
		case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
			state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (int16_t)(state->ds.windowXRes-(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH+NUCL_FULLINFOBOX_BACKBUTTON_POS_XR)*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (int16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (int16_t)(NUCL_FULLINFOBOX_BACKBUTTON_WIDTH*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)((NUCL_FULLINFOBOX_BACKBUTTON_POS_XR+1.0f)*state->ds.uiUserScale); //Fitt's law
			state->ds.uiElemExtMinusY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON] = (uint16_t)((NUCL_FULLINFOBOX_BACKBUTTON_POS_Y+1.0f)*state->ds.uiUserScale); //Fitt's law
			break;
		case UIELEM_NUCL_FULLINFOBOX_RXNBUTTON:
			if(state->ds.selectedRxn == 0){
				state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (int16_t)(NUCL_FULLINFOBOX_RXNBUTTON_WIDTH*state->ds.uiUserScale);
			}else{
				char rxnStr[32];
    		getRxnStr(rxnStr,&dat->ndat,dat->ndat.nuclData[state->chartSelectedNucl].firstRxn + (uint32_t)(state->ds.selectedRxn-1));
				state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (int16_t)(getTextWidth(rdat,FONTSIZE_NORMAL,rxnStr)/rdat->uiDPIScale + 60*state->ds.uiUserScale);
			}
			state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (int16_t)(state->ds.windowXRes-state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON]-(NUCL_FULLINFOBOX_RXNBUTTON_POS_XR*state->ds.uiUserScale));
			state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (int16_t)(NUCL_FULLINFOBOX_BACKBUTTON_POS_Y*state->ds.uiUserScale);
			state->ds.uiElemExtMinusY[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] = (uint16_t)((NUCL_FULLINFOBOX_BACKBUTTON_POS_Y+1.0f)*state->ds.uiUserScale); //Fitt's law
			break;
		case UIELEM_RXN_MENU:
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] + state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON]);
			float effResY = (float)(state->ds.windowYRes);
			if(effResY > 650.0f*state->ds.uiUserScale){
				effResY = 650.0f*state->ds.uiUserScale; //limit vertical size of the menu
			}
			state->ds.rxnMenuColumns = (uint8_t)(SDL_ceilf((dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1.0f)/SDL_floorf((float)(effResY - state->ds.uiElemPosY[uiElemInd])/(RXN_MENU_ITEM_SPACING*state->ds.uiUserScale) - 1.0f)));
			if(state->ds.rxnMenuColumns == 0){
				state->ds.rxnMenuColumns = 1;
			}
			if((state->ds.rxnMenuColumns < 3)&&(((dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1.0f)/state->ds.rxnMenuColumns) > 8.5f)){
				state->ds.rxnMenuColumns++;
			}
			//SDL_Log("numRxns: %u, columns: %u\n",dat->ndat.nuclData[state->chartSelectedNucl].numRxns,state->ds.rxnMenuColumns);
			int16_t menuWidth = (int16_t)((RXN_MENU_COLUMN_WIDTH*state->ds.rxnMenuColumns + 4.0f*PANEL_EDGE_SIZE)*state->ds.uiUserScale);
			while(menuWidth > (state->ds.windowXRes - 2*UI_PADDING_SIZE*state->ds.uiUserScale)){
				if(state->ds.rxnMenuColumns > 1){
					state->ds.rxnMenuColumns--;
					menuWidth = (int16_t)((RXN_MENU_COLUMN_WIDTH*state->ds.rxnMenuColumns + 4.0f*PANEL_EDGE_SIZE)*state->ds.uiUserScale);
				}else{
					break;
				}
			}
			state->ds.uiElemWidth[uiElemInd] = menuWidth;
			state->ds.uiElemHeight[uiElemInd] = (int16_t)((SDL_ceilf((dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1.0f)/(state->ds.rxnMenuColumns*1.0f))*RXN_MENU_ITEM_SPACING + 2*PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON] + state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_RXNBUTTON]/2 - state->ds.uiElemWidth[UIELEM_RXN_MENU]/2);
			if((state->ds.uiElemPosX[uiElemInd] + state->ds.uiElemWidth[uiElemInd]) > state->ds.windowXRes){
				//offset menu to keep it on-screen
				state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes - state->ds.uiElemWidth[uiElemInd] - UI_PADDING_SIZE*state->ds.uiUserScale);
			}
			//SDL_Log("position: %i %i, dimensions: %i %i\n",state->ds.uiElemPosX[uiElemInd],state->ds.uiElemPosY[uiElemInd],state->ds.uiElemWidth[uiElemInd],state->ds.uiElemHeight[uiElemInd]);
			break;
		case UIELEM_NUCL_FULLINFOBOX_SCROLLBAR:
			state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (int16_t)(state->ds.windowXRes - NUCL_FULLINFOBOX_SCROLLBAR_POS_XR*state->ds.uiUserScale);
			state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (int16_t)((NUCL_FULLINFOBOX_LEVELLIST_POS_Y + UI_PADDING_SIZE)*state->ds.uiUserScale);
			state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (int16_t)(0.5f*UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (int16_t)(state->ds.windowYRes - state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] - 2*UI_PADDING_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtMinusX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(0.5f*NUCL_FULLINFOBOX_SCROLLBAR_POS_XR*state->ds.uiUserScale); //make mouse target a bit bigger
			state->ds.uiElemExtPlusX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(NUCL_FULLINFOBOX_SCROLLBAR_POS_XR*state->ds.uiUserScale); //Fitts' law (extend to screen edge)
			state->ds.uiElemExtPlusY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR] = (uint16_t)(NUCL_FULLINFOBOX_LEVELLIST_POS_Y*state->ds.uiUserScale); //Fitts' law (extend to screen edge)
			break;
		case UIELEM_ZOOMIN_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((UI_TILE_SIZE+ZOOM_BUTTON_POS_XR)*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->ds.windowYRes-((UI_TILE_SIZE+ZOOM_BUTTON_POS_YB+CHART_AXIS_DEPTH)*state->ds.uiUserScale));
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[uiElemInd] = (uint16_t)((ZOOM_BUTTON_POS_XR + 1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtPlusY[uiElemInd] = (uint16_t)((ZOOM_BUTTON_POS_YB + 1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtMinusX[uiElemInd] = (uint16_t)((0.5f*ZOOM_BUTTON_POS_XR + 1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			break;
		case UIELEM_ZOOMOUT_BUTTON:
			state->ds.uiElemPosX[uiElemInd] = (int16_t)(state->ds.windowXRes-((2*(UI_TILE_SIZE+ZOOM_BUTTON_POS_XR))*state->ds.uiUserScale));
			state->ds.uiElemPosY[uiElemInd] = (int16_t)(state->ds.windowYRes-((UI_TILE_SIZE+ZOOM_BUTTON_POS_YB+CHART_AXIS_DEPTH)*state->ds.uiUserScale));
			state->ds.uiElemWidth[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemHeight[uiElemInd] = (int16_t)(UI_TILE_SIZE*state->ds.uiUserScale);
			state->ds.uiElemExtPlusX[uiElemInd] = (uint16_t)((ZOOM_BUTTON_POS_XR + 1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			state->ds.uiElemExtPlusY[uiElemInd] = (uint16_t)((ZOOM_BUTTON_POS_YB + 1.0f)*state->ds.uiUserScale); //prevent clicking chart 'in between' buttons
			break;
		default:
			break;
	}
}
void updateUIElemPositions(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){
    updateSingleUIElemPosition(dat,state,rdat,i);
  }
	clearSelectionStrs(&state->tss,1); //update selection strings if needed
}

//removes all selectable string data where the strings intersect with the provided rect
void removeSelectableStringsInRect(text_selection_state *restrict tss, const SDL_FRect rect){
  if(tss->selStrsModifiable){
		//SDL_Log("Removing selection strings in rect: [ %.2f %.2f %.2f %.2f ]\n",(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
    for(uint16_t i=0; i<tss->numSelStrs; i++){
      if(SDL_HasRectIntersectionFloat(&rect,&tss->selectableStrRect[i])){
        //delet the overlapping selection string data
				//SDL_Log("Removing selection string %u.\n",i);
        for(uint16_t j=(i+1); j<tss->numSelStrs; j++){
          memcpy(&tss->selectableStrRect[j-1],&tss->selectableStrRect[j],sizeof(SDL_FRect));
          memcpy(&tss->selectableStrFontSize[j-1],&tss->selectableStrFontSize[j],sizeof(uint8_t));
          memcpy(&tss->selectableStrTxt[j-1],&tss->selectableStrTxt[j],sizeof(char)*MAX_SELECTABLE_STR_LEN);
        }
        tss->numSelStrs--;
				i--;
      }
    }
		tss->selectedStr = 65535; //de-select any selected string
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
	if(state->uiState == UISTATE_FULLLEVELINFO){
		state->ds.nuclFullInfoMaxScrollY = getMaxNumLvlDispLines(&dat->ndat,state);
		setFullLevelInfoDimensions(dat,state,rdat,state->chartSelectedNucl);
	}else if(state->uiState == UISTATE_INFOBOX){
		setInfoBoxDimensions(dat,state,rdat,state->chartSelectedNucl);
	}
	updateUIElemPositions(dat,state,rdat); //UI element positions
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
  updateUIElemPositions(dat,state,rdat); //UI element positions
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
