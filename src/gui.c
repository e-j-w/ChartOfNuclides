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
  //printf("range: %f\n",(double)range);
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
  //printf("half-life: %0.6f\n",halflifeSeconds);
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
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-7){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.7f;
  }else if(halflifeSeconds > 1.0E-15){
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 0.8f;
  }else{
    col.r = 1.0f;
    col.g = 0.6f;
    col.b = 1.0f;
  }
  return col;
}

void drawNuclBoxLabel(const app_data *restrict dat, const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const float boxWidth, SDL_Color col, const uint16_t nuclInd){
  char tmpStr[32];
  float drawXPos, drawYPos;
  uint16_t Z = (uint16_t)dat->ndat.nuclData[nuclInd].Z;
  if(ds->chartZoomScale >= 8.0f){
    uint16_t N = (uint16_t)dat->ndat.nuclData[nuclInd].N;
    snprintf(tmpStr,32,"%u",N+Z);
    float totalLblWidth = ((float)(FC_GetWidth(rdat->smallFont,tmpStr) + FC_GetWidth(rdat->bigFont,getElemStr((uint8_t)Z))))/rdat->uiScale;
    drawXPos = xPos+boxWidth*0.5f - totalLblWidth*0.5f;
    if(ds->chartZoomScale >= 12.0f){
      drawYPos = yPos + NUCLBOX_LABEL_MARGIN*ds->chartZoomScale;
    }else{
      float totalLblHeight = ((float)(FC_GetHeight(rdat->smallFont,tmpStr) + FC_GetHeight(rdat->bigFont,getElemStr((uint8_t)Z))))/rdat->uiScale;
      drawYPos = yPos+boxWidth*0.5f - totalLblHeight*0.5f;
    }
    drawXPos += drawTextAlignedSized(rdat,drawXPos,drawYPos,rdat->smallFont,col,255,tmpStr,ALIGN_LEFT,16384); //draw number label
    drawTextAlignedSized(rdat,drawXPos,drawYPos+10.0f,rdat->bigFont,col,255,getElemStr((uint8_t)Z),ALIGN_LEFT,16384); //draw element label
    if(ds->chartZoomScale >= 12.0f){
      drawXPos = xPos + NUCLBOX_LABEL_MARGIN*ds->chartZoomScale;
      drawYPos = yPos + NUCLBOX_LABEL_MARGIN*ds->chartZoomScale;
      getGSHalfLifeStr(tmpStr,&dat->ndat,nuclInd);
      drawTextAlignedSized(rdat,drawXPos,drawYPos+36.0f,rdat->font,col,255,tmpStr,ALIGN_LEFT,16384); //draw GS half-life label
      if((ds->chartZoomScale >= 15.0f)&&(dat->ndat.nuclData[nuclInd].numLevels > 0)){
        uint8_t yOffsets = 3;
        uint8_t yOffsetLimit = 3;
        if(ds->chartZoomScale < 18.0f){
          yOffsetLimit += 1;
        }else if(ds->chartZoomScale < 22.0f){
          yOffsetLimit += 2;
        }else if(ds->chartZoomScale < 28.0f){
          yOffsetLimit += 3;
        }else if(ds->chartZoomScale < 30.0f){
          yOffsetLimit += 4;
        }else{
          yOffsetLimit += 5;
        }
        if(dat->ndat.nuclData[nuclInd].abundance.unit == VALUE_UNIT_PERCENT){
          getAbundanceStr(tmpStr,&dat->ndat,nuclInd);
          if(yOffsets < yOffsetLimit){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f),rdat->font,col,255,tmpStr,ALIGN_LEFT,16384); //draw abundance label
            yOffsets += 1;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f),rdat->font,col,255,"(...)",ALIGN_LEFT,16384);
          }
        }
        uint32_t gsLevInd = (uint32_t)(dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel);
        for(int8_t i=0; i<dat->ndat.levels[gsLevInd].numDecModes; i++){
          getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[gsLevInd].firstDecMode + (uint32_t)i);
          //printf("%s\n",tmpStr);
          if((yOffsets < yOffsetLimit)||(i==(dat->ndat.levels[gsLevInd].numDecModes-1))){
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f),rdat->font,col,255,tmpStr,ALIGN_LEFT,16384); //draw decay mode label
            yOffsets += 1;
          }else{
            drawTextAlignedSized(rdat,drawXPos,drawYPos+(yOffsets*20.0f),rdat->font,col,255,"(...)",ALIGN_LEFT,16384);
            break;
          }
        }
      }
    }
  }else{
    drawTextAlignedSized(rdat,xPos+boxWidth*0.5f,yPos+boxWidth*0.5f,rdat->font,col,255,getElemStr((uint8_t)Z),ALIGN_CENTER,16384); //draw element label only
  }
}

