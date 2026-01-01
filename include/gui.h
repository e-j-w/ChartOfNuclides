/*
Copyright (C) 2017-2026 J. Williams

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

/* Higher-level drawing functions handling display of the user interface. */

#ifndef GUI_H
#define GUI_H

#include "formats.h"
#include "strops.h"

//function prototypes
void drawPerformanceStats(const ui_theme_rules *restrict uirules, const app_state *restrict state, const thread_manager_state *restrict tms, resource_data *restrict rdat, const float deltaTime);

void drawUI(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat);

#endif
