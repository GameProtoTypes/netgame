#pragma once
#include "glm.hpp"

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
	int scheduledTickIdx;//when it is scheduled to take effect on all clients



	ClientActionCode actionCode;
	int intParameters[CAC_MAX];//MAX of ClientActionCode_******_IntParams enums

} typedef ClientAction;


struct ActionTracking {

	int clientId;

	uint32_t hostGivenId;
	uint32_t clientGivenId;
	int32_t ticksLate;//action could not be applied on client at scheduled tickId;
	bool finalActionVerified;
	bool clientApplied;
	uint32_t sentTickIdx;//when the action was sent by client

} typedef ActionTracking;



struct ActionWrap {
	ClientAction action;
	ActionTracking tracking;

} typedef ActionWrap;

#define MAX_ACTIONS_PER_TICK (MAX_CLIENTS)
struct GameStateActions {
	ActionWrap clientActions[MAX_ACTIONS_PER_TICK];
	int32_t numActions;




	uint32_t tickIdx;
	int32_t pauseState;


	//ClientSide only stuff that is processed in cl but not strictly gamestate and driven by host.
	uint32_t clientId;



	int32_t mouseLocx;
	int32_t mouseLocy;
	int32_t mouseLocWorldx_Q16;
	int32_t mouseLocWorldy_Q16;
	int32_t mouseState;

	float viewMatrix[4][4];
	float viewMatrix_Inv[4][4];

	int32_t dummyVars[32];

}typedef GameStateActions;


enum MouseButtonBits {
	MouseButtonBits_PrimaryPressed,
	MouseButtonBits_SecondaryPressed,
	MouseButtonBits_PrimaryReleased,
	MouseButtonBits_SecondaryReleased,
	MouseButtonBits_PrimaryDown,
	MouseButtonBits_SecondaryDown
} typedef MouseButtonBits;


#define GUI_PXPERSCREEN (2000)
#define GUI_PXPERSCREEN_F (2000.0f)
