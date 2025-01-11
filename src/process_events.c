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

/* Functions handling user input processing */

#include "process_events.h"
#include "data_ops.h"
#include "drawing.h"
#include "strops.h" //for search query editing
#include "gui_constants.h" //to compute mouse/pointer interactions

void fcScrollAction(app_state *restrict state, const float deltaVal){
  //SDL_Log("scroll delta: %f\n",(double)deltaVal);
  const float screenNumLines = (float)(getNumScreenLvlDispLines(&state->ds));
  state->ds.nuclFullInfoScrollStartY = state->ds.nuclFullInfoScrollY;
  state->ds.nuclFullInfoScrollToY = state->ds.nuclFullInfoScrollY - 0.25f*state->scrollSpeedMultiplier*screenNumLines*deltaVal;
  if(state->ds.nuclFullInfoScrollToY < 0.0f){
    state->ds.nuclFullInfoScrollToY = 0.0f;
  }else if(state->ds.nuclFullInfoScrollToY > state->ds.nuclFullInfoMaxScrollY){
    state->ds.nuclFullInfoScrollToY = (float)state->ds.nuclFullInfoMaxScrollY;
  }
  state->ds.timeSinceFCScollStart = 0.0f;
  state->ds.fcScrollInProgress = 1;
  state->ds.fcScrollFinished = 0;
  //SDL_Log("scroll pos: %f, scroll to: %f\n",(double)state->ds.nuclFullInfoScrollY,(double)state->ds.nuclFullInfoScrollToY);
}

