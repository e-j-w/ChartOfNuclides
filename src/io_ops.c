/* © J. Williams 2017-2024 */
/* Functions handling save/load and other file I/O operations */

#include "io_ops.h"
#include "data_ops.h"

//CONFIG FILE FUNCTIONS
//Functions handling reading/writing the .ini file containing game settings

//read data from a config file
static int readConfigFile(FILE *file, app_state *restrict state){
  char fullLine[256],par[256],val[256];
  char *tok;

  while(!(feof(file))){ //go until the end of file is reached
    if(fgets(fullLine,256,file)!=NULL){

      fullLine[strcspn(fullLine, "\r\n")] = 0; //strips newline characters from the string read by fgets

      //parse the line
      tok = strtok(fullLine,"=");
      if(tok != NULL){
        strncpy(par,tok,sizeof(par)-1);
        tok = strtok(NULL,"=");
        if(tok != NULL){
          strncpy(val,tok,sizeof(val)-1);
        }
        
      }

      //printf("par: %s, val: %s\n",par,val);

      //read in parameter values
      if(strcmp(par,"performance_overlay") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.drawPerformanceStats = 1;
        }else{
          state->ds.drawPerformanceStats = 0;
        }
      }else if(strcmp(par,"fullscreen") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.windowFullscreenMode = 1;
        }else{
          state->ds.windowFullscreenMode = 0;
        }
      }else if(strcmp(par,"window_res_x") == 0){
        int res = atoi(val);
        if(res>=MIN_RENDER_WIDTH){
          state->ds.windowXRes = (uint16_t)res;
        }else{
          printf("WARNING: invalid window x-resolution (%i) in config file, setting to default.\n",res);
          state->ds.windowXRes = MIN_RENDER_WIDTH;
        }
      }else if(strcmp(par,"window_res_y") == 0){
        int res = atoi(val);
        if(res>=MIN_RENDER_HEIGHT){
          state->ds.windowYRes = (uint16_t)res;
        }else{
          printf("WARNING: invalid window y-resolution (%i) in config file, setting to default.\n",res);
          state->ds.windowYRes = MIN_RENDER_HEIGHT;
        }
      }else if(strcmp(par,"gamepad_deadzone") == 0){
        int dz = atoi(val);
        if((dz > 1000)&&(dz < 32768)){
          state->gamepadDeadzone = (uint16_t)dz;
        }else{
          printf("WARNING: invalid gamepad deadzone (%i) in config file, setting to default.\n",dz);
          state->gamepadDeadzone = 16000;
        }
      }

    }
  }

  return 1;
}

//write current settings to a config file
static int writeConfigFile(FILE *file, const app_rules *restrict rules, const app_state *restrict state){

  fprintf(file,"### %s Configuration File ###\n",rules->appName);

  fprintf(file,"\n### Video Settings ###\n");
  if(state->ds.windowFullscreenMode){
    fprintf(file,"fullscreen=yes\n");
  }else{
    fprintf(file,"fullscreen=no\n");
  }
  //fprintf(file,"# Windowed resolution: 0 = 1280 x 720, 1 = 1920 x 1080\n");
  fprintf(file,"window_res_x=%i\n",state->ds.windowXRes);
  fprintf(file,"window_res_y=%i\n",state->ds.windowYRes);

  fprintf(file,"\n### Gamepad Settings ###\n");
  fprintf(file,"gamepad_deadzone=%i\n",state->gamepadDeadzone);

  fprintf(file,"\n### Other Settings ###\n");
  if(state->ds.drawPerformanceStats == 1){
    fprintf(file,"performance_overlay=yes\n");
  }else{
    fprintf(file,"performance_overlay=no\n");
  }

  return 1;
}


void updateConfigFile(const char *configPath, const app_rules *restrict rules, const app_state *restrict state){

  char configFilePath[512];
  snprintf(configFilePath,512,"%scon.ini",configPath);
  FILE *configFile = fopen(configFilePath, "w");

  if(configFile != NULL){
    writeConfigFile(configFile,rules,state); //write the default configuration values
    fclose(configFile);
    printf("Wrote preferences to configuration file (%s).\n",configFilePath);
  }else{
    printf("WARNING: Unable to write preferences to configuration file (%s)/.\n",configFilePath);
  }
}

void updatePrefsFromConfigFile(const char *configPath, const app_rules *restrict rules, app_state *restrict state){

  char configFilePath[512];
  snprintf(configFilePath,512,"%scon.ini",configPath);
  FILE *configFile = fopen(configFilePath, "r");

  if(configFile != NULL){
    readConfigFile(configFile,state); //read the configuration values
    fclose(configFile);
    printf("Preferences read from configuration file (%s).\n",configFilePath);
  }else{
    printf("No configuration file present, making a new one.\n");
    updateConfigFile(configPath,rules,state);
  }
}


