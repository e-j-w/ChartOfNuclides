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

/* Higher-level drawing functions handling display of the user interface. */
/* The UI scale shouldn't be handled here, it should instead be handled in the lower level functions of state->ds.sds.h*/

#include "gui.h"
#include "gui_constants.h"
#include "juicer.h" //contains easing functions used in animations
#include "drawing.h"
#include "data_ops.h"

float getAxisTickSpacing(float range){
  //SDL_Log("range: %f\n",(double)range);
  if(range < 12.0f){
    return 1.0f;
  }else if(range < 30.0f){
    return 2.0f;
  }else if(range < 60.0f){
    return 5.0f;
  }else if(range < 90.0f){
    return 10.0f;
  }else if(range < 200.0f){
    return 20.0f;
  }else{
    return 40.0f;
  }
}

SDL_FColor getHalfLifeCol(const double halflifeSeconds){
  //SDL_Log("half-life: %0.6f\n",halflifeSeconds);
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  if(halflifeSeconds > 1.0E15){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(halflifeSeconds > 1.0E10){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 1.0E7){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 1.0E5){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E4){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E3){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E2){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E1){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-1){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 1.0E-2){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 1.0E-3){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.2f;
  }else if(halflifeSeconds > 1.0E-4){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(halflifeSeconds > 1.0E-5){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(halflifeSeconds > 1.0E-6){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(halflifeSeconds > 1.0E-7){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-9){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(halflifeSeconds > 1.0E-12){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else{
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getDecayModeCol(const uint8_t dcyMode){
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  switch(dcyMode){
    case DECAYMODE_BETAPLUS:
      col.r = 0.8f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2BETAPLUS:
      col.r = 0.4f;
      col.g = 0.3f;
      col.b = 0.5f;
      break;
    case DECAYMODE_EC:
      col.r = 0.6f;
      col.g = 0.8f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2EC:
      col.r = 0.3f;
      col.g = 0.4f;
      col.b = 0.5f;
      break;
    case DECAYMODE_ECANDBETAPLUS:
      col.r = 0.8f;
      col.g = 0.8f;
      col.b = 1.0f;
      break;
    case DECAYMODE_BETAMINUS:
      col.r = 1.0f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_2BETAMINUS:
      col.r = 0.5f;
      col.g = 0.3f;
      col.b = 0.5f;
      break;
    case DECAYMODE_ALPHA:
    case DECAYMODE_BETAMINUS_ALPHA:
      col.r = 1.0f;
      col.g = 1.0f;
      col.b = 0.6f;
      break;
    case DECAYMODE_PROTON:
    case DECAYMODE_TWOPROTON:
    case DECAYMODE_BETAPLUS_PROTON:
    case DECAYMODE_BETAPLUS_TWOPROTON:
    case DECAYMODE_EC_PROTON:
      col.r = 1.0f;
      col.g = 0.6f;
      col.b = 0.6f;
      break;
    case DECAYMODE_NEUTRON:
    case DECAYMODE_TWONEUTRON:
    case DECAYMODE_BETAMINUS_NEUTRON:
      col.r = 0.6f;
      col.g = 0.6f;
      col.b = 1.0f;
      break;
    case DECAYMODE_IT:
      col.r = 0.6f;
      col.g = 0.8f;
      col.b = 0.8f;
      break;
    case DECAYMODE_SPONTANEOUSFISSION:
    case DECAYMODE_BETAMINUS_SPONTANEOUSFISSION:
      col.r = 0.6f;
      col.g = 1.0f;
      col.b = 0.6f;
      break;
    case (DECAYMODE_ENUM_LENGTH+1):
      //stable
      col.r = 0.0f;
      col.g = 0.0f;
      col.b = 0.0f;
      break;
    case DECAYMODE_ENUM_LENGTH:
    default:
      //no decay mode found
      col.r = 0.7f;
      col.g = 0.7f;
      col.b = 0.7f;
      break;
  }
  return col;
}

SDL_Color getDecayModeTextCol(const uint8_t dcyMode){
  const SDL_FColor bgCol = getDecayModeCol(dcyMode);
  if((bgCol.r + bgCol.g + bgCol.b) < 2.0f){
    return whiteCol8Bit;
  }
  return blackCol8Bit;
}

SDL_FColor get2PlusCol(const double e2PlusKeV){
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  if(e2PlusKeV >= 8000.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(e2PlusKeV >= 7000.0){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 6000.0){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 5000.0){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 4000.0){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 3000.0){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 2500.0){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 2000.0){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 1500.0){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 1200.0){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 900.0){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 700.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.2f;
  }else if(e2PlusKeV >= 500.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(e2PlusKeV >= 400.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(e2PlusKeV >= 300.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(e2PlusKeV >= 200.0){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(e2PlusKeV >= 100.0){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(e2PlusKeV >= 50.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(e2PlusKeV > 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  return col;
}

SDL_FColor getR42Col(const double r42){
  SDL_FColor col;
  col.r = 1.0f;
  col.g = 1.0f;
  col.b = 1.0f;
  col.a = 1.0f;
  if(r42 >= 5.0){
    col.r = 0.0f;
    col.g = 0.0f;
    col.b = 0.0f;
  }else if(r42 >= 3.8){
    col.r = 0.0f;
    col.g = 0.1f;
    col.b = 0.4f;
  }else if(r42 >= 3.5){
    col.r = 0.1f;
    col.g = 0.2f;
    col.b = 0.5f;
  }else if(r42 >= 3.4){
    col.r = 0.1f;
    col.g = 0.3f;
    col.b = 0.7f;
  }else if(r42 >= 3.3){
    col.r = 0.2f;
    col.g = 0.4f;
    col.b = 0.8f;
  }else if(r42 >= 3.2){
    col.r = 0.3f;
    col.g = 0.5f;
    col.b = 0.8f;
  }else if(r42 >= 3.1){
    col.r = 0.3f;
    col.g = 0.7f;
    col.b = 0.7f;
  }else if(r42 >= 2.8){
    col.r = 0.3f;
    col.g = 0.9f;
    col.b = 0.7f;
  }else if(r42 >= 2.5){
    col.r = 0.5f;
    col.g = 1.0f;
    col.b = 0.7f;
  }else if(r42 >= 2.2){
    col.r = 0.7f;
    col.g = 0.9f;
    col.b = 0.5f;
  }else if(r42 >= 2.1){
    col.r = 0.9f;
    col.g = 0.9f;
    col.b = 0.4f;
  }else if(r42 >= 2.0){
    col.r = 1.0f;
    col.g = 1.0f;
    col.b = 0.2f;
  }else if(r42 >= 1.9){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.4f;
  }else if(r42 >= 1.8){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.5f;
  }else if(r42 >= 1.4){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.6f;
  }else if(r42 >= 1.3){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(r42 >= 1.2){
    col.r = 1.0f;
    col.g = 0.7f;
    col.b = 0.8f;
  }else if(r42 >= 1.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }else if(r42 > 0.0){
    col.r = 1.0f;
    col.g = 0.8f;
    col.b = 0.9f;
  }
  return col;
}

uint8_t getNuclBoxLabelNumLines(const app_data *restrict dat, const uint16_t nuclInd){
  uint8_t numLines = 2;
  if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
    numLines++;
  }
  if(dat->ndat.nuclData[nuclInd].numLevels > 0){
    uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel);
    if(dat->ndat.levels[gsLevInd].numDecModes > 0){
      numLines += (uint8_t)(dat->ndat.levels[gsLevInd].numDecModes);
    }
  }
  return numLines;
}

//draws label for the R42 energy box
void drawR42BoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const double r42){
  if(r42 >= 0.0){
    char tmpStr[32];
    float drawXPos, drawYPos;
    float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*ds->chartZoomScale*ds->uiUserScale;
    if(boxHeight > 38.0f){
      drawXPos = xPos + labelMargin + (2.0f*ds->uiUserScale);
      float totalLblHeight = getTextHeight(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_R42]])/rdat->uiDPIScale;
      drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f + 4.0f*ds->uiUserScale;
      drawTextAlignedSized(rdat,drawXPos,drawYPos,col,FONTSIZE_LARGE,255,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_R42]],ALIGN_LEFT,16384); //draw label
      //handle energy label
      drawXPos = xPos + boxWidth - labelMargin;
      snprintf(tmpStr,32,"%0.2f",r42);
      float drawSpace = boxWidth  - getTextWidth(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_R42]])/rdat->uiDPIScale;
      //SDL_Log("remaining space: %f, width: %f\n",(double)drawSpace,(double)getTextWidth(rdat,FONTSIZE_LARGE,tmpStr));
      if(drawSpace > getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale){
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw energy label
      }else{
        drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_RIGHT,16384);
      }
    }
  }
}

//draws label for the 2+ energy box
void draw2PlusEnergyBoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint32_t lvlInd){
  char tmpStr[32];
  float drawXPos, drawYPos;
  float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*ds->chartZoomScale*ds->uiUserScale;
  if(boxHeight > 38.0f){
    drawXPos = xPos + labelMargin + (2.0f*ds->uiUserScale);
    float totalLblHeight = getTextHeight(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]])/rdat->uiDPIScale;
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f + 4.0f*ds->uiUserScale;
    drawTextAlignedSized(rdat,drawXPos,drawYPos,col,FONTSIZE_LARGE,255,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]],ALIGN_LEFT,16384); //draw label
    //handle energy label
    drawXPos = xPos + boxWidth - labelMargin;
    getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
    strcat(tmpStr," keV");
    float drawSpace = boxWidth  - getTextWidth(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]])/rdat->uiDPIScale;
    //SDL_Log("remaining space: %f, width: %f\n",(double)drawSpace,(double)getTextWidth(rdat,FONTSIZE_LARGE,tmpStr));
    if(drawSpace > getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw energy label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_RIGHT,16384);
    }
  }
}

