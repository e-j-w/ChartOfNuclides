/* © J. Williams 2017-2024 */
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
