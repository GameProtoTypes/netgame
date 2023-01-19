module;


#include "GE.ImGui.defines.h"
#include "GE.Basic.Macro.defines.h"
#include <iostream>



#define ALL_CORE_PARAMS \
GameState* gameState,\
GameStateActions* gameStateActions, \
float* peepVBOBuffer, \
float* particleVBOBuffer, \
ge_ubyte* mapTile1VBO, \
ge_uint* mapTile1AttrVBO, \
ge_uint* mapTile1OtherAttrVBO, \
ge_ubyte* mapTile2VBO, \
ge_uint* mapTile2AttrVBO, \
ge_uint* mapTile2OtherAttrVBO, \
float* guiVBO, \
float* linesVBO




#define ALL_CORE_PARAMS_PASS  \
((GameState*) gameState), \
((GameStateActions*) gameStateActions), \
peepVBOBuffer, \
particleVBOBuffer, \
mapTile1VBO, \
mapTile1AttrVBO, \
mapTile1OtherAttrVBO, \
mapTile2VBO, \
mapTile2AttrVBO, \
mapTile2OtherAttrVBO, \
guiVBO, \
linesVBO

module GE.ImGui;

import Game;
using namespace Game;
namespace GE {


void GUI_PushContainer(SyncedGui* gui, ge_int2 pos, ge_int2 size)
{
    gui->wOSidx++;
    if(gui->wOSidx >= SYNCGUI_MAX_DEPTH)
        printf("ERROR: GUI_PushContainer Overflow (SYNCGUI_MAX_DEPTH)");

    gui->widgetContainerGeomStack[gui->wOSidx].x = pos.x;
    gui->widgetContainerGeomStack[gui->wOSidx].y = pos.y;
    gui->widgetContainerGeomStack[gui->wOSidx].z = size.x;
    gui->widgetContainerGeomStack[gui->wOSidx].w = size.y;
}


void GUI_PopContianer(SyncedGui* gui)
{
    gui->wOSidx--;
    if(gui->wOSidx < -1)
        printf("ERROR: GUI PopOffset Call Missmatch.");
}

ge_int2 GUI_GetTotalOffset(SyncedGui* gui)
{
    ge_int2 sum = ge_int2{0,0};
    for(int i = 0; i <= gui->wOSidx; i++)
    {
        ge_int2 a = ge_int2(gui->widgetContainerGeomStack[i].x, gui->widgetContainerGeomStack[i].y);
        sum = GE2_ADD(sum, a);
    }
    return sum;
}
ge_int2 GUI_GetContainerSize(SyncedGui* gui)
{
    ge_int2 a = ge_int2(gui->widgetContainerGeomStack[gui->wOSidx].z, gui->widgetContainerGeomStack[gui->wOSidx].w);
    return a;
}


#define GUI_COMMON_WIDGET_START() \
    ge_int2 origPos = pos; \
    ge_int2 origSize = size; \
    bool goodStart = gui_CommonWidgetStart(ALL_CORE_PARAMS_PASS,  gui, &pos, &size, flags); \




ge_int4 GUI_MERGED_CLIP(SyncedGui* gui)
{
    int idx = gui->clipStackIdx;
    ge_int4 totalClip =  ge_int4{0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    ge_int2 totalClipEnd;
    totalClipEnd.x = totalClip.x + totalClip.z;
    totalClipEnd.y = totalClip.y + totalClip.w;

    while(idx >=0)
    {
        ge_int4 clip = gui->clipStack[idx];
        ge_int2 clipEnd;
        clipEnd.x = clip.x + clip.z;
        clipEnd.y = clip.y + clip.w;

        if(clip.x > totalClip.x)
            totalClip.x = clip.x;
        if(clip.y > totalClip.y)
            totalClip.y = clip.y;

        if(clipEnd.x < totalClipEnd.x)
        {
            totalClipEnd.x = clipEnd.x;
        }

        if(clipEnd.y < totalClipEnd.y)
        {
            totalClipEnd.y = clipEnd.y;
        }

        idx--;
    }

    totalClip.z = totalClipEnd.x - totalClip.x;
    totalClip.w = totalClipEnd.y - totalClip.y;

    return totalClip;
}

bool gui_CommonWidgetStart(ALL_CORE_PARAMS, SyncedGui* gui, ge_int2 * pos, ge_int2* size, GuiFlags flags)
{

    if(flags & GuiFlags_FillParent)
    {
        *pos = (ge_int2)(0,0);
        *size = GUI_GetContainerSize(gui);
    }

    *pos = GE2_ADD(*pos, GUI_GetTotalOffset(gui));

    //check clipping if inside parent

    if(gui->clipStackIdx > 0)
    {
        ge_int4 curClip = GUI_MERGED_CLIP(gui);
        if(pos->x > curClip.x + curClip.z)
            return false;
        
        if(pos->y > curClip.y + curClip.w)
            return false;

        if(pos->x + size->x < curClip.x)
            return false;
        
        if(pos->y + size->y < curClip.y)
            return false;
    }




    return true;
}


bool GUI_MOUSE_ON_GUI(SyncedGui* gui)
{
    return gui->mouseOnGUI || gui->mouseOnGUI_1;
}

//call before gui path
void GUI_RESET(ALL_CORE_PARAMS, SyncedGui* gui, ge_int2 mouseLoc, int mouseState, GuiStatePassType passType, bool isLocalClient)
{
    gui->passType = passType;
    gui->isLocalClient = isLocalClient;
    if(passType == GuiStatePassType_NoLogic)
    {
        //clear just drawn rectangles
        const int stride = 7;
        for(int idx = (gui->guiRenderRectIdx-1)*stride; idx >=0; idx--)
        {
            guiVBO[idx] = 0;
        }
    }
    else
    {

    }
    gui->guiRenderRectIdx = 0;
    gui->nextId = 0;



    gui->mouseFrameDelta = GE2_SUB(mouseLoc, gui->mouseLoc);

    gui->mouseLoc = mouseLoc;
    gui->mouseState = mouseState;


    gui->lastActiveWidget = gui->activeWidget;
    gui->hoverWidget = -1;
    gui->activeWidget = -1;

    gui->draggedOff = 0;
    gui->wOSidx = -1;
   
    gui->clipStackIdx = 0;
    gui->clipStack[0] = ge_int4{0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    
    gui->mouseDragging = 0;  
    if(BITGET(mouseState, MouseButtonBits_PrimaryPressed) || BITGET(mouseState, MouseButtonBits_SecondaryPressed))
    {
        gui->mouseLocBegin = mouseLoc;
        gui->mouseFrameDelta = (ge_int2)(0,0);
       // printf("pressedbegin "); Print_GE_INT2(mouseLoc);
        gui->mouseDragging = 1;
    }
    else if(BITGET(mouseState, MouseButtonBits_PrimaryReleased) || BITGET(mouseState, MouseButtonBits_SecondaryReleased))
    {
        gui->ignoreAll = 0;  
        //printf("pressedrelease "); Print_GE_INT2(mouseLoc);
        gui->mouseDragging = 1;
    }

    if(BITGET(mouseState, MouseButtonBits_PrimaryDown))
    {
        gui->mouseDragging = 1;

    }

}

//call after gui path
void GUI_RESET_POST(ALL_CORE_PARAMS, SyncedGui* gui)
{

    if(BITGET(gui->mouseState, MouseButtonBits_PrimaryPressed) || BITGET(gui->mouseState, MouseButtonBits_SecondaryPressed))
    {
        if(gui->mouseOnGUI == 0)
        {
            gui->ignoreAll = 1;
        }
        else
        {
            gui->dragOff = 1;
        }
    }
    else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) || BITGET(gui->mouseState, MouseButtonBits_SecondaryReleased))
    {
        if(gui->dragOff)
        {
            gui->dragOff = 0;
            gui->draggedOff = 1;
        }
    }

    gui->mouseOnGUI_1 = gui->mouseOnGUI;
    gui->mouseOnGUI = 0;
}




void GUI_INIT_STYLE(ALL_CORE_PARAMS)
{

    gameState->guiStyle.TEXT_COLOR = COLOR_WHITE;

    gameState->guiStyle.UV_WHITE = ge_float2(40.0/255, 20.0/255);

    gameState->guiStyle.BUTTON_COLOR_DEF        =  COLOR_GRAY;
    gameState->guiStyle.BUTTON_COLOR_HOVER      =  COLOR_LIGHTGRAY;
    gameState->guiStyle.BUTTON_COLOR_ACTIVE     =  COLOR_DARKGRAY;
    gameState->guiStyle.BUTTON_COLOR_TOGGLED    =  COLOR_GRAY2;

    gameState->guiStyle.SLIDER_COLOR_BACKGROUND = COLOR_DARKDARKGRAY;

    gameState->guiStyle.WINDOW_PADDING = 5;
    gameState->guiStyle.WINDOW_HEADER_SIZE = 30;
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.


    
}








SyncedGui* GetGuiState(ALL_CORE_PARAMS, int clientId)
{
    return &gameState->clientStates[clientId].gui;
}
int GrabGuiId(SyncedGui* gui)
{
    int id =  gui->nextId;
    gui->nextId++;
    return id;
}

void GUI_DrawRectangle(ALL_CORE_PARAMS, SyncedGui* gui, 
int x, 
int y,
int width,
int height,
ge_float3 color, 
ge_float2 UVStart, 
ge_float2 UVEnd,
bool bevelled)
{
    if(gui->passType == GuiStatePassType_Synced)
    {
       return;
    }

     
    ge_uint idx = gui->guiRenderRectIdx;

    ge_float2 UVSize = GE2_SUB(UVEnd, UVStart);

    //clip
    ge_int4 clip = GUI_MERGED_CLIP(gui);
    ge_int2 clipEnd;
    clipEnd.x = clip.x + clip.z;
    clipEnd.y = clip.y + clip.w;

    if(x < clip.x)
    {
        width = width - (clip.x - x);

        x = clip.x;
    }
    
    if(y < clip.y)
    {
        height = height - (clip.y - y);
        y = clip.y;
    }
    
    if(x + width > clipEnd.x)
    {

        width = clipEnd.x - x;
    }
    if(y + height > clipEnd.y)
    {
        height = clipEnd.y - y;
    }

    //early out if not rendered
    if(width <= 0)
        return;
    if(height <= 0)
        return;


    //map to [0,1]
    float xf = x/GUI_PXPERSCREEN_F;
    float yf = y/GUI_PXPERSCREEN_F;
    float widthf = width/GUI_PXPERSCREEN_F;
    float heightf = height/GUI_PXPERSCREEN_F;


    //map [0,1] to [-1,1]
    xf*= 2.0f;
    yf*= -2.0f;
    xf+=-1.0f;
    yf+=1.0f;

    widthf *= 2.0f;
    heightf*= 2.0f;


  
    const int stride = 7;
    float bevelx = 0.0f;
    float bevely = 0.0f;
    if(bevelled)
    {
        bevelx = GE_CLAMP(0.005f, 0.0f, widthf/2.0f);
        bevely = GE_CLAMP(0.005f, 0.0f, heightf/2.0f);
    }

    
    //main quad
    guiVBO[idx*stride + 0] = xf+widthf  - bevelx;//A
    guiVBO[idx*stride + 1] = yf-heightf + bevely;//A

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf + bevelx;//B
    guiVBO[idx*stride + 1] = yf - bevely;//B

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf + bevelx;//C
    guiVBO[idx*stride + 1] = yf-heightf+ bevely;//C

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf  - bevelx;//A
    guiVBO[idx*stride + 1] = yf-heightf + bevely;//A

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf - bevelx;//D
    guiVBO[idx*stride + 1] = yf - bevely;//D

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf + bevelx;
    guiVBO[idx*stride + 1] = yf - bevely;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    ge_float3 colorBottom = GE3_MUL(color , ge_float3(0.8,0.8,0.8));
    //Bottom Quad
    guiVBO[idx*stride + 0] = xf+widthf ;//E
    guiVBO[idx*stride + 1] = yf-heightf ;//E

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf + bevelx;//C
    guiVBO[idx*stride + 1] = yf-heightf+ bevely;//C

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf ;//F
    guiVBO[idx*stride + 1] = yf-heightf;//F

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf ;//E
    guiVBO[idx*stride + 1] = yf-heightf ;//E

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf+widthf  - bevelx;//A
    guiVBO[idx*stride + 1] =  yf-heightf + bevely;//A

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf + bevelx;//C
    guiVBO[idx*stride + 1] =  yf-heightf+ bevely;//C

    guiVBO[idx*stride + 2] = colorBottom.x;
    guiVBO[idx*stride + 3] = colorBottom.y;
    guiVBO[idx*stride + 4] = colorBottom.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    //rightQuad
    ge_float3 colorRight = GE3_MUL(color , ge_float3(0.8,0.8,0.8));
    guiVBO[idx*stride + 0] = xf+widthf; //G
    guiVBO[idx*stride + 1] = yf;//G

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf - bevelx;//D
    guiVBO[idx*stride + 1] = yf - bevely;//D

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf ;//E
    guiVBO[idx*stride + 1] = yf-heightf ;//E

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf - bevelx;//D
    guiVBO[idx*stride + 1] = yf - bevely;//D

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf+widthf  - bevelx;//A
    guiVBO[idx*stride + 1] =  yf-heightf + bevely;//A

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf+widthf ;//E
    guiVBO[idx*stride + 1] =  yf-heightf ;//E

    guiVBO[idx*stride + 2] = colorRight.x;
    guiVBO[idx*stride + 3] = colorRight.y;
    guiVBO[idx*stride + 4] = colorRight.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;


     //TopQuad
    ge_float3 colorTop = GE3_MUL(color , (ge_float3)(0.9,0.9,0.9));
    guiVBO[idx*stride + 0] = xf+widthf; //G
    guiVBO[idx*stride + 1] = yf;//G

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf ;//H
    guiVBO[idx*stride + 1] = yf ;//H

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf+widthf - bevelx;//D
    guiVBO[idx*stride + 1] =  yf - bevely;//D

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf ;//H
    guiVBO[idx*stride + 1] =  yf ;//H

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =   xf + bevelx;//B
    guiVBO[idx*stride + 1] =   yf - bevely;//B

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =   xf+widthf - bevelx;//D
    guiVBO[idx*stride + 1] =   yf - bevely;//D

    guiVBO[idx*stride + 2] = colorTop.x;
    guiVBO[idx*stride + 3] = colorTop.y;
    guiVBO[idx*stride + 4] = colorTop.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

     //LeftQuad
    ge_float3 colorLeft = GE3_MUL(color , (ge_float3)(0.9,0.9,0.9));
    guiVBO[idx*stride + 0] = xf + bevelx;//B
    guiVBO[idx*stride + 1] = yf - bevely;//B

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf ;//H
    guiVBO[idx*stride + 1] = yf ;//H

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =   xf ;//F
    guiVBO[idx*stride + 1] =   yf-heightf;//F

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf + bevelx;//C
    guiVBO[idx*stride + 1] =  yf-heightf+ bevely;//C

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] =   xf + bevelx;//B
    guiVBO[idx*stride + 1] =   yf - bevely;//B

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] =  xf ;//F
    guiVBO[idx*stride + 1] =  yf-heightf;//F

    guiVBO[idx*stride + 2] = colorLeft.x;
    guiVBO[idx*stride + 3] = colorLeft.y;
    guiVBO[idx*stride + 4] = colorLeft.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    

    gui->guiRenderRectIdx = idx;
      
}

