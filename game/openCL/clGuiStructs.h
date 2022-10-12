#pragma once


#define SYNCGUI_MAX_WIDGETS (512)
#define SYNCGUI_MAX_DEPTH (8)



enum GuiFlags
{
	GuiFlags_FillParent = 1,
	GuiFlags_Mouse_Draggable = 1 << 1
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

	float3 BUTTON_COLOR;
	float3 BUTTON_COLOR_HOVER;
	float3 BUTTON_COLOR_ACTIVE;
	float3 BUTTON_COLOR_TOGGLED;

	float3 SLIDER_COLOR_BACKGROUND;

}typedef GuiStyle;




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

	cl_uchar mouseOnGUI;

	ge_int2 widgetOffsetStack[SYNCGUI_MAX_DEPTH];
	int wOSidx;

	ge_int4 clipStack[SYNCGUI_MAX_DEPTH];
	int clipStackIdx;

	ge_int2 curBoundStart;
	ge_int2 curBoundEnd;

	GuiStatePassType passType;

	int stateInts[SYNCGUI_MAX_WIDGETS];
	int nextStateIntIdx;

    bool toggleExclusionGroupActive;
	offsetPtr toggleExclusionGroupStateInts[SYNCGUI_MAX_DEPTH];
	int toggleExlusionGroupIdx;
	offsetPtr lastToggleStateIntPtr;
} typedef SyncedGui;

