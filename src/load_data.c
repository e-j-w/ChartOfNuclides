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

/* Functions used to load and process app data files such as maps, sprites, etc. */

#include <stdio.h>
#include "load_data.h"

//function to find and load the app data file, trying various platform-independent
//and platform-dependent locations
//inp is a double pointer as this function is expected to modify the value of the pointer *inp
int findAndLoadAppDataFile(SDL_IOStream **inp, resource_data *restrict rdat, const uint8_t useCached){
  if(!useCached){
    //no file path has been cached yet
    //try the user's local pref path first
    SDL_snprintf(rdat->appDataFilepath,270,"%scon.dat",rdat->appPrefPath);
  }
  *inp = SDL_IOFromFile(rdat->appDataFilepath, "rb");
  if(*inp==NULL){
    //try base path (path where the execuatable is)
    SDL_snprintf(rdat->appDataFilepath,270,"%scon.dat",SDL_GetBasePath());
    *inp = SDL_IOFromFile(rdat->appDataFilepath, "rb");
    if(*inp==NULL){
      //try platform specific paths
      const char *platformStr = SDL_GetPlatform();
      if(strcmp(platformStr,"Linux")==0){
        SDL_snprintf(rdat->appDataFilepath,270,"/usr/share/con/con.dat");
      }else{
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Could not find platform-specific location for app data file (con.dat).",rdat->window);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't determine platform specific data file location.\n");
        return -1;
      }
      *inp = SDL_IOFromFile(rdat->appDataFilepath, "rb");
      if(*inp==NULL){
        //check Flatpak data location
        SDL_snprintf(rdat->appDataFilepath,270,"/app/share/con.dat");
        *inp = SDL_IOFromFile(rdat->appDataFilepath, "rb");
        if(*inp==NULL){
          SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file (con.dat) doesn't exist or is unreadable.",rdat->window);
          SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read data package file %s.\n",rdat->appDataFilepath);
          return -1;
        }
      }
    }
  }
  SDL_Log("Opened app data file: %s\n",rdat->appDataFilepath);
  return 0;
}

