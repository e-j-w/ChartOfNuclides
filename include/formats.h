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

//app data parameters (should all be powers of 2)
#define MAX_NUM_STRINGS          128  //maximum number of text strings

//search parameters
#define SEARCH_STRING_MAX_SIZE   256
#define MAX_SEARCH_TOKENS        16
#define MAX_SEARCH_RESULTS       64 //number of results to cache (max 64, indexed by corrRes bitpattern)
#define MAX_DISP_SEARCH_RESULTS  4  //number of results to display
#define SEARCH_RESULT_DATASIZE   4
#define UNUSED_SEARCH_RESULT     4294967295U

//text selection parameters
#define MAX_SELECTABLE_STRS      1024 //maximum number of onscreen text strings that can be selectable at once
#define MAX_SELECTABLE_STR_LEN   256 //maximum length of selectable text strings (should be larger than 32, which is the size used by some string composition functions in data_ops.c)

//context menu parameters
#define MAX_CONTEXT_MENU_ITEMS   4 //maximum number of items in the context menu

//increasing these numbers will increase the size of 
//the nuclear database stored in memory (and on disk)
#define ENSDFSTRBUFSIZE          262144 //2^18
#define MAXMULTPERLEVEL          3
#define MAXNUMNUCL               3500
#define MAXNUMLVLS               200000
#define MAXNUMTRAN               300000
#define MAXSPINPARVAL            190000
#define MAXNUMDECAYMODES         11000
#define MAXNUMREACTIONS          15000
#define MAX_NEUTRON_NUM          200
#define MAX_PROTON_NUM           130

#define MAX_SPIN_VARS            32 //maximum spin variables (ie. J1, J2, J3...) per nuclide

#define ISOMER_MVAL_HL_THRESHOLD    1.0E-3 //half-life (in seconds) lower threshold for an m-value to be assigned to an isomer
#define ISOMER_MVAL_E_THRESHOLD     0.02   //energy (keV) upper threshold for an m-value to be assigned to an isomer

#define NUMSHELLCLOSURES 7
static const uint16_t shellClosureValues[NUMSHELLCLOSURES] = {2,8,20,28,50,82,126};

//thread pool parameters
#define MAX_NUM_THREADS 64 //maximum number of threads allowed in the thread pool
                           //(cannot be > 64 as aliveThreads is uint64_t)
#define THREAD_UPDATE_DELAY 10 //delay (in ms) for each thread to update its state

//structures

typedef struct
{
  double val;
  uint8_t unit; //bits 0 to 6: unit (from value_unit_enum), bit 7: whether value is amibiguous (displayed with '?')
  uint8_t err; //uncertainty value (on trailing sig figs)
  int8_t exponent; //value after exponent (eg. -5 for 4.2E-5), if value is in exponent form
  uint16_t format; //bits 0-3: number of sig figs after the decimal place
  //bit 4: whether or not to use exponent
  //bits:5-8 value type (from value_type_enum)
  //bits 9-15: reserve value dependent on value type (-ve error if VALUETYPE_ASYMERROR,
  //X index if VALUETYPE_X or VALUETYPE_PLUSX)
}dblValWithErr; //parsed value with an error (double precision)

typedef struct
{
  float val;
  uint8_t unit; //bits 0 to 6: unit (from value_unit_enum), bit 7: whether value is amibiguous (displayed with '?')
  uint8_t err; //uncertainty value (on trailing sig figs)
  int8_t exponent; //value after exponent (eg. -5 for 4.2E-5), if value is in exponent form
  uint16_t format; //bits 0-3: number of sig figs after the decimal place
  //bit 4: whether or not to use exponent
  //bits:5-8 value type (from value_type_enum)
  //bits 9-15: reserve value dependent on value type (-ve error if VALUETYPE_ASYMERROR,
  //X index if VALUETYPE_X or VALUETYPE_PLUSX)
  //For non-variable gamma energies, bit 9 specifies whether the value is tentative
}valWithErr; //parsed value with an error

typedef struct
{
  uint32_t rxnStrBufStartPos;
  uint8_t rxnStrLen;
  uint8_t type; //values from reaction_type_enum
}reaction; //reaction populating a level

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
  //bits 10-13: whether value is tentative (values from tentative_sp_enum)
}spinparval; //spin parity value

