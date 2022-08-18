#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <memory>
#include <sstream>

#include "slikenet/peerinterface.h"
#include "slikenet/peer.h"
#include "slikenet/NatPunchthroughClient.h"
#include "slikenet/MessageIdentifiers.h"
#include "slikenet/BitStream.h"
#include "slikenet/DataCompressor.h"


#include "zlib.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "peep.h"

#include "GameGPUCompute.h"




#define GAMESERVERPORT (50010)

#define MAX_HOST_CONNECTIONS 256 
#define MAX_USER_MESSAGE_LENGTH 255


#define MINTICKTIMEMS (16)
#define GOODTICKTIMEMS (MINTICKTIMEMS*2)
#define SLOWTICKTIMEMS (MINTICKTIMEMS*4)
#define MAXTICKTIMEMS (MINTICKTIMEMS*10)

#define REALLYBIGPING (10000)
#define TRANSFERCHUNKSIZE (1024*1024)

#define CLIENT_TIMEOUT_TICKS (25)
class GameNetworking
{
public:
	GameNetworking(std::shared_ptr<GameState> gameState, std::shared_ptr<GameStateActions> gameStateActions, GameGPUCompute* gameCompute) {
	
		this->gameState = gameState;
		this->gameStateActions = gameStateActions;
		this->gameCompute = gameCompute;
	}
	~GameNetworking() {}


	enum MESSAGE_ENUMS {
		MESSAGE_ENUM_GENERIC_MESSAGE = 0,

		MESSAGE_ENUM_CLIENT_INITIALDATA,

		MESSAGE_ENUM_HOST_SYNCDATA1,
		MESSAGE_ENUM_HOST_GAMEDATA_PART,

		MESSAGE_ENUM_CLIENT_GAMEDATA_PART_ACK,

		MESSAGE_ENUM_CLIENT_SYNC_COMPLETE,




		MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC,
		MESSAGE_ENUM_HOST_ROUTINE_TICKSYNC,

		MESSAGE_ENUM_CLIENT_DISCONNECT_NOTIFY,
		MESSAGE_ENUM_HOST_DISCONNECTED_NOTIFY,


		MESSAGE_ENUM_CLIENT_ACTIONUPDATE,
		MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA
	};



	void Init();




	void CLIENT_SendActionUpdate_ToHost(std::vector<ActionWrap>& clientActions);



	void CLIENT_SENDInitialData_ToHost();


	void CLIENT_ApplyActions();

	void CLIENT_SendGamePartAck();
	void CLIENT_SendSyncComplete();



	void HOST_SendActionUpdates_ToClients(const std::vector<ActionWrap>& actions);

	void HOST_SendSyncStart_ToClient(int32_t cliIdx, SLNet::RakNetGUID clientAddr);
	void HOST_SendGamePart_ToClient(uint32_t clientGUID);


	void HOST_HandleDisconnectByCLientGUID(uint32_t clientGUID);
	void HOST_HandleDisconnectByID(int32_t clientID);

	void StartServer(int32_t port);
	
	void StopServer()
	{

	}

	//send null terminated message
	void SendMessage(char* message);

	void SendTickSyncToHost();
	void SendTickSyncToClients();


	uint64_t CheckSumGameState(GameState* state);
	uint64_t CheckSumAction(ActionWrap* state);


	void ConnectToHost(SLNet::SystemAddress hostAddress);

	void CLIENT_HardDisconnect();
	void CLIENT_SoftDisconnect();

	void Update();
	void UpdateThrottling();
	void UpdateHandleMessages();




	int32_t MaxStandardConnections = MAX_HOST_CONNECTIONS;


	bool serverRunning = false;
	bool connectedToHost = false;
	bool fullyConnectedToHost = false;

	SLNet::SocketDescriptor SocketDesc = SLNet::SocketDescriptor();

	//current recieving package
	SLNet::Packet* packet;


	// The flag for breaking the loop inside the packet listening thread.
	bool isListening = false;

	void AddClientInternal(uint32_t clientGUID, SLNet::RakNetGUID rakguid);

	SLNet::RakPeerInterface* peerInterface;
	
	struct clientMeta {

		int32_t cliId;
		int32_t hostTickOffset;
		uint32_t clientGUID;
		SLNet::RakNetGUID rakGuid;
		int ticksSinceLastCommunication;
		int downloadingState;
		int32_t avgHostPing;
	};
	std::vector<clientMeta> clients;
	int32_t nextCliIdx = 0;	
	int32_t clientId = -1;

	clientMeta* GetClientMetaDataFromCliId(uint8_t cliId);

	clientMeta* GetClientMetaDataFromCliGUID(uint32_t cliGUID);

	std::string HostConsolePrint() { std::string str;
		char buff[100];
		snprintf(buff, sizeof(buff), "[HOST (tickIdx: %d)] ", gameStateActions->tickIdx);
		return buff;
	}
	std::string ClientConsolePrint() {
		char buff[100];
		snprintf(buff, sizeof(buff), "[CLIENT (tickIdx: %d)] ", gameStateActions->tickIdx);
		return buff;
	}

	void ClientDisconnectRoutines();


	SLNet::RakNetGUID hostPeer;

	GameGPUCompute* gameCompute = nullptr;

	std::shared_ptr<GameState> gameState;
	std::shared_ptr<GameStateActions> gameStateActions;

	std::shared_ptr<GameState> CLIENT_gameStateTransfer;
	std::shared_ptr<GameState> HOST_gameStateTransfer;
	
	struct snapshotWrap {
		std::shared_ptr<GameState> gameState;
		std::shared_ptr<GameStateActions> gameStateActions;
		uint64_t checksum = static_cast<uint64_t>( -1 );
		
	};
	std::vector<snapshotWrap> CLIENT_snapshotStorageQueue;
	std::vector<ActionWrap> CLIENT_actionList;

	uint64_t HOST_nextTransferOffset[MAX_CLIENTS] = { 0 };
	uint64_t CLIENT_nextTransferOffset = 0;
	uint64_t transferFullCheckSum = 0;
	bool HOST_snapshotLocked = false;//snapshot locked because its being transfered or other reason.
	float gameStateTransferPercent = 0.0f;

	bool actionStateDirty = false;

	uint32_t HOST_lastActionScheduleTickIdx = 0;//keep schedule order.

	uint32_t lastFreezeTick = 0;
	int32_t snapshotFreq = 200;

	int32_t targetTickTimeMs = GOODTICKTIMEMS;
	float integralAccumulatorTickPID = 0.0f;
	float tickPIDError = 0.0f;

	int64_t lastFrameTimeMs = MINTICKTIMEMS;

	uint32_t clientGUID;
};

