#define ALL_CORE_PARAMS \
StaticData* staticData, \
GameState* gameState,\
GameStateActions* gameStateActions, \
GuiStyle* guiStyle, \
ge_float* peepVBOBuffer, \
ge_float* particleVBOBuffer, \
ge_ubyte* mapTile1VBO, \
ge_ubyte* mapTile1AttrVBO, \
ge_ubyte* mapTile1OtherAttrVBO, \
ge_ubyte* mapTile2VBO, \
ge_ubyte* mapTile2AttrVBO, \
ge_ubyte* mapTile2OtherAttrVBO, \
ge_float* guiVBO, \
ge_float* linesVBO, \
int threadIdx, int numThreads




#define ALL_CORE_PARAMS_PASS  \
((StaticData*) staticData), \
((GameState*) gameState), \
((GameStateActions*) gameStateActions), \
guiStyle, \
peepVBOBuffer, \
particleVBOBuffer, \
mapTile1VBO, \
mapTile1AttrVBO, \
mapTile1OtherAttrVBO, \
mapTile2VBO, \
mapTile2AttrVBO, \
mapTile2OtherAttrVBO, \
guiVBO, \
linesVBO, \
threadIdx,  numThreads



#define GAMEGUI_PARAMS \
    SyncedGui* gui, int id, GUI_OPTIONS


#define GAMEGUI_PARAMS_PASS \
   gui, GrabGuiId(gui)