//import all app data
int importAppData(app_data *restrict dat, resource_data *restrict rdat){

  SDL_IOStream *inp = NULL;
  char readStr[6];
  uint8_t version = 255;
  int64_t fileSize;
  size_t totalAlloc = 0;
  rdat->themeOffset = 0;
  
  if(findAndLoadAppDataFile(&inp,rdat,0)==-1){
    return -1;
  }
  //read header
  if(SDL_ReadIO(inp,readStr,sizeof(readStr))!=sizeof(readStr)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable header in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read header from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  readStr[5]='\0';
  if(strcmp(readStr,"<>|<>")!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid header in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - bad header in data file %s (%s)\n",rdat->appDataFilepath,readStr);
    return -1;
  }
  //read version number
  SDL_ReadIO(inp,&version,sizeof(uint8_t));
  if(version!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid app data file version.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid data file version (%u).\n",version);
    return -1;
  }

  //load application icon data
  fileSize=0;
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable application icon in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid application icon filesize (%li) from file %s - %s.\n",(long int)fileSize,rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  void *icoData=(void*)SDL_calloc(1,(size_t)fileSize);
  totalAlloc += (size_t)fileSize;
  if(icoData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't allocate memory for application icon data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,icoData,(size_t)fileSize)!=(size_t)fileSize){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable application icon in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read application icon data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  rdat->iconSurface = IMG_LoadSizedSVG_IO(SDL_IOFromConstMem(icoData,(size_t)fileSize),(int)(128.0f*rdat->uiScale),(int)(128.0f*rdat->uiScale));
  if(rdat->iconSurface == NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid application icon in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read application icon data - %s.\n",SDL_GetError());
    return -1;
  }
  free(icoData);
  SDL_SetSurfaceRLE(rdat->iconSurface, 1); //enable RLE acceleration

  //load app_data
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize!=(int64_t)sizeof(app_data))){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid app_data size (%li) from file %s - %s.\n",(long int)fileSize,rdat->appDataFilepath,SDL_GetError());
    SDL_Log("Expected: %lu\n",(long unsigned int)sizeof(app_data));
    return -1;
  }
  if(SDL_ReadIO(inp,dat,sizeof(app_data))!=sizeof(app_data)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not read data bank.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read app data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  
  //load UI theme texture data
  rdat->themeOffset = (size_t)SDL_TellIO(inp);
  fileSize=0;
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid texture atlas filesize (%li) from file %s.\n",(long int)fileSize,rdat->appDataFilepath);
    return -1;
  }
  void *themeData=(void*)SDL_calloc(1,(size_t)fileSize);
  totalAlloc += (size_t)fileSize;
  if(themeData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't allocate memory for UI theme texture data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,themeData,(size_t)fileSize)!=(size_t)fileSize){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read UI theme texture data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  SDL_Surface *surface = IMG_LoadSizedSVG_IO(SDL_IOFromConstMem(themeData,(size_t)fileSize),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_X*rdat->uiThemeScale),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_Y*rdat->uiThemeScale));
  if(surface == NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't UI theme texture atlas data - %s.\n",SDL_GetError());
    return -1;
  }
  free(themeData);
  SDL_SetSurfaceRLE(surface, 1); //enable RLE acceleration
  //upload texture atlas into GPU memory
  rdat->uiThemeTex = SDL_CreateTextureFromSurface(rdat->renderer,surface);
  SDL_SetTextureScaleMode(rdat->uiThemeTex,SDL_SCALEMODE_LINEAR);
  SDL_DestroySurface(surface);
  
  //load font
  int64_t fontFilesize=0;
  if((SDL_ReadIO(inp,&fontFilesize,sizeof(int64_t))!=sizeof(int64_t))||(fontFilesize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid font filesize (%li) from file %s - %s.\n",(long int)fontFilesize,rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  rdat->fontData=(void*)SDL_calloc(1,(size_t)fontFilesize);
  totalAlloc += (size_t)fontFilesize;
  if(rdat->fontData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't allocate memory for font data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,rdat->fontData,(size_t)fontFilesize)!=(size_t)fontFilesize){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read font data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }

  for(uint8_t i=0; i<FONTSIZE_ENUM_LENGTH; i++){
    rdat->font[i] = FC_CreateFont();
    FC_LoadFont_RW(rdat->font[i], rdat->renderer,SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(Uint32)(fontSizes[i]*rdat->uiScale),whiteCol8Bit,TTF_STYLE_NORMAL);
    if(rdat->font[i]==NULL){
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - unable to load resource.",rdat->window);
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read font resource %u - %s.\n",i,SDL_GetError());
      return -1;
    }
  }

  //read footer
  if(SDL_ReadIO(inp,readStr,sizeof(readStr))!=sizeof(readStr)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Unreadable footer in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read footer from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  readStr[5]='\0';
  if(strcmp(readStr,"<>|<>")!=0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","Invalid footer in app data file.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - bad footer in data file %s (%s)\n",rdat->appDataFilepath,readStr);
    return -1;
  }
  
  if(SDL_CloseIO(inp)==0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - failed to close data file %s\n",rdat->appDataFilepath);
    return -1;
  }

  SDL_Log("Data import complete - allocated %li bytes.\n",(long int)totalAlloc);

  //getc(stdin);
  return 0; //success
  
}

//similar to importAppData, but only handles loading and rescaling the font data
int regenerateThemeAndFontCache(app_data *restrict dat, resource_data *restrict rdat){
  
  if(rdat->themeOffset <= 0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - attempting to regenerate cache when app data was not previously imported. Doing that now...\n");
    return importAppData(dat,rdat);
  }

  SDL_IOStream *inp = NULL;
  if(findAndLoadAppDataFile(&inp,rdat,1)==-1){
    return -1;
  }
  if(SDL_SeekIO(inp,(Sint64)rdat->themeOffset,SDL_IO_SEEK_SET)<0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - couldn't seek to theme data in file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }

  //free previously used resources
  if(rdat->uiThemeTex){
    SDL_DestroyTexture(rdat->uiThemeTex);
  }
  for(uint8_t i=0; i<FONTSIZE_ENUM_LENGTH; i++){
    if(rdat->font[i]){
      FC_FreeFont(rdat->font[i]);
    }
  }

  //read theme data
  rdat->themeOffset = (size_t)SDL_TellIO(inp);
  int64_t fileSize=0;
  if((SDL_ReadIO(inp,&fileSize,sizeof(int64_t))!=sizeof(int64_t))||(fileSize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - invalid texture atlas filesize (%li) from file %s.\n",(long int)fileSize,rdat->appDataFilepath);
    return -1;
  }
  void *themeData=(void*)SDL_calloc(1,(size_t)fileSize);
  if(themeData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't allocate memory for UI theme texture data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,themeData,(size_t)fileSize)!=(size_t)fileSize){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read UI theme texture data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  SDL_Surface *surface = IMG_LoadSizedSVG_IO(SDL_IOFromConstMem(themeData,(size_t)fileSize),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_X*rdat->uiThemeScale),(int)(UI_TILE_SIZE*UI_THEME_TEX_TILES_Y*rdat->uiThemeScale));
  if(surface == NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't UI theme texture atlas data - %s.\n",SDL_GetError());
    return -1;
  }
  free(themeData);
  SDL_SetSurfaceRLE(surface, 1); //enable RLE acceleration
  //upload texture atlas into GPU memory
  rdat->uiThemeTex = SDL_CreateTextureFromSurface(rdat->renderer,surface);
  SDL_SetTextureScaleMode(rdat->uiThemeTex,SDL_SCALEMODE_LINEAR);
  SDL_DestroySurface(surface);

  //load font
  int64_t fontFilesize=0;
  if((SDL_ReadIO(inp,&fontFilesize,sizeof(int64_t))!=sizeof(int64_t))||(fontFilesize<=0)){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - invalid data size.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - invalid font filesize (%li) from file %s - %s.\n",(long int)fontFilesize,rdat->appDataFilepath,SDL_GetError());
    return -1;
  }
  rdat->fontData=(void*)SDL_calloc(1,(size_t)fontFilesize);
  if(rdat->fontData==NULL){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - could not allocate memory.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - couldn't allocate memory for font data.\n");
    exit(-1);
  }
  if(SDL_ReadIO(inp,rdat->fontData,(size_t)fontFilesize)!=(size_t)fontFilesize){
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - couldn't read font data from file %s - %s.\n",rdat->appDataFilepath,SDL_GetError());
    return -1;
  }

//
  for(uint8_t i=0; i<FONTSIZE_ENUM_LENGTH; i++){
    rdat->font[i] = FC_CreateFont();
    FC_LoadFont_RW(rdat->font[i], rdat->renderer,SDL_IOFromConstMem(rdat->fontData,(size_t)fontFilesize),1,(Uint32)(fontSizes[i]*rdat->uiScale),whiteCol8Bit,TTF_STYLE_NORMAL);
    if(rdat->font[i]==NULL){
      SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file read error - unable to load resource.",rdat->window);
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"importAppData - couldn't read font resource %u - %s.\n",i,SDL_GetError());
      return -1;
    }
  }

  if(SDL_CloseIO(inp)==0){
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","App data file I/O error.",rdat->window);
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"regenerateFontCache - failed to close data file %s\n",rdat->appDataFilepath);
    return -1;
  }
  return 0; //success

}

