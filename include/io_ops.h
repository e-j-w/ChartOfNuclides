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

/* Functions handling save/load and other file I/O operations */

#ifndef IOOPS_H
#define IOOPS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "formats.h"

//function prototypes
void updateConfigFile(const char *configPath, const app_rules *restrict rules, const app_state *restrict state);

void updatePrefsFromConfigFile(const char *configPath, const app_rules *restrict rules, app_state *restrict state);

#endif
