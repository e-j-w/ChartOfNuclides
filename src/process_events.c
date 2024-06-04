/* © J. Williams 2017-2024 */
/* Functions handling user input processing */

#include "process_events.h"
#include "data_ops.h"
#include "gui_constants.h" //to compute mouse/pointer interactions

//leftButton: values from input_state_enum
void processMouse(app_state *restrict state){

  state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
  state->mouseholdElement = UIELEM_ENUM_LENGTH;

  if(state->ds.dragInProgress == 0){
    for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){
      if(state->interactableElement & (uint32_t)(1U << i)){
        if((state->mouseHoldStartPosXPx >= state->ds.uiElemPosX[i])&&(state->mouseHoldStartPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseHoldStartPosYPx >= state->ds.uiElemPosY[i])&&(state->mouseHoldStartPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
          state->mouseholdElement = i;
        }
        if((state->mouseXPx >= state->ds.uiElemPosX[i])&&(state->mouseXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseYPx >= state->ds.uiElemPosY[i])&&(state->mouseYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
          state->mouseoverElement = i;
          if((state->mouseClickPosXPx >= state->ds.uiElemPosX[i])&&(state->mouseClickPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]))&&(state->mouseClickPosYPx >= state->ds.uiElemPosY[i])&&(state->mouseClickPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]))){
            //take action
            uiElemClickAction(state,i);
            return;
          }
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

  //only get here if no button was clicked
  //check if a click was made outside of any button
  if(state->mouseClickPosXPx >= 0.0f){
    //unclick
    uiElemClickAction(state,UIELEM_ENUM_LENGTH);
  }

  if((state->mouseWheelUsed != 0)&&(fabsf(state->mouseWheelVal)>0.05f)){
    mouseWheelAction(state);
  }

}

void processSingleEvent(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const SDL_Event evt){
  switch(evt.type){
    case SDL_EVENT_QUIT:
      state->quitAppFlag = 1; //quit game
      break;
    case SDL_EVENT_WINDOW_RESIZED:
    case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
      updateWindowRes(dat,&state->ds,rdat);
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
      if(evt.wheel.direction == SDL_MOUSEWHEEL_NORMAL){
        if(evt.wheel.y > 0){
          //printf("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->mouseWheelVal = evt.wheel.y;
          state->mouseWheelUsed = 1;
        }else if(evt.wheel.y < 0){
          //printf("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->mouseWheelVal = evt.wheel.y;
          state->mouseWheelUsed = 1;
        }
      }else{
        if(evt.wheel.y > 0){
          //printf("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->mouseWheelVal = evt.wheel.y;
          state->mouseWheelUsed = 1;
        }else if(evt.wheel.y < 0){
          //printf("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->mouseWheelVal = evt.wheel.y;
          state->mouseWheelUsed = 1;
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
        case SDL_SCANCODE_P:
          state->ds.drawPerformanceStats = !state->ds.drawPerformanceStats;
          break;
        case SDL_SCANCODE_LEFT:
          if(state->uiState == UISTATE_DEFAULT){
            state->ds.chartPanStartX = state->ds.chartPosX;
            state->ds.chartPanStartY = state->ds.chartPosY;
            state->ds.chartPanToX = state->ds.chartPosX - (CHART_PAN_DIST/state->ds.chartZoomScale);
            state->ds.chartPanToY = state->ds.chartPosY;
            if(state->ds.chartPanToX <= 0.0f){
              state->ds.chartPanToX = 0.0f;
            }
            state->ds.timeSincePanStart = 0.0f;
            state->ds.panInProgress = 1;
            state->ds.panFinished = 0;
          }
          break;
        case SDL_SCANCODE_RIGHT:
          if(state->uiState == UISTATE_DEFAULT){
            state->ds.chartPanStartX = state->ds.chartPosX;
            state->ds.chartPanStartY = state->ds.chartPosY;
            state->ds.chartPanToX = state->ds.chartPosX + (CHART_PAN_DIST/state->ds.chartZoomScale);
            state->ds.chartPanToY = state->ds.chartPosY;
            if(state->ds.chartPanToX >= (dat->ndat.maxN+1)){
              state->ds.chartPanToX = dat->ndat.maxN+1.0f;
            }
            state->ds.timeSincePanStart = 0.0f;
            state->ds.panInProgress = 1;
            state->ds.panFinished = 0;
          }
          break;
        case SDL_SCANCODE_UP:
          if(state->uiState == UISTATE_DEFAULT){
            state->ds.chartPanStartX = state->ds.chartPosX;
            state->ds.chartPanStartY = state->ds.chartPosY;
            state->ds.chartPanToX = state->ds.chartPosX;
            state->ds.chartPanToY = state->ds.chartPosY + (CHART_PAN_DIST/state->ds.chartZoomScale);
            if(state->ds.chartPanToY >= (dat->ndat.maxZ+1)){
              state->ds.chartPanToY = dat->ndat.maxZ+1.0f;
            }
            state->ds.timeSincePanStart = 0.0f;
            state->ds.panInProgress = 1;
            state->ds.panFinished = 0;
            //printf("pan start: [%0.2f %0.2f], pan to: [%0.2f %0.2f]\n",(double)state->ds.chartPanStartX,(double)state->ds.chartPanStartY,(double)state->ds.chartPanToX,(double)state->ds.chartPanToY);
          }
          break;
        case SDL_SCANCODE_DOWN:
          if(state->uiState == UISTATE_DEFAULT){
            state->ds.chartPanStartX = state->ds.chartPosX;
            state->ds.chartPanStartY = state->ds.chartPosY;
            state->ds.chartPanToX = state->ds.chartPosX;
            state->ds.chartPanToY = state->ds.chartPosY - (CHART_PAN_DIST/state->ds.chartZoomScale);
            if(state->ds.chartPanToY <= 0.0f){
              state->ds.chartPanToY = 0.0f;
            }
            state->ds.timeSincePanStart = 0.0f;
            state->ds.panInProgress = 1;
            state->ds.panFinished = 0;
          }
          break;
        case SDL_SCANCODE_F11:
          state->ds.windowFullscreenMode = !state->ds.windowFullscreenMode;
          handleScreenGraphicsMode(dat,&state->ds,rdat);
        default:
          break;
      }
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
      break;
    case SDL_EVENT_KEY_UP: //released key
      state->ds.panFinished = 1;
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
    state->mouseWheelUsed = 0;

    if((state->ds.uiAnimPlaying != 0)||(state->ds.zoomInProgress)||(state->ds.dragInProgress)||(state->ds.panInProgress)){
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
    processMouse(state);

}

