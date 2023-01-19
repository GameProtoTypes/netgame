#define GUIID_PASS ALL_CORE_PARAMS_PASS, gui, GrabGuiId(gui)
#define GUIID_DEF ALL_CORE_PARAMS, SyncedGui* gui, int id
#define GUIID_DEF_ALL GUIID_DEF, ge_int2 pos, ge_int2 size, GuiFlags flags

#define GUI_FAKESWITCH_PARAM_INT(PARAM) GuiFakeSwitch_Param_Int(gui, PARAM)
#define GUI_AUTO_SIZE (ge_int2){-1,-1}
#define GUI_BUTTON_COLOR_DEF (gameState->guiStyle.BUTTON_COLOR_TOGGLED)

#define SYNCGUI_MAX_DEPTH (16)



#define COLOR_DARKDARKGRAY (ge_float3)(0.2, 0.2, 0.2)
#define COLOR_DARKGRAY (ge_float3)(0.3, 0.3, 0.3)
#define COLOR_GRAY2 (ge_float3)(0.4,0.4,0.4)
#define COLOR_GRAY (ge_float3)(0.5,0.5,0.5)
#define COLOR_LIGHTGRAY (ge_float3)(0.7, 0.7, 0.7)
#define COLOR_WHITE (ge_float3)(1.0,1.0,1.0)



#define COLOR_RED (ge_float3)(1.0,0.0,0.0)
#define COLOR_GREEN (ge_float3)(0.0,1.0,0.0)
#define COLOR_BLUE (ge_float3)(0.0,0.0,1.0)
#define COLOR_ORANGE (ge_float3)(0.95,0.38,0.0)
