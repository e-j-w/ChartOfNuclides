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

#ifndef DRAWING_H
#define DRAWING_H

#include "formats.h"

#define HALFPI 1.5708f

//function prototypes
void setAtlTexColAlpha(drawing_state *restrict ds, resource_data *restrict rdat, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t alpha);

void drawPanelBG(resource_data *restrict rdat, const SDL_FRect menuRect, const float alpha);

void drawTextButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const char *text);
void drawIcon(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd);
void drawIconButton(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const uint16_t x, const uint16_t y, const uint16_t w, const uint8_t highlightState, const uint8_t alpha, const uint8_t iconInd);

void drawSelectionRect(resource_data *restrict rdat, const SDL_FRect pos, const SDL_FColor col, const float thicknessPx);

void drawScreenDimmer(const drawing_state *restrict ds, resource_data *restrict rdat, const float alpha);

float drawTextAlignedSized(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const Uint8 alpha, const char *txt, const uint8_t alignment, const Uint16 maxWidth);
void drawTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const char *txt, const uint8_t alignment);
void drawText(resource_data *restrict rdat, const float xPos, const float yPos, FC_Font *font, const SDL_Color textColor, const char *txt);
void drawColoredTextAligned(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const char *txt, const uint8_t alignment);
void drawColoredText(resource_data *restrict rdat, const float xPos, const float yPos, const SDL_Color textColor, const char *txt);
void drawDefaultTextAligned(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt, const uint8_t alignment);
void drawDefaultText(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float xPos, const float yPos, const char *txt);

void drawFlatRect(resource_data *restrict rdat, const SDL_FRect rect, const SDL_FColor col);
void drawFlatBG(const drawing_state *restrict ds, resource_data *restrict rdat, const SDL_FColor col);

void drawLine(resource_data *restrict rdat, const float x1, const float y1, const float x2, const float y2, const float thickness, const SDL_FColor col1, const SDL_FColor col2);

void drawGradient(resource_data *restrict rdat, const SDL_Rect gradientRect, const SDL_FColor col1, const SDL_FColor col2, const uint8_t direction);

#endif
