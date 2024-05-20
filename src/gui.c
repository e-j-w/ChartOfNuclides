/* © J. Williams 2017-2024 */
/* Higher-level drawing functions handling display of the user interface. */
/* The UI scale shouldn't be handled here, it should instead be handled in the lower level functions of state->ds.sds.h*/

#include "gui.h"
#include "gui_constants.h"
#include "juicer.h" //contains easing functions used in animations
#include "drawing.h"
#include "data_ops.h"


void drawChartOfNuclides(const app_data *restrict dat, const app_state *restrict state, resource_data *restrict rdat){
  
  float minX = getMinChartN(&state->ds);
  float maxX = getMaxChartN(&state->ds);
  float minY = getMinChartZ(&state->ds);
  float maxY = getMaxChartZ(&state->ds);
  //printf("N range: [%0.2f %0.2f], Z range: [%0.2f %0.2f]\n",(double)minX,(double)maxX,(double)minY,(double)maxY);

  SDL_FRect rect;
  rect.w = DEFAULT_NUCLBOX_DIM*state->ds.chartZoomScale;
  rect.h = rect.w;

  for(int i=0;i<dat->ndat.numNucl;i++){
    if((dat->ndat.nuclData[i].N >= (int16_t)floorf(minX))&&(dat->ndat.nuclData[i].N >= 0)){
      if(dat->ndat.nuclData[i].N <= (int16_t)ceilf(maxX)){
        if((dat->ndat.nuclData[i].Z >= (int16_t)floorf(minY))&&(dat->ndat.nuclData[i].Z >= 0)){
          if(dat->ndat.nuclData[i].Z <= (int16_t)ceilf(maxY)){
            //draw nuclide on chart
            rect.x = ((float)dat->ndat.nuclData[i].N - minX)*rect.w;
            rect.y = (maxY - (float)dat->ndat.nuclData[i].Z)*rect.h;
            //printf("N: %i, Z: %i, pos: [%0.2f %0.2f %0.2f %0.2f]\n",dat->ndat.nuclData[i].N,dat->ndat.nuclData[i].Z,(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
            drawFlatRect(rdat,rect,grayCol);
          }
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
  
  SDL_Rect msgBoxPanelRect;
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
    SDL_Rect menuRect;
    menuRect.x = state->ds.uiElemPosX[UIELEM_PRIMARY_MENU];
    menuRect.y = state->ds.uiElemPosY[UIELEM_PRIMARY_MENU];
    menuRect.w = state->ds.uiElemWidth[UIELEM_PRIMARY_MENU];
    menuRect.h = state->ds.uiElemHeight[UIELEM_PRIMARY_MENU];
    drawPanelBG(rdat,menuRect,1.0f);
  }
  if(state->ds.shownElements & (1U << UIELEM_MSG_BOX)){
    drawMessageBox(dat,state,rdat);
  }

  
  
  
}
