/* © J. Williams 2017-2024 */
/* Functions handling low-level operations on game state, or calculations using game data/state */

#ifndef DOPS_H
#define DOPS_H

#include <stdio.h>
#include "formats.h"

//function prototypes
void initializeTempState(app_state *restrict state);

void startUIAnimation(drawing_state *ds, const uint8_t uiAnim);
void stopUIAnimation(drawing_state *ds, const uint8_t uiAnim);
void updateUIAnimationTimes(drawing_state *ds, const float deltaTime);

void updateDrawingState(const app_data *restrict dat, app_state *restrict state, const float deltaTime);

const char* getElemStr(const uint8_t Z);
const char* getHalfLifeUnitShortStr(const uint8_t unit);
const char* getValueTypeShortStr(const uint8_t type);\
void getHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint32_t lev);
void getGSHalfLifeStr(char strOut[32], const ndata *restrict nd, const uint16_t nuclInd);
void getDecayModeStr(char strOut[32], const ndata *restrict nd, const uint32_t dcyModeInd);

double getNuclLevelHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd, const uint16_t nuclLevel);
double getNuclGSHalfLifeSeconds(const ndata *restrict nd, const uint16_t nuclInd);

float mouseXtoN(const drawing_state *restrict ds, const float mouseX);
float mouseYtoZ(const drawing_state *restrict ds, const float mouseY);
float getMinChartN(const drawing_state *restrict ds);
float getMaxChartN(const drawing_state *restrict ds);
float getMinChartZ(const drawing_state *restrict ds);
float getMaxChartZ(const drawing_state *restrict ds);
float getChartWidthN(const drawing_state *restrict ds);
float getChartHeightZ(const drawing_state *restrict ds);

void mouseWheelAction(app_state *restrict state);

void changeUIState(app_state *restrict state, const uint8_t newState);
void uiElemClickAction(app_state *restrict state, const uint8_t uiElemID);

void updateWindowRes(app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat);
void handleScreenGraphicsMode(app_data *restrict dat, drawing_state *restrict ds, resource_data *restrict rdat);

#endif
