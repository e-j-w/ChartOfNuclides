#ifndef LU_H
#define LU_H

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//increasing these numbers will increase the size of 
//the nuclear database stored in memory
#define MAXCASCDELENGTH 10
#define MAXCASCDESPERNUCL 20
#define MAXGAMMASPERLEVEL 10
#define MAXLEVELSPERNUCL 50
#define MAXNUMNUCL 5000

//structures
typedef struct
{
  int numGammas; //number of steps in the cascade
  double energies[MAXCASCDELENGTH]; //energies of the gammas in the cascade in keV
}gamma_cascade; //an individual gamma cascade

typedef struct
{
  char sp[10]; //spin parity of the level
  double energy; //level energy in keV
  double gamma_energies[MAXGAMMASPERLEVEL];
}gamma_level; //an individual excited level

typedef struct
{
	char nuclName[10]; //name of the nucleus, eg. '68SE'
	int N; //neutrons in nucleus
	int Z; //protons in nucleus
	int numLevels; //number of excited levels in this nucleus
	gamma_level levels[MAXLEVELSPERNUCL]; //cascades belonging to the nucleus
	int numCascades; //number of cascades stored for this nucleus
	gamma_cascade cascades[MAXCASCDESPERNUCL]; //cascades belonging to the nucleus
}nucl; //gamma data for a given nucleus

typedef struct
{
	int numNucl; //number of nuclei for which data is stored
	nucl nuclData[MAXNUMNUCL];
}gdata; //complete set of gamma data for all nuclei

#endif

