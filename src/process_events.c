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

/* Functions handling user input processing */

#include "process_events.h"
#include "data_ops.h"
#include "gui_constants.h" //to compute mouse/pointer interactions

void fcScrollAction(app_state *restrict state, const float deltaVal){
  //SDL_Log("scroll delta: %f\n",(double)deltaVal);
  state->ds.nuclFullInfoScrollStartY = state->ds.nuclFullInfoScrollY;
  state->ds.nuclFullInfoScrollToY = state->ds.nuclFullInfoScrollY - 1.0f*state->scrollSpeedMultiplier*deltaVal;
  if(state->ds.nuclFullInfoScrollToY < 0.0f){
    state->ds.nuclFullInfoScrollToY = 0.0f;
  }else if(state->ds.nuclFullInfoScrollToY > state->ds.nuclFullInfoMaxScrollY){
    state->ds.nuclFullInfoScrollToY = (float)state->ds.nuclFullInfoMaxScrollY;
  }
  state->ds.timeSinceFCScollStart = 0.0f;
  state->ds.fcScrollInProgress = 1;
  state->ds.fcScrollFinished = 0;
  //SDL_Log("scroll pos: %f, scroll to: %f\n",(double)state->ds.nuclFullInfoScrollY,(double)state->ds.nuclFullInfoScrollToY);
}

