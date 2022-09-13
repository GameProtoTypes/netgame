#pragma once
#include "cpugpuvectortypes.h"



struct SIZETESTSDATA {

	cl_uint gameStateStructureSize;
	cl_uint staticDataStructSize;



} typedef SIZETESTSDATA;



enum ClientActionCode {
	ClientActionCode_MouseStateChange,
	ClientActionCode_KeyStateChange,
	ClientActionCode_NONE = 255
} typedef ClientActionCode;

enum ClientActionCode_MouseStateChange_IntParams {
	CAC_MouseStateChange_Param_GUI_X,
	CAC_MouseStateChange_Param_GUI_Y,
	CAC_MouseStateChange_Param_WORLD_X_Q16,
	CAC_MouseStateChange_Param_WORLD_Y_Q16,
	CAC_MouseStateChange_Param_BUTTON_BITS,
	CAC_MAX//Move to largest enum as appropriate
};

enum ClientActionCode_KeyStateChange_IntParams {
	CAC_KeyStateChange_Param_KEY,
	CAC_KeyStateChange_Param_STATE
};


struct ClientAction {
	cl_uint scheduledTickIdx;//when it is scheduled to take effect on all clients

	ClientActionCode actionCode;
	int intParameters[CAC_MAX];//MAX of ClientActionCode_******_IntParams enums

} typedef ClientAction;


struct ActionTracking {

	cl_int clientId;

	cl_uint hostGivenId;
	cl_uint clientGivenId;
	cl_int ticksLate;//action could not be applied on client at scheduled tickId;
	bool finalActionVerified;
	bool clientApplied;

} typedef ActionTracking;



struct ActionWrap {
	ClientAction action;
	ActionTracking tracking;

} typedef ActionWrap;


struct GameStateActions {
	ActionWrap clientActions[32];
	cl_int numActions;

	cl_uint tickIdx;
	int32_t pauseState;


	//ClientSide only stuff that is processed in cl but not strictly gamestate and driven by host.
	cl_uint clientId;
	cl_int mapZView;
	cl_int mapZView_1;

	ge_int2 mouseLoc;
	cl_int mouseState;


	cl_int dummyVars[32];

}typedef GameStateActions;





#define GUI_PXPERSCREEN (1000)
#define GUI_PXPERSCREEN_F (1000.0f)