void drawChartOfNuclides(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){

  float minX = getMinChartN(&state->ds);
  float maxX = getMaxChartN(&state->ds);
  float minY = getMinChartZ(&state->ds);
  float maxY = getMaxChartZ(&state->ds);
  
  //printf("N range: [%0.2f %0.2f], Z range: [%0.2f %0.2f]\n",(double)minX,(double)maxX,(double)minY,(double)maxY);

  SDL_FRect rect;
  rect.w = DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale*rdat->uiScale;
  rect.h = rect.w;

  for(int i=0;i<dat->ndat.numNucl;i++){
    if((dat->ndat.nuclData[i].flags & 3U) == OBSFLAG_OBSERVED){
      if((dat->ndat.nuclData[i].N >= (int16_t)floorf(minX))&&(dat->ndat.nuclData[i].N >= 0)){
        if(dat->ndat.nuclData[i].N <= (int16_t)ceilf(maxX)){
          if((dat->ndat.nuclData[i].Z >= (int16_t)floorf(minY))&&(dat->ndat.nuclData[i].Z >= 0)){
            if(dat->ndat.nuclData[i].Z <= (int16_t)ceilf(maxY)){
              //draw nuclide on chart
              rect.x = ((float)dat->ndat.nuclData[i].N - minX)*rect.w;
              rect.y = (maxY - (float)dat->ndat.nuclData[i].Z)*rect.h;
              //printf("N: %i, Z: %i, i: %i, pos: [%0.2f %0.2f %0.2f %0.2f]\n",dat->ndat.nuclData[i].N,dat->ndat.nuclData[i].Z,i,(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
              const double hl = getNuclGSHalfLifeSeconds(&dat->ndat,(uint16_t)i);
              drawFlatRect(rdat,rect,getHalfLifeCol(hl));
              if(state->ds.chartZoomScale >= 4.0f){
                drawNuclBoxLabel(dat,&state->ds,rdat,rect.x/rdat->uiScale,rect.y/rdat->uiScale,rect.w/rdat->uiScale,(hl > 1.0E4) ? whiteCol8Bit : blackCol8Bit,(uint16_t)i);
              }
            }
          }
        }
      }
    }
  }

  //draw shell closure lines
  //some fudging of line positions was needed to fix alignment with fractional scaling
  SDL_FColor scCol = blackCol;
  scCol.a = (state->ds.chartZoomScale/4.0f) - 0.25f;
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
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //bottom of shell closure is on screen
                drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.minZforN[shellClosureValues[i]] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }else if(dat->ndat.maxZforN[shellClosureValues[i]] < maxY){
              //top of shell closure is on screen
              drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(maxY - dat->ndat.maxZforN[shellClosureValues[i]])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
            }else{
              //top and bottom of shell closure are both off screen
              drawLine(rdat,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              drawLine(rdat,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,0.0f,(shellClosureValues[i] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale - 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowYRes,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
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
                drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }else{
                //left edge of shell closure is on screen
                drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
                drawLine(rdat,(dat->ndat.minNforZ[shellClosureValues[i]] - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              }
            }else if(dat->ndat.maxNforZ[shellClosureValues[i]] < maxX){
              //right edge of shell closure is on screen
              drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,(dat->ndat.maxNforZ[shellClosureValues[i]] - minX + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
            }else{
              //left and right edges of shell closure are both off screen
              drawLine(rdat,0.0f,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i])*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
              drawLine(rdat,0.0f,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,state->ds.windowXRes,(maxY - shellClosureValues[i] + 1.0f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale + 0.5f*CHART_SHELLCLOSURELINE_THICKNESS,CHART_SHELLCLOSURELINE_THICKNESS,scCol,scCol);
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
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*rect.w - CHART_SHELLCLOSURELINE_THICKNESS;
      rect.w += 2.0f*CHART_SHELLCLOSURELINE_THICKNESS;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*rect.h - CHART_SHELLCLOSURELINE_THICKNESS;
      rect.h += 2.0f*CHART_SHELLCLOSURELINE_THICKNESS;
      drawSelectionRect(rdat,rect,selectionCol,2.0f*CHART_SHELLCLOSURELINE_THICKNESS);
    }else{
      rect.x = ((float)dat->ndat.nuclData[state->chartSelectedNucl].N - minX)*rect.w;
      rect.y = (maxY - (float)dat->ndat.nuclData[state->chartSelectedNucl].Z)*rect.h;
      drawFlatRect(rdat,rect,selectionCol);
    }
  }

  //draw x and y axes
  rect.w = state->ds.windowXRenderRes;
  rect.h = CHART_AXIS_DEPTH*rdat->uiScale;
  rect.x = 0;
  rect.y = state->ds.windowYRenderRes - rect.h;
  drawFlatRect(rdat,rect,whiteTransparentCol);
  rect.w = rect.h;
  rect.h = rect.y;
  rect.y = 0;
  drawFlatRect(rdat,rect,whiteTransparentCol);
  drawTextAligned(rdat,CHART_AXIS_DEPTH*0.5f,CHART_AXIS_DEPTH*0.5f,rdat->bigFont,blackCol8Bit,"Z",ALIGN_CENTER);
  drawTextAligned(rdat,state->ds.windowXRes - CHART_AXIS_DEPTH*0.5f,state->ds.windowYRes - CHART_AXIS_DEPTH*0.5f,rdat->bigFont,blackCol8Bit,"N",ALIGN_CENTER);
  //draw ticks
  char tmpStr[32];
  rect.y = state->ds.windowYRes - CHART_AXIS_DEPTH*0.5f;
  float tickSpacing = getAxisTickSpacing(roundf(maxX - minX)); //round to remove jitter when panning the chart and the axis range is very close to an integer value
  for(float i=(minX-fmodf(minX,tickSpacing));i<maxX;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(floorf(i));
      if((i<MAX_MASS_NUM)&&(i<=MAX_NEUTRON_NUM)){
        rect.x = (i + 0.5f - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale;
        if(rect.x < (state->ds.windowXRes - CHART_AXIS_DEPTH)){ //dodge axis label
          snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,rdat->font,blackCol8Bit,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }
  rect.x = CHART_AXIS_DEPTH*0.5f;
  tickSpacing = getAxisTickSpacing(maxY - minY);
  for(float i=(minY-fmodf(minY,tickSpacing));i<maxY;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(floorf(i));
      if((i<MAX_MASS_NUM)&&(i<MAX_PROTON_NUM)){
        rect.y = (maxY - i + 0.5f)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale;
        if(rect.y > CHART_AXIS_DEPTH){ //dodge axis label
          snprintf(tmpStr,32,"%u",numInd); //is this slow?
          drawTextAlignedSized(rdat,rect.x,rect.y,rdat->font,blackCol8Bit,255,tmpStr,ALIGN_CENTER,16384); //draw number label
        }
      }
    }
  }

}

//draw some stats, ie. FPS overlay and further diagnostic info
void drawPerformanceStats(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float deltaTime){

  //draw background
  SDL_FRect perfOvRect;
  perfOvRect.w = (628.0f*rdat->uiScale);
  perfOvRect.h = (108.0f*rdat->uiScale);
  perfOvRect.x = 0.0f;
  perfOvRect.y = 0.0f;
  
  SDL_SetRenderDrawColor(rdat->renderer,255,255,255,150);
  SDL_RenderFillRect(rdat->renderer,&perfOvRect);

  //draw text
  char txtStr[256];
  SDL_snprintf(txtStr,256,"Performance stats (press <P> to toggle display)");
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR,txtStr);
  SDL_snprintf(txtStr,256,"FPS: %4.1f",1.0/((double)deltaTime));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+PERF_OVERLAY_Y_SPACING,txtStr);
  SDL_snprintf(txtStr,256,"Frame time (ms): %4.3f",(double)(deltaTime*1000.0f));
  drawDefaultText(uirules,rdat,PERF_OVERLAY_BUTTON_X_ANCHOR,PERF_OVERLAY_BUTTON_Y_ANCHOR+2*PERF_OVERLAY_Y_SPACING,txtStr);
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
    return HIGHLIGHT_NORMAL;
  }
}

void drawNuclInfoBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat, const uint16_t nuclInd){
  
  uint16_t yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_HIDE)){
    yOffset = (uint16_t)(300.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLINFOBOX_SHOW)){
    yOffset = (uint16_t)(300.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW]/(UI_ANIM_LENGTH)));
  }
  Uint8 alpha = 255;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_NUCLHIGHLIGHT_SHOW)){
    alpha = (uint8_t)(255.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_NUCLHIGHLIGHT_SHOW]/UI_ANIM_LENGTH));
  }
  
  
  //draw panel background
  SDL_FRect infoBoxPanelRect;
  infoBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_NUCL_INFOBOX];
  infoBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + yOffset;
  infoBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX];
  infoBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX];
  drawPanelBG(rdat,infoBoxPanelRect,1.0f);

  //draw strings
  char tmpStr[32];
  float drawXPos = (float)(infoBoxPanelRect.x + 4*UI_PADDING_SIZE);
  float drawYPos = (float)(infoBoxPanelRect.y + 4*UI_PADDING_SIZE) + 40.0f;
  drawTextAlignedSized(rdat,drawXPos,drawYPos,rdat->font,blackCol8Bit,alpha,dat->strings[LOCSTR_GM_STATE],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawYPos += 30.0f;
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_ENERGY_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,dat->strings[LOCSTR_ENERGY_KEV],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_JPI_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,dat->strings[LOCSTR_JPI],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_HALFLIFE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,dat->strings[LOCSTR_HALFLIFE],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_DECAYMODE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,dat->strings[LOCSTR_DECAYMODE],ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);

  //ground state
  drawYPos += 30.0f;
  uint32_t lvlInd = dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel;
  snprintf(tmpStr,32,"%.0f",(double)dat->ndat.levels[lvlInd].energy);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_ENERGY_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  getSpinParStr(tmpStr,&dat->ndat,lvlInd);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_JPI_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  getHalfLifeStr(tmpStr,&dat->ndat,lvlInd);
  drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_HALFLIFE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
  if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
    drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_DECAYMODE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,"N/A",ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw no decay mode label
  }else{
    for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
      getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
      //printf("%s\n",tmpStr);
      if(drawYPos < (float)(state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX])){
        drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_DECAYMODE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw decay mode label
        drawYPos += 20.0f;
      }else{
        break;
      }
    }
  }

  //longest isomer
  //printf("Isomer index: %u\n",dat->ndat.nuclData[nuclInd].longestIsomerLevel);
  lvlInd = dat->ndat.nuclData[nuclInd].longestIsomerLevel;
  if((lvlInd != MAXNUMLVLS)&&(lvlInd != (dat->ndat.nuclData[nuclInd].firstLevel + dat->ndat.nuclData[nuclInd].gsLevel))){
    drawYPos += 30.0f;
    snprintf(tmpStr,32,"%.0f",(double)dat->ndat.levels[lvlInd].energy);
    drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_ENERGY_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    getSpinParStr(tmpStr,&dat->ndat,lvlInd);
    drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_JPI_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    getHalfLifeStr(tmpStr,&dat->ndat,lvlInd);
    drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_HALFLIFE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]);
    if(dat->ndat.levels[lvlInd].halfLife.unit == VALUE_UNIT_STABLE){
      drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_DECAYMODE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,"N/A",ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw no decay mode label
    }else{
      for(int8_t i=0; i<dat->ndat.levels[lvlInd].numDecModes; i++){
        getDecayModeStr(tmpStr,&dat->ndat,dat->ndat.levels[lvlInd].firstDecMode + (uint32_t)i);
        //printf("%s\n",tmpStr);
        if(drawYPos < (float)(state->ds.uiElemPosY[UIELEM_NUCL_INFOBOX] + state->ds.uiElemHeight[UIELEM_NUCL_INFOBOX])){
          drawTextAlignedSized(rdat,drawXPos+NUCL_INFOBOX_DECAYMODE_COL_OFFSET,drawYPos,rdat->font,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,state->ds.uiElemWidth[UIELEM_NUCL_INFOBOX]); //draw decay mode label
          drawYPos += 20.0f;
        }else{
          break;
        }
      }
    }
  }

  //header
  drawXPos = (float)(infoBoxPanelRect.x + 4*UI_PADDING_SIZE);
  drawYPos = (float)(infoBoxPanelRect.y + 4*UI_PADDING_SIZE);
  snprintf(tmpStr,32,"%u",(uint16_t)(dat->ndat.nuclData[nuclInd].Z + dat->ndat.nuclData[nuclInd].N));
  drawXPos += drawTextAlignedSized(rdat,drawXPos,drawYPos,rdat->smallFont,blackCol8Bit,alpha,tmpStr,ALIGN_LEFT,16384); //draw number label
  drawTextAlignedSized(rdat,drawXPos,drawYPos+10.0f,rdat->bigFont,blackCol8Bit,alpha,getElemStr((uint8_t)dat->ndat.nuclData[nuclInd].Z),ALIGN_LEFT,16384); //draw element label


  //printf("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_NUCLINFOBOX_HIDE],alpha);
}

