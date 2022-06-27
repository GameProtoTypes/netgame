#include "GameNetworking.h"

 void GameNetworking::Init()
{
	std::cout << "GameNetworking::Init" << std::endl;

	this->peerInterface = SLNet::RakPeerInterface::GetInstance();

	lastFreezeGameState = new GameState();


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
		 bs.Write(static_cast<int32_t>(client.cliId));
		 bs.Write(static_cast<uint8_t>(turnActions.size()));
		 for (ActionWrap& actionWrap : turnActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
		 }

		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client.rakGuid, false);
	 }
	 turnActions.clear();
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

 void GameNetworking::ConnectToHost(SLNet::SystemAddress hostAddress)
{
	std::cout << "[CLIENT] Connecting To Host: " << hostAddress.ToString(false) << " Port: " << hostAddress.GetPort() << std::endl;
	SLNet::SocketDescriptor desc;
	if(!serverRunning)
		peerInterface->Startup(1, &desc, 1);
	

		SLNet::ConnectionAttemptResult connectInitSuccess = peerInterface->Connect(hostAddress.ToString(false), hostAddress.GetPort(), nullptr, 0, nullptr);
		if (connectInitSuccess != SLNet::CONNECTION_ATTEMPT_STARTED)
		{
			if(serverRunning && (connectInitSuccess != SLNet::ALREADY_CONNECTED_TO_ENDPOINT))
				std::cout << "[CLIENT] Connect Init Failure: " << int32_t(connectInitSuccess) << std::endl;
			else
			{
				SHARED_CLIENT_CONNECT(hostPeer);

			}
		}
	
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


 void GameNetworking::Update()
{
	if(connectedToHost && fullyConnectedToHost)
		SendTickSyncToHost();

	//if (serverRunning)
	//	SendTickSyncToClients();


	if (serverRunning)
	{

		if (gameState->tickIdx % freezeFreq == 0)
		{
			memcpy(reinterpret_cast<void*>(lastFreezeGameState), 
				reinterpret_cast<void*>(gameState), sizeof(GameState));

			freezeFrameActions_1 = freezeFrameActions;
			freezeFrameActions.clear();
		}

		//check if running actions are accounted for.
		



		//slow down simulation to bring in client 
		
		//if (maxTickLag > 2)
		//	minTickTimeMs = 66;
		//else
		//	minTickTimeMs = 33;


	}


	for (this->packet = this->peerInterface->Receive();
		this->packet;
		this->peerInterface->DeallocatePacket(this->packet),
		this->packet = this->peerInterface->Receive())
	{
		SLNet::BitStream bts(this->packet->data,
			this->packet->length,
			false);


		SLNet::RakNetGUID systemGUID = peerInterface->GetGuidFromSystemAddress(this->packet->systemAddress);

		// Check the packet identifier
		switch (this->packet->data[0])
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			std::cout << "[CLIENT] Peer: ID_CONNECTION_REQUEST_ACCEPTED" << std::endl;
			connectedToHost = true;

			if (serverRunning)
			{
				SHARED_CLIENT_CONNECT(systemGUID);
			}

			hostPeer = systemGUID;

		}
			break;
		case ID_NEW_INCOMING_CONNECTION:
		{
			std::cout << "[HOST] Peer: A peer " << systemGUID.ToString()
				<< " has connected." << std::endl;


			Host_AddClientInternal(systemGUID);


			HOST_SendSync_ToClient(clients.back().cliId, systemGUID);

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
			else if (msgtype == MESSAGE_ENUM_HOST_SYNCDATA1)
			{



				bts.Read(this->clientId);
				
				std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_SYNCDATA1 received, CLientId is now " << this->clientId <<", sending acknologement." << std::endl;


				bool s = bts.Read(reinterpret_cast<char*>(gameState), sizeof(GameState));
				if (!s) assert(0);



				CLIENT_SendSyncComplete();

				fullyConnectedToHost = true;
			}
			else if (msgtype == MESSAGE_ENUM_CLIENT_SYNC_COMPLETE)
			{
				std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_SYNC_COMPLETE received, Resuming sim." << std::endl;

				gameState->pauseState = 0;
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

				//sync client id as assigned by the host.
				int32_t cliId;
				bts.Read(cliId);
				this->clientId = cliId;


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
				ActionTracking actionTracking;
				bts.Read(reinterpret_cast<char*>(&actionTracking), sizeof(ActionTracking));
				std::cout << "[HOST] Recieved Expired Action Error " << std::endl;
				std::cout << "[HOST] REVERTING........" << std::endl;
			}
			else if (msgtype == MESSAGE_ENUM_CLIENT_ROUTINE_TICKSYNC)
			{
				int32_t cliId;
				bts.Read(cliId);


				uint32_t client_tickIdx;
				bts.Read(client_tickIdx);

				int32_t tickTime;
				bts.Read(tickTime);

				int32_t offset = int32_t(client_tickIdx) - int32_t(gameState->tickIdx);

				clientMeta* meta = GetClientMetaDataFromCliId(cliId);
				meta->hostTickOffset = offset;
			}

			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Peer: Remote peer lost connection." << std::endl;
			HOST_HandleDisconnectByGUID(systemGUID);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "Peer: A peer has disconnected." << std::endl;
			HOST_HandleDisconnectByGUID(systemGUID);

			break;
		case ID_CONNECTION_LOST:
			std::cout << "Peer: A connection was lost." << std::endl;

			HOST_HandleDisconnectByGUID(systemGUID);
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