ge_ubyte GUI_BoundsCheck(ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
{
    if((pos.x >= boundStart.x) && (pos.x < boundEnd.x) && (pos.y >= boundStart.y) && (pos.y < boundEnd.y) )
    {
        return 1;
    }
    return 0;
}
ge_ubyte GUI_InteractionBoundsCheck(SyncedGui* gui, ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
{
    ge_int4 currentClip = GUI_MERGED_CLIP(gui); 
    if(currentClip.x > boundStart.x)
    {
        boundStart.x = currentClip.x;
    }
    if(currentClip.y > boundStart.y)
    {
        boundStart.y = currentClip.y;
    }


    ge_int2 clipEnd;
    clipEnd.x = currentClip.x + currentClip.z;
    clipEnd.y = currentClip.y + currentClip.w;

    if(clipEnd.x < boundEnd.x)
        boundEnd.x = clipEnd.x;

    if(clipEnd.y < boundEnd.y)
        boundEnd.y = clipEnd.y;

    return GUI_BoundsCheck(boundStart, boundEnd, pos);
}






void GUI_PushClip(SyncedGui* gui, ge_int2 startPos, ge_int2 size)
{
    startPos = GE2_ADD(startPos, GUI_GetTotalOffset(gui));

    gui->clipStackIdx++;

    gui->clipStack[gui->clipStackIdx].x = startPos.x;
    gui->clipStack[gui->clipStackIdx].y = startPos.y;
    gui->clipStack[gui->clipStackIdx].z = size.x;
    gui->clipStack[gui->clipStackIdx].w = size.y;
}

void GUI_PopClip(SyncedGui* gui)
{
    gui->clipStackIdx--;
    if(gui->clipStackIdx == 0)
    {
        gui->clipStackIdx = 0;
        gui->clipStack[gui->clipStackIdx] = ge_int4{0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    }
    else if(gui->clipStackIdx < 0)
    {
        printf("ERROR: GUI_PopClip Missmatch\n");
        gui->clipStackIdx=0;
    }
}




ge_float4 ascii_to_uv(char ch)
{
    int x = ((int)ch)*16;
    int y = 16*(x / 256);
    x = x % 256;


    return ge_float4(x/255.0,y/255.0, (x+ 16)/255.0, (y+16)/255.0);
}

#define GUI_TEXT_CODE \
 GUI_COMMON_WIDGET_START()\
\
    const float charWidth = 10;\
    const float charHeight = 16;\
\
    int i = 0;\
    float posx = pos.x;\
    float posy = pos.y;\
    float offsetx = 0;\
    while(str[i] != '\0')\
    {\
        ge_float2 uvstart;\
        ge_float2 uvend;\
\
        ge_float4 uv = ascii_to_uv(str[i]);\
        uvstart.x = uv.x;\
        uvstart.y = uv.y;\
        uvend.x = uv.z;\
        uvend.y = uv.w;\
\
\
        if(str[i] == '\n')\
        {\
            posy += charHeight;\
            offsetx = 0;\
            i++;\
            continue;\
        }\
        GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, posx + offsetx, posy, charWidth, charHeight, gameState->guiStyle.TEXT_COLOR, uvstart, uvend  , false );\
        i++;\
        offsetx+=charWidth;\
    }

void GUI_TEXT(GUIID_DEF_ALL, char* str)
{
    GUI_COMMON_WIDGET_START()

    const float charWidth = 10;
    const float charHeight = 16;

    int i = 0;
    float posx = pos.x;
    float posy = pos.y;
    float offsetx = 0;
    while(str[i] != '\0')
    {
        ge_float2 uvstart;
        ge_float2 uvend;

        ge_float4 uv = ascii_to_uv(str[i]);
        uvstart.x = uv.x;
        uvstart.y = uv.y;
        uvend.x = uv.z;
        uvend.y = uv.w;


        if(str[i] == '\n')
        {
            posy += charHeight;
            offsetx = 0;
            i++;
            continue;
        }
        GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, posx + offsetx, posy, charWidth, charHeight, gameState->guiStyle.TEXT_COLOR, uvstart, uvend  , false );\
        i++;
        offsetx+=charWidth;
    }
}



void GUI_LABEL(GUIID_DEF_ALL, char* str, ge_float3 color)
{

    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return;



    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE  , false);
    
    if(str != NULL)
    {
        GUI_PushClip(gui, origPos, origSize);
            GUI_TEXT(GUIID_PASS,  origPos, origSize, GuiFlags(0), str);
        GUI_PopClip(gui);
    }
}

