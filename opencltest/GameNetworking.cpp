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
	 if (actionStateDirty) {
		 std::cout << "Sending ActionList Update to host " << hostPeer.ToString() << std::endl;


		 SLNet::BitStream bs;
		 bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_CLIENT_ACTIONUPDATE));
		 bs.Write(static_cast<unsigned int>(clientActions.size()));
		
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
		 bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_HOST_ACTION_SCHEDULE_DATA));
		 bs.Write(static_cast<unsigned char>(clients.size()));
		 bs.Write(static_cast<unsigned char>(client.cliId));
		 bs.Write(static_cast<unsigned char>(turnActions.size()));
		 for (ActionWrap& actionWrap : turnActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));
		 }

		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client.rakGuid, false);
	 }
	 turnActions.clear();
 }

 void GameNetworking::StartServer(int port)
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
	peerInterface->Startup(1, &desc, 1);
	SLNet::ConnectionAttemptResult connectInitSuccess = peerInterface->Connect(hostAddress.ToString(false), hostAddress.GetPort(), nullptr, 0, nullptr);
	

	if (connectInitSuccess != SLNet::CONNECTION_ATTEMPT_STARTED)
	{
		std::cout << "[CLIENT] Connect Init Failure: " << int(connectInitSuccess) << std::endl;


	}
}
 void GameNetworking::SendMessage(char* message)
 {
	 SLNet::BitStream bs;
	 int32_t len = strlen(message);
	 bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
	 bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_GENERIC_MESSAGE));
	 bs.Write(static_cast<unsigned int>(len));

	 
	 if (len > MAX_USER_MESSAGE_LENGTH)
		 len = MAX_USER_MESSAGE_LENGTH;

	 bs.Write(message, static_cast<unsigned int>(len));


	 //broadcast to all
	 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED,
		 1, SLNet::UNASSIGNED_RAKNET_GUID, true);
 }


 void GameNetworking::Update()
{
	if(connectedToHost)
		SendTickSyncToHost();



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
		
		if (maxTickLag > 2)
			minTickTimeMs = 66;
		else
			minTickTimeMs = 33;


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
				Host_AddClientInternal(systemGUID);

				gameState->pauseState = 1;
				HOST_SendSync_ToClient(clients.size() - 1, systemGUID);
				
			}

			hostPeer = systemGUID;

		}
			break;
		case ID_NEW_INCOMING_CONNECTION:
		{
			std::cout << "[HOST] Peer: A peer " << systemGUID.ToString()
				<< " has connected." << std::endl;


			Host_AddClientInternal(systemGUID);


			HOST_SendSync_ToClient(clients.size() - 1, systemGUID);

		}
			break;
			
		case ID_USER_PACKET_ENUM:
			unsigned char rcv_id;
			bts.Read(rcv_id);

			unsigned char msgtype;
			bts.Read(msgtype);



				

			if (msgtype == MESSAGE_ENUM_GENERIC_MESSAGE)
			{	
				unsigned int length;
				bts.Read(length);

				
				
				char message[MAX_USER_MESSAGE_LENGTH];

				bts.Read(message, length);
				message[length] = '\0';
				std::cout << "Peer: MESSAGE_ENUM_GENERIC_MESSAGE received: " << message << std::endl;
			}
			else if (msgtype == MESSAGE_ENUM_HOST_SYNCDATA1)
			{


				std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_SYNCDATA1 received, sending acknologement." << std::endl;

				bts.Read(clientId);
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

				unsigned int numActions;
				bts.Read(numActions);

				std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_ACTIONUPDATE received (" << int(numActions) << " actions)" << std::endl;


				for (int i = 0; i < numActions; i++)
				{
					ActionWrap actionWrap;
					bts.Read(reinterpret_cast<char*>(&actionWrap), sizeof(ActionWrap));

					int tickLatency = (gameState->tickIdx - actionWrap.action.submittedTickIdx);
						

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
				unsigned char numClients;
				bts.Read(numClients);
				gameState->numClients = numClients;

				//sync client id as assigned by the host.
				unsigned char cliId;
				bts.Read(cliId);
				clientId = cliId;


				unsigned char numActions;
				bts.Read(numActions);


				for (unsigned char a = 0; a < numActions; a++)
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
			else if (msgtype == MESSAGE_ROUTINE_TICKSYNC)
			{
				unsigned char cliId;
				bts.Read(cliId);


				unsigned int client_tickIdx;
				bts.Read(client_tickIdx);

				int tickTime;
				bts.Read(tickTime);

				int offset = int(client_tickIdx) - int(gameState->tickIdx);

				clientMeta* meta = GetClientMetaDataFromCliId(cliId);
				meta->hostTickOffset = offset;
			}

			break;
		case ID_REMOTE_DISCONNECTION_NOTIFICATION:
			std::cout << "Peer: Remote peer has disconnected." << std::endl;

				

			break;
		case ID_REMOTE_CONNECTION_LOST:
			std::cout << "Peer: Remote peer lost connection." << std::endl;
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			std::cout << "Peer: A peer has disconnected." << std::endl;
			break;
		case ID_CONNECTION_LOST:
			std::cout << "Peer: A connection was lost." << std::endl;
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			std::cout << "Peer: A connection failed." << std::endl;
			break;
		default:

			std::cout << "Peer: Received a packet with unspecified message identifier: " << int(this->packet->data[0]) << std::endl;


			break;
		} // check package identifier
	} // package receive loop




}