//draws label for the decay mode box
void drawDecayModeBoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint8_t dcyMode){
  char tmpStr[32];
  float drawXPos, drawYPos;
  float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*ds->chartZoomScale*ds->uiUserScale;
  if(boxHeight > 38.0f){
    drawXPos = xPos + labelMargin + (2.0f*ds->uiUserScale);
    float totalLblHeight = getTextHeight(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]])/rdat->uiDPIScale;
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f + 4.0f*ds->uiUserScale;
    drawTextAlignedSized(rdat,drawXPos,drawYPos,col,FONTSIZE_LARGE,255,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]],ALIGN_LEFT,16384); //draw label
    //handle energy label
    drawXPos = xPos + boxWidth - labelMargin;
    if(dcyMode == DECAYMODE_ENUM_LENGTH){
      snprintf(tmpStr,32,"%s",dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]]);
    }else{
      snprintf(tmpStr,32,"%s",getDecayTypeShortStr(dcyMode));
    }
    float drawSpace = boxWidth  - getTextWidth(rdat,FONTSIZE_LARGE,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]])/rdat->uiDPIScale;
    //SDL_Log("remaining space: %f, width: %f\n",(double)drawSpace,(double)getTextWidth(rdat,FONTSIZE_LARGE,tmpStr));
    if(drawSpace > getTextWidth(rdat,FONTSIZE_LARGE,tmpStr)/rdat->uiDPIScale){
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw decay mode label
    }else{
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(2.5f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_RIGHT,16384);
    }
  }
}

//draws label for the isomer box
//isomerInd: which isomer is being drawn (m-number, eg. 178m1Hf vs. 178m2Hf)
void drawisomerBoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd, const uint32_t isomerLvl, const uint8_t isomerMVal){
  char tmpStr[32];
  float drawXPos, drawYPos;
  float labelMargin = NUCLBOX_LABEL_SMALLMARGIN*ds->chartZoomScale*ds->uiUserScale;
  uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  uint16_t N = (uint16_t)dat->ndat.nuclData[nuclInd].N;
  if(boxHeight > 38.0f){
    if((isomerMVal==0)||(dat->ndat.nuclData[nuclInd].numIsomerMVals <= 1)){
      snprintf(tmpStr,32,"%um",N+Z);
    }else{
      snprintf(tmpStr,32,"%um%u",N+Z,isomerMVal);
    }
    drawXPos = xPos + labelMargin;
    float totalLblHeight = (getTextHeight(rdat,FONTSIZE_SMALL,tmpStr) + getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z)))/rdat->uiDPIScale;
    drawYPos = yPos+boxHeight*0.5f - totalLblHeight*0.5f + 4.0f*ds->uiUserScale;
    drawXPos += (drawTextAlignedSized(rdat,drawXPos,drawYPos,col,FONTSIZE_SMALL,255,tmpStr,ALIGN_LEFT,16384)).w; //draw number label
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*ds->uiUserScale),col,FONTSIZE_LARGE,255,getElemStr((uint8_t)Z),ALIGN_LEFT,16384); //draw element label
    //handle half-life, spin-parity labels
    drawXPos = xPos + boxWidth - labelMargin;
    if(dat->ndat.levels[isomerLvl].numSpinParVals > 0){
      //spin-parity is known
      getSpinParStr(tmpStr,&dat->ndat,isomerLvl);
      drawTextAlignedSized(rdat,drawXPos,drawYPos-(2.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
      getHalfLifeStr(tmpStr,dat,isomerLvl,1,0,ds->useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(18.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw half-life label
    }else{
      //only half-life known
      getHalfLifeStr(tmpStr,dat,isomerLvl,1,0,ds->useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(8.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw half-life label
    }
  }
}

//draws the main label for a box on the chart of nuclides
void drawNuclBoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, const float boxHeight, SDL_Color col, const uint16_t nuclInd){
  char tmpStr[32];
  float drawXPos, drawYPos;
  float labelMargin = NUCLBOX_LABEL_MARGIN*ds->chartZoomScale*ds->uiUserScale;
  float labelSmallMargin = 2.0f*NUCLBOX_LABEL_SMALLMARGIN*ds->chartZoomScale*ds->uiUserScale;
  Uint16 maxLblWidth = (Uint16)(boxWidth - 2.0f*labelMargin);
  uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  if(ds->chartZoomScale >= 8.0f){
    uint16_t N = (uint16_t)dat->ndat.nuclData[nuclInd].N;
    snprintf(tmpStr,32,"%u",N+Z);
    float totalLblWidth = (getTextWidth(rdat,FONTSIZE_SMALL,tmpStr) + getTextWidth(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z)))/rdat->uiDPIScale;
    drawXPos = xPos+boxWidth*0.5f - totalLblWidth*0.5f;
    if(ds->chartZoomScale >= 12.0f){
      drawYPos = yPos + labelMargin;
    }else{
      float totalLblHeight = (getTextHeight(rdat,FONTSIZE_SMALL,tmpStr) - (10.0f*ds->uiUserScale) + getTextHeight(rdat,FONTSIZE_LARGE,getElemStr((uint8_t)Z)))/rdat->uiDPIScale;
      drawYPos = yPos+boxWidth*0.5f - totalLblHeight*0.5f;
    }
    drawXPos += (drawTextAlignedSized(rdat,drawXPos,drawYPos,col,FONTSIZE_SMALL,255,tmpStr,ALIGN_LEFT,16384)).w; //draw number label
    drawTextAlignedSized(rdat,drawXPos,drawYPos+(10.0f*ds->uiUserScale),col,FONTSIZE_LARGE,255,getElemStr((uint8_t)Z),ALIGN_LEFT,16384); //draw element label
    getGSHalfLifeStr(tmpStr,dat,nuclInd,ds->useLifetimes);
    if((ds->chartZoomScale >= 12.0f)&&(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) <= (maxLblWidth*rdat->uiDPIScale))){
      drawXPos = xPos + labelMargin;
      drawYPos = yPos + labelMargin;
      drawTextAlignedSized(rdat,drawXPos,drawYPos+(36.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_LEFT,16384); //draw GS half-life label
      if((ds->chartZoomScale >= 15.0f)&&(dat->ndat.nuclData[nuclInd].numLevels > 0)){
        uint8_t yOffsets = 3;
        uint8_t yOffsetLimit = 3;
        yOffsetLimit += (uint8_t)(floorf((boxHeight - 75.0f*ds->uiUserScale)/(30.0f*ds->uiUserScale)));
        if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
          getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
          uint8_t drawYOffsets = 1;
          if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
            drawYOffsets = 2;
          }
          if((yOffsets+drawYOffsets) <= yOffsetLimit){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_LEFT,maxLblWidth); //draw abundance label
            yOffsets += drawYOffsets;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_LEFT,maxLblWidth);
          }
        }
        uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel);
        for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          uint8_t drawYOffsets = 1;
          if(getTextWidth(rdat,FONTSIZE_NORMAL,tmpStr) > (maxLblWidth*rdat->uiDPIScale)){
            drawYOffsets = 2;
          }
          if(((yOffsets+drawYOffsets) <= yOffsetLimit)||((drawYOffsets == 1)&&(i==(dat->ndat.levels[gsLevInd].numDecModes-1)))){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_LEFT,maxLblWidth); //draw decay mode label
            //SDL_Log("height: %f\n",(double)height);
            yOffsets += drawYOffsets;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f*ds->uiUserScale),col,FONTSIZE_NORMAL,255,"(...)",ALIGN_LEFT,maxLblWidth);
            break;
          }
        }
        if(ds->chartZoomScale >= 18.0f){
          //draw corner label(s)
          getSpinParStr(tmpStr,&dat->ndat,gsLevInd);
          drawTextAlignedSized(rdat,xPos+boxWidth-labelSmallMargin,yPos+labelSmallMargin,col,FONTSIZE_NORMAL,255,tmpStr,ALIGN_RIGHT,16384); //draw spin-parity label
        }
      }
    }else if(ds->chartZoomScale >= 12.0f){
      drawXPos = xPos + labelMargin;
      drawYPos = yPos + labelMargin;
      drawTextAlignedSized(rdat,drawXPos,drawYPos+36.0f,col,FONTSIZE_NORMAL,255,"(...)",ALIGN_LEFT,16384);
    }
  }else{
    drawTextAlignedSized(rdat,xPos+boxWidth*0.5f,yPos+boxWidth*0.5f,col,FONTSIZE_NORMAL,255,getElemStr((uint8_t)Z),ALIGN_CENTER,16384); //draw element label only
  }
}

