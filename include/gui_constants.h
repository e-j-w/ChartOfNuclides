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

/* Values which define the spacing and visual layout of GUI elements in the app,
which can be referenced by both the GUI draw code as well as the interaction code
(for mouse pointer interactions). */

#ifndef GUI_CONST_H
#define GUI_CONST_H

/* GLOBAL GUI PARAMETERS */

//minimum dimensions of the window, assuming 1x UI scaling
#define MIN_RENDER_WIDTH  800
#define MIN_RENDER_HEIGHT 480

//maximum dimensions of the window
#define MAX_RENDER_WIDTH  16384
#define MAX_RENDER_HEIGHT 16384

#define UI_ANIM_LENGTH        0.2f //length of UI animations, in seconds
#define SHORT_UI_ANIM_LENGTH  0.1f

#define UI_TILE_SIZE         32  //pixel size of a UI tile at 1x scaling
#define UI_PADDING_SIZE      4   //pixel size of the default UI element padding
#define PANEL_EDGE_SIZE      6   //pixel size of the unusable edge of a panel background (eg. the shadow section)
//dimensions of the UI theme texture, in units of tiles
#define UI_THEME_TEX_TILES_X 3
#define UI_THEME_TEX_TILES_Y 12

//positions of UI theme elements, in tiles
#define UITHEME_BUTTON_TILE_X 0
#define UITHEME_BUTTON_TILE_Y 0
#define UITHEME_PANELBG_TILE_X 0
#define UITHEME_PANELBG_TILE_Y 5
#define UITHEME_HIGHLIGHT_TILE_X 0
#define UITHEME_HIGHLIGHT_TILE_Y 2
#define UITHEME_SCROLLBAR_TILE_X 3
#define UITHEME_SCROLLBAR_TILE_Y 0

//UI theme icons
static const uint8_t UITHEME_ICON_TILE_X[UIICON_ENUM_LENGTH] = {0,1,2,1,2,0,1,0,1};
static const uint8_t UITHEME_ICON_TILE_Y[UIICON_ENUM_LENGTH] = {10,10,10,9,9,8,8,11,11};

//font sizes (assuming UI scale of 1)
static const uint16_t fontSizes[FONTSIZE_ENUM_LENGTH] = {13,15,19};

//user UI scaling factors
static const float uiScales[UISCALE_ENUM_LENGTH] = {1.0f,1.20f,1.5f,1.75f};

/* UI LAYOUT CONSTANTS */
/* see updateUIElemPositions in data_ops.c for how these values are assigned */
/* make sure these are always fully enclosed by brackets, as they may be scaled by the code */

#define MENU_BUTTON_POS_XR         UI_PADDING_SIZE   //in unscaled pixels
#define MENU_BUTTON_POS_Y          UI_PADDING_SIZE   //in unscaled pixels
#define MENU_BUTTON_WIDTH          40 //in unscaled pixels

#define PRIMARY_MENU_ITEM_SPACING  40 //in unscaled pixels
#define PRIMARY_MENU_POS_XR        (MENU_BUTTON_POS_XR)   //in unscaled pixels
#define PRIMARY_MENU_POS_Y         (UI_TILE_SIZE+UI_PADDING_SIZE)  //in unscaled pixels
#define PRIMARY_MENU_WIDTH         200 //in unscaled pixels
#define PRIMARY_MENU_HEIGHT        (2*PRIMARY_MENU_ITEM_SPACING + 2*PANEL_EDGE_SIZE + 3*UI_PADDING_SIZE) //in unscaled pixels

#define CHARTVIEW_BUTTON_POS_XR    (2*UI_PADDING_SIZE + MENU_BUTTON_WIDTH)   //in unscaled pixels
#define CHARTVIEW_BUTTON_POS_Y     UI_PADDING_SIZE   //in unscaled pixels
#define CHARTVIEW_BUTTON_WIDTH     120 //in unscaled pixels

#define CHARTVIEW_MENU_ITEM_SPACING  40 //in unscaled pixels
#define CHARTVIEW_MENU_POS_XR        (CHARTVIEW_BUTTON_POS_XR)   //in unscaled pixels
#define CHARTVIEW_MENU_POS_Y         (UI_TILE_SIZE+UI_PADDING_SIZE)  //in unscaled pixels
#define CHARTVIEW_MENU_WIDTH         180 //in unscaled pixels
#define CHARTVIEW_MENU_HEIGHT        ((CHARTVIEW_ENUM_LENGTH+1)*CHARTVIEW_MENU_ITEM_SPACING + 2*PANEL_EDGE_SIZE + 4*UI_PADDING_SIZE) //in unscaled pixels