typedef struct
{
  valWithErr energy; //transition energy, in keV
  valWithErr intensity; //relative intensity
  valWithErr icc; //internal conversion coefficient
  valWithErr delta; //mixing ratio
  uint8_t numMultipoles; //only uses the first couple bits
  uint8_t multipole[MAXMULTPERLEVEL]; //bit 0: E (unset) or M (set), bits 1-4: multipole order, bits 5-6: values from tentative_mult_enum, bit 7: if set, bit 0 corresponds to quadrupole/dipole
  uint8_t finalLvlOffset; //offset of the index of the final level from the initial level
}transition; //a transition between levels

typedef struct
{
  valWithErr energy; //level energy in keV
  valWithErr halfLife;
  uint64_t populatingRxns; //bit-pattern specifying which reactions populate this level
  uint32_t firstTran; //index of first transition from this level
  uint32_t firstSpinParVal; //index of the first spin-parity value for this level
  uint16_t firstDecMode;
  uint16_t numTran; //number of gamma rays in this level
  uint8_t numSpinParVals; //number of assigned spin parity values for this level
  int8_t numDecModes; //-1 by default for no decay modes specified (assume 100% IT in that case)
  float decayProb[DECAYMODE_ENUM_LENGTH]; //% probability of each decay mode for this level
  uint8_t format; //bit 0: whether spin-parity values are half integer (if set, then spinVal is multiplied by 0.5)
  //bits 1-4: labels for special levels (see special_level_enum)
  //bits 5-7: m-value for isomer levels
}level; //an individual excited level

typedef struct
{
  valWithErr qbeta, qbetaplus, qec, qalpha; //Q(beta-), Q(beta+), Q(e- capture), Q(alpha) 
  valWithErr sp, sn; //proton and neutron separation energies
  dblValWithErr massExcess; //mass excess (keV), double precision needed 'cause that's how well masses are known
  dblValWithErr beA; //binding energy per nucleon (keV), double precision needed 'cause that's how well masses are known
  dblValWithErr massAMU; //mass in atomic mass units (u), double precision needed 'cause that's how well masses are known
  int16_t N; //neutrons in nuclide
  int16_t Z; //protons in nuclide
  uint32_t firstLevel; //index of first level in this nuclide
  uint16_t numLevels; //number of excited levels in this nuclide
  uint8_t gsLevel; //which level in the nucleus (0 indexed) is the ground state (determined when parsing ENSDF, not always 0)
  uint32_t longestIsomerLevel; //which isomer in the nucleus is longest lived (=MAXNUMLVLS if no isomers)
  uint8_t longestIsomerMVal; //m-value of longest lived isomer (eg. 1 or 2 for 178m1Hf vs. 178m2Hf)
  uint8_t numIsomerMVals; //total number of isomer m-values assigned
  uint16_t firstRxn; //index of first reaction populating this nuclide
  uint8_t numRxns;
  valWithErr abundance;
  uint8_t flags; //bits 0 to 1: observation flag (observed/unobserved/inferred/tentative), 
                 //bit 2: set if any internal conversion coefficients are measured in this nuclide
                 //bit 3: set if any gamma mixing ratios are measured in this nuclide
}nucl; //gamma data for a given nuclide

typedef struct
{
  uint32_t numLvls; //number of levels across all nuclides
  uint32_t numTran; //number of transitions across all levels
  int16_t numNucl; //number of nuclides for which data is stored (-1 if no nuclides)
  uint16_t numDecModes; //number of decay modes across all levels
  uint16_t numRxns; //number of populating reactions across all nuclides
  uint32_t numSpinParVals; //number of spin-parity values across all levels
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
  reaction rxn[MAXNUMREACTIONS]; //reactions populating nuclides
  spinparval spv[MAXSPINPARVAL];
  char ensdfStrBuf[ENSDFSTRBUFSIZE]; //huge buffer for directly copied ENSDF strings (reaction strings, comments, etc)
  uint32_t ensdfStrBufLen;
}ndata; //complete set of gamma data for all nuclides


typedef struct
{
  uint8_t resultType;  //values from search_agent_enum
  uint32_t resultVal[SEARCH_RESULT_DATASIZE];  //defines the result (eg. nuclide index for nuclide search)
  float relevance;     //a value which defines how relevant the result is (for sorting)
}search_result;

