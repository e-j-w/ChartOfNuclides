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

/* Lower-level drawing functions handling basic graphics display, drawing of maps, etc. */

#include "drawing.h"
#include "gui_constants.h"
#include "juicer.h" //contains easing functions used in animations
#include "data_ops.h"

//sets color and alpha modulation for the texture atlas
//supposedly calling this interferes with draw call batching,
//so it should be used sparingly  
void setUITexColAlpha(resource_data *restrict rdat, const float r, const float g, const float b, const float alpha){

  /*if((r!=255)||(g!=255)||(b!=255)){
    printf("Color mod: %u %u %u\n",r,g,b);
  }*/
  if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,alpha)==0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"setUITexColAlpha - cannot set UI texture alpha - %s\n",SDL_GetError());
  }
  if(SDL_SetTextureColorModFloat(rdat->uiThemeTex,r,g,b)==0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"setUITexColAlpha - cannot set UI texture color modulation - %s\n",SDL_GetError());
  }
}

//draw a scrollbar
//sbPos: position of the scrollbar 'handle', between 0 and 1
//sbViewSize: fraction (between 0 and 1) of the view controlled by the scrollbar that is visible, used to set the 'handle' size
void drawScrollBar(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect sbRect, const uint8_t highlightState, const float alpha, const float sbPos, const float sbViewSize){

  switch(highlightState){
    case HIGHLIGHT_NORMAL:
    default:
      setUITexColAlpha(rdat,uirules->modNormalCol.r,uirules->modNormalCol.g,uirules->modNormalCol.b,alpha);
      break;
    case HIGHLIGHT_MOUSEOVER:
      setUITexColAlpha(rdat,uirules->modMouseOverCol.r,uirules->modMouseOverCol.g,uirules->modMouseOverCol.b,alpha);
      break;
    case HIGHLIGHT_SELECTED:
      setUITexColAlpha(rdat,uirules->modSelectedCol.r,uirules->modSelectedCol.g,uirules->modSelectedCol.b,alpha);
      break;
  }

  float clampedSBPos = sbPos;
  if(clampedSBPos > 1.0f){
    clampedSBPos = 1.0f;
  }else if(clampedSBPos < 0.0f){
    clampedSBPos = 0.0f;
  }
  
  SDL_FRect srcRect, destRect;
  const float nineSliceSize = 0.15f*UI_TILE_SIZE*rdat->uiThemeScale;
  //scrollbar background
  srcRect.x = (0.5f + UITHEME_SCROLLBAR_TILE_X)*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.y = UITHEME_SCROLLBAR_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.w = 0.5f*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.h = UI_TILE_SIZE*rdat->uiThemeScale;
  destRect.x = sbRect.x*rdat->uiDPIScale;
  destRect.y = sbRect.y*rdat->uiDPIScale;
  destRect.w = sbRect.w*rdat->uiDPIScale;
  destRect.h = sbRect.h*rdat->uiDPIScale;
  SDL_RenderTexture9Grid(rdat->renderer,rdat->uiThemeTex,&srcRect,0.0f,0.0f,nineSliceSize,nineSliceSize,0.0f,&destRect);
  //scrollbar handle
  srcRect.x = UITHEME_SCROLLBAR_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.y = UITHEME_SCROLLBAR_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
  destRect.h = sbViewSize*sbRect.h*rdat->uiDPIScale; //handle size
  if(destRect.h < 2*nineSliceSize){
    destRect.h = 2*nineSliceSize; //prevent artifacts from drawing very small handles
  }
  destRect.y = (sbRect.y + clampedSBPos*(sbRect.h - destRect.h/rdat->uiDPIScale))*rdat->uiDPIScale;
  SDL_RenderTexture9Grid(rdat->renderer,rdat->uiThemeTex,&srcRect,0.0f,0.0f,nineSliceSize,nineSliceSize,0.0f,&destRect);
  
  //reset the color modulation
  setUITexColAlpha(rdat,1.0f,1.0f,1.0f,1.0f);
}