#define DIMMER_OPACITY               0.20f

//message box (for errors/warnings)
#define MESSAGE_BOX_WIDTH            360 //in unscaled pixels
#define MESSAGE_BOX_HEIGHT           240 //in unscaled pixels
#define MESSAGE_BOX_HEADERTXT_Y      (6*UI_PADDING_SIZE)
#define MESSAGE_BOX_OK_BUTTON_WIDTH  120
#define MESSAGE_BOX_OK_BUTTON_YB     (3*UI_PADDING_SIZE)

//about box (credits etc.)
#define ABOUT_BOX_WIDTH            420 //in unscaled pixels
#define ABOUT_BOX_HEIGHT           300 //in unscaled pixels
#define ABOUT_BOX_HEADERTXT_Y      (8*UI_PADDING_SIZE)
#define ABOUT_BOX_VERSION_Y        (14*UI_PADDING_SIZE)
#define ABOUT_BOX_STR1_Y           110 //in unscaled pixels
#define ABOUT_BOX_STR2_Y           166 //in unscaled pixels
#define ABOUT_BOX_OK_BUTTON_WIDTH  120
#define ABOUT_BOX_OK_BUTTON_YB     (4*UI_PADDING_SIZE)

//preferences dialog
#define PREFS_DIALOG_WIDTH                      340 //in unscaled pixels
#define PREFS_DIALOG_HEIGHT                     260 //in unscaled pixels
#define PREFS_DIALOG_HEADERTXT_X                (6*UI_PADDING_SIZE)
#define PREFS_DIALOG_HEADERTXT_Y                (6*UI_PADDING_SIZE)
#define PREFS_DIALOG_PREF_Y_SPACING             36 //in unscaled pixels
#define PREFS_DIALOG_PREFCOL1_X                 (6*UI_PADDING_SIZE)
#define PREFS_DIALOG_PREFCOL1_Y                 80 //in unscaled pixels
#define PREFS_DIALOG_UISCALE_BUTTON_WIDTH       120 //in unscaled pixels
#define PREFS_DIALOG_UISCALE_MENU_WIDTH         140 //in unscaled pixels
#define PREFS_DIALOG_UISCALE_MENU_ITEM_SPACING  40 //in unscaled pixels

//chart of nuclides
#define DEFAULT_NUCLBOX_DIM                8.0f
#define DEFAULT_LOWBOX_PADDING             0.25f //padding for the 'lower' box containing isomer info etc.
#define MIN_CHART_ZOOM_SCALE               0.5f
#define MAX_CHART_ZOOM_SCALE               32.0f
#define CHART_ZOOM_TIME                    0.2f //time (in seconds) for the zoom animation to finish
#define NUCLBOX_LABEL_MARGIN               1.0f //in scaled pixels
#define NUCLBOX_LABEL_SMALLMARGIN          0.25f //in scaled pixels
#define CHART_KEY_PAN_TIME                 0.1f //time (in seconds) for the pan animation to finish (keyboard)
#define CHART_DOUBLECLICK_PAN_TIME         0.3f //time (in seconds) for the pan animation to finish (double click)
#define CHART_PAN_DIST                     10.0f //in tiles (scaled by zoom factor)
#define CHART_AXIS_DEPTH                   40.0f //in scaled pixels
#define CHART_SHELLCLOSURELINE_THICKNESS   3.0f

#define NUCL_INFOBOX_X_PADDING              (UI_PADDING_SIZE+CHART_AXIS_DEPTH) //in unscaled pixels
#define NUCL_INFOBOX_WIDTH                  500 //in unscaled pixels
#define NUCL_INFOBOX_MIN_HEIGHT             120 //in unscaled pixels
#define NUCL_INFOBOX_BIGLINE_HEIGHT         30.0f //in unscaled pixels
#define NUCL_INFOBOX_SMALLLINE_HEIGHT       20.0f //in unscaled pixels
#define NUCL_INFOBOX_ENERGY_COL_OFFSET      0.0f //in unscaled pixels
#define NUCL_INFOBOX_JPI_COL_OFFSET         140.0f //in unscaled pixels
#define NUCL_INFOBOX_HALFLIFE_COL_OFFSET    220.0f //in unscaled pixels
#define NUCL_INFOBOX_DECAYMODE_COL_OFFSET   350.0f //in unscaled pixels
#define NUCL_INFOBOX_ALLLEVELS_BUTTON_WIDTH 170 //in unscaled pixels