typedef struct
{
  SDL_FColor bgCol; //background colors
  SDL_Color textColNormal, textColInactive; //colors for text (cannot be floating point as this is not supported by SDL_ttf)
  SDL_FColor modNormalCol, modMouseOverCol, modSelectedCol, modSelectedAndMouseOverCol; //color modulations for UI elements in different states
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
  float uiUserScale; //scaling factor for UI, user preference
  float chartPosX, chartPosY, chartZoomScale; //x and y in chart units
  float chartZoomStartScale, chartZoomToScale; //start and target zoom scale
  float chartZoomToX, chartZoomToY; //in chart units
  float chartPanToX, chartPanToY; //in chart units
  float chartZoomStartMouseX, chartZoomStartMouseY, chartZoomStartMouseXFrac, chartZoomStartMouseYFrac; //in chart units
  float chartPanStartX, chartPanStartY;
  float chartDragStartX, chartDragStartY;
  float chartDragStartMouseX, chartDragStartMouseY; //in scaled pixels
  float textDragStartMouseX; //in scaled pixels
  float timeSinceZoomStart, timeSincePanStart, totalPanTime, timeSinceFCScollStart, timeSinceFCNuclChangeStart;
  float infoBoxTableHeight; //the height in unscaled pixels needed to show the info box ground and isomeric state info
  float infoBoxWidth, infoBoxEColOffset, infoBoxJpiColOffset, infoBoxHlColOffset, infoBoxDcyModeColOffset;
  float infoBoxCurrentX, infoBoxCurrentY, infoBoxCurrentDispWidth, infoBoxCurrentDispHeight, infoBoxPrevX, infoBoxPrevY, infoBoxPrevDispWidth, infoBoxPrevDispHeight;
  float fullInfoColWidth[LLCOLUMN_ENUM_LENGTH];
  uint32_t fullInfoQValEntryPos[QVAL_ENUM_LENGTH]; //positions to show Q values in-line with the level list
  uint8_t fullInfoQValOrder[QVAL_ENUM_LENGTH]; //specifies the order in which to display Q values
  float nuclFullInfoScrollStartY, nuclFullInfoScrollToY, nuclFullInfoScrollY; //full level info view: number of lines scrolled in the y-direction
  uint16_t nuclFullInfoMaxScrollY; //maximum scroll position, in lines
  uint16_t nuclFullInfoShownColumns; //bit-pattern describing which columns are shown in the full level info view (values from level_list_column_enum)
  uint32_t nuclFullInfoLastDispLvl; //the index of the last level to be displayed on the level list (used to determine which Q values are shown in-line)
  uint8_t rxnMenuColumns; //how many columns to use in the reaction menu
  uint8_t mouseOverRxn, mouseHoldRxn, selectedRxn; //which item in the reaction menu is moused-over/selected, =255 if none
  uint64_t shownElements; //bit pattern describing which UI elements are being shown, values from ui_element_enum
  uint32_t uiAnimPlaying; //bit pattern describing which UI animations are playing
  float timeLeftInUIAnimation[UIANIM_ENUM_LENGTH]; //time left in each UI animation
  uint16_t windowXRes, windowYRes; //resolution of the window
  uint16_t windowXRenderRes, windowYRenderRes; //render resolution (in pixels) taking HI-DPI into account
  int16_t uiElemPosX[UIELEM_ENUM_LENGTH], uiElemPosY[UIELEM_ENUM_LENGTH], uiElemWidth[UIELEM_ENUM_LENGTH], uiElemHeight[UIELEM_ENUM_LENGTH]; //UI element positioning, used both for drawing elements and calculating mouse interactions 
  uint16_t uiElemExtPlusX[UIELEM_ENUM_LENGTH], uiElemExtPlusY[UIELEM_ENUM_LENGTH], uiElemExtMinusX[UIELEM_ENUM_LENGTH], uiElemExtMinusY[UIELEM_ENUM_LENGTH]; //'extension' of each UI element, to allow mouse interactions outside of the visible area of the UI element (eg. so that checkbox text can be clicked as well as the checkbox itself)
  uint16_t searchEntryDispStartChar, searchEntryDispNumChars; //search string display state
  uint8_t interfaceSizeInd; //user preference for UI scaling, values from ui_scale_enum
  unsigned int fcScrollInProgress : 1;
  unsigned int fcScrollFinished : 1;
  unsigned int fcNuclChangeInProgress : 1;
  unsigned int fcNuclChangeFinished : 1;
  unsigned int panInProgress : 1;
  unsigned int panFinished : 1;
  unsigned int chartDragInProgress : 1;
  unsigned int textDragInProgress : 2; //0=no text drag, 1=dragging over text, 2=mouse hover over text
  unsigned int chartDragFinished : 1;
  unsigned int textDragFinished : 1;
  unsigned int zoomInProgress : 1;
  unsigned int zoomFinished : 1;
  unsigned int useLifetimes : 1;
  unsigned int useUIAnimations : 1;
  unsigned int drawShellClosures : 1;
  unsigned int drawPerformanceStats : 1; //0=don't draw, 1=draw
  unsigned int windowFullscreenMode : 1; //true if the window is fullscreen
  unsigned int forceRedraw : 1; //true if a re-draw should be forced
}drawing_state; //struct containing values used for drawing