void GUI_IMAGE(GUIID_DEF_ALL, ge_float2 uvStart, ge_float2 uvEnd, ge_float3 color)
{
 
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
    {   
        return;
    }
     GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, GE2_ADD(uvStart , ge_float2(1.0,0.0)), GE2_ADD(uvEnd , ge_float2(1.0,0.0)) , false );

}

bool GUI_BUTTON(GUIID_DEF_ALL, ge_float3 color, char* str, int* down,  bool* toggleState)
{

    GUI_COMMON_WIDGET_START()


    if(!goodStart)
    {
        if(toggleState != NULL)
        {
            return *toggleState;
        }
        else
        {
            return 0;
        }
    }
        
    


    bool ret = false;
    if(down != NULL)
        *down = 0;
    if((GUI_InteractionBoundsCheck(gui, pos, GE2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id)))
    {
        gui->hoverWidget = id;

        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown))
        {
            gui->activeWidget = id;
            if(down != NULL)
                *down = 1;
        }
        
        else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased))
        {
            ret = 1;
            if(toggleState != NULL)
            {
                if(*toggleState)
                    (*toggleState) = false;
                else
                {

                    (*toggleState) = true;

                }
            }
        }
        gui->mouseOnGUI = 1;
    }


    if(gui->hoverWidget == id)
        color =  GE3_MUL(color,gameState->guiStyle.BUTTON_COLOR_HOVER);

    if(gui->activeWidget == id){
        color =  GE3_MUL(color,gameState->guiStyle.BUTTON_COLOR_ACTIVE);
    }

    if((toggleState != NULL) && (*toggleState))
    {

        color =  GE3_MUL(color,gameState->guiStyle.BUTTON_COLOR_TOGGLED);
    }


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE , (flags & GuiFlags_Beveled) );
    
    if(str != NULL)
    {
        GUI_PushClip(gui, origPos, origSize);
            GUI_TEXT(GUIID_PASS,  origPos, origSize, GuiFlags(0), str);
        GUI_PopClip(gui);
    }

    if(toggleState != NULL)
    {
        ret = (*toggleState);
    }

    return ret;
}




