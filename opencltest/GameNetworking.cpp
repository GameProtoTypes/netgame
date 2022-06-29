#include "GameNetworking.h"





#include <vector>
#include <iostream>
#include <sstream>
#include <random>
#include <climits>
#include <algorithm>
#include <functional>
#include <string>





 void GameNetworking::Init()
{
	std::cout << "GameNetworking::Init" << std::endl;


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<uint32_t> dis(0, -1);
	clientGUID = static_cast<uint32_t>(dis(gen));

	std::cout << "GameNetworking::Client GUID: " << clientGUID << std::endl;

	this->peerInterface = SLNet::RakPeerInterface::GetInstance();
	
	HOST_gameStateSnapshotStorage = std::make_shared<GameState>();
	CLIENT_snapshotStorageQueue.push_back(std::make_shared<GameState>());

	CLIENT_gameStateTransfer = std::make_shared<GameState>();



	std::cout << "GameNetworking::Init  End" << std::endl;
}

 void GameNetworking::CLIENT_SendActionUpdate_ToHost(std::vector<ActionWrap>& clientActions)
 {
	 if (actionStateDirty && fullyConnectedToHost) {
		 std::cout << "Sending ActionList Update to host " << hostPeer.ToString() << std::endl;


		 SLNet::BitStream bs;
		 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_ACTIONUPDATE));
		 bs.Write(static_cast<uint32_t>(clientActions.size()));
		
		 for (ActionWrap& actionWrap : clientActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
		 }
		 
		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, hostPeer, false);


		 actionStateDirty = false;
	 }
 }

 void GameNetworking::HOST_SendActionUpdates_ToClients()
 {
	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA to " << clients.size() << " clients" << std::endl;
	 

	 for (auto client : clients)
	 {
		 SLNet::BitStream bs;
		 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA));
		 bs.Write(static_cast<uint8_t>(clients.size()));
		 bs.Write(static_cast<uint8_t>(turnActions.size()));
		 for (ActionWrap& actionWrap : turnActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
		 }

		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client.rakGuid, false);
	 }
	 turnActions.clear();
 }

 void GameNetworking::HOST_SendReSync_ToClients()
 {
	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_RESYNC_NOTIFICATION to " << clients.size() << " clients" << std::endl;

	 for (const auto& client : clients)
	 {
		 SLNet::BitStream bs;
		 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_RESYNC_NOTIFICATION));
		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client.rakGuid, false);
	 }
 }

 inline void GameNetworking::CLIENT_SENDInitialData_ToHost()
 {
	 SLNet::BitStream bs;

	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_INITIALDATA));
	 bs.Write(static_cast<uint32_t>(clientGUID));

	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, hostPeer, false);
 }

 void GameNetworking::CLIENT_SendGamePartAck()
 {
	 SLNet::BitStream bs;

	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_GAMEDATA_PART_ACK));
	 bs.Write(static_cast<uint32_t>(clientGUID));

	 this->peerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE, 1, hostPeer, false);
 }

 inline void GameNetworking::CLIENT_SendSyncComplete()
 {
	 SLNet::BitStream bs;
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_SYNC_COMPLETE));
	 bs.Write(static_cast<uint32_t>(clientGUID));
	 this->peerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE, 1, hostPeer, false);
 }

 inline void GameNetworking::HOST_SendSyncStart_ToClient(int32_t cliIdx, SLNet::RakNetGUID clientAddr)
 {
	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_SYNCDATA1 To client ( ClientId: " << cliIdx << ")" << std::endl;
	 SLNet::BitStream bs;
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_SYNCDATA1));
	 bs.Write(static_cast<int32_t>(cliIdx));
	 bs.Write(CheckSumGameState(HOST_gameStateSnapshotStorage.get()));

	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, clientAddr, false);

	 HOST_snapshotLocked = true;
 }

 void GameNetworking::HOST_SendGamePart_ToClient(uint32_t clientGUID) {

	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_GAMEDATA_PART To client ( ClientGUID: " << clientGUID << ")" << std::endl;
	 clientMeta* client = GetClientMetaDataFromCliGUID(clientGUID);
	 SLNet::BitStream bs;
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_GAMEDATA_PART));

	 uint64_t gameStateSize = sizeof(GameState);
	 uint32_t chunkSize = TRANSFERCHUNKSIZE;
	 uint64_t n = HOST_nextTransferOffset[client->cliId] + chunkSize;
	 if (n >= gameStateSize)
		 chunkSize -= n - gameStateSize +1;

	 bs.Write(chunkSize);
	 
	 SLNet::DataCompressor compressor;
	 compressor.Compress(reinterpret_cast<unsigned char*>(HOST_gameStateSnapshotStorage.get()) + HOST_nextTransferOffset[client->cliId], chunkSize, &bs);

	 HOST_nextTransferOffset[client->cliId] += chunkSize;




	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client->rakGuid, false);
 }

 void GameNetworking::StartServer(int32_t port)
 {	
	std::cout << "GameNetworking, Starting Server.." << std::endl;

	SocketDesc.port = port;
	this->peerInterface->Startup(MaxStandardConnections, &SocketDesc, 1);
	this->peerInterface->SetMaximumIncomingConnections(MaxStandardConnections);

	//this->peerInterface->ApplyNetworkSimulator(0.5f, 1000000, 200);
	serverRunning = true;
 }

 inline void GameNetworking::SendTickSyncToHost()
 {
	 int32_t ping = peerInterface->GetAveragePing(hostPeer);
	 if (ping <= 0) ping = REALLYBIGPING;

	 SLNet::BitStream bs;
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC));
	 bs.Write(static_cast<int32_t>(clientId));
	 bs.Write(static_cast<uint32_t>(gameState->tickIdx));
	 bs.Write(static_cast<int32_t>(targetTickTimeMs));
	 bs.Write(static_cast<uint32_t>(clientGUID));
	 bs.Write(static_cast<int32_t>(peerInterface->GetAveragePing(hostPeer)));

	 this->peerInterface->Send(&bs, HIGH_PRIORITY, UNRELIABLE,
		 1, hostPeer, false);
 }

 inline void GameNetworking::SendTickSyncToClients()
 {
	 for (int i = 0; i < clients.size(); i++)
	 {
		 clientMeta* client = &clients[i];

		 SLNet::BitStream bs;
		 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_ROUTINE_TICKSYNC));
		 bs.Write(static_cast<uint8_t>(clients.size()));
		 for (int c = 0; c < clients.size(); c++)
		 {
			 bs.Write(reinterpret_cast<char*>(&clients[c]), sizeof(clientMeta));
		 }

		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client->rakGuid, false);
	 }
 }

 inline uint64_t GameNetworking::CheckSumGameState(GameState* state)
 {
	 uint64_t sum = 0;
	 uint8_t* bytePtr = reinterpret_cast<uint8_t*>(state);
	 for (uint64_t i = 0; i < sizeof(GameState); i++)
	 {
		 sum += bytePtr[i];
	 }
	 return sum;
 }

 void GameNetworking::ConnectToHost(SLNet::SystemAddress hostAddress)
{
	std::cout << "[CLIENT] Connecting To Host: " << hostAddress.ToString(false) << " Port: " << hostAddress.GetPort() << std::endl;
	SLNet::SocketDescriptor desc;
	if(!serverRunning)
		peerInterface->Startup(1, &desc, 1);
	
	int port = hostAddress.GetPort();
	if (port == 0)
		port = GAMESERVERPORT;

	SLNet::ConnectionAttemptResult connectInitSuccess = peerInterface->Connect(hostAddress.ToString(false), port, nullptr, 0, nullptr);
	if (connectInitSuccess != SLNet::CONNECTION_ATTEMPT_STARTED)
	{
		if(serverRunning && (connectInitSuccess != SLNet::ALREADY_CONNECTED_TO_ENDPOINT))
			std::cout << "[CLIENT] Connect Init Failure: " << int32_t(connectInitSuccess) << std::endl;
		else
		{
			if (serverRunning)//fake reconnect and start process.
			{
				std::cout << "[CLIENT] Fake Reconnecting.." << std::endl;
				connectedToHost = true;
				
				CLIENT_SENDInitialData_ToHost();
			}


		}
	}
	
}
 void GameNetworking::CLIENT_Disconnect()
 {
	 std::cout << "[CLIENT] Disconnecting..." << std::endl;
	 if (serverRunning)
	 {
		 //"fake" disconnect to self
		 peerInterface->CloseConnection(SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		 HOST_HandleDisconnectByID(clientId);
	 }
	 else
	 {
		 peerInterface->CloseConnection(hostPeer, true);
		 peerInterface->Shutdown(1000);

		 clients.clear();
	 }

	 clientId = -1;
	 connectedToHost = false;
	 fullyConnectedToHost = false;

 }
 void GameNetworking::SendMessage(char* message)
 {
	 SLNet::BitStream bs;
	 int32_t len = strlen(message);
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_GENERIC_MESSAGE));
	 bs.Write(static_cast<uint32_t>(len));

	 
	 if (len > MAX_USER_MESSAGE_LENGTH)
		 len = MAX_USER_MESSAGE_LENGTH;

	 bs.Write(message, static_cast<uint32_t>(len));


	 //broadcast to all
	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED,
		 1, SLNet::UNASSIGNED_RAKNET_GUID, true);
 }

 void GameNetworking::UpdateThrottling()
 {
	 clientMeta* thisClient = GetClientMetaDataFromCliGUID(clientGUID);
	 clientMeta serverDummyClientThing;
	 if (thisClient == nullptr)
	 {
		 //std::cout << "USING DUMMY CLIENT FOR THROTTLE" << std::endl;
		 thisClient = &serverDummyClientThing;
		 thisClient->hostTickOffset = 0;
		 thisClient->avgHostPing = 0;
	 }

	 //get stats on client swarm and slow down 
	 int32_t maxOffset = -9999;
	 int32_t minOffset = 9999;
	 int32_t safetyOffset = 5;

	 for (int i = 0; i < clients.size(); i++)
	 {
		 clientMeta* client = &clients[i];
		 if (client == thisClient)
			 continue;

		 if (client->hostTickOffset > maxOffset)
			 maxOffset = client->hostTickOffset;
		 if (client->hostTickOffset < minOffset)
			 minOffset = client->hostTickOffset;
	 }


	 safetyOffset += thisClient->avgHostPing / MINTICKTIMEMS;
	 int32_t distToMin = thisClient->hostTickOffset - minOffset;
	 int32_t distToMax = thisClient->hostTickOffset - maxOffset;

	 int32_t distToCompare;
	 if (serverRunning)
		 distToCompare = distToMax;
	 else
		 distToCompare = distToMin;

	 tickPIDError = distToCompare - safetyOffset;

	 //integralAccumulatorTickPID += 0.02 * error;

	 float pFactor = 8.0f;

	 if (clients.size() > 1 && fullyConnectedToHost)
	 {

		 if (!serverRunning && thisClient->hostTickOffset > -safetyOffset)
		 {
			 targetTickTimeMs = MINTICKTIMEMS * (thisClient->hostTickOffset - (-safetyOffset));
		 }
		 else
		 {

			 //slow down for fastest if server, slow down for slowest if client.
			 targetTickTimeMs = MINTICKTIMEMS + pFactor * (tickPIDError)+integralAccumulatorTickPID;
		 }



		 // clamp to min
		 if (thisClient->hostTickOffset < -safetyOffset * 10)
		 {
			 if (targetTickTimeMs <= 0)//alllow fastest speed up from very delayed clients.
				 targetTickTimeMs = 0;
		 }
		 else
		 {
			 if (targetTickTimeMs <= MINTICKTIMEMS)
				 targetTickTimeMs = MINTICKTIMEMS;
		 }


		 if (targetTickTimeMs >= MAXTICKTIMEMS)
			 targetTickTimeMs = MAXTICKTIMEMS;

	 }
	 else
	 {
		 targetTickTimeMs = MINTICKTIMEMS;
	 }

	 if (!fullyConnectedToHost)
		 targetTickTimeMs = 0;

 }
 void GameNetworking::UpdateHandleMessages()
 {

	 for (this->packet = this->peerInterface->Receive();
		 this->packet;
		 this->peerInterface->DeallocatePacket(this->packet),
		 this->packet = this->peerInterface->Receive())
	 {
		 SLNet::BitStream bts(this->packet->data,
			 this->packet->length,
			 false);


		 SLNet::RakNetGUID systemGUID = this->packet->guid;

		 // Check the packet identifier
		 switch (this->packet->data[0])
		 {
		 case ID_CONNECTION_REQUEST_ACCEPTED:
		 {
			 std::cout << "[CLIENT] Peer: ID_CONNECTION_REQUEST_ACCEPTED" << std::endl;
			 connectedToHost = true;

			 hostPeer = systemGUID;

			 CLIENT_SENDInitialData_ToHost();

		 }
		 break;
		 case ID_NEW_INCOMING_CONNECTION:
		 {
			 std::cout << "[HOST] Peer: A peer " << systemGUID.ToString()
				 << " has connected." << std::endl;



		 }
		 break;

		 case ID_USER_PACKET_ENUM:
			 uint8_t rcv_id;
			 bts.Read(rcv_id);

			 uint8_t msgtype;
			 bts.Read(msgtype);

			 if (msgtype == MESSAGE_ENUM_GENERIC_MESSAGE)
			 {
				 uint32_t length;
				 bts.Read(length);



				 char message[MAX_USER_MESSAGE_LENGTH];

				 bts.Read(message, length);
				 message[length] = '\0';
				 std::cout << "Peer: MESSAGE_ENUM_GENERIC_MESSAGE received: " << message << std::endl;
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_INITIALDATA)
			 {

				 uint32_t clientGUID;
				 bts.Read(clientGUID);

				 std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_INITIALDATA received from ClientGUID: " << clientGUID << std::endl;


				 SHARED_CLIENT_CONNECT(clientGUID, systemGUID);
			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_SYNCDATA1)
			 {
				 bts.Read(this->clientId);
				 std::cout << "[CLIENT] CLientId is now " << this->clientId << std::endl;


				 bts.Read(transferFullCheckSum);

				 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_SYNCDATA1 received" << std::endl;


			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_GAMEDATA_PART)
			 {
				 uint32_t chunkSize;
				 bts.Read(chunkSize);

				 SLNet::DataCompressor compressor;
				 compressor.Decompress(&bts, reinterpret_cast<unsigned char*>(CLIENT_gameStateTransfer.get()) + CLIENT_nextTransferOffset);



				 CLIENT_nextTransferOffset += chunkSize;
				 gameStateTransferPercent = (float(CLIENT_nextTransferOffset) / sizeof(GameState));
				 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_GAMEDATA_PART: " << 100 * gameStateTransferPercent << "%" << std::endl;


				 if (chunkSize < TRANSFERCHUNKSIZE)
				 {
					 if (transferFullCheckSum != CheckSumGameState(CLIENT_gameStateTransfer.get()))
					 {
						 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_GAMEDATA_PART received CHECKSUM MISSMATCH!" << std::endl;
						 std::cout << "Correct SUM: " << transferFullCheckSum << ", CHECKSUM: " << CheckSumGameState(CLIENT_gameStateTransfer.get()) << std::endl;

						 assert(0);
					 }
					 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_GAMEDATA_PART final GameState part Recieved, sending acknologement." << std::endl;
					 memcpy(gameState.get(), CLIENT_gameStateTransfer.get(), sizeof(GameState));

					 gameState->pauseState = 0;
					 CLIENT_nextTransferOffset = 0;
					 gameStateTransferPercent = 0.0f;
					 CLIENT_SendSyncComplete();
					 fullyConnectedToHost = true;
				 }
				 else
					 CLIENT_SendGamePartAck();
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_GAMEDATA_PART_ACK)
			 {
				 uint32_t clientGUID;
				 bts.Read(clientGUID);

				 HOST_SendGamePart_ToClient(clientGUID);
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_SYNC_COMPLETE)
			 {
				 std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_SYNC_COMPLETE received, Resuming sim." << std::endl;
				 std::cout << "un-pausing." << std::endl;

				 uint32_t clientGUID;
				 bts.Read(clientGUID);

				 clientMeta* client = GetClientMetaDataFromCliGUID(clientGUID);
				 client->downloadingState = 0;
				 gameState->pauseState = 0;
				 HOST_snapshotLocked = false;

				 HOST_nextTransferOffset[client->cliId] = 0;
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_ACTIONUPDATE)
			 {

				 uint32_t numActions;
				 bts.Read(numActions);

				 std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_ACTIONUPDATE received (" << int32_t(numActions) << " actions)" << std::endl;


				 for (int32_t i = 0; i < numActions; i++)
				 {
					 ActionWrap actionWrap;
					 bts.Read(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));

					 int32_t tickLatency = (gameState->tickIdx - actionWrap.action.submittedTickIdx);


					 actionWrap.action.scheduledTickIdx = gameState->tickIdx + 0;
					 turnActions.push_back(actionWrap);
					 runningActions.push_back(actionWrap);
					 freezeFrameActions.push_back(actionWrap);
				 }





				 //all clients have given input
				 //send combined final turn to all clients
				 HOST_SendActionUpdates_ToClients();

			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA)
			 {

				 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA received" << std::endl;

				 //get info on how many total clients there are...
				 uint8_t numClients;
				 bts.Read(numClients);
				 gameState->numClients = numClients;


				 uint8_t numActions;
				 bts.Read(numActions);


				 for (uint8_t a = 0; a < numActions; a++)
				 {
					 ActionWrap actionWrap;
					 bts.Read(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
					 turnActions.push_back(actionWrap);
				 }
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_ACTION_ERROR)
			 {
				 uint32_t clientGUID;
				 bts.Read(clientGUID);
				 clientMeta* client = GetClientMetaDataFromCliGUID(clientGUID);
				 if (client != nullptr)
				 {
					 ActionTracking actionTracking;
					 bts.Read(reinterpret_cast<char*>(&actionTracking), sizeof(ActionTracking));
					 std::cout << "[HOST] Recieved Expired Action Error from client: " << clientGUID << " (Throttling)" << std::endl;

					 memcpy(gameState.get(), HOST_gameStateSnapshotStorage.get(), sizeof(GameState));

					 HOST_SendReSync_ToClients();
				 }
				 else
				 {
					 assert(0);
				 }
				

			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC)
			 {
				 int32_t cliId;
				 bts.Read(cliId);


				 uint32_t client_tickIdx;
				 bts.Read(client_tickIdx);

				 int32_t tickTime;
				 bts.Read(tickTime);

				 uint32_t clientGUID;
				 bts.Read(clientGUID);


				 int32_t ping;
				 bts.Read(ping);


				 int32_t offset = int32_t(client_tickIdx) - int32_t(gameState->tickIdx);

				 clientMeta* meta = GetClientMetaDataFromCliGUID(clientGUID);
				 if (meta != nullptr)
				 {
					 meta->avgHostPing = ping;
					 meta->hostTickOffset = offset;
					 meta->ticksSinceLastCommunication = 0;

				 }
				 else
				 {
					 std::cout << "[HOST] recieved MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC but no entry for GUID:" << clientGUID << " in client list!" << std::endl;
				 }
			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_ROUTINE_TICKSYNC)
			 {
				 uint8_t numClients;
				 bts.Read(numClients);

				 std::vector<clientMeta> clientList;
				 for (int i = 0; i < numClients; i++)
				 {
					 clientMeta clientData;
					 bts.Read(reinterpret_cast<char*>(&clientData), sizeof(clientMeta));
					 //invalidate stuff useless to clients
					 clientData.rakGuid = SLNet::RakNetGUID();
					 clientList.push_back(clientData);
				 }
				 if (!serverRunning)
					 clients = clientList;

			 }
			 break;
		 case ID_REMOTE_CONNECTION_LOST:
			 //std::cout << "Peer: Remote peer lost connection.(untrackable client at this point)" << std::endl;

			 break;
		 case ID_DISCONNECTION_NOTIFICATION:
			 //std::cout << "Peer: A peer has disconnected. (untrackable client at this point)" << std::endl;

			 break;
		 case ID_CONNECTION_LOST:
			 //std::cout << "Peer: A connection was lost.(untrackable client at this point)" << std::endl;

			 break;
		 case ID_CONNECTION_ATTEMPT_FAILED:
			 std::cout << "Peer: A connection failed." << std::endl;
			 break;
		 default:

			 std::cout << "Peer: Received a packet with unspecified message identifier: " << int32_t(this->packet->data[0]) << std::endl;


			 break;
		 } // check package identifier
	 } // package receive loop
 }
 void GameNetworking::Update()
{

	if(connectedToHost && fullyConnectedToHost)
		SendTickSyncToHost();

	if (serverRunning)
		SendTickSyncToClients();

	if (gameState->tickIdx % freezeFreq == 0 && !HOST_snapshotLocked)
	{
		if (serverRunning)
		{
			memcpy(reinterpret_cast<void*>(HOST_gameStateSnapshotStorage.get()),
				reinterpret_cast<void*>(gameState.get()), sizeof(GameState));
		}

		
		while (CLIENT_snapshotStorageQueue.size() > 4)
		{
			CLIENT_snapshotStorageQueue.erase(CLIENT_snapshotStorageQueue.begin());
		}
		CLIENT_snapshotStorageQueue.push_back(std::make_shared<GameState>());


		memcpy(reinterpret_cast<void*>(CLIENT_snapshotStorageQueue.back().get()),
			reinterpret_cast<void*>(gameState.get()), sizeof(GameState));




		freezeFrameActions_1 = freezeFrameActions;
		freezeFrameActions.clear();
	}
	if (serverRunning)
	{



		//check if running actions are accounted for.
		

		//handle timeout disconnects.
		for (int i = 0; i < clients.size(); i++)
		{
			if(clients[i].downloadingState == 0)
				clients[i].ticksSinceLastCommunication++;

			
		}		
		for (int i = 0; i < clients.size(); i++)
		{
			if (clients[i].ticksSinceLastCommunication > 200)
			{
				std::cout << "[HOST] Timeout from clientGUID: " << clients[i].clientGUID
					<< " Timeout: " << clients[i].ticksSinceLastCommunication << std::endl;
				HOST_HandleDisconnectByCLientGUID(clients[i].clientGUID);
			}
		}
	}
	
	UpdateThrottling();

	UpdateHandleMessages();
}

 void GameNetworking::HOST_HandleDisconnectByCLientGUID(uint32_t clientGUID)
{
	std::cout << "[HOST] Removing Client Entry clientGUID: " << clientGUID << std::endl;
	int removeIdx = -1;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].clientGUID == clientGUID)
		{
			removeIdx = i;
			gameState->clientStates[clientId].connected = false;
		}
	}
	if (removeIdx >= 0)
	{
		clients.erase(std::next(clients.begin(), removeIdx));

	}
	else
		assert(0);
}

 void GameNetworking::HOST_HandleDisconnectByID(int32_t clientID)
{
	std::cout << "[HOST] Removing Client Entry clientID: " << clientID << std::endl;
	int removeIdx = -1;
	for (int i = 0; i < clients.size(); i++)
	{
		if (clients[i].cliId == clientID)
		{
			removeIdx = i;
			gameState->clientStates[clientId].connected = false;
		}
	}
	if (removeIdx >= 0)
		clients.erase(std::next(clients.begin(), removeIdx));
	else
		assert(0);
}

 void GameNetworking::SHARED_CLIENT_CONNECT(uint32_t clientGUID, SLNet::RakNetGUID systemGUID)
{
	AddClientInternal(clientGUID, systemGUID);
	connectedToHost = true;
	std::cout << "Pausing." << std::endl;
	gameState->pauseState = 1;
	clients.back().downloadingState = 1;
	

	HOST_SendSyncStart_ToClient(clients.back().cliId, systemGUID);
	HOST_SendGamePart_ToClient(clients.back().clientGUID);
}

void GameNetworking::AddClientInternal(uint32_t clientGUID, SLNet::RakNetGUID rakguid)
{
	clientMeta meta;
	meta.cliId = nextCliIdx;
	meta.rakGuid = rakguid;
	meta.clientGUID = clientGUID;
	meta.hostTickOffset = 0;
	meta.avgHostPing = REALLYBIGPING;
	meta.ticksSinceLastCommunication = 0;
	meta.downloadingState = 0;
	clients.push_back(meta);
	gameState->clientStates[meta.cliId].connected = 1;

	nextCliIdx++;
}

GameNetworking::clientMeta* GameNetworking::GetClientMetaDataFromCliId(uint8_t cliId)
{
	for (int32_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].cliId == cliId)
			return &clients[i];
	}

	return nullptr;
}

GameNetworking::clientMeta* GameNetworking::GetClientMetaDataFromCliGUID(uint32_t cliGUID)
{
	for (int32_t i = 0; i < clients.size(); i++)
	{
		if (clients[i].clientGUID == cliGUID)
			return &clients[i];
	}

	return nullptr;
}