void drawBGElement(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect elemRect, const uint8_t elemType, const uint8_t highlightState, const float alpha){

  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,alpha)==false){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawBGElement - cannot set UI texture alpha - %s\n",SDL_GetError());
    }
  }
  
  SDL_FRect srcRect, destRect;
  const float nineSliceSize = 0.4f*UI_TILE_SIZE*rdat->uiThemeScale;
  switch(elemType){
    case UIELEMTYPE_PANEL:
      srcRect.x = UITHEME_PANELBG_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.y = UITHEME_PANELBG_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.w = 3*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.h = srcRect.w;
      break;
    case UIELEMTYPE_BUTTON:
      srcRect.x = UITHEME_BUTTON_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.y = UITHEME_BUTTON_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.w = 3*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.h = UI_TILE_SIZE*rdat->uiThemeScale;
      break;
    case UIELEMTYPE_ENTRYBOX:
      srcRect.x = UITHEME_ENTRYBOX_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.y = UITHEME_ENTRYBOX_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.w = 3*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.h = UI_TILE_SIZE*rdat->uiThemeScale;
      break;
    default:
      SDL_Log("WARNING: drawBGElement - invalid element type (%u).\n",elemType);
      return;
  }
  
  destRect.x = elemRect.x*rdat->uiDPIScale;
  destRect.y = elemRect.y*rdat->uiDPIScale;
  destRect.w = elemRect.w*rdat->uiDPIScale;
  destRect.h = elemRect.h*rdat->uiDPIScale;

  switch(highlightState){
    case HIGHLIGHT_NORMAL:
    default:
      setUITexColAlpha(rdat,uirules->modNormalCol.r,uirules->modNormalCol.g,uirules->modNormalCol.b,alpha);
      break;
    case HIGHLIGHT_MOUSEOVER:
      setUITexColAlpha(rdat,uirules->modMouseOverCol.r,uirules->modMouseOverCol.g,uirules->modMouseOverCol.b,alpha);
      break;
    case HIGHLIGHT_SELECTED:
      setUITexColAlpha(rdat,uirules->modSelectedCol.r,uirules->modSelectedCol.g,uirules->modSelectedCol.b,alpha);
      break;
  }

  SDL_RenderTexture9Grid(rdat->renderer,rdat->uiThemeTex,&srcRect,nineSliceSize,nineSliceSize,nineSliceSize,nineSliceSize,0.0f,&destRect);
  
  //reset the color modulation
  setUITexColAlpha(rdat,1.0f,1.0f,1.0f,1.0f);
  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,1.0f)==0){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawBGElement - cannot reset UI texture alpha - %s\n",SDL_GetError());
    }
  }

}

//draw a panel background
//panelRect: dimensions and position of the panel, in unscaled pixels
void drawPanelBG(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect panelRect, const float alpha){
  drawBGElement(uirules,rdat,panelRect,UIELEMTYPE_PANEL,HIGHLIGHT_NORMAL,alpha);
}
void drawButtonBG(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect buttonRect, const uint8_t highlightState, const float alpha){
  drawBGElement(uirules,rdat,buttonRect,UIELEMTYPE_BUTTON,highlightState,alpha);
}

void drawMenuBGWithArrow(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect panelRect, const int16_t arrowPosX, const float alpha){
  //fade in panel first, then arrow
  //this is done to prevent drawing artifacts from using
  //two overlapping textures with transparency
  float panelAlpha = 1.0f;
  float arrowAlpha = 1.0f;
  if(alpha < 0.8f){
    panelAlpha = 1.25f*alpha;
    arrowAlpha = 0.0f;
  }else if(alpha < 1.0f){
    arrowAlpha = 5.0f*(alpha - 0.8f);
  }
  drawPanelBG(uirules,rdat,panelRect,panelAlpha);
  float arrowY = panelRect.y - 4.0f*rdat->uiScale/rdat->uiDPIScale;
  if(arrowY < 0.0f){
    arrowY = 0.0f;
  }
  drawIcon(uirules,rdat,arrowPosX,(int16_t)arrowY,(int16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale),HIGHLIGHT_NORMAL,arrowAlpha,UIICON_PANELEDGEARROW);
}

//elemType: values from uielem_type_enum 
void drawButtonOrEntryElem(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const uint8_t elemType, const float alpha){
  
  SDL_FRect destRectUnscaled;
  destRectUnscaled.x = (float)(x);
  destRectUnscaled.y = (float)(y);
  destRectUnscaled.w = (float)(w);
  destRectUnscaled.h = (float)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale);
  switch(elemType){
    case UIELEMTYPE_BUTTON:
      drawBGElement(uirules,rdat,destRectUnscaled,UIELEMTYPE_BUTTON,highlightState,alpha);
      break;
    case UIELEMTYPE_ENTRYBOX:
      drawBGElement(uirules,rdat,destRectUnscaled,UIELEMTYPE_ENTRYBOX,highlightState,alpha);
      break;
    default:
      SDL_Log("WARNING: drawButtonOrEntryElem - invalid element type (%u).\n",elemType);
      return;
  }
  
}