ge_ubyte GUI_SLIDER_INT_HORIZONTAL(GUIID_DEF_ALL, int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return 0;

    ge_int2 posHandle;
    ge_int2 sizeHandle;

    sizeHandle.y = size.y;
    sizeHandle.x = size.x/10;

    if((GUI_InteractionBoundsCheck(gui, pos, GE2_ADD(pos, size), gui->mouseLoc)&&(gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id+1)))
    {
        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {
            float perc = ((float)(gui->mouseLoc.x - pos.x))/size.x;
            int d = perc*(max-min);
            (*value) = GE_CLAMP(min + d, min, max-1);
        }
        gui->mouseOnGUI = 1;
    }


    posHandle.y = origPos.y;
    posHandle.x = origPos.x + (((size.x-sizeHandle.x)*((*value - min)))/(max-1-min));

    posHandle.x = GE_CLAMP(posHandle.x, origPos.x, origPos.x + size.x - sizeHandle.x);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE , false );
    

    int down;
    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, GuiFlags(0), GUI_BUTTON_COLOR_DEF, NULL, &down, NULL);

    return 0;
}

ge_ubyte GUI_SLIDER_INT_VERTICAL(GUIID_DEF_ALL, int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return 0;

    ge_int2 posHandle;
    ge_int2 sizeHandle;

    sizeHandle.y = size.y/10;
    sizeHandle.x = size.x;
    ge_ubyte ret = 0;
    if((GUI_InteractionBoundsCheck(gui, pos, GE2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id+1)))
    {
        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {
            float perc = ((float)(gui->mouseLoc.y  - pos.y))/(size.y );
            int d = perc*(max-min);
            (*value) = GE_CLAMP(min + d, min, max-1);
            ret = 1;
        }
        gui->mouseOnGUI = 1;
    }


    posHandle.x = origPos.x;

    posHandle.y = origPos.y + (((size.y-sizeHandle.y)*((*value - min)))/(max-1-min));

    posHandle.y = GE_CLAMP(posHandle.y, origPos.y, origPos.y + size.y - sizeHandle.y);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE, false );


    int down;

    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, GuiFlags(0), GUI_BUTTON_COLOR_DEF, NULL, &down, NULL);

    return ret;
}


