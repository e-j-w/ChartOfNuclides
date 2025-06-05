/* © J. Williams 2017-2024 */
/* Functions handling low-level operations on game state, or calculations using game data/state */

#ifndef SOPS_H
#define SOPS_H

#include <stdio.h>
#include <stdlib.h>
#include "formats.h"

//function prototypes
void tokenizeSearchStr(search_state *restrict ss);
void searchELevel(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss);
void searchEGamma(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss);
void searchGammaCascade(const ndata *restrict ndat, const drawing_state *restrict ds, search_state *restrict ss);
void searchNuclides(const ndata *restrict ndat, search_state *restrict ss);

#endif
