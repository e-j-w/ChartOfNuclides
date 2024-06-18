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

/* Functions used to load and process app data files */

#ifndef LDATA_H
#define LDATA_H

#include <stdlib.h>

#include "formats.h"
#include "gui_constants.h"

int importAppData(app_data *restrict dat, resource_data *restrict rdat);
int regenerateFontCache(app_data *restrict dat, resource_data *restrict rdat);

#endif