bool GUI_SCROLLBOX_BEGIN(GUIID_DEF_ALL, ge_int2 scrollSpace, int* scrollx, int* scrolly)
{
    GUI_COMMON_WIDGET_START();
    if(!goodStart)
        return false;


    ge_int2 scrollOffset;
    scrollOffset.x = 0;
    scrollOffset.y = 0;

    ge_int2 vertSliderPos;
    vertSliderPos.y = origPos.y;
    vertSliderPos.x = origPos.x + size.x - 20;

    ge_int2 vertSliderSize;
    vertSliderSize.x = 20;
    vertSliderSize.y = size.y-20;

    ge_int2 horSliderPos;
    horSliderPos.y = origPos.y + size.y - 20;
    horSliderPos.x = origPos.x;

    ge_int2 horSliderSize;
    horSliderSize.x = size.x-20;
    horSliderSize.y = 20;


    ge_int2 canvasSize;
    canvasSize.x = size.x - 20;
    canvasSize.y = size.y - 20;

    int a = scrollSpace.x - canvasSize.x;
    int b = scrollSpace.y - canvasSize.y;

    if(b>0)
    {

        GUI_SLIDER_INT_VERTICAL(GUIID_PASS, vertSliderPos, vertSliderSize, GuiFlags(0), scrolly, 0, b);
        scrollOffset.y = -*scrolly;
    }
    if(a>0)
    {
        
        GUI_SLIDER_INT_HORIZONTAL(GUIID_PASS, horSliderPos, horSliderSize, GuiFlags(0), scrollx, 0, a);
        scrollOffset.x= -*scrollx;
    }

    GUI_PushClip(gui, origPos, canvasSize);

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    canvasSize.x, canvasSize.y, (ge_float3)(0.2,0.2,0.2), gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE , false );

    GUI_PushContainer(gui, GE2_ADD(origPos , scrollOffset), origSize);

    return true;
}

