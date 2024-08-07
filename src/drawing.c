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
  if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,alpha)<0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"setUITexColAlpha - cannot set UI texture alpha - %s\n",SDL_GetError());
  }
  if(SDL_SetTextureColorModFloat(rdat->uiThemeTex,r,g,b)<0){
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"setUITexColAlpha - cannot set UI texture color modulation - %s\n",SDL_GetError());
  }
}

//draw a panel background with an arrow
//panelRect: dimensions and position of the panel, in unscaled pixels
void drawPanelBG(resource_data *restrict rdat, const SDL_FRect panelRect, const float alpha){

  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,alpha)<0){
      SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawPanelBG - cannot set UI texture alpha - %s\n",SDL_GetError());
    }
  }
  int32_t remainingWidth = (int32_t)panelRect.w;
  int32_t remainingHeight = (int32_t)panelRect.h;
  SDL_FRect srcRect, destRect;
  destRect.x = rdat->uiScale*((float)panelRect.x);
  destRect.y = rdat->uiScale*((float)panelRect.y);
  if((remainingWidth > 2*UI_TILE_SIZE)&&(remainingHeight > 2*UI_TILE_SIZE)){
    //draw the top row of the panel
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    destRect.h = destRect.w;
    //draw the top left side of the panel
    srcRect.x = UITHEME_PANELBG_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
    srcRect.y = UITHEME_PANELBG_TILE_Y*UI_TILE_SIZE*rdat->uiThemeScale;
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    srcRect.h = srcRect.w;
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
    remainingWidth -= UI_TILE_SIZE;
    //draw the top middle of the panel
    srcRect.x += srcRect.w;
    while(remainingWidth > UI_TILE_SIZE){
      int32_t tileWidth;
      if((remainingWidth - UI_TILE_SIZE) >= UI_TILE_SIZE){
        tileWidth = UI_TILE_SIZE; //panel wide enough to draw an entire middle tile
      }else{
        tileWidth = remainingWidth - UI_TILE_SIZE; //can only draw a partial UI tile
      }
      srcRect.w = (float)(tileWidth)*rdat->uiThemeScale;
      destRect.x += destRect.w;
      destRect.w = (float)(tileWidth)*rdat->uiScale;
      SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
      remainingWidth -= tileWidth;
    }
    //draw the top right side of the panel
    srcRect.x += (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    destRect.x += destRect.w;
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
    remainingHeight -= UI_TILE_SIZE;
    //draw the middle row(s) of the panel
    while(remainingHeight > UI_TILE_SIZE){
      remainingWidth = (int32_t)panelRect.w;
      int32_t tileHeight;
      if((remainingHeight - UI_TILE_SIZE) >= UI_TILE_SIZE){
        tileHeight = UI_TILE_SIZE; //panel tall enough to draw an entire middle tile
      }else{
        tileHeight = remainingHeight - UI_TILE_SIZE; //can only draw a partial UI tile
      }
      srcRect.h = (float)(tileHeight)*rdat->uiThemeScale;
      destRect.x = rdat->uiScale*((float)panelRect.x);
      destRect.y += destRect.h;
      destRect.h = (float)(tileHeight)*rdat->uiScale;
      destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
      //draw the middle left side of the panel
      srcRect.x = UITHEME_PANELBG_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.y = (UITHEME_PANELBG_TILE_Y+1)*UI_TILE_SIZE*rdat->uiThemeScale;
      srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
      SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
      remainingWidth -= UI_TILE_SIZE;
      //draw the middle-middle of the panel
      srcRect.x += srcRect.w;
      while(remainingWidth > UI_TILE_SIZE){
        int32_t tileWidth;
        if((remainingWidth - UI_TILE_SIZE) >= UI_TILE_SIZE){
          tileWidth = UI_TILE_SIZE; //panel wide enough to draw an entire middle tile
        }else{
          tileWidth = remainingWidth - UI_TILE_SIZE; //can only draw a partial UI tile
        }
        srcRect.w = (float)(tileWidth)*rdat->uiThemeScale;
        destRect.x += destRect.w;
        destRect.w = (float)(tileWidth)*rdat->uiScale;
        SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
        remainingWidth -= tileWidth;
      }
      //draw the middle right side of the panel
      srcRect.x += (float)(UI_TILE_SIZE*rdat->uiThemeScale);
      srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
      destRect.x += destRect.w;
      destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
      SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
      remainingHeight -= tileHeight;
    }
    //draw the bottom row of the panel
    remainingWidth = (int32_t)panelRect.w;
    srcRect.h = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    destRect.x = rdat->uiScale*((float)panelRect.x);
    destRect.y += destRect.h;
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    destRect.h = destRect.w;
    //draw the bottom left side of the panel
    srcRect.x = UITHEME_PANELBG_TILE_X*UI_TILE_SIZE*rdat->uiThemeScale;
    srcRect.y = (UITHEME_PANELBG_TILE_Y+2)*UI_TILE_SIZE*rdat->uiThemeScale;
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
    remainingWidth -= UI_TILE_SIZE;
    //draw the bottom-middle of the panel
    srcRect.x += srcRect.w;
    while(remainingWidth > UI_TILE_SIZE){
      int32_t tileWidth;
      if((remainingWidth - UI_TILE_SIZE) >= UI_TILE_SIZE){
        tileWidth = UI_TILE_SIZE; //panel wide enough to draw an entire middle tile
      }else{
        tileWidth = remainingWidth - UI_TILE_SIZE; //can only draw a partial UI tile
      }
      srcRect.w = (float)(tileWidth)*rdat->uiThemeScale;
      destRect.x += destRect.w;
      destRect.w = (float)(tileWidth)*rdat->uiScale;
      SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
      remainingWidth -= tileWidth;
    }
    //draw the bottom right side of the panel
    srcRect.x += (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    srcRect.w = (float)(UI_TILE_SIZE*rdat->uiThemeScale);
    destRect.x += destRect.w;
    destRect.w = (float)(UI_TILE_SIZE*rdat->uiScale);
    SDL_RenderTexture(rdat->renderer,rdat->uiThemeTex,&srcRect,&destRect);
  }else{
    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"drawPanelBG - panel size too small.\n");
  }
  
  if(alpha != 1.0f){
    if(SDL_SetTextureAlphaModFloat(rdat->uiThemeTex,1.0f)<0){
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
  float remainingWidth = (float)w;
  if(remainingWidth <= 2.0f*UI_TILE_SIZE){
    //draw a smaller button using only the left/right side tiles
    destRect.x = (float)(x*rdat->uiScale);
    destRect.y = (float)(y*rdat->uiScale);
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
    destRect.x = (float)(x*rdat->uiScale);
    destRect.y = (float)(y*rdat->uiScale);
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
  float textY = (float)y + (float)(UI_TILE_SIZE)/2.0f;
  drawTextAlignedSized(rdat,textX,textY,rdat->font,uirules->textColNormal,alpha,text,ALIGN_CENTER,(Uint16)w);
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
  drawPos.x = (float)(x*rdat->uiScale) + (float)(w*rdat->uiScale)/2.0f - drawPos.w/2.0f;
  drawPos.y = (float)(y*rdat->uiScale) + (float)(UI_TILE_SIZE*rdat->uiScale)/2.0f - drawPos.h/2.0f;
  //printf("drawPos: %i %i %i %i\n",drawPos.x,drawPos.y,drawPos.w,drawPos.h);
  if(SDL_SetTextureAlphaMod(rdat->uiThemeTex,alpha)<0){
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
  drawIcon(uirules,rdat,x+UI_PADDING_SIZE,y,UI_TILE_SIZE,HIGHLIGHT_NORMAL,alpha,iconInd);
  //get the text width and height
  //these should already fit a 1 tile height button well, with the default font size
  //(remember that the font size is scaled by the UI scale, during font import)
  float textX = (float)x + (float)(w + UI_TILE_SIZE - 2*UI_PADDING_SIZE)/2.0f;
  float textY = (float)y + (float)(UI_TILE_SIZE)/2.0f;
  //printf("text x: %f, y: %f\n",(double)textX,(double)textY);
  drawTextAlignedSized(rdat,textX,textY,rdat->font,uirules->textColNormal,255,text,ALIGN_CENTER,(Uint16)(w*rdat->uiScale));
  
}

void drawCheckbox(const ui_theme_rules *restrict uirules, resource_data *restrict rdat,const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t checked){
  drawIcon(uirules,rdat,x,y,w,highlightState,alpha,checked ? UIICON_CHECKBOX_CHECKED : UIICON_CHECKBOX_UNCHECKED);
}

//draws a selection indicator with the position and size specified by the input SDL_Rect
void drawSelectionRect(resource_data *restrict rdat, const SDL_FRect pos, const SDL_FColor col, const float thicknessPx){
  float scaledThickness = thicknessPx*rdat->uiScale;
  SDL_SetRenderDrawColorFloat(rdat->renderer,col.r,col.g,col.b,col.a);
  SDL_FRect rect;
  rect.x = pos.x;
  rect.y = pos.y;
  rect.w = pos.w;
  rect.h = scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.y = pos.y + pos.h - scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.x = pos.x;
  rect.y = pos.y + scaledThickness;
  rect.w = scaledThickness;
  rect.h = pos.h - 2*scaledThickness;
  SDL_RenderFillRect(rdat->renderer,&rect);
  rect.x = pos.x + pos.w - scaledThickness;
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

//returns a rect containing dimensions of the drawn text (can be used for alignment purposes)
SDL_FRect drawTextAlignedSized(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const Uint8 alpha, const char *txt, const uint8_t alignment, const Uint16 maxWidth){
  float drawX = xPos*rdat->uiScale;
  float drawY = yPos*rdat->uiScale;
  float drawW = (float)FC_GetWidth(font,txt);
  if(drawW > maxWidth*rdat->uiScale){
    drawW = maxWidth*rdat->uiScale;
  }
  //SDL_Log("Width: %f\n", (double)drawW);
  if(alignment == ALIGN_RIGHT){
    drawX = drawX - drawW;
  }else if(alignment == ALIGN_CENTER){
    float drawH = (float)FC_GetHeight(font,txt);
    drawX = drawX - drawW/2.0f;
    drawY = drawY - drawH/2.0f;
  }
  SDL_FRect drawRect;
  if(alpha != textColor.a){
    SDL_Color drawCol = textColor;
    drawCol.a = alpha;
    drawRect = FC_DrawColumnColor(font,rdat->renderer,drawX,drawY,(Uint16)(maxWidth*rdat->uiScale),drawCol,txt);
  }else{
    drawRect = FC_DrawColumnColor(font,rdat->renderer,drawX,drawY,(Uint16)(maxWidth*rdat->uiScale),textColor,txt);
  }
  drawRect.w /= rdat->uiScale;
  drawRect.h /= rdat->uiScale;
  return drawRect;
}
void drawTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const char *txt, const uint8_t alignment){
  drawTextAlignedSized(rdat,xPos,yPos,font,textColor,255,txt,alignment,16384);
}
void drawText(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const char *txt){
  drawTextAligned(rdat,xPos,yPos,font,textColor,txt,ALIGN_LEFT);
}
void drawColoredTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const char *txt, const uint8_t alignment){
  drawTextAligned(rdat,xPos,yPos,rdat->font,textColor,txt,alignment);
}
void drawDefaultTextAligned(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt, const uint8_t alignment){
  drawColoredTextAligned(rdat,xPos,yPos,uirules->textColNormal,txt,alignment);
}
void drawColoredText(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const char *txt){
  drawColoredTextAligned(rdat,xPos,yPos,textColor,txt,ALIGN_LEFT);
}
void drawDefaultText(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt){
  drawDefaultTextAligned(uirules,rdat,xPos,yPos,txt,ALIGN_LEFT);
}

//draws a flat colored rectangle, without transparency
void drawFlatRect(resource_data *restrict rdat, const SDL_FRect rect, const SDL_FColor col){

  //draw a rectangle covering the screen
  SDL_SetRenderDrawColorFloat(rdat->renderer, col.r, col.g, col.b, col.a);
  SDL_RenderFillRect(rdat->renderer, &rect);

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
