#pragma once




#include "clGuiStructs.h"







#define GUIID_PASS ALL_CORE_PARAMS_PASS, gui, GrabGuiId(gui)


#define GUIID_DEF ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, int id
#define GUIID_DEF_ALL GUIID_DEF, ge_int2 pos, ge_int2 size, GuiFlags flags



#define GUI_FAKESWITCH_PARAM_INT(PARAM) GuiFakeSwitch_Param_Int(gui, PARAM)
#define GUI_AUTO_SIZE (ge_int2){-1,-1}


void GUI_PushContainer(PARAM_GLOBAL_POINTER SyncedGui* gui, ge_int2 pos, ge_int2 size)
{
    gui->wOSidx++;
    if(gui->wOSidx >= SYNCGUI_MAX_DEPTH)
        printf("ERROR: GUI_PushContainer Overflow (SYNCGUI_MAX_DEPTH)");

    gui->widgetContainerGeomStack[gui->wOSidx].x = pos.x;
    gui->widgetContainerGeomStack[gui->wOSidx].y = pos.y;
    gui->widgetContainerGeomStack[gui->wOSidx].z = size.x;
    gui->widgetContainerGeomStack[gui->wOSidx].w = size.y;
}


void GUI_PopContianer(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    gui->wOSidx--;
    if(gui->wOSidx < -1)
        printf("ERROR: GUI PopOffset Call Missmatch.");
}

ge_int2 GUI_GetTotalOffset(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    ge_int2 sum = (ge_int2){0,0};
    for(int i = 0; i <= gui->wOSidx; i++)
    {
        ge_int2 a = (ge_int2)(gui->widgetContainerGeomStack[i].x, gui->widgetContainerGeomStack[i].y);
        sum = INT2_ADD(sum, a);
    }
    return sum;
}
ge_int2 GUI_GetContainerSize(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    ge_int2 a = (ge_int2)(gui->widgetContainerGeomStack[gui->wOSidx].z, gui->widgetContainerGeomStack[gui->wOSidx].w);
    return a;
}


#define GUI_COMMON_WIDGET_START() \
    ge_int2 origPos = pos; \
    ge_int2 origSize = size; \
    bool goodStart = gui_CommonWidgetStart(ALL_CORE_PARAMS_PASS,  gui, &pos, &size, flags); \