//x, y, w: position and size of the button, assuming a UI scale of 1 (height assumed to be one tile height)
//highlightState: values from highlight_state_enum
void drawButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const float alpha){
   drawButtonOrEntryElem(uirules,rdat,x,y,w,highlightState,UIELEMTYPE_BUTTON,alpha);
}

//draw a button with a text label
void drawTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const uint8_t alpha, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  //remember that the font size is scaled by the UI scale, during font import
  const float textX = (float)x + (float)(w)/2.0f;
  const float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_CENTER,(Uint16)w);
}

void drawIcon(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const float alpha, const uint8_t iconInd){
  
  if(iconInd >= UIICON_ENUM_LENGTH){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawIcon - invalid icon index (%u).\n",iconInd);
    return;
  }

  switch(highlightState){
    case HIGHLIGHT_NORMAL:
    default:
      setUITexColAlpha(rdat,uirules->modNormalCol.r,uirules->modNormalCol.g,uirules->modNormalCol.b,alpha);
      break;
    case HIGHLIGHT_MOUSEOVER:
      setUITexColAlpha(rdat,uirules->modMouseOverCol.r,uirules->modMouseOverCol.g,uirules->modMouseOverCol.b,alpha);
      break;
    case HIGHLIGHT_SELECTED:
      setUITexColAlpha(rdat,uirules->modSelectedCol.r,uirules->modSelectedCol.g,uirules->modSelectedCol.b,alpha);
      break;
  }

  SDL_FRect drawPos,srcRect;
  srcRect.x = UITHEME_ICON_TILE_X[iconInd]*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.y = UITHEME_ICON_TILE_Y[iconInd]*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.w = UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.h = srcRect.w;
  drawPos.w = (float)(UI_TILE_SIZE*rdat->uiScale);
  drawPos.h = (float)(UI_TILE_SIZE*rdat->uiScale);
  drawPos.x = (float)(x*rdat->uiDPIScale) + (float)(w*rdat->uiDPIScale)/2.0f - drawPos.w/2.0f;
  drawPos.y = (float)(y*rdat->uiDPIScale) + (float)(UI_TILE_SIZE*rdat->uiScale)/2.0f - drawPos.h/2.0f;
  SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&drawPos);

  //reset the color modulation
  setUITexColAlpha(rdat,1.0f,1.0f,1.0f,1.0f);

}

void drawIconButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const float alpha, const uint8_t iconInd){
  drawButton(uirules,rdat,x,y,w,highlightState,alpha);
  drawIcon(uirules,rdat,x,y,w,HIGHLIGHT_NORMAL,alpha,iconInd);
}

void drawIconAndTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,(int16_t)(x+UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale),y,(int16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale),HIGHLIGHT_NORMAL,(float)(alpha/255.0f),iconInd);
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  const float textX = (float)x + (((float)(w*rdat->uiDPIScale + UI_TILE_SIZE*rdat->uiScale - 2*UI_PADDING_SIZE*rdat->uiScale)/2.0f)/rdat->uiDPIScale);
  const float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  //printf("text x: %f, y: %f\n",(double)textX,(double)textY);
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_CENTER,(Uint16)(w*rdat->uiScale));
}

void drawDropDownTextButtonFontSize(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t fontSizeInd, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,(int16_t)(x+w-(UI_PADDING_SIZE+UI_TILE_SIZE)*rdat->uiScale/rdat->uiDPIScale),y,(int16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale),HIGHLIGHT_NORMAL,(float)(alpha/255.0f),UIICON_DROPDOWNARROW);
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  const float textX = (float)x + (((float)(w*rdat->uiDPIScale - UI_TILE_SIZE*rdat->uiScale)/2.0f)/rdat->uiDPIScale);
  float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  if(fontSizeInd == FONTSIZE_NORMAL_BOLD){
    textY -= 0.5f*rdat->uiScale/rdat->uiDPIScale; //hack to center bolded text
  }
  //printf("text x: %f, y: %f\n",(double)textX,(double)textY);
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,fontSizeInd,alpha,text,ALIGN_CENTER,(Uint16)(w*rdat->uiScale));
}

void drawDropDownTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const uint8_t alpha, const char *text){
  drawDropDownTextButtonFontSize(uirules,rdat,x,y,w,highlightState,alpha,FONTSIZE_NORMAL,text);
}

void drawTextEntryBox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t boxHighlightState, const uint8_t textHighlightState, const uint8_t alpha, const char *text){
  drawButtonOrEntryElem(uirules,rdat,x,y,w,boxHighlightState,UIELEMTYPE_ENTRYBOX,(float)(alpha/255.0f));
  const float textX = (float)(x + 3*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale);
  const float textY = (float)(y + 6*rdat->uiScale/rdat->uiDPIScale);
  switch(textHighlightState){
    case HIGHLIGHT_NORMAL:
    default:
      drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_LEFT,(Uint16)w);
      break;
    case HIGHLIGHT_INACTIVE:
      drawTextAlignedSized(rdat,textX,textY,uirules->textColInactive,FONTSIZE_NORMAL,alpha,text,ALIGN_LEFT,(Uint16)w);
      break;
  }
}

//cursorPos: -ve value = draw no cursor
void drawIconAndTextEntryBox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const int16_t x, const int16_t y, const int16_t w, const uint8_t boxHighlightState, const uint8_t textHighlightState, const uint8_t alpha, const uint8_t iconInd, const char *text, const uint16_t txtDrawStartPos, const uint16_t txtDrawNumChars, const int cursorPos){
  const int16_t iconWidth = (int16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale);
  drawButtonOrEntryElem(uirules,rdat,x,y,w,boxHighlightState,UIELEMTYPE_ENTRYBOX,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,(int16_t)(x+UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale),y,iconWidth,HIGHLIGHT_NORMAL,(float)(alpha/255.0f),iconInd);
  const float textX = (float)(x + (UI_TILE_SIZE + UI_PADDING_SIZE)*rdat->uiScale/rdat->uiDPIScale);
  const float textY = (float)(y + 6*rdat->uiScale/rdat->uiDPIScale);
  switch(textHighlightState){
    case HIGHLIGHT_NORMAL:
    default:
      ; //suppress pedantic warning
      char tmpTxt[256];
      if((txtDrawStartPos > 0)||((txtDrawNumChars < 256)&&(txtDrawNumChars < strlen(text)))){
        memcpy(tmpTxt,text+txtDrawStartPos,sizeof(char)*txtDrawNumChars);
        tmpTxt[txtDrawNumChars] = '\0'; //null terminate string
      }else{
        SDL_strlcpy(tmpTxt,text,255);
      }
      //draw text
      drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,tmpTxt,ALIGN_LEFT,65535U);
      //draw cursor
      if((cursorPos >= txtDrawStartPos)&&(cursorPos <= (txtDrawStartPos+txtDrawNumChars))){
        SDL_FColor lineCol = grayCol;
        lineCol.a = alpha/255.0f;
        memcpy(tmpTxt,text+txtDrawStartPos,sizeof(char)*((size_t)(cursorPos-txtDrawStartPos)));
        tmpTxt[cursorPos-txtDrawStartPos] = '\0'; //null terminate string
        //SDL_Log("%s|\n",cursorTxt);
        float crXPos = textX+getTextWidth(rdat,FONTSIZE_NORMAL,tmpTxt)/rdat->uiDPIScale;
        if(cursorPos >= (int)strlen(text)){
          crXPos += 1.0f*rdat->uiScale/rdat->uiDPIScale; //offset the cursor slightly at the end of the text string
        }
        drawLine(rdat,crXPos,textY,crXPos,textY+(UI_TILE_SIZE-3*UI_PADDING_SIZE)*rdat->uiScale/rdat->uiDPIScale,1.0f,lineCol,lineCol);
      }
      break;
    case HIGHLIGHT_INACTIVE:
      ; //suppress pedantic warning
      drawTextAlignedSized(rdat,textX,textY,uirules->textColInactive,FONTSIZE_NORMAL,alpha,text,ALIGN_LEFT,65535U);
      if(cursorPos >= 0){
        SDL_FColor lineCol = grayCol;
        lineCol.a = alpha/255.0f;
        drawLine(rdat,textX-4.0f,textY,textX-4.0f,textY+UI_TILE_SIZE-2*UI_PADDING_SIZE,1.0f,lineCol,lineCol);
      }
      break;
  }
}

