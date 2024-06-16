/* © J. Williams 2017-2024 */
/* As the filename says, these are enums used throughout the engine */

enum input_type_enum{
INPUT_TYPE_KEYBOARD, INPUT_TYPE_GAMEPAD, INPUT_TYPE_MOUSE,
INPUT_TYPE_ENUM_LENGTH
};
enum input_enum {
INPUT_DOWN, INPUT_UP, INPUT_LEFT, INPUT_RIGHT, //directional inputs
INPUT_ZOOM, INPUT_ENUM_LENGTH
};
enum loc_string_enum{LOCSTR_APPLY,LOCSTR_CANCEL,LOCSTR_OK,LOCSTR_NODB,LOCSTR_ENUM_LENGTH
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
HIGHLIGHT_ENUM_LENGTH
};
enum ui_element_enum{
UIELEM_MENU_BUTTON,
UIELEM_PRIMARY_MENU,
UIELEM_MSG_BOX, //used to show warnings or errors
UIELEM_MSG_BOX_OK_BUTTON,
UIELEM_NUCL_INFOBOX,
UIELEM_ENUM_LENGTH
};
enum ui_state_enum{
UISTATE_DEFAULT, //no menus open
UISTATE_UNINTERACTABLE,
UISTATE_MSG_BOX,
UISTATE_ENUM_LENGTH
};
enum ui_icon_enum{
UIICON_MENU,
UIICON_SPECTRUM,
UIICON_CLOSE,
UIICON_ENUM_LENGTH
};
enum ui_animation_enum{
UIANIM_CHART_FADEIN,
UIANIM_MSG_BOX_SHOW,
UIANIM_MSG_BOX_HIDE,
UIANIM_NUCLHIGHLIGHT_SHOW,
UIANIM_NUCLHIGHLIGHT_HIDE,
UIANIM_NUCLINFOBOX_SHOW,
UIANIM_NUCLINFOBOX_HIDE,
UIANIM_ENUM_LENGTH
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
DECAYMODE_PROTON,
DECAYMODE_TWOPROTON,
DECAYMODE_NEUTRON,
DECAYMODE_TWONEUTRON,
DECAYMODE_DEUTERON,
DECAYMODE_3HE,
DECAYMODE_SPONTANEOUSFISSION,
DECAYMODE_ENUM_LENGTH
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
VALUETYPE_UNKNOWN,
VALUETYPE_ENUM_LENGTH
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
