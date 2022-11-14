
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


void GuiState_Init(PARAM_GLOBAL_POINTER GuiState* state)
{


    for(int w = 0 ; w < GUI_MAX_WINDOWS; w++)
    {
        state->windowPositions[w] = (ge_int2)(100,100);
        state->windowSizes[w] = (ge_int2)(200,400);
    }
    

}