void drawCheckbox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat,const int16_t x, const int16_t y, const int16_t w, const uint8_t highlightState, const float alpha, const uint8_t checked){
  if(checked){
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_OUTLINE);
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_CHECK);
  }else{
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_OUTLINE);
  }
}

//draws a selection indicator with the position and size specified by the input SDL_Rect
void drawSelectionRect(resource_data *restrict rdat, const SDL_FRect pos, const SDL_FColor col, const float thicknessPx){
  const float scaledThickness = thicknessPx*rdat->uiDPIScale;
  SDL_SetRenderDrawColorFloat(rdat->renderer,col.r,col.g,col.b,col.a);
  SDL_FRect rect;
  rect.x = pos.x*rdat->uiDPIScale;
  rect.y = pos.y*rdat->uiDPIScale;
  rect.w = pos.w*rdat->uiDPIScale;
  rect.h = scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.y = pos.y*rdat->uiDPIScale + pos.h*rdat->uiDPIScale - scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.x = pos.x*rdat->uiDPIScale;
  rect.y = pos.y*rdat->uiDPIScale + scaledThickness;
  rect.w = scaledThickness;
  rect.h = pos.h*rdat->uiDPIScale - 2*scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.x = pos.x*rdat->uiDPIScale + pos.w*rdat->uiDPIScale - scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
}

//draw a transparent black overlay over the full screen
void drawScreenDimmer(const drawing_state *restrict ds, resource_data *restrict rdat, const float alpha){
  
  SDL_FRect drawPos;
  drawPos.x = 0.0f;
  drawPos.y = 0.0f;
  drawPos.w = (float)ds->windowXRenderRes;
  drawPos.h = (float)ds->windowYRenderRes;
  SDL_SetRenderDrawColorFloat(rdat->renderer,0.0f,0.0f,0.0f,alpha);
  SDL_RenderFillRect(rdat->renderer, &drawPos);

}

float getTextHeight(resource_data *restrict rdat, const uint8_t fontSizeInd, const char *str){
  int textW = 0;
  int textH = 0;
  TTF_Text *ttxt = TTF_CreateText(rdat->te,rdat->font[fontSizeInd],str,0);
  TTF_GetTextSize(ttxt,&textW,&textH);
  TTF_DestroyText(ttxt);
  return (float)textH;
}
float getTextWidth(resource_data *restrict rdat, const uint8_t fontSizeInd, const char *str){
  int textW = 0;
  int textH = 0;
  TTF_Text *ttxt = TTF_CreateText(rdat->te,rdat->font[fontSizeInd],str,0);
  TTF_GetTextSize(ttxt,&textW,&textH);
  TTF_DestroyText(ttxt);
  return (float)textW;
}
float getTextHeightScaleIndependent(resource_data *restrict rdat, const uint8_t fontSizeInd, const char *str){
  return getTextHeight(rdat,fontSizeInd,str)/rdat->uiScale;
}
float getTextWidthScaleIndependent(resource_data *restrict rdat, const uint8_t fontSizeInd, const char *str){
  return getTextWidth(rdat,fontSizeInd,str)/rdat->uiScale;
}

