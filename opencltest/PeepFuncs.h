#pragma once
#include "peep.h"

void ClientActionInit(ClientAction* action)
{
	action->action_DoSelect = 0;
	action->params_DoSelect_StartX_Q16 = 0;
	action->params_DoSelect_StartY_Q16 = 0;
	action->params_DoSelect_EndX_Q16 = 0;
	action->params_DoSelect_EndY_Q16 = 0;

	action->action_CommandToLocation = 0;
	action->params_CommandToLocation_X_Q16 = 0;
	action->params_CommandToLocation_Y_Q16 = 0;
}


void ActionTrackingInit(ActionTracking* actionTracking)
{
	actionTracking->clientId = 0;
	actionTracking->ticksLate = 0;
}

void ActionWrapInit(ActionWrap* actionWrap)
{
	ClientActionInit(&actionWrap->action);
	ActionTrackingInit(&actionWrap->tracking);
}