ge_int4 GUI_MERGED_CLIP(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    int idx = gui->clipStackIdx;
    ge_int4 totalClip =  (ge_int4){0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
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

bool gui_CommonWidgetStart(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, ge_int2 * pos, ge_int2* size, GuiFlags flags)
{

    if(flags & GuiFlags_FillParent)
    {
        *pos = (ge_int2)(0,0);
        *size = GUI_GetContainerSize(gui);
    }

    *pos = INT2_ADD(*pos, GUI_GetTotalOffset(gui));

    ge_int4 curClip = GUI_MERGED_CLIP(gui);

    if(pos->x > curClip.x + curClip.z)
        return false;
    
    if(pos->y > curClip.y + curClip.w)
        return false;

    if(pos->x + size->x < curClip.x)
        return false;
    
    if(pos->y + size->y < curClip.y)
        return false;
    
    return true;
}


cl_uchar GUI_MOUSE_ON_GUI(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    return gui->mouseOnGUI;
}

//call before gui path
void GUI_RESET(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, ge_int2 mouseLoc, int mouseState, GuiStatePassType passType)
{
    gui->passType = passType;
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

    gui->mouseFrameDelta = INT2_SUB(mouseLoc, gui->mouseLoc);

    gui->mouseLoc = mouseLoc;
    gui->mouseState = mouseState;


    gui->lastActiveWidget = gui->activeWidget;
    gui->hoverWidget = -1;
    gui->activeWidget = -1;

    gui->draggedOff = 0;
    gui->wOSidx = -1;
   
    gui->clipStackIdx = 0;
    gui->clipStack[0] = (ge_int4){0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    
    if(BITGET(mouseState, MouseButtonBits_PrimaryPressed) || BITGET(mouseState, MouseButtonBits_SecondaryPressed))
    {
        gui->mouseLocBegin = mouseLoc;
        gui->mouseDragging = 1;

    }
    else if(BITGET(mouseState, MouseButtonBits_PrimaryReleased) || BITGET(mouseState, MouseButtonBits_SecondaryReleased))
    {
        gui->ignoreAll = 0;  
        gui->mouseDragging = 0;  
    }

}

//call after gui path
void GUI_RESET_POST(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui)
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


    gui->mouseOnGUI = 0;
}




void GUI_INIT_STYLE(ALL_CORE_PARAMS)
{

    gameState->guiStyle.TEXT_COLOR = (float3)(1.0,1.0,1.0);

    gameState->guiStyle.UV_WHITE = (float2)(40.0/255, 20.0/255);

    gameState->guiStyle.BUTTON_COLOR            = (float3)(0.5, 0.5, 0.5);
    gameState->guiStyle.BUTTON_COLOR_HOVER      = (float3)(0.7, 0.7, 0.7);
    gameState->guiStyle.BUTTON_COLOR_ACTIVE     = (float3)(0.3, 0.3, 0.3);
    gameState->guiStyle.BUTTON_COLOR_TOGGLED    = (float3)(0.4, 0.4, 0.4);

    gameState->guiStyle.SLIDER_COLOR_BACKGROUND = (float3)(0.2, 0.2, 0.2);

    gameState->guiStyle.WINDOW_PADDING = 5;
    gameState->guiStyle.WINDOW_HEADER_SIZE = 20;
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.
//  gameState->guiStyle.


    
}








global SyncedGui* GetGuiState(ALL_CORE_PARAMS, int clientId)
{
    return &gameState->clientStates[clientId].gui;
}
int GrabGuiId(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    int id =  gui->nextId;;
    gui->nextId++;
    return id;
}


void GUI_DrawRectangle(ALL_CORE_PARAMS, PARAM_GLOBAL_POINTER SyncedGui* gui, int x, int y, int width, int height, float3 color, float2 UVStart, float2 UVEnd)
{
    if(gui->passType == GuiStatePassType_Synced)
    {
        return;
    }


    uint idx = gui->guiRenderRectIdx;

    float2 UVSize = UVEnd - UVStart;

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
    heightf*= -2.0f;



    const int stride = 7;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf+heightf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVEnd.y;
    idx++;

    guiVBO[idx*stride + 0] = xf+widthf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVEnd.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    guiVBO[idx*stride + 0] = xf;
    guiVBO[idx*stride + 1] = yf;

    guiVBO[idx*stride + 2] = color.x;
    guiVBO[idx*stride + 3] = color.y;
    guiVBO[idx*stride + 4] = color.z;
    guiVBO[idx*stride + 5] = UVStart.x;
    guiVBO[idx*stride + 6] = UVStart.y;
    idx++;

    gui->guiRenderRectIdx = idx;
}

cl_uchar GUI_BoundsCheck(ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
{
    if((pos.x >= boundStart.x) && (pos.x < boundEnd.x) && (pos.y >= boundStart.y) && (pos.y < boundEnd.y) )
    {
        return 1;
    }
    return 0;
}
cl_uchar GUI_InteractionBoundsCheck(PARAM_GLOBAL_POINTER SyncedGui* gui, ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
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






void GUI_PushClip(PARAM_GLOBAL_POINTER SyncedGui* gui, ge_int2 startPos, ge_int2 size)
{
    startPos = INT2_ADD(startPos, GUI_GetTotalOffset(gui));

    gui->clipStackIdx++;

    gui->clipStack[gui->clipStackIdx].x = startPos.x;
    gui->clipStack[gui->clipStackIdx].y = startPos.y;
    gui->clipStack[gui->clipStackIdx].z = size.x;
    gui->clipStack[gui->clipStackIdx].w = size.y;
}

void GUI_PopClip(PARAM_GLOBAL_POINTER SyncedGui* gui)
{
    gui->clipStackIdx--;
    if(gui->clipStackIdx == 0)
    {
        gui->clipStackIdx = 0;
        gui->clipStack[gui->clipStackIdx] = (ge_int4){0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    }
    else if(gui->clipStackIdx < 0)
    {
        printf("ERROR: GUI_PopClip Missmatch\n");
        gui->clipStackIdx=0;
    }
}




float4 ascii_to_uv(char ch)
{
    int x = ((int)ch)*16;
    int y = 16*(x / 256);
    x = x % 256;


    return (float4)(x/255.0,y/255.0, (x+ 16)/255.0, (y+16)/255.0);
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
        float2 uvstart;
        float2 uvend;

        float4 uv = ascii_to_uv(str[i]);
        uvstart.x = uv.x;
        uvstart.y = uv.y;
        uvend.x = uv.z;
        uvend.y = uv.w;
        //printf("deawinggdfjkg");

        if(str[i] == '\n')
        {
            posy += charHeight;
            offsetx = 0;
            i++;
            continue;
        }

        

        GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, posx + offsetx, posy, charWidth, charHeight, gameState->guiStyle.TEXT_COLOR, uvstart, uvend );
        i++;
        offsetx+=charWidth;
    }


}
void GUI_LABEL(GUIID_DEF_ALL, char* str, float3 color)
{

    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return;



    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    
    if(str != NULL)
    {
        GUI_PushClip(gui, origPos, origSize);
            GUI_TEXT(GUIID_PASS,  origPos, origSize, 0, str);
        GUI_PopClip(gui);
    }
}

void GUI_IMAGE(GUIID_DEF_ALL, float2 uvStart, float2 uvEnd, float3 color)
{

    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return;

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, uvStart + (float2)(1.0,0.0), uvEnd+ (float2)(1.0,0.0) );

}

cl_uchar GUI_BUTTON(GUIID_DEF_ALL, char* str, int* down, bool* toggleState)
{

    GUI_COMMON_WIDGET_START()


    if(toggleState != NULL)
    {
        if(!goodStart)
            return *toggleState;
    }


    cl_uchar ret = 0;
    *down = 0;
    if((GUI_InteractionBoundsCheck(gui, pos, INT2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id)))
    {
        gui->hoverWidget = id;

        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown))
        {
            gui->activeWidget = id;
            *down = 1;
        }
        
        else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased))
        {
            ret = 1;
            if(toggleState != NULL)
            {
                if(*toggleState)
                    *toggleState = 0;
                else
                {

                    *toggleState = 1;


                }
            }
        }
        gui->mouseOnGUI = 1;
    }

    float3 color = gameState->guiStyle.BUTTON_COLOR;

    if(gui->hoverWidget == id)
        color =  gameState->guiStyle.BUTTON_COLOR_HOVER;

    if(gui->activeWidget == id){
        color = gameState->guiStyle.BUTTON_COLOR_ACTIVE;
    }

    if(toggleState != NULL && *toggleState == true)
    {
        color = gameState->guiStyle.BUTTON_COLOR_TOGGLED;
    }


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    
    if(str != NULL)
    {
        GUI_PushClip(gui, origPos, origSize);
            GUI_TEXT(GUIID_PASS,  origPos, origSize, 0, str);
        GUI_PopClip(gui);
    }

    if(toggleState != NULL)
    {
        ret = *toggleState;
    }

    return ret;
}







