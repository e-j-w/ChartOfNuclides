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

/* Functions handling low-level operations on game state, or calculations 
using game data/state */

#ifndef DOPS_H
#define DOPS_H

#include <stdio.h>
#include "formats.h"

//function prototypes
void initializeTempState(const app_data *restrict dat, app_state *restrict state, thread_manager_state *restrict tms);

void clearSelectionStrs(text_selection_state *restrict tss, const uint8_t modifiableAfter);

void startUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim);
void stopUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim);
void updateUIAnimationTimes(const app_data *restrict dat, app_state *restrict state, const float deltaTime);

void updateDrawingState(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float deltaTime);

const char* getSpecialLvlStr(const app_data *restrict dat, const uint8_t specialLvlInd);
const char* getFullElemStr(const uint8_t Z, const uint8_t N);
const char* getElemStr(const uint8_t Z);
void getNuclNameStr(char strOut[32], const nucl *restrict nuclide, const uint8_t isomerMVal);
uint8_t elemStrToZ(const char *elemStr);
const char* getValueUnitShortStr(const uint8_t unit);
const char* getValueTypeShortStr(const uint8_t type);
const char* getDecayTypeShortStr(const uint8_t type);
const char* getElementFamilyStr(const app_data *restrict dat, const int16_t Z);
void getMassValStr(char strOut[32], const dblValWithErr mVal, const uint8_t showErr);
void getQValStr(char strOut[32], const valWithErr qVal, const uint8_t showErr);
void getGammaEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr);
void getGammaIntensityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr);
void getGammaMultipolarityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd);
void getLvlEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr);
void getHalfLifeStr(char strOut[32], const app_data *restrict dat, const uint32_t lvlInd, const uint8_t showErr, const uint8_t showUnknown, const uint8_t useLifetime);
void getGSHalfLifeStr(char strOut[32], const app_data *restrict dat, const uint16_t nuclInd, const uint8_t useLifetime);
void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd);
void getMostProbableDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd);
void getRxnStr(char strOut[32], const ndata *restrict nd, const uint32_t rxnInd);
void getAbundanceStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd);
void getSpinParStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd);

double getRawDblValFromDB(const dblValWithErr *restrict valStruct);
double getRawValFromDB(const valWithErr *restrict valStruct);
double getRawErrFromDB(const valWithErr *restrict valStruct);

double getLevelEnergykeV(const ndata *restrict nd, const uint32_t levelInd);

int8_t getMostProbableParity(const ndata *restrict nd, const uint32_t lvlInd);
double getMostProbableSpin(const ndata *restrict nd, const uint32_t lvlInd);
uint16_t getNumUnknownLvls(const ndata *restrict nd, const uint16_t nuclInd);
double getBEA(const ndata *restrict nd, const uint16_t nuclInd);
uint32_t get2PlusLvlInd(const ndata *restrict nd, const uint16_t nuclInd);
double getBeta2(const ndata *restrict nd, const uint16_t nuclInd);
double getR42(const ndata *restrict nd, const uint16_t nuclInd);
double get2PlusEnergy(const ndata *restrict nd, const uint16_t nuclInd);

double getLevelHalfLifeSeconds(const ndata *restrict nd, const uint32_t levelInd);
double getNuclLevelHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel);
double getNuclGSHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd);

uint8_t getLevelMostProbableDcyMode(const ndata *restrict nd, const uint32_t lvlInd);
uint8_t getNuclGSMostProbableDcyMode(const ndata *restrict nd, const uint16_t nuclInd);

uint32_t getFinalLvlInd(const ndata *restrict nd, const uint32_t initialLevel, const uint32_t tran);

uint16_t getNuclInd(const ndata *restrict nd, const int16_t N, const int16_t Z);

uint16_t getNumScreenLvlDispLines(const drawing_state *restrict ds);
uint16_t getNumTotalLvlDispLines(const ndata *restrict nd, const app_state *restrict state);
uint16_t getNumDispLinesUpToLvl(const ndata *restrict nd, const app_state *restrict state, const uint16_t nuclLevel);
uint16_t getNumDispLinesForLvl(const ndata *restrict nd, const uint32_t lvlInd);

float mouseXtoN(const drawing_state *restrict ds, const float mouseX);
float mouseYtoZ(const drawing_state *restrict ds, const float mouseY);
float getMinChartN(const drawing_state *restrict ds);
float getMaxChartN(const drawing_state *restrict ds);
float getMinChartZ(const drawing_state *restrict ds);
float getMaxChartZ(const drawing_state *restrict ds);
float getChartWidthN(const drawing_state *restrict ds);
float getChartHeightZ(const drawing_state *restrict ds);

float mouseXPxToN(const drawing_state *restrict ds, const float mouseX);
float mouseYPxToZ(const drawing_state *restrict ds, const float mouseY);
float getChartWidthNAfterZoom(const drawing_state *restrict ds);
float getChartHeightZAfterZoom(const drawing_state *restrict ds);

void changeUIState(const app_data *restrict dat, app_state *restrict state, const uint8_t newState);
void panChartToPos(const app_data *restrict dat, drawing_state *restrict ds, const uint16_t posN, const uint16_t posZ, float panTime);
uint16_t getNearestNuclInd(const app_data *restrict dat, const int16_t N, const int16_t Z);
void setSelectedNuclOnLevelList(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t N, const uint16_t Z, const uint8_t updateRxn);
void setSelectedNuclOnChart(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t N, const uint16_t Z, const uint8_t forcePan);
void setSelectedNuclOnChartDirect(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint16_t selNucl, const uint8_t forcePan);
void updateSearchUIState(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);
void uiElemHoldAction(const app_data *restrict dat, app_state *restrict state, const uint8_t uiElemID);
void uiElemMouseoverAction(const app_state *restrict state, resource_data *restrict rdat);
void uiElemClickAction(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t doubleClick, const uint8_t uiElemID);

uint16_t getNumTextCharsUnderWidth(resource_data *restrict rdat, const uint16_t widthPx, const char *text, const uint16_t txtStartChar, const uint8_t fontSizeInd);

uint8_t getRxnMenuNumRxnsPerColumn(const app_data *restrict dat, const app_state *restrict state);
SDL_FRect getRxnMenuButtonRect(const app_state *restrict state, const uint8_t numRxnPerCol, const uint8_t menuItem);

void updateSingleUIElemPosition(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const uint8_t uiElemInd);
void updateUIElemPositions(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);

void removeSelectableStringsInRect(text_selection_state *restrict tss, const SDL_FRect rect);

float getUIthemeScale(const float uiScale);
void updateUIScale(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);
void updateWindowRes(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);
void handleScreenGraphicsMode(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);

#endif
