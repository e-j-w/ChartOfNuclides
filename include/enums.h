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

/* As the filename says, these are enums used throughout the app */

enum input_type_enum{
INPUT_TYPE_KEYBOARD, INPUT_TYPE_GAMEPAD, INPUT_TYPE_MOUSE,
INPUT_TYPE_ENUM_LENGTH
};
enum input_enum {
INPUT_DOWN, INPUT_UP, INPUT_LEFT, INPUT_RIGHT, //directional inputs
INPUT_ALTDOWN, INPUT_ALTUP, INPUT_ALTLEFT, INPUT_ALTRIGHT, 
INPUT_ZOOM, INPUT_DOUBLECLICK, INPUT_SELECT, INPUT_BACK, 
INPUT_MENU, INPUT_ENUM_LENGTH
};
enum loc_string_enum{LOCSTR_APPLY,LOCSTR_CANCEL,LOCSTR_OK,LOCSTR_NODB,
LOCSTR_GM_STATE,LOCSTR_QALPHA,LOCSTR_QBETAMNUS,LOCSTR_SP,LOCSTR_SN,LOCSTR_UNKNOWN,
LOCSTR_LEVELINFO_HEADER,LOCSTR_ENERGY_KEV,LOCSTR_JPI,LOCSTR_HALFLIFE,
LOCSTR_LIFETIME,LOCSTR_DECAYMODE,LOCSTR_ENERGY_GAMMA,LOCSTR_INTENSITY_GAMMA,
LOCSTR_MULTIPOLARITY_GAMMA,LOCSTR_FINALLEVEL,LOCSTR_PROTONSDESC,LOCSTR_NEUTRONSDESC,
LOCSTR_NOTNATURAL,LOCSTR_ALLLEVELS,LOCSTR_BACKTOSUMMARY,LOCSTR_MENUITEM_PREFS,
LOCSTR_MENUITEM_ABOUT,
LOCSTR_ABOUTSTR_VERSION,LOCSTR_ABOUTSTR_1,LOCSTR_ABOUTSTR_2,LOCSTR_ABOUTSTR_3,
LOCSTR_ABOUTSTR_4,LOCSTR_PREF_SHELLCLOSURE,LOCSTR_PREF_LIFETIME,LOCSTR_PREF_UIANIM,
LOCSTR_PREF_UISCALE,LOCSTR_SMALL,LOCSTR_DEFAULT,LOCSTR_LARGE,LOCSTR_HUGE,
LOCSTR_SL_HOYLE,LOCSTR_SL_NATURALLYOCCURINGISOMER,LOCSTR_SL_CLOCKISOMER,
LOCSTR_CHARTVIEW_MENUTITLE,LOCSTR_CHARTVIEW_LIFETIME,
LOCSTR_CHARTVIEW_HALFLIFE, //GUI code expects menu item strings to be defined in order here
LOCSTR_CHARTVIEW_DECAYMODE,
LOCSTR_CHARTVIEW_2PLUS,
LOCSTR_CHARTVIEW_R42,
LOCSTR_SEARCH_PLACEHOLDER,
LOCSTR_ENUM_LENGTH
}; //localization srings
enum draw_opt_enum{
DRAWOPT_NONE,
DRAWOPT_HORFLIP,        //horizontal flip
DRAWOPT_VERTFLIP,       //vertical flip
DRAWOPT_HORANDVERTFLIP, //horizontal and vertical flip
DRAWOPT_TOPHALFONLY,    //top half only
DRAWOPT_BOTHALFONLY,    //bottom half only
DRAWOPT_LEFTHALFONLY,   //left half only
DRAWOPT_RIGHTHALFONLY,  //right half only
DRAWOPT_RIGHTALIGN,     //right-aligned, no flip
DRAWOPT_CENTERALIGN,    //center-aligned, no flip
DRAWOPT_ENUM_LENGTH
};
enum alignment_enum{
ALIGN_LEFT,
ALIGN_CENTER,
ALIGN_RIGHT,
ALIGN_ENUM_LENGTH
};
enum highlight_state_enum{
HIGHLIGHT_NORMAL,
HIGHLIGHT_MOUSEOVER,
HIGHLIGHT_SELECTED,
HIGHLIGHT_INACTIVE,
HIGHLIGHT_ENUM_LENGTH
};
enum uielem_type_enum{
UIELEMTYPE_BUTTON, 
UIELEMTYPE_ENTRYBOX,
UIELEMTYPE_ENUM_LENGTH
};
enum ui_element_enum{
UIELEM_CHARTOFNUCLIDES, //some parts of the code assume this is the first enum entry, don't change
UIELEM_MSG_BOX_OK_BUTTON,
UIELEM_MSG_BOX, //used to show warnings or errors
UIELEM_ABOUT_BOX_OK_BUTTON,
UIELEM_ABOUT_BOX,
UIELEM_UISM_SMALL_BUTTON,
UIELEM_UISM_DEFAULT_BUTTON,
UIELEM_UISM_LARGE_BUTTON,
UIELEM_UISM_HUGE_BUTTON,
UIELEM_PREFS_UISCALE_MENU, //buttons in ui scale menu should be defined in the entries directly preceding this, in top to bottom order
UIELEM_PREFS_DIALOG_UISCALE_DROPDOWN,
UIELEM_PREFS_DIALOG_SHELLCLOSURE_CHECKBOX,
UIELEM_PREFS_DIALOG_LIFETIME_CHECKBOX,
UIELEM_PREFS_DIALOG_UIANIM_CHECKBOX,
UIELEM_PREFS_DIALOG_CLOSEBUTTON,
UIELEM_PREFS_DIALOG, //buttons in prefs dialog should be defined in the entries directly preceding this, in top to bottom order
UIELEM_CVM_HALFLIFE_BUTTON,
UIELEM_CVM_DECAYMODE_BUTTON,
UIELEM_CVM_2PLUS_BUTTON,
UIELEM_CVM_R42_BUTTON,
UIELEM_CHARTVIEW_MENU, //buttons in chart view menu should be defined in the entries directly preceding this, in top to bottom order
UIELEM_PM_PREFS_BUTTON,
UIELEM_PM_ABOUT_BUTTON,
UIELEM_PRIMARY_MENU, //buttons in primary menu should be defined in the entries directly preceding this, in top to bottom order
UIELEM_SEARCH_ENTRYBOX,
UIELEM_SEARCH_MENU,
UIELEM_MENU_BUTTON,
UIELEM_CHARTVIEW_BUTTON,
UIELEM_SEARCH_BUTTON,
UIELEM_NUCL_INFOBOX_ALLLEVELSBUTTON,
UIELEM_NUCL_INFOBOX_CLOSEBUTTON,
UIELEM_NUCL_INFOBOX,
UIELEM_NUCL_FULLINFOBOX_SCROLLBAR,
UIELEM_NUCL_FULLINFOBOX_BACKBUTTON,
UIELEM_NUCL_FULLINFOBOX,
UIELEM_ZOOMIN_BUTTON,
UIELEM_ZOOMOUT_BUTTON,
UIELEM_ENUM_LENGTH
}; //order in which UI elements are defined here determines 
//order in which they receive input, if they overlap 
//(eariler entries in the enum receive input first)
enum ui_state_enum{
UISTATE_CHARTONLY, //no menus open
UISTATE_CHARTWITHMENU, //primary menu, chart view menu, etc.
UISTATE_INFOBOX, //small nuclide info box
UISTATE_FULLLEVELINFO, //full nuclide level listing
UISTATE_FULLLEVELINFOWITHMENU,
UISTATE_MSG_BOX,
UISTATE_ABOUT_BOX,
UISTATE_PREFS_DIALOG,
UISTATE_ENUM_LENGTH
};
enum ui_icon_enum{
UIICON_MENU,
UIICON_SEARCH,
UIICON_SEARCHGRAY,
UIICON_CLOSE,
UIICON_DOWNARROWS,
UIICON_UPARROWS,
UIICON_CHECKBOX_OUTLINE,
UIICON_CHECKBOX_CHECK,
UIICON_CHARTVIEW,
UIICON_DROPDOWNARROW,
UIICON_ZOOMOUT,
UIICON_ZOOMIN,
UIICON_ENUM_LENGTH
};
enum ui_animation_enum{
UIANIM_CHART_FADEIN,
UIANIM_PRIMARY_MENU_SHOW,
UIANIM_PRIMARY_MENU_HIDE,
UIANIM_CHARTVIEW_MENU_SHOW,
UIANIM_CHARTVIEW_MENU_HIDE,
UIANIM_SEARCH_MENU_SHOW,
UIANIM_SEARCH_MENU_HIDE,
UIANIM_UISCALE_MENU_SHOW,
UIANIM_UISCALE_MENU_HIDE,
UIANIM_MODAL_BOX_SHOW, //applies to all modals, including: message box, about box, prefs dialog...
UIANIM_MODAL_BOX_HIDE,
UIANIM_NUCLHIGHLIGHT_SHOW,
UIANIM_NUCLHIGHLIGHT_HIDE,
UIANIM_NUCLINFOBOX_SHOW,
UIANIM_NUCLINFOBOX_HIDE,
UIANIM_NUCLINFOBOX_EXPAND,
UIANIM_NUCLINFOBOX_CONTRACT,
UIANIM_NUCLINFOBOX_TXTFADEIN,
UIANIM_NUCLINFOBOX_TXTFADEOUT,
UIANIM_ENUM_LENGTH
};
enum font_size_enum{
FONTSIZE_SMALL,
FONTSIZE_NORMAL,
FONTSIZE_LARGE,
FONTSIZE_ENUM_LENGTH
};
enum ui_scale_enum{
UISCALE_SMALL,
UISCALE_NORMAL,
UISCALE_LARGE,
UISCALE_HUGE,
UISCALE_ENUM_LENGTH
};
enum chart_view_enum{
CHARTVIEW_HALFLIFE, //colors on chart of nuclides correspond to half-lives
CHARTVIEW_DECAYMODE,
CHARTVIEW_2PLUS,
CHARTVIEW_R42,
CHARTVIEW_ENUM_LENGTH
};
enum value_unit_enum{
VALUE_UNIT_STABLE,
VALUE_UNIT_YEARS,
VALUE_UNIT_DAYS,
VALUE_UNIT_HOURS,
VALUE_UNIT_MINUTES,
VALUE_UNIT_SECONDS,
VALUE_UNIT_MILLISECONDS,
VALUE_UNIT_MICROSECONDS,
VALUE_UNIT_NANOSECONDS,
VALUE_UNIT_PICOSECONDS,
VALUE_UNIT_FEMTOSECONDS,
VALUE_UNIT_ATTOSECONDS,
VALUE_UNIT_EV,
VALUE_UNIT_KEV,
VALUE_UNIT_MEV,
VALUE_UNIT_NOVAL, //no measured value
VALUE_UNIT_PERCENT, //for abundances
VALUE_UNIT_ENUM_LENGTH
};
enum decay_mode_enum{
DECAYMODE_IT,
DECAYMODE_EC,
DECAYMODE_ECANDBETAPLUS,
DECAYMODE_BETAPLUS,
DECAYMODE_BETAPLUS_PROTON,
DECAYMODE_BETAPLUS_TWOPROTON,
DECAYMODE_BETAMINUS,
DECAYMODE_BETAMINUS_NEUTRON,
DECAYMODE_EC_PROTON,
DECAYMODE_ALPHA,
DECAYMODE_BETAMINUS_ALPHA,
DECAYMODE_PROTON,
DECAYMODE_TWOPROTON,
DECAYMODE_NEUTRON,
DECAYMODE_TWONEUTRON,
DECAYMODE_DEUTERON,
DECAYMODE_3HE,
DECAYMODE_SPONTANEOUSFISSION,
DECAYMODE_BETAMINUS_SPONTANEOUSFISSION,
DECAYMODE_2BETAMINUS,
DECAYMODE_2BETAPLUS,
DECAYMODE_2EC,
DECAYMODE_14C,
DECAYMODE_20NE,
DECAYMODE_25NE,
DECAYMODE_28MG,
DECAYMODE_34SI,
DECAYMODE_ENUM_LENGTH
};
enum tentative_sp_enum{
TENTATIVESP_NONE,
TENTATIVESP_SPINANDPARITY, //all quantites (eg. spin AND parity)
TENTATIVESP_SPINONLY, //first quantity only (eg. spin), or D for multipoles
TENTATIVESP_PARITYONLY, //second quantity only (eg. parity), or Q for multipoles
TENTATIVESP_NOSPIN,
TENTATIVESP_RANGE,
TENTATIVESP_ENUM_LENGTH
};
enum tentative_mult_enum{
TENTATIVEMULT_NONE,
TENTATIVEMULT_DERIVED, //square brackets
TENTATIVEMULT_YES, //round brackets
TENTATIVEMULT_ENUM_LENGTH
};
enum observation_flag_enum{
OBSFLAG_OBSERVED,
OBSFLAG_UNOBSERVED,
OBSFLAG_INFERRED,
OBSFLAG_TENTATIVE,
OBSFLAG_ENUM_LENGTH
};
enum value_type_enum{
VALUETYPE_NUMBER, //should always be the first (default) entry in the enum
VALUETYPE_GREATERTHAN,
VALUETYPE_GREATEROREQUALTHAN,
VALUETYPE_LESSTHAN,
VALUETYPE_LESSOREQUALTHAN,
VALUETYPE_APPROX,
VALUETYPE_ASYMERROR,
VALUETYPE_X, //unknown value, which other values are reported relative to
VALUETYPE_PLUSX, //for values relative to another value X
VALUETYPE_UNKNOWN,
VALUETYPE_ENUM_LENGTH
};
enum special_level_enum{
SPECIALLEVEL_NONE, //0 is treated as an invalid value
SPECIALLEVEL_HOYLE,
SPECIALLEVEL_NATURALLYOCCURINGISOMER,
SPECIALLEVEL_CLOCKISOMER,
SPECIALLEVEL_ENUM_LENGTH
};
enum kbd_mod_enum{
KBD_MOD_NONE,
KBD_MOD_CTRL,
KBD_MOD_SHIFT,
KBD_MOD_ENUM_LENGTH
};
enum thread_state_enum{
THREADSTATE_IDLE,
THREADSTATE_KILL,
THREADSTATE_ENUM_LENGTH
};
enum cli_opt_enum{
CLI_NOGAMEPAD,
CLI_ENUM_LENGTH
};