cl_uchar GUI_SLIDER_INT_HORIZONTAL(GUIID_DEF_ALL, PARAM_GLOBAL_POINTER int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return 0;

    ge_int2 posHandle;
    ge_int2 sizeHandle;

    sizeHandle.y = size.y;
    sizeHandle.x = size.x/10;

    if((GUI_InteractionBoundsCheck(gui, pos, INT2_ADD(pos, size), gui->mouseLoc)&&(gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id+1)))
    {
        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {
            float perc = ((float)(gui->mouseLoc.x - pos.x))/size.x;
            int d = perc*(max-min);
            (*value) = clamp(min + d, min, max-1);
        }
        gui->mouseOnGUI = 1;
    }


    posHandle.y = origPos.y;
    posHandle.x = origPos.x + (((size.x-sizeHandle.x)*((*value - min)))/(max-1-min));

    posHandle.x = clamp(posHandle.x, origPos.x, origPos.x + size.x - sizeHandle.x);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    

    int down;
    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, 0, NULL, &down, NULL);

    return 0;
}

cl_uchar GUI_SLIDER_INT_VERTICAL(GUIID_DEF_ALL, PARAM_GLOBAL_POINTER int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()
    if(!goodStart)
        return 0;

    ge_int2 posHandle;
    ge_int2 sizeHandle;

    sizeHandle.y = size.y/10;
    sizeHandle.x = size.x;

    if((GUI_InteractionBoundsCheck(gui, pos, INT2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id+1)))
    {
        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown) || BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased) )
        {
            float perc = ((float)(gui->mouseLoc.y  - pos.y))/(size.y );
            int d = perc*(max-min);
            (*value) = clamp(min + d, min, max-1);
        }
        gui->mouseOnGUI = 1;
    }


    posHandle.x = origPos.x;

    posHandle.y = origPos.y + (((size.y-sizeHandle.y)*((*value - min)))/(max-1-min));

    posHandle.y = clamp(posHandle.y, origPos.y, origPos.y + size.y - sizeHandle.y);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );


    int down;
    LOCAL_STRL(valueTxt, "strofnoconsequence", valueTxtLen);
    CL_ITOA(*value, valueTxt, valueTxtLen, 10); 
    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, 0, valueTxt, &down, NULL);

    return 0;
}


