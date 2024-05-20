/* © J. Williams 2017-2024 */
/* Functions handling low-level operations on ENSDF database, or calculations using app data/state */

#include "juicer.h"
#include "data_ops.h"
#include "load_data.h"
#include "gui_constants.h"
#include "basename.h" //SDL_basename implementation, based on https://github.com/libsdl-org/SDL/issues/7915, remove once this is added to SDL3

//Initializes the temporary (unsaved) portion of the app state.
void initializeTempState(app_state *restrict state){
  //input
  state->mouseX = -1;
  state->mouseY = -1;
  state->mouseHoldStartPosX = -1;
  state->mouseHoldStartPosY = -1;
  state->lastAxisValX=0;
  state->lastAxisValY=0;
  state->activeAxis=0; //the last used axis
  state->lastInputType = INPUT_TYPE_KEYBOARD; //default input type
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
void updateDrawingState(drawing_state *ds, const float deltaTime){
	if(ds->zoomFinished){
		//we want the zooming flag to persist for 1 frame beyond the
		//end of the zoom, to force the UI to redraw
		//printf("Finished zoom.\n");
		ds->zoomInProgress = 0;
		ds->zoomFinished = 0; //reset flag
	}
	if(ds->zoomInProgress){
		ds->timeSinceZoomStart += deltaTime;
		ds->chartZoomScale = ds->chartZoomStartScale + ((ds->chartZoomToScale - ds->chartZoomStartScale)*ds->timeSinceZoomStart/CHART_ZOOM_TIME);
		if(ds->chartZoomToScale > ds->chartZoomStartScale){
			ds->chartPosX = ds->chartZoomStartX + ((ds->chartZoomToX - ds->chartZoomStartX)*juice_smoothStop2(ds->timeSinceZoomStart/CHART_ZOOM_TIME));
			ds->chartPosY = ds->chartZoomStartY + ((ds->chartZoomToY - ds->chartZoomStartY)*juice_smoothStop2(ds->timeSinceZoomStart/CHART_ZOOM_TIME));
		}else{
			ds->chartPosX = ds->chartZoomStartX + ((ds->chartZoomToX - ds->chartZoomStartX)*juice_smoothStart2(ds->timeSinceZoomStart/CHART_ZOOM_TIME));
			ds->chartPosY = ds->chartZoomStartY + ((ds->chartZoomToY - ds->chartZoomStartY)*juice_smoothStart2(ds->timeSinceZoomStart/CHART_ZOOM_TIME));
		}
		if(ds->timeSinceZoomStart >= CHART_ZOOM_TIME){
			ds->chartZoomScale = ds->chartZoomToScale;
			ds->chartPosX = ds->chartZoomToX;
			ds->chartPosY = ds->chartZoomToY;
			ds->zoomFinished = 1;
		}
		
		//printf("zoom scale: %0.4f\n",(double)ds->chartZoomScale);
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
				return -1.0; //stable
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

float mouseXtoN(const drawing_state *restrict ds, const float mouseX){
	return ds->chartPosX + ((mouseX - ds->windowXRes/2.0f)/(DEFAULT_NUCLBOX_DIM*ds->chartZoomScale));
}
float mouseYtoZ(const drawing_state *restrict ds, const float mouseY){
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

void mouseWheelAction(app_state *restrict state){
	if(state->uiState == UISTATE_DEFAULT){
		if(state->mouseWheelDir == 1){
			//zoom in
			state->ds.chartZoomStartScale = state->ds.chartZoomScale;
			state->ds.chartZoomToScale = state->ds.chartZoomToScale*2.0f;
			state->ds.chartZoomStartX = state->ds.chartPosX;
			state->ds.chartZoomStartY = state->ds.chartPosY;
			if(state->ds.chartZoomToScale > MAX_CHART_ZOOM_SCALE){
				state->ds.chartZoomToScale = MAX_CHART_ZOOM_SCALE;
			}
			if(state->ds.zoomInProgress == 0){
				state->ds.chartZoomStartMouseN = mouseXtoN(&state->ds,state->mouseX);
				state->ds.chartZoomStartMouseZ = mouseYtoZ(&state->ds,state->mouseY);
			}
			float xZoomFrac = (state->ds.chartZoomStartMouseN - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
			float afterZoomMinN = state->ds.chartZoomStartMouseN - getChartWidthNAfterZoom(&state->ds)*xZoomFrac;
			state->ds.chartZoomToX = afterZoomMinN + getChartWidthNAfterZoom(&state->ds)*0.5f;
			float yZoomFrac = (state->ds.chartZoomStartMouseZ - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
			float afterZoomMinZ = state->ds.chartZoomStartMouseZ - getChartHeightZAfterZoom(&state->ds)*yZoomFrac;
			state->ds.chartZoomToY = afterZoomMinZ + getChartHeightZAfterZoom(&state->ds)*0.5f;
			if(state->ds.chartZoomToX > MAX_CHART_ZOOM_X){
				state->ds.chartZoomToX = MAX_CHART_ZOOM_X;
			}
			if(state->ds.chartZoomToY > MAX_CHART_ZOOM_Y){
				state->ds.chartZoomToY = MAX_CHART_ZOOM_Y;
			}
			//printf("xZoomFrac: %0.3f, afterZoomMinN: %0.3f\n",(double)xZoomFrac,(double)afterZoomMinN);
			//printf("yZoomFrac: %0.3f, afterZoomMinZ: %0.3f\n",(double)yZoomFrac,(double)afterZoomMinZ);
			//printf("N: %0.1f, Z: %0.1f, start: [%0.1f %0.1f], zoom to: [%0.1f %0.1f]\n",(double)mouseXtoN(&state->ds,state->mouseX),(double)mouseYtoZ(&state->ds,state->mouseY),(double)state->ds.chartZoomStartX,(double)state->ds.chartZoomStartY,(double)state->ds.chartZoomToX,(double)state->ds.chartZoomToY);
			state->ds.timeSinceZoomStart = 0.0f;
			state->ds.zoomInProgress = 1;
			state->ds.zoomFinished = 0;
		}else if(state->mouseWheelDir == 2){
			//zoom out
			state->ds.chartZoomStartScale = state->ds.chartZoomScale;
			state->ds.chartZoomToScale = state->ds.chartZoomToScale*0.5f;
			state->ds.chartZoomStartX = state->ds.chartPosX;
			state->ds.chartZoomStartY = state->ds.chartPosY;
			if(state->ds.chartZoomToScale < MIN_CHART_ZOOM_SCALE){
				state->ds.chartZoomToScale = MIN_CHART_ZOOM_SCALE;
			}
			if(state->ds.zoomInProgress == 0){
				state->ds.chartZoomStartMouseN = mouseXtoN(&state->ds,state->mouseX);
				state->ds.chartZoomStartMouseZ = mouseYtoZ(&state->ds,state->mouseY);
			}
			float xZoomFrac = (state->ds.chartZoomStartMouseN - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
			float afterZoomMinN = state->ds.chartZoomStartMouseN - getChartWidthNAfterZoom(&state->ds)*xZoomFrac;
			state->ds.chartZoomToX = afterZoomMinN + getChartWidthNAfterZoom(&state->ds)*0.5f;
			float yZoomFrac = (state->ds.chartZoomStartMouseZ - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
			float afterZoomMinZ = state->ds.chartZoomStartMouseZ - getChartHeightZAfterZoom(&state->ds)*yZoomFrac;
			state->ds.chartZoomToY = afterZoomMinZ + getChartHeightZAfterZoom(&state->ds)*0.5f;
			if(state->ds.chartZoomToX < 0.0f){
				state->ds.chartZoomToX = 0.0f;
			}
			if(state->ds.chartZoomToY < 0.0f){
				state->ds.chartZoomToY = 0.0f;
			}
			//printf("xZoomFrac: %0.3f, afterZoomMinN: %0.3f\n",(double)xZoomFrac,(double)afterZoomMinN);
			//printf("yZoomFrac: %0.3f, afterZoomMinZ: %0.3f\n",(double)yZoomFrac,(double)afterZoomMinZ);
			//printf("N: %0.1f, Z: %0.1f, start: [%0.1f %0.1f], zoom to: [%0.1f %0.1f]\n",(double)mouseXtoN(&state->ds,state->mouseX),(double)mouseYtoZ(&state->ds,state->mouseY),(double)state->ds.chartZoomStartX,(double)state->ds.chartZoomStartY,(double)state->ds.chartZoomToX,(double)state->ds.chartZoomToY);
			state->ds.timeSinceZoomStart = 0.0f;
			state->ds.zoomInProgress = 1;
			state->ds.zoomFinished = 0;
		}
	}
	//printf("scale: %0.2f\n",(double)state->ds.chartZoomScale);
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
      case UIELEM_CHARTDRAW_AREA:
        ds->uiElemPosX[i] = 0;
        ds->uiElemPosY[i] = (uint16_t)(MENU_BAR_HEIGHT);
        ds->uiElemWidth[i] = (uint16_t)(ds->windowXRes);
        ds->uiElemHeight[i] = (uint16_t)((ds->windowYRes - MENU_BAR_HEIGHT));
        break;
      default:
        break;
    }
  }
}

void updateWindowRes(drawing_state *restrict ds, resource_data *restrict rdat){
  int wwidth, wheight;
  int rwidth, rheight;
  SDL_GetWindowSize(rdat->window, &wwidth, &wheight);
  SDL_GetWindowSizeInPixels(rdat->window, &rwidth, &rheight);
  if((rwidth != ds->windowXRenderRes)||(rheight != ds->windowYRenderRes)){
    ds->forceRedraw = 1;
  }
  rdat->uiScale = (float)rwidth/((float)wwidth); //set UI scale properly for HI-DPI
  ds->windowXRes = (uint16_t)wwidth;
  ds->windowYRes = (uint16_t)wheight;
  ds->windowXRenderRes = (uint16_t)rwidth;
  ds->windowYRenderRes = (uint16_t)rheight;

  //update things that depend on the window res
  updateUIElemPositions(ds); //UI element positions
}

void handleScreenGraphicsMode(drawing_state *restrict ds, resource_data *restrict rdat){

  //handle vsync and frame cap
  SDL_SetRenderVSync(rdat->renderer,1); //vsync always enabled
  rdat->uiScale = 1.0f;

  if(ds->windowFullscreenMode){
    if(SDL_SetWindowFullscreen(rdat->window,SDL_TRUE) != 0){
      printf("WARNING: cannot set fullscreen mode - %s\n",SDL_GetError());
    }
    updateWindowRes(ds,rdat);
    //printf("Full screen display mode.  Window resolution: %u x %u.\n",ds->windowXRes,ds->windowYRes);
  }else{
    if(SDL_SetWindowFullscreen(rdat->window,0) != 0){
      printf("WARNING: cannot set windowed mode - %s\n",SDL_GetError());
    }
    updateWindowRes(ds,rdat);
    //printf("Windowed display mode.  Window resolution: %u x %u.\n",ds->windowXRes,ds->windowYRes);
  }
}