void drawMessageBox(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float alpha = 1.0f;
  uint16_t yOffset = 0;
  if(state->ds.uiAnimPlaying & (1U << UIANIM_MSG_BOX_HIDE)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_HIDE]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(255.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_HIDE]/(UI_ANIM_LENGTH)));
    yOffset = (uint16_t)(100.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_HIDE]/UI_ANIM_LENGTH));
  }else if(state->ds.uiAnimPlaying & (1U << UIANIM_MSG_BOX_SHOW)){
    alpha = (float)(DIMMER_OPACITY*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_SHOW]/UI_ANIM_LENGTH));
    drawScreenDimmer(&state->ds,rdat,alpha);
    alpha = (float)(255.0f*juice_smoothStop2(1.0f - state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_SHOW]/UI_ANIM_LENGTH));
    yOffset = (uint16_t)(100.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_SHOW]/(UI_ANIM_LENGTH)));
  }else{
    drawScreenDimmer(&state->ds,rdat,DIMMER_OPACITY);
  }
  
  SDL_FRect msgBoxPanelRect;
  msgBoxPanelRect.x = state->ds.uiElemPosX[UIELEM_MSG_BOX];
  msgBoxPanelRect.y = state->ds.uiElemPosY[UIELEM_MSG_BOX] + yOffset;
  msgBoxPanelRect.w = state->ds.uiElemWidth[UIELEM_MSG_BOX];
  msgBoxPanelRect.h = state->ds.uiElemHeight[UIELEM_MSG_BOX];
  drawPanelBG(rdat,msgBoxPanelRect,alpha);

  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+MESSAGE_BOX_HEADERTXT_Y,rdat->bigFont,dat->rules.themeRules.textColNormal,(uint8_t)floorf(alpha*255.0f),state->msgBoxHeaderTxt,ALIGN_CENTER,(Uint16)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+(msgBoxPanelRect.h/2),rdat->font,dat->rules.themeRules.textColNormal,(uint8_t)floorf(alpha*255.0f),state->msgBoxTxt,ALIGN_CENTER,(Uint16)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
  drawTextButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_MSG_BOX_OK_BUTTON],state->ds.uiElemPosY[UIELEM_MSG_BOX_OK_BUTTON]+yOffset,state->ds.uiElemWidth[UIELEM_MSG_BOX_OK_BUTTON],getHighlightState(state,UIELEM_MSG_BOX_OK_BUTTON),(uint8_t)floorf(alpha*255.0f),dat->strings[dat->locStringIDs[LOCSTR_OK]]);
  //printf("%.3f %.3f alpha %u\n",(double)state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_SHOW],(double)state->ds.timeLeftInUIAnimation[UIANIM_MSG_BOX_HIDE],alpha);
}