void GUI_SCROLLBOX_END(GUIID_DEF)
{
   GUI_PopContianer(gui);
   GUI_PopClip(gui);

}
bool GUI_BEGIN_CONTEXT_MENU(GUIID_DEF_ALL)
{
    GUI_COMMON_WIDGET_START();

    GUI_PushClip(gui, pos, size);
    if(!goodStart)
    {
        GUI_PopClip(gui);
        return false;
    }

    GUI_PushContainer(gui, GE2_ADD(origPos,ge_int2(gameState->guiStyle.WINDOW_PADDING,gameState->guiStyle.WINDOW_HEADER_SIZE)), 
    GE2_SUB(origSize,ge_int2(gameState->guiStyle.WINDOW_PADDING*2,gameState->guiStyle.WINDOW_PADDING + gameState->guiStyle.WINDOW_HEADER_SIZE)));
    
    return true;
}

void GUI_END_CONTEXT_MENU(GUIID_DEF)
{
    GUI_PopClip(gui);

    GUI_PopContianer(gui);
}




bool GUI_BEGIN_DROP_LIST_SELECT(GUIID_DEF_ALL, 
    char* list,
    int* strLocations, 
    int listSize,
    
    bool* dropOpen,
    int* curSelection
)
{
    GUI_COMMON_WIDGET_START();

    return true;
}

