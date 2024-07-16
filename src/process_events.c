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
  //printf("scroll delta: %f\n",(double)deltaVal);
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
  //printf("scroll pos: %f, scroll to: %f\n",(double)state->ds.nuclFullInfoScrollY,(double)state->ds.nuclFullInfoScrollToY);
}

void processInputFlags(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){
  
  /* Handle directional input */
  
  if(state->uiState == UISTATE_DEFAULT){

    //in main chart view, handle chart panning
    uint32_t up = (state->inputFlags & (1U << INPUT_UP));
    uint32_t down = (state->inputFlags & (1U << INPUT_DOWN));
    uint32_t left = (state->inputFlags & (1U << INPUT_LEFT));
    uint32_t right = (state->inputFlags & (1U << INPUT_RIGHT));
    //printf("dir: [%u %u %u %u]\n",!(up==0),!(down==0),!(left==0),!(right==0));
    
    if(left || right || up || down){
      if(left && !right){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        state->ds.chartPanToX = state->ds.chartPosX - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToY = state->ds.chartPosY;
        }
        if(state->ds.chartPanToX <= 0.0f){
          state->ds.chartPanToX = 0.0f;
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
        if(state->ds.chartPanToX >= (dat->ndat.maxN+1)){
          state->ds.chartPanToX = dat->ndat.maxN+1.0f;
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
        if(state->ds.chartPanToY >= (dat->ndat.maxZ+1)){
          state->ds.chartPanToY = dat->ndat.maxZ+1.0f;
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
        //printf("pan start: [%0.2f %0.2f], pan to: [%0.2f %0.2f]\n",(double)state->ds.chartPanStartX,(double)state->ds.chartPanStartY,(double)state->ds.chartPanToX,(double)state->ds.chartPanToY);
      }else if(down && !up){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToX = state->ds.chartPosX;
        }
        state->ds.chartPanToY = state->ds.chartPosY - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.chartPanToY <= 0.0f){
          state->ds.chartPanToY = 0.0f;
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
      }
    }
  }else if(state->uiState == UISTATE_FULLLEVELINFO){
    uint32_t up = (state->inputFlags & (1U << INPUT_UP));
    uint32_t down = (state->inputFlags & (1U << INPUT_DOWN));

    if(up && !down){
      fcScrollAction(state,0.5f);
    }else if(down && !up){
      fcScrollAction(state,-0.5f);
    }

  }
  if(state->inputFlags & (1U << INPUT_BACK)){
    //escape open menus
    if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
      startUIAnimation(&state->ds,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
      startUIAnimation(&state->ds,UIANIM_NUCLHIGHLIGHT_HIDE);
    }else if((state->uiState == UISTATE_FULLLEVELINFO)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)){
      uiElemClickAction(dat,state,0,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON); //go back to the main chart
    }else if(state->ds.windowFullscreenMode){
      //exit fullscreen
      state->ds.windowFullscreenMode = 0;
      handleScreenGraphicsMode(dat,state,rdat); 
    }
  }

  /* Handle mouse input */

  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->mouseholdElement = UIELEM_ENUM_LENGTH;

  if(state->ds.dragInProgress == 0){
    for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){ //ordering in ui_element_enum defines order in which UI elements receive input
      if(state->interactableElement & (uint32_t)(1U << i)){
        if((state->mouseHoldStartPosXPx >= state->ds.uiElemPosX[i])&&(state->mouseHoldStartPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseHoldStartPosYPx >= state->ds.uiElemPosY[i])&&(state->mouseHoldStartPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
          state->mouseholdElement = i;
        }
        if((state->mouseXPx >= state->ds.uiElemPosX[i])&&(state->mouseXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseYPx >= state->ds.uiElemPosY[i])&&(state->mouseYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
          state->mouseoverElement = i;
          //printf("mouseover element: %u\n",i);
          if((state->mouseClickPosXPx >= state->ds.uiElemPosX[i])&&(state->mouseClickPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseClickPosYPx >= state->ds.uiElemPosY[i])&&(state->mouseClickPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
            //take action
            uiElemClickAction(dat,state,0,i);
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
      //printf("start drag\n");
    }
  }

  //check for mouse release
  if(state->mouseHoldStartPosXPx < 0.0f){
    state->ds.dragFinished = 1;
  }

  uint32_t doubleClick = (state->inputFlags & (1U << INPUT_DOUBLECLICK));

  //only get here if no button was clicked
  //check if a click was made outside of any button
  //printf("click pos x: %f, drag start: [%f %f]\n",(double)state->mouseClickPosXPx,(double)state->ds.chartDragStartMouseX,(double)state->ds.chartDragStartMouseY);
  if((state->mouseClickPosXPx >= 0.0f) && (fabsf(state->ds.chartDragStartMouseX - state->mouseXPx) < 5.0f) && (fabsf(state->ds.chartDragStartMouseY - state->mouseYPx) < 5.0f) ){
    //unclick (or click on chart view)
    if(doubleClick){
      uiElemClickAction(dat,state,1,UIELEM_ENUM_LENGTH);
    }else{
      uiElemClickAction(dat,state,0,UIELEM_ENUM_LENGTH);
    }
  }

  /* Handle zoom input */
  if((state->inputFlags & (1U << INPUT_ZOOM))&&(fabsf(state->zoomDeltaVal)>0.05f)){
    if(state->uiState == UISTATE_DEFAULT){
      if(state->zoomDeltaVal != 0.0f){
        state->ds.chartZoomStartScale = state->ds.chartZoomScale;
        if(state->zoomDeltaVal > 0){
          //zoom in
          state->ds.chartZoomToScale += state->zoomDeltaVal*state->ds.chartZoomToScale*0.5f;
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
            state->ds.chartZoomStartMouseY = state->ds.chartPosY;
          }
          if(state->ds.chartZoomStartMouseX > dat->ndat.maxN){
            state->ds.chartZoomStartMouseX = (float)dat->ndat.maxN;
          }else if(state->ds.chartZoomStartMouseX < 0.0f){
            state->ds.chartZoomStartMouseX = 0.0f;
          }
          if(state->ds.chartZoomStartMouseY > dat->ndat.maxZ){
            state->ds.chartZoomStartMouseY = (float)dat->ndat.maxZ;
          }else if(state->ds.chartZoomStartMouseY < 0.0f){
            state->ds.chartZoomStartMouseY = 0.0f;
          }
        }
        state->ds.chartZoomStartMouseXFrac = (state->ds.chartZoomStartMouseX - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
        float afterZoomMinN = state->ds.chartZoomStartMouseX - getChartWidthNAfterZoom(&state->ds)*state->ds.chartZoomStartMouseXFrac;
        state->ds.chartZoomToX = afterZoomMinN + getChartWidthNAfterZoom(&state->ds)*0.5f;
        state->ds.chartZoomStartMouseYFrac = (state->ds.chartZoomStartMouseY - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
        float afterZoomMinZ = state->ds.chartZoomStartMouseY - getChartHeightZAfterZoom(&state->ds)*state->ds.chartZoomStartMouseYFrac;
        state->ds.chartZoomToY = afterZoomMinZ + getChartHeightZAfterZoom(&state->ds)*0.5f;
        if(state->ds.chartZoomToX > dat->ndat.maxN){
          state->ds.chartZoomToX = (float)dat->ndat.maxN;
        }else if(state->ds.chartZoomToX < 0.0f){
          state->ds.chartZoomToX = 0.0f;
        }
        if(state->ds.chartZoomToY > dat->ndat.maxZ){
          state->ds.chartZoomToY = (float)dat->ndat.maxZ;
        }else if(state->ds.chartZoomToY < 0.0f){
          state->ds.chartZoomToY = 0.0f;
        }
        //printf("xZoomFrac: %0.3f, afterZoomMinN: %0.3f\n",(double)state->ds.chartZoomStartMouseXFrac,(double)afterZoomMinN);
        //printf("yZoomFrac: %0.3f, afterZoomMinZ: %0.3f\n",(double)state->ds.chartZoomStartMouseYFrac,(double)afterZoomMinZ);
        state->ds.timeSinceZoomStart = 0.0f;
        state->ds.zoomInProgress = 1;
        state->ds.zoomFinished = 0;
      }
      //printf("scale: %0.2f\n",(double)state->ds.chartZoomScale);
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
        printf("Gamepad added.\n");
        rdat->gamepad = NULL;
        int num_joysticks;
        SDL_JoystickID *joysticks = SDL_GetJoysticks(&num_joysticks);
        for(int i=0; i<num_joysticks; i++){
          if(SDL_IsGamepad(joysticks[i])){
            rdat->gamepad = SDL_OpenGamepad(joysticks[i]);
            if(rdat->gamepad){
              break;
            }else{
              printf("WARNING: Could not open game gamepad %i: %s\n", i, SDL_GetError());
            }
          }
        }
      }
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      printf("Gamepad removed.\n");
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
      break;
    case SDL_EVENT_MOUSE_MOTION:
      SDL_GetMouseState(&state->mouseXPx,&state->mouseYPx); //update mouse position
      if(state->lastInputType != INPUT_TYPE_MOUSE){
        state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      }
      //printf("Mouse position: %i %i\n",*mouseX,*mouseY);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      switch(evt.button.button){
        case SDL_BUTTON_LEFT:
          //printf("Left mouse button down.\n");
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
            //printf("Double click.\n");
            state->inputFlags |= (1U << INPUT_DOUBLECLICK);
          }
          //printf("Left mouse button up.\n");
          break;
        case SDL_BUTTON_RIGHT:
          //printf("Right mouse button up.\n");
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      if(evt.wheel.direction == SDL_MOUSEWHEEL_NORMAL){
        if(evt.wheel.y > 0){
          //printf("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //printf("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }
      }else{
        if(evt.wheel.y > 0){
          //printf("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //printf("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
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
      switch(evt.key.keysym.scancode){
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
          state->inputFlags |= (1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
          state->inputFlags |= (1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          state->inputFlags |= (1U << INPUT_UP);
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          state->inputFlags |= (1U << INPUT_DOWN);
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
        case SDL_SCANCODE_F:
          if((state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]==0.0f)){
            uiElemClickAction(dat,state,0,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
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
      switch(evt.key.keysym.scancode){
        case SDL_SCANCODE_LEFT:
        case SDL_SCANCODE_A:
          state->inputFlags &= ~(1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_RIGHT:
        case SDL_SCANCODE_D:
          state->inputFlags &= ~(1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_UP:
        case SDL_SCANCODE_W:
          state->inputFlags &= ~(1U << INPUT_UP);
          break;
        case SDL_SCANCODE_DOWN:
        case SDL_SCANCODE_S:
          state->inputFlags &= ~(1U << INPUT_DOWN);
          break;
        default:
          break;
      }
      break;
    //analog stick control
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: //gamepad axis, use large values to account for deadzone
      //printf("axis: %i, val: %i, dz: %i\n",evt.gaxis.axis,evt.gaxis.value,state->gamepadDeadzone);
      //evtCnt++;
      switch(evt.gaxis.axis){
        case SDL_GAMEPAD_AXIS_LEFTX:
        case SDL_GAMEPAD_AXIS_RIGHTX:
          if(evt.gaxis.value > state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            printf("Gamepad right.\n");
            if(state->lastAxisValX==0){
              printf("Gamepad right once.\n");
            }
            state->lastAxisValX = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            printf("Gamepad left.\n");
            if(state->lastAxisValX==0){
              printf("Gamepad left once.\n");
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
            printf("Gamepad down.\n");
            if(state->lastAxisValY==0){
              printf("Gamepad down once.\n");
            }
            state->lastAxisValY = evt.gaxis.value;
            state->activeAxis = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
            printf("Gamepad up.\n");
            if(state->lastAxisValY==0){
              printf("Gamepad up once.\n");
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

    if((state->ds.uiAnimPlaying != 0)||(state->ds.zoomInProgress)||(state->ds.dragInProgress)||(state->ds.panInProgress)||(state->ds.fcScrollInProgress)){
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