void GUI_SCROLLBOX_BEGIN(GUIID_DEF_ALL, ge_int2 scrollSpace, PARAM_GLOBAL_POINTER int* scrollx, PARAM_GLOBAL_POINTER int* scrolly)
{
    GUI_COMMON_WIDGET_START();
    if(!goodStart)
        return;


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

        GUI_SLIDER_INT_VERTICAL(GUIID_PASS, vertSliderPos, vertSliderSize, 0, scrolly, 0, b);
        scrollOffset.y = -*scrolly;
    }
    if(a>0)
    {
        
        GUI_SLIDER_INT_HORIZONTAL(GUIID_PASS, horSliderPos, horSliderSize, 0, scrollx, 0, a);
        scrollOffset.x= -*scrollx;
    }

    GUI_PushClip(gui, origPos, canvasSize);

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    canvasSize.x, canvasSize.y, (float3)(0.2,0.2,0.2), gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );

    GUI_PushContainer(gui, origPos + scrollOffset, origSize);

  
}

void GUI_SCROLLBOX_END(GUIID_DEF)
{
   GUI_PopContianer(gui);
   GUI_PopClip(gui);

}


bool GUI_BEGIN_WINDOW(GUIID_DEF_ALL)
{

    GUI_COMMON_WIDGET_START();
    if(!goodStart)
        return false;



    float3 headerColor = (float3)(0.4,0.4,0.4);


    if((GUI_InteractionBoundsCheck(gui, pos, INT2_ADD(pos, size), gui->mouseLoc) && (gui->ignoreAll == 0) && (gui->dragOff == 0)) || (gui->dragOff && ( gui->lastActiveWidget == id)))
    {
        gui->hoverWidget = id;

        if(BITGET(gui->mouseState, MouseButtonBits_PrimaryDown))
        {
            gui->activeWidget = id;
            headerColor = (float3)(0.2,0.2,0.2);
          //  pos += gui->mouseFrameDelta;
         //   origPos += gui->mouseFrameDelta;
        }
        
        else if(BITGET(gui->mouseState, MouseButtonBits_PrimaryReleased))
        {
            headerColor = (float3)(0.5,0.5,0.5);
        }
        gui->mouseOnGUI = 1;
    }







\

    GUI_PushClip(gui, pos, size);
    
    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, gameState->guiStyle.WINDOW_HEADER_SIZE, headerColor, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y+gameState->guiStyle.WINDOW_HEADER_SIZE, 
    size.x, size.y-gameState->guiStyle.WINDOW_HEADER_SIZE, (float3)(0.6,0.6,0.6), gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );

    GUI_PushContainer(gui, origPos+(ge_int2)(gameState->guiStyle.WINDOW_PADDING,gameState->guiStyle.WINDOW_HEADER_SIZE), 
    origSize-(ge_int2)(gameState->guiStyle.WINDOW_PADDING*2,gameState->guiStyle.WINDOW_PADDING + gameState->guiStyle.WINDOW_HEADER_SIZE));

}

void GUI_END_WINDOW(GUIID_DEF)
{
    GUI_PopClip(gui);

    GUI_PopContianer(gui);
}