//returns a rect containing dimensions of the drawn text (can be used for alignment and text selection purposes)
//fontSizeInd: values from font_size_enum
SDL_FRect drawTextAlignedSized(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const Uint8 alpha, const char *txt, const uint8_t alignment, const Uint16 maxWidth){
  float drawX = xPos*rdat->uiDPIScale;
  float drawY = yPos*rdat->uiDPIScale;
  float drawW = getTextWidth(rdat,fontSizeInd,txt);
  if(drawW > maxWidth*rdat->uiDPIScale){
    drawW = maxWidth*rdat->uiDPIScale;
  }
  //SDL_Log("Width: %f\n", (double)drawW);
  if(alignment == ALIGN_RIGHT){
    drawX = drawX - (maxWidth*rdat->uiDPIScale);
    TTF_SetFontWrapAlignment(rdat->font[fontSizeInd],TTF_HORIZONTAL_ALIGN_RIGHT);
  }else if(alignment == ALIGN_CENTER){
    const float drawH = getTextHeight(rdat,fontSizeInd,txt);
    drawX = drawX - (maxWidth*rdat->uiDPIScale)/2.0f;
    drawY = drawY - drawH/2.0f;
    TTF_SetFontWrapAlignment(rdat->font[fontSizeInd],TTF_HORIZONTAL_ALIGN_CENTER);
  }else{
    TTF_SetFontWrapAlignment(rdat->font[fontSizeInd],TTF_HORIZONTAL_ALIGN_LEFT);
  }
  //TTF_SetFontStyle(rdat->font[fontSizeInd],TTF_STYLE_UNDERLINE);
  TTF_Text *ttxt = TTF_CreateText(rdat->te,rdat->font[fontSizeInd],txt,0);
  TTF_SetTextWrapWidth(ttxt,(int)(maxWidth*rdat->uiDPIScale));
  SDL_FRect drawRect;
  drawRect.x = drawX;
  drawRect.y = drawY;
  drawRect.w = getTextWidth(rdat,fontSizeInd,txt);
  drawRect.h = getTextHeight(rdat,fontSizeInd,txt);
  SDL_Color drawCol = textColor;
  if(alpha != textColor.a){
    drawCol.a = alpha;
  }
  TTF_SetTextColor(ttxt,drawCol.r,drawCol.g,drawCol.b,drawCol.a);
  //TTF_SetTextColorFloat(ttxt,drawCol.r/255.0f,drawCol.g/255.0f,drawCol.b/255.0f,drawCol.a/255.0f);
  TTF_DrawRendererText(ttxt,drawX,drawY);
  if(alignment == ALIGN_RIGHT){
    drawRect.x = xPos - (drawRect.w/rdat->uiDPIScale);
    drawRect.y = yPos;
  }else if(alignment == ALIGN_CENTER){
    drawRect.x = xPos - (0.5f*drawRect.w/rdat->uiDPIScale);
    drawRect.y = yPos - (0.5f*drawRect.h/rdat->uiDPIScale);
  }else{
    drawRect.x /= rdat->uiDPIScale;
    drawRect.y = yPos;
  }
  drawRect.w /= rdat->uiDPIScale;
  drawRect.h /= rdat->uiDPIScale;
  TTF_DestroyText(ttxt);
  if((fontStyles[fontSizeInd] == FONTSTYLE_UL)||(fontStyles[fontSizeInd] == FONTSTYLE_BOLDUL)){
    //draw an underline
    //SDL_ttf in principle supports this OOTB, but the color is wrong
    //(and this way gives me more control over the look and feel)
    SDL_FColor lineCol = {drawCol.r/255.0f,drawCol.g/255.0f,drawCol.b/255.0f,drawCol.a/255.0f};
    drawLine(rdat,drawRect.x,drawRect.y+0.93f*drawRect.h,drawRect.x+drawRect.w,drawRect.y+0.93f*drawRect.h,1.0f,lineCol,lineCol);
  }
  return drawRect;
}

SDL_FRect drawSelectableTextAlignedSizedWithMetadata(resource_data *restrict rdat, text_selection_state *restrict tss, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const Uint8 alpha, const char *txt, const uint8_t alignment, const Uint16 maxWidth, const uint32_t metadata){
  
  SDL_FRect rect = drawTextAlignedSized(rdat,xPos,yPos,textColor,fontSizeInd,alpha,txt,alignment,maxWidth);

  //add the string to the list of selectable strings
  if(tss->selStrsModifiable){
    if(tss->numSelStrs < MAX_SELECTABLE_STRS){
      //SDL_Log("Adding selection string %u: %s\n",tss->numSelStrs,txt);
      //SDL_Log("  Rect: %0.3f %0.3f %0.3f %0.3f\n",(double)rect.x,(double)rect.y,(double)rect.w,(double)rect.h);
      tss->selectableStrRect[tss->numSelStrs] = rect;
      tss->selectableStrProp[tss->numSelStrs] = (uint8_t)(fontSizeInd & 15U);
      tss->selectableStrMetadata[tss->numSelStrs] = metadata;
      SDL_strlcpy(tss->selectableStrTxt[tss->numSelStrs],txt,MAX_SELECTABLE_STR_LEN);
      tss->numSelStrs++;
    }
  }

  return rect;

}
SDL_FRect drawSelectableTextAlignedSized(resource_data *restrict rdat, text_selection_state *restrict tss, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const Uint8 alpha, const char *txt, const uint8_t alignment, const Uint16 maxWidth){
  return drawSelectableTextAlignedSizedWithMetadata(rdat,tss,xPos,yPos,textColor,fontSizeInd,alpha,txt,alignment,maxWidth,STR_METADATA_UNUSED);
}

void drawTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const char *txt, const uint8_t alignment){
  drawTextAlignedSized(rdat,xPos,yPos,textColor,fontSizeInd,255,txt,alignment,16384);
}
void drawText(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const char *txt){
  drawTextAligned(rdat,xPos,yPos,textColor,fontSizeInd,txt,ALIGN_LEFT);
}
void drawColoredTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const uint8_t fontSizeInd, const char *txt, const uint8_t alignment){
  drawTextAligned(rdat,xPos,yPos,textColor,fontSizeInd,txt,alignment);
}
void drawDefaultTextAligned(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt, const uint8_t alignment){
  drawColoredTextAligned(rdat,xPos,yPos,uirules->textColNormal,FONTSIZE_NORMAL,txt,alignment);
}
void drawColoredText(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const char *txt){
  drawColoredTextAligned(rdat,xPos,yPos,textColor,FONTSIZE_NORMAL,txt,ALIGN_LEFT);
}
void drawDefaultText(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt){
  drawDefaultTextAligned(uirules,rdat,xPos,yPos,txt,ALIGN_LEFT);
}

SDL_FRect getTooltipRect(const drawing_state *restrict ds, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt){
  float txtMaxWidth = (TOOLTIP_MAX_WIDTH - 8.0f*UI_PADDING_SIZE)*rdat->uiScale/rdat->uiDPIScale;
  int textW = 0;
  int textH = 0;
  TTF_Text *ttxt = TTF_CreateText(rdat->te,rdat->font[FONTSIZE_NORMAL],txt,0);
  TTF_GetTextSize(ttxt,&textW,&textH);
  if(textW > (int)(txtMaxWidth*rdat->uiDPIScale)){
    //text needs to be wrapped
    TTF_SetTextWrapWidth(ttxt,(int)(txtMaxWidth*rdat->uiDPIScale));
    TTF_GetTextSize(ttxt,&textW,&textH);
  }
  TTF_DestroyText(ttxt);
  float width = ((float)textW/rdat->uiDPIScale) + 8.0f*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale;
  float height = ((float)textH/rdat->uiDPIScale) + 8.0f*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale;
  float xPosOut = xPos;
  float yPosOut = yPos;
  if(xPos + width > ds->windowXRes){
    xPosOut -= (width + 2.0f*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale);
  }
  if(yPos + height > ds->windowYRes){
    yPosOut -= (height + 2.0f*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale);
  }

  SDL_FRect rect = {xPosOut, yPosOut, width, height};
  return rect;
}

void drawTooltipBox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const SDL_FRect rect, const float alpha, const char *txt){

  float txtMaxWidth = (TOOLTIP_MAX_WIDTH - 8.0f*UI_PADDING_SIZE)*rdat->uiScale/rdat->uiDPIScale;
  drawPanelBG(uirules,rdat,rect,alpha);
  drawTextAlignedSized(rdat,rect.x + 4*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale,rect.y + 4*UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale,blackCol8Bit,FONTSIZE_NORMAL,(Uint8)(alpha*255.0f),txt,ALIGN_LEFT,(Uint16)txtMaxWidth);

}

//draws a flat colored rectangle, without transparency
void drawFlatRect(resource_data *restrict rdat, const SDL_FRect rect, const SDL_FColor col){

  //draw a rectangle covering the screen
  SDL_SetRenderDrawColorFloat(rdat->renderer, col.r, col.g, col.b, col.a);
  SDL_FRect scaledRect = rect;
  scaledRect.x = rect.x*rdat->uiDPIScale;
  scaledRect.y = rect.y*rdat->uiDPIScale;
  scaledRect.w = rect.w*rdat->uiDPIScale;
  scaledRect.h = rect.h*rdat->uiDPIScale;
  SDL_RenderFillRect(rdat->renderer, &scaledRect);

}

//draws a flat colored background, without transparency
void drawFlatBG(const drawing_state *restrict ds, resource_data *restrict rdat, const SDL_FColor col){

  SDL_FRect drawPos;
  drawPos.w = (float)ds->windowXRenderRes;
  drawPos.h = (float)ds->windowYRenderRes;
  drawPos.x = 0.0f;
  drawPos.y = 0.0f;

  //draw a rectangle covering the screen
  drawFlatRect(rdat,drawPos,col);

}