#define NUCL_FULLINFOBOX_SCROLL_TIME                0.1f //time (in seconds) for the list scroll animation to finish
#define NUCL_FULLINFOBOX_BACKBUTTON_POS_XR          (2*MENU_BUTTON_POS_XR + MENU_BUTTON_WIDTH)
#define NUCL_FULLINFOBOX_BACKBUTTON_POS_Y           MENU_BUTTON_POS_Y
#define NUCL_FULLINFOBOX_BACKBUTTON_WIDTH           NUCL_INFOBOX_ALLLEVELS_BUTTON_WIDTH
#define NUCL_FULLINFOBOX_QVAL_POS_X                 (6*UI_PADDING_SIZE) //in unscaled pixels
#define NUCL_FULLINFOBOX_QVAL_POS_Y                 60.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_ENERGY_COL_WIDTH           140.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_JPI_COL_WIDTH              120.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_HALFLIFE_COL_WIDTH         130.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_EGAMMA_COL_WIDTH           120.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_IGAMMA_COL_WIDTH           100.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_MGAMMA_COL_WIDTH           100.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_FINALLEVEL_E_COL_WIDTH     100.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_FINALLEVEL_JPI_COL_WIDTH   120.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_ALLCOL_WIDTH               (NUCL_FULLINFOBOX_ENERGY_COL_WIDTH+NUCL_FULLINFOBOX_JPI_COL_WIDTH+NUCL_FULLINFOBOX_HALFLIFE_COL_WIDTH+NUCL_FULLINFOBOX_EGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_IGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_MGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_FINALLEVEL_E_COL_WIDTH+NUCL_FULLINFOBOX_FINALLEVEL_JPI_COL_WIDTH)
#define NUCL_FULLINFOBOX_ALLCOL_EXCLM_WIDTH         (NUCL_FULLINFOBOX_ENERGY_COL_WIDTH+NUCL_FULLINFOBOX_JPI_COL_WIDTH+NUCL_FULLINFOBOX_HALFLIFE_COL_WIDTH+NUCL_FULLINFOBOX_EGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_IGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_FINALLEVEL_E_COL_WIDTH+NUCL_FULLINFOBOX_FINALLEVEL_JPI_COL_WIDTH)
#define NUCL_FULLINFOBOX_ALLCOL_EXCLMJF_WIDTH       (NUCL_FULLINFOBOX_ENERGY_COL_WIDTH+NUCL_FULLINFOBOX_JPI_COL_WIDTH+NUCL_FULLINFOBOX_HALFLIFE_COL_WIDTH+NUCL_FULLINFOBOX_EGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_IGAMMA_COL_WIDTH+NUCL_FULLINFOBOX_FINALLEVEL_E_COL_WIDTH)
#define NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y     92.0f //in unscaled pixels
#define NUCL_FULLINFOBOX_LEVELLIST_POS_Y            (NUCL_FULLINFOBOX_LEVELLIST_HEADER_POS_Y + 58.0f) //in unscaled pixels

//performance overlay
#define PERF_OVERLAY_BUTTON_X_ANCHOR  16
#define PERF_OVERLAY_BUTTON_Y_ANCHOR  16
#define PERF_OVERLAY_Y_SPACING        28

//colors for drawing
static const SDL_FColor blackCol = {0.0f,0.0f,0.0f,1.0f};
static const SDL_FColor grayCol = {0.5f,0.5f,0.5f,1.0f};
static const SDL_FColor darkGrayCol = {0.2f,0.2f,0.2f,1.0f};
static const SDL_FColor redCol = {1.0f,0.0f,0.0f,1.0f};
static const SDL_FColor whiteCol = {1.0f,1.0f,1.0f,1.0f};
static const SDL_FColor whiteTransparentCol = {1.0f,1.0f,1.0f,0.7f};

static const SDL_Color whiteCol8Bit = {255,255,255,255};
static const SDL_Color blackCol8Bit = {0,0,0,255};

#endif
