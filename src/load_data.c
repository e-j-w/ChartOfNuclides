/* © J. Williams 2017-2024 */
/* Functions used to load and process app data files such as maps, sprites, etc. */

#include <stdio.h>
#include "load_data.h"

//import all app data
int importAppData(app_data *restrict dat, resource_data *restrict rdat){

  char filePath[270], readStr[6];
  uint8_t version = 255;
  int64_t fileSize;
  size_t totalAlloc = 0;
  
  snprintf(filePath,270,"%scon.dat",rdat->appBasePath);
  SDL_IOStream *inp = SDL_IOFromFile(filePath, "rb");
  if(inp==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file (con.dat) doesn't exist or is unreadable.",rdat->window);
    printf("ERROR: importAppData - couldn't read data package file %s.\n",filePath);
    return -1;
  }
  //read header
  if(SDL_ReadIO(inp,readStr,sizeof(readStr))!=sizeof(readStr)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable header in app data file.",rdat->window);
    printf("ERROR: importAppData - couldn't read header from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  readStr[5]='\0';
  if(strcmp(readStr,"<>|<>")!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid header in app data file.",rdat->window);
    printf("ERROR: importAppData - bad header in data file %s (%s)\n",filePath,readStr);
    return -1;
  }
  //read version number
  SDL_ReadIO(inp,&version,sizeof(uint8_t));
  if(version!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid app data file version.",rdat->window);
    printf("ERROR: importAppData - invalid data file version (%u).\n",version);
    return -1;
  }

  //load application icon data
  fileSize=0;
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable application icon in app data file.",rdat->window);
    printf("ERROR: importAppData - invalid application icon filesize (%li) from file %s - %s.\n",(long int)fileSize,filePath,SDL_GetError());
    return -1;
  }
  void *icoData=(void*)SDL_calloc(1,(size_t)fileSize);
  totalAlloc += (size_t)fileSize;
  if(icoData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    printf("ERROR: importAppData - couldn't allocate memory for application icon data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,icoData,(size_t)fileSize)!=(size_t)fileSize){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable application icon in app data file.",rdat->window);
    printf("ERROR: importAppData - couldn't read application icon data from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  rdat->iconSurface = IMG_LoadSizedSVG_IO(SDL_IOFromConstMem(icoData,(size_t)fileSize),(int)(128.0f*rdat->uiScale),(int)(128.0f*rdat->uiScale));
  if(rdat->iconSurface == NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid application icon in app data file.",rdat->window);
    printf("ERROR: importAppData - couldn't read application icon data - %s.\n",SDL_GetError());
    return -1;
  }
  free(icoData);
  SDL_SetSurfaceRLE(rdat->iconSurface, 1); //enable RLE acceleration

  //load app_data
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize!=(int64_t)sizeof(app_data))){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    printf("ERROR: importAppData - invalid app_data size (%li) from file %s - %s.\n",(long int)fileSize,filePath,SDL_GetError());
    printf("Expected: %lu\n",(long unsigned int)sizeof(app_data));
    return -1;
  }
  if(SDL_ReadIO(inp,dat,sizeof(app_data))!=sizeof(app_data)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not read data bank.",rdat->window);
    printf("ERROR: importAppData - couldn't read app data from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  
  //load UI theme texture data
  fileSize=0;
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    printf("ERROR: importAppData - invalid texture atlas filesize (%li) from file %s.\n",(long int)fileSize,filePath);
    return -1;
  }
  void *themeData=(void*)SDL_calloc(1,(size_t)fileSize);
  totalAlloc += (size_t)fileSize;
  if(themeData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    printf("ERROR: importAppData - couldn't allocate memory for UI theme texture data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,themeData,(size_t)fileSize)!=(size_t)fileSize){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    printf("ERROR: importAppData - couldn't read UI theme texture data from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  SDL_Surface *surface = IMG_LoadSizedSVG_IO(SDL_IOFromConstMem(themeData,(size_t)fileSize),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_X*rdat->uiScale),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_Y*rdat->uiScale));
  if(surface == NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    printf("ERROR: importAppData - couldn't UI theme texture atlas data - %s.\n",SDL_GetError());
    return -1;
  }
  free(themeData);
  SDL_SetSurfaceRLE(surface, 1); //enable RLE acceleration
  //upload texture atlas into GPU memory
  rdat->uiThemeTex = SDL_CreateTextureFromSurface(rdat->renderer,surface);
  SDL_DestroySurface(surface);
  
  //load font
  int64_t fontFilesize;
  fontFilesize=0;
  if((SDL_ReadIO(inp,&fontFilesize,sizeof(int64_t))!=sizeof(int64_t))||(fontFilesize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    printf("ERROR: importAppData - invalid font filesize (%li) from file %s - %s.\n",(long int)fontFilesize,filePath,SDL_GetError());
    return -1;
  }
  rdat->fontData=(void*)SDL_calloc(1,(size_t)fontFilesize);
  totalAlloc += (size_t)fontFilesize;
  if(rdat->fontData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    printf("ERROR: importAppData - couldn't allocate memory for font data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,rdat->fontData,(size_t)fontFilesize)!=(size_t)fontFilesize){
    printf("ERROR: importAppData - couldn't read font data from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  
  //cache font glyphs in 3 sizes, to prevent having to dynamically scale them,
  //which would wreck performance
  rdat->smallFont = TTF_OpenFontIO(SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(int)(SMALL_FONT_SIZE*rdat->uiScale));
  rdat->font = TTF_OpenFontIO(SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(int)(DEFAULT_FONT_SIZE*rdat->uiScale));
  rdat->bigFont = TTF_OpenFontIO(SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(int)(BIG_FONT_SIZE*rdat->uiScale));
  rdat->hugeFont = TTF_OpenFontIO(SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(int)(HUGE_FONT_SIZE*rdat->uiScale));
  if(rdat->font==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - unable to load resource.",rdat->window);
    printf("ERROR: importAppData - couldn't read font resource - %s.\n",SDL_GetError());
    return -1;
  }

  //read footer
  if(SDL_ReadIO(inp,readStr,sizeof(readStr))!=sizeof(readStr)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable footer in app data file.",rdat->window);
    printf("ERROR: importAppData - couldn't read footer from file %s - %s.\n",filePath,SDL_GetError());
    return -1;
  }
  readStr[5]='\0';
  if(strcmp(readStr,"<>|<>")!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid footer in app data file.",rdat->window);
    printf("ERROR: importAppData - bad footer in data file %s (%s)\n",filePath,readStr);
    return -1;
  }
  
  if(SDL_CloseIO(inp)!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    printf("ERROR: importAppData - failed to close data file %s\n",filePath);
    return -1;
  }

  printf("Data import complete - allocated %li bytes.\n",(long int)totalAlloc);

  //getc(stdin);
  return 0; //success
  
}

