#pragma once
#define GUIID_PASS ALL_CORE_PARAMS_PASS, gui, GrabGuiId(gui)


#define GUIID_DEF ALL_CORE_PARAMS, SyncedGui* gui, int id
#define GUIID_DEF_POSSIZE GUIID_DEF, ge_int2 pos, ge_int2 size

#define GUI_FAKESWITCH_PARAM_INT(PARAM) GuiFakeSwitch_Param_Int(gui, PARAM)
#define GUI_GETOFFSET() GUI_GetOffset(gui)
#define GUI_AUTO_SIZE (ge_int2){-1,-1}





void GUI_PushOffset(SyncedGui* gui, ge_int2 offset)
{
    gui->wOSidx++;
    if(gui->wOSidx >= SYNCGUI_MAX_DEPTH)
        printf("ERROR: GUI_PushOffset Overflow (SYNCGUI_MAX_DEPTH)");

    gui->widgetOffsetStack[gui->wOSidx] = offset;
}



void GUI_Begin_ScrollArea(GUIID_DEF_POSSIZE, ge_int2 scroll_offset)
{
    
    GUI_PushOffset(gui, scroll_offset);
}


void GUI_PopOffset(SyncedGui* gui)
{
    gui->wOSidx--;
    if(gui->wOSidx < -1)
        printf("ERROR: GUI PopOffset Call Missmatch.");
}



void  GUI_End_ScrollArea(SyncedGui* gui)
{
    GUI_PopOffset(gui);
}




cl_uchar GUI_MOUSE_ON_GUI(SyncedGui* gui)
{
    return gui->mouseOnGUI;
}

//call before gui path
void GUI_RESET(ALL_CORE_PARAMS, SyncedGui* gui, ge_int2 mouseLoc, int mouseState, GuiStatePassType passType)
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
    gui->nextFakeIntIdx = 0;

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


    gui->mouseOnGUI = 0;
}




void GUI_INIT_STYLE(ALL_CORE_PARAMS)
{

    gameState->guiStyle.TEXT_COLOR = (float3)(1.0,1.0,1.0);

    gameState->guiStyle.UV_WHITE = (float2)(40.0/255, 20.0/255);

    gameState->guiStyle.BUTTON_COLOR = (float3)(0.5,0.5,0.5);
    gameState->guiStyle.BUTTON_COLOR_HOVER = (float3)(0.7,0.7,0.7);
    gameState->guiStyle.BUTTON_COLOR_ACTIVE = (float3)(0.4,0.4,0.4);

    gameState->guiStyle.SLIDER_COLOR_BACKGROUND = (float3)(0.2,0.2,0.2);

}








SyncedGui* GetGuiState(ALL_CORE_PARAMS, int clientId)
{
    return &gameState->clientStates[clientId].gui;
}
int GrabGuiId(SyncedGui* gui)
{
    int id =  gui->nextId;;
    gui->nextId++;
    return id;
}

ge_int4 GUI_MERGED_CLIP(SyncedGui* gui)
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

void GUI_DrawRectangle(ALL_CORE_PARAMS, SyncedGui* gui, int x, int y, int width, int height, float3 color, float2 UVStart, float2 UVEnd)
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
cl_uchar GUI_InteractionBoundsCheck(SyncedGui* gui, ge_int2 boundStart, ge_int2 boundEnd, ge_int2 pos)
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

int* GUI_GetFakeInt(SyncedGui* gui)
{
    int* param = &gui->fakeInts[gui->nextFakeIntIdx];
    gui->nextFakeIntIdx++;

    if(gui->nextFakeIntIdx >= SYNCGUI_MAX_WIDGETS)
        printf("GUI_OUT_OF_WIDGETS\n");
    
    return param;
}

int* GuiFakeSwitch_Param_Int(SyncedGui* gui, int* param)
{
    if(gui->passType == GuiStatePassType_Synced)
    {
        return param;
    }
    else
    {
        return GUI_GetFakeInt(gui);
    }
}




ge_int2 GUI_GetOffset(SyncedGui* gui)
{
    ge_int2 sum = (ge_int2){0,0};
    for(int i = 0; i <= gui->wOSidx; i++)
    {
        sum = INT2_ADD(sum, gui->widgetOffsetStack[gui->wOSidx]);
    }
    return sum;
}


void GUI_PushClip(SyncedGui* gui, ge_int2 startPos, ge_int2 size)
{
    startPos = INT2_ADD(startPos, GUI_GETOFFSET());

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
        gui->clipStack[gui->clipStackIdx] = (ge_int4){0,0,GUI_PXPERSCREEN,GUI_PXPERSCREEN};
    }
    else if(gui->clipStackIdx < 0)
    {
        printf("ERROR: GUI_PopClip Missmatch\n");
        gui->clipStackIdx=0;
    }
}



