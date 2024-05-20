/* © J. Williams 2017-2024 */

#include "app.h"

void shutdownApp(const global_data *gdat, const uint8_t skipDealloc){
  updateConfigFile(gdat->rdat.appBasePath,&gdat->dat.rules,&gdat->state); //save settings to disk
  if(!skipDealloc){
    TTF_CloseFont(gdat->rdat.font);
    if(gdat->rdat.fontData!=NULL){
      SDL_free(gdat->rdat.fontData);
    }
    SDL_DestroyTexture(gdat->rdat.uiThemeTex);
    SDL_DestroyRenderer(gdat->rdat.renderer);
    SDL_DestroyWindow(gdat->rdat.window);
  }
  TTF_Quit();
  //SDL_Quit(); //hangs on some systems?
  printf("SDL shutdown.\n");
  exit(0);
}

int main(int argc, char *argv[]){

  setlocale(LC_ALL, "en_ca.UTF-8");

  #ifdef __MINGW32__
  setbuf(stdout,NULL); //needed to show printf output on Windows
  #endif

  //parse command line arguments
  uint8_t cliArgs = 0;
  for(int i=1; i<argc; i++){
    if(strcmp(argv[i],"--nogamepad")==0){
      cliArgs |= (uint8_t)(1U<<CLI_NOGAMEPAD);
    }
  }

  /*for(i=0;i<SDL_GetNumVideoDrivers();i++){
    printf("Video driver available: %s\n",SDL_GetVideoDriver(i));
  }*/

  uint32_t sdlFlags = 0;
  if(cliArgs&(1U<<CLI_NOGAMEPAD)){
    sdlFlags = SDL_INIT_VIDEO|SDL_INIT_TIMER;
  }else{
    sdlFlags = SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_GAMEPAD;
  }
  if(SDL_Init(sdlFlags) != 0){
    printf("Cannot initialize SDL: %s\n",SDL_GetError());
    return 0;
  }
  if(TTF_Init() != 0){
    printf("Cannot initialize SDL_TTF: %s\n", TTF_GetError());
    return 0;
  }
  printf("SDL startup.\n");

  //allocate structures
  global_data *gdat=(global_data*)SDL_calloc(1,sizeof(global_data));
  if(gdat==NULL){
    printf("ERROR: could not allocate application data structure.\n");
    return 0;
  }else{
    printf("Application data structure allocated.\n");
    printf("app_state     - allocated: %10li bytes\n",(long int)sizeof(app_state));
    printf("app_data      - allocated: %10li bytes\n",(long int)sizeof(app_data));
    printf("resource_data - allocated: %10li bytes\n",(long int)sizeof(resource_data));
  }

  //setup paths
  gdat->rdat.appBasePath = SDL_GetBasePath();

  //load preferences (needed prior to window created)
  gdat->state.gamepadDeadzone = 16000;
  gdat->state.ds.windowXRes = MIN_RENDER_WIDTH;
  gdat->state.ds.windowYRes = MIN_RENDER_HEIGHT;
  gdat->state.lastInputType = INPUT_TYPE_KEYBOARD;
  gdat->state.mouseoverElement = UIELEM_ENUM_LENGTH;
  gdat->state.ds.drawPerformanceStats = 0;

  initializeTempState(&gdat->state);
  updatePrefsFromConfigFile(gdat->rdat.appBasePath,&gdat->dat.rules,&gdat->state); //read settings from .ini file

  //setup the application window
  gdat->rdat.window = SDL_CreateWindow("Loading...",gdat->state.ds.windowXRes,gdat->state.ds.windowYRes,SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY);
  if(!gdat->rdat.window){
    printf("ERROR: cannot create window - %s\n",SDL_GetError());
    SDL_Quit();
    return 0;
  }
  //setup the renderer
  gdat->rdat.renderer = SDL_CreateRenderer(gdat->rdat.window, NULL, SDL_RENDERER_PRESENTVSYNC);
  if(!gdat->rdat.renderer){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Cannot initialize renderer.",gdat->rdat.window);
    printf("ERROR: cannot create renderer - %s\n",SDL_GetError());
    SDL_DestroyWindow(gdat->rdat.window);
    SDL_Quit();
    return 0;
  }
  SDL_SetRenderTarget(gdat->rdat.renderer, NULL); //unset render to texture
  SDL_SetRenderDrawBlendMode(gdat->rdat.renderer,SDL_BLENDMODE_BLEND);
  SDL_SetWindowMinimumSize(gdat->rdat.window,MIN_RENDER_WIDTH,MIN_RENDER_HEIGHT);
  SDL_SetWindowMaximumSize(gdat->rdat.window,MAX_RENDER_WIDTH,MAX_RENDER_HEIGHT);
  handleScreenGraphicsMode(&gdat->state.ds,&gdat->rdat); //handle fullscreen
  gdat->state.ds.forceRedraw = 1; //force draw the first frame
  
  //setup splash screen
  gdat->dat.rules.themeRules.whiteBGCol.r = 255;
  gdat->dat.rules.themeRules.whiteBGCol.g = 255;
  gdat->dat.rules.themeRules.whiteBGCol.b = 255;
  gdat->dat.rules.themeRules.whiteBGCol.a = 255;
  drawFlatBG(&gdat->state.ds,&gdat->rdat,gdat->dat.rules.themeRules.whiteBGCol);
  SDL_RenderPresent(gdat->rdat.renderer); //tell the renderer to actually show the image

  //import game data and resources
  if(importAppData(&gdat->dat,&gdat->rdat)!=0){
    shutdownApp(gdat,1);
  }
  SDL_SetWindowIcon(gdat->rdat.window,gdat->rdat.iconSurface);
  SDL_SetWindowTitle(gdat->rdat.window,gdat->dat.rules.appName);

  generateTextCache(&gdat->rdat); //drawing.c

  //timing
  uint64_t timeNow = SDL_GetPerformanceCounter();
  uint64_t timeLast = timeNow;
  float deltaTime = 0.0f;
  float freqFac = 1000.0f/(float)SDL_GetPerformanceFrequency();

  //deal with effects of command line arguments
  gdat->state.gamepadDisabled = 0;
  if(cliArgs&(1U<<CLI_NOGAMEPAD)){
    printf("Disabling gamepad/gamepad from command line option.\n");
    gdat->state.gamepadDisabled = 1;
  }
  
  //main loop
  while(!gdat->state.quitAppFlag){

    //timing
    timeLast = timeNow;
    timeNow = SDL_GetPerformanceCounter();
    deltaTime = (float)(timeNow - timeLast)*freqFac; //in milliseconds
    //printf("deltaTime: %f\n",deltaTime);
    deltaTime = deltaTime/1000.0f; //convert to seconds
    if(deltaTime > 0.033f){
      deltaTime = 0.033f; //because of main thread blocking, set maximum delta to prevent weird timing bugs
    }

    updateUIAnimationTimes(&gdat->state.ds,deltaTime);

    updateDrawingState(&gdat->state,deltaTime);

    processFrameEvents(&gdat->state,&gdat->rdat); //can block the main thread to save CPU, see process_events.h
    
    //SDL_RenderClear(gdat->rdat.renderer); //clear the window, disabled for optimization purposes

    //draw to the screen
    drawFlatBG(&gdat->state.ds,&gdat->rdat,gdat->dat.rules.themeRules.whiteBGCol);
    drawUI(&gdat->dat,&gdat->state,&gdat->rdat,deltaTime);

    if(gdat->state.ds.drawPerformanceStats == 1){
      drawPerformanceStats(&gdat->dat.rules.themeRules,&gdat->rdat,deltaTime);
    }

    SDL_RenderPresent(gdat->rdat.renderer); //tell the renderer to actually show the image

  }
  
  //SDL_DestroyTexture(targetTex);
  shutdownApp(gdat,0);

  return 0;
}

