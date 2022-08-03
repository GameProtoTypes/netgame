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
	


	CLIENT_gameStateTransfer = std::make_shared<GameState>();
	HOST_gameStateTransfer = std::make_shared<GameState>();

	snapshotWrap wrap;
	wrap.gameState = std::make_shared<GameState>();
	wrap.gameStateB = std::make_shared<GameStateB>();

	memcpy(wrap.gameState.get(), gameState.get(), sizeof(GameState));
	memcpy(wrap.gameStateB.get(), gameStateB.get(), sizeof(GameStateB));

	CLIENT_snapshotStorageQueue.push_back(wrap);



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

 void GameNetworking::HOST_SendActionUpdates_ToClients(const std::vector<ActionWrap>& actions)
 {
	 if (actions.size() == 0)
		 return;
	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA to " << clients.size() << " clients ("<< actions.size() << " actions)" << std::endl;
	 

	 for (auto client : clients)
	 {
		 SLNet::BitStream bs;
		 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA));
		 bs.Write(static_cast<uint8_t>(clients.size()));
		 bs.Write(static_cast<uint8_t>(actions.size()));
		 for (ActionWrap actionWrap : actions)
		 {
			 bs.Write(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
		 }

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


 void GameNetworking::CLIENT_ApplyActions()
 {
	 bool repeat = false;
	 do {
		 repeat = false;
		 //std::cout << "CLIENT_ApplyActions" << std::endl;
		 std::vector<int32_t> removals;
		 std::vector<ActionWrap> newList;

		 int snapShotIdx = CLIENT_snapshotStorageQueue.size() - 1;
		 std::vector<ActionWrap>& actionSack = CLIENT_actionList;

		 //first pass check all turns for lateness
		 uint32_t earliestExpiredScheduledTick = static_cast<uint32_t>(-1);
		 for (int32_t b = 0; b < actionSack.size(); b++)
		 {
			 if (actionSack[b].tracking.clientApplied)
				 continue;

			 ClientAction* action = &actionSack[b].action;
			 ActionTracking* actTracking = &actionSack[b].tracking;
			 if (action->scheduledTickIdx < gameStateB->tickIdx)
			 {
				 if (action->scheduledTickIdx < earliestExpiredScheduledTick)
					 earliestExpiredScheduledTick = action->scheduledTickIdx;
			 }
		 }

		 if (earliestExpiredScheduledTick != static_cast<uint32_t>(-1))
		 {
			 int32_t ticksLate = gameStateB->tickIdx - earliestExpiredScheduledTick;

			 std::cout << ClientConsolePrint() << "Expired Action " << ticksLate << " Ticks Late" << std::endl;
			 std::cout << ClientConsolePrint() << "Going Back To Last Snapshot to catchback up." << std::endl;

			 
			 while (snapShotIdx >= 0 && CLIENT_snapshotStorageQueue[snapShotIdx].gameStateB->tickIdx > earliestExpiredScheduledTick)
			 {
				 std::cout << ClientConsolePrint() << "Snapshot not far enough back, trying previous snapshot.." << std::endl;
				 snapShotIdx--;
			 }

			 if (snapShotIdx >= 0)
			 {
				 std::cout << ClientConsolePrint() << "Using Snapshot Idx: " << snapShotIdx << std::endl;

				 memcpy(gameState.get(), CLIENT_snapshotStorageQueue[snapShotIdx].gameState.get(), sizeof(GameState));
				 memcpy(gameStateB.get(), CLIENT_snapshotStorageQueue[snapShotIdx].gameStateB.get(), sizeof(GameStateB));//not needed?


				 gameCompute->WriteFullGameState();

				 for (int i = 0; i < actionSack.size(); i++)
				 {
					 if (actionSack[i].action.scheduledTickIdx >= gameStateB.get()->tickIdx)
						 actionSack[i].tracking.clientApplied = false;

					 if (actionSack[i].action.scheduledTickIdx == gameStateB.get()->tickIdx)
					 {	
						 std::cout << ClientConsolePrint()  << "RARE tickIdx equality case..";
						 repeat = true;
					 }
				 }
			 }
			 else
			 {
				 std::cout << ClientConsolePrint() << "All is lost!  Disconnecting..." << std::endl;
				 CLIENT_HardDisconnect();
				 return;
			 }
		 }


		 //at this point game state may be reverted from above.

		 int32_t i = 0;
		 for (int32_t a = 0; a < actionSack.size(); a++)
		 {
			 if (actionSack[a].tracking.clientApplied)
				 continue;

			 ClientAction* action = &actionSack[a].action;
			 ActionTracking* actTracking = &actionSack[a].tracking;
			 if (action->scheduledTickIdx == gameStateB->tickIdx)
			 {

				 std::cout << "[ACTIONTRACK]" << ClientConsolePrint() << " Using Action: ACS: [" << CheckSumAction(&actionSack[a]) 
					 << "]" << std::endl;

				 actionSack[a].tracking.clientApplied = true;
				 gameStateB->clientActions[i] = actionSack[a];

				 i++;
			 }
			 else
			 {
				 newList.push_back(actionSack[a]);
			 }
		 }
		 gameStateB->numActions = i;
	 } while (repeat);

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
	 bs.Write(reinterpret_cast<char*>(gameStateB.get()), sizeof(GameStateB));
	 bs.Write(CheckSumGameState(HOST_gameStateTransfer.get()));

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
	 compressor.Compress(reinterpret_cast<unsigned char*>(HOST_gameStateTransfer.get()) + HOST_nextTransferOffset[client->cliId], chunkSize, &bs);

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
	 bs.Write(static_cast<uint32_t>(gameStateB->tickIdx));
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

 uint64_t GameNetworking::CheckSumAction(ActionWrap* state)
 {
	 uint64_t sum = 0;
	 uint8_t* bytePtr = reinterpret_cast<uint8_t*>(state);
	 for (uint64_t i = 0; i < sizeof(ActionWrap); i++)
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
 void GameNetworking::CLIENT_HardDisconnect()
 {
	 std::cout << "[CLIENT] Hard Disconnecting..." << std::endl;
	 ClientDisconnectRoutines();

 }

 void GameNetworking::ClientDisconnectRoutines()
 {
	 if (serverRunning)
	 {
		 //"fake" disconnect to self
		 peerInterface->CloseConnection(SLNet::UNASSIGNED_SYSTEM_ADDRESS, true);

		 HOST_HandleDisconnectByID(clientId);
	 }
	 else
	 {
		 peerInterface->CloseConnection(hostPeer, true);


		 clients.clear();
	 }

	 clientId = -1;
	 connectedToHost = false;
	 fullyConnectedToHost = false;
 }
 void GameNetworking::CLIENT_SoftDisconnect()
 {
	 std::cout << ClientConsolePrint() << " Soft Disconnecting. Sending MESSAGE_ENUM_CLIENT_DISCONNECT_NOTIFY." << std::endl;


	 //send disconnect notify first to host then disconnect will happen on host notify
	 SLNet::BitStream bs;
	 bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_CLIENT_DISCONNECT_NOTIFY));
	 bs.Write(static_cast<uint32_t>(clientGUID));

	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, hostPeer, false);
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
	 if (thisClient == nullptr)//case for uncconected to server
	 {
		 targetTickTimeMs = MINTICKTIMEMS;
		 return;
	 }
	 if (clients.size() == 1 && fullyConnectedToHost)//case for client/server hybrid with no extra clients.
	 {
		 targetTickTimeMs = MINTICKTIMEMS;
		 return;
	 }

	 //get stats on client swarm and slow down 
	 int32_t maxOffset = -9999;
	 int32_t minOffset = 9999;
	 int32_t safetyOffset = 10;

	 for (int i = 0; i < clients.size(); i++)
	 {
		 clientMeta* client = &clients[i];
		 if (client == thisClient)
		 {
			 continue;//dont compare to self.
		 }

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

	 tickPIDError = distToCompare;

	 
	 float pFactor = 4.0f;
	 
	 if (clients.size() > 1 && fullyConnectedToHost)
	 {


	     //slow down for fastest if server-hybrid, slow down for slowest if client.

		 if (serverRunning)
		 {
			 tickPIDError -= safetyOffset;
		 }
		 else
		 {
			 tickPIDError += safetyOffset;
		 }

		targetTickTimeMs = GOODTICKTIMEMS + pFactor * (tickPIDError);//A





		if (targetTickTimeMs >= MAXTICKTIMEMS)
		{
			targetTickTimeMs = MAXTICKTIMEMS;
		}
		else if (targetTickTimeMs <= 0)//allow fastest speed up from very delayed clients.
		{
 			targetTickTimeMs = 0;

		}
	 }
	 else if (serverRunning && !fullyConnectedToHost && clients.size() >= 1)//Server only with at least a client
	 {
		 targetTickTimeMs = MINTICKTIMEMS + pFactor * (tickPIDError);//used?

	 }
	 else
	 {
		 targetTickTimeMs = MINTICKTIMEMS;

	 }
	 


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


				 AddClientInternal(clientGUID, systemGUID);
				 
				 if(clientGUID == this->clientGUID)
					connectedToHost = true;//for hybrid

				 clients.back().downloadingState = 1;

				 gameCompute->ReadFullGameState();

				 memcpy(HOST_gameStateTransfer.get(), gameState.get(), sizeof(GameState));

				 std::cout << "Pausing." << std::endl;
				 

				 HOST_SendSyncStart_ToClient(clients.back().cliId, systemGUID);
				 gameStateB->pauseState = 1;

				 HOST_SendGamePart_ToClient(clients.back().clientGUID);
			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_SYNCDATA1)
			 {
				 bts.Read(this->clientId);
				 std::cout << "[CLIENT] CLientId is now " << this->clientId << std::endl;


				 bts.Read(reinterpret_cast<char*>(gameStateB.get()), sizeof(GameStateB));
				 gameStateB->pauseState = 1;
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
					 uint64_t recievedCS = CheckSumGameState(CLIENT_gameStateTransfer.get());

					 if (transferFullCheckSum != recievedCS)
					 {
						 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_GAMEDATA_PART received CHECKSUM MISSMATCH!" << std::endl;
						 std::cout << "Correct SUM: " << transferFullCheckSum << ", CHECKSUM: " << recievedCS << std::endl;

						 assert(0);
					 }
					 std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_GAMEDATA_PART final GameState part Recieved, sending acknologement." << std::endl;
					 memcpy(gameState.get(), CLIENT_gameStateTransfer.get(), sizeof(GameState));
					 gameCompute->WriteFullGameState();
					 
					 gameStateB->pauseState = 0;
					 gameStateB->clientId = clientId;
					 std::cout << "[CLIENT] GSCS: " << recievedCS << std::endl;

					 //also add as a snapshot
					 snapshotWrap wrap;
					 wrap.gameState = std::make_shared<GameState>();
					 wrap.gameStateB = std::make_shared<GameStateB>();

					 CLIENT_snapshotStorageQueue.push_back(wrap);
					 memcpy(CLIENT_snapshotStorageQueue.back().gameState.get(), gameState.get(), sizeof(GameState));
					 memcpy(CLIENT_snapshotStorageQueue.back().gameStateB.get(), gameStateB.get(), sizeof(GameStateB));




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
				 gameStateB->pauseState = 0;
				 HOST_snapshotLocked = false;

				 HOST_nextTransferOffset[client->cliId] = 0;

				 std::cout << "[HOST] GSCS: " << CheckSumGameState(gameState.get()) << std::endl;
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_ACTIONUPDATE)
			 {

				 uint32_t numActions;
				 bts.Read(numActions);

				 std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_ACTIONUPDATE received (" << int32_t(numActions) << " actions)" << std::endl;

				 //schedule and send out actions right away.
				 std::vector<ActionWrap> actions;
				 if (gameStateB.get()->pauseState == 0) {
					 for (int32_t i = 0; i < numActions; i++)
					 {
						 ActionWrap actionWrap;
						 bts.Read(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));


						 if (gameStateB->tickIdx > HOST_lastActionScheduleTickIdx)//ignore actions if client is in catchup. and enforce actions all on different ticks
						 {
							 actionWrap.action.scheduledTickIdx = gameStateB->tickIdx + 0;
							 HOST_lastActionScheduleTickIdx = actionWrap.action.scheduledTickIdx;
							 actions.push_back(actionWrap);
						 }
					 }
				 }
				 //all clients have given input
				 //send combined final turn to all clients
				 HOST_SendActionUpdates_ToClients(actions);

			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA)
			 {

				 std::cout << ClientConsolePrint() << "MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA received" << std::endl;

				 //get info on how many total clients there are...
				 uint8_t numClients;
				 bts.Read(numClients);


				 uint8_t numActions;
				 bts.Read(numActions);


				 for (uint8_t a = 0; a < numActions; a++)
				 {
					 ActionWrap actionWrap;
					 bts.Read(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));

					 std::cout << ClientConsolePrint() << "Adding action scheduled for tick " << actionWrap.action.scheduledTickIdx << std::endl;

					 //add action to the current snapshot postactions
					 CLIENT_actionList.push_back(actionWrap);
				 }
			 }
			 else if (msgtype == MESSAGE_ENUM_HOST_DISCONNECTED_NOTIFY)
			 {
				 uint32_t clientGUID;
				 bts.Read(clientGUID);

				 if (clientGUID == this->clientGUID)
				 {
					 std::cout << ClientConsolePrint() << " Recieved MESSAGE_ENUM_HOST_DISCONNECTED_NOTIFY for Self. Disconnected From Host." << std::endl;

					 //no longer connected.  clean up nessecary stuff.	 
					 ClientDisconnectRoutines();
				 }
			 }
			 else if (msgtype == MESSAGE_ENUM_CLIENT_DISCONNECT_NOTIFY)
			 {
				uint32_t clientGUID;
				bts.Read(clientGUID);

				std::cout << HostConsolePrint() << " Received MESSAGE_ENUM_CLIENT_DISCONNECT_NOTIFY for client " << clientGUID << std::endl;

				HOST_HandleDisconnectByCLientGUID(clientGUID);
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


				 int32_t offset = int32_t(client_tickIdx) - int32_t(gameStateB->tickIdx);

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




	if (gameStateB->tickIdx % snapshotFreq == 0 && !HOST_snapshotLocked)
	{
		/*
		while (CLIENT_snapshotStorageQueue.size() > 1)
		{
			CLIENT_snapshotStorageQueue.erase(CLIENT_snapshotStorageQueue.begin());
		}
		snapshotWrap wrap;
		wrap.gameState = std::make_shared<GameState>();
		wrap.gameStateB = std::make_shared<GameStateB>();
		CLIENT_snapshotStorageQueue.push_back(wrap);
		std::cout << "Snapshot Taken" << std::endl;

		memcpy(reinterpret_cast<void*>(CLIENT_snapshotStorageQueue.back().gameState.get()),
			reinterpret_cast<void*>(gameState.get()), sizeof(GameState));
		memcpy(reinterpret_cast<void*>(CLIENT_snapshotStorageQueue.back().gameStateB.get()),
			reinterpret_cast<void*>(gameStateB.get()), sizeof(GameStateB));

		//CLIENT_snapshotStorageQueue.back().checksum = CheckSumGameState(CLIENT_snapshotStorageQueue.back().gameState.get());
		*/
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

	if (fullyConnectedToHost)
	{

		CLIENT_ApplyActions();
	}
}

 void GameNetworking::HOST_HandleDisconnectByCLientGUID(uint32_t clientGUID)
{
	std::cout << "[HOST] Broadcasting Client " << clientGUID << "Disconnect Notification Before Removal : " << std::endl;
	clientMeta* client = GetClientMetaDataFromCliGUID(clientGUID);

	SLNet::BitStream bs;
	bs.Write(static_cast<uint8_t>(ID_USER_PACKET_ENUM));
	bs.Write(static_cast<uint8_t>(MESSAGE_ENUM_HOST_DISCONNECTED_NOTIFY));
	bs.Write(static_cast<uint32_t>(clientGUID));
	
	for (auto meta : clients)
	{
		this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, meta.rakGuid, false);
	}


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
