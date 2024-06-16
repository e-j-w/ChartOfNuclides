/* © J. Williams 2017-2024 */
/* Values which define the spacing and visual layout of GUI elements in the app,
which can be referenced by both the GUI draw code as well as the interaction code
(for mouse pointer interactions). */

#ifndef GUI_CONST_H
#define GUI_CONST_H

/* GLOBAL GUI PARAMETERS */

#define UI_ANIM_LENGTH       0.2f //length of UI animations, in seconds
#define SPEC_ZOOM_TIME       0.5f //amount of time for spectrum zoom animations

#define UI_TILE_SIZE         32  //pixel size of a UI tile at 1x scaling
#define UI_PADDING_SIZE      4   //pixel size of the default UI element padding
//dimensions of the UI theme texture, in units of tiles
#define UI_THEME_TEX_TILES_X 3
#define UI_THEME_TEX_TILES_Y 11

//positions of UI theme elements, in tiles
#define UITHEME_BUTTON_TILE_X 0
#define UITHEME_BUTTON_TILE_Y 0
#define UITHEME_PANELBG_TILE_X 0
#define UITHEME_PANELBG_TILE_Y 5

//UI theme icons
#define NUM_THEME_ICONS  UIICON_ENUM_LENGTH
static const uint8_t UITHEME_ICON_TILE_X[NUM_THEME_ICONS] = {0,1,2};
static const uint8_t UITHEME_ICON_TILE_Y[NUM_THEME_ICONS] = {10,10,10};

#define SMALL_FONT_SIZE      13
#define DEFAULT_FONT_SIZE    15
#define BIG_FONT_SIZE        19
#define HUGE_FONT_SIZE       24

/* UI LAYOUT CONSTANTS */
/* see updateUIElemPositions in data_ops.c for how these values are assigned */

#define MENU_BAR_HEIGHT      (UI_TILE_SIZE+2*UI_PADDING_SIZE)

#define MENU_BUTTON_POS_XR    UI_PADDING_SIZE   //in unscaled pixels
#define MENU_BUTTON_POS_Y     UI_PADDING_SIZE   //in unscaled pixels
#define MENU_BUTTON_WIDTH     40 //in unscaled pixels

#define PRIMARY_MENU_POS_XR   (UI_PADDING_SIZE-4)   //in unscaled pixels
#define PRIMARY_MENU_POS_Y    (UI_TILE_SIZE+2*UI_PADDING_SIZE)  //in unscaled pixels
#define PRIMARY_MENU_WIDTH    160 //in unscaled pixels
#define PRIMARY_MENU_HEIGHT   280 //in unscaled pixels

#define DIMMER_OPACITY        0.20f

//message box (for errors/warnings)
#define MESSAGE_BOX_WIDTH            360 //in unscaled pixels
#define MESSAGE_BOX_HEIGHT           240 //in unscaled pixels
#define MESSAGE_BOX_HEADERTXT_Y      (6*UI_PADDING_SIZE)
#define MESSAGE_BOX_OK_BUTTON_WIDTH  120
#define MESSAGE_BOX_OK_BUTTON_YB     (3*UI_PADDING_SIZE)

//chart of nuclides
#define DEFAULT_NUCLBOX_DIM                8.0f
#define MIN_CHART_ZOOM_SCALE               0.5f
#define MAX_CHART_ZOOM_SCALE               32.0f
#define CHART_ZOOM_TIME                    0.2f //time (in seconds) for the zoom animation to finish
#define NUCLBOX_LABEL_MARGIN               1.0f //in scaled pixels
#define CHART_PAN_TIME                     0.2f //time (in seconds) for the pan animation to finish
#define CHART_PAN_DIST                     20.0f //in tiles (scaled by zoom factor)
#define CHART_AXIS_DEPTH                   40.0f //in scaled pixels
#define CHART_SHELLCLOSURELINE_THICKNESS   3.0f

#define NUCL_INFOBOX_WIDTH            (MIN_RENDER_WIDTH-2*UI_PADDING_SIZE-2*CHART_AXIS_DEPTH) //in unscaled pixels
#define NUCL_INFOBOX_HEIGHT           240 //in unscaled pixels

//performance overlay
#define PERF_OVERLAY_BUTTON_X_ANCHOR  16
#define PERF_OVERLAY_BUTTON_Y_ANCHOR  16
#define PERF_OVERLAY_Y_SPACING        28

//colors for drawing
static const SDL_FColor blackCol = {0.0f,0.0f,0.0f,1.0f};
static const SDL_FColor grayCol = {0.5f,0.5f,0.5f,1.0f};
static const SDL_FColor redCol = {1.0f,0.0f,0.0f,1.0f};
static const SDL_FColor whiteCol = {1.0f,1.0f,1.0f,1.0f};
static const SDL_FColor whiteTransparentCol = {1.0f,1.0f,1.0f,0.7f};

static const SDL_Color whiteCol8Bit = {255,255,255,255};
static const SDL_Color blackCol8Bit = {0,0,0,255};

#endif