void processInputFlags(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  
  /* Handle directional input */
  uint32_t up,down,left,right,altup,altdown,altleft,altright;
  up = (state->inputFlags & (1U << INPUT_UP));
  down = (state->inputFlags & (1U << INPUT_DOWN));
  left = (state->inputFlags & (1U << INPUT_LEFT));
  right = (state->inputFlags & (1U << INPUT_RIGHT));
  altup = (state->inputFlags & (1U << INPUT_ALTUP));
  altdown = (state->inputFlags & (1U << INPUT_ALTDOWN));
  altleft = (state->inputFlags & (1U << INPUT_ALTLEFT));
  altright = (state->inputFlags & (1U << INPUT_ALTRIGHT));
  
  if(state->uiState == UISTATE_DEFAULT){

    //in main chart view, handle chart panning
    
    //SDL_Log("dir: [%u %u %u %u]\n",!(up==0),!(down==0),!(left==0),!(right==0));
    
    if(left || right || up || down){
      if(left && !right){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        state->ds.chartPanToX = state->ds.chartPosX - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToY = state->ds.chartPosY;
        }
        if(state->ds.chartPanToX <= (-0.25f*getChartWidthN(&state->ds))){
          state->ds.chartPanToX = (-0.25f*getChartWidthN(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
      }else if(right && !left){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        state->ds.chartPanToX = state->ds.chartPosX + (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToY = state->ds.chartPosY;
        }
        if(state->ds.chartPanToX >= (dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds)))){
          state->ds.chartPanToX = dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
      }
      if(up && !down){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToX = state->ds.chartPosX;
        }
        state->ds.chartPanToY = state->ds.chartPosY + (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.chartPanToY >= (dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds)))){
          state->ds.chartPanToY = dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
        //SDL_Log("pan start: [%0.2f %0.2f], pan to: [%0.2f %0.2f]\n",(double)state->ds.chartPanStartX,(double)state->ds.chartPanStartY,(double)state->ds.chartPanToX,(double)state->ds.chartPanToY);
      }else if(down && !up){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToX = state->ds.chartPosX;
        }
        state->ds.chartPanToY = state->ds.chartPosY - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.chartPanToY <= (-0.25f*getChartHeightZ(&state->ds))){
          state->ds.chartPanToY = (-0.25f*getChartHeightZ(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
      }
    }else if(altleft || altright || altup || altdown){
      
      if(state->chartSelectedNucl == MAXNUMNUCL){
        //select nucleus
        int16_t selectedN = (int16_t)(state->ds.chartPosX - 0.5f);
        int16_t selectedZ = (int16_t)(state->ds.chartPosY + 0.5f + (16.0f/state->ds.chartZoomScale));
        uint16_t selNucl = getNearestNuclInd(dat,selectedN,selectedZ);
        setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
      }else{
        //change selected nucleus
        if(state->ds.panInProgress == 0){
          if(altleft && !altright){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }else if(altright && !altleft){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }
          if(altup && !altdown){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }else if(altdown && !altup){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }
        }
      }
      
    }
  }else if(state->uiState == UISTATE_FULLLEVELINFO){

    if(up && !down){
      fcScrollAction(state,0.5f);
    }else if(down && !up){
      fcScrollAction(state,-0.5f);
    }else if(state->ds.fcNuclChangeInProgress == 0){
      //change selected nucleus
      if(altleft && !altright){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
            break;
          }
        }
      }else if(altright && !altleft){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
            break;
          }
        }
      }
      if(altup && !altdown){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i));
            break;
          }
        }
      }else if(altdown && !altup){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i));
            break;
          }
        }
      }
    }

  }
  if(state->inputFlags & (1U << INPUT_BACK)){
    //escape open menus
    //handle modal dialogs first
    if((state->ds.shownElements & (1U << UIELEM_ABOUT_BOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      changeUIState(dat,state,state->lastUIState); //restore previous interactable elements
      startUIAnimation(dat,state,UIANIM_MODAL_BOX_HIDE); //hide the about/message box, see stopUIAnimation()
    }else if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
      //close the UI scale menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
    }else if((state->ds.shownElements & (1U << UIELEM_PREFS_DIALOG))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      changeUIState(dat,state,state->lastUIState); //restore previous interactable elements
      startUIAnimation(dat,state,UIANIM_MODAL_BOX_HIDE); //hide the about/message box, see stopUIAnimation()
    }else if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
      startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
      startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
    }else if((state->uiState == UISTATE_FULLLEVELINFO)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)){
      uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON); //go back to the main chart
    }else if((state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
      //close the primary menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
    }else if((state->ds.shownElements & (1U << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
      //close the chart view menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON);
    }else if(state->ds.windowFullscreenMode){
      //exit fullscreen
      state->ds.windowFullscreenMode = 0;
      handleScreenGraphicsMode(dat,state,rdat); 
    }else if((state->uiState == UISTATE_DEFAULT)&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f)){
      //if nothing else is going on, open the primary menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
    }
  }

  /* Handle mouse input */

  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->mouseholdElement = UIELEM_ENUM_LENGTH;

  if(state->ds.dragInProgress == 0){
    for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){ //ordering in ui_element_enum defines order in which UI elements receive input
      if(state->interactableElement & (uint32_t)(1U << i)){
        if((state->mouseHoldStartPosXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseHoldStartPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseHoldStartPosYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseHoldStartPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
          state->mouseholdElement = i;
        }
        if((state->mouseXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
          state->mouseoverElement = i;
          //SDL_Log("mouseover element: %u\n",i);
          if((state->mouseClickPosXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseClickPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseClickPosYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseClickPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
            //take action
            uiElemClickAction(dat,state,rdat,0,i);
            return;
          }
          break;
        }
      }
    }

    //handle click and drag on the chart of nuclides
    if((state->uiState == UISTATE_DEFAULT)&&(state->mouseholdElement == UIELEM_ENUM_LENGTH)&&(state->mouseHoldStartPosXPx >= 0.0f)){
      state->ds.chartDragStartX = state->ds.chartPosX;
      state->ds.chartDragStartY = state->ds.chartPosY;
      state->ds.chartDragStartMouseX = state->mouseXPx;
      state->ds.chartDragStartMouseY = state->mouseYPx;
      state->ds.dragInProgress = 1;
      //SDL_Log("start drag\n");
    }
  }

  //check for mouse release
  if(state->mouseHoldStartPosXPx < 0.0f){
    state->ds.dragFinished = 1;
  }

  uint32_t doubleClick = (state->inputFlags & (1U << INPUT_DOUBLECLICK));

  //only get here if no button was clicked
  //check if a click was made outside of any button
  //SDL_Log("click pos x: %f, drag start: [%f %f]\n",(double)state->mouseClickPosXPx,(double)state->ds.chartDragStartMouseX,(double)state->ds.chartDragStartMouseY);
  if(state->uiState == UISTATE_DEFAULT){
    if((state->mouseClickPosXPx >= 0.0f) && (fabsf(state->ds.chartDragStartMouseX - state->mouseXPx) < 5.0f) && (fabsf(state->ds.chartDragStartMouseY - state->mouseYPx) < 5.0f) ){
      //unclick (or click on chart view)
      if(doubleClick){
        uiElemClickAction(dat,state,rdat,1,UIELEM_ENUM_LENGTH);
      }else{
        uiElemClickAction(dat,state,rdat,0,UIELEM_ENUM_LENGTH);
      }
    }
  }else if(state->uiState == UISTATE_FULLLEVELINFO){
    if(state->mouseClickPosXPx >= 0.0f){
      //clicked outside of interactable items on the full level info screen
      uiElemClickAction(dat,state,rdat,0,UIELEM_ENUM_LENGTH);
    }
  }else if(state->uiState == UISTATE_PREFS_DIALOG){
    if(state->mouseClickPosXPx >= 0.0f){
      if((state->ds.shownElements & (1U << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
        //close the UI scale menu
        uiElemClickAction(dat,state,rdat,0,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
      }
    }
  }

  /* Handle zoom input */
  if((state->inputFlags & (1U << INPUT_ZOOM))&&(fabsf(state->zoomDeltaVal)>0.05f)){
    if(state->uiState == UISTATE_DEFAULT){
      if(state->zoomDeltaVal != 0.0f){
        state->ds.chartZoomStartScale = state->ds.chartZoomScale;
        if(state->zoomDeltaVal > 0){
          //zoom in
          state->ds.chartZoomToScale += state->zoomDeltaVal*state->ds.chartZoomToScale*1.0f;
          if(state->ds.chartZoomToScale > MAX_CHART_ZOOM_SCALE){
            state->ds.chartZoomToScale = MAX_CHART_ZOOM_SCALE;
          }
        }else{
          //zoom out
          state->ds.chartZoomToScale += state->zoomDeltaVal*state->ds.chartZoomToScale*0.5f;
          if(state->ds.chartZoomToScale < MIN_CHART_ZOOM_SCALE){
            state->ds.chartZoomToScale = MIN_CHART_ZOOM_SCALE;
          }
        }
        if(state->ds.zoomInProgress == 0){
          if(state->lastInputType == INPUT_TYPE_MOUSE){
            //zoom using mouse wheel/touchpad, zoom to cursor location
            state->ds.chartZoomStartMouseX = mouseXPxToN(&state->ds,state->mouseXPx);
            state->ds.chartZoomStartMouseY = mouseYPxToZ(&state->ds,state->mouseYPx);
          }else{
            //zoom using keyboard or gamepad, zoom to center of screen
            state->ds.chartZoomStartMouseX = state->ds.chartPosX;
            if(state->chartSelectedNucl != MAXNUMNUCL){
              state->ds.chartZoomStartMouseY = state->ds.chartPosY + (16.0f/state->ds.chartZoomScale); //corrected for position of selected nuclide
            }else{
              state->ds.chartZoomStartMouseY = state->ds.chartPosY; //centred on screen
            }
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
        float afterZoomMinN = state->ds.chartZoomStartMouseX - getChartWidthNAfterZoom(&state->ds)*state->ds.chartZoomStartMouseXFrac;
        state->ds.chartZoomToX = afterZoomMinN + getChartWidthNAfterZoom(&state->ds)*0.5f;
        state->ds.chartZoomStartMouseYFrac = (state->ds.chartZoomStartMouseY - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
        float afterZoomMinZ = state->ds.chartZoomStartMouseY - getChartHeightZAfterZoom(&state->ds)*state->ds.chartZoomStartMouseYFrac;
        state->ds.chartZoomToY = afterZoomMinZ + getChartHeightZAfterZoom(&state->ds)*0.5f;
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
        //SDL_Log("xZoomFrac: %0.3f, afterZoomMinN: %0.3f\n",(double)state->ds.chartZoomStartMouseXFrac,(double)afterZoomMinN);
        //SDL_Log("yZoomFrac: %0.3f, afterZoomMinZ: %0.3f\n",(double)state->ds.chartZoomStartMouseYFrac,(double)afterZoomMinZ);
        state->ds.timeSinceZoomStart = 0.0f;
        state->ds.zoomInProgress = 1;
        state->ds.zoomFinished = 0;
      }
      //SDL_Log("scale: %0.2f\n",(double)state->ds.chartZoomScale);
    }else if(state->uiState == UISTATE_FULLLEVELINFO){
      fcScrollAction(state,state->zoomDeltaVal*2.0f);
    }
    
  }

}

void processSingleEvent(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const SDL_Event evt){
  switch(evt.type){
    case SDL_EVENT_QUIT:
      state->quitAppFlag = 1; //quit game
      break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
      updateWindowRes(dat,state,rdat);
      break;
    case SDL_EVENT_GAMEPAD_ADDED:
      //setup the gamepad
      if(state->gamepadDisabled == 0){
        SDL_Log("Gamepad added.\n");
        rdat->gamepad = NULL;
        int num_joysticks;
        SDL_JoystickID *joysticks = SDL_GetJoysticks(&num_joysticks);
        for(int i=0; i<num_joysticks; i++){
          if(SDL_IsGamepad(joysticks[i])){
            rdat->gamepad = SDL_OpenGamepad(joysticks[i]);
            if(rdat->gamepad){
              break;
            }else{
              SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Could not open game gamepad %i: %s\n", i, SDL_GetError());
            }
          }
        }
      }
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      SDL_Log("Gamepad removed.\n");
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
      break;
    case SDL_EVENT_MOUSE_MOTION:
      SDL_GetMouseState(&state->mouseXPx,&state->mouseYPx); //update mouse position
      if(state->lastInputType != INPUT_TYPE_MOUSE){
        state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      }
      //SDL_Log("Mouse position: %i %i\n",*mouseX,*mouseY);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      switch(evt.button.button){
        case SDL_BUTTON_LEFT:
          //SDL_Log("Left mouse button down.\n");
          SDL_GetMouseState(&state->mouseXPx,&state->mouseYPx); //update mouse position
          state->mouseHoldStartPosXPx = state->mouseXPx;
          state->mouseHoldStartPosYPx = state->mouseYPx;
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      switch(evt.button.button){
        case SDL_BUTTON_LEFT:
          state->mouseHoldStartPosXPx = -1.0f;
          state->mouseHoldStartPosYPx = -1.0f;
          state->mouseClickPosXPx = state->mouseXPx;
          state->mouseClickPosYPx = state->mouseYPx;
          if(evt.button.clicks > 1){
            //SDL_Log("Double click.\n");
            state->inputFlags |= (1U << INPUT_DOUBLECLICK);
          }
          //SDL_Log("Left mouse button up.\n");
          break;
        case SDL_BUTTON_RIGHT:
          //SDL_Log("Right mouse button up.\n");
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      if(evt.wheel.direction == SDL_MOUSEWHEEL_NORMAL){
        if(evt.wheel.y > 0){
          //SDL_Log("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //SDL_Log("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }
      }else{
        if(evt.wheel.y > 0){
          //SDL_Log("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //SDL_Log("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }
      }
      break;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
      state->mouseoverElement = UIELEM_ENUM_LENGTH; //nothing moused over if cursor out of window
      state->mouseXPx = -1.0f;
      state->mouseYPx = -1.0f;
      //state->ds.forceRedraw = 1;
      break;
    
    case SDL_EVENT_KEY_DOWN: //pressing key
      state->lastInputType = INPUT_TYPE_KEYBOARD; //set keyboard input
      switch(evt.key.scancode){
        case SDL_SCANCODE_LEFT:
          state->inputFlags |= (1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_A:
          state->inputFlags |= (1U << INPUT_ALTLEFT);
          break;
        case SDL_SCANCODE_RIGHT:
          state->inputFlags |= (1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_D:
          state->inputFlags |= (1U << INPUT_ALTRIGHT);
          break;
        case SDL_SCANCODE_UP:
          state->inputFlags |= (1U << INPUT_UP);
          break;
        case SDL_SCANCODE_W:
          state->inputFlags |= (1U << INPUT_ALTUP);
          break;
        case SDL_SCANCODE_DOWN:
          state->inputFlags |= (1U << INPUT_DOWN);
          break;
        case SDL_SCANCODE_S:
          state->inputFlags |= (1U << INPUT_ALTDOWN);
          break;
        case SDL_SCANCODE_ESCAPE:
        case SDL_SCANCODE_BACKSPACE:
          state->inputFlags |= (1U << INPUT_BACK);
          break;
        case SDL_SCANCODE_EQUALS:
          state->zoomDeltaVal = 1.0f;
          state->inputFlags |= (1U << INPUT_ZOOM);
          break;
        case SDL_SCANCODE_MINUS:
          state->zoomDeltaVal = -1.0f;
          state->inputFlags |= (1U << INPUT_ZOOM);
          break;
        case SDL_SCANCODE_LEFTBRACKET:
          if(state->chartView == 0){
            state->chartView = (uint8_t)(CHARTVIEW_ENUM_LENGTH - 1);
          }else{
            state->chartView--;
          }
          break;
        case SDL_SCANCODE_RIGHTBRACKET:
          if(state->chartView == (uint8_t)(CHARTVIEW_ENUM_LENGTH - 1)){
            state->chartView = 0;
          }else{
            state->chartView++;
          }
          break;
        case SDL_SCANCODE_F7:
          //scale UI down
          if(state->ds.interfaceSizeInd > 0){
            state->ds.interfaceSizeInd--;
          }else{
            state->ds.interfaceSizeInd = UISCALE_ENUM_LENGTH - 1;
          }
          state->ds.uiUserScale = uiScales[state->ds.interfaceSizeInd];
          updateUIScale(dat,state,rdat);
          break;
        case SDL_SCANCODE_F8:
          //scale UI up
          if(state->ds.interfaceSizeInd < (UISCALE_ENUM_LENGTH - 1)){
            state->ds.interfaceSizeInd++;
          }else{
            state->ds.interfaceSizeInd = 0;
          }
          state->ds.uiUserScale = uiScales[state->ds.interfaceSizeInd];
          updateUIScale(dat,state,rdat);
          break;
        case SDL_SCANCODE_RETURN:
          if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)){
            uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
          }
          break;
        case SDL_SCANCODE_P:
          state->ds.drawPerformanceStats = !state->ds.drawPerformanceStats;
          break;
        case SDL_SCANCODE_F11:
          state->ds.windowFullscreenMode = !state->ds.windowFullscreenMode;
          handleScreenGraphicsMode(dat,state,rdat);
        default:
          break;
      }
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      break;
    case SDL_EVENT_KEY_UP: //released key
      switch(evt.key.scancode){
        case SDL_SCANCODE_LEFT:
          state->inputFlags &= ~(1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_A:
          state->inputFlags &= ~(1U << INPUT_ALTLEFT);
          break;
        case SDL_SCANCODE_RIGHT:
          state->inputFlags &= ~(1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_D:
          state->inputFlags &= ~(1U << INPUT_ALTRIGHT);
          break;
        case SDL_SCANCODE_UP:
          state->inputFlags &= ~(1U << INPUT_UP);
          break;
        case SDL_SCANCODE_W:
          state->inputFlags &= ~(1U << INPUT_ALTUP);
          break;
        case SDL_SCANCODE_DOWN:
          state->inputFlags &= ~(1U << INPUT_DOWN);
          break;
        case SDL_SCANCODE_S:
          state->inputFlags &= ~(1U << INPUT_ALTDOWN);
          break;
        default:
          break;
      }
      break;
    //analog stick control
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: //gamepad axis, use large values to account for deadzone
      //SDL_Log("axis: %i, val: %i, dz: %i\n",evt.gaxis.axis,evt.gaxis.value,state->gamepadDeadzone);
      //evtCnt++;
      switch(evt.gaxis.axis){
        case SDL_GAMEPAD_AXIS_LEFTX:
        case SDL_GAMEPAD_AXIS_RIGHTX:
          if(evt.gaxis.value > state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            SDL_Log("Gamepad right.\n");
            if(state->lastAxisValX==0){
              SDL_Log("Gamepad right once.\n");
            }
            state->lastAxisValX = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            SDL_Log("Gamepad left.\n");
            if(state->lastAxisValX==0){
              SDL_Log("Gamepad left once.\n");
            }
            state->lastAxisValX = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if((state->lastAxisValX!=0)&&(state->activeAxis==evt.gaxis.axis)){
            state->lastAxisValX = 0;
          }
          break;
        case SDL_GAMEPAD_AXIS_LEFTY:
        case SDL_GAMEPAD_AXIS_RIGHTY:
          if(evt.gaxis.value > state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            SDL_Log("Gamepad down.\n");
            if(state->lastAxisValY==0){
              SDL_Log("Gamepad down once.\n");
            }
            state->lastAxisValY = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            SDL_Log("Gamepad up.\n");
            if(state->lastAxisValY==0){
              SDL_Log("Gamepad up once.\n");
            }
            state->lastAxisValY = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if((state->lastAxisValY!=0)&&(state->activeAxis==evt.gaxis.axis)){
            state->lastAxisValY = 0;
          }
          break;
        default:
          break;
      }
      
      break;
    default:
      break;
  }
}

//called once per frame, processes user inputs and other SDL events
//takes input flags from the previous frame as an argument, returns flags for the current frame
void processFrameEvents(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

    //reset values
    state->mouseClickPosXPx = -1.0f;
    state->mouseClickPosYPx = -1.0f;
    state->inputFlags &= ~(1U << INPUT_ZOOM);
    state->inputFlags &= ~(1U << INPUT_BACK);
    if(state->inputFlags & (1U << INPUT_DOUBLECLICK)){
      state->inputFlags &= ~(1U << INPUT_DOUBLECLICK);
    }

    if((state->ds.uiAnimPlaying != 0)||(state->ds.zoomInProgress)||(state->ds.dragInProgress)||(state->ds.panInProgress)||(state->ds.fcScrollInProgress)||(state->ds.fcNuclChangeInProgress)){
      //a UI animation is playing, don't block the main thread
      state->ds.forceRedraw = 1;
    }

    //process events, update input state
    SDL_Event evt;
    if(state->ds.forceRedraw){
      //process all events without blocking
      //effectively forces a re-draw later on in the main loop
      //useful if eg. an animation is playing
      while(SDL_PollEvent(&evt)){
        processSingleEvent(dat,state,rdat,evt);
      }
      state->ds.forceRedraw = 0; //reset flag
    }else{
      //block, ie. wait for the first event to occur before doing anything (saves CPU)
      if(SDL_WaitEvent(&evt)){ 
        processSingleEvent(dat,state,rdat,evt);
        while(SDL_PollEvent(&evt)){
          processSingleEvent(dat,state,rdat,evt);
        }
      }
    }

    //process the results of input state
    processInputFlags(dat,state,rdat);

}

