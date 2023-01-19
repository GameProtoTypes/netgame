#pragma once


#define SYNCGUI_MAX_WIDGETS (4096)
#define SYNCGUI_MAX_DEPTH (8)




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


enum GuiFlags
{
	GuiFlags_FillParent = 1,
	GuiFlags_Mouse_Draggable = 1 << 1,
	GuiFlags_Beveled  = 1 << 2
} typedef GuiFlags;

enum GuiStatePassType
{
	GuiStatePassType_NoLogic,
	GuiStatePassType_Synced
} typedef GuiStatePassType;


struct GuiStyle
{
	ge_float2 UV_WHITE;

	ge_float3 TEXT_COLOR;

	ge_float3 BUTTON_COLOR_DEF;
	ge_float3 BUTTON_COLOR_HOVER;
	ge_float3 BUTTON_COLOR_ACTIVE;
	ge_float3 BUTTON_COLOR_TOGGLED;

	ge_float3 SLIDER_COLOR_BACKGROUND;



	ge_int WINDOW_PADDING;
	ge_int WINDOW_HEADER_SIZE;

} typedef GuiStyle;


struct GuiStateInt
{
	ge_int id;
	union
	{
		ge_int Vi;
		ge_int2 Vi2;
	} value;
} typedef GuiStateInt;

#define NUM_EDITOR_MENU_TABS (6)

#define GUI_MAX_WINDOWS (10)
struct GuiState
{
	
	bool menuToggles[NUM_EDITOR_MENU_TABS];
	int menuScrollx;
	int menuScrolly;

	ge_int2 windowPositions[GUI_MAX_WINDOWS];
	ge_int2 windowSizes[GUI_MAX_WINDOWS];

    int scrollBoxes_x[GUI_MAX_WINDOWS];
    int scrollBoxes_y[GUI_MAX_WINDOWS];
} typedef GuiState;





struct SyncedGui
{
	ge_int guiRenderRectIdx;
	ge_int nextId;

	ge_int2 mouseLocBegin;
	ge_int2 mouseLoc;
	ge_int2 mouseFrameDelta;
	ge_int mouseState;

	ge_int fakeDummyInt;
	bool fakeDummyBool;

	ge_ubyte ignoreAll;//1 when mouse button pushed off-gui and then goes on gui while button(s) held
	ge_ubyte dragOff;//1 when gui held and then mouse dragged off.
	ge_ubyte draggedOff;//1 when gui held and then mouse dragged off and released.

	ge_ubyte mouseDragging;

	ge_int lastActiveWidget;
	ge_int hoverWidget;
	ge_int activeWidget;

	bool mouseOnGUI;
	bool mouseOnGUI_1;

	ge_int4 widgetContainerGeomStack[SYNCGUI_MAX_DEPTH];
	ge_int wOSidx;

	ge_int4 clipStack[SYNCGUI_MAX_DEPTH];
	ge_int clipStackIdx;



	ge_int2 curBoundStart;
	ge_int2 curBoundEnd;

	GuiStatePassType passType;
	bool isLocalClient;


	GuiState guiState;

} typedef SyncedGui;