void processInputFlags(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  /* Handle directional input */
  uint32_t up,down,left,right,altup,altdown,altleft,altright;
  up = (state->inputFlags & (1U << INPUT_UP));
  down = (state->inputFlags & (1U << INPUT_DOWN));
  left = (state->inputFlags & (1U << INPUT_LEFT));
  right = (state->inputFlags & (1U << INPUT_RIGHT));
  altup = (state->inputFlags & (1U << INPUT_ALTUP));
  altdown = (state->inputFlags & (1U << INPUT_ALTDOWN));
  altleft = (state->inputFlags & (1U << INPUT_ALTLEFT));
  altright = (state->inputFlags & (1U << INPUT_ALTRIGHT));
  
  const uint8_t chartDraggable = ((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX));
  const uint8_t chartPannable =  ((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX));

  if((SDL_TextInputActive(rdat->window))&&(strlen(state->ss.searchString) > 0)){
    //scroll between search results
    if(state->ss.numResults > 0){
      if(up && !down){
        if((state->ss.numResults > 1)&&(state->mouseoverElement >= UIELEM_SEARCH_RESULT)&&(state->mouseoverElement <= UIELEM_SEARCH_RESULT_4)){
          if(state->mouseoverElement <= UIELEM_SEARCH_RESULT){
            state->mouseoverElement = (uint8_t)(UIELEM_SEARCH_RESULT + state->ss.numResults - 1);
          }else{
            state->mouseoverElement--;
          }
        }else{
          if(state->mouseoverElement != UIELEM_SEARCH_RESULT){
            state->mouseoverElement = UIELEM_SEARCH_RESULT;
          }else{
            state->mouseoverElement = UIELEM_ENUM_LENGTH;
          }
        }
      }else if(down && !up){
        if((state->ss.numResults > 1)&&(state->mouseoverElement >= UIELEM_SEARCH_RESULT)&&(state->mouseoverElement <= UIELEM_SEARCH_RESULT_4)){
          if(state->mouseoverElement >= (UIELEM_SEARCH_RESULT + state->ss.numResults - 1)){
            state->mouseoverElement = UIELEM_SEARCH_RESULT;
          }else{
            state->mouseoverElement++;
          }
        }else{
          if(state->mouseoverElement != UIELEM_SEARCH_RESULT){
            state->mouseoverElement = UIELEM_SEARCH_RESULT;
          }else{
            state->mouseoverElement = UIELEM_ENUM_LENGTH;
          }
        }
      }else if((left && !right)&&((state->mouseoverElement < UIELEM_SEARCH_RESULT)||(state->mouseoverElement > UIELEM_SEARCH_RESULT_4))){
        if(state->searchCursorPos > 0){
          state->searchCursorPos -= 1;
          if(state->searchCursorPos > 0){
            if((state->searchCursorPos-1) < state->ds.searchEntryDispStartChar){
              state->ds.searchEntryDispStartChar = (uint16_t)(state->searchCursorPos-1);
              state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
            }
          }else{
            state->ds.searchEntryDispStartChar = 0;
            state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
          }
        }
      }else if((right && !left)&&((state->mouseoverElement < UIELEM_SEARCH_RESULT)||(state->mouseoverElement > UIELEM_SEARCH_RESULT_4))){
        if(state->searchCursorPos < (int)strlen(state->ss.searchString)){
          state->searchCursorPos += 1;
          state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
          while((state->searchCursorPos - state->ds.searchEntryDispStartChar) > state->ds.searchEntryDispNumChars){
            state->ds.searchEntryDispStartChar++;
            if(state->ds.searchEntryDispStartChar == 0){
              SDL_Log("WARNING: overflow\n");
              break;
            }
          }
        }
      }else if(left || right){
        goto menu_navigation; //considered harmful
      }
    }else{
      if(left && !right){
        if(state->searchCursorPos > 0){
          state->searchCursorPos -= 1;
          if(state->searchCursorPos > 0){
            if((state->searchCursorPos-1) < state->ds.searchEntryDispStartChar){
              state->ds.searchEntryDispStartChar = (uint16_t)(state->searchCursorPos-1);
              state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
            }
          }else{
            state->ds.searchEntryDispStartChar = 0;
            state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
          }
        }
      }else if(right && !left){
        if(state->searchCursorPos < (int)strlen(state->ss.searchString)){
          state->searchCursorPos += 1;
          state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
          while((state->searchCursorPos - state->ds.searchEntryDispStartChar) > state->ds.searchEntryDispNumChars){
            state->ds.searchEntryDispStartChar++;
            if(state->ds.searchEntryDispStartChar == 0){
              SDL_Log("WARNING: overflow\n");
              break;
            }
          }
        }
      }
    }
  }else if(chartPannable){

    //in main chart view, handle chart panning
    //SDL_Log("Processing input, mouseover element: %u\n",state->mouseoverElement);
    
    if(left || right || up || down){
      //SDL_Log("dir: [%u %u %u %u]\n",!(up==0),!(down==0),!(left==0),!(right==0));
      if(left && !right){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        state->ds.chartPanToX = state->ds.chartPosX - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToY = state->ds.chartPosY;
        }
        if(state->ds.chartPanToX <= (-0.25f*getChartWidthN(&state->ds))){
          state->ds.chartPanToX = (-0.25f*getChartWidthN(&state->ds));
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
        if(state->ds.chartPanToX >= (dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds)))){
          state->ds.chartPanToX = dat->ndat.maxN+(0.25f*getChartWidthN(&state->ds));
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
        if(state->ds.chartPanToY >= (dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds)))){
          state->ds.chartPanToY = dat->ndat.maxZ+(0.25f*getChartHeightZ(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
        //SDL_Log("pan start: [%0.2f %0.2f], pan to: [%0.2f %0.2f]\n",(double)state->ds.chartPanStartX,(double)state->ds.chartPanStartY,(double)state->ds.chartPanToX,(double)state->ds.chartPanToY);
      }else if(down && !up){
        state->ds.chartPanStartX = state->ds.chartPosX;
        state->ds.chartPanStartY = state->ds.chartPosY;
        if(state->ds.panInProgress == 0){
          state->ds.chartPanToX = state->ds.chartPosX;
        }
        state->ds.chartPanToY = state->ds.chartPosY - (CHART_PAN_DIST/state->ds.chartZoomScale);
        if(state->ds.chartPanToY <= (-0.25f*getChartHeightZ(&state->ds))){
          state->ds.chartPanToY = (-0.25f*getChartHeightZ(&state->ds));
        }
        state->ds.timeSincePanStart = 0.0f;
        state->ds.totalPanTime = CHART_KEY_PAN_TIME;
        state->ds.panInProgress = 1;
        state->ds.panFinished = 0;
      }
    }else if(altleft || altright || altup || altdown){

      //close any menus
      if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
        //close the primary menu
        uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
      }else if((state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
        //close the chart view menu
        uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON);
      }
      
      if(state->chartSelectedNucl == MAXNUMNUCL){
        //select nucleus
        int16_t selectedN = (int16_t)(state->ds.chartPosX - 0.5f);
        int16_t selectedZ = (int16_t)(state->ds.chartPosY + 0.5f + (16.0f/state->ds.chartZoomScale));
        uint16_t selNucl = getNearestNuclInd(dat,selectedN,selectedZ);
        setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
      }else{
        //change selected nucleus
        if(state->ds.panInProgress == 0){
          if(altleft && !altright){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }else if(altright && !altleft){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }
          if(altup && !altdown){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }else if(altdown && !altup){
            for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
              uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i));
              if(selNucl != MAXNUMNUCL){
                setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
                break;
              }
            }
          }
        }
      }
    }
  }else if((state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
    menu_navigation:
    if(left || right || up || down){
      if(state->clickedUIElem == UIELEM_MENU_BUTTON){
        //primary menu navigation using arrow keys
        if((state->mouseoverElement >= UIELEM_PRIMARY_MENU)||(state->mouseoverElement < (UIELEM_PRIMARY_MENU-PRIMARY_MENU_NUM_UIELEMENTS))){
          //no menu item was selected with the keyboard or highlighted with the mouse previously
          //select the first menu item
          state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU-PRIMARY_MENU_NUM_UIELEMENTS);
        }else{
          uint8_t selMenuElem = (uint8_t)(PRIMARY_MENU_NUM_UIELEMENTS - (UIELEM_PRIMARY_MENU - state->mouseoverElement));
          if(selMenuElem >= PRIMARY_MENU_NUM_UIELEMENTS){
            state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU-PRIMARY_MENU_NUM_UIELEMENTS);
          }
          if(up && !down){
            if(selMenuElem > 0){
              state->mouseoverElement--;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU-1);
            }
          }else if(down && !up){
            if(selMenuElem < (PRIMARY_MENU_NUM_UIELEMENTS-1)){
              state->mouseoverElement++;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PRIMARY_MENU-PRIMARY_MENU_NUM_UIELEMENTS);
            }
          }else if(left && !right){
            if(!(state->ds.shownElements & (1UL << UIELEM_NUCL_FULLINFOBOX))){
              if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f)){
                uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON); //open the chart view menu
              }
            }else{
              if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f)){
                uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON); //close the menu
                state->mouseoverElement = UIELEM_NUCL_FULLINFOBOX_BACKBUTTON; //highlight the back button (has to happen after uiElemClickAction, as this is reset during click action)
                state->lastOpenedMenu = UIELEM_NUCL_FULLINFOBOX_BACKBUTTON; //set back button as selected
              }
            }
          }else if(right && !left){
            if(!(state->ds.shownElements & (1UL << UIELEM_NUCL_FULLINFOBOX))){
              if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f)){
                uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_BUTTON); //open the search menu
              }
            }else{
              if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]==0.0f)){
                uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON); //open the reaction menu
              }
            }
          }
        }
      }else if(state->clickedUIElem == UIELEM_CHARTVIEW_BUTTON){
        //chart view menu navigation using arrow keys
        if((state->mouseoverElement >= UIELEM_CHARTVIEW_MENU)||(state->mouseoverElement < (UIELEM_CHARTVIEW_MENU-CHARTVIEW_ENUM_LENGTH))){
          //no menu item was selected with the keyboard or highlighted with the mouse previously
          //select the first menu item
          state->mouseoverElement = (uint8_t)(UIELEM_CHARTVIEW_MENU-CHARTVIEW_ENUM_LENGTH);
        }else{
          uint8_t selMenuElem = (uint8_t)(CHARTVIEW_ENUM_LENGTH - (UIELEM_CHARTVIEW_MENU - state->mouseoverElement));
          if(selMenuElem >= CHARTVIEW_ENUM_LENGTH){
            state->mouseoverElement = (uint8_t)(UIELEM_CHARTVIEW_MENU-CHARTVIEW_ENUM_LENGTH);
          }
          if(up && !down){
            if(selMenuElem > 0){
              state->mouseoverElement--;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_CHARTVIEW_MENU-1);
            }
          }else if(down && !up){
            if(selMenuElem < (CHARTVIEW_ENUM_LENGTH-1)){
              state->mouseoverElement++;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_CHARTVIEW_MENU-CHARTVIEW_ENUM_LENGTH);
            }
          }else if(right && !left){
            if((state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]==0.0f)){
              uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON); //open the primary menu
            }
          }else if(left && !right){
            if((state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]==0.0f)){
              uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_BUTTON); //open the search menu
            }
          }
        }
      }else if(state->clickedUIElem == UIELEM_SEARCH_BUTTON){
        //search menu navigation using arrow keys
        if(right && !left){
          if((state->ds.shownElements & (1UL << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]==0.0f)){
            uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON); //open the chart view menu
          }
        }else if(left && !right){
          if((state->ds.shownElements & (1UL << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]==0.0f)){
            uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON); //open the search menu
          }
        }
      }else if(state->clickedUIElem == UIELEM_NUCL_FULLINFOBOX_RXNBUTTON){
        //reaction menu navigation using arrow keys
        if(right && !left){
          if((state->ds.shownElements & (1UL << UIELEM_RXN_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_SHOW]==0.0f)){
            uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON); //close the menu
            state->mouseoverElement = UIELEM_NUCL_FULLINFOBOX_BACKBUTTON; //highlight the back button (has to happen after uiElemClickAction, as this is reset during click action)
            state->lastOpenedMenu = UIELEM_NUCL_FULLINFOBOX_BACKBUTTON; //set back button as selected
          }
        }else if(left && !right){
          if((state->ds.shownElements & (1UL << UIELEM_RXN_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_SHOW]==0.0f)){
            uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON); //open the primary menu
          }
        }
      }else if(state->lastOpenedMenu == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON){
        //back button navigation using arrow keys
        if(right && !left){
          uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON); //open the primary menu
        }else if(left && !right){
          uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON); //open the reaction menu
        }
      }
    }
  }else if(state->uiState == UISTATE_FULLLEVELINFO){

    if(up && !down){
      fcScrollAction(state,1.0f);
    }else if(down && !up){
      fcScrollAction(state,-1.0f);
    }else if(state->ds.fcNuclChangeInProgress == 0){
      //change selected nucleus
      if(altleft && !altright){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N-i),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z),0);
            break;
          }
        }
      }else if(altright && !altleft){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N+i),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z),0);
            break;
          }
        }
      }
      if(altup && !altdown){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z+i),0);
            break;
          }
        }
      }else if(altdown && !altup){
        for(uint8_t i=1; i<10; i++){ //skip empty entries in the chart if they exist
          uint16_t selNucl = getNuclInd(&dat->ndat,(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(int16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i));
          if(selNucl != MAXNUMNUCL){
            setSelectedNuclOnLevelList(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z-i),0);
            break;
          }
        }
      }
    }
  }else if(state->uiState == UISTATE_PREFS_DIALOG){
    if(left || right || up || down){
      //SDL_Log("dir: [%u %u %u %u]\n",!(up==0),!(down==0),!(left==0),!(right==0));
      if((state->ds.shownElements & (1UL << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
        //UI scale dropdown navigation using arrow keys
        state->mouseholdElement = UIELEM_ENUM_LENGTH; //remove any previous selection highlight
        if((state->mouseoverElement >= UIELEM_PREFS_UISCALE_MENU)||(state->mouseoverElement < (UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH))){
          //no menu item was selected with the keyboard or highlighted with the mouse previously
          //select the first menu item
          state->mouseoverElement = (uint8_t)(UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH);
        }else{
          uint8_t selMenuElem = (uint8_t)(UISCALE_ENUM_LENGTH - (UIELEM_PREFS_UISCALE_MENU - state->mouseoverElement));
          if(selMenuElem >= UISCALE_ENUM_LENGTH){
            state->mouseoverElement = (uint8_t)(UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH);
          }
          if(up && !down){
            if(selMenuElem > 0){
              state->mouseoverElement--;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PREFS_UISCALE_MENU-1);
            }
          }else if(down && !up){
            if(selMenuElem < (UISCALE_ENUM_LENGTH-1)){
              state->mouseoverElement++;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH);
            }
          }
        }
      }else{
        //pref menu navigation using arrow keys
        state->mouseholdElement = UIELEM_ENUM_LENGTH; //remove any previous selection highlight
        if((state->mouseoverElement >= UIELEM_PREFS_DIALOG)||(state->mouseoverElement < (UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS))){
          //no menu item was selected with the keyboard or highlighted with the mouse previously
          //select the first menu item
          state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS);
        }else{
          uint8_t selMenuElem = (uint8_t)(PREFS_DIALOG_NUM_UIELEMENTS - (UIELEM_PREFS_DIALOG - state->mouseoverElement));
          if(selMenuElem >= PREFS_DIALOG_NUM_UIELEMENTS){
            state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS);
          }
          if(up && !down){
            if(selMenuElem > 0){
              state->mouseoverElement--;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG-1);
            }
          }else if(down && !up){
            if(selMenuElem < (PREFS_DIALOG_NUM_UIELEMENTS-1)){
              state->mouseoverElement++;
            }else{
              state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS);
            }
          }else if((left && !right)||(right && !left)){
            if(state->mouseoverElement == UIELEM_PREFS_DIALOG_CLOSEBUTTON){
              state->mouseoverElement = (uint8_t)(UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS);
            }else{
              state->mouseoverElement = UIELEM_PREFS_DIALOG_CLOSEBUTTON;
            }
          }
        }
      }
    }
  }


  if(state->inputFlags & (1U << INPUT_SELECT)){
    if((state->ds.shownElements & (1UL << UIELEM_SEARCH_MENU))&&(state->ss.numResults > 0)){
      //commit search result
      if((state->mouseoverElement >= UIELEM_SEARCH_RESULT)&&(state->mouseoverElement <= UIELEM_SEARCH_RESULT_4)){
        state->mouseholdElement = state->mouseoverElement;
        uiElemClickAction(dat,state,rdat,0,state->mouseoverElement);
      }else{
        //select the first search result
        state->mouseholdElement = UIELEM_SEARCH_RESULT;
        uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_RESULT);
      }
    }else if((state->ds.shownElements & (1UL << UIELEM_ABOUT_BOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      //close the about box
      state->mouseholdElement = UIELEM_ABOUT_BOX_OK_BUTTON;
      uiElemClickAction(dat,state,rdat,0,UIELEM_ABOUT_BOX_OK_BUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
      //select primary menu button
      if((state->mouseoverElement < UIELEM_PRIMARY_MENU)&&(state->mouseoverElement >= (UIELEM_PRIMARY_MENU-PRIMARY_MENU_NUM_UIELEMENTS))){
        state->mouseholdElement = state->mouseoverElement;
        uiElemClickAction(dat,state,rdat,0,state->mouseoverElement);
      }
    }else if((state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
      //select chart view menu button
      if((state->mouseoverElement < UIELEM_CHARTVIEW_MENU)&&(state->mouseoverElement >= (UIELEM_CHARTVIEW_MENU-CHARTVIEW_ENUM_LENGTH))){
        state->mouseholdElement = state->mouseoverElement;
        uiElemClickAction(dat,state,rdat,0,state->mouseoverElement);
      }
    }else if((state->ds.shownElements & (1UL << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
      //select UI scale dropdown menu button
      if((state->mouseoverElement < UIELEM_PREFS_UISCALE_MENU)&&(state->mouseoverElement >= (UIELEM_PREFS_UISCALE_MENU-UISCALE_ENUM_LENGTH))){
        state->mouseholdElement = state->mouseoverElement;
        uiElemClickAction(dat,state,rdat,0,state->mouseoverElement);
      }
    }else if((state->ds.shownElements & (1UL << UIELEM_PREFS_DIALOG))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      //select pref menu button
      //only get here if the UI scale dropdown isn't open 
      if((state->mouseoverElement < UIELEM_PREFS_DIALOG)&&(state->mouseoverElement >= (UIELEM_PREFS_DIALOG-PREFS_DIALOG_NUM_UIELEMENTS))){
        state->mouseholdElement = state->mouseoverElement;
        uiElemClickAction(dat,state,rdat,0,state->mouseoverElement);
      }
    }else if((state->ds.shownElements & (1UL << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)){
      state->mouseholdElement = UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON;
      uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_NUCL_FULLINFOBOX))&&(state->mouseoverElement == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)&&(state->lastOpenedMenu == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)&&(state->lastInputType == INPUT_TYPE_KEYBOARD)){
      if((state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]==0.0f)){
        //navigated to the back button by keyboard, so select it now
        uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON); //go back to the main chart
        state->mouseholdElement = UIELEM_ENUM_LENGTH;
      }
    }else if(state->uiState == UISTATE_CHARTONLY){
      if(state->chartSelectedNucl == MAXNUMNUCL){
        //select the center-screen nuclide on the chart
        int16_t selectedN = (int16_t)(state->ds.chartPosX - 0.5f);
        int16_t selectedZ = (int16_t)(state->ds.chartPosY + 0.5f + (16.0f/state->ds.chartZoomScale));
        uint16_t selNucl = getNearestNuclInd(dat,selectedN,selectedZ);
        setSelectedNuclOnChartDirect(dat,state,rdat,selNucl,2);
      }
    }
  }else if(state->inputFlags & (1U << INPUT_BACK)){
    //escape open menus
    //handle modal dialogs first
    if((state->ds.shownElements & (1UL << UIELEM_ABOUT_BOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      //close the about box
      state->mouseholdElement = UIELEM_ABOUT_BOX_OK_BUTTON;
      uiElemClickAction(dat,state,rdat,0,UIELEM_ABOUT_BOX_OK_BUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
      //close the UI scale menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
    }else if((state->ds.shownElements & (1UL << UIELEM_PREFS_DIALOG))&&(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]==0.0f)){
      state->mouseholdElement = UIELEM_PREFS_DIALOG_CLOSEBUTTON;
      uiElemClickAction(dat,state,rdat,0,UIELEM_PREFS_DIALOG_CLOSEBUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]==0.0f)){
      //close the primary menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f)){
      //close the chart view menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_SEARCH_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]==0.0f)){
      //close the chart view menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_BUTTON);
    }else if((state->ds.shownElements & (1UL << UIELEM_NUCL_INFOBOX))&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]==0.0f)){
      startUIAnimation(dat,state,UIANIM_NUCLINFOBOX_HIDE); //hide the info box, see stopUIAnimation() for info box hiding action
      startUIAnimation(dat,state,UIANIM_NUCLHIGHLIGHT_HIDE);
    }else if((state->ds.shownElements & (1UL << UIELEM_RXN_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_HIDE]==0.0f)){
      //close the reaction menu
      uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON);
    }else if((state->uiState == UISTATE_FULLLEVELINFOWITHMENU)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]==0.0f)){
      state->uiState = UISTATE_FULLLEVELINFO;
      state->mouseoverElement = UIELEM_ENUM_LENGTH;
    }else if((state->uiState == UISTATE_FULLLEVELINFO)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]==0.0f)){
      uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON); //go back to the main chart
      state->mouseholdElement = UIELEM_ENUM_LENGTH;
    }else if(state->ds.windowFullscreenMode){
      //exit fullscreen
      state->ds.windowFullscreenMode = 0;
      handleScreenGraphicsMode(dat,state,rdat); 
    }
  }else if(state->inputFlags & (1U << INPUT_MENU)){
    if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_CHARTWITHMENU)||(state->uiState == UISTATE_INFOBOX)){
      //open the primary menu
      switch(state->lastOpenedMenu){
        case UIELEM_SEARCH_MENU:
          uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_BUTTON);
          break;
        case UIELEM_CHARTVIEW_MENU:
          uiElemClickAction(dat,state,rdat,0,UIELEM_CHARTVIEW_BUTTON);
          break;
        case UIELEM_PRIMARY_MENU:
        default:
          uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
          break;
      }
    }else if(state->uiState == UISTATE_FULLLEVELINFO){
      switch(state->lastOpenedMenu){
        case UIELEM_NUCL_FULLINFOBOX_BACKBUTTON:
          if((state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]==0.0f)&&(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]==0.0f)){
            if(state->mouseoverElement == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON){
              uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON); //go back to the main chart
              state->mouseholdElement = UIELEM_ENUM_LENGTH;
            }else{
              state->uiState = UISTATE_FULLLEVELINFOWITHMENU; //allow menu navigation by keyboard
              state->mouseoverElement = UIELEM_NUCL_FULLINFOBOX_BACKBUTTON; //highlight the back button
            }
          }
          break;
        case UIELEM_RXN_MENU:
          uiElemClickAction(dat,state,rdat,0,UIELEM_NUCL_FULLINFOBOX_RXNBUTTON);
          break;
        case UIELEM_PRIMARY_MENU:
        default:
          uiElemClickAction(dat,state,rdat,0,UIELEM_MENU_BUTTON);
          break;
      }
    }
    state->mouseholdElement = UIELEM_ENUM_LENGTH; //remove any previous selection highlight
  }

  /* Handle mouse input */
  if(state->lastInputType == INPUT_TYPE_MOUSE){

    SDL_ShowCursor();

    state->mouseoverElement = UIELEM_ENUM_LENGTH; //by default, no element is moused over
    state->mouseholdElement = UIELEM_ENUM_LENGTH;

    if(state->ds.dragInProgress == 0){
      //first, handle dynamic menu (menus without a fixed number of entries) selections
      uint8_t dynamicMenuItemInteracted = 0;
      if(state->ds.shownElements & (1UL << UIELEM_RXN_MENU)){
        uint8_t numRxnPerCol = getRxnMenuNumRxnsPerColumn(dat,state);
        SDL_FRect buttonRect;
        if(state->ds.timeLeftInUIAnimation[UIANIM_RXN_MENU_HIDE]==0.0f){
          //SDL_Log("Mouse-over reaction: %u, selected reaction: %u.\n",state->ds.mouseOverRxn,state->ds.selectedRxn);
          for(uint8_t i=0;i<(dat->ndat.nuclData[state->chartSelectedNucl].numRxns+1);i++){
            buttonRect = getRxnMenuButtonRect(state,numRxnPerCol,i);
            if((state->mouseXPx >= buttonRect.x)&&(state->mouseXPx < (buttonRect.x + buttonRect.w))){
              if((state->mouseYPx >= buttonRect.y)&&(state->mouseYPx < (buttonRect.y + buttonRect.h))){
                state->ds.mouseOverRxn = i;
                if((state->mouseClickPosXPx >= buttonRect.x)&&(state->mouseClickPosXPx < (buttonRect.x + buttonRect.w))){
                  if((state->mouseClickPosYPx >= buttonRect.y)&&(state->mouseClickPosYPx < (buttonRect.y + buttonRect.h))){
                    state->ds.selectedRxn = i;
                    //SDL_Log("Clicked reaction menu item %u.\n",state->ds.selectedRxn);
                    setSelectedNuclOnLevelList(dat,state,rdat,(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].N),(uint16_t)(dat->ndat.nuclData[state->chartSelectedNucl].Z),1);
                    break;
                  }
                }
                dynamicMenuItemInteracted = 1;
                break;
              }
            }
          }
        }
      }
      if(dynamicMenuItemInteracted == 0){
        for(uint8_t i=0; i<UIELEM_ENUM_LENGTH; i++){ //ordering in ui_element_enum defines order in which UI elements receive input
          if(state->interactableElement & (uint64_t)(1LU << i)){
            if((state->mouseHoldStartPosXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseHoldStartPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseHoldStartPosYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseHoldStartPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
              state->mouseholdElement = i;
              //SDL_Log("Holding element %u\n",i);
              uiElemHoldAction(dat,state,state->mouseholdElement);
            }
            if((state->mouseXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
              state->mouseoverElement = i;
              //SDL_Log("mouseover element: %u\n",i);
              if((state->mouseClickPosXPx >= (state->ds.uiElemPosX[i]-state->ds.uiElemExtMinusX[i]))&&(state->mouseClickPosXPx < (state->ds.uiElemPosX[i]+state->ds.uiElemWidth[i]+state->ds.uiElemExtPlusX[i]))&&(state->mouseClickPosYPx >= (state->ds.uiElemPosY[i]-state->ds.uiElemExtMinusY[i]))&&(state->mouseClickPosYPx < (state->ds.uiElemPosY[i]+state->ds.uiElemHeight[i]+state->ds.uiElemExtPlusY[i]))){
                //take action
                uiElemClickAction(dat,state,rdat,0,i); //data_ops.c
                //SDL_Log("Clicked element %u\n",i);
                return;
              }
              break;
            }
          }
        }
      }

      uiElemMouseoverAction(rdat,state->mouseoverElement); //handle cursor changes with mouse position

      //handle click and drag on the chart of nuclides
      if((chartDraggable)&&(state->mouseholdElement == UIELEM_ENUM_LENGTH)&&(state->mouseHoldStartPosXPx >= 0.0f)){
        state->ds.chartDragStartX = state->ds.chartPosX;
        state->ds.chartDragStartY = state->ds.chartPosY;
        state->ds.chartDragStartMouseX = state->mouseXPx;
        state->ds.chartDragStartMouseY = state->mouseYPx;
        state->ds.dragInProgress = 1;
        //SDL_Log("start drag\n");
      }
    }

    //check for mouse release
    if(state->mouseHoldStartPosXPx < 0.0f){
      if(state->ds.dragInProgress){
        //SDL_Log("Mouse released.\n");
        state->ds.dragFinished = 1;
      }
    }

    uint32_t doubleClick = (state->inputFlags & (1U << INPUT_DOUBLECLICK));

    //only get here if no button was clicked
    //check if a click was made outside of any button
    //SDL_Log("click pos x: %f, drag start: [%f %f]\n",(double)state->mouseClickPosXPx,(double)state->ds.chartDragStartMouseX,(double)state->ds.chartDragStartMouseY);
    if(chartDraggable){
      if((state->mouseClickPosXPx >= 0.0f) && (fabsf(state->ds.chartDragStartMouseX - state->mouseXPx) < 5.0f) && (fabsf(state->ds.chartDragStartMouseY - state->mouseYPx) < 5.0f) ){
        //unclick (or click on chart view)
        if(doubleClick){
          uiElemClickAction(dat,state,rdat,1,UIELEM_ENUM_LENGTH);
        }else{
          uiElemClickAction(dat,state,rdat,0,UIELEM_ENUM_LENGTH);
        }
      }
    }else if((state->uiState == UISTATE_FULLLEVELINFO)||(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
      if(state->mouseClickPosXPx >= 0.0f){
        //clicked outside of interactable items on the full level info screen
        uiElemClickAction(dat,state,rdat,0,UIELEM_ENUM_LENGTH);
      }
    }else if(state->uiState == UISTATE_PREFS_DIALOG){
      if(state->mouseClickPosXPx >= 0.0f){
        if((state->ds.shownElements & (1UL << UIELEM_PREFS_UISCALE_MENU))&&(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f)){
          //close the UI scale menu
          uiElemClickAction(dat,state,rdat,0,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN);
        }
      }
    }
  }else{
    SDL_HideCursor();
  }

  /* Handle zoom input */
  if((state->inputFlags & (1U << INPUT_ZOOM))&&(fabsf(state->zoomDeltaVal)>0.05f)){
    if(chartDraggable){
      if(state->zoomDeltaVal != 0.0f){
        state->ds.chartZoomStartScale = state->ds.chartZoomScale;
        if(state->zoomDeltaVal > 0){
          //zoom in
          state->ds.chartZoomToScale += state->zoomDeltaVal*state->ds.chartZoomToScale*1.0f;
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
          //zoom using mouse wheel/touchpad, zoom to cursor location
          //(other input methods just call the zoom button interaction code in data_ops.c)
          state->ds.chartZoomStartMouseX = mouseXPxToN(&state->ds,state->mouseXPx);
          state->ds.chartZoomStartMouseY = mouseYPxToZ(&state->ds,state->mouseYPx);

          if(state->ds.chartZoomStartMouseX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
            state->ds.chartZoomStartMouseX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
          }else if(state->ds.chartZoomStartMouseX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
            state->ds.chartZoomStartMouseX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
          }
          if(state->ds.chartZoomStartMouseY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
            state->ds.chartZoomStartMouseY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
          }else if(state->ds.chartZoomStartMouseY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
            state->ds.chartZoomStartMouseY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
          }
        }
        state->ds.chartZoomStartMouseXFrac = (state->ds.chartZoomStartMouseX - getMinChartN(&state->ds))/getChartWidthN(&state->ds);
        float afterZoomMinN = state->ds.chartZoomStartMouseX - getChartWidthNAfterZoom(&state->ds)*state->ds.chartZoomStartMouseXFrac;
        state->ds.chartZoomToX = afterZoomMinN + getChartWidthNAfterZoom(&state->ds)*0.5f;
        state->ds.chartZoomStartMouseYFrac = (state->ds.chartZoomStartMouseY - getMinChartZ(&state->ds))/getChartHeightZ(&state->ds);
        float afterZoomMinZ = state->ds.chartZoomStartMouseY - getChartHeightZAfterZoom(&state->ds)*state->ds.chartZoomStartMouseYFrac;
        state->ds.chartZoomToY = afterZoomMinZ + getChartHeightZAfterZoom(&state->ds)*0.5f;
        if(state->ds.chartZoomToX > (dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds)))){
          state->ds.chartZoomToX = (float)dat->ndat.maxN+(0.25f*getChartWidthNAfterZoom(&state->ds));
        }else if(state->ds.chartZoomToX < (-0.25f*getChartWidthNAfterZoom(&state->ds))){
          state->ds.chartZoomToX = (-0.25f*getChartWidthNAfterZoom(&state->ds));
        }
        if(state->ds.chartZoomToY > (dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds)))){
          state->ds.chartZoomToY = (float)dat->ndat.maxZ+(0.25f*getChartHeightZAfterZoom(&state->ds));
        }else if(state->ds.chartZoomToY < (-0.25f*getChartHeightZAfterZoom(&state->ds))){
          state->ds.chartZoomToY = (-0.25f*getChartHeightZAfterZoom(&state->ds));
        }
        //SDL_Log("xZoomFrac: %0.3f, afterZoomMinN: %0.3f\n",(double)state->ds.chartZoomStartMouseXFrac,(double)afterZoomMinN);
        //SDL_Log("yZoomFrac: %0.3f, afterZoomMinZ: %0.3f\n",(double)state->ds.chartZoomStartMouseYFrac,(double)afterZoomMinZ);
        state->ds.timeSinceZoomStart = 0.0f;
        state->ds.zoomInProgress = 1;
        state->ds.zoomFinished = 0;
      }
      //SDL_Log("scale: %0.2f\n",(double)state->ds.chartZoomScale);
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
        SDL_Log("Gamepad added.\n");
        rdat->gamepad = NULL;
        int num_joysticks;
        SDL_JoystickID *joysticks = SDL_GetJoysticks(&num_joysticks);
        for(int i=0; i<num_joysticks; i++){
          if(SDL_IsGamepad(joysticks[i])){
            rdat->gamepad = SDL_OpenGamepad(joysticks[i]);
            if(rdat->gamepad){
              break;
            }else{
              SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"Could not open game gamepad %i: %s\n", i, SDL_GetError());
            }
          }
        }
      }
      break;
    case SDL_EVENT_GAMEPAD_REMOVED:
      SDL_Log("Gamepad removed.\n");
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
      switch(evt.gbutton.button){
        case SDL_GAMEPAD_BUTTON_DPAD_UP:
          if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFO)){
            state->inputFlags |= (1U << INPUT_ALTUP);
          }else{
            state->inputFlags |= (1U << INPUT_UP);
          }
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
          if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFO)){
            state->inputFlags |= (1U << INPUT_ALTDOWN);
          }else{
            state->inputFlags |= (1U << INPUT_DOWN);
          }
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
          if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFO)){
            state->inputFlags |= (1U << INPUT_ALTLEFT);
          }else{
            state->inputFlags |= (1U << INPUT_LEFT);
          }
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
          if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFO)){
            state->inputFlags |= (1U << INPUT_ALTRIGHT);
          }else{
            state->inputFlags |= (1U << INPUT_RIGHT);
          }
          break;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
          state->mouseholdElement = UIELEM_ZOOMOUT_BUTTON;
          uiElemClickAction(dat,state,rdat,0,UIELEM_ZOOMOUT_BUTTON);
          break;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
          state->mouseholdElement = UIELEM_ZOOMIN_BUTTON;
          uiElemClickAction(dat,state,rdat,0,UIELEM_ZOOMIN_BUTTON);
          break;
        case SDL_GAMEPAD_BUTTON_START:
        case SDL_GAMEPAD_BUTTON_NORTH:
          if((state->uiState == UISTATE_CHARTONLY)||(state->uiState == UISTATE_INFOBOX)||(state->uiState == UISTATE_FULLLEVELINFO)){
            //open menu
            state->inputFlags |= (1U << INPUT_MENU);
          }
          break;
        case SDL_GAMEPAD_BUTTON_EAST:
          state->inputFlags |= (1U << INPUT_SELECT);
          break;
        case SDL_GAMEPAD_BUTTON_BACK:
        case SDL_GAMEPAD_BUTTON_SOUTH:
          if(state->uiState != UISTATE_CHARTONLY){
            state->inputFlags |= (1U << INPUT_BACK);
          }
          break;
        case SDL_GAMEPAD_BUTTON_GUIDE:
          state->ds.windowFullscreenMode = !state->ds.windowFullscreenMode;
          handleScreenGraphicsMode(dat,state,rdat);
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_MOUSE_MOTION:
      SDL_GetMouseState(&state->mouseXPx,&state->mouseYPx); //update mouse position
      if(state->lastInputType != INPUT_TYPE_MOUSE){
        state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      }
      if((state->lastOpenedMenu == UIELEM_NUCL_FULLINFOBOX_BACKBUTTON)&&(state->uiState == UISTATE_FULLLEVELINFOWITHMENU)){
        state->uiState = UISTATE_FULLLEVELINFO; //exit menu control
      }
      //SDL_Log("Mouse position: %i %i\n",*mouseX,*mouseY);
      break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      switch(evt.button.button){
        case SDL_BUTTON_LEFT:
          //SDL_Log("Left mouse button down.\n");
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
            //SDL_Log("Double click.\n");
            state->inputFlags |= (1U << INPUT_DOUBLECLICK);
          }
          //SDL_Log("Left mouse button up.\n");
          break;
        case SDL_BUTTON_RIGHT:
          //SDL_Log("Right mouse button up.\n");
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_MOUSE_WHEEL:
      state->lastInputType = INPUT_TYPE_MOUSE; //set mouse input
      if(evt.wheel.direction == SDL_MOUSEWHEEL_NORMAL){
        if(evt.wheel.y > 0){
          //SDL_Log("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //SDL_Log("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }
      }else{
        if(evt.wheel.y > 0){
          //SDL_Log("Mouse wheel down %0.3f.\n",(double)evt.wheel.y);
          state->zoomDeltaVal = evt.wheel.y;
          state->inputFlags |= (1U << INPUT_ZOOM);
        }else if(evt.wheel.y < 0){
          //SDL_Log("Mouse wheel up %0.3f.\n",(double)evt.wheel.y);
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
      switch(evt.key.scancode){
        case SDL_SCANCODE_LEFT:
          state->inputFlags |= (1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_A:
          if(!(SDL_TextInputActive(rdat->window))){
            state->inputFlags |= (1U << INPUT_ALTLEFT);
          }
          break;
        case SDL_SCANCODE_RIGHT:
          state->inputFlags |= (1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_D:
          if(!(SDL_TextInputActive(rdat->window))){
            state->inputFlags |= (1U << INPUT_ALTRIGHT);
          }
          break;
        case SDL_SCANCODE_UP:
          state->inputFlags |= (1U << INPUT_UP);
          break;
        case SDL_SCANCODE_W:
          if(!(SDL_TextInputActive(rdat->window))){
            state->inputFlags |= (1U << INPUT_ALTUP);
          }
          break;
        case SDL_SCANCODE_DOWN:
          state->inputFlags |= (1U << INPUT_DOWN);
          break;
        case SDL_SCANCODE_S:
          if(!(SDL_TextInputActive(rdat->window))){
            state->inputFlags |= (1U << INPUT_ALTDOWN);
          }
          break;
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:
          if(state->kbdModVal == KBD_MOD_NONE){
            state->inputFlags |= (1U << INPUT_MENU);
          }
          break;
        case SDL_SCANCODE_ESCAPE:
          if(SDL_TextInputActive(rdat->window)){
            SDL_StopTextInput(rdat->window);
          }
          state->inputFlags |= (1U << INPUT_BACK);
          break;
        case SDL_SCANCODE_BACKSPACE:
          if(!(SDL_TextInputActive(rdat->window))){
            state->inputFlags |= (1U << INPUT_BACK);
          }else{
            if(strDelChar(state->ss.searchString,SEARCH_STRING_MAX_SIZE,(size_t)state->searchCursorPos) >= 0){
              state->searchCursorPos -= 1;
              if(state->searchCursorPos > 0){
                if((state->searchCursorPos-1) < state->ds.searchEntryDispStartChar){
                  state->ds.searchEntryDispStartChar = (uint16_t)(state->searchCursorPos-1);
                  state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
                }
              }else{
                state->ds.searchEntryDispStartChar = 0;
                state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
              }
              state->searchStrUpdated = 1;
              //SDL_Log("cursor pos: %u, start disp: %u\n",state->searchCursorPos,state->ds.searchEntryDispStartChar);
              //SDL_Log("search query: %s\n",state->ss.searchString);
            }
          }
          break;
        case SDL_SCANCODE_EQUALS:
          if(!(SDL_TextInputActive(rdat->window))){
            state->mouseholdElement = UIELEM_ZOOMIN_BUTTON;
            uiElemClickAction(dat,state,rdat,0,UIELEM_ZOOMIN_BUTTON);
          }
          break;
        case SDL_SCANCODE_MINUS:
          if(!(SDL_TextInputActive(rdat->window))){
            state->mouseholdElement = UIELEM_ZOOMOUT_BUTTON;
            uiElemClickAction(dat,state,rdat,0,UIELEM_ZOOMOUT_BUTTON);
          }
          break;
        case SDL_SCANCODE_LEFTBRACKET:
          if(!(SDL_TextInputActive(rdat->window))){
            if(state->chartView == 0){
              state->chartView = (uint8_t)(CHARTVIEW_ENUM_LENGTH - 1);
            }else{
              state->chartView--;
            }
          }
          break;
        case SDL_SCANCODE_RIGHTBRACKET:
          if(!(SDL_TextInputActive(rdat->window))){
            if(state->chartView == (uint8_t)(CHARTVIEW_ENUM_LENGTH - 1)){
              state->chartView = 0;
            }else{
              state->chartView++;
            }
          }
          break;
        case SDL_SCANCODE_F:
          if(state->kbdModVal == KBD_MOD_CTRL){
            if(state->ds.shownElements & (1UL << UIELEM_CHARTOFNUCLIDES)){
              uiElemClickAction(dat,state,rdat,0,UIELEM_SEARCH_BUTTON); //open search, also activates text input
            }
          }
          break;
        /*case SDL_SCANCODE_F7:
          //scale UI down
          if(state->ds.interfaceSizeInd > 0){
            state->ds.interfaceSizeInd--;
          }else{
            state->ds.interfaceSizeInd = UISCALE_ENUM_LENGTH - 1;
          }
          state->ds.uiUserScale = uiScales[state->ds.interfaceSizeInd];
          updateUIScale(dat,state,rdat);
          break;
        case SDL_SCANCODE_F8:
          //scale UI up
          if(state->ds.interfaceSizeInd < (UISCALE_ENUM_LENGTH - 1)){
            state->ds.interfaceSizeInd++;
          }else{
            state->ds.interfaceSizeInd = 0;
          }
          state->ds.uiUserScale = uiScales[state->ds.interfaceSizeInd];
          updateUIScale(dat,state,rdat);
          break;*/
        case SDL_SCANCODE_RETURN:
          state->inputFlags |= (1U << INPUT_SELECT);
          break;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
          if(!(SDL_TextInputActive(rdat->window))){
            state->kbdModVal = KBD_MOD_SHIFT;
          }
          break;
        case SDL_SCANCODE_P:
          if(!(SDL_TextInputActive(rdat->window))){
            state->ds.drawPerformanceStats = !state->ds.drawPerformanceStats;
          }
          break;
        case SDL_SCANCODE_F11:
          state->ds.windowFullscreenMode = !state->ds.windowFullscreenMode;
          handleScreenGraphicsMode(dat,state,rdat);
          break;
        case SDL_SCANCODE_Q:
          if(state->kbdModVal == KBD_MOD_CTRL){
            state->quitAppFlag = 1; //quit the app
          }
          break;
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
          state->kbdModVal = KBD_MOD_CTRL;
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_GAMEPAD_BUTTON_UP: //released button
      state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
      switch(evt.gbutton.button){
        case SDL_GAMEPAD_BUTTON_DPAD_UP:
          state->inputFlags &= ~(1U << INPUT_ALTUP);
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
          state->inputFlags &= ~(1U << INPUT_ALTDOWN);
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
          state->inputFlags &= ~(1U << INPUT_ALTLEFT);
          break;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
          state->inputFlags &= ~(1U << INPUT_ALTRIGHT);
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_KEY_UP: //released key
      switch(evt.key.scancode){
        case SDL_SCANCODE_LEFT:
          state->inputFlags &= ~(1U << INPUT_LEFT);
          break;
        case SDL_SCANCODE_A:
          state->inputFlags &= ~(1U << INPUT_ALTLEFT);
          break;
        case SDL_SCANCODE_RIGHT:
          state->inputFlags &= ~(1U << INPUT_RIGHT);
          break;
        case SDL_SCANCODE_D:
          state->inputFlags &= ~(1U << INPUT_ALTRIGHT);
          break;
        case SDL_SCANCODE_UP:
          state->inputFlags &= ~(1U << INPUT_UP);
          break;
        case SDL_SCANCODE_W:
          state->inputFlags &= ~(1U << INPUT_ALTUP);
          break;
        case SDL_SCANCODE_DOWN:
          state->inputFlags &= ~(1U << INPUT_DOWN);
          break;
        case SDL_SCANCODE_S:
          state->inputFlags &= ~(1U << INPUT_ALTDOWN);
          break;
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:
          state->kbdModVal = KBD_MOD_NONE;
          break;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:
          state->kbdModVal = KBD_MOD_NONE;
          break;
        default:
          break;
      }
      break;
    //analog stick control
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: //gamepad axis, use large values to account for deadzone
      //SDL_Log("axis: %i, val: %i, dz: %i\n",evt.gaxis.axis,evt.gaxis.value,state->gamepadDeadzone);
      //evtCnt++;
      switch(evt.gaxis.axis){
        case SDL_GAMEPAD_AXIS_LEFTX:
          if(evt.gaxis.value > state->gamepadDeadzone){
            if(state->lastAxisValLX==0){
              //SDL_Log("Gamepad right once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_RIGHT);
            }
            state->lastAxisValLX = evt.gaxis.value;
            state->activeAxisX = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            if(state->lastAxisValLX==0){
              //SDL_Log("Gamepad left once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_LEFT);
            }
            state->lastAxisValLX = evt.gaxis.value;
            state->activeAxisX = evt.gaxis.axis;
          }else if(state->activeAxisX == evt.gaxis.axis){
            //SDL_Log("Stopped gamepad horizontal axis.\n");
            state->lastAxisValLX = 0;
            state->activeAxisX = 255;
            state->inputFlags &= ~(1U << INPUT_LEFT);
            state->inputFlags &= ~(1U << INPUT_RIGHT);
          }
          break;
        case SDL_GAMEPAD_AXIS_RIGHTX:
          //SDL_Log("LR axis val: %i\n",evt.gaxis.value);
          if(evt.gaxis.value > state->gamepadDeadzone){
            if(state->lastAxisValRX==0){
              //SDL_Log("Gamepad right once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_RIGHT);
            }
            state->lastAxisValRX = evt.gaxis.value;
            state->activeAxisX = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            if(state->lastAxisValRX==0){
              //SDL_Log("Gamepad left once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_LEFT);
            }
            state->lastAxisValRX = evt.gaxis.value;
            state->activeAxisX = evt.gaxis.axis;
          }else if(state->activeAxisX == evt.gaxis.axis){
            //SDL_Log("Stopped gamepad horizontal axis.\n");
            state->lastAxisValRX = 0;
            state->activeAxisX = 255;
            state->inputFlags &= ~(1U << INPUT_LEFT);
            state->inputFlags &= ~(1U << INPUT_RIGHT);
          }
          break;
        case SDL_GAMEPAD_AXIS_LEFTY:
          if(evt.gaxis.value > state->gamepadDeadzone){
            if(state->lastAxisValLY==0){
              //SDL_Log("Gamepad down once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_DOWN);
            }
            state->lastAxisValLY = evt.gaxis.value;
            state->activeAxisY = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            if(state->lastAxisValLY==0){
              //SDL_Log("Gamepad up once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_UP);
            }
            state->lastAxisValLY = evt.gaxis.value;
            state->activeAxisY = evt.gaxis.axis;
          }else if(state->activeAxisY == evt.gaxis.axis){
            //SDL_Log("Stopped gamepad vertical axis.\n");
            state->lastAxisValLY = 0;
            state->activeAxisY = 255;
            state->inputFlags &= ~(1U << INPUT_UP);
            state->inputFlags &= ~(1U << INPUT_DOWN);
          }
          break;
        case SDL_GAMEPAD_AXIS_RIGHTY:
          //SDL_Log("UD axis val: %i\n",evt.gaxis.value);
          if(evt.gaxis.value > state->gamepadDeadzone){
            if(state->lastAxisValRY==0){
              //SDL_Log("Gamepad down once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_DOWN);
            }
            state->lastAxisValRY = evt.gaxis.value;
            state->activeAxisY = evt.gaxis.axis;
          }else if(evt.gaxis.value < -1*state->gamepadDeadzone){
            if(state->lastAxisValRY==0){
              //SDL_Log("Gamepad up once.\n");
              state->lastInputType = INPUT_TYPE_GAMEPAD; //set gamepad input
              state->inputFlags |= (1U << INPUT_UP);
            }
            state->lastAxisValRY = evt.gaxis.value;
            state->activeAxisY = evt.gaxis.axis;
          }else if(state->activeAxisY == evt.gaxis.axis){
            //SDL_Log("Stopped gamepad vertical axis.\n");
            state->lastAxisValRY = 0;
            state->activeAxisY = 255;
            state->inputFlags &= ~(1U << INPUT_UP);
            state->inputFlags &= ~(1U << INPUT_DOWN);
          }
          break;
        default:
          break;
      }
      break;
    case SDL_EVENT_TEXT_INPUT:
      //SDL_Log("text input: %s\n",evt.text.text);
      // Add new text onto the end of the search string
      if(strInsert(state->ss.searchString,SEARCH_STRING_MAX_SIZE,evt.text.text,(size_t)state->searchCursorPos) >= 0){
        state->searchCursorPos += (int)strlen(evt.text.text);
        if(state->searchCursorPos > (SEARCH_STRING_MAX_SIZE-1)){
          state->searchCursorPos = (SEARCH_STRING_MAX_SIZE-1);
        }
        state->ds.searchEntryDispNumChars = getNumTextCharsUnderWidth(rdat,SEARCH_MENU_ENTRYBOX_ENTRY_WIDTH,state->ss.searchString,state->ds.searchEntryDispStartChar);
        while((state->searchCursorPos - state->ds.searchEntryDispStartChar) > state->ds.searchEntryDispNumChars){
          state->ds.searchEntryDispStartChar++;
          if(state->ds.searchEntryDispStartChar == 0){
            SDL_Log("WARNING: overflow\n");
            break;
          }
        }
        /*if(state->ss.numResults > 0){
          state->mouseoverElement = (uint8_t)(UIELEM_SEARCH_RESULT); //select the first search result
        }*/
        state->searchStrUpdated = 1;
        //SDL_Log("Search query display start: %u, num chars: %u, cursor pos: %u\n",state->ds.searchEntryDispStartChar,state->ds.searchEntryDispNumChars,state->searchCursorPos);
        //SDL_Log("search query: %s\n",state->ss.searchString);
      }
      break;
    /*case SDL_EVENT_TEXT_EDITING:
      state->searchCursorPos = evt.edit.start; //seems to reset to 0 when changing workspace
      state->searchSelectionLen = evt.edit.length;
      break;*/
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
    state->inputFlags &= ~(1U << INPUT_SELECT);
    state->inputFlags &= ~(1U << INPUT_BACK);
    state->inputFlags &= ~(1U << INPUT_MENU);
    //whenever the directional inputs are not used for continuous actions such as chart navigation,
    //make sure that they don't persist across frames (keys will still repeat, just want to avoid
    //case where the input flag is not reset)
    if((state->uiState != UISTATE_CHARTONLY)&&(state->uiState != UISTATE_INFOBOX)&&(state->uiState != UISTATE_FULLLEVELINFO)){
      state->inputFlags &= ~(1U << INPUT_UP);
      state->inputFlags &= ~(1U << INPUT_DOWN);
      state->inputFlags &= ~(1U << INPUT_LEFT);
      state->inputFlags &= ~(1U << INPUT_RIGHT);
    }
    if(state->inputFlags & (1U << INPUT_DOUBLECLICK)){
      state->inputFlags &= ~(1U << INPUT_DOUBLECLICK);
    }

    if((state->ds.uiAnimPlaying != 0)||(state->ds.zoomInProgress)||(state->ds.dragInProgress)||(state->ds.panInProgress)||(state->ds.fcScrollInProgress)||(state->ds.fcNuclChangeInProgress)){
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