void drawChartOfNuclides(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){

  float minX = getMinChartN(&state->ds);
  float maxX = getMaxChartN(&state->ds);
  float minY = getMinChartZ(&state->ds);
  float maxY = getMaxChartZ(&state->ds);
  
  //SDL_Log("N range: [%0.2f %0.2f], Z range: [%0.2f %0.2f]\n",(double)minX,(double)maxX,(double)minY,(double)maxY);

  SDL_FRect rect, lowBoxRect;
  float nuclBoxWidth = DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
  float lowBoxPadding = DEFAULT_LOWBOX_PADDING*state->ds.chartZoomScale*state->ds.uiUserScale;
  float boxLineLimit = powf(state->ds.chartZoomScale/MAX_CHART_ZOOM_SCALE,2.0f)*9.0f;

  //calculate size of low box (extra info like isomers, 2+ energies etc.)
  float lowBoxHeight = 0.0f;
  if(state->ds.chartZoomScale >= 6.0f){
    lowBoxHeight = (boxLineLimit - 3)*20.0f*state->ds.uiUserScale;
    if(lowBoxHeight < 0.0f){
      lowBoxHeight = 0.0f;
    }
    lowBoxHeight += 1.5f*DEFAULT_LOWBOX_PADDING*state->ds.chartZoomScale*state->ds.uiUserScale; //minimum size
    if(lowBoxHeight > (80.0f*state->ds.uiUserScale)){
      lowBoxHeight = 80.0f*state->ds.uiUserScale; //limit low box size
    }
  }
  

  //printf("line limit: %0.3f\n",(double)boxLineLimit);
  for(uint16_t i=0;i<dat->ndat.numNucl;i++){
    if((dat->ndat.nuclData[i].flags & 3U) == OBSFLAG_OBSERVED){
      if((dat->ndat.nuclData[i].N >= (int16_t)SDL_floorf(minX))&&(dat->ndat.nuclData[i].N >= 0)){
        if(dat->ndat.nuclData[i].N <= (int16_t)SDL_ceilf(maxX)){
          if((dat->ndat.nuclData[i].Z >= (int16_t)SDL_floorf(minY))&&(dat->ndat.nuclData[i].Z >= 0)){
            if(dat->ndat.nuclData[i].Z <= (int16_t)SDL_ceilf(maxY)){
              //draw nuclide on chart
              rect.w = nuclBoxWidth;
              rect.h = rect.w;
              rect.x = ((float)dat->ndat.nuclData[i].N - minX)*rect.w;
              rect.y = (maxY - (float)dat->ndat.nuclData[i].Z)*rect.h;
              //SDL_Log("N: %i, Z: %i, i: %i, pos: [%0.2f %0.2f %0.2f %0.2f]\n",dat->ndat.nuclData[i].N,dat->ndat.nuclData[i].Z,i,(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
              //const double hl = getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i);
              if(state->chartView == CHARTVIEW_HALFLIFE){
                drawFlatRect(rdat,rect,getHalfLifeCol(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i)));
              }else if(state->chartView == CHARTVIEW_DECAYMODE){
                drawFlatRect(rdat,rect,getDecayModeCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i)));
              }else if(state->chartView == CHARTVIEW_2PLUS){
                drawFlatRect(rdat,rect,get2PlusCol(get2PlusEnergy(&dat->ndat,(uint16_t)i)));
              }else if(state->chartView == CHARTVIEW_R42){
                drawFlatRect(rdat,rect,getR42Col(getR42(&dat->ndat,(uint16_t)i)));
              }
              
              if(state->ds.chartZoomScale >= 4.0f){
                uint8_t drawingLowBox = 0;
                if(state->ds.chartZoomScale >= 6.0f){
                  //setup low box rect
                  lowBoxRect.x = rect.x + lowBoxPadding;
                  lowBoxRect.y = rect.y + rect.h - lowBoxHeight - lowBoxPadding;
                  lowBoxRect.w = nuclBoxWidth - 2.0f*lowBoxPadding;
                  lowBoxRect.h = lowBoxHeight;
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    //draw isomer box
                    uint32_t isomerLvl = dat->ndat.nuclData[i].longestIsomerLevel;
                    const double isomerHl = getLevelHalfLifeSeconds(&dat->ndat,isomerLvl);
                    if((isomerLvl != MAXNUMLVLS)&&(isomerLvl != (dat->ndat.nuclData[i].firstLevel + dat->ndat.nuclData[i].gsLevel))){
                      if((isomerHl >= 1.0E-1)||(isomerHl > getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i))){ //only show 'important' isomers on chart
                        drawingLowBox = 1;
                        SDL_FColor boxCol = getHalfLifeCol(isomerHl);
                        if(state->ds.chartZoomScale < 7.0f){
                          //handle fading in of isomer boxes
                          boxCol.a =  1.0f - (7.0f-state->ds.chartZoomScale);
                        }
                        drawFlatRect(rdat,lowBoxRect,boxCol);
                        drawisomerBoxLabel(dat,&state->ds,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,(isomerHl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i,isomerLvl,dat->ndat.nuclData[i].longestIsomerMVal);
                      }
                    }
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    //draw decay mode box
                    uint8_t dcyMode = getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i);
                    if(dcyMode < (DECAYMODE_ENUM_LENGTH+1)){
                      drawingLowBox = 1;
                      SDL_FColor boxCol = whiteCol;
                      if(state->ds.chartZoomScale < 7.0f){
                        //handle fading in of 2+ boxes
                        boxCol.a =  1.0f - (7.0f-state->ds.chartZoomScale);
                      }
                      drawFlatRect(rdat,lowBoxRect,boxCol);
                      drawDecayModeBoxLabel(dat,&state->ds,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,blackCol8Bit,dcyMode);
                    }
                  }else if(state->chartView == CHARTVIEW_2PLUS){
                    //draw 2+ energy box
                    double e2Plus = get2PlusEnergy(&dat->ndat,(uint16_t)i);
                    if(e2Plus > 0.0){
                      drawingLowBox = 1;
                      SDL_FColor boxCol = whiteCol;
                      if(state->ds.chartZoomScale < 7.0f){
                        //handle fading in of 2+ boxes
                        boxCol.a =  1.0f - (7.0f-state->ds.chartZoomScale);
                      }
                      drawFlatRect(rdat,lowBoxRect,boxCol);
                      draw2PlusEnergyBoxLabel(dat,&state->ds,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,blackCol8Bit,get2PlusLvlInd(&dat->ndat,i));
                    }
                  }else if(state->chartView == CHARTVIEW_R42){
                    //draw R42 energy box
                    double r42 = getR42(&dat->ndat,(uint16_t)i);
                    if(r42 >= 0.0){
                      drawingLowBox = 1;
                      SDL_FColor boxCol = whiteCol;
                      if(state->ds.chartZoomScale < 7.0f){
                        //handle fading in of R42 boxes
                        boxCol.a =  1.0f - (7.0f-state->ds.chartZoomScale);
                      }
                      drawFlatRect(rdat,lowBoxRect,boxCol);
                      drawR42BoxLabel(dat,&state->ds,rdat,lowBoxRect.x,lowBoxRect.y,lowBoxRect.w,lowBoxRect.h,blackCol8Bit,r42);
                    }
                  }
                }
                if(drawingLowBox){
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i) > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),getDecayModeTextCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i)),(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_2PLUS){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(get2PlusEnergy(&dat->ndat,(uint16_t)i) >= 3000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_R42){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,(rect.h-lowBoxHeight-(2.0f*lowBoxPadding)),(getR42(&dat->ndat,(uint16_t)i) >= 3.2) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }
                }else{
                  if(state->chartView == CHARTVIEW_HALFLIFE){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,rect.h,(getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i) > 1.0E3) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_DECAYMODE){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,rect.h,getDecayModeTextCol(getNuclGSMostProbableDcyMode(&dat->ndat,(uint16_t)i)),(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_2PLUS){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,rect.h,(get2PlusEnergy(&dat->ndat,(uint16_t)i) >= 3000.0) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }else if(state->chartView == CHARTVIEW_R42){
                    drawNuclBoxLabel(dat,&state->ds,rdat,rect.x,rect.y,rect.w,rect.h,(getR42(&dat->ndat,(uint16_t)i) >= 3.2) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  //draw shell closure lines
  //some fudging of line positions was needed to fix alignment with fractional scaling
  if(state->ds.drawShellClosures){
    SDL_FColor scCol = darkGrayCol;
    scCol.a = (state->ds.chartZoomScale/0.5f) - 3.0f;
    if(scCol.a > 0.0f){
      if(scCol.a > 1.0f){
        scCol.a = 1.0f;
      }
      for(uint8_t i=0; i<NUMSHELLCLOSURES; i++){
        //neutron shell closures
        if(shellClosureValues[i] <= dat->ndat.maxN){
          if(shellClosureValues[i] >= (minX - 1.0f)){
            if(shellClosureValues[i] <= (maxX + 1.0f)){
              if(dat->ndat.minZforN[shellClosureValues[i]] > minY){
                if(dat->ndat.maxZforN[shellClosureValues[i]] < maxY){
                  //top and bottom of shell closure are both on screen
                  drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }else{
                  //bottom of shell closure is on screen
                  drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }
              }else if(dat->ndat.maxZforN[shellClosureValues[i]] < maxY){
                //top of shell closure is on screen
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //top and bottom of shell closure are both off screen
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }
          }
        }
        //proton shell closures
        if(shellClosureValues[i] <= dat->ndat.maxZ){
          if(shellClosureValues[i] >= (minY - 1.0f)){
            if(shellClosureValues[i] <= (maxY + 1.0f)){
              if(dat->ndat.minNforZ[shellClosureValues[i]] > minX){
                if(dat->ndat.maxNforZ[shellClosureValues[i]] < maxX){
                  //left and right edges of shell closure are both on screen
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }else{
                  //left edge of shell closure is on screen
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                  drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                }
              }else if(dat->ndat.maxNforZ[shellClosureValues[i]] < maxX){
                //right edge of shell closure is on screen
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //left and right edges of shell closure are both off screen
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }
          }
        }
      }
    }
  }
  

  //handle highlighting selected nuclide
  if(state->chartSelectedNucl != MAXNUMNUCL){
    float alpha = 1.0f;
    if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLHIGHLIGHT_HIDE)){
      alpha = (float)(juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLHIGHLIGHT_HIDE]/UI_ANIM_LENGTH));
    }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLHIGHLIGHT_SHOW)){
      alpha = (float)(juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLHIGHLIGHT_SHOW]/UI_ANIM_LENGTH));
    }
    SDL_FColor selectionCol = whiteCol;
    selectionCol.a = alpha;
    if(state->ds.chartZoomScale >= 2.0f){
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*nuclBoxWidth - CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.w =  nuclBoxWidth + 2.0f*CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*nuclBoxWidth - CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale;
      rect.h = rect.w;
      drawSelectionRect(rdat,rect,selectionCol,2.0f*CHART_SHELLCLOSURELINE_THICKNESS*state->ds.uiUserScale);
    }else{
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*nuclBoxWidth;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*nuclBoxWidth;
      drawFlatRect(rdat,rect,selectionCol);
    }
  }

  //draw x and y axes
  rect.w = state->ds.windowXRes;
  rect.h = CHART_AXIS_DEPTH*state->ds.uiUserScale;
  rect.x = 0;
  rect.y = state->ds.windowYRes - rect.h;
  drawFlatRect(rdat,rect,whiteTransparentCol);
  rect.w = rect.h;
  rect.h = rect.y;
  rect.y = 0;
  drawFlatRect(rdat,rect,whiteTransparentCol);
  drawTextAligned(rdat,CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_LARGE,"Z",ALIGN_CENTER);
  drawTextAligned(rdat,state->ds.windowXRes - CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,state->ds.windowYRes - CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale,blackCol8Bit,FONTSIZE_LARGE,"N",ALIGN_CENTER);
  //draw ticks
  char tmpStr[32];
  rect.y = state->ds.windowYRes - (CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale);
  float tickSpacing = getAxisTickSpacing(roundf(maxX - minX)); //round to remove jitter when panning the chart and the axis range is very close to an integer value
  for(float i=(minX-fmodf(minX,tickSpacing));i<maxX;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(SDL_floorf(i));
      if((i<MAX_MASS_NUM)&&(i<=MAX_NEUTRON_NUM)){
        rect.x = (i + 0.5f - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
        if(rect.x < (state->ds.windowXRes - CHART_AXIS_DEPTH*state->ds.uiUserScale)){ //dodge axis label
          snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,blackCol8Bit,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }
  rect.x = CHART_AXIS_DEPTH*0.5f*state->ds.uiUserScale;
  tickSpacing = getAxisTickSpacing(roundf(maxY - minY));
  for(float i=(minY-fmodf(minY,tickSpacing));i<maxY;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(SDL_floorf(i));
      if((i<MAX_MASS_NUM)&&(i<MAX_PROTON_NUM)){
        rect.y = (maxY - i + 0.5f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*state->ds.uiUserScale;
        if(rect.y > CHART_AXIS_DEPTH*state->ds.uiUserScale){ //dodge axis label
          snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,blackCol8Bit,FONTSIZE_NORMAL,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }

}

//draw some stats, ie. FPS overlay and further diagnostic info
void drawPerformanceStats(const ui_theme_rules *restrict uirules, const app_state *restrict state, const thread_manager_state *restrict tms, resource_data *restrict rdat, const float deltaTime){

  //draw background
  SDL_FRect perfOvRect;
  perfOvRect.w = (628.0f*rdat->uiScale);
  perfOvRect.h = (192.0f*rdat->uiScale);
  perfOvRect.x = 0.0f;
  perfOvRect.y = 0.0f;
  
  SDL_SetRenderDrawColor(rdat->renderer,255,255,255,150);
  SDL_RenderFillRect(rdat->renderer,&perfOvRect);

  //draw text
  char txtStr[256];
  SDL_snprintf(txtStr,256,"Debug stats (press <P> to toggle this display)");
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR,txtStr);
  SDL_snprintf(txtStr,256,"UI scale: %0.2f, DPI scale: %0.2f, Resolution: %u x %u (logical), %u x %u (actual)",(double)rdat->uiScale,(double)rdat->uiDPIScale,state->ds.windowXRes,state->ds.windowYRes,state->ds.windowXRenderRes,state->ds.windowYRenderRes);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Zoom scale: %4.1f, UI state: %u",(double)state->ds.chartZoomScale,state->uiState);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+2*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Active threads: %2u",tms->numThreads);
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+3*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"FPS: %4.1f",1.0/((double)deltaTime));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+4*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
  SDL_snprintf(txtStr,256,"Frame time (ms): %4.3f",(double)(deltaTime*1000.0f));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+5*PERF_OVERLAY_Y_SPACING*state->ds.uiUserScale,txtStr);
}

uint8_t getHighlightState(const app_state *restrict state, const uint8_t uiElem){
  if(state->clickedUIElem == uiElem){
    return HIGHLIGHT_SELECTED;
  }
  if(state->mouseoverElement==uiElem){
    if(state->mouseholdElement==uiElem){
      return HIGHLIGHT_SELECTED;
    }else{
      return HIGHLIGHT_MOUSEOVER;
    }
  }else{
    if(state->mouseholdElement==uiElem){
      return HIGHLIGHT_SELECTED;
    }
    return HIGHLIGHT_NORMAL;
  }
}

//return value: width of header text
void drawInfoBoxHeader(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float x, const float y, const Uint8 alpha, const uint16_t nuclInd){
  char tmpStr[32];
  float drawXPos = (float)(x + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*ds->uiUserScale);
  float drawYPos = (float)(y + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*ds->uiUserScale);
  uint16_t nucA = (uint16_t)(dat->ndat.nuclData[nuclInd].Z + dat->ndat.nuclData[nuclInd].N);
  snprintf(tmpStr,32,"%u",nucA);
  drawXPos += (drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_SMALL,alpha,tmpStr,ALIGN_LEFT,16384)).w; //draw number label
  if((uint16_t)(dat->ndat.nuclData[nuclInd].Z <= 1)&&(dat->ndat.nuclData[nuclInd].N <= 2)){
    snprintf(tmpStr,32,"%s (%s)",getElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z),getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N));
  }else if((uint16_t)(dat->ndat.nuclData[nuclInd].Z == 0)&&(dat->ndat.nuclData[nuclInd].N <= 5)){
    snprintf(tmpStr,32,"%s (%s)",getElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z),getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N));
  }else{
    snprintf(tmpStr,32,"%s (%s-%u)",getElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z),getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,(uint8_t)dat->ndat.nuclData[nuclInd].N),nucA);
  }
  drawTextAlignedSized(rdat,drawXPos+(2.0f*ds->uiUserScale),drawYPos+(6.0f*ds->uiUserScale),blackCol8Bit,FONTSIZE_LARGE,alpha,tmpStr,ALIGN_LEFT,16384); //draw element label
}

void drawNuclFullInfoBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){

  float txtYOffset = 0.0f;
  Uint8 txtAlpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_TXTFADEOUT)){
    txtYOffset = (80.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEOUT]/SHORT_UI_ANIM_LENGTH));
    txtAlpha = (uint8_t)(255.0f*juice_smoothStop2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEOUT]/SHORT_UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_TXTFADEIN)){
    txtYOffset = (80.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEIN]/UI_ANIM_LENGTH));
    txtAlpha = (uint8_t)(255.0f*juice_smoothStart2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_TXTFADEIN]/UI_ANIM_LENGTH));
  }
  
  //level and gamma data
  char tmpStr[32];
  SDL_FRect rect;
  SDL_FColor dividerLineCol = lightGrayCol;
  dividerLineCol.a = txtAlpha/255.0f;
  uint8_t drawMode = 0; //0=draw all columns, 1=skip multipolarity, 2=skip multipolarity, final spin
  float allColWidth = state->ds.fullInfoAllColWidth*state->ds.uiUserScale;
  if(allColWidth > (state->ds.windowXRes - 4*UI_PADDING_SIZE*state->ds.uiUserScale)){
    drawMode = 1;
    allColWidth = state->ds.fullInfoAllColWidthExclM*state->ds.uiUserScale;
    if(allColWidth > (state->ds.windowXRes - 4*UI_PADDING_SIZE*state->ds.uiUserScale)){
      drawMode = 2;
      allColWidth = state->ds.fullInfoAllColWidthExcluMFinalJpi*state->ds.uiUserScale;
    }
  }
  float origDrawXPos = (state->ds.windowXRes - allColWidth)/2.0f;
  float drawXPos = origDrawXPos;
  float drawYPos = (NUCL_FULLINFOBOX_LEVELLIST_POS_Y - NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.nuclFullInfoScrollY)*state->ds.uiUserScale + txtYOffset;
  rect.x = origDrawXPos - 2*UI_PADDING_SIZE*state->ds.uiUserScale;
  rect.w = allColWidth + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  //SDL_Log("drawYPos: %f\n",(double)drawYPos);
  float levelStartDrawPos;
  for(uint32_t lvlInd = dat->ndat.nuclData[nuclInd].firstLevel; lvlInd<(dat->ndat.nuclData[nuclInd].firstLevel+dat->ndat.nuclData[nuclInd].numLevels); lvlInd++){
    drawXPos = origDrawXPos;
    uint16_t numLines = getNumDispLinesForLvl(&dat->ndat,lvlInd);
    if(((drawYPos + NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale*numLines) >= NUCL_FULLINFOBOX_LEVELLIST_POS_Y)&&(drawYPos <= state->ds.windowYRes)){
      
      const double hl = getLevelHalfLifeSeconds(&dat->ndat,lvlInd);
      if(hl > 1.0E-9){
        //highlight isomers and stable states
        //first check that the lifetime is not an upper limit
        if((((dat->ndat.levels[lvlInd].halfLife.format >> 5U) & 15U) != VALUETYPE_LESSTHAN)&&(((dat->ndat.levels[lvlInd].halfLife.format >> 5U) & 15U) != VALUETYPE_LESSOREQUALTHAN)){
          
          rect.h = (NUCL_INFOBOX_SMALLLINE_HEIGHT*numLines)*state->ds.uiUserScale;
          rect.y = drawYPos;
          SDL_FColor rectCol = getHalfLifeCol(hl);
          if(txtAlpha != 255){
            rectCol.a *= txtAlpha/255.0f;
          }
          drawFlatRect(rdat,rect,rectCol);
        }
        
      }
      
      levelStartDrawPos = drawYPos;
      if(lvlInd == (dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel)){
        getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,0);
      }else{
        getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
      }
      drawTextAlignedSized(rdat,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      //handle special level info
      uint8_t slInd = (uint8_t)((dat->ndat.levels[lvlInd].format >> 1U) & 127U);
      if(slInd > 0){
        drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
        char slStr[64];
        SDL_snprintf(slStr,64,"  (%s)",getSpecialLvlStr(dat,slInd));
        //SDL_Log("%s\n",tmpStr);
        drawTextAlignedSized(rdat,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,slStr,ALIGN_LEFT,16384);
        drawYPos = levelStartDrawPos;
      }
      drawXPos += state->ds.fullInfoElevelColWidth*state->ds.uiUserScale;
      getSpinParStr(tmpStr,&dat->ndat,lvlInd);
      drawTextAlignedSized(rdat,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      drawXPos += state->ds.fullInfoJpiColWidth*state->ds.uiUserScale;
      getHalfLifeStr(tmpStr,dat,lvlInd,1,0,state->ds.useLifetimes);
      drawTextAlignedSized(rdat,drawXPos,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384);
      if(dat->ndat.levels[lvlInd].numDecModes > 0){
        for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
          drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
          //SDL_Log("%s\n",tmpStr);
          drawTextAlignedSized(rdat,drawXPos+(2*UI_PADDING_SIZE*state->ds.uiUserScale),drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw decay mode label
        }
      }
      drawXPos += state->ds.fullInfoHlColWidth*state->ds.uiUserScale;
      if(dat->ndat.levels[lvlInd].numTran > 0){
        drawYPos = levelStartDrawPos;
        for(uint16_t i=0; i<dat->ndat.levels[lvlInd].numTran; i++){
          float drawXPosTran = drawXPos;
          getGammaEnergyStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
          drawTextAlignedSized(rdat,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition energy label
          drawXPosTran += state->ds.fullInfoEgammaColWidth*state->ds.uiUserScale;
          getGammaIntensityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i),1);
          drawTextAlignedSized(rdat,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition intensity label
          drawXPosTran += state->ds.fullInfoIgammaColWidth*state->ds.uiUserScale;
          if(drawMode == 0){
            getGammaMultipolarityStr(tmpStr,&dat->ndat,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
            drawTextAlignedSized(rdat,drawXPosTran,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw transition multipolarity label
            drawXPosTran += state->ds.fullInfoMgammaColWidth*state->ds.uiUserScale;
          }
          if(dat->ndat.tran[(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i)].finalLvlOffset != 0){
            float drawXPosFL = drawXPosTran;
            uint32_t finalLvlInd = getFinalLvlInd(&dat->ndat,lvlInd,(uint32_t)(dat->ndat.levels[lvlInd].firstTran + i));
            getLvlEnergyStr(tmpStr,&dat->ndat,finalLvlInd,0);
            drawTextAlignedSized(rdat,drawXPosFL,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_LEFT,16384); //draw final level energy label
            if(drawMode <= 1){
              drawXPosFL += (state->ds.fullInfoFinalElevelColWidth+state->ds.fullInfoFinalJpiColWidth)*state->ds.uiUserScale;
              getSpinParStr(tmpStr,&dat->ndat,finalLvlInd);
              drawTextAlignedSized(rdat,drawXPosFL,drawYPos,(hl > 1.0E3) ? whiteCol8Bit : blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384); //draw final level spin-parity label
            }
          }

          drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
        }
      }
      drawYPos = levelStartDrawPos + numLines*NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
    }else{
      if(drawYPos > state->ds.windowYRes){
        break;
      }
      drawYPos += numLines*NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
    }
    //draw divider line
    drawLine(rdat,rect.x,drawYPos,rect.x+rect.w,drawYPos,NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale,dividerLineCol,dividerLineCol);
    
  }

  //scroll bar
  if(state->ds.nuclFullInfoMaxScrollY > 0){
    rect.x = (float)(state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
    rect.y = (float)(state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
    rect.w = (float)(state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
    rect.h = (float)(state->ds.uiElemHeight[UIELEM_NUCL_FULLINFOBOX_SCROLLBAR]);
    const float sbPos = state->ds.nuclFullInfoScrollY/state->ds.nuclFullInfoMaxScrollY;
    const float sbViewSize = (float)(getNumScreenLvlDispLines(&state->ds))/(float)(getNumTotalLvlDispLines(&dat->ndat,state));
    const float sbAlpha = (float)(txtAlpha/255.0f);
    //SDL_Log("x: %f, y: %f, w: %f. h: %f\n",(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
    //SDL_Log("pos: %f, view size: %f, alpha: %f\n",(double)sbPos,(double)sbViewSize,(double)sbAlpha);
    drawScrollBar(&dat->rules.themeRules,rdat,rect,getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_SCROLLBAR),sbAlpha,sbPos,sbViewSize);
  }
  

  //rect to hide over-scrolled level info
  rect.x = 0.0f;
  rect.y = 0.0f;
  rect.w = state->ds.windowXRes;
  rect.h = NUCL_FULLINFOBOX_LEVELLIST_POS_Y*state->ds.uiUserScale;
  drawFlatRect(rdat,rect,dat->rules.themeRules.bgCol);
  //rect underneath column titles
  SDL_FColor tableHeaderRectCol = {0.92f,0.92f,0.92f,txtAlpha/255.0f};
  rect.y = (NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y - 2*UI_PADDING_SIZE)*state->ds.uiUserScale + txtYOffset;
  rect.h = (NUCL_FULLINFOBOX_LEVELLIST_POS_Y - NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y + 2*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawFlatRect(rdat,rect,tableHeaderRectCol);
  //draw divider line
  drawLine(rdat,0.0f,rect.y+rect.h,state->ds.windowXRes,rect.y+rect.h,NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale,dividerLineCol,dividerLineCol);

  //header
  char descStr[64];
  drawInfoBoxHeader(dat,&state->ds,rdat,0.0f,0.0f,255,nuclInd);
  //proton and neutron numbers, abundance
  drawYPos = NUCL_FULLINFOBOX_NZVALS_POS_Y*state->ds.uiUserScale + txtYOffset;
  SDL_snprintf(descStr,64,"%s: %3i, %s: %3i",dat->strings[dat->locStringIDs[LOCSTR_PROTONSDESC]],dat->ndat.nuclData[nuclInd].Z,dat->strings[dat->locStringIDs[LOCSTR_NEUTRONSDESC]],dat->ndat.nuclData[nuclInd].N);
  drawTextAlignedSized(rdat,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,descStr,ALIGN_LEFT,16384);
  drawYPos += 18.0f*state->ds.uiUserScale;
  if(dat->ndat.nuclData[nuclInd].abundance.val > 0.0f){
    getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
    SDL_snprintf(descStr,64,"%s of %s on Earth",tmpStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,255));
    drawTextAlignedSized(rdat,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,descStr,ALIGN_LEFT,16384);
  }else{
    drawTextAlignedSized(rdat,NUCL_FULLINFOBOX_NZVALS_POS_X*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_SMALL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_NOTNATURAL]],ALIGN_LEFT,16384);
  }
  
  //Q-values
  drawXPos = state->ds.windowXRes - NUCL_FULLINFOBOX_QVAL_POS_XR*state->ds.uiUserScale;
  drawYPos = NUCL_FULLINFOBOX_QVAL_POS_Y*state->ds.uiUserScale + txtYOffset;
  if(dat->ndat.nuclData[nuclInd].sp.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].sp,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_SP]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].sp.unit));
    rect = drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
    drawXPos -= rect.w + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  }
  if(dat->ndat.nuclData[nuclInd].sn.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].sn,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_SN]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].sn.unit));
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
  }
  drawXPos = state->ds.windowXRes - NUCL_FULLINFOBOX_QVAL_POS_XR*state->ds.uiUserScale;
  drawYPos += 20.0f*state->ds.uiUserScale;
  if(dat->ndat.nuclData[nuclInd].qbeta.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].qbeta,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_QBETAMNUS]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].qbeta.unit));
    rect = drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
    drawXPos -= rect.w + 4*UI_PADDING_SIZE*state->ds.uiUserScale;
  }
  if(dat->ndat.nuclData[nuclInd].qalpha.val != 0.0f){
    getQValStr(descStr,dat->ndat.nuclData[nuclInd].qalpha,1);
    SDL_snprintf(tmpStr,32,"%s=%s %s",dat->strings[dat->locStringIDs[LOCSTR_QALPHA]],descStr,getValueUnitShortStr(dat->ndat.nuclData[nuclInd].qalpha.unit));
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,tmpStr,ALIGN_RIGHT,16384);
  }
  

  //draw column title strings
  drawXPos = origDrawXPos;
  drawYPos = NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y*state->ds.uiUserScale + txtYOffset;
  drawTextAlignedSized(rdat,drawXPos - 2*UI_PADDING_SIZE*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LEVELINFO_HEADER]],ALIGN_LEFT,16384);
  drawYPos += (NUCL_INFOBOX_BIGLINE_HEIGHT + 0.5f*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_KEV]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoElevelColWidth*state->ds.uiUserScale;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_JPI]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoJpiColWidth*state->ds.uiUserScale;
  if(state->ds.useLifetimes){
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LIFETIME]],ALIGN_LEFT,16384);
  }else{
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_HALFLIFE]],ALIGN_LEFT,16384);
  }
  drawXPos += state->ds.fullInfoHlColWidth*state->ds.uiUserScale;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_GAMMA]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoEgammaColWidth*state->ds.uiUserScale;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_INTENSITY_GAMMA]],ALIGN_LEFT,16384);
  drawXPos += state->ds.fullInfoIgammaColWidth*state->ds.uiUserScale;
  if(drawMode == 0){
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MULTIPOLARITY_GAMMA]],ALIGN_LEFT,16384);
    drawXPos += state->ds.fullInfoMgammaColWidth*state->ds.uiUserScale;
  }
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_FINALLEVEL]],ALIGN_LEFT,16384);

  //back button
  drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],state->ds.uiElemPosY[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],state->ds.uiElemWidth[UIELEM_NUCL_FULLINFOBOX_BACKBUTTON],getHighlightState(state,UIELEM_NUCL_FULLINFOBOX_BACKBUTTON),255,UIICON_DOWNARROWS,dat->strings[dat->locStringIDs[LOCSTR_BACKTOSUMMARY]]);
}

