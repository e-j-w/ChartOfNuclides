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

//draw a panel background
//panelRect: dimensions and position of the panel, in unscaled pixels
void drawPanelBG(resource_data *restrict rdat, const SDL_FRect panelRect, const float alpha){

  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,alpha)==false){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawPanelBG - cannot set UI texture alpha - %s\n",SDL_GetError());
    }
  }
  
  SDL_FRect srcRect, destRect;
  const float nineSliceSize = 0.5f*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.x = UITHEME_PANELBG_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.y = UITHEME_PANELBG_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.w = 3*UI_TILE_SIZE*rdat->uiThemeScale;
  srcRect.h = srcRect.w;
  destRect.x = panelRect.x*rdat->uiDPIScale;
  destRect.y = panelRect.y*rdat->uiDPIScale;
  destRect.w = panelRect.w*rdat->uiDPIScale;
  destRect.h = panelRect.h*rdat->uiDPIScale;
  SDL_RenderTexture9Grid(rdat->renderer,rdat->uiThemeTex,&srcRect,nineSliceSize,nineSliceSize,nineSliceSize,nineSliceSize,0.0f,&destRect);
  
  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,1.0f)==0){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawPanelBG - cannot reset UI texture alpha - %s\n",SDL_GetError());
    }
  }
}

//x, y, w: position and size of the button, assuming a UI scale of 1 (height assumed to be one tile height)
//highlightState: values from highlight_state_enum
void drawButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const float alpha){
  
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

  SDL_FRect srcRect, destRect;
  float remainingWidth = (float)w*rdat->uiDPIScale/rdat->uiScale;
  if(remainingWidth <= 2.0f*UI_TILE_SIZE){
    //draw a smaller button using only the left/right side tiles
    destRect.x = (float)(x*rdat->uiDPIScale);
    destRect.y = (float)(y*rdat->uiDPIScale);
    destRect.w = (float)((float)remainingWidth*rdat->uiScale/2.0f);
    destRect.h = (float)(UI_TILE_SIZE*rdat->uiScale);
    srcRect.x = 0.0f;
    srcRect.y = 0.0f;
    srcRect.w = (float)((float)remainingWidth*rdat->uiThemeScale/2.0f);
    srcRect.h = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    //draw the left side of the button
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
    remainingWidth -= (remainingWidth/2.0f);
    //draw the right side of the button
    srcRect.x += (float)(3.0f*UI_TILE_SIZE*rdat->uiThemeScale - (float)remainingWidth*rdat->uiThemeScale);
    srcRect.w = (float)((float)remainingWidth*rdat->uiThemeScale);
    destRect.x += destRect.w;
    destRect.w = (float)((float)remainingWidth*rdat->uiScale);
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
  }else{
    destRect.x = (float)(x*rdat->uiDPIScale);
    destRect.y = (float)(y*rdat->uiDPIScale);
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    destRect.h = destRect.w;
    srcRect.x = 0.0f;
    srcRect.y = 0.0f;
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    srcRect.h = srcRect.w;
    //draw the left side of the button
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
    remainingWidth -= UI_TILE_SIZE;
    //draw the middle section of the button
    srcRect.x += srcRect.w;
    while(remainingWidth > UI_TILE_SIZE){
      float tileWidth;
      if((remainingWidth - UI_TILE_SIZE) >= UI_TILE_SIZE){
        tileWidth = UI_TILE_SIZE; //button wide enough to draw an entire middle tile
      }else{
        tileWidth = remainingWidth - UI_TILE_SIZE; //can only draw a partial UI tile
      }
      srcRect.w = tileWidth*rdat->uiThemeScale;
      destRect.x += destRect.w;
      destRect.w = tileWidth*rdat->uiScale;
      SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
      remainingWidth -= tileWidth;
    }
    //draw the right side of the button
    srcRect.x += (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    destRect.x += destRect.w;
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
  }

  //reset the color modulation
  setUITexColAlpha(rdat,1.0f,1.0f,1.0f,1.0f);

}

//draw a button with a text label
void drawTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  float textX = (float)x + (float)(w)/2.0f;
  float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_CENTER,(Uint16)w);
}