//meta-function which draws any UI menus, if applicable
void drawUI(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float deltaTime){
  
  (void)deltaTime; //unused for now

  //draw chart of nuclides below everything else
  drawChartOfNuclides(dat,state,rdat);

  //draw button(s)
  drawIconButton(&dat->rules.themeRules,rdat,state->ds.uiElemPosX[UIELEM_MENU_BUTTON],state->ds.uiElemPosY[UIELEM_MENU_BUTTON],state->ds.uiElemWidth[UIELEM_MENU_BUTTON],getHighlightState(state,UIELEM_MENU_BUTTON),255,UIICON_MENU);
  
  //draw menus/panels etc.
  if(state->ds.shownElements & (1U << UIELEM_PRIMARY_MENU)){
    SDL_FRect menuRect;
    menuRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU];
    menuRect.y = state->ds.uiElemPosY[UIELEM_PRIMARY_MENU];
    menuRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
    menuRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
    drawPanelBG(rdat,menuRect,1.0f);
  }
  if(state->ds.shownElements & (1U << UIELEM_MSG_BOX)){
    drawMessageBox(dat,state,rdat);
  }
  if(state->ds.shownElements & (1U << UIELEM_NUCL_INFOBOX)){
    drawNuclInfoBox(dat,state,rdat,state->chartSelectedNucl);
  }

  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHART_FADEIN)){
    
    SDL_FColor white = {1.0f,1.0f,1.0f,1.0f};
    white.a = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHART_FADEIN]/UI_ANIM_LENGTH));
    drawFlatBG(&state->ds,rdat,white);
  }
  
}