void drawNuclInfoBox(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){
  
  uint16_t yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_HIDE)){
    yOffset = (uint16_t)(300.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_SHOW)){
    yOffset = (uint16_t)(300.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]/(UI_ANIM_LENGTH)));
  }
  Uint8 alpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
    alpha = (uint8_t)(255.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
    alpha = (uint8_t)(255.0f*juice_smoothStop2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH));
  }

  //draw panel background
  SDL_FRect infoBoxPanelRect;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
    //expand from normal size to full screen
    float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH);
    infoBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX]*(1.0f - animFrac);
    infoBoxPanelRect.y = (state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + yOffset)*(1.0f - animFrac);
    infoBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] + animFrac*(state->ds.windowXRes - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    infoBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX] + animFrac*(state->ds.windowYRes - state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX]);
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
    //contract from full screen to normal size
    float animFrac = juice_smoothStop3(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH);
    infoBoxPanelRect.x = 0.0f + animFrac*(state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX]);
    infoBoxPanelRect.y = 0.0f + animFrac*(state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + yOffset);
    infoBoxPanelRect.w = state->ds.windowXRes + animFrac*(state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX] - state->ds.windowXRes);
    infoBoxPanelRect.h = state->ds.windowYRes + animFrac*(state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX] - state->ds.windowYRes);
  }else{
    //normal sized info box
    infoBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX];
    infoBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + yOffset;
    infoBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX];
    infoBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX];
  }
  drawPanelBG(&dat->rules.themeRules,rdat,infoBoxPanelRect,1.0f);

  //draw column title strings
  char tmpStr[32];
  float drawXPos = (float)(infoBoxPanelRect.x + (state->ds.infoBoxEColOffset)*state->ds.uiUserScale);
  float drawYPos = (float)(infoBoxPanelRect.y + (state->ds.infoBoxEColOffset + 34.0f)*state->ds.uiUserScale);
  if(dat->ndat.nuclData[nuclInd].abundance.val > 0.0f){
    char abundanceStr[64];
    getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
    SDL_snprintf(abundanceStr,64,"...is %s of %s on Earth.",tmpStr,getFullElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z,255));
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_SMALL,alpha,abundanceStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
		drawYPos += NUCL_INFOBOX_ABUNDANCE_LINE_HEIGHT*state->ds.uiUserScale;
	}else{
    drawYPos += 6.0f*state->ds.uiUserScale;
  }
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_GM_STATE]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawYPos += NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_ENERGY_KEV]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_JPI]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  if(state->ds.useLifetimes){
    drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_LIFETIME]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  }else{
    drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_HALFLIFE]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  }
  drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_DECAYMODE]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);

  //draw divider line
  drawYPos += 0.9f*NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  drawLine(rdat,drawXPos,drawYPos,drawXPos + infoBoxPanelRect.w - 2.0f*state->ds.infoBoxEColOffset*state->ds.uiUserScale,drawYPos,NUCL_FULLINFOBOX_DIVIDER_LINE_THICKNESS*state->ds.uiUserScale,lightGrayCol,lightGrayCol);

  //ground state
  drawYPos += 0.2f*NUCL_INFOBOX_BIGLINE_HEIGHT*state->ds.uiUserScale;
  uint32_t lvlInd = dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel;
  getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,0);
  drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  getSpinParStr(tmpStr,&dat->ndat,lvlInd);
  drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  getHalfLifeStr(tmpStr,dat,lvlInd,1,1,state->ds.useLifetimes);
  drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
    drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,"N/A",ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw no decay mode label
    drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
  }else{
    if(dat->ndat.levels[lvlInd].numDecModes > 0){
      for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
        getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
        //SDL_Log("%s\n",tmpStr);
        drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw decay mode label
        drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
      }
    }else{
      drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,dat->strings[dat->locStringIDs[LOCSTR_UNKNOWN]],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw decay mode label
      drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
    }
    
  }

  //longest isomer
  //SDL_Log("Isomer index: %u\n",dat->ndat.nuclData[nuclInd].longestIsomerLevel);
  lvlInd = dat->ndat.nuclData[nuclInd].longestIsomerLevel;
  if((lvlInd != MAXNUMLVLS)&&(lvlInd != (dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel))){
    drawYPos += ((NUCL_INFOBOX_BIGLINE_HEIGHT - NUCL_INFOBOX_SMALLLINE_HEIGHT)*state->ds.uiUserScale);
    getLvlEnergyStr(tmpStr,&dat->ndat,lvlInd,1);
    drawTextAlignedSized(rdat,drawXPos,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    getSpinParStr(tmpStr,&dat->ndat,lvlInd);
    drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxJpiColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    getHalfLifeStr(tmpStr,dat,lvlInd,1,1,state->ds.useLifetimes);
    drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxHlColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
      drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,"N/A",ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw no decay mode label
    }else{
      for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
        getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
        //SDL_Log("%s\n",tmpStr);
        drawTextAlignedSized(rdat,drawXPos+state->ds.infoBoxDcyModeColOffset*state->ds.uiUserScale,drawYPos,blackCol8Bit,FONTSIZE_NORMAL,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw decay mode label
        drawYPos += NUCL_INFOBOX_SMALLLINE_HEIGHT*state->ds.uiUserScale;
      }
    }
  }

  //header
  drawInfoBoxHeader(dat,&state->ds,rdat,infoBoxPanelRect.x,infoBoxPanelRect.y,255,nuclInd);

  //all level info button
  updateSingleUIElemPosition(dat,state,rdat,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON);
  if((state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND))||(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT))){ 
    drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],getHighlightState(state,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON),255,UIICON_UPARROWS,dat->strings[dat->locStringIDs[LOCSTR_ALLLEVELS]]);
  }else{
    drawXPos = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] + 0.5f*(infoBoxPanelRect.w - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    drawYPos = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON] + yOffset;
    drawIconAndTextButton(&dat->rules.themeRules,rdat,(uint16_t)drawXPos,(uint16_t)drawYPos,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON],getHighlightState(state,UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON),255,UIICON_UPARROWS,dat->strings[dat->locStringIDs[LOCSTR_ALLLEVELS]]);
  }
  
  //close button/icon
  alpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_EXPAND)){
    //expand from normal size to full screen
    alpha = (uint8_t)(255.0f*(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_EXPAND]/SHORT_UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_CONTRACT)){
    alpha = (uint8_t)(255.0f*(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_CONTRACT]/UI_ANIM_LENGTH));
  }
  drawXPos = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] + 0.5f*(infoBoxPanelRect.w - state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawYPos = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX_CLOSEBUTTON] + infoBoxPanelRect.y - state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX];
  drawIcon(&dat->rules.themeRules,rdat,(uint16_t)drawXPos,(uint16_t)drawYPos,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX_CLOSEBUTTON],getHighlightState(state,UIELEM_NUCL_INFOBOX_CLOSEBUTTON),alpha,UIICON_CLOSE);

  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE],alpha);
}

void drawMessageBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  uint16_t yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (uint16_t)(100.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (uint16_t)(100.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  
  SDL_FRect msgBoxPanelRect;
  msgBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_MSG_BOX];
  msgBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_MSG_BOX] + yOffset;
  msgBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_MSG_BOX];
  msgBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_MSG_BOX];
  drawPanelBG(&dat->rules.themeRules,rdat,msgBoxPanelRect,alpha);

  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+MESSAGE_BOX_HEADERTXT_Y,dat->rules.themeRules.textColNormal,FONTSIZE_LARGE,(uint8_t)SDL_floorf(alpha*255.0f),state->msgBoxHeaderTxt,ALIGN_CENTER,(Uint16)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+(msgBoxPanelRect.h/2),dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,(uint8_t)SDL_floorf(alpha*255.0f),state->msgBoxTxt,ALIGN_CENTER,(Uint16)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
  drawTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_MSG_BOX_OK_BUTTON],state->ds.uiElemPosY[UIELEM_MSG_BOX_OK_BUTTON]+yOffset,state->ds.uiElemWidth[UIELEM_MSG_BOX_OK_BUTTON],getHighlightState(state,UIELEM_MSG_BOX_OK_BUTTON),(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_OK]]);
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE],alpha);
}

void drawAboutBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-100.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (100.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  
  SDL_FRect aboutBoxPanelRect;
  aboutBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_ABOUT_BOX];
  aboutBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_ABOUT_BOX] + yOffset;
  aboutBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_ABOUT_BOX];
  aboutBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_ABOUT_BOX];
  drawPanelBG(&dat->rules.themeRules,rdat,aboutBoxPanelRect,alpha);

  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_HEADERTXT_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_LARGE,(uint8_t)SDL_floorf(alpha*255.0f),dat->rules.appName,ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 2*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_VERSION_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_VERSION]],ALIGN_CENTER,16384);
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR1_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_1]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 12*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR2_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_2]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 14*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR3_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_SMALL,(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_3]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 14*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,aboutBoxPanelRect.x+(aboutBoxPanelRect.w/2),aboutBoxPanelRect.y+ABOUT_BOX_STR4_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_ABOUTSTR_4]],ALIGN_CENTER,(Uint16)(aboutBoxPanelRect.w - 12*UI_PADDING_SIZE*state->ds.uiUserScale));
  drawTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_ABOUT_BOX_OK_BUTTON],(uint16_t)(state->ds.uiElemPosY[UIELEM_ABOUT_BOX_OK_BUTTON]+yOffset),state->ds.uiElemWidth[UIELEM_ABOUT_BOX_OK_BUTTON],getHighlightState(state,UIELEM_ABOUT_BOX_OK_BUTTON),(uint8_t)SDL_floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_OK]]);
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE],alpha);
}

