/* © J. Williams 2017-2026 */
/* Functions handling low-level operations on game state, or calculations using game data/state */

#ifndef SOPS_H
#define SOPS_H

#include <stdio.h>
#include <stdlib.h>
#include "formats.h"

//function prototypes
void tokenizeSearchStr(search_state *restrict ss);
void searchELevel(const ndata *restrict ndat, const app_state *state, search_state *ss);
void searchELevelDiff(const ndata *restrict ndat, const app_state *state, search_state *ss);
void searchEGamma(const ndata *restrict ndat, const app_state *state, search_state *ss);
void searchGammaCascade(const ndata *restrict ndat, const app_state *state, search_state *ss);
void searchHalfLife(const ndata *restrict ndat, const app_state *state, search_state *ss);
void searchNuclides(const ndata *restrict ndat, search_state *restrict ss);
void searchSpecialStrings(search_state *restrict ss);

#endif
