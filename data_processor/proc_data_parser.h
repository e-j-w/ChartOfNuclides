/* © J. Williams 2017-2024 */

#ifndef PROCDATAPARSER_H
#define PROCDATAPARSER_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "formats.h" //includes data formats (structs) used in the game
#include "data_ops.h"

//struct which is used temporarily during data import to map names to indices 
typedef struct
{
  char assetID[MAX_ARRAY_SIZE][256];
  char assetFileExt[MAX_ARRAY_SIZE][8];
  uint16_t numAssets; //number of assets in the asset mapping
}asset_mapping;

//prototypes
int parseAppData(app_data *restrict dat, const char *appBasePath);

#endif
