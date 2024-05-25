/* © J. Williams 2017-2024 */
/* Data formats used in the app */

#ifndef FORMATS_H
#define FORMATS_H

#include <stdint.h> //allows uint8_t and similiar types
#include <ctype.h> //for isspace
#include <math.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "enums.h"

//minimum dimensions of the window
#define MIN_RENDER_WIDTH  640
#define MIN_RENDER_HEIGHT 360

//maximum dimensions of the window
#define MAX_RENDER_WIDTH  16384
#define MAX_RENDER_HEIGHT 16384

//texture cache parameters
#define TEXT_TEX_CACHE_SIZE 8192

//app data parameters (should all be powers of 2)
#define MAX_ARRAY_SIZE                 65536
#define MAX_NUM_STRINGS                512  //maximum number of text strings

//increasing these numbers will increase the size of 
//the nuclear database stored in memory (and on disk)
#define MAXCASCDELENGTH 20
#define MAXCASCDESPERNUCL 50
#define MAXGAMMASPERLEVEL 10
#define MAXSPPERLEVEL 3
#define MAXNUMNUCL               3500
#define MAXNUMLVLS               200000
#define MAXNUMTRAN               300000
#define MAXNUMDECAYMODES         6000
#define MAX_NEUTRON_NUM          200
#define MAX_PROTON_NUM           130
#define MAX_MASS_NUM             350

#define MAXNUMPARSERVALS 10 //maximum number of values that can parsed at once on a line

#define NUMSHELLCLOSURES 7
static const uint16_t shellClosureValues[NUMSHELLCLOSURES] = {2,8,20,28,50,82,126};

//structures
typedef struct
{
  int16_t numLevels; //number of steps in the cascade
  float energies[MAXCASCDELENGTH]; //energies of the levels in the cascade in keV
  float gammaEnergies[MAXCASCDELENGTH];
}gamma_cascade; //an individual gamma cascade

typedef struct
{
  float prob;
  uint8_t probErr;
  uint8_t probType;
  uint8_t type; //values from decay_mode_enum
}decayMode; //decay mode for level

typedef struct
{
  int16_t spinVal; //the spin value (-1 if unknown)
  uint8_t halfInt; //if spin-parity value is half integer (1=true, in this case spinVal is multiplied by 0.5)
  int8_t parVal;  //the parity value (1 if positive, -1 if negative, 0 if unknown)
  uint8_t tentative; //0 if not tentative, 1 if all tentative, 2 if only spin tentative, 3 if GE
}spinparval; //spin parity value

typedef struct
{
  uint8_t type; //0=gamma, other values for beta, alpha, etc.
  float energy; //transition energy, in keV
  float intensity; //relative intensity
}transition; //a transition between levels

typedef struct
{
  float energy; //level energy in keV
  uint8_t energyErr; //energy uncertainty value
  float halfLife; //level half-life (-1 if unknown)
  uint8_t halfLifeUnit; //units for level half-life (values from halflife_unit_enum)
  uint8_t halfLifeFormat; //bits 0-3: number of sig figs after the decimal place, bit 4: whether or not to use exponent, bits:5-7 value type (from value_type_enum);
  int16_t halfLifeErr; //hal-life uncertainty value
  int16_t numSpinParVals; //number of assigned spin parity values
  spinparval spval[MAXSPPERLEVEL]; //assinged spin parity value(s) 
  int16_t numTran; //number of gamma rays in this level
  uint32_t firstTran; //index of first transition from this level
  int8_t numDecModes; //-1 by default for no decay modes specified (assume 100% IT in that case)
  uint32_t firstDecMode;
  float decayProb[DECAYMODE_ENUM_LENGTH]; //% probability of each decay mode for this level
}level; //an individual excited level

typedef struct
{
  int16_t N; //neutrons in nuclide
  int16_t Z; //protons in nuclide
  float qbeta, qalpha;
  float sp, sn; //proton and neutron separation energies
  uint32_t firstLevel; //index of first level in this nuclide
  uint16_t numLevels; //number of excited levels in this nuclide
  int16_t numCascades; //number of cascades stored for this nuclide
  gamma_cascade cascades[MAXCASCDESPERNUCL]; //cascades belonging to the nuclide
}nucl; //gamma data for a given nuclide