typedef struct
{
  char searchString[SEARCH_STRING_MAX_SIZE];   //the user's search query
  char searchTok[MAX_SEARCH_TOKENS][16];
  //the search results that are currently being worked on
  //will only be displayed once the search is completed
  search_result updatedResults[MAX_SEARCH_RESULTS];
  uint8_t numUpdatedResults; //the number of results returned so far
  //the search results to display
  search_result results[MAX_SEARCH_RESULTS];
  uint8_t numResults; //the number of results returned so far
  uint8_t numSearchTok;
  uint8_t boostedResultType;  //result type which is prioritized, values from search_agent_enum
  uint16_t boostedNucl; //nuclide which is prioritized, MAXNUMNUCL if none (or if only one nuclide is being searched, specifies that nuclide)
  uint32_t finishedSearchAgents; //bit pattern specifying which search agents have finished
  uint8_t searchInProgress; //values from search_state_enum
  SDL_Semaphore *canUpdateResults;
}search_state; //struct containing search data

typedef struct
{
  SDL_FRect selectableStrRect[MAX_SELECTABLE_STRS]; //position and size of selectable text
  char selectableStrTxt[MAX_SELECTABLE_STRS][MAX_SELECTABLE_STR_LEN]; //the actual strings that are selectable
  uint8_t selectableStrProp[MAX_SELECTABLE_STRS]; //bits 0-2: font size of each selectable string (from font_size_enum), bit 3: whether string is clickable, bit 4: click action (from text_click_action_enum)
  uint16_t selectableStrClickPar[MAX_SELECTABLE_STRS];
  uint16_t numSelStrs;
  uint16_t selectedStr; //65535 if nothing selected
  uint8_t selStartPos, selEndPos; //which character indices correspond to the start and end of the selected text
  uint8_t selStrsModifiable; //flag which is set whenever the selection strings can be updated, and is cleared at the end of each frame
}text_selection_state; //struct containing text selection data

typedef struct
{
  char headerText[32];
  uint8_t useHeaderText;
  uint8_t numContextMenuItems; //number of items to show in the context menu
  uint8_t contextMenuItems[MAX_CONTEXT_MENU_ITEMS]; //values from context_menu_item_enum
  uint8_t mouseOverContextItem; //255 if none moused over
  uint8_t clickedContextItem; //255 if none clicked on
  uint16_t selectionInd; //index of nuclide or other item corresponding to this context menu
}context_menu_state; //struct containing context menu data