void drawIcon(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd){
  
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
  //printf("drawPos: %i %i %i %i\n",drawPos.x,drawPos.y,drawPos.w,drawPos.h);
  if(SDL_SetTextureAlphaMod(rdat->uiThemeTex,alpha)==0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawIcon - cannot set texture alpha - %s\n",SDL_GetError());
  }
  SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&drawPos);
  SDL_SetTextureAlphaMod(rdat->uiThemeTex,255);

  //reset the color modulation
  setUITexColAlpha(rdat,1.0f,1.0f,1.0f,1.0f);

}

void drawIconButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,x,y,w,HIGHLIGHT_NORMAL,alpha,iconInd);
}

void drawIconAndTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,(uint16_t)(x+UI_PADDING_SIZE*rdat->uiScale/rdat->uiDPIScale),y,(uint16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale),HIGHLIGHT_NORMAL,alpha,iconInd);
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  float textX = (float)x + (((float)(w*rdat->uiDPIScale + UI_TILE_SIZE*rdat->uiScale - 2*UI_PADDING_SIZE*rdat->uiScale)/2.0f)/rdat->uiDPIScale);
  float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  //printf("text x: %f, y: %f\n",(double)textX,(double)textY);
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_CENTER,(Uint16)(w*rdat->uiScale));
}

void drawDropDownTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const char *text){
  drawButton(uirules,rdat,x,y,w,highlightState,(float)(alpha/255.0f));
  drawIcon(uirules,rdat,(uint16_t)(x+w-(UI_PADDING_SIZE+UI_TILE_SIZE)*rdat->uiScale/rdat->uiDPIScale),y,(uint16_t)(UI_TILE_SIZE*rdat->uiScale/rdat->uiDPIScale),HIGHLIGHT_NORMAL,alpha,UIICON_DROPDOWNARROW);
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  float textX = (float)x + (((float)(w*rdat->uiDPIScale - UI_TILE_SIZE*rdat->uiScale)/2.0f)/rdat->uiDPIScale);
  float textY = (float)y + ((float)(UI_TILE_SIZE)/2.0f)*rdat->uiScale/rdat->uiDPIScale;
  //printf("text x: %f, y: %f\n",(double)textX,(double)textY);
  drawTextAlignedSized(rdat,textX,textY,uirules->textColNormal,FONTSIZE_NORMAL,alpha,text,ALIGN_CENTER,(Uint16)(w*rdat->uiScale));
}

void drawCheckbox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat,const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t checked){
  if(checked){
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_OUTLINE);
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_CHECK);
  }else{
    drawIcon(uirules,rdat,x,y,w,highlightState,alpha,UIICON_CHECKBOX_OUTLINE);
  }
}

//draws a selection indicator with the position and size specified by the input SDL_Rect
void drawSelectionRect(resource_data *restrict rdat, const SDL_FRect pos, const SDL_FColor col, const float thicknessPx){
  float scaledThickness = thicknessPx*rdat->uiDPIScale;
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
  return FC_GetHeight(rdat->font[fontSizeInd],str);
}
float getTextWidth(resource_data *restrict rdat, const uint8_t fontSizeInd, const char *str){
  return FC_GetWidth(rdat->font[fontSizeInd],str);
}

//returns a rect containing dimensions of the drawn text (can be used for alignment purposes)
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
    drawX = drawX - drawW;
  }else if(alignment == ALIGN_CENTER){
    float drawH = getTextHeight(rdat,fontSizeInd,txt);
    drawX = drawX - drawW/2.0f;
    drawY = drawY - drawH/2.0f;
  }
  SDL_FRect drawRect;
  if(alpha != textColor.a){
    SDL_Color drawCol = textColor;
    drawCol.a = alpha;
    drawRect = FC_DrawColumnColor(rdat->font[fontSizeInd],rdat->renderer,drawX,drawY,(Uint16)(maxWidth*rdat->uiDPIScale),drawCol,txt);
  }else{
    drawRect = FC_DrawColumnColor(rdat->font[fontSizeInd],rdat->renderer,drawX,drawY,(Uint16)(maxWidth*rdat->uiDPIScale),textColor,txt);
  }
  drawRect.w /= rdat->uiDPIScale;
  drawRect.h /= rdat->uiDPIScale;
  return drawRect;
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
  float x1s = SDL_floorf(x1*rdat->uiScale); //floorf prevents flickering of moving elements when fractional scaling
  float x2s = SDL_floorf(x2*rdat->uiScale);
  float y1s = SDL_floorf(y1*rdat->uiScale);
  float y2s = SDL_floorf(y2*rdat->uiScale);
  float ths = thickness*rdat->uiScale;
  
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
