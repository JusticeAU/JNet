#include "JNetMasterServer.h"
#include "JNetPackets.h"
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
		switch(receivedEvent.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			switch (receivedEvent.channelID)
			{
			case 0:
				InterpretUserPacket(receivedEvent);
				break;
			case 1:
				InterpretBalancedServerPacket(receivedEvent);
				break;
			default:
				break;
			}
			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
		{
			break;
		}
		}
	}
}

void JNet::MasterServer::InterpretUserPacket(_ENetEvent& receivedEvent)
{
	// switch on packet type
	JNetPacket* packet = (JNetPacket*)receivedEvent.packet->data;
	switch (packet->type)
	{
	case JNetPacketType::UAuth:
	{
		ENetPacket* infoPacket;
		ENetPeer* serverPeer = receivedEvent.peer;

		if (m_balancedServers.size() == 0)
		{
			JNet::ErrorMessage errorMsg;
			strcpy_s(errorMsg.message, "No Servers available. Please try again later");
			infoPacket = enet_packet_create(&errorMsg, sizeof(JNet::ErrorMessage), ENET_PACKET_FLAG_RELIABLE);
		}
		else
		{
			BalancedServerReference server = m_balancedServers[m_nextServer];
			std::cout << "Redirect user to " << server.name << std::endl; // should show username here for verbosity.
			JNet::MasterServerRedirect redirectTo;
			strcpy_s(redirectTo.name, server.name.c_str());
			strcpy_s(redirectTo.hostname, server.address.c_str());
			redirectTo.port = server.port;
			infoPacket = enet_packet_create(&redirectTo, sizeof(JNet::MasterServerRedirect), ENET_PACKET_FLAG_RELIABLE);

			m_nextServer++;
			if (m_nextServer == m_balancedServers.size()) m_nextServer = 0;
		}

		enet_peer_send(serverPeer, 0, infoPacket);
		enet_peer_disconnect_later(serverPeer, 0);
		break;
	}
	default:
	{
		std::cout << "We received some other data!" << std::endl;
		break;
	}
	}
}

void JNet::MasterServer::InterpretBalancedServerPacket(_ENetEvent& receivedEvent)
{
	JNetPacket* packet = (JNetPacket*)receivedEvent.packet->data;
	switch (packet->type)
	{
	case JNetPacketType::BSRegister:
		// Add to list
	{
		BalancedServerRegister* bsRegister = (BalancedServerRegister*)receivedEvent.packet->data;
		std::cout << "Received a register event from a Balanced Server" << std::endl;
		std::cout << "\tName: " << bsRegister->name << " - Hostname: " << bsRegister->hostname << " - Port: " << bsRegister->port << std::endl;
		BalancedServerReference server;
		server.name = bsRegister->name;
		server.address = bsRegister->hostname;
		server.port = bsRegister->port;
		m_balancedServers.push_back(server);
	}
		break;
	case JNetPacketType::BSUpdate:
		// Update this BS
	{
		BalancedServerUpdate* bsUpdate = (BalancedServerUpdate*)receivedEvent.packet->data;
		std::cout << "Received an update event from a Balanced Server" << std::endl;
		std::cout << "\tPlayer Count: " << bsUpdate->playerCount
			<< " - Player Capacity: " << bsUpdate->playerCapacity
			<< " - Session Count: " << bsUpdate->sessionCount
			<< " - Open for new connections: " << bsUpdate->open << std::endl;
	}
		break;
	default:
		std::cout << "Received an unknown packet type from Balanced Server" << std::endl;
	}
}
