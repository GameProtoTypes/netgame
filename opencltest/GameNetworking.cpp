#include "GameNetworking.h"

 void GameNetworking::Init()
{
	std::cout << "GameNetworking::Init" << std::endl;

	this->peerInterface = SLNet::RakPeerInterface::GetInstance();




	// Start the thread for packet receiving
	//this->isListening = true;
	//this->listenLoopThread = new std::thread(&GameNetworking::ListenLoop, this);


	std::cout << "GameNetworking::Init  End" << std::endl;
}

 void GameNetworking::CLIENT_SendActionUpdate_ToHost(std::vector<ClientAction>& clientActions)
 {
	 if (actionStateDirty) {
		 std::cout << "Sending ActionList Update to host " << hostAddr.ToString() << std::endl;


		 SLNet::BitStream bs;
		 bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_CLIENT_ACTIONUPDATE));
		 bs.Write(static_cast<unsigned int>(clientActions.size() * sizeof(ClientAction)));
		 bs.Write(static_cast<unsigned char>(clientActions.size()));
		 for (ClientAction& action : clientActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&action), sizeof(ClientAction));

		 }
		 //broadcast to host
		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, SLNet::UNASSIGNED_RAKNET_GUID, true);


		 actionStateDirty = false;
	 }
 }

 void GameNetworking::HOST_SendActionUpdates_ToClients()
 {
	 std::cout << "[HOST] Sending MESSAGE_ENUM_HOST_TURNDATA to " << clients.size() << " clients" << std::endl;
	 

	 for (auto client : clients)
	 {
		 SLNet::BitStream bs;
		 bs.Write(static_cast<unsigned char>(ID_USER_PACKET_ENUM));
		 bs.Write(static_cast<unsigned char>(MESSAGE_ENUM_HOST_TURNDATA));
		 bs.Write(static_cast<unsigned int>(clients.size() * sizeof(ClientAction)));
		 bs.Write(static_cast<unsigned char>(clients.size()));
		 bs.Write(static_cast<unsigned char>(client.first));
		 for (ClientAction& action : turnActions)
		 {
			 bs.Write(reinterpret_cast<char*>(&action), sizeof(ClientAction));
		 }

		 this->peerInterface->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 1, client.second, false);
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
	std::cout << "Connecting To Host: " << hostAddress.ToString(false) << " Port: " << hostAddress.GetPort() << std::endl;
	SLNet::SocketDescriptor desc;
	peerInterface->Startup(1, &desc, 1);
	bool connectInitSuccess = peerInterface->Connect(hostAddress.ToString(false), hostAddress.GetPort(), nullptr, 0, nullptr);
	
	if (connectInitSuccess != SLNet::CONNECTION_ATTEMPT_STARTED)
	{
		std::cout << "Connect Init Failure: " << int(connectInitSuccess) << std::endl;


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


 void GameNetworking::ListenLoop()
{

	//while (this->isListening)
	//{
		for (this->packet = this->peerInterface->Receive();
			this->packet;
			this->peerInterface->DeallocatePacket(this->packet),
			this->packet = this->peerInterface->Receive())
		{
			SLNet::BitStream bts(this->packet->data,
				this->packet->length,
				false);

			// Check the packet identifier
			switch (this->packet->data[0])
			{
			case ID_CONNECTION_REQUEST_ACCEPTED:
			{
				std::cout << "[CLIENT] Peer: ID_CONNECTION_REQUEST_ACCEPTED" << std::endl;
				hostAddr = this->packet->systemAddress;
				connectedToHost = true;

				if (serverRunning)
				{
					std::pair<unsigned char, SLNet::SystemAddress> cli;
					cli.first = nextCliIdx;
					cli.second = hostAddr;
					clients.push_back(cli);
					nextCliIdx++;
				}

			}
				break;
			case ID_NEW_INCOMING_CONNECTION:
			{
				std::cout << "[HOST] Peer: A peer " << this->packet->systemAddress.ToString(true)
					<< " has connected." << std::endl;


				std::pair<unsigned char, SLNet::SystemAddress> cli;
				cli.first = nextCliIdx;
				cli.second = this->packet->systemAddress;
				clients.push_back(cli);
				nextCliIdx++;

				HOST_SendSync_ToClient(clients.size() - 1, this->packet->systemAddress);

			}
				break;
			
			case ID_USER_PACKET_ENUM:
				unsigned char rcv_id;
				bts.Read(rcv_id);

				unsigned char msgtype;
				bts.Read(msgtype);


				unsigned int length;
				bts.Read(length);

				

				if (msgtype == MESSAGE_ENUM_GENERIC_MESSAGE)
				{	char message[MAX_USER_MESSAGE_LENGTH];

					bts.Read(message, length);
					message[length] = '\0';
					std::cout << "Peer: MESSAGE_ENUM_GENERIC_MESSAGE received: " << message << std::endl;
				}
				else if (msgtype == MESSAGE_ENUM_HOST_SYNCDATA)
				{
					std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_SYNCDATA received" << std::endl;

					bts.Read(localClientStateIdx);

				}
				else if (msgtype == MESSAGE_ENUM_CLIENT_ACTIONUPDATE)
				{

					unsigned char numActions;
					bts.Read(numActions);
					for (int i = 0; i < numActions; i++)
					{
						ClientAction action;
						bts.Read(reinterpret_cast<char*>(&action), length);
						turnActions.push_back(action);
					}
						
					
					std::cout << "[HOST] Peer: MESSAGE_ENUM_CLIENT_ACTIONUPDATE received" << std::endl;



					//all clients have given input
					//send combined final turn to all clients
					HOST_SendActionUpdates_ToClients();
					
				}
				else if (msgtype == MESSAGE_ENUM_HOST_TURNDATA)
				{
					std::cout << "[CLIENT] Peer: MESSAGE_ENUM_HOST_TURNDATA received" << std::endl;
					
					unsigned char numClients;
					bts.Read(numClients);
					gameState->numClients = numClients;

					//sync client id as assigned by the host.
					unsigned char cliId;
					bts.Read(cliId);

					localClientStateIdx = cliId;

					for (unsigned char cli = 0; cli < numClients; cli++)
					{
						ClientAction actionSet;
						bts.Read(reinterpret_cast<char*>(&actionSet), sizeof(ClientAction));
						turnActions.push_back(actionSet);
					}
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


	//} // listening loop

	

}
