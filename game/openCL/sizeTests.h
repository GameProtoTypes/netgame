#pragma once
#include "cpugpuvectortypes.h"



struct SIZETESTSDATA {

	cl_uint gameStateStructureSize;
	cl_uint staticDataStructSize;



} typedef SIZETESTSDATA;



enum ClientActionCode {
	ClientActionCode_DoSelect,
	ClientActionCode_CommandToLocation,
	ClientActionCode_CommandTileDelete,
	ClientActionCode_SetZView,
	ClientActionCode_NONE = 255
} typedef ClientActionCode;

enum ClientActionCode_DoSelect_IntParams {
	CAC_DoSelect_Param_StartX_Q16,
	CAC_DoSelect_Param_StartY_Q16,
	CAC_DoSelect_Param_EndX_Q16,
	CAC_DoSelect_Param_EndY_Q16,
	CAC_DoSelect_Param_ZMapView,
	CAC_MAX//Move to largest enum as appropriate
};

enum ClientActionCode_CommandToLocation_IntParams {
	CAC_CommandToLocation_Param_X_Q16,
	CAC_CommandToLocation_Param_Y_Q16
};

enum ClientActionCode_CommandTileDelete_IntParams {
	CAC_CommandTileDelete_Param_X_Q16,
	CAC_CommandTileDelete_Param_Y_Q16
};
enum ClientActionCode_SetZView_IntParams {
	CAC_CommandTileDelete_Param_ZViewIdx
};

struct ClientAction {

	//cl_uint submittedTickIdx;//the client tickidx when the action was created
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


	cl_int dummyVars[32];

}typedef GameStateActions;







