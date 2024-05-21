/* © J. Williams 2017-2024 */

#include "proc_data.h" //definitions and global variables

static void writeAsset(SDL_IOStream *out, const char *assetPath, const char *basePath){

  SDL_IOStream *asset;
  char filePath[270];
  
  snprintf(filePath,270,"%s%s",basePath,assetPath);
  asset = SDL_IOFromFile(filePath, "rb");
  if(asset!=NULL){
    int64_t fileSize = SDL_GetIOSize(asset);
    SDL_CloseIO(asset);
    if(fileSize>0){
      if(SDL_WriteIO(out,&fileSize,sizeof(int64_t))!=sizeof(int64_t)){
        printf("ERROR: writeAsset - couldn't write filesize of file %s to output file - %s.\n",filePath,SDL_GetError());
        exit(-1);
      }
      printf("   Writing asset: %s, %li bytes\n",filePath,(long int)fileSize);
      //allocate memory and read in data from the source asset file
      asset = SDL_IOFromFile(filePath, "rb");
      void *assetData=(void*)SDL_calloc(1,(size_t)fileSize);
      if(assetData==NULL){
        printf("ERROR: writeAsset - couldn't allocate memory.\n");
        exit(-1);
      }
      if(SDL_ReadIO(asset,assetData,(size_t)fileSize)!=(size_t)fileSize){
        printf("ERROR: writeAsset - couldn't read data from file %s - %s.\n",filePath,SDL_GetError());
        exit(-1);
      }
      SDL_CloseIO(asset);
      //write asset data
      if(SDL_WriteIO(out,assetData,(size_t)fileSize)!=(size_t)fileSize){
        printf("ERROR: writeAsset - couldn't write data to output file - %s.\n",SDL_GetError());
        exit(-1);
      }
      //clean up
      SDL_free(assetData);
    }else{
      printf("ERROR: writeAsset - couldn't determine length of file %s\n",filePath);
      exit(-1);
    }
  }else{
    printf("ERROR: writeAsset - couldn't open asset with file path %s\n",filePath);
    exit(-1);
  }

}


int main(int argc, char *argv[]){

  (void)argc; (void)argv; //required main function parameters are unused
  
  setlocale(LC_ALL, "en_ca.UTF-8");

  #ifdef __MINGW32__
  setbuf(stdout,NULL); //needed to show printf output on Windows
  #endif

  if(SDL_Init(SDL_INIT_VIDEO) != 0){
    printf("Cannot initialize SDL: %s\n",SDL_GetError());
    return 0;
  }

  const char fileName[16] = "con.dat";
  const char headerStr[6] = "<>|<>";
  const uint8_t version = 0; //revision of data format
  const char *appBasePath = SDL_GetBasePath();

  //parse data + metadata into an app_data struct
  app_data *dat=(app_data*)SDL_calloc(1,sizeof(app_data));
  if(parseAppData(dat,appBasePath)==-1){
    printf("ERROR: failed to parse app data.\n");
    SDL_free(dat);
    SDL_Quit();
    return 0;
  }

  printf("Writing assets to asset bundle...\n");
  SDL_IOStream *out = SDL_IOFromFile(fileName, "wb");
  
  if(out!=NULL){
    SDL_WriteIO(out,&headerStr[0],sizeof(headerStr)); //write header
    SDL_WriteIO(out,&version,sizeof(uint8_t));
    writeAsset(out,"data/icon.svg",appBasePath);
    int64_t dataSize = (int64_t)sizeof(app_data);
    //printf("Data size: %li\n",dataSize);
    SDL_WriteIO(out,&dataSize,sizeof(int64_t));
    if(SDL_WriteIO(out,dat,sizeof(app_data))!=sizeof(app_data)){
      printf("ERROR: data write error - %s.\n",SDL_GetError());
      exit(-1);
    }
    writeAsset(out,"data/theme.svg",appBasePath);
    writeAsset(out,"data/font.ttf",appBasePath);
    SDL_WriteIO(out,&headerStr[0],sizeof(headerStr)); //write footer
  }else{
    printf("ERROR: cannot open output data file.\n");
    exit(-1);
  }

  printf("Asset writing complete.\n");
  
  if(SDL_CloseIO(out)!=0){
    printf("ERROR: cannot close output data file - %s.\n",SDL_GetError());
  }

  printf("Bundled app data written to file: %s\n",fileName);

  SDL_free(dat);
  SDL_Quit();
  return 0;
}

