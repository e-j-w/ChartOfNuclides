/* © J. Williams 2017-2024 */
/* Functions handling low-level operations on game state, or calculations using game data/state */

#ifndef DOPS_H
#define DOPS_H

#include <stdio.h>
#include "formats.h"

//function prototypes
void initializeTempState(const app_data *restrict dat, app_state *restrict state);

void startUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim);
void stopUIAnimation(const app_data *restrict dat, app_state *restrict state, const uint8_t uiAnim);
void updateUIAnimationTimes(const app_data *restrict dat, app_state *restrict state, const float deltaTime);

void updateDrawingState(const app_data *restrict dat, app_state *restrict state, const float deltaTime);

const char* getFullElemStr(const uint8_t Z, const uint8_t N);
const char* getElemStr(const uint8_t Z);
const char* getHalfLifeUnitShortStr(const uint8_t unit);
const char* getValueTypeShortStr(const uint8_t type);
void getGammaEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr);
void getGammaIntensityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd, const uint8_t showErr);
void getGammaMultipolarityStr(char strOut[32], const ndata *restrict nd, const uint32_t tranInd);
void getLvlEnergyStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr);
void getHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd, const uint8_t showErr, const uint8_t showUnknown);
void getGSHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd);
void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd);
void getAbundanceStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd);
void getSpinParStr(char strOut[32], const ndata *restrict nd, const uint32_t lvlInd);

double getRawValFromDB(const valWithErr *restrict valStruct);
double getRawErrFromDB(const valWithErr *restrict valStruct);

double getLevelEnergykeV(const ndata *restrict nd, const uint32_t levelInd);

double getLevelHalfLifeSeconds(const ndata *restrict nd, const uint32_t levelInd);
double getNuclLevelHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel);
double getNuclGSHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd);

uint32_t getFinalLvlInd(const ndata *restrict nd, const uint32_t initialLevel, const uint32_t tran);

uint16_t getNuclInd(const ndata *restrict nd, const int16_t N, const int16_t Z);

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
void uiElemClickAction(const app_data *restrict dat, app_state *restrict state, const uint8_t doubleClick, const uint8_t uiElemID);

void updateSingleUIElemPosition(drawing_state *restrict ds, const uint8_t uiElemInd);

float getUIthemeScale(const float uiScale);
void updateWindowRes(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);
void handleScreenGraphicsMode(app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);

#endif