//draw a colored line of a given thickness
void drawLine(resource_data *restrict rdat, const float x1, const float y1, const float x2, const float y2, const float thickness, const SDL_FColor col1, const SDL_FColor col2){
  
  //scale coordinates
  float x1s = SDL_floorf(x1*rdat->uiDPIScale); //floorf prevents flickering of moving elements when fractional scaling
  float x2s = SDL_floorf(x2*rdat->uiDPIScale);
  float y1s = SDL_floorf(y1*rdat->uiDPIScale);
  float y2s = SDL_floorf(y2*rdat->uiDPIScale);
  float ths = thickness*rdat->uiDPIScale;
  
  /*//use built-in function for single pixel width lines
  if((thickness <= 1.0f)&&(col1.r == col2.r)&&(col1.g == col2.g)&&(col1.b == col2.b)&&(col1.a == col2.a)){
    SDL_SetRenderDrawColor(rdat->renderer,(Uint8)col1.r,(Uint8)col1.g,(Uint8)col1.b,(Uint8)col1.a);
    SDL_RenderLine(rdat->renderer,x1s,y1s,x2s,y2s);
    return;
  }*/
  
  float angle = SDL_atan2f((y2s-y1s),(x2s-x1s));
  float oppAngle = HALFPI - angle; 
  SDL_Vertex lineVert[6];
  lineVert[0].position.x = x1s;
  lineVert[0].position.y = y1s;
  lineVert[0].color = col1;
  lineVert[1].position.x = x2s;
  lineVert[1].position.y = y2s;
  lineVert[1].color = col2;
  lineVert[2].position.x = x2s + (ths*SDL_cosf(oppAngle));
  lineVert[2].position.y = y2s - (ths*SDL_sinf(oppAngle));
  lineVert[2].color = col2;
  lineVert[3].position.x = lineVert[2].position.x;
  lineVert[3].position.y = lineVert[2].position.y;
  lineVert[3].color = lineVert[2].color;
  lineVert[4].position.x = lineVert[0].position.x;
  lineVert[4].position.y = lineVert[0].position.y;
  lineVert[4].color = lineVert[0].color;
  lineVert[5].position.x = x1s + (ths*SDL_cosf(oppAngle));
  lineVert[5].position.y = y1s - (ths*SDL_sinf(oppAngle));
  lineVert[5].color = col1;

  SDL_RenderGeometry(rdat->renderer, NULL, lineVert, 6, NULL, 0);
}

//draws a gradient
//direction: 0=left-right, 1=up-down
void drawGradient(resource_data *restrict rdat, const SDL_Rect gradientRect, const SDL_FColor col1, const SDL_FColor col2, const uint8_t direction){

  SDL_Vertex gradientVert[2][3];

  switch(direction){
    case 1:
      gradientVert[0][0].position.x = (float)(gradientRect.x);
      gradientVert[0][0].position.y = (float)(gradientRect.y);
      gradientVert[0][0].color = col1;
      gradientVert[0][1].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[0][1].position.y = (float)(gradientRect.y);
      gradientVert[0][1].color = col1;
      gradientVert[0][2].position.x = (float)(gradientRect.x);
      gradientVert[0][2].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[0][2].color = col2;
      gradientVert[1][0].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[1][0].position.y = (float)(gradientRect.y);
      gradientVert[1][0].color = col1;
      gradientVert[1][1].position.x = (float)(gradientRect.x);
      gradientVert[1][1].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[1][1].color = col2;
      gradientVert[1][2].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[1][2].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[1][2].color = col2;
      break;
    case 0:
    default:
      gradientVert[0][0].position.x = (float)(gradientRect.x);
      gradientVert[0][0].position.y = (float)(gradientRect.y);
      gradientVert[0][0].color = col1;
      gradientVert[0][1].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[0][1].position.y = (float)(gradientRect.y);
      gradientVert[0][1].color = col2;
      gradientVert[0][2].position.x = (float)(gradientRect.x);
      gradientVert[0][2].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[0][2].color = col1;
      gradientVert[1][0].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[1][0].position.y = (float)(gradientRect.y);
      gradientVert[1][0].color = col2;
      gradientVert[1][1].position.x = (float)(gradientRect.x);
      gradientVert[1][1].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[1][1].color = col1;
      gradientVert[1][2].position.x = (float)(gradientRect.x + gradientRect.w);
      gradientVert[1][2].position.y = (float)(gradientRect.y + gradientRect.h);
      gradientVert[1][2].color = col2;
      break;
  }

  SDL_RenderGeometry(rdat->renderer, NULL, gradientVert[0], 3, NULL, 0);
  SDL_RenderGeometry(rdat->renderer, NULL, gradientVert[1], 3, NULL, 0);

}