uint16_t getStrIndForUIScale(const app_data *restrict dat, const drawing_state *restrict ds){
  switch(ds->interfaceSizeInd){
    case UISCALE_SMALL:
      return dat->locStringIDs[LOCSTR_SMALL];
      break;
    case UISCALE_LARGE:
      return dat->locStringIDs[LOCSTR_LARGE];
      break;
    case UISCALE_HUGE:
      return dat->locStringIDs[LOCSTR_HUGE];
      break;
    case UISCALE_NORMAL:
    default:
      return dat->locStringIDs[LOCSTR_DEFAULT];
      break;
  }
}

void drawPrefsDialog(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-100.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MODAL_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (100.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  
  SDL_FRect prefsDialogPanelRect;
  prefsDialogPanelRect.x = state->ds.uiElemPosX[UIELEM_PREFS_DIALOG];
  prefsDialogPanelRect.y = state->ds.uiElemPosY[UIELEM_PREFS_DIALOG] + yOffset;
  prefsDialogPanelRect.w = state->ds.uiElemWidth[UIELEM_PREFS_DIALOG];
  prefsDialogPanelRect.h = state->ds.uiElemHeight[UIELEM_PREFS_DIALOG];
  drawPanelBG(&dat->rules.themeRules,rdat,prefsDialogPanelRect,alpha);

  uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+PREFS_DIALOG_HEADERTXT_X*state->ds.uiUserScale,prefsDialogPanelRect.y+PREFS_DIALOG_HEADERTXT_Y*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_LARGE,alpha8,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_PREFS]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_SHELLCLOSURE]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(2*PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_LIFETIME]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_TILE_SIZE+2*UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f+(3*PREFS_DIALOG_PREF_Y_SPACING)+(2*UI_PADDING_SIZE))*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_UIANIM]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawTextAlignedSized(rdat,prefsDialogPanelRect.x+(PREFS_DIALOG_PREFCOL1_X+UI_PADDING_SIZE)*state->ds.uiUserScale,prefsDialogPanelRect.y+(PREFS_DIALOG_PREFCOL1_Y+6.0f)*state->ds.uiUserScale,dat->rules.themeRules.textColNormal,FONTSIZE_NORMAL,alpha8,dat->strings[dat->locStringIDs[LOCSTR_PREF_UISCALE]],ALIGN_LEFT,(Uint16)(prefsDialogPanelRect.w));
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX],(uint16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX),alpha8,state->ds.drawShellClosures);
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX],(uint16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX),alpha8,state->ds.useLifetimes);
  drawCheckbox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX],(uint16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX],getHighlightState(state,UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX),alpha8,state->ds.useUIAnimations);
  drawDropDownTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN],(uint16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN]+yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN],getHighlightState(state,UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN),alpha8,dat->strings[getStrIndForUIScale(dat,&state->ds)]);
  //close button
  drawIcon(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_PREFS_DIALOG_CLOSEBUTTON],(uint16_t)(state->ds.uiElemPosY[UIELEM_PREFS_DIALOG_CLOSEBUTTON] + yOffset),state->ds.uiElemWidth[UIELEM_PREFS_DIALOG_CLOSEBUTTON],getHighlightState(state,UIELEM_PREFS_DIALOG_CLOSEBUTTON),alpha8,UIICON_CLOSE);
  
  //SDL_Log("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MODAL_BOX_HIDE],alpha);
}

void drawUIScaleMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_UISCALE_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_UISCALE_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU];
  drawPanelBG(&dat->rules.themeRules,rdat,drawRect,alpha);
  
  //draw menu item highlight
  if(state->ds.timeLeftInUIAnimation[UIANIM_UISCALE_MENU_HIDE]==0.0f){
    for(uint8_t i=1;i<=UISCALE_ENUM_LENGTH;i++){
      drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU-i];
      drawRect.y = (state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU-i] + yOffset);
      drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU-i];
      drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU-i];
      switch(getHighlightState(state,UIELEM_PREFS_UISCALE_MENU-i)){
        case HIGHLIGHT_SELECTED:
          drawFlatRect(rdat,drawRect,dat->rules.themeRules.modSelectedCol);
          break;
        case HIGHLIGHT_MOUSEOVER:
          drawFlatRect(rdat,drawRect,dat->rules.themeRules.modMouseOverCol);
          break;
        case HIGHLIGHT_NORMAL:
        default:
          break;
      }
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_PREFS_UISCALE_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PREFS_UISCALE_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PREFS_UISCALE_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PREFS_UISCALE_MENU];
  Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 0.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_SMALL]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 1.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_DEFAULT]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 2.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_LARGE]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 3.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_HUGE]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));

}

void drawSearchMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_SEARCH_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_SEARCH_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_SEARCH_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  uint8_t alpha8 = (uint8_t)SDL_floorf(alpha*255.0f);
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_SEARCH_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_SEARCH_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_SEARCH_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_SEARCH_MENU];
  drawPanelBG(&dat->rules.themeRules,rdat,drawRect,alpha);

  //draw entry box
  if(strncmp(state->ss.searchString,"",256)==0){
    drawIconAndTextEntryBox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_ENTRYBOX],(uint16_t)(state->ds.uiElemPosY[UIELEM_SEARCH_ENTRYBOX]+yOffset),state->ds.uiElemWidth[UIELEM_SEARCH_ENTRYBOX],getHighlightState(state,UIELEM_SEARCH_ENTRYBOX),HIGHLIGHT_INACTIVE,alpha8,UIICON_SEARCHGRAY,dat->strings[dat->locStringIDs[LOCSTR_SEARCH_PLACEHOLDER]],0,256,-1);
  }else{
    drawIconAndTextEntryBox(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_ENTRYBOX],(uint16_t)(state->ds.uiElemPosY[UIELEM_SEARCH_ENTRYBOX]+yOffset),state->ds.uiElemWidth[UIELEM_SEARCH_ENTRYBOX],getHighlightState(state,UIELEM_SEARCH_ENTRYBOX),HIGHLIGHT_NORMAL,alpha8,UIICON_SEARCH,state->ss.searchString,state->ds.searchEntryDispStartChar,state->ds.searchEntryDispNumChars,state->searchCursorPos);
  }

  //draw results
  char tmpStr[32];
  drawRect.x = state->ds.uiElemPosX[UIELEM_SEARCH_RESULT];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_SEARCH_RESULT] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_SEARCH_RESULT];
  drawRect.h = state->ds.uiElemHeight[UIELEM_SEARCH_RESULT];
  for(uint8_t i=0; i<state->ss.numResults; i++){
    drawButtonBG(&dat->rules.themeRules,rdat,drawRect,HIGHLIGHT_NORMAL,alpha);
    switch(state->ss.results[i].resultType){
      case SEARCHAGENT_NUCLIDE:
        ; //suppress warning in gcc
        uint16_t Z = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal].Z;
        uint16_t N = (uint16_t)dat->ndat.nuclData[state->ss.results[i].resultVal].N;
        snprintf(tmpStr,32,"%u",N+Z);
        float numWidth = drawTextAlignedSized(rdat,drawRect.x+12.0f,drawRect.y+12.0f,blackCol8Bit,FONTSIZE_SMALL,255,tmpStr,ALIGN_LEFT,16384).w; //draw number label
        drawTextAlignedSized(rdat,drawRect.x+12.0f+numWidth,drawRect.y+12.0f+(10.0f*state->ds.uiUserScale),blackCol8Bit,FONTSIZE_LARGE,255,getElemStr((uint8_t)Z),ALIGN_LEFT,16384); //draw element label
        drawTextAlignedSized(rdat,drawRect.x+12.0f,drawRect.y+SEARCH_MENU_RESULT_HEIGHT-24.0f,grayCol8Bit,FONTSIZE_NORMAL,255,dat->strings[dat->locStringIDs[LOCSTR_SEARCHRES_NUCLIDE]],ALIGN_LEFT,16384);
        break;
      default:
        continue;
    }
    drawRect.y += (float)(SEARCH_MENU_RESULT_HEIGHT*state->ds.uiUserScale);
  }

}

void drawChartViewMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHARTVIEW_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_CHARTVIEW_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU];
  drawPanelBG(&dat->rules.themeRules,rdat,drawRect,alpha);
  
  //draw menu item highlight
  SDL_FColor highlightCol;
  if(state->ds.timeLeftInUIAnimation[UIANIM_CHARTVIEW_MENU_HIDE]==0.0f){
    for(uint8_t i=1;i<=CHARTVIEW_ENUM_LENGTH;i++){
      drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU-i];
      drawRect.y = (state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU-i] + yOffset);
      drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU-i];
      drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU-i];
      switch(getHighlightState(state,UIELEM_CHARTVIEW_MENU-i)){
        case HIGHLIGHT_SELECTED:
          highlightCol = dat->rules.themeRules.modSelectedCol;
          highlightCol.a = alpha;
          drawFlatRect(rdat,drawRect,highlightCol);
          break;
        case HIGHLIGHT_MOUSEOVER:
          highlightCol = dat->rules.themeRules.modMouseOverCol;
          highlightCol.a = alpha;
          drawFlatRect(rdat,drawRect,highlightCol);
          break;
        case HIGHLIGHT_NORMAL:
        default:
          break;
      }
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + (UI_TILE_SIZE + PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_CHARTVIEW_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_CHARTVIEW_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_CHARTVIEW_MENU];
  Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale,drawRect.y + 0.4f*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_MENUTITLE]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
  for(uint8_t i=0;i<CHARTVIEW_ENUM_LENGTH;i++){
    if(state->chartView == i){
      //draw checkmark icon
      drawIcon(&dat->rules.themeRules,rdat,(uint16_t)(state->ds.uiElemPosX[UIELEM_CHARTVIEW_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale),(uint16_t)(drawRect.y + (1.25f + 1.0f*i)*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset),UI_TILE_SIZE,HIGHLIGHT_NORMAL,txtAlpha,UIICON_CHECKBOX_CHECK);
    }
    if((i==0)&&(state->ds.useLifetimes)){
      drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (1.4f + 1.0f*i)*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
    }else{
      drawTextAlignedSized(rdat,drawRect.x,drawRect.y + (1.4f + 1.0f*i)*CHARTVIEW_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE+i]],ALIGN_LEFT,(Uint16)(drawRect.w - 8*UI_PADDING_SIZE*state->ds.uiUserScale));
    }
  }

}

void drawPrimaryMenu(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  float yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_PRIMARY_MENU_HIDE)){
    alpha = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_PRIMARY_MENU_SHOW)){
    alpha = (float)(1.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]/UI_ANIM_LENGTH));
    yOffset = (-30.0f*state->ds.uiUserScale*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_PRIMARY_MENU_SHOW]/(UI_ANIM_LENGTH)));
  }
  //SDL_Log("alpha: %f\n",(double)alpha);
  
  //draw menu background
  SDL_FRect drawRect;
  drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU];
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PRIMARY_MENU] + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
  drawPanelBG(&dat->rules.themeRules,rdat,drawRect,alpha);
  
  //draw menu item highlight
  SDL_FColor highlightCol;
  for(uint8_t i=1;i<=2;i++){
    drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU-i];
    drawRect.y = (state->ds.uiElemPosY[UIELEM_PRIMARY_MENU-i] + yOffset);
    drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU-i];
    drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU-i];
    switch(getHighlightState(state,UIELEM_PRIMARY_MENU-i)){
      case HIGHLIGHT_SELECTED:
        highlightCol = dat->rules.themeRules.modSelectedCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
        break;
      case HIGHLIGHT_MOUSEOVER:
        highlightCol = dat->rules.themeRules.modMouseOverCol;
        highlightCol.a = alpha;
        drawFlatRect(rdat,drawRect,highlightCol);
        break;
      case HIGHLIGHT_NORMAL:
      default:
        break;
    }
  }

  //draw menu item text
  drawRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU] + (PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE)*state->ds.uiUserScale;
  drawRect.y = ((float)state->ds.uiElemPosY[UIELEM_PRIMARY_MENU] + PANEL_EDGE_SIZE*state->ds.uiUserScale + yOffset);
  drawRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
  drawRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
  Uint8 txtAlpha = (Uint8)(alpha*255.0f);
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 0.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_PREFS]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));
  drawTextAlignedSized(rdat,drawRect.x,drawRect.y + 1.4f*PRIMARY_MENU_ITEM_SPACING*state->ds.uiUserScale + yOffset,blackCol8Bit,FONTSIZE_NORMAL,txtAlpha,dat->strings[dat->locStringIDs[LOCSTR_MENUITEM_ABOUT]],ALIGN_LEFT,(Uint16)(drawRect.w - (PANEL_EDGE_SIZE + 6*UI_PADDING_SIZE)*state->ds.uiUserScale));

}

//meta-function which draws any UI menus, if applicable
void drawUI(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat){

  //draw background
  drawFlatBG(&state->ds,rdat,dat->rules.themeRules.bgCol);

  //draw chart of nuclides below everything else
  if(state->ds.shownElements & (1UL << UIELEM_CHARTOFNUCLIDES)){
    drawChartOfNuclides(dat,state,rdat);
    if(state->chartView == CHARTVIEW_HALFLIFE){
      if(state->ds.useLifetimes){
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_LIFETIME]]);
      }else{
        drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_HALFLIFE]]);
      }
    }else if(state->chartView == CHARTVIEW_DECAYMODE){
      drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_DECAYMODE]]);
    }else if(state->chartView == CHARTVIEW_2PLUS){
      drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_2PLUS]]);
    }else if(state->chartView == CHARTVIEW_R42){
      drawIconAndTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemPosY[UIELEM_CHARTVIEW_BUTTON],state->ds.uiElemWidth[UIELEM_CHARTVIEW_BUTTON],getHighlightState(state,UIELEM_CHARTVIEW_BUTTON),255,UIICON_CHARTVIEW,dat->strings[dat->locStringIDs[LOCSTR_CHARTVIEW_R42]]);
    }
    drawIconButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_SEARCH_BUTTON],state->ds.uiElemPosY[UIELEM_SEARCH_BUTTON],state->ds.uiElemWidth[UIELEM_SEARCH_BUTTON],getHighlightState(state,UIELEM_SEARCH_BUTTON),255,UIICON_SEARCH);
    drawIcon(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_ZOOMIN_BUTTON],state->ds.uiElemPosY[UIELEM_ZOOMIN_BUTTON],state->ds.uiElemWidth[UIELEM_ZOOMIN_BUTTON],getHighlightState(state,UIELEM_ZOOMIN_BUTTON),255,UIICON_ZOOMIN);
    drawIcon(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_ZOOMOUT_BUTTON],state->ds.uiElemPosY[UIELEM_ZOOMOUT_BUTTON],state->ds.uiElemWidth[UIELEM_ZOOMOUT_BUTTON],getHighlightState(state,UIELEM_ZOOMOUT_BUTTON),255,UIICON_ZOOMOUT);
  }
  
  //draw menus/panels etc.
  if(state->ds.shownElements & (1UL << UIELEM_NUCL_INFOBOX)){
    drawNuclInfoBox(dat,state,rdat,state->chartSelectedNucl);
  }else if(state->ds.shownElements & (1UL << UIELEM_NUCL_FULLINFOBOX)){
    drawNuclFullInfoBox(dat,state,rdat,state->chartSelectedNucl);
  }
  if(state->ds.shownElements & (1UL << UIELEM_PRIMARY_MENU)){
    drawPrimaryMenu(dat,state,rdat);
  }
  if(state->ds.shownElements & (1UL << UIELEM_CHARTVIEW_MENU)){
    drawChartViewMenu(dat,state,rdat);
  }
  if(state->ds.shownElements & (1UL << UIELEM_SEARCH_MENU)){
    updateSingleUIElemPosition(dat,state,rdat,UIELEM_SEARCH_MENU);
    drawSearchMenu(dat,state,rdat);
  }

  //draw persistent button(s)
  drawIconButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_MENU_BUTTON],state->ds.uiElemPosY[UIELEM_MENU_BUTTON],state->ds.uiElemWidth[UIELEM_MENU_BUTTON],getHighlightState(state,UIELEM_MENU_BUTTON),255,UIICON_MENU);

  //draw modal dialogs
  if(state->ds.shownElements & (1UL << UIELEM_MSG_BOX)){
    drawMessageBox(dat,state,rdat);
  }else if(state->ds.shownElements & (1UL << UIELEM_ABOUT_BOX)){
    drawAboutBox(dat,state,rdat);
  }else if(state->ds.shownElements & (1UL << UIELEM_PREFS_DIALOG)){
    drawPrefsDialog(dat,state,rdat);
    if(state->ds.shownElements & (1UL << UIELEM_PREFS_UISCALE_MENU)){
      drawUIScaleMenu(dat,state,rdat);
    }
  }

  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHART_FADEIN)){
    
    SDL_FColor white = {1.0f,1.0f,1.0f,1.0f};
    white.a = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHART_FADEIN]/UI_ANIM_LENGTH));
    drawFlatBG(&state->ds,rdat,white);
  }
  
}
