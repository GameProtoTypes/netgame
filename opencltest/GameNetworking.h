#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <memory>

#include "slikenet/peerinterface.h"
#include "slikenet/peer.h"
#include "slikenet/NatPunchthroughClient.h"
#include "slikenet/MessageIdentifiers.h"
#include "slikenet/BitStream.h"
#include "slikenet/DataCompressor.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "peep.h"

#define GAMESERVERPORT (50010)

#define MAX_HOST_CONNECTIONS 256 
#define MAX_USER_MESSAGE_LENGTH 255


#define MINTICKTIMEMS (33)
#define SLOWTICKTIMEMS (MINTICKTIMEMS*4)
#define MAXTICKTIMEMS (MINTICKTIMEMS*10)

#define REALLYBIGPING (10000)
#define TRANSFERCHUNKSIZE (1024*1024)

#define CLIENT_TIMEOUT_TICKS (25)
class GameNetworking
{
public:
	GameNetworking(std::shared_ptr<GameState> gameState) {
	
		this->gameState = gameState;
	}
	~GameNetworking() {}


	enum MESSAGE_ENUMS {
		MESSAGE_ENUM_GENERIC_MESSAGE = 0,

		MESSAGE_ENUM_CLIENT_INITIALDATA,

		MESSAGE_ENUM_HOST_SYNCDATA1,
		MESSAGE_ENUM_HOST_GAMEDATA_PART,

		MESSAGE_ENUM_HOST_RESYNC_NOTIFICATION,

		MESSAGE_ENUM_CLIENT_GAMEDATA_PART_ACK,

		MESSAGE_ENUM_CLIENT_SYNC_COMPLETE,




		MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC,
		MESSAGE_ENUM_HOST_ROUTINE_TICKSYNC,



		MESSAGE_ENUM_CLIENT_ACTIONUPDATE,
		MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA,
		MESSAGE_ENUM_CLIENT_ACTION_ERROR,
	};



	void Init();




	void CLIENT_SendActionUpdate_ToHost(std::vector<ActionWrap>& clientActions);



	void CLIENT_SENDInitialData_ToHost();


	void CLIENT_ApplyActions();

	void CLIENT_SendGamePartAck();
	void CLIENT_SendSyncComplete();



	void HOST_SendActionUpdates_ToClients(const std::vector<ActionWrap>& actions);
	void HOST_SendReSync_ToClients();

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


	void ConnectToHost(SLNet::SystemAddress hostAddress);

	void CLIENT_Disconnect();

	void Update();
	void UpdateThrottling();
	void UpdateHandleMessages();



	void SHARED_CLIENT_CONNECT(uint32_t clientGUID, SLNet::RakNetGUID systemGUID);
	
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


	SLNet::RakNetGUID hostPeer;


	std::shared_ptr<GameState> gameState;
	std::shared_ptr<GameState> CLIENT_gameStateTransfer;
	
	struct snapshotWrap {
		std::shared_ptr<GameState> gameState;
		uint64_t checksum = static_cast<uint64_t>( -1 );
		std::vector<ActionWrap> postActions;
	};
	std::vector<snapshotWrap> CLIENT_snapshotStorageQueue;


	uint64_t HOST_nextTransferOffset[MAX_CLIENTS] = { 0 };
	uint64_t CLIENT_nextTransferOffset = 0;
	uint64_t transferFullCheckSum = 0;
	bool HOST_snapshotLocked = false;//snapshot locked because its being transfered or other reason.
	float gameStateTransferPercent = 0.0f;

	bool actionStateDirty = false;

	

	uint32_t lastFreezeTick = 0;
	int32_t snapshotFreq = 200;

	int32_t targetTickTimeMs = MINTICKTIMEMS;
	float integralAccumulatorTickPID = 0.0f;
	float tickPIDError = 0.0f;

	uint32_t clientGUID;
};

