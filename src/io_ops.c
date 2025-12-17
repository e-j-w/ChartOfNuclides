/*
Copyright (C) 2017-2025 J. Williams

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

/* Functions handling save/load and other file I/O operations */

#include "io_ops.h"
#include "data_ops.h"
#include "gui_constants.h"

//CONFIG FILE FUNCTIONS
//Functions handling reading/writing the .ini file containing game settings

//read data from a config file
static int readConfigFile(FILE *file, app_state *restrict state){
  char fullLine[256],par[256],val[256];
  char *tok;
  char *saveptr = NULL;

  while(!(feof(file))){ //go until the end of file is reached
    if(fgets(fullLine,256,file)!=NULL){

      fullLine[strcspn(fullLine, "\r\n")] = 0; //strips newline characters from the string read by fgets

      //parse the line
      tok = SDL_strtok_r(fullLine,"=",&saveptr);
      if(tok != NULL){
        SDL_strlcpy(par,tok,sizeof(par)-1);
        tok = SDL_strtok_r(NULL,"=",&saveptr);
        if(tok != NULL){
          SDL_strlcpy(val,tok,sizeof(val)-1);
        }
        
      }

      //SDL_Log("par: %s, val: %s\n",par,val);

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
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"invalid window x-resolution (%i) in config file, setting to default.\n",res);
          state->ds.windowXRes = MIN_RENDER_WIDTH;
        }
      }else if(strcmp(par,"window_res_y") == 0){
        int res = atoi(val);
        if(res>=MIN_RENDER_HEIGHT){
          state->ds.windowYRes = (uint16_t)res;
        }else{
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"invalid window y-resolution (%i) in config file, setting to default.\n",res);
          state->ds.windowYRes = MIN_RENDER_HEIGHT;
        }
      }else if(strcmp(par,"chart_pos_x") == 0){
        float posx = (float)atof(val);
        state->ds.chartPosX = posx;
      }else if(strcmp(par,"chart_pos_y") == 0){
        float posy = (float)atof(val);
        state->ds.chartPosY = posy;
      }else if(strcmp(par,"zoom_scale") == 0){
        float zs = (float)atof(val);
        if((zs>=MIN_CHART_ZOOM_SCALE)&&(zs<=MAX_CHART_ZOOM_SCALE)){
          state->ds.chartZoomScale = zs;
        }else{
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"invalid chart zoom scale (%f) in config file, setting to default.\n",(double)zs);
          state->ds.chartZoomScale = 1.0f;
        }
        state->ds.chartZoomToScale = state->ds.chartZoomScale;
	      state->ds.chartZoomStartScale = state->ds.chartZoomScale;
      }else if(strcmp(par,"shell_closures") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.drawShellClosures = 1;
        }else{
          state->ds.drawShellClosures = 0;
        }
      }else if(strcmp(par,"use_lifetimes") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.useLifetimes = 1;
        }else{
          state->ds.useLifetimes = 0;
        }
      }else if(strcmp(par,"chart_display_mode") == 0){
        int view = atoi(val);
        if((view >= 0)&&(view < CHARTVIEW_ENUM_LENGTH)){
          state->chartView = (uint8_t)view;
        }
      }else if(strcmp(par,"use_ui_animations") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.useUIAnimations = 1;
        }else{
          state->ds.useUIAnimations = 0;
        }
      }else if(strcmp(par,"use_level_list_separation_energies") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.useLevelListSeparationEnergies = 1;
        }else{
          state->ds.useLevelListSeparationEnergies = 0;
        }
      }else if(strcmp(par,"use_level_list_parent_thresholds") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.useLevelListParentThresholds = 1;
        }else{
          state->ds.useLevelListParentThresholds = 0;
        }
      }else if(strcmp(par,"use_level_list_comment_tooltips") == 0){
        if(strcmp(val,"yes") == 0){
          state->ds.useLevelListCommentTooltips = 1;
        }else{
          state->ds.useLevelListCommentTooltips = 0;
        }
      }else if(strcmp(par,"user_interface_size") == 0){
        int sval = atoi(val);
        if((sval >= 0)&&(sval < UISCALE_ENUM_LENGTH)){
          state->ds.interfaceSizeInd = (uint8_t)sval;
        }
      }else if(strcmp(par,"reaction_mode") == 0){
        int rval = atoi(val);
        if((rval >= 0)&&(rval < REACTIONMODE_ENUM_LENGTH)){
          state->ds.reactionModeInd = (uint8_t)rval;
        }
      }else if(strcmp(par,"gamepad_deadzone") == 0){
        int dz = atoi(val);
        if((dz > 1000)&&(dz < 32768)){
          state->gamepadDeadzone = (uint16_t)dz;
        }else{
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"invalid gamepad deadzone (%i) in config file, setting to default.\n",dz);
          state->gamepadDeadzone = 16000;
        }
      }else if(strcmp(par,"gamepad_trigger_deadzone") == 0){
        int dz = atoi(val);
        if((dz > 1000)&&(dz < 32768)){
          state->gamepadTriggerDeadzone = (uint16_t)dz;
        }else{
          SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"invalid gamepad deadzone (%i) in config file, setting to default.\n",dz);
          state->gamepadTriggerDeadzone = 4000;
        }
      }

    }
  }

  //clamp chart display range
	if(state->ds.chartPosX < (-0.25f*getChartWidthN(&state->ds))){
		state->ds.chartPosX = (-0.25f*getChartWidthN(&state->ds));
	}else if(state->ds.chartPosX > (MAX_NEUTRON_NUM+1)){
		state->ds.chartPosX = (float)MAX_NEUTRON_NUM+1.0f;
	}
	if(state->ds.chartPosY < (-0.25f*getChartHeightZ(&state->ds))){
		state->ds.chartPosY = (-0.25f*getChartHeightZ(&state->ds));
	}else if(state->ds.chartPosY > (MAX_PROTON_NUM+1)){
		state->ds.chartPosY = (float)MAX_PROTON_NUM+1.0f;
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
  fprintf(file,"window_res_x=%u\n",state->ds.windowXRes);
  fprintf(file,"window_res_y=%u\n",state->ds.windowYRes);

  fprintf(file,"\n### Display Settings ###\n");
  fprintf(file,"chart_pos_x=%0.3f\n",(double)state->ds.chartPosX);
  fprintf(file,"chart_pos_y=%0.3f\n",(double)state->ds.chartPosY);
  fprintf(file,"zoom_scale=%0.3f\n",(double)state->ds.chartZoomScale);
  if(state->ds.drawShellClosures){
    fprintf(file,"shell_closures=yes\n");
  }else{
    fprintf(file,"shell_closures=no\n");
  }
  if(state->ds.useLifetimes){
    fprintf(file,"use_lifetimes=yes\n");
  }else{
    fprintf(file,"use_lifetimes=no\n");
  }
  if(state->ds.useLevelListSeparationEnergies){
    fprintf(file,"use_level_list_separation_energies=yes\n");
  }else{
    fprintf(file,"use_level_list_separation_energies=no\n");
  }
  if(state->ds.useLevelListParentThresholds){
    fprintf(file,"use_level_list_parent_thresholds=yes\n");
  }else{
    fprintf(file,"use_level_list_parent_thresholds=no\n");
  }
  if(state->ds.useLevelListCommentTooltips){
    fprintf(file,"use_level_list_comment_tooltips=yes\n");
  }else{
    fprintf(file,"use_level_list_comment_tooltips=no\n");
  }
  fprintf(file,"chart_display_mode=%u\n",state->chartView);
  if(state->ds.useUIAnimations){
    fprintf(file,"use_ui_animations=yes\n");
  }else{
    fprintf(file,"use_ui_animations=no\n");
  }
  fprintf(file,"user_interface_size=%u\n",state->ds.interfaceSizeInd);
  fprintf(file,"reaction_mode=%u\n",state->ds.reactionModeInd);

  fprintf(file,"\n### Gamepad Settings ###\n");
  fprintf(file,"gamepad_deadzone=%i\n",state->gamepadDeadzone);
  fprintf(file,"gamepad_trigger_deadzone=%i\n",state->gamepadTriggerDeadzone);

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
  SDL_snprintf(configFilePath,512,"%schart.ini",configPath);
  FILE *configFile = fopen(configFilePath, "w");

  if(configFile != NULL){
    writeConfigFile(configFile,rules,state); //write the current configuration values
    fclose(configFile);
    SDL_Log("Wrote preferences to configuration file (%s).\n",configFilePath);
  }else{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Unable to write preferences to configuration file (%s)/.\n",configFilePath);
  }
}

void updatePrefsFromConfigFile(const char *configPath, const app_rules *restrict rules, app_state *restrict state){

  char configFilePath[512];
  SDL_snprintf(configFilePath,512,"%schart.ini",configPath);
  FILE *configFile = fopen(configFilePath, "r");

  if(configFile != NULL){
    readConfigFile(configFile,state); //read the configuration values
    fclose(configFile);
    SDL_Log("Preferences read from configuration file (%s).\n",configFilePath);
  }else{
    SDL_Log("No configuration file present, making a new one.\n");
    updateConfigFile(configPath,rules,state);
  }
}


