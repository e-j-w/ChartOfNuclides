/* © J. Williams 2017-2024 */
/* Higher-level drawing functions handling display of the user interface. */

#ifndef GUI_H
#define GUI_H

#include "formats.h"

//function prototypes
void drawPerformanceStats(const ui_theme_rules *restrict uirules, resource_data *restrict rdat, const float deltaTime);

void drawUI(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, const float deltaTime);

#endif