typedef struct
{
  drawing_state ds;          //the state information for drawing
  search_state ss;           //the state information for search queries
  text_selection_state tss;  //the state information for text selection
  context_menu_state cms;    //the state information for right-click context menus
  char copiedTxt[MAX_SELECTABLE_STR_LEN];  //buffer for copying text to clipboard
  int searchCursorPos, searchSelectionLen; //for search query text editing
  float mouseXPx, mouseYPx; //current position of the mouse, in pixels
  float mouseHoldStartPosXPx, mouseHoldStartPosYPx; //mouse position at the start of the last mouse button down event, in pixels
  float mouseClickPosXPx, mouseClickPosYPx; //mouse position at the start of a mouse (left) button up event, in pixels
  float mouseRightClickPosXPx, mouseRightClickPosYPx;
  float mouseScrollbarClickPosYPx;
  float zoomDeltaVal; //amount to zoom by, equivalent to mouse wheel or touchpad scroll delta, can be very small for subtle touchpad events
  uint32_t inputFlags;
  uint16_t gamepadDeadzone,gamepadTriggerDeadzone;
  int16_t lastAxisValLX, lastAxisValLY, lastAxisValRX, lastAxisValRY; //used to mask out spurious events in the gamepad axis deadzone
  uint8_t activeAxisX, activeAxisY; //the last used analog stick axis
  uint8_t chartView; //values from chart_view_enum
  uint8_t clickedUIElem; //which UI element was last clicked, values from ui_element_enum
  uint8_t lastOpenedMenu; //values from ui_element_enum
  float timeSinceLastSec; //used to track play time, the amount of seconds that have passed since the last tick
  float mouseHoldTime; //amount of time the mouse button has been held down for
  uint8_t mouseoverElement; //which UI element is moused over, =UIELEM_ENUM_LENGTH if none are moused over
  uint8_t mouseholdElement; //which UI element is the mouse button being held over, =UIELEM_ENUM_LENGTH if none
  uint8_t uiState, lastUIState; //modal state of the UI, values from ui_state_enum
  uint16_t chartSelectedNucl; //nucleus selected on the chart, =MAXNUMNUCL if none selected
  uint64_t interactableElement; //bit pattern describing which UI elements are interactable, values from ui_element_enum
  unsigned int mouseMovedDuringClick : 1; //whether or not the mouse was moved significantly while the mouse button is held down
  unsigned int searchStrUpdated : 1;
  unsigned int kbdModVal : 2; //values from kbd_mod_enum
  unsigned int lastInputType : 2; //0=keyboard, 1=apppad, 2=mouse
  
  unsigned int quitAppFlag : 1; //0=take no action, 1=quit app
}app_state; //structure containing all app state data (persistent AND temporary)

typedef struct
{
  SDL_Surface *screenshot; //surface for any screenshots
  unsigned int takingScreenshot : 2; //0=not taking a screenshot, 1=taking a screenshot at the end of the current frame, 2=saving screenshot
}screenshot_data; //structure containing data relating to resources such as textures and fonts

typedef struct
{
  screenshot_data ssdat;
  size_t themeOffset; //data file offset for the embedded theme
  float uiDPIScale; //scaling factor for UI, for HI-DPI only
  float uiScale; //overall scaling factor for UI
  float uiThemeScale; //scale of the UI theme texture in GPU memory
  SDL_Surface *iconSurface; //surface for the application icon
  SDL_Texture *uiThemeTex; //the main texture atlas
  SDL_Texture *tempTex; //used to store temporary texture data during draw operations
  //using different fonts (rather than resizing a single font) decreases CPU usage at the expense of memory
  TTF_TextEngine *te;
  TTF_Font *font[FONTSIZE_ENUM_LENGTH]; //the default font
  void *fontData; //memory address of the font data (must stay alive as long as fonts are used)
  SDL_Cursor *defaultCursor, *dragCursor, *textEntryCursor;
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

//data passed to a thread in the thread pool
typedef struct
{
  //pointers to game data accessible to threads go here
  uint8_t threadNum; //unique identifier for this thread
  uint8_t threadState; //state of the thread, values from thread_state_enum
  uint8_t threadPar; //thread parameter (eg. which search agent to run)
  //data that the thread has access to:
  app_state *state;        //the application state
  app_data *dat;           //the application data
}thread_data;

//structure for thread management
typedef struct
{
  uint8_t numThreads; //number of threads to have active in the thread pool
  uint64_t aliveThreads; //bit-pattern specifying which threads are in use
  thread_data threadData[MAX_NUM_THREADS]; //data to give to each thread
  uint8_t masterThreadState; //what state the threads are expected to be in, values from thread_state_enum
}thread_manager_state;

//structure containing all application data
//used so that all app data can be allocated in a single block of memory
typedef struct
{
  app_state state;
  app_data dat;
  thread_manager_state tms;
  resource_data rdat;
}global_data;

#endif
