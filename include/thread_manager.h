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

/* Functions handling the thread pool */

#ifndef TMAN_H
#define TMAN_H

#include "formats.h"

//function prototypes

int startSearchThreads(app_data *restrict dat, app_state *restrict state, thread_manager_state *restrict tms);
void updateThreads(const app_data *restrict dat, app_state *restrict state, resource_data *restrict rdat, thread_manager_state *restrict tms);
void stopThreadPool(thread_manager_state *restrict tms);

#endif
