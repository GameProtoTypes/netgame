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

		MESSAGE_ROUTINE_TICKSYNC,

		MESSAGE_ENUM_CLIENT_ACTIONUPDATE,
		MESSAGE_ENUM_HOST_TURNDATA
	};








	void Init();




	void CLIENT_SendActionUpdate_ToHost(std::vector<ClientAction>& clientActions);


	void HOST_SendActionUpdates_ToClients();

	void CLIENT_ApplyCombinedTurn()
	{
		std::vector<int> removals;
		std::vector<ClientAction> newList;
		int i = 0;
		for (int a = 0; a < turnActions.size(); a++)
		{
			if (turnActions[a].scheduledTickIdx == gameState->tickIdx)
			{
				gameState->clientActions[i] = turnActions[a];
				i++;
			}
			else if (turnActions[a].scheduledTickIdx < gameState->tickIdx)
			{
				std::cout << "expired action!!!!!!";
			}
			else
			{
				newList.push_back(turnActions[a]);
			}
		}
		turnActions = newList;
		gameState->numActions = i;
	}

	void HOST_SendSync_ToClient(unsigned char cliIdx, SLNet::SystemAddress clientAddr)
	{
		SLNet::BitStream bs;
		bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_HOST_SYNCDATA1));
		bs.Write(static_cast<unsigned int>(sizeof(unsigned char) + sizeof(GameState)));
		bs.Write(cliIdx);
		bs.Write(reinterpret_cast<char*>(gameState), sizeof(GameState));

		this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, clientAddr, false);
	}

	void StartServer(int port);
	
	void StopServer()
	{

	}

	//send null terminated message
	void SendMessage(char* message);

	void SendTickSyncToHost()
	{
		SLNet::BitStream bs;

		bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		bs.Write(static_cast<unsigned char>(MESSAGE_ROUTINE_TICKSYNC));
		bs.Write(static_cast<unsigned int>(gameState->tickIdx));
		bs.Write(static_cast<int>(minTickTimeMs));

		
		this->peerInterface->Send(&bs, HIGH_PRIORITY, UNRELIABLE,
			1, hostAddr, false);
	}

	void ConnectToHost(SLNet::SystemAddress hostAddress);


	void Update();
	int MaxStandardConnections = MAX_HOST_CONNECTIONS;


	bool serverRunning = false;
	bool connectedToHost = false;

	//bool EnableNatPunch = false;
	SLNet::SocketDescriptor SocketDesc = SLNet::SocketDescriptor();

	//current recieving package
	SLNet::Packet* packet;
	SLNet::SystemAddress natPunchServerAddress;
	//SLNet::NatPunchthroughClient natPunchthroughClient;

	// Flag for deciding if the current peer should send a connection
	// request to the remote peer upon a successful NAT punchthrough.
	bool sentNatPunchthroughRequest = false;


	// The  thread for receiving packets in the background.
	std::thread* listenLoopThread;
	// The flag for breaking the loop inside the packet listening thread.
	bool isListening;

	SLNet::RakPeerInterface* peerInterface;
	
	
	std::vector<std::pair<unsigned char, SLNet::SystemAddress>> clients;
	unsigned char nextCliIdx = 0;
	int clientTickLags[MAX_CLIENTS] = { 0 };

	SLNet::SystemAddress hostAddr;

	GameState* gameState;
	unsigned char localClientStateIdx = 0;
	bool actionStateDirty = false;


	std::vector<ClientAction> turnActions;
	

	int minTickTimeMs = 33;

};

