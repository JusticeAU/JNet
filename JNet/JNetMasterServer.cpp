#include "JNetMasterServer.h"
#include "enet/enet.h"

#include <random>

JNet::MasterServer::MasterServer()
{
	m_address = new ENetAddress();
	m_address->host = ENET_HOST_ANY;
	m_address->port = 6000;
}

JNet::MasterServer::~MasterServer()
{
}

void JNet::MasterServer::Initialize()
{
	if (enet_initialize() != 0)
		std::cout << "Enet failed to initialise." << std::endl;

	m_ENetServer = enet_host_create(m_address, 32, 2, 0, 0);

	if (m_ENetServer != nullptr)
		std::cout << "Successfully created Server" << std::endl;


	// Create two dummy servers
	BalancedServerReference server1;
	server1.name = "Australia 1";
	server1.address = "192.168.1.0";
	server1.port = 6050;
	m_balancedServers.push_back(server1);

	BalancedServerReference server2;
	server2.name = "Australia 2";
	server2.address = "192.168.1.0";
	server2.port = 6051;
	m_balancedServers.push_back(server2);
}

void JNet::MasterServer::Run()
{
	while (true)
	{
		Process();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

void JNet::MasterServer::Process()
{
	// handle enet buffered packets
	ENetEvent receivedEvent;
	while (enet_host_service(m_ENetServer, &receivedEvent, 100) > 0)
	{
        std::cout << "Master Server received a packet!" << std::endl;
		// switch on packet type
        switch (receivedEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "A new client connected to the Master Server" << std::endl;
			std::cout << "Redirect user to " << m_balancedServers[m_nextServer].name << std::endl;
			
			ENetPeer* serverPeer = nullptr;
			serverPeer = receivedEvent.peer;
			ENetPacket* infoPacket = enet_packet_create(&m_balancedServers[m_nextServer], sizeof(BalancedServerReference), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(serverPeer, 0, infoPacket);

			m_nextServer++;
			if (m_nextServer == m_balancedServers.size()) m_nextServer = 0;

            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "A client disconnected from the Master Server" << std::endl;
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::cout << "We received a user-defined packet!" << std::endl;
            break;
        }
        default:
        {
            std::cout << "We received some other data!" << std::endl;
            break;
        }
        }
	}

	// if auth request, qualify, determine server, respond
}