typedef struct
{
  int16_t numNucl; //number of nuclides for which data is stored (-1 if no nuclides)
  uint32_t numLvls; //number of levels across all nuclides
  uint32_t numTran; //number of transitions across all levels
  uint32_t numDecModes; //number of decay modes across all levels
  nucl nuclData[MAXNUMNUCL]; //data for individual nuclides
  level levels[MAXNUMLVLS]; //levels belonging to nuclides
  transition tran[MAXNUMTRAN]; //transitions between levels
  decayMode dcyMode[MAXNUMDECAYMODES]; //decay modes of levels
}ndata; //complete set of gamma data for all nuclides


typedef struct
{
  SDL_FColor whiteBGCol; //background colors
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
  float chartZoomStartMouseX, chartZoomStartMouseY, chartZoomStartMouseXFrac, chartZoomStartMouseYFrac; //in chart units
  float chartDragStartX, chartDragStartY;
  float chartDragStartMouseX, chartDragStartMouseY; //in scaled pixels
  float timeSinceZoomStart;
  uint32_t shownElements; //bit pattern describing which UI elements are being shown, values from ui_element_enum
  uint32_t uiAnimPlaying; //bit pattern describing which UI animations are playing
  float timeLeftInUIAnimation[UIANIM_ENUM_LENGTH]; //time left in each UI animation
  uint16_t windowXRes, windowYRes; //resolution of the window
  uint16_t windowXRenderRes, windowYRenderRes; //render resolution (in pixels) taking HI-DPI into account
  uint16_t uiElemPosX[UIELEM_ENUM_LENGTH], uiElemPosY[UIELEM_ENUM_LENGTH], uiElemWidth[UIELEM_ENUM_LENGTH], uiElemHeight[UIELEM_ENUM_LENGTH];
  uint8_t texModR, texModG, texModB, texModA; //texture color and alpha modulation values
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
  float mouseWheelVal; //scroll delta, can be very small for subtle touchpad events 
  uint16_t gamepadDeadzone,tmpGamepadDeadzone;
  int16_t lastAxisValX, lastAxisValY; //used to mask out spurious events in the gamepad axis deadzone
  uint8_t activeAxis; //the last used analog stick axis
  uint8_t clickedUIElem; //which UI element was last clicked, values from ui_element_enum
  float timeSinceLastSec; //used to track play time, the amount of seconds that have passed since the last tick
  float mouseHoldTime; //amount of time the mouse button has been held down for
  uint8_t mouseoverElement; //which UI element is moused over, =UIELEM_ENUM_LENGTH if none are moused over
  uint8_t mouseholdElement; //which UI element is the mouse button being held over, =UIELEM_ENUM_LENGTH if none
  uint8_t uiState; //modal state of the UI, values from ui_state_enum
  uint32_t interactableElement; //bit pattern describing which UI elements are interactable, values from ui_element_enum
  unsigned int mouseWheelUsed : 1;
  unsigned int holdingScrollbar : 1; //whether or not a scrollbar is being held
  unsigned int lastInputType : 2; //0=keyboard, 1=apppad, 2=mouse
  unsigned int gamepadDisabled : 1; //1=gamepad/apppad disabled
  unsigned int quitAppFlag : 1; //0=take no action, 1=quit app
}app_state; //structure containing all app state data (persistent AND temporary)

typedef struct
{
  float uiScale; //scaling factor for UI, in units of 32px
  SDL_Surface *iconSurface; //surface for the application icon
  SDL_Texture *uiThemeTex; //the main texture atlas
  SDL_Texture *textTexCache[TEXT_TEX_CACHE_SIZE]; //cached textures for text rendering
  SDL_Texture *tempTex; //used to store temporary texture data during draw operations
  //using different fonts (rather than resizing a single font) decreases CPU usage at the expense of memory
  TTF_Font *smallFont, *font, *bigFont, *hugeFont; //the default font (in 3 sizes)
  void *fontData; //memory adresses of the font data (must stay alive as long as fonts are used)
  SDL_Gamepad *gamepad;
  SDL_Renderer *renderer;
  SDL_Window *window;
  char *appBasePath, *appPrefPath; //filesystem paths to on-disk resources
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
