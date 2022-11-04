#pragma once


#define SYNCGUI_MAX_WIDGETS (4096)
#define SYNCGUI_MAX_DEPTH (8)




#define COLOR_DARKDARKGRAY (float3)(0.2, 0.2, 0.2)
#define COLOR_DARKGRAY (float3)(0.3, 0.3, 0.3)
#define COLOR_GRAY2 (float3)(0.4,0.4,0.4)
#define COLOR_GRAY (float3)(0.5,0.5,0.5)
#define COLOR_LIGHTGRAY (float3)(0.7, 0.7, 0.7)
#define COLOR_WHITE (float3)(1.0,1.0,1.0)



#define COLOR_RED (float3)(1.0,0.0,0.0)
#define COLOR_GREEN (float3)(0.0,1.0,0.0)
#define COLOR_BLUE (float3)(0.0,0.0,1.0)
#define COLOR_ORANGE (float3)(0.95,0.38,0.0)


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
	float2 UV_WHITE;

	float3 TEXT_COLOR;

	float3 BUTTON_COLOR_DEF;
	float3 BUTTON_COLOR_HOVER;
	float3 BUTTON_COLOR_ACTIVE;
	float3 BUTTON_COLOR_TOGGLED;

	float3 SLIDER_COLOR_BACKGROUND;



	int WINDOW_PADDING;
	int WINDOW_HEADER_SIZE;

} typedef GuiStyle;


struct GuiStateInt
{
	int id;
	union
	{
		int Vi;
		ge_int2 Vi2;
	} value;
} typedef GuiStateInt;

struct SyncedGui
{
	int guiRenderRectIdx;
	int nextId;

	ge_int2 mouseLocBegin;
	ge_int2 mouseLoc;
	ge_int2 mouseFrameDelta;
	int mouseState;

	int fakeDummyInt;

	cl_uchar ignoreAll;//1 when mouse button pushed off-gui and then goes on gui while button(s) held
	cl_uchar dragOff;//1 when gui held and then mouse dragged off.
	cl_uchar draggedOff;//1 when gui held and then mouse dragged off and released.

	cl_uchar mouseDragging;

	int lastActiveWidget;
	int hoverWidget;
	int activeWidget;

	bool mouseOnGUI;
	bool mouseOnGUI_1;

	ge_int4 widgetContainerGeomStack[SYNCGUI_MAX_DEPTH];
	int wOSidx;

	ge_int4 clipStack[SYNCGUI_MAX_DEPTH];
	int clipStackIdx;

	ge_int2 curBoundStart;
	ge_int2 curBoundEnd;

	GuiStatePassType passType;
	bool isLocalClient;


	GuiState guiState;

} typedef SyncedGui;

