/* © J. Williams 2017-2024 */
/* Higher-level drawing functions handling display of the user interface. */
/* The UI scale shouldn't be handled here, it should instead be handled in the lower level functions of state->ds.sds.h*/

#include "gui.h"
#include "gui_constants.h"
#include "juicer.h" //contains easing functions used in animations
#include "drawing.h"
#include "data_ops.h"

float getAxisTickSpacing(float range){
  if(range < 12.0f){
    return 2.0f;
  }else if(range < 30.0f){
    return 5.0f;
  }else if(range < 60.0f){
    return 10.0f;
  }else if(range < 120.0f){
    return 20.0f;
  }else if(range < 180.0f){
    return 30.0f;
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

void drawNuclBoxLabel(const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, SDL_Color col, const uint16_t N, const uint16_t Z, const uint32_t nuclInd){
  if(ds->chartZoomScale >= 8.0f){
    float numLblWidth = drawTextFromCache(rdat,xPos,yPos,col,ALIGN_LEFT,(uint16_t)(N+Z)); //draw number label
    drawTextFromCache(rdat,xPos+numLblWidth+2.0f,yPos+10.0f,col,ALIGN_LEFT,MAX_MASS_NUM+Z); //draw element label
    if(ds->chartZoomScale >= 12.0f){
      drawTextFromCache(rdat,xPos,yPos+36,col,ALIGN_LEFT,MAX_MASS_NUM+MAX_PROTON_NUM+MAX_PROTON_NUM+nuclInd); //draw half-life label
    }
  }else{
    drawTextFromCache(rdat,xPos,yPos,col,ALIGN_LEFT,MAX_MASS_NUM+MAX_PROTON_NUM+Z); //draw element label only
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
              float txtX = (rect.x/rdat->uiScale + NUCLBOX_NAME_MARGIN*state->ds.chartZoomScale);
              float txtY = (rect.y/rdat->uiScale + NUCLBOX_NAME_MARGIN*state->ds.chartZoomScale);
              drawNuclBoxLabel(&state->ds,rdat,txtX,txtY,(hl > 1.0E4) ? whiteCol8Bit : blackCol8Bit,(uint16_t)(dat->ndat.nuclData[i].N),(uint16_t)(dat->ndat.nuclData[i].Z),(uint32_t)i);
            }
          }
        }
      }
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
  rect.y = state->ds.windowYRes - CHART_AXIS_DEPTH*0.5f;
  float tickSpacing = getAxisTickSpacing(maxX - minX);
  for(float i=(minX-fmodf(minX,tickSpacing));i<maxX;i+=tickSpacing){
    if(i >= 0.0f){
      uint16_t numInd = (uint16_t)(floorf(i));
      if((i<MAX_MASS_NUM)&&(i<=MAX_NEUTRON_NUM)){
        rect.x = (i + 0.5f - minX)*DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale;
        if(rect.x < (state->ds.windowXRes - CHART_AXIS_DEPTH)){ //dodge axis label
          drawTextFromCache(rdat,rect.x,rect.y,blackCol8Bit,ALIGN_CENTER,numInd);
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
          drawTextFromCache(rdat,rect.x,rect.y,blackCol8Bit,ALIGN_CENTER,numInd);
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

  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+MESSAGE_BOX_HEADERTXT_Y,rdat->bigFont,dat->rules.themeRules.textColNormal,(uint8_t)floorf(alpha*255.0f),state->msgBoxHeaderTxt,ALIGN_CENTER,(Uint32)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
  drawTextAlignedSized(rdat,msgBoxPanelRect.x+(msgBoxPanelRect.w/2),msgBoxPanelRect.y+(msgBoxPanelRect.h/2),rdat->font,dat->rules.themeRules.textColNormal,(uint8_t)floorf(alpha*255.0f),state->msgBoxTxt,ALIGN_CENTER,(Uint32)(msgBoxPanelRect.w - 2*UI_PADDING_SIZE));
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

  if(state->ds.uiAnimPlaying & (1U << UIANIM_CHART_FADEIN)){
    
    SDL_FColor white = {1.0f,1.0f,1.0f,1.0f};
    white.a = (float)(1.0f*juice_smoothStart2(state->ds.timeLeftInUIAnimation[UIANIM_CHART_FADEIN]/UI_ANIM_LENGTH));
    drawFlatBG(&state->ds,rdat,white);
  }
  
}
