#pragma once

#include <iostream>
#include <vector>
#include <thread>

#include "slikenet/peerinterface.h"
#include "slikenet/peer.h"
#include "slikenet/NatPunchthroughClient.h"
#include "slikenet/MessageIdentifiers.h"
#include "slikenet/BitStream.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "peep.h"



#define MAX_HOST_CONNECTIONS 256 
#define MAX_USER_MESSAGE_LENGTH 255






class GameNetworking
{
public:
	GameNetworking(GameState* gameState) {
	
		this->gameState = gameState;
	}
	~GameNetworking() {}


	enum MESSAGE_ENUMS {
		MESSAGE_ENUM_GENERIC_MESSAGE = 0,

		MESSAGE_ENUM_HOST_SYNCDATA1,
		MESSAGE_ENUM_HOST_SYNCDATA2,

		MESSAGE_ENUM_CLIENT_SYNC_COMPLETE,




		MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC,
		MESSAGE_ENUM_HOST_ROUTINE_TICKSYNC,



		MESSAGE_ENUM_CLIENT_ACTIONUPDATE,
		MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA,
		MESSAGE_ENUM_CLIENT_ACTION_ERROR,
	};



	void Init();




	void CLIENT_SendActionUpdate_ToHost(std::vector<ActionWrap>& clientActions);


	void HOST_SendActionUpdates_ToClients();

	void CLIENT_ApplyCombinedTurn()
	{
		std::vector<int32_t> removals;
		std::vector<ActionWrap> newList;
		int32_t i = 0;
		for (int32_t a = 0; a < turnActions.size(); a++)
		{
			ClientAction* action = &turnActions[a].action;
			ActionTracking* actTracking = &turnActions[a].tracking;
			if (action->scheduledTickIdx == gameState->tickIdx)
			{
				std::cout << "[CLIENT] Using Action" << std::endl;
				gameState->clientActions[i] = turnActions[a];
				i++;

			}
			else if (action->scheduledTickIdx < gameState->tickIdx)
			{

				actTracking->ticksLate = gameState->tickIdx - action->scheduledTickIdx;
				std::cout << "[CLIENT] Expired Action "
					<< actTracking->ticksLate << " Ticks Late" << std::endl;


				SLNet::BitStream bs;
				bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
				bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_ACTION_ERROR));
				bs.Write(reinterpret_cast<char*>(actTracking), sizeof(ActionTracking));


				this->peerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE, 1, hostPeer, false);

			}
			else
			{
				newList.push_back(turnActions[a]);
			}
		}
		turnActions = newList;
		gameState->numActions = i;
	}

	void CLIENT_SendSyncComplete()
	{
		SLNet::BitStream bs;
		bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_SYNC_COMPLETE));

		this->peerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE, 1, hostPeer, false);
	}

	void HOST_SendSync_ToClient(uint8_t cliIdx, SLNet::RakNetGUID clientAddr)
	{
		std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_SYNCDATA1 To client" << std::endl;
		SLNet::BitStream bs;
		bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_SYNCDATA1));
		bs.Write(static_cast<uint8_t>(cliIdx));
		bs.Write(reinterpret_cast<char*>(gameState), sizeof(GameState));

		this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, clientAddr, false);
	}

	void StartServer(int32_t port);
	
	void StopServer()
	{

	}

	//send null terminated message
	void SendMessage(char* message);

	void SendTickSyncToHost()
	{
		SLNet::BitStream bs;

		bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC));
		bs.Write(static_cast<uint8_t>(clientId));
		bs.Write(static_cast<uint32_t>(gameState->tickIdx));
		bs.Write(static_cast<int32_t>(minTickTimeMs));

		
		this->peerInterface->Send(&bs, HIGH_PRIORITY, UNRELIABLE,
			1, hostPeer, false);
	}

	void ConnectToHost(SLNet::SystemAddress hostAddress);


	void Update();
	
	
	
	int32_t MaxStandardConnections = MAX_HOST_CONNECTIONS;


	bool serverRunning = false;
	bool connectedToHost = false;

	SLNet::SocketDescriptor SocketDesc = SLNet::SocketDescriptor();

	//current recieving package
	SLNet::Packet* packet;

	// The  thread for receiving packets in the background.
	std::thread* listenLoopThread;
	// The flag for breaking the loop inside the packet listening thread.
	bool isListening;

	void Host_AddClientInternal(SLNet::RakNetGUID guid)
	{
		clientMeta meta;
		meta.cliId = nextCliIdx;
		meta.rakGuid = guid;
		meta.hostTickOffset = 0;
		clients.push_back(meta);
		nextCliIdx++;
	}

	SLNet::RakPeerInterface* peerInterface;
	
	struct clientMeta {
		uint8_t cliId;
		int32_t hostTickOffset;
		SLNet::RakNetGUID rakGuid;
	};
	std::vector<clientMeta> clients;
	uint8_t nextCliIdx = 0;
	clientMeta* GetClientMetaDataFromCliId(uint8_t cliId)
	{
		for (int32_t i = 0; i < clients.size(); i++)
		{
			if (clients[i].cliId == cliId)
				return &clients[i];
		}

		return nullptr;
	}

	int32_t maxTickLag = 999999; // tick lag of slowest client

	bool fullyConnectedToHost = false;
	SLNet::RakNetGUID hostPeer;

	GameState* gameState = nullptr;
	uint8_t clientId = 0;
	bool actionStateDirty = false;


	std::vector<ActionWrap> turnActions;

	std::vector<ActionWrap> runningActions;
	std::vector<ActionWrap> freezeFrameActions;
	std::vector<ActionWrap> freezeFrameActions_1;

	uint32_t lastFreezeTick = 0;
	GameState* lastFreezeGameState = nullptr;
	int32_t freezeFreq = 20;

	int32_t minTickTimeMs = 33;

};
