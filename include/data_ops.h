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

void updateDrawingState(drawing_state *ds, const float deltaTime);

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

void updateWindowRes(drawing_state *restrict ds, resource_data *restrict rdat);
void handleScreenGraphicsMode(drawing_state *restrict ds, resource_data *restrict rdat);

#endif