#pragma once
#include "peep.h"




void ActionTrackingInit(ActionTracking* actionTracking)
{
	actionTracking->clientId = 0;
	actionTracking->ticksLate = 0;
	actionTracking->clientApplied = false;
}

void ActionWrapInit(ActionWrap* actionWrap)
{
	ActionTrackingInit(&actionWrap->tracking);
}