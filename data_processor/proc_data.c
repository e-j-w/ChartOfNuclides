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

#include "proc_data.h" //definitions and global variables

static void writeAsset(SDL_IOStream *out, const char *assetPath, const char *basePath){

  SDL_IOStream *asset;
  char filePath[270];
  
  SDL_snprintf(filePath,270,"%s%s",basePath,assetPath);
  asset = SDL_IOFromFile(filePath, "rb");
  if(asset!=NULL){
    int64_t fileSize = SDL_GetIOSize(asset);
    SDL_CloseIO(asset);
    if(fileSize>0){
      if(SDL_WriteIO(out,&fileSize,sizeof(int64_t))!=sizeof(int64_t)){
        SDL_Log("ERROR: writeAsset - couldn't write filesize of file %s to output file - %s.\n",filePath,SDL_GetError());
        exit(-1);
      }
      SDL_Log("   Writing asset: %s, %li bytes\n",filePath,(long int)fileSize);
      //allocate memory and read in data from the source asset file
      asset = SDL_IOFromFile(filePath, "rb");
      void *assetData=(void*)SDL_calloc(1,(size_t)fileSize);
      if(assetData==NULL){
        SDL_Log("ERROR: writeAsset - couldn't allocate memory.\n");
        exit(-1);
      }
      if(SDL_ReadIO(asset,assetData,(size_t)fileSize)!=(size_t)fileSize){
        SDL_Log("ERROR: writeAsset - couldn't read data from file %s - %s.\n",filePath,SDL_GetError());
        exit(-1);
      }
      SDL_CloseIO(asset);
      //write asset data
      if(SDL_WriteIO(out,assetData,(size_t)fileSize)!=(size_t)fileSize){
        SDL_Log("ERROR: writeAsset - couldn't write data to output file - %s.\n",SDL_GetError());
        exit(-1);
      }
      //clean up
      SDL_free(assetData);
    }else{
      SDL_Log("ERROR: writeAsset - couldn't determine length of file %s\n",filePath);
      exit(-1);
    }
  }else{
    SDL_Log("ERROR: writeAsset - couldn't open asset with file path %s\n",filePath);
    exit(-1);
  }

}


int main(int argc, char *argv[]){

  (void)argc; (void)argv; //required main function parameters are unused
  
  setlocale(LC_ALL, "en_ca.UTF-8");

  #ifdef __MINGW32__
  setbuf(stdout,NULL); //needed to show printf output on Windows
  #endif

  if(SDL_Init(0)==0){
    SDL_Log("Cannot initialize SDL: %s\n",SDL_GetError());
    return 0;
  }
  
  const char *appBasePath = SDL_GetBasePath();
  char fileName[512];
  SDL_snprintf(fileName,512,"%scon.dat",appBasePath);
  const char headerStr[6] = "<>|<>";
  const uint8_t version = 0; //revision of data format

  //parse data + metadata into an app_data struct
  app_data *dat=(app_data*)SDL_calloc(1,sizeof(app_data));
  if(parseAppData(dat,appBasePath)==-1){
    SDL_Log("ERROR: failed to parse app data.\n");
    SDL_free(dat);
    SDL_Quit();
    return 0;
  }

  SDL_Log("Writing assets to asset bundle...\n");
  SDL_IOStream *out = SDL_IOFromFile(fileName, "wb");
  
  if(out!=NULL){
    SDL_WriteIO(out,&headerStr[0],sizeof(headerStr)); //write header
    SDL_WriteIO(out,&version,sizeof(uint8_t));
    writeAsset(out,"data/io.github.e_j_w.ChartOfNuclides.svg",appBasePath);
    int64_t dataSize = (int64_t)sizeof(app_data);
    //SDL_Log("Data size: %li\n",dataSize);
    SDL_WriteIO(out,&dataSize,sizeof(int64_t));
    if(SDL_WriteIO(out,dat,sizeof(app_data))!=sizeof(app_data)){
      SDL_Log("ERROR: data write error - %s.\n",SDL_GetError());
      exit(-1);
    }
    writeAsset(out,"data/theme.svg",appBasePath);
    writeAsset(out,"data/font.ttf",appBasePath);
    SDL_WriteIO(out,&headerStr[0],sizeof(headerStr)); //write footer
  }else{
    SDL_Log("ERROR: cannot open output data file.\n");
    exit(-1);
  }

  SDL_Log("Asset writing complete.\n");
  
  if(SDL_CloseIO(out)==0){
    SDL_Log("ERROR: cannot close output data file - %s.\n",SDL_GetError());
  }

  SDL_Log("Bundled app data written to file: %s\n",fileName);

  SDL_free(dat);
  SDL_Quit();
  return 0;
}

