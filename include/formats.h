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

#ifndef FORMATS_H
#define FORMATS_H

#include <stdint.h> //allows uint8_t and similiar types
#include <ctype.h> //for isspace
#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "SDL_FontCache.h"

#include "enums.h"

//app data parameters (should all be powers of 2)
#define MAX_ARRAY_SIZE                 65536
#define MAX_NUM_STRINGS                512  //maximum number of text strings

//increasing these numbers will increase the size of 
//the nuclear database stored in memory (and on disk)
#define MAXCASCDELENGTH 20
#define MAXGAMMASPERLEVEL 10
#define MAXSPPERLEVEL            3
#define MAXMULTPERLEVEL          3
#define MAXNUMNUCL               3500
#define MAXNUMLVLS               200000
#define MAXNUMTRAN               300000
#define MAXNUMDECAYMODES         20000
#define MAX_NEUTRON_NUM          200
#define MAX_PROTON_NUM           130
#define MAX_MASS_NUM             350

#define MAX_SPIN_VARS            32 //maximum spin variables (ie. J1, J2, J3...) per nuclide

#define NUMSHELLCLOSURES 7
static const uint16_t shellClosureValues[NUMSHELLCLOSURES] = {2,8,20,28,50,82,126};

//structures
typedef struct
{
  float val;
  uint8_t unit; //unit, (from halflife_unit_enum for half-lives)
  uint8_t err; //uncertainty value (on trailing sig figs)
  int8_t exponent; //value after exponent (eg. -5 for 4.2E-5), if value is in exponent form
  uint16_t format; //bits 0-3: number of sig figs after the decimal place
  //bit 4: whether or not to use exponent
  //bits:5-8 value type (from value_type_enum)
  //bits 9-15: reserve value dependent on value type (-ve error if VALUETYPE_ASYMERROR,
  //X index if VALUETYPE_X or VALUETYPE_PLUSX)
}valWithErr; //parsed value with an error

typedef struct
{
  valWithErr prob;
  uint8_t type; //values from decay_mode_enum
}decayMode; //decay mode for level

typedef struct
{
  uint8_t spinVal; //the spin value (255 if unknown)
  int8_t parVal;  //the parity value (1 if positive, -1 if negative, 0 if unknown)
  uint16_t format; //bit 0: whether or not spin is variable
  //bits 1-4: value type (from value_type_enum)
  //bits 5-9: variable index if bit 0 is set
  //bits 9-11: whether value is tentative (values from tentative_sp_enum)
}spinparval; //spin parity value

typedef struct
{
  valWithErr energy; //transition energy, in keV
  valWithErr intensity; //relative intensity
  uint8_t numMultipoles; //only uses the first couple bits
  uint8_t multipole[MAXMULTPERLEVEL]; //bit 0: E (unset) or M (set), bits 1-4: multipole order, bits 5-6: values from tentative_mult_enum, bit 7: if set, bit 0 corresponds to quadrupole/dipole
  uint8_t finalLvlOffset; //offset of the index of the final level from the initial level
}transition; //a transition between levels

typedef struct
{
  valWithErr energy; //level energy in keV
  valWithErr halfLife;
  int16_t numSpinParVals; //number of assigned spin parity values
  spinparval spval[MAXSPPERLEVEL]; //assinged spin parity value(s) 
  uint8_t halfInt; //if spin-parity values are half integer (1=true, in this case spinVal is multiplied by 0.5)
  uint16_t numTran; //number of gamma rays in this level
  uint32_t firstTran; //index of first transition from this level
  int8_t numDecModes; //-1 by default for no decay modes specified (assume 100% IT in that case)
  uint16_t firstDecMode;
  float decayProb[DECAYMODE_ENUM_LENGTH]; //% probability of each decay mode for this level
}level; //an individual excited level