#define GUI_COMMON_WIDGET_START() gui_CommonWidgetStart(ALL_CORE_PARAMS_PASS,  gui, &pos, &size);
void gui_CommonWidgetStart(ALL_CORE_PARAMS, SyncedGui* gui, ge_int2 * pos, ge_int2* size)
{
    *pos = INT2_ADD(*pos, GUI_GETOFFSET());
}








float4 ascii_to_uv(char ch)
{
    int x = ((int)ch)*16;
    int y = 16*(x / 256);
    x = x % 256;


    return (float4)(x/255.0,y/255.0, (x+ 16)/255.0, (y+16)/255.0);
}



void GUI_TEXT(GUIID_DEF_POSSIZE, char* str)
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
void GUI_LABEL(GUIID_DEF_POSSIZE, char* str, float3 color)
{
    GUI_COMMON_WIDGET_START()

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    
    if(str != NULL)
    {
        GUI_PushClip(gui, pos, size);
            GUI_TEXT(GUIID_PASS,  pos, size, str);
        GUI_PopClip(gui);
    }
}

cl_uchar GUI_BUTTON(GUIID_DEF_POSSIZE, char* str, int* down)
{
    ge_int2 origPos = pos;
    ge_int2 origSize = size;
    GUI_COMMON_WIDGET_START()


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
        }
        gui->mouseOnGUI = 1;
    }

    float3 color = gameState->guiStyle.BUTTON_COLOR;

    if(gui->hoverWidget == id)
        color =  gameState->guiStyle.BUTTON_COLOR_HOVER;

    if(gui->activeWidget == id){
        color = gameState->guiStyle.BUTTON_COLOR_ACTIVE;
    }


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, color, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    
    if(str != NULL)
    {
        GUI_PushClip(gui, origPos, origSize);
            GUI_TEXT(GUIID_PASS,  origPos, origSize, str);
        GUI_PopClip(gui);
    }


    return ret;
}







cl_uchar GUI_SLIDER_INT_HORIZONTAL(GUIID_DEF_POSSIZE, int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()

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


    posHandle.y = pos.y;
    posHandle.x = pos.x + (((size.x-sizeHandle.x)*((*value - min)))/(max-1-min));

    posHandle.x = clamp(posHandle.x, pos.x, pos.x + size.x - sizeHandle.x);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    

    int down;
    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, NULL, &down);

}

cl_uchar GUI_SLIDER_INT_VERTICAL(GUIID_DEF_POSSIZE, int* value, int min, int max)
{
    GUI_COMMON_WIDGET_START()

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


    posHandle.x = pos.x;

    posHandle.y = pos.y + (((size.y-sizeHandle.y)*((*value - min)))/(max-1-min));

    posHandle.y = clamp(posHandle.y, pos.y, pos.y + size.y - sizeHandle.y);


    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    size.x, size.y, gameState->guiStyle.SLIDER_COLOR_BACKGROUND, gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );
    

    int down;
    LOCAL_STRL(valueTxt, "strofnoconsequence", valueTxtLen);
    CL_ITOA(*value, valueTxt, valueTxtLen, 10); 
    GUI_BUTTON(GUIID_PASS, posHandle, sizeHandle, valueTxt, &down);

}


void GUI_SCROLLBOX_BEGIN(GUIID_DEF_POSSIZE, ge_int2 scrollSpace)
{
    ge_int2 scrollOffset;
    scrollOffset.x = 0;
    scrollOffset.y = 0;
    int* scrollx=GUI_GetFakeInt(gui);
    int* scrolly=GUI_GetFakeInt(gui);

    ge_int2 vertSliderPos;
    vertSliderPos.y = pos.y;
    vertSliderPos.x = pos.x + size.x - 20;

    ge_int2 vertSliderSize;
    vertSliderSize.x = 20;
    vertSliderSize.y = size.y-20;

    ge_int2 horSliderPos;
    horSliderPos.y = pos.y + size.y - 20;
    horSliderPos.x = pos.x;

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
        GUI_SLIDER_INT_VERTICAL(GUIID_PASS, vertSliderPos, vertSliderSize, scrolly, 0, b);
        scrollOffset.y = -*scrolly;
    }
    if(a>0)
    {
        GUI_SLIDER_INT_HORIZONTAL(GUIID_PASS, horSliderPos, horSliderSize, scrollx, 0, a);
        scrollOffset.x= -*scrollx;
    }

    GUI_PushClip(gui, pos, canvasSize);

    GUI_DrawRectangle(ALL_CORE_PARAMS_PASS, gui, pos.x, pos.y, 
    canvasSize.x, canvasSize.y, (float3)(0.3,0.3,0.3), gameState->guiStyle.UV_WHITE, gameState->guiStyle.UV_WHITE );

    GUI_Begin_ScrollArea(GUIID_PASS, pos, canvasSize, scrollOffset);
}

void GUI_SCROLLBOX_END(GUIID_DEF)
{
    GUI_End_ScrollArea(gui);
    GUI_PopClip(gui);

}