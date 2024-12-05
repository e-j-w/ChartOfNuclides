/*
Chart of Nuclides - ENSDF data browser
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

#include "app.h"

void shutdownApp(const global_data *gdat, const uint8_t skipDealloc){
  updateConfigFile(gdat->rdat.appPrefPath,&gdat->dat.rules,&gdat->state); //save settings to disk
  if(!skipDealloc){
    for(uint8_t i=0; i<FONTSIZE_ENUM_LENGTH; i++){
      if(gdat->rdat.font[i]){
        FC_FreeFont(gdat->rdat.font[i]);
      }
    }
    if(gdat->rdat.fontData!=NULL){
      SDL_free(gdat->rdat.fontData);
    }
    SDL_DestroyTexture(gdat->rdat.uiThemeTex);
    SDL_DestroyRenderer(gdat->rdat.renderer);
    SDL_DestroyWindow(gdat->rdat.window);
  }
  TTF_Quit();
  //SDL_Quit(); //hangs on some systems?
  SDL_Log("SDL shutdown.\n");
  exit(0);
}

int main(int argc, char *argv[]){

  setlocale(LC_ALL, "en_ca.UTF-8");

  //parse command line arguments
  uint8_t cliArgs = 0;
  for(int i=1; i<argc; i++){
    if(strcmp(argv[i],"--nogamepad")==0){
      cliArgs |= (uint8_t)(1U<<CLI_NOGAMEPAD);
    }
  }
  
  /*for(i=0;i<SDL_GetNumVideoDrivers();i++){
    SDL_Log("Video driver available: %s\n",SDL_GetVideoDriver(i));
  }*/

  uint32_t sdlFlags = 0;
  if(cliArgs&(1U<<CLI_NOGAMEPAD)){
    sdlFlags = SDL_INIT_VIDEO;
  }else{
    sdlFlags = SDL_INIT_VIDEO|SDL_INIT_GAMEPAD;
  }
  if(strcmp(SDL_GetPlatform(),"Linux")==0){
    //Use Wayland by default on Linux instead of X11,
    //since Wayland seems to have better frame pacing,
    //and support for high resolution trackpads.
    SDL_SetHint(SDL_HINT_VIDEO_DRIVER,"wayland");
  }
  if(SDL_Init(sdlFlags) == 0){
    if(strcmp(SDL_GetPlatform(),"Linux")==0){
      SDL_ResetHint(SDL_HINT_VIDEO_DRIVER); //try X11 instead of wayland
      if(SDL_Init(sdlFlags) == 0){
        SDL_Log("Cannot initialize SDL: %s\n",SDL_GetError());
        return 0;
      }
    }else{
      SDL_Log("Cannot initialize SDL: %s\n",SDL_GetError());
      return 0;
    }
  }
  if(TTF_Init() == 0){
    SDL_Log("Cannot initialize SDL_TTF: %s\n",SDL_GetError());
    return 0;
  }
  SDL_Log("SDL startup.\n");

  //allocate structures
  global_data *gdat=(global_data*)SDL_calloc(1,sizeof(global_data));
  if(gdat==NULL){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"could not allocate application data structure.\n");
    return 0;
  }else{
    SDL_Log("Application data structure allocated.\n");
    SDL_Log("app_state     - allocated: %10li bytes\n",(long int)sizeof(app_state));
    SDL_Log("app_data      - allocated: %10li bytes\n",(long int)sizeof(app_data));
    SDL_Log("resource_data - allocated: %10li bytes\n",(long int)sizeof(resource_data));
  }

  //load preferences (needed prior to window created)
  gdat->rdat.appPrefPath = SDL_GetPrefPath(NULL,"con"); //setup path
  initializeTempState(&gdat->dat,&gdat->state,&gdat->tms);
  updatePrefsFromConfigFile(gdat->rdat.appPrefPath,&gdat->dat.rules,&gdat->state); //read settings from .ini file

  //setup the application window
  gdat->rdat.window = SDL_CreateWindow("Loading...",gdat->state.ds.windowXRes,gdat->state.ds.windowYRes,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if(!gdat->rdat.window){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"cannot create window - %s\n",SDL_GetError());
    SDL_Quit();
    return 0;
  }
  //setup the renderer
  gdat->rdat.renderer = SDL_CreateRenderer(gdat->rdat.window, NULL);
  if(!gdat->rdat.renderer){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Cannot initialize renderer.",gdat->rdat.window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"cannot create renderer - %s\n",SDL_GetError());
    SDL_DestroyWindow(gdat->rdat.window);
    SDL_Quit();
    return 0;
  }
  SDL_SetRenderTarget(gdat->rdat.renderer, NULL); //unset render to texture
  SDL_SetRenderDrawBlendMode(gdat->rdat.renderer,SDL_BLENDMODE_BLEND);
  SDL_SetWindowMinimumSize(gdat->rdat.window,MIN_RENDER_WIDTH,MIN_RENDER_HEIGHT);
  SDL_SetWindowMaximumSize(gdat->rdat.window,MAX_RENDER_WIDTH,MAX_RENDER_HEIGHT);
  //set UI scale
  int wwidth, wheight, rwidth, rheight;
  SDL_GetWindowSize(gdat->rdat.window, &wwidth, &wheight);
  SDL_GetWindowSizeInPixels(gdat->rdat.window, &rwidth, &rheight);
	gdat->rdat.uiDPIScale = (float)rwidth/((float)wwidth);
  updateUIScale(&gdat->dat,&gdat->state,&gdat->rdat);
  gdat->rdat.uiThemeScale = getUIthemeScale(gdat->rdat.uiScale);
  handleScreenGraphicsMode(&gdat->dat,&gdat->state,&gdat->rdat); //handle fullscreen
  gdat->state.ds.forceRedraw = 1; //force draw the first frame

  //cursors
  gdat->rdat.defaultCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
  gdat->rdat.dragCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
  gdat->rdat.textEntryCursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
  SDL_SetCursor(gdat->rdat.defaultCursor);
  
  //setup splash screen
  gdat->dat.rules.themeRules.bgCol.r = 255;
  gdat->dat.rules.themeRules.bgCol.g = 255;
  gdat->dat.rules.themeRules.bgCol.b = 255;
  gdat->dat.rules.themeRules.bgCol.a = 255;
  drawFlatBG(&gdat->state.ds,&gdat->rdat,gdat->dat.rules.themeRules.bgCol);
  SDL_RenderPresent(gdat->rdat.renderer); //tell the renderer to actually show the image

  //import game data and resources
  //initial text cache is also gneerated here
  if(importAppData(&gdat->dat,&gdat->rdat)!=0){
    shutdownApp(gdat,1);
  }
  SDL_SetWindowIcon(gdat->rdat.window,gdat->rdat.iconSurface);
  SDL_SetWindowTitle(gdat->rdat.window,gdat->dat.rules.appName);
  updateUIElemPositions(&gdat->dat,&gdat->state,&gdat->rdat); //some UI element positions depend on info only available after importAppData(), like font sizes

  //timing
  uint64_t timeNow = SDL_GetPerformanceCounter();
  uint64_t timeLast = timeNow;
  float deltaTime = 0.0f;
  float freqFac = 1000.0f/(float)SDL_GetPerformanceFrequency();

  //deal with effects of command line arguments
  gdat->state.gamepadDisabled = 0;
  if(cliArgs&(1U<<CLI_NOGAMEPAD)){
    SDL_Log("Disabling gamepad/gamepad from command line option.\n");
    gdat->state.gamepadDisabled = 1;
  }

  startUIAnimation(&gdat->dat,&gdat->state,UIANIM_CHART_FADEIN);
  
  //main loop
  while(!gdat->state.quitAppFlag){

    //timing
    timeLast = timeNow;
    timeNow = SDL_GetPerformanceCounter();
    deltaTime = (float)(timeNow - timeLast)*freqFac; //in milliseconds
    //SDL_Log("deltaTime: %f\n",deltaTime);
    deltaTime = deltaTime/1000.0f; //convert to seconds
    if(deltaTime > 0.033f){
      deltaTime = 0.001f; //because of main thread blocking, set artificially low delta to prevent weird timing bugs
    }

    updateThreads(&gdat->dat,&gdat->state,&gdat->rdat,&gdat->tms);

    updateUIAnimationTimes(&gdat->dat,&gdat->state,deltaTime);

    updateDrawingState(&gdat->dat,&gdat->state,&gdat->rdat,deltaTime);

    processFrameEvents(&gdat->dat,&gdat->state,&gdat->rdat); //can block the main thread to save CPU, see process_events.h
    
    if(gdat->state.searchStrUpdated){
      gdat->state.searchStrUpdated = 0;
      startSearchThreads(&gdat->dat,&gdat->state,&gdat->tms);
    }
    //SDL_RenderClear(gdat->rdat.renderer); //clear the window, disabled for optimization purposes

    drawUI(&gdat->dat,&gdat->state,&gdat->rdat); //gui.c

    if(gdat->state.ds.drawPerformanceStats == 1){
      drawPerformanceStats(&gdat->dat.rules.themeRules,&gdat->state,&gdat->tms,&gdat->rdat,deltaTime);
    }

    SDL_RenderPresent(gdat->rdat.renderer); //tell the renderer to actually show the image

  }
  
  //SDL_DestroyTexture(targetTex);
  shutdownApp(gdat,0);

  return 0;
}