typedef struct
{
  float qbeta, qalpha;
  float sp, sn; //proton and neutron separation energies
  int16_t N; //neutrons in nuclide
  int16_t Z; //protons in nuclide
  uint32_t firstLevel; //index of first level in this nuclide
  uint16_t numLevels; //number of excited levels in this nuclide
  uint8_t gsLevel; //which level in the nucleus (0 indexed) is the ground state (determined when parsing ENSDF, not always 0)
  uint32_t longestIsomerLevel; //which isomer in the nucleus is longest lived (=MAXNUMLVLS if no isomers)
  uint8_t longestIsomerMVal; //m-value of longest lived isomer (eg. 1 or 2 for 178m1Hf vs. 178m2Hf)
  uint8_t numIsomerMVals; //total number of isomer m-values assigned
  valWithErr abundance;
  uint8_t flags; //bits 0 to 1: observation flag (observed/unobserved/inferred/tentative)
}nucl; //gamma data for a given nuclide

typedef struct
{
  uint32_t numLvls; //number of levels across all nuclides
  uint32_t numTran; //number of transitions across all levels
  int16_t numNucl; //number of nuclides for which data is stored (-1 if no nuclides)
  uint16_t numDecModes; //number of decay modes across all levels
  uint16_t minNforZ[MAX_PROTON_NUM];
  uint16_t maxNforZ[MAX_PROTON_NUM];
  uint16_t minZforN[MAX_NEUTRON_NUM];
  uint16_t maxZforN[MAX_NEUTRON_NUM];
  uint16_t maxZ;
  uint16_t maxN;
  nucl nuclData[MAXNUMNUCL]; //data for individual nuclides
  level levels[MAXNUMLVLS]; //levels belonging to nuclides
  transition tran[MAXNUMTRAN]; //transitions between levels
  decayMode dcyMode[MAXNUMDECAYMODES]; //decay modes of levels
}ndata; //complete set of gamma data for all nuclides


typedef struct
{
  SDL_FColor bgCol; //background colors
  SDL_Color textColNormal; //colors for text (cannot be floating point as this is not supported by SDL_ttf)
  SDL_FColor modNormalCol, modMouseOverCol, modSelectedCol; //color modulations for UI elements in different states
}ui_theme_rules; //struct containing global UI thene rules

typedef struct
{
  char appName[64]; //the name of the app
  uint16_t appLogoTexID; //index for the texture for the app logo
  uint8_t textFontID, buttonFontID;
  ui_theme_rules themeRules;
}app_rules; //struct containing global app rules

typedef struct
{
  float chartPosX, chartPosY, chartZoomScale; //x and y in chart units
  float chartZoomStartScale, chartZoomToScale; //start and target zoom scale
  float chartZoomToX, chartZoomToY; //in chart units
  float chartPanToX, chartPanToY; //in chart units
  float chartZoomStartMouseX, chartZoomStartMouseY, chartZoomStartMouseXFrac, chartZoomStartMouseYFrac; //in chart units
  float chartPanStartX, chartPanStartY;
  float chartDragStartX, chartDragStartY;
  float chartDragStartMouseX, chartDragStartMouseY; //in scaled pixels
  float timeSinceZoomStart, timeSincePanStart, totalPanTime, timeSinceFCScollStart;
  float infoBoxTableHeight; //the height in unscaled pixels needed to show the info box ground and isomeric state info
  float nuclFullInfoScrollStartY, nuclFullInfoScrollToY, nuclFullInfoScrollY; //full level info view: number of lines scrolled in the y-direction
  uint16_t nuclFullInfoMaxScrollY; //maximum scroll position, in lines
  uint32_t shownElements; //bit pattern describing which UI elements are being shown, values from ui_element_enum
  uint32_t uiAnimPlaying; //bit pattern describing which UI animations are playing
  float timeLeftInUIAnimation[UIANIM_ENUM_LENGTH]; //time left in each UI animation
  uint16_t windowXRes, windowYRes; //resolution of the window
  uint16_t windowXRenderRes, windowYRenderRes; //render resolution (in pixels) taking HI-DPI into account
  uint16_t uiElemPosX[UIELEM_ENUM_LENGTH], uiElemPosY[UIELEM_ENUM_LENGTH], uiElemWidth[UIELEM_ENUM_LENGTH], uiElemHeight[UIELEM_ENUM_LENGTH];
  uint8_t texModR, texModG, texModB, texModA; //texture color and alpha modulation values
  unsigned int fcScrollInProgress : 1;
  unsigned int fcScrollFinished : 1;
  unsigned int panInProgress : 1;
  unsigned int panFinished : 1;
  unsigned int dragInProgress : 1;
  unsigned int dragFinished : 1;
  unsigned int zoomInProgress : 1;
  unsigned int zoomFinished : 1;
  unsigned int useZoomAnimations : 1;
  unsigned int drawPerformanceStats : 1; //0=don't draw, 1=draw
  unsigned int windowFullscreenMode : 1; //true if the window is fullscreen
  unsigned int forceRedraw : 1; //true if a re-draw should be forced
}drawing_state; //struct containing values used for drawing

