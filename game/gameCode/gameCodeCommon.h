#pragma once
#define MAX_CLIENTS (1024)


#include "netSyncStructs.h"
#include "gameBasicTypes.h"

#define ALL_CORE_PARAMS  \
GameState* gameState,\
GameStateActions* gameStateActions,\
int threadIdx, int numThreads

#define ALL_CORE_PARAMS_TYPES  \
GameState* ,\
GameStateActions* ,\
int , int 



#define ALL_CORE_PARAMS_PASS  \
((GameState*) gameState), \
((GameStateActions*) gameStateActions)


struct SynchronizedClientState {

	int32_t connected;
	uint32_t selectedPeepsLastIdx;

    int companyValue = 0;
} typedef SynchronizedClientState;


struct GameState
{
	SynchronizedClientState clientStates[MAX_CLIENTS];
	int numClients;

    int tickidx;
};



void Kernel_A(ALL_CORE_PARAMS);
void Kernel_B(ALL_CORE_PARAMS);
void Kernel_C(ALL_CORE_PARAMS);
void Kernel_D(ALL_CORE_PARAMS);