void GUI_END_DROP_LIST_SELECT(GUIID_DEF)
{

}



bool GUI_BEGIN_WINDOW(GUIID_DEF, 
ge_int2* windowPos,
ge_int2* windowSize, 
GuiFlags flags, char* str )
{
    ge_int2 pos = *windowPos;
    ge_int2 size = *windowSize;


    GUI_COMMON_WIDGET_START();

    GUI_PushClip(gui, pos, size);


    if(!goodStart)
    {
        GUI_PopClip(gui);
        return false;
    }

    ge_float3 headerColor = (ge_float3)(0.4,0.4,0.4);


    if((GUI_InteractionBoundsCheck(gui, pos, GE2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id)))
    {
        gui->hoverWidget = id;
        gui->mouseOnGUI = 1;


        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {

            if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown))
                gui->activeWidget = id;
          
           
           if(gui->passType == GuiStatePassType_NoLogic)
           {
                printf("noNetframe delta(down): ");GE2_PRINT(gui->mouseFrameDelta);
                printf("noNet mousepos: "); GE2_PRINT(gui->mouseLoc);
                (*windowPos) = GE2_ADD((*windowPos), gui->mouseFrameDelta);
                printf("noNetWindowPos: "); GE2_PRINT((*windowPos));
           }
        }
        


        if(gui->lastActiveWidget == id && BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased))
        {
           // headerColor = (ge_float3)(0.5,0.5,0.5);

            //printf("window, guipasstype: %d, is local client: %d", gui->passType, gui->isLocalClient);
           
            if( gui->passType == GuiStatePassType_Synced)
            {
                printf("Netframe delta(rel): ");GE2_PRINT(gui->mouseFrameDelta);
                (*windowPos) =GE2_ADD((*windowPos), GE2_SUB(gui->mouseLoc , gui->mouseLocBegin));
                printf("NetWindowPos: "); GE2_PRINT((*windowPos));
                printf("NetDelta: "); GE2_PRINT((GE2_SUB(gui->mouseLoc , gui->mouseLocBegin)));
            }

            //resolve screen edges
            ge_int2 bottomRight = GE2_ADD((*windowPos) , (*windowSize));
            if(bottomRight.x > GUI_PXPERSCREEN)
            {
                windowPos->x -=  bottomRight.x - GUI_PXPERSCREEN;
            }
            if(bottomRight.y > GUI_PXPERSCREEN)
            {
                windowPos->y -=  bottomRight.y - GUI_PXPERSCREEN;
            }

            if(windowPos->x < 0)
                windowPos->x = 0;
            if(windowPos->y < 0)
                windowPos->y = 0;    
        }

    }









    GUI_LABEL(GUIID_PASS, pos,
    (ge_int2)(size.x, gameState->guiStyle.WINDOW_HEADER_SIZE), GuiFlags(0), str,  headerColor);
    
    //GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    //size.x, gameState->guiStyle.WINDOW_HEADER_SIZE, headerColor, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y+gameState->guiStyle.WINDOW_HEADER_SIZE, 
    size.x, size.y-gameState->guiStyle.WINDOW_HEADER_SIZE, (ge_float3)(0.6,0.6,0.6), gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE , false);

    GUI_PushContainer(gui, GE2_ADD(origPos,(ge_int2)(gameState->guiStyle.WINDOW_PADDING,gameState->guiStyle.WINDOW_HEADER_SIZE)), 
    GE2_SUB(origSize,(ge_int2)(gameState->guiStyle.WINDOW_PADDING*2,gameState->guiStyle.WINDOW_PADDING + gameState->guiStyle.WINDOW_HEADER_SIZE)));

    return true;
}

void GUI_END_WINDOW(GUIID_DEF)
{
    GUI_PopClip(gui);

    GUI_PopContianer(gui);
}

void GUI_UpdateToggleGroup(bool* bank, int bankSize, int idx)
{
    for(int i = 0; i < bankSize; i++)
    {
        if(i == idx)
            continue;

        bank[i] = false;
    }

}




}