typedef struct
{
  drawing_state ds;          //the state information for drawing
  char msgBoxHeaderTxt[32]; //text to show at the top of the message box
  char msgBoxTxt[256];      //main text to show in the message box
  float mouseXPx, mouseYPx; //current position of the mouse, in pixels
  float mouseHoldStartPosXPx, mouseHoldStartPosYPx; //mouse position at the start of the last mouse button down event, in pixels
  float mouseClickPosXPx, mouseClickPosYPx; //mouse position at the start of a mouse (left) button up event, in pixels
  float zoomDeltaVal; //amount to zoom by, equivalent to mouse wheel or touchpad scroll delta, can be very small for subtle touchpad events
  float scrollSpeedMultiplier;
  uint32_t inputFlags;
  uint16_t gamepadDeadzone,tmpGamepadDeadzone;
  int16_t lastAxisValX, lastAxisValY; //used to mask out spurious events in the gamepad axis deadzone
  uint8_t activeAxis; //the last used analog stick axis
  uint8_t clickedUIElem; //which UI element was last clicked, values from ui_element_enum
  float timeSinceLastSec; //used to track play time, the amount of seconds that have passed since the last tick
  float mouseHoldTime; //amount of time the mouse button has been held down for
  uint8_t mouseoverElement; //which UI element is moused over, =UIELEM_ENUM_LENGTH if none are moused over
  uint8_t mouseholdElement; //which UI element is the mouse button being held over, =UIELEM_ENUM_LENGTH if none
  uint8_t uiState, lastUIState; //modal state of the UI, values from ui_state_enum
  uint16_t chartSelectedNucl; //nucleus selected on the chart, =MAXNUMNUCL if none selected
  uint32_t interactableElement; //bit pattern describing which UI elements are interactable, values from ui_element_enum
  unsigned int lastInputType : 2; //0=keyboard, 1=apppad, 2=mouse
  unsigned int gamepadDisabled : 1; //1=gamepad/apppad disabled
  unsigned int quitAppFlag : 1; //0=take no action, 1=quit app
}app_state; //structure containing all app state data (persistent AND temporary)

typedef struct
{
  size_t themeOffset; //data file offset for the embedded theme
  float uiScale; //scaling factor for UI, for HI-DPI
  float uiThemeScale; //scale of the UI theme texture in GPU memory
  SDL_Surface *iconSurface; //surface for the application icon
  SDL_Texture *uiThemeTex; //the main texture atlas
  SDL_Texture *tempTex; //used to store temporary texture data during draw operations
  //using different fonts (rather than resizing a single font) decreases CPU usage at the expense of memory
  FC_Font *smallFont, *font, *bigFont, *hugeFont; //the default font (in 3 sizes)
  void *fontData; //memory adresses of the font data (must stay alive as long as fonts are used)
  SDL_Gamepad *gamepad;
  SDL_Renderer *renderer;
  SDL_Window *window;
  char *appPrefPath; //filesystem paths to on-disk resources
  char appDataFilepath[270]; //the absolute path to the app data file
}resource_data; //structure containing data relating to resources such as textures and fonts

typedef struct
{
  app_rules rules; //app rules
  ndata ndat; //nuclear structure database
  char strings[MAX_NUM_STRINGS][256]; //array of text strings used in the app
  uint16_t numStrings; //total number of text strings used
  uint16_t locStringIDs[LOCSTR_ENUM_LENGTH];
}app_data; //structure for all imported app data

//structure containing all application data
//used so that all app data can be allocated in a single block of memory
typedef struct
{
  app_state state;
  app_data dat;
  resource_data rdat;
}global_data;

#endif
