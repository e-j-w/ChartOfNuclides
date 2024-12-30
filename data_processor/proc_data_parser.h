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

#ifndef PROCDATAPARSER_H
#define PROCDATAPARSER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "formats.h" //includes data formats (structs) used in the game
#include "data_ops.h"

#define MAXNUMPARSERVALS 10 //maximum number of values that can parsed at once on a line

//struct which is used temporarily during data import to map names to indices 
typedef struct
{
  char assetID[MAX_ARRAY_SIZE][256];
  char assetFileExt[MAX_ARRAY_SIZE][8];
  uint16_t numAssets; //number of assets in the asset mapping
}asset_mapping;

typedef struct
{
  uint32_t rxnInd[MAXRXNSPERNUCL]; //index of parsed reaction
  char rxnChar[MAXRXNSPERNUCL]; //which character is used in the ENSDF file to represent each reaction
}reaction_mapping;

typedef struct
{
  char spVars[MAX_SPIN_VARS][16];
	uint8_t numSpVars; //number of variables (eg. J1, J2, J3...) parsed in spin parity values
}sp_var_data; //spin-parity variable data

//prototypes
int parseAppData(app_data *restrict dat, const char *appBasePath);

#endif
