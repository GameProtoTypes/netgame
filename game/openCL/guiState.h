#define GUI_MAX_WINDOWS (10)
struct GuiState
{
	
	bool menuToggles[4];
	int menuScrollx;
	int menuScrolly;

	ge_int2 windowPositions[GUI_MAX_WINDOWS];
	ge_int2 windowSizes[GUI_MAX_WINDOWS];
} typedef GuiState;


void GuiState_Init(GuiState* state)
{


    for(int w = 0 ; w < GUI_MAX_WINDOWS; w++)
    {
        state->windowPositions[w] = (ge_int2)(100,100);
        state->windowSizes[w] = (ge_int2)(200,400);
